/*
 *    V2BlankBrowser - browser with respect your monitor
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
  GtkWidget *v1_entry;
  GtkBox *v1_tab_container;
  GtkBox *tab_container;
  GtkWindow *m_wnd;
  guint32 r_click_time;
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


static void home_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) widget;
  (void) event;
  
  WebKitWebView *view = get_webview(widget);
  webkit_web_view_load_uri((WebKitWebView *) view, "https://www.patreon.com/easylinux");
}

static void load_fnc(WebKitWebView *web_view, WebKitLoadEvent load_event, gpointer user_data)
{
  const char *title = NULL;
  const char *uri = webkit_web_view_get_uri(web_view);;
  struct wnd_data *wnd_data = (struct wnd_data*) user_data;
  
  if (WEBKIT_LOAD_STARTED == load_event) {
  
    title = uri;
  }
  if (WEBKIT_LOAD_FINISHED == load_event) {
    
   title = webkit_web_view_get_title(web_view);
  }
  
  if (NULL != title) {
    gtk_entry_set_text(g_object_get_data(web_view, "url"), uri);
    
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
static void switch_tab(GtkNotebook* self, GtkWidget* page, guint page_num, gpointer user_data)
{
  struct wnd_data *m_wnd= (struct wnd_data *) user_data;
  GtkBox *box = g_object_get_data(page, "box");
  
  if (NULL == box) return;
  
  real_window_resize(m_wnd->m_wnd, gtk_widget_get_allocated_width(m_wnd->m_wnd), gtk_widget_get_allocated_height(m_wnd->m_wnd), box);
}

gboolean show_widgets(GtkWidget* self, GdkEventButton *event, gpointer user_data)
{
  gtk_widget_set_visible(user_data, TRUE);
  
  return FALSE;
}

gboolean event_event(GtkWidget* self, GdkEventButton *event, gpointer user_data)
{
  struct wnd_data *ev_store = (struct wnd_data*) user_data;
  
  if (event->button == 3 && ev_store->r_click_time  == 0) {  
 
     ev_store->r_click_time = event->time;
     
     gtk_main();
     
     if (ev_store->r_click_time) {
  
       // hidding
       GtkWidget *p = self;
       
       while (p) {
       
         if (GTK_IS_FIXED(p)) {
         
           break;
        }
         p = gtk_widget_get_parent(p);
       }
       
       
       ev_store->r_click_time = 0;
       if (!p) {
       
         return FALSE;
       }
       
       gtk_widget_set_visible(p,FALSE);
       return TRUE;
    }
    ev_store->r_click_time = 0;
  }
  
  return FALSE;
}

gboolean event_event2(GtkWidget* self, GdkEventButton *event, gpointer user_data)
{

  struct wnd_data *ev_store = (struct wnd_data*) user_data;
  
  if (event->button != 3 || ev_store->r_click_time == 0) {
  
    return FALSE;
  }

  
  if (ev_store->r_click_time + 5000 < event->time) {
  
    ev_store->r_click_time = 0;
  }
  gtk_main_quit();
  
}


static gboolean toolbox_enter(GtkWidget *wid, GdkEventCrossing *event)
{
  GtkCssProvider *prov;
  
  while (wid) {
  
    
    if (GTK_IS_FIXED(wid)) {
    
      break;
    }
    
    wid = gtk_widget_get_parent(wid);
  }
  
  if (! wid) {
  
    return FALSE;
  }
  
  prov = g_object_get_data(wid, "css");
  
  gtk_css_provider_load_from_data(prov, "* {}", -1, NULL);
  return FALSE;
}


static gboolean toolbox_leave(GtkWidget *wid, GdkEventCrossing *event)
{
  GtkCssProvider *prov;
  
  while (wid) {
    
    
    if (GTK_IS_FIXED(wid)) {
      
      break;
    }
    
    wid = gtk_widget_get_parent(wid);
  }
  
  if (! wid) {
    
    return FALSE;
  }
  
  prov = g_object_get_data(wid, "css");
  
  
  gtk_css_provider_load_from_data(prov, "* {background-color: rgba(65,10,20, 0.2);}", -1, NULL);
  return FALSE;
}

static void real_new_tab(GtkWidget *widget, struct wnd_data *wnd_data)
{
#define OVERLAY_OVER_OVERLAY
GtkWidget  *info_btn;
GtkWidget *mWindow, *button, *new_tab_btn, *back, *forward, *home;
GtkBox *box;
GtkBox *navigation_btns;
GtkFixed *fixed;
GtkOverlay *overlay;
#ifdef OVERLAY_OVER_OVERLAY
GtkOverlay *root_overlay, *helper_overlay;
#endif
GtkEntry *url;
GtkNotebook *tabs;
GtkCssProvider *prov;
GValue value = G_VALUE_INIT;
GValue value2 = G_VALUE_INIT;
GtkEventBox *eb = gtk_event_box_new();

overlay = (GtkOverlay*) gtk_overlay_new();
#ifdef OVERLAY_OVER_OVERLAY
root_overlay = gtk_overlay_new();
helper_overlay = gtk_overlay_new();
#endif


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
g_signal_connect(home, "button-press-event", G_CALLBACK(home_clicked), &wnd_data);

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

g_object_set_data(fixed, "css", prov);

gtk_widget_set_events ( button , GDK_ENTER_NOTIFY_MASK);
gtk_widget_add_events ( button , GDK_LEAVE_NOTIFY_MASK);
gtk_widget_set_events ( back , GDK_ENTER_NOTIFY_MASK);
gtk_widget_add_events ( back , GDK_LEAVE_NOTIFY_MASK);
gtk_widget_set_events ( forward , GDK_ENTER_NOTIFY_MASK);
gtk_widget_add_events ( forward , GDK_LEAVE_NOTIFY_MASK);
gtk_widget_set_events ( home, GDK_ENTER_NOTIFY_MASK);
gtk_widget_add_events ( home, GDK_LEAVE_NOTIFY_MASK);
gtk_widget_set_events ( url , GDK_ENTER_NOTIFY_MASK);
gtk_widget_add_events ( url , GDK_LEAVE_NOTIFY_MASK);

g_signal_connect (button, "enter-notify-event", toolbox_enter, NULL);
g_signal_connect (button, "leave-notify-event", toolbox_leave, NULL);
g_signal_connect (back, "enter-notify-event", toolbox_enter, NULL);
g_signal_connect (back, "leave-notify-event", toolbox_leave, NULL);
g_signal_connect (forward, "enter-notify-event", toolbox_enter, NULL);
g_signal_connect (forward, "leave-notify-event", toolbox_leave, NULL);
g_signal_connect (home, "enter-notify-event", toolbox_enter, NULL);
g_signal_connect (home, "leave-notify-event", toolbox_leave, NULL);
g_signal_connect (url, "enter-notify-event", toolbox_enter, NULL);
g_signal_connect (url, "leave-notify-event", toolbox_leave, NULL);

gtk_style_context_get_property(gtk_widget_get_style_context((GtkWidget*)url), "font-size", GTK_STATE_FLAG_NORMAL, &value);

g_value_init(&value2, G_TYPE_UINT);



if (!g_value_transform(&value, &value2)) {
  
  exit(1);
}

int height = g_value_get_uint(&value2);

gtk_box_pack_start((GtkBox*)box, (GtkWidget*)navigation_btns, 0, 0, 0);
gtk_box_pack_start((GtkBox*)box, (GtkWidget*)url, 1, 1, 0);
gtk_box_pack_start((GtkBox*)box, button, 0, 0,0);

gtk_widget_set_tooltip_text(url, "Right click to hide. Press right mouse button for more than five seconds to show menu. After hide, press on tab to unhide");
gtk_widget_set_tooltip_text(button, "Right click to hide. Press right mouse button for more than five seconds to show menu. After hide, press on tab to unhide");
gtk_widget_set_tooltip_text(home, "Right click to hide. Press right mouse button for more than five seconds to show menu. After hide, press on tab to unhide");
gtk_widget_set_tooltip_text(forward, "Right click to hide. Press right mouse button for more than five seconds to show menu. After hide, press on tab to unhide");
gtk_widget_set_tooltip_text(back, "Right click to hide. Press right mouse button for more than five seconds to show menu. After hide, press on tab to unhide");


g_object_set_data((GObject*)box, "navigation_btns", navigation_btns);
g_object_set_data((GObject*)box, "button", button);
#ifdef OVERLAY_OVER_OVERLAY
g_object_set_data((GObject*)root_overlay, "button", button);
g_object_set_data((GObject*)root_overlay, "navigation_btns", navigation_btns);
#else
g_object_set_data((GObject*)overlay, "button", button);
g_object_set_data((GObject*)overlay, "navigation_btns", navigation_btns);
#endif
gtk_fixed_put((GtkFixed*)fixed, (GtkWidget*)box, 0, 0);

WebKitWebView *wv =  (WebKitWebView *)create_tab("about:blank", wnd_data);
g_signal_connect(wv, "load-changed", G_CALLBACK(load_fnc), wnd_data);
gtk_overlay_add_overlay(overlay, (GtkWidget*)wv);

g_signal_connect(url, "activate", (GCallback) go_to_addr, NULL);

g_signal_connect(url, "button-press-event", event_event, wnd_data );
g_signal_connect(button, "button-press-event", event_event, wnd_data );
g_signal_connect(home, "button-press-event", event_event, wnd_data );
g_signal_connect(forward, "button-press-event", event_event,wnd_data );
g_signal_connect(back, "button-press-event", event_event, wnd_data );


g_signal_connect(url, "button-release-event", event_event2, wnd_data );
g_signal_connect(button, "button-release-event", event_event2, wnd_data );
g_signal_connect(home, "button-release-event", event_event2, wnd_data );
g_signal_connect(forward, "button-release-event", event_event2,wnd_data );
g_signal_connect(back, "button-release-event", event_event2, wnd_data );


gtk_overlay_add_overlay(overlay, (GtkWidget*)fixed);
#ifdef OVERLAY_OVER_OVERLAY
g_object_set_data((GObject*)root_overlay, "webview", wv);
g_object_set_data((GObject*)root_overlay, "nav_btn", navigation_btns);
g_object_set_data((GObject*)root_overlay, "box", box);
#else
g_object_set_data((GObject*)overlay, "webview", wv);
g_object_set_data((GObject*)overlay, "nav_btn", navigation_btns);
g_object_set_data((GObject*)overlay, "box", box);
#endif

gtk_overlay_set_overlay_pass_through(overlay, (GtkWidget*)fixed, TRUE);



  GtkLabel *tabLabel = gtk_label_new("");
  GtkBox *tabBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkButton *closeBtn = gtk_button_new_with_label("X");
  
  
  
  gtk_box_pack_start(tabBox, tabLabel, 0, 0, 2);
  gtk_box_pack_start(tabBox, closeBtn, 0, 0, 2);
  
  gtk_container_add(eb, tabBox);
  
  gtk_widget_show_all(tabBox);
  
  
  g_signal_connect(eb, "button-press-event", show_widgets, fixed);
  
  gint position = gtk_notebook_page_num(wnd_data->tab_container, wnd_data->main_tab);
  
  if (1 > position) {
    
    position = 1;
  }
  
#ifdef OVERLAY_OVER_OVERLAY
  gtk_overlay_add_overlay(root_overlay, overlay);
  gtk_overlay_add_overlay(root_overlay, helper_overlay);
  gtk_notebook_insert_page((GtkNotebook*)wnd_data->tab_container, (GtkWidget*)root_overlay, eb, position - 1);
#else
  gtk_notebook_insert_page((GtkNotebook*)wnd_data->tab_container, (GtkWidget*)overlay, eb, position - 1);
#endif
  
  #ifdef OVERLAY_OVER_OVERLAY
  gtk_overlay_set_overlay_pass_through(root_overlay, (GtkWidget*)overlay, TRUE);
  gtk_overlay_set_overlay_pass_through(root_overlay, (GtkWidget*)helper_overlay, TRUE);
  #endif
  
  g_object_set_data(wv, "title_tab_container", tabLabel);
  g_object_set_data(wv, "url", url);
  g_signal_connect(closeBtn,"clicked", aclose_tab, wv);
  
  if (widget) {
    
    gtk_entry_set_text(url, gtk_entry_get_text(widget));
    
    go_to_addr(url, NULL);
  }
  real_window_resize(wnd_data->m_wnd, gtk_widget_get_allocated_width(wnd_data->m_wnd), gtk_widget_get_allocated_height(wnd_data->m_wnd), box);
  
  
  // Called init v1 gui and pass overlay to it
#ifdef OVERLAY_OVER_OVERLAY  
  init_v1_ui(wnd_data, helper_overlay);
  
  gtk_widget_show_all((GtkWidget*)root_overlay);
#else
  init_v1_ui(wnd_data, overlay);
  
  gtk_widget_show_all((GtkWidget*)overlay);
#endif
  gtk_overlay_reorder_overlay(overlay, wv, 0);
  gtk_overlay_reorder_overlay(overlay, fixed, 1);
}

static void new_tab(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  
  struct wnd_data *wnd_data = (struct wnd_data*) user_data;
  real_new_tab(widget, wnd_data);
}


gboolean window_resize(GtkWidget* self, GdkEventConfigure *event, gpointer user_data)
{
  struct wnd_data *m_wnd= (struct wnd_data *) user_data;
  GtkWidget *page = gtk_notebook_get_nth_page(m_wnd->tab_container, gtk_notebook_get_current_page(m_wnd->tab_container));
  GtkBox *box = g_object_get_data(page, "box");
  
  puts("WILL RESIZE?");
  if (NULL == box) return TRUE;
  puts("WILL RESIZE");
  real_window_resize(self, event->width, event->height, box);
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
  
  gtk_label_set_markup(license, "<b><i>License</i></b>\n<b>Copyright 2021</b> by Sławomir Lach s l a w e k @ l a c h . a r t . p l\nV2BlankBrowser is under <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU/GPLv3</a>");
  
  gtk_box_pack_start(box, license, 1, 1, 0);
  
  gtk_widget_show_all(box);
}


static void teleport_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  
  GtkButton *button;
  GtkBox *fixed1 = (GtkBox*) user_data;
  GtkBox *fixed2 = (GtkBox*) gtk_widget_get_parent((GtkWidget*)fixed1); 
  GtkWidget *box = g_object_get_data((GObject*)fixed1, "v1_navigation_btns");
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
  
  box = g_object_get_data((GObject*)fixed1, "v1_navigation_btns");
  button = g_object_get_data((GObject*)fixed1, "v1_button");
  gtk_widget_set_visible((GtkWidget*)button, FALSE);  
  gtk_widget_set_visible((GtkWidget*)box, FALSE);  
  g_object_set_data((GObject*)gtk_widget_get_parent((GtkWidget*)fixed1), "locked", NULL);
}

static gboolean show_button( GtkWidget *widget, GdkEventMotion *event ) {
  
  int set_to_y = 0;
  GtkWidget *parent = gtk_widget_get_parent((GtkWidget*)widget);
  GtkWidget *overlay = gtk_widget_get_parent((GtkWidget*)gtk_widget_get_parent(parent));
  
  if (g_object_get_data((GObject*)overlay, "locked")) {
    
    return TRUE;
  }
  g_object_set_data((GObject*)overlay, "locked", (gpointer)(intptr_t)TRUE);
  
  GtkWidget *button = g_object_get_data((GObject*)parent, "v1_button");
  GtkBox *box = g_object_get_data((GObject*)parent, "v1_navigation_btns");
  
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
  
  GtkWidget *button = g_object_get_data((GObject*)widget, "v1_button");
  GtkWidget *box = g_object_get_data((GObject*)widget, "v1_navigation_btns");
  g_object_set_data((GObject*)widget, "locked", NULL);
  
  gtk_widget_set_visible((GtkWidget*)box, FALSE); 
  gtk_widget_set_visible((GtkWidget*)button, FALSE); 
  return TRUE;
}

static void show_tabs(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  
  GtkWidget *menu = (GtkWidget *) user_data;
  
  gtk_menu_popup_at_widget((GtkMenu*)menu, widget, GDK_GRAVITY_CENTER, GDK_GRAVITY_CENTER, NULL);
}

void init_v1_ui(struct wnd_data *wnd_data, GtkOverlay *overlay)
{
  GtkFixed *fixed;
  GtkWidget  *info_btn;
  GtkWidget  *button, *new_tab_btn, *back, *forward, *home, *tabs, *tabs_menu;
  GtkFixed *box;
  GtkBox *navigation_btns;
  GtkEntry *url;
  GtkCssProvider *prov;
  GValue value = G_VALUE_INIT;
  GValue value2 = G_VALUE_INIT;
  
  tabs_menu = gtk_menu_new();
  
  new_tab_btn = gtk_menu_item_new_with_label("+");
  
  
  gtk_menu_attach((GtkMenu*)tabs_menu, new_tab_btn, 0, 3, 0, 1);
  
  g_object_set_data((GObject*)tabs_menu, "wnd", wnd_data);
  gtk_widget_show((GtkWidget*)new_tab_btn);
  
  
  
  fixed = (GtkFixed*) gtk_fixed_new();
  box = (GtkFixed*)gtk_fixed_new();
  url = (GtkEntry*)gtk_entry_new();
  
 // webContainer = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  
  
  
  button = gtk_button_new_with_label("↓");
  back = gtk_button_new_with_label("⇦");
  forward = gtk_button_new_with_label("⇨");
  home = gtk_button_new_with_label("⌂");
  tabs = gtk_button_new_with_label("⮛");
  info_btn = gtk_button_new_with_label("i");
  
  g_signal_connect(tabs, "button-press-event", G_CALLBACK(show_tabs), tabs_menu);
  
  g_signal_connect(back, "button-press-event", G_CALLBACK(back_clicked), wnd_data);
  g_signal_connect(forward, "button-press-event", G_CALLBACK(forward_clicked), wnd_data);
  
  navigation_btns = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);// gtk_fixed_new((GtkFixed*));
  
  gtk_box_pack_start((GtkBox*)navigation_btns, back, TRUE, FALSE, 0);
  gtk_box_pack_start((GtkBox*)navigation_btns, home, TRUE, FALSE, 0);
  gtk_box_pack_start((GtkBox*)navigation_btns, forward, TRUE, FALSE, 0);
  gtk_box_pack_start((GtkBox*)navigation_btns, tabs, TRUE, FALSE, 0);
  gtk_box_pack_start((GtkBox*)navigation_btns, info_btn, TRUE, FALSE, 0);
  
  gtk_widget_add_events((GtkWidget*)url, GDK_POINTER_MOTION_MASK);
  
  
  g_signal_connect(info_btn, "button-press-event", G_CALLBACK(goto_info_page), wnd_data);
  g_signal_connect(new_tab_btn, "button-press-event", G_CALLBACK(new_tab), wnd_data);
  g_signal_connect(button, "button-press-event", G_CALLBACK(teleport_clicked), box);
  gtk_widget_add_events((GtkWidget*)box, GDK_POINTER_MOTION_MASK);
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
  gtk_fixed_put((GtkFixed*)box, button, 100, 300);
  
  
  gtk_fixed_put((GtkFixed*)navigation_btns, forward, 0, 0);
  gtk_fixed_put((GtkFixed*)navigation_btns, home, 0, 20);
  gtk_fixed_put((GtkFixed*)navigation_btns, back, 0, 40);
  
  g_object_set_data((GObject*)box, "v1_navigation_btns", navigation_btns);
  g_object_set_data((GObject*)box, "v1_button", button);
  g_object_set_data((GObject*)overlay, "v1_button", button);
  g_object_set_data((GObject*)overlay, "v1_navigation_btns", navigation_btns);
  
  gtk_widget_set_size_request((GtkWidget*)box, 100, height);
  gtk_widget_set_size_request((GtkWidget*)url, 80, height);
  gtk_widget_set_size_request((GtkWidget*)button, 20, height);
  gtk_widget_set_size_request((GtkWidget*)navigation_btns, 20, 100);
  gtk_widget_set_size_request((GtkWidget*)fixed, 100, height);
  
  gtk_fixed_put((GtkFixed*)fixed, (GtkWidget*)navigation_btns, 0, 0);
  
  gtk_fixed_put((GtkFixed*)fixed, (GtkWidget*)box, 0, 0);
  
 // g_object_set_data((GObject*)webContainer, "urlbar", url);
 // gtk_overlay_add_overlay(overlay, (GtkWidget*)webContainer);
  gtk_overlay_add_overlay(overlay, (GtkWidget*)fixed);
  gtk_overlay_set_overlay_pass_through(overlay, (GtkWidget*)fixed, TRUE);
  
  
  wnd_data->v1_entry = url;
  //wnd_data->v1_tab_container = webContainer;
  
  
 // gtk_container_add((GtkContainer*)wnd_data->m_wnd, (GtkWidget*)overlay);
  gtk_widget_set_visible((GtkWidget*)button, FALSE); 
  gtk_widget_set_visible((GtkWidget*)navigation_btns, FALSE); 
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
  
  g_signal_connect(tabs, "switch-page", switch_tab, &wnd_data);
  
  create_main_page(tabs, &wnd_data);
  
  mWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  wnd_data.m_wnd = (GtkWindow*) mWindow;
  
  wnd_data.r_click_time = 0;
    
  g_signal_connect((gpointer)mWindow, "delete-event", (GCallback)close_app, NULL);
  
  
  gtk_widget_show((GtkWidget*)new_tab_btn);
  
  gtk_window_set_default_size((GtkWindow*)mWindow, 800, 600);
  
  wnd_data.tab_container = tabs;
  

  gtk_container_add((GtkContainer*)mWindow, (GtkWidget*)tabs);
  
  
  g_signal_connect((GObject*)wnd_data.m_wnd, "configure-event", (GCallback)window_resize, &wnd_data);

  
  gtk_widget_show_all((GtkWidget*)mWindow);

  
  gtk_main();
}
