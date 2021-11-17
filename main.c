/*
 *    BlankBrowser - browser with respect your monitor
 *    Copyright (C) 2021  Lach Sławomir <slawek@lach.art.pl>
 *    
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
//  *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <string.h>
#include <linux/limits.h>
#include <stdbool.h>

#include <sys/param.h>

struct wnd_data {

  GtkWidget *main_tab;
  GtkBox *tab_container;
  GtkWindow *m_wnd;
};

static WebKitWebView *get_webview(GtkWidget *widget);

static void real_window_resize(GtkWidget* self, int width, int height, gpointer user_data);
gboolean window_resize(GtkWidget* self, GdkEventConfigure *event, gpointer user_data);
static void aswitch_tab_btn(GtkWidget *widget, gpointer user_data);
static void real_new_tab(GtkWidget *widget, struct wnd_data *wnd_data);


static WebKitWebView *get_webview(GtkWidget *widget)
{
  WebKitWebView *view;
  GtkWidget *p, *pp;
  
  p = gtk_widget_get_parent((GtkWidget*)widget);
  
  while (p) {
    
    pp = p;
    
    if ((view = g_object_get_data((GObject*)pp, "webview")) != NULL) {
      
      return view;
    }
    p = gtk_widget_get_parent(p);
  }
  
  return NULL;
}

static void go_to_addr(GtkEntry *entry, gpointer user_data)
{
  WebKitWebView *view = get_webview((GtkWidget*) entry);
  
  webkit_web_view_load_uri(view, gtk_entry_get_text(entry));
}

static void create_new_tab(GtkEntry *entry, gpointer user_data)
{
  WebKitWebView *view = NULL;
  GtkWidget *p, *pp;
  struct wnd_data *wnd = (struct wnd_data*) user_data;
  
  p = gtk_widget_get_parent((GtkWidget*)entry);
  
  while (p) {
    
    pp = p;
    
    if (GTK_IS_NOTEBOOK(p)) {
      
      break;
    }
    p = gtk_widget_get_parent(p);
  }
  
  
  real_new_tab(entry, wnd);
  
  gtk_notebook_set_current_page(pp, -1);
}

static void goto_info_page(GtkEntry *entry, GdkEvent *event, gpointer user_data)
{
  (void) entry;
  (void) event;
  
  //WebKitWebView *view = (WebKitWebView *) ((struct wnd_data*)user_data)->current_tab;
  /* For some reason, no new item was inserted into history, but current is replaced */
#if 0
  webkit_web_view_load_html(view, "<html><head><title>About BlankBrowser</title></head><body><h1>License</h1><p><strong>Copyright 2021</strong> by Sławomir Lach s l a w e k @ l a c h . a r t . p l</p><p>BlankBrowser is under <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU/GPLv3</a></p></body></html>", NULL);
#else
  //webkit_web_view_load_uri(view, "data:text/html;charset=utf-8,<html><head><title>About BlankBrowser</title></head><body><h1>License</h1><p><strong>Copyright 2021</strong> by Sławomir Lach s l a w e k @ l a c h . a r t . p l</p><p>BlankBrowser is under <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU/GPLv3</a></p></body></html>");
#endif
}

static void back_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) widget;
  (void) event;
  
  WebKitWebView *view = get_webview(widget);
  webkit_web_view_go_back((WebKitWebView *) view);
}

static void forward_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) widget;
  (void) event;
  
  WebKitWebView *view = get_webview(widget);
  webkit_web_view_go_forward((WebKitWebView *) view);
}

static void load_fnc(WebKitWebView *web_view, WebKitLoadEvent load_event, gpointer user_data)
{
  const char *title = NULL;
  struct wnd_data *wnd_data = (struct wnd_data*) user_data;
  
  if (WEBKIT_LOAD_STARTED == load_event) {
  
    title = webkit_web_view_get_uri(web_view);
  }
  if (WEBKIT_LOAD_FINISHED == load_event) {
    
   title = webkit_web_view_get_title(web_view);
  }
  
  if (NULL != title) {
    gtk_label_set_label(g_object_get_data((GObject*)web_view, "title_tab_container"), 
                            title);
    gtk_window_set_title((GtkWindow*)wnd_data->m_wnd, title);
  }
}

gboolean close_app(GtkWidget *widget,GdkEvent *event, gpointer user_data)
{
  (void) widget;
  (void) event;
  (void) user_data;
  
  exit(0);
}

static GtkWidget *create_tab(const char *uri, struct wnd_data *wnd_data)
{  
  WebKitWebView *webContent;
  
  webContent = (WebKitWebView*) webkit_web_view_new();
  webkit_web_view_load_uri(webContent, uri);
  
  return (GtkWidget*)webContent;
}

static void aclose_tab(GtkWidget *widget, gpointer user_data)
{
  
  GtkWidget *p, *pp;
  
  pp = (GtkWidget*)user_data;
  p = gtk_widget_get_parent(pp);
  
  while (p) {
    
    if (GTK_IS_NOTEBOOK(p)) {
      
      gtk_notebook_remove_page(p, gtk_notebook_page_num(p,pp) );
      break;
    }
    
    pp = p;
    p = gtk_widget_get_parent(p);
  }
}

static void real_new_tab(GtkWidget *widget, struct wnd_data *wnd_data)
{
GtkWidget  *info_btn;
GtkWidget *mWindow, *button, *new_tab_btn, *back, *forward, *home;
GtkBox *box;
GtkBox *navigation_btns;
GtkFixed *fixed;
GtkOverlay *overlay;
GtkEntry *url;
GtkNotebook *tabs;
GtkCssProvider *prov;
GValue value = G_VALUE_INIT;
GValue value2 = G_VALUE_INIT;

overlay = (GtkOverlay*) gtk_overlay_new();



fixed = (GtkFixed*) gtk_fixed_new();
box = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
url = (GtkEntry*)gtk_entry_new();


button = gtk_button_new_with_label("↓");
back = gtk_button_new_with_label("⇦");
forward = gtk_button_new_with_label("⇨");
home = gtk_button_new_with_label("⌂");
info_btn = gtk_button_new_with_label("i");

g_signal_connect(back, "button-press-event", G_CALLBACK(back_clicked), &wnd_data);
g_signal_connect(forward, "button-press-event", G_CALLBACK(forward_clicked), &wnd_data);

navigation_btns = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

gtk_box_pack_start((GtkBox*)navigation_btns, back, TRUE, FALSE, 0);
gtk_box_pack_start((GtkBox*)navigation_btns, home, TRUE, FALSE, 0);
gtk_box_pack_start((GtkBox*)navigation_btns, forward, TRUE, FALSE, 0);

gtk_widget_add_events((GtkWidget*)url, GDK_POINTER_MOTION_MASK);

prov = gtk_css_provider_new();
gtk_css_provider_load_from_data(prov, "* {background-color: rgba(65,10,20, 0.2);}", -1, NULL);

//gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget*)url), (GtkStyleProvider*) prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget*)box), (GtkStyleProvider*) prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget*)fixed), (GtkStyleProvider*) prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget*)navigation_btns), (GtkStyleProvider*) prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget*)button), (GtkStyleProvider*) prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget*)back), (GtkStyleProvider*) prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget*)forward), (GtkStyleProvider*) prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget*)url), (GtkStyleProvider*) prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

gtk_style_context_get_property(gtk_widget_get_style_context((GtkWidget*)url), "font-size", GTK_STATE_FLAG_NORMAL, &value);

g_value_init(&value2, G_TYPE_UINT);



if (!g_value_transform(&value, &value2)) {
  
  exit(1);
}

int height = g_value_get_uint(&value2);

gtk_box_pack_start((GtkBox*)box, (GtkWidget*)navigation_btns, 0, 0, 0);
gtk_box_pack_start((GtkBox*)box, (GtkWidget*)url, 1, 1, 0);
gtk_box_pack_start((GtkBox*)box, button, 0, 0,0);

g_object_set_data((GObject*)box, "navigation_btns", navigation_btns);
g_object_set_data((GObject*)box, "button", button);
g_object_set_data((GObject*)overlay, "button", button);
g_object_set_data((GObject*)overlay, "navigation_btns", navigation_btns);

gtk_fixed_put((GtkFixed*)fixed, (GtkWidget*)box, 0, 0);

WebKitWebView *wv =  (WebKitWebView *)create_tab("about:blank", wnd_data);
g_signal_connect(wv, "load-changed", G_CALLBACK(load_fnc), wnd_data);
gtk_overlay_add_overlay(overlay, (GtkWidget*)wv);

g_signal_connect(url, "activate", (GCallback) go_to_addr, NULL);


gtk_overlay_add_overlay(overlay, (GtkWidget*)fixed);

g_object_set_data((GObject*)fixed, "webview", wv);
g_object_set_data((GObject*)fixed, "nav_btn", navigation_btns);
gtk_overlay_set_overlay_pass_through(overlay, (GtkWidget*)fixed, TRUE);



  GtkLabel *tabLabel = gtk_label_new("");
  GtkBox *tabBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkButton *closeBtn = gtk_button_new_with_label("X");
  
  gtk_box_pack_start(tabBox, tabLabel, 0, 0, 2);
  gtk_box_pack_start(tabBox, closeBtn, 0, 0, 2);
  
  gtk_widget_show_all(tabBox);
  
  
  gint position = gtk_notebook_page_num(wnd_data->tab_container, wnd_data->main_tab);
  
  if (1 > position) {
    
    position = 1;
  }
  
  //gtk_notebook_append_page((GtkNotebook*)wnd_data->tab_container, (GtkWidget*)overlay, tabBox);
  
  gtk_notebook_insert_page((GtkNotebook*)wnd_data->tab_container, (GtkWidget*)overlay, tabBox, position - 1);
  
  gtk_widget_set_opacity((GtkWidget*)button, 0.7);
  gtk_widget_set_opacity((GtkWidget*)home, 0.7);
  gtk_widget_set_opacity((GtkWidget*)back, 0.7);
  gtk_widget_set_opacity((GtkWidget*)forward, 0.7);
  g_signal_connect((GObject*)wnd_data->m_wnd, "configure-event", (GCallback)window_resize, box);
  
  
  g_object_set_data(wv, "title_tab_container", tabLabel);
  g_signal_connect(closeBtn,"clicked", aclose_tab, wv);
  
  //gtk_widget_set_opacity(fixed, 50);
 // gtk_widget_set_opacity(box, 50);
  
   
  
  
  if (widget) {
    
    gtk_entry_set_text(url, gtk_entry_get_text(widget));
    
    go_to_addr(url, NULL);
  }
  real_window_resize(wnd_data->m_wnd, gtk_widget_get_allocated_width(wnd_data->m_wnd), gtk_widget_get_allocated_height(wnd_data->m_wnd), box);
  
  gtk_widget_show_all((GtkWidget*)overlay);
}

static void new_tab(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  
  struct wnd_data *wnd_data = (struct wnd_data*) user_data;
  real_new_tab(widget, wnd_data);
}


gboolean window_resize(GtkWidget* self, GdkEventConfigure *event, gpointer user_data)
{
  
  real_window_resize(self, event->width, event->height, user_data);
  return TRUE;
}

static void real_window_resize(GtkWidget* self, int width, int height, gpointer user_data) 
{
  GtkEntry *entry = (GtkEntry*) user_data;;
  GtkAllocation cAllocation;
  
  int mwidth = MAX(width * 0.8, width - 60);
  
  if (width - 60 < mwidth) {
    
    cAllocation.x = 0;
    mwidth = width;
    gtk_widget_set_visible(g_object_get_data((GObject*)gtk_widget_get_parent((GtkWidget*)entry), "nav_btn"), FALSE);
  }
  else {
    
    gtk_widget_set_visible(g_object_get_data((GObject*)gtk_widget_get_parent((GtkWidget*)entry), "nav_btn"), TRUE);
  }
  
  
  gtk_widget_get_allocation(entry, &cAllocation);
  
  cAllocation.width = mwidth;
  cAllocation.x = (width - mwidth) / 2;
  
  gtk_widget_size_allocate(entry, &cAllocation);
  
  gtk_fixed_move((GtkFixed*)gtk_widget_get_parent(entry), entry, cAllocation.x, 0);
  gtk_widget_set_size_request(entry, mwidth, cAllocation.height);
}


void create_main_page(GtkNotebook *notebook, struct wnd_data *wnd)
{
  GtkBox *box, *box2;
  GtkEntry *url;
  
  box = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  box2= (GtkBox*)gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  url = (GtkEntry*)gtk_entry_new();
  
  
  
  gtk_box_pack_start(box2, url, 1,1,0);
  gtk_box_pack_start(box, box2, 1,1,0);
  
  gtk_notebook_append_page(notebook, box, NULL);
  
  wnd->main_tab = box;
  
  g_signal_connect((GObject*) url, "activate", create_new_tab, wnd);
  
  GtkLabel *license = gtk_label_new(NULL);
  
  gtk_label_set_markup(license, "<b><i>License</i></b>\n<b>Copyright 2021</b> by Sławomir Lach s l a w e k @ l a c h . a r t . p l\nBlankBrowser is under <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU/GPLv3</a>");
  
  gtk_box_pack_start(box, license, 1, 1, 0);
  
  gtk_widget_show_all(box);
}

int main(int argc, char **argv)
{
  struct wnd_data wnd_data;
  GtkWidget  *info_btn;
  GtkWidget *mWindow, *button, *new_tab_btn, *back, *forward, *home;
  GtkNotebook *tabs;
  GtkCssProvider *prov;
  GValue value = G_VALUE_INIT;
  GValue value2 = G_VALUE_INIT;
  
  gtk_init(&argc, &argv);
  
  //webkit_web_context_set_web_extensions_directory (webkit_web_context_get_default (), 
  //                                                 INSTALL_PREFIX "lib/extensions");
  download_manager_init();
  
  
  tabs = gtk_notebook_new();
  
  create_main_page(tabs, &wnd_data);
  
  mWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  wnd_data.m_wnd = (GtkWindow*) mWindow;
  
  g_signal_connect((gpointer)mWindow, "delete-event", (GCallback)close_app, NULL);
  
  
  gtk_widget_show((GtkWidget*)new_tab_btn);
  
  gtk_window_set_default_size((GtkWindow*)mWindow, 800, 600);
  
  wnd_data.tab_container = tabs;
  

  gtk_container_add((GtkContainer*)mWindow, (GtkWidget*)tabs);
  
  gtk_widget_show_all((GtkWidget*)mWindow);

  
  gtk_main();
}
