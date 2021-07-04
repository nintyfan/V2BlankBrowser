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

struct wnd_data {

  WebKitWebView *current_tab;
  GtkEntry *entry;
  GtkBox *tab_container;
  unsigned int menu_items;
  GtkWindow *m_wnd;
};

static void aswitch_tab_btn(GtkWidget *widget, gpointer user_data);
static void real_new_tab(GtkWidget *widget, struct wnd_data *wnd_data);

static gboolean show_button( GtkWidget *widget, GdkEventMotion *event ) {

  int set_to_y = 0;
  GtkWidget *parent = gtk_widget_get_parent((GtkWidget*)widget);
  GtkWidget *overlay = gtk_widget_get_parent((GtkWidget*)gtk_widget_get_parent(parent));
  
  if (g_object_get_data((GObject*)overlay, "locked")) {
  
    return TRUE;
  }
  g_object_set_data((GObject*)overlay, "locked", (gpointer)(intptr_t)TRUE);
  
  GtkWidget *button = g_object_get_data((GObject*)parent, "button");
  GtkBox *box = g_object_get_data((GObject*)parent, "navigation_btns");
  
  GtkAllocation *allocation = g_new(GtkAllocation, 1);
  gtk_widget_get_allocation((GtkWidget*)widget, allocation);
  
  if (allocation->y != 0) {
  
    set_to_y = allocation->y - 100 + allocation->height;
  }
  gtk_fixed_move((GtkFixed*)gtk_widget_get_parent((GtkWidget*)parent), (GtkWidget*)box, allocation->width - 10, set_to_y);
  
  g_free(allocation);
  
  gtk_widget_set_visible((GtkWidget*)button, TRUE); 
  gtk_widget_set_visible((GtkWidget*)box, TRUE); 
  
  gtk_fixed_move((GtkFixed*)parent, button, event->x, event->y);
  
  return TRUE;
}


static gboolean hide_button( GtkWidget *widget, GdkEventMotion *event ) {
  
  (void) event;
  
  GtkWidget *button = g_object_get_data((GObject*)widget, "button");
  GtkWidget *box = g_object_get_data((GObject*)widget, "navigation_btns");
  g_object_set_data((GObject*)widget, "locked", NULL);
  
  gtk_widget_set_visible((GtkWidget*)box, FALSE); 
  gtk_widget_set_visible((GtkWidget*)button, FALSE); 
  return TRUE;
}

static void go_to_addr(GtkEntry *entry, gpointer user_data)
{
  WebKitWebView *view = (WebKitWebView *) ((struct wnd_data*)user_data)->current_tab;
  
  webkit_web_view_load_uri(view, gtk_entry_get_text(entry));
}

static void goto_info_page(GtkEntry *entry, GdkEvent *event, gpointer user_data)
{
  (void) entry;
  (void) event;
  
  WebKitWebView *view = (WebKitWebView *) ((struct wnd_data*)user_data)->current_tab;
  /* For some reason, no new item was inserted into history, but current is replaced */
#if 0
  webkit_web_view_load_html(view, "<html><head><title>About BlankBrowser</title></head><body><h1>License</h1><p><strong>Copyright 2021</strong> by Sławomir Lach s l a w e k @ l a c h . a r t . p l</p><p>BlankBrowser is under <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU/GPLv3</a></p></body></html>", NULL);
#else
  webkit_web_view_load_uri(view, "data:text/html;charset=utf-8,<html><head><title>About BlankBrowser</title></head><body><h1>License</h1><p><strong>Copyright 2021</strong> by Sławomir Lach s l a w e k @ l a c h . a r t . p l</p><p>BlankBrowser is under <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU/GPLv3</a></p></body></html>");
#endif
}

static void back_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) widget;
  (void) event;
  
  WebKitWebView *view = (WebKitWebView *) ((struct wnd_data*)user_data)->current_tab;
  webkit_web_view_go_back((WebKitWebView *) view);
}

static void forward_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) widget;
  (void) event;
  
  WebKitWebView *view = (WebKitWebView *) ((struct wnd_data*)user_data)->current_tab;
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
    gtk_menu_item_set_label(g_object_get_data((GObject*)web_view, "title_tab_container"), 
                            title);
    gtk_window_set_title((GtkWindow*)wnd_data->m_wnd, title);
  }
}

static void teleport_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  
  GtkButton *button;
  GtkBox *fixed1 = (GtkBox*) user_data;
  GtkBox *fixed2 = (GtkBox*) gtk_widget_get_parent((GtkWidget*)fixed1); 
  GtkWidget *box = g_object_get_data((GObject*)fixed1, "navigation_btns");
  const char *data = g_object_get_data((GObject*)widget, "position");
  if (data && 0 == strcmp(data, "down")) {
  
     gtk_button_set_label((GtkButton*)widget, "↓");
     g_object_set_data((GObject*)widget, "position", "up");
     
     gtk_fixed_move((GtkFixed*)fixed2, (GtkWidget*)fixed1, 0, 0);
     gtk_fixed_move((GtkFixed*)fixed2, box, 0, 0);
  }
  else {
    
    int h;
    GtkAllocation* allocation = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation((GtkWidget*)fixed2, allocation);
    
    h = allocation->height;
    
    gtk_widget_get_allocation((GtkWidget*)widget, allocation);
    
    gtk_button_set_label((GtkButton*)widget, "↑"); 
    g_object_set_data((GObject*)widget, "position", "down");
    
    h = h - allocation->height;
    
    gtk_fixed_move((GtkFixed*)fixed2, (GtkWidget*)fixed1, 0, h);
    
    gtk_widget_get_allocation((GtkWidget*)fixed2, allocation);
    
    h = allocation->height;
    gtk_widget_get_allocation((GtkWidget*)box, allocation);
    h = h - allocation->height;
    
    gtk_fixed_move((GtkFixed*)fixed2, box, 0, h);
    g_free(allocation);
  }
  
  box = g_object_get_data((GObject*)fixed1, "navigation_btns");
  button = g_object_get_data((GObject*)fixed1, "button");
  gtk_widget_set_visible((GtkWidget*)button, FALSE);  
  gtk_widget_set_visible((GtkWidget*)box, FALSE);  
  g_object_set_data((GObject*)gtk_widget_get_parent((GtkWidget*)fixed1), "locked", NULL);
}

gboolean close_app(GtkWidget *widget,GdkEvent *event, gpointer user_data)
{
  (void) widget;
  (void) event;
  (void) user_data;
  
  exit(0);
}

static void show_tabs(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  
  GtkWidget *menu = (GtkWidget *) user_data;

  gtk_menu_popup_at_widget((GtkMenu*)menu, widget, GDK_GRAVITY_CENTER, GDK_GRAVITY_CENTER, NULL);
}

static void switch_tab(const char *uri, struct wnd_data *wnd_data, WebKitWebView *webContent)
{
  GList *children = gtk_container_get_children((GtkContainer*)wnd_data->tab_container);
  
  if (children) {
    
    GtkWidget *wid = g_list_nth_data(children, 0);
    
    if (wid) {
      
      g_object_ref(wid);
      gtk_container_remove((GtkContainer*)wnd_data->tab_container, wid);
    }
  }
  g_clear_list (&children, NULL);
  
  gtk_entry_set_text(wnd_data->entry, uri);
  
  gtk_box_pack_start((GtkBox*)wnd_data->tab_container, (GtkWidget*)webContent, TRUE, TRUE, 0);
  
  wnd_data->current_tab = webContent;
  gtk_widget_show((GtkWidget*)webContent);
}

static GtkWidget *create_tab(const char *uri, struct wnd_data *wnd_data)
{  
  WebKitWebView *webContent;
  
  webContent = (WebKitWebView*) webkit_web_view_new();
  webkit_web_view_load_uri(webContent, uri);
  
  switch_tab(uri, wnd_data, webContent);
  
  return (GtkWidget*)webContent;
}

static void aclose_tab(GtkWidget *widget, gpointer user_data)
{
  GtkWidget *close_btn = widget;
  GtkWidget *switch_btn = g_object_get_data((GObject*)close_btn, "switch_btn");
  GtkWidget *menu = gtk_widget_get_parent((GtkWidget*)close_btn);
  
  struct wnd_data *wnd_data = (struct wnd_data*) g_object_get_data((GObject*)menu, "wnd");
  
  g_object_ref(close_btn);
  
  gtk_widget_destroy((GtkWidget*)g_object_get_data((GObject*)switch_btn, "webContent"));
  gtk_container_remove((GtkContainer*)menu, close_btn);
  gtk_container_remove((GtkContainer*)menu, switch_btn);
  
  GList *children = gtk_container_get_children((GtkContainer*)menu);
  
  GtkWidget *wid = g_list_nth_data(children, 1);
    
  if (wid) {
      
      
      switch_tab(webkit_web_view_get_uri(g_object_get_data((GObject*)wid, "webContent")), user_data, g_object_get_data((GObject*)wid, "webContent"));
      
      int i = 1;
      
      for (; i < wnd_data->menu_items + 1; i+=2) {
        
        gtk_menu_attach((GtkMenu*) menu, g_list_nth_data(children, i), 0, 1 , i, i+1);
        gtk_menu_attach((GtkMenu*) menu, g_list_nth_data(children, i+1), 2, 3, i, i+1 );
      }
      
  }
  else {
  
    real_new_tab( g_list_nth_data(children, 0), wnd_data);
  }
  
  g_clear_list (&children, NULL);
}

static void real_new_tab(GtkWidget *widget, struct wnd_data *wnd_data)
{
  GtkWidget *close_btn = gtk_menu_item_new_with_label("X");
  GtkWidget *switch_btn = gtk_menu_item_new_with_label("New Page");
  GtkWidget *tabs_menu = gtk_widget_get_parent((GtkWidget*)widget);
  WebKitWebView *wv =  (WebKitWebView *)create_tab("about:blank", wnd_data);
  
   
   gtk_menu_attach((GtkMenu*)tabs_menu, switch_btn, 0, 1, wnd_data->menu_items, wnd_data->menu_items + 1);
   gtk_menu_attach((GtkMenu*)tabs_menu, close_btn, 2, 3, wnd_data->menu_items, wnd_data->menu_items + 1);
   g_object_set_data((GObject*)switch_btn, "webContent", wv);
   g_object_set_data((GObject*)close_btn, "switch_btn", switch_btn);
   g_object_set_data((GObject*)wv, "title_tab_container", switch_btn);
   gtk_widget_show_all((GtkWidget*)switch_btn);
   gtk_widget_show_all((GtkWidget*)close_btn);
  
  ++(wnd_data->menu_items);

  g_signal_connect(wv, "load-changed", G_CALLBACK(load_fnc), wnd_data);
  
  g_signal_connect(close_btn, "activate", G_CALLBACK(aclose_tab), wnd_data);
  g_signal_connect(switch_btn, "activate", G_CALLBACK(aswitch_tab_btn), wnd_data);
}

static void aswitch_tab_btn(GtkWidget *widget, gpointer user_data)
{
  
  WebKitWebView *wv = (WebKitWebView*) g_object_get_data((GObject*)widget, "webContent");
  switch_tab(webkit_web_view_get_uri(wv), ((struct wnd_data*) user_data), wv);
}

static void new_tab(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  
  struct wnd_data *wnd_data = (struct wnd_data*) user_data;
  real_new_tab(widget, wnd_data);
}

int main(int argc, char **argv)
{
  struct wnd_data wnd_data;
  GtkWidget  *info_btn;
  GtkWidget *mWindow, *button, *new_tab_btn, *back, *forward, *home, *tabs, *tabs_menu;
  GtkFixed *box;
  GtkBox *navigation_btns;
  GtkFixed *fixed;
  GtkOverlay *overlay;
  GtkBox *webContainer;
  GtkEntry *url;
  GtkCssProvider *prov;
  GValue value = G_VALUE_INIT;
  GValue value2 = G_VALUE_INIT;
  
  /* 0 + add new tab */
  wnd_data.menu_items = 1;
  
  gtk_init(&argc, &argv);
  
  //webkit_web_context_set_web_extensions_directory (webkit_web_context_get_default (), 
  //                                                 INSTALL_PREFIX "lib/extensions");
  mWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  wnd_data.m_wnd = (GtkWindow*) mWindow;
  
  g_signal_connect((gpointer)mWindow, "delete-event", (GCallback)close_app, NULL);
  
  tabs_menu = gtk_menu_new();
  
  new_tab_btn = gtk_menu_item_new_with_label("+");
  
  
  gtk_menu_attach((GtkMenu*)tabs_menu, new_tab_btn, 0, 3, 0, 1);
  
  g_object_set_data((GObject*)tabs_menu, "wnd", &wnd_data);
  gtk_widget_show((GtkWidget*)new_tab_btn);
  
  gtk_window_set_default_size((GtkWindow*)mWindow, 800, 600);
  
  overlay = (GtkOverlay*) gtk_overlay_new();
  


  fixed = (GtkFixed*) gtk_fixed_new();
  box = (GtkFixed*)gtk_fixed_new();
  url = (GtkEntry*)gtk_entry_new();
  
  webContainer = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  
 
  button = gtk_button_new_with_label("↓");
  back = gtk_button_new_with_label("⇦");
  forward = gtk_button_new_with_label("⇨");
  home = gtk_button_new_with_label("⌂");
  tabs = gtk_button_new_with_label("⮛");
  info_btn = gtk_button_new_with_label("i");
  
  g_signal_connect(tabs, "button-press-event", G_CALLBACK(show_tabs), tabs_menu);
  
  g_signal_connect(back, "button-press-event", G_CALLBACK(back_clicked), &wnd_data);
  g_signal_connect(forward, "button-press-event", G_CALLBACK(forward_clicked), &wnd_data);
  
  navigation_btns = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);// gtk_fixed_new((GtkFixed*));
  
  gtk_box_pack_start((GtkBox*)navigation_btns, back, TRUE, FALSE, 0);
  gtk_box_pack_start((GtkBox*)navigation_btns, home, TRUE, FALSE, 0);
  gtk_box_pack_start((GtkBox*)navigation_btns, forward, TRUE, FALSE, 0);
  gtk_box_pack_start((GtkBox*)navigation_btns, tabs, TRUE, FALSE, 0);
  gtk_box_pack_start((GtkBox*)navigation_btns, info_btn, TRUE, FALSE, 0);
  
  gtk_widget_add_events((GtkWidget*)url, GDK_POINTER_MOTION_MASK);
  
  
  g_signal_connect(info_btn, "button-press-event", G_CALLBACK(goto_info_page), &wnd_data);
  g_signal_connect(new_tab_btn, "button-press-event", G_CALLBACK(new_tab), &wnd_data);
  g_signal_connect(button, "button-press-event", G_CALLBACK(teleport_clicked), box);
  //gtk_widget_add_events((GtkWidget*)box, GDK_POINTER_MOTION_MASK);
 // g_signal_connect (url, "draw",G_CALLBACK(draw_url), NULL);
  g_signal_connect (url, "motion_notify_event",G_CALLBACK(show_button), NULL);
  g_signal_connect (overlay, "motion_notify_event",G_CALLBACK(hide_button), NULL);
  
  prov = gtk_css_provider_new();
  gtk_css_provider_load_from_data(prov, "* {background-color: rgba(65,10,20, 0.8);}", -1, NULL);
  
  gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget*)url), (GtkStyleProvider*) prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  gtk_style_context_get_property(gtk_widget_get_style_context((GtkWidget*)url), "font-size", GTK_STATE_FLAG_NORMAL, &value);
  
  g_value_init(&value2, G_TYPE_UINT);
  
  if (!g_value_transform(&value, &value2)) {
  
    exit(1);
  }
  
  int height = g_value_get_uint(&value2);
  

  //gtk_fixed_put((GtkFixed*)fixed, box, 10, 10);
  //gtk_fixed_put((GtkFixed*)fixed, button, 10, 10);
  
  gtk_fixed_put((GtkFixed*)box, (GtkWidget*)url, 0, 0);
  gtk_fixed_put((GtkFixed*)box, button, 100, 0);
  
  
  gtk_fixed_put((GtkFixed*)navigation_btns, forward, 0, 0);
  gtk_fixed_put((GtkFixed*)navigation_btns, home, 0, 20);
  gtk_fixed_put((GtkFixed*)navigation_btns, back, 0, 40);
  
  g_object_set_data((GObject*)box, "navigation_btns", navigation_btns);
  g_object_set_data((GObject*)box, "button", button);
  g_object_set_data((GObject*)overlay, "button", button);
  g_object_set_data((GObject*)overlay, "navigation_btns", navigation_btns);

  gtk_widget_set_size_request((GtkWidget*)box, 100, height);
  gtk_widget_set_size_request((GtkWidget*)url, 80, height);
  gtk_widget_set_size_request((GtkWidget*)button, 20, height);
  gtk_widget_set_size_request((GtkWidget*)navigation_btns, 20, 100);
  gtk_widget_set_size_request((GtkWidget*)fixed, 100, height);
  
  gtk_fixed_put((GtkFixed*)fixed, (GtkWidget*)navigation_btns, 0, 0);
  
  gtk_fixed_put((GtkFixed*)fixed, (GtkWidget*)box, 0, 0);
  
  g_object_set_data((GObject*)webContainer, "urlbar", url);
  gtk_overlay_add_overlay(overlay, (GtkWidget*)webContainer);
  gtk_overlay_add_overlay(overlay, (GtkWidget*)fixed);
  gtk_overlay_set_overlay_pass_through(overlay, (GtkWidget*)fixed, TRUE);
  
  wnd_data.entry = url;
  wnd_data.tab_container = webContainer;

  real_new_tab(new_tab_btn, &wnd_data);
  
  gtk_container_add((GtkContainer*)mWindow, (GtkWidget*)overlay);
  
  gtk_widget_show_all((GtkWidget*)mWindow);
  gtk_widget_set_visible((GtkWidget*)button, FALSE); 
  gtk_widget_set_visible((GtkWidget*)navigation_btns, FALSE); 

  g_signal_connect (url, "activate", G_CALLBACK (go_to_addr), &wnd_data);
  
  gtk_main();
}
