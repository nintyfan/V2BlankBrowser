/*
 *    Bonsole - display output of bonsole client library in graphical form
 *    Copyright (C) 2020  Lach Sławomir <slawek@lach.art.pl>
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
 *    
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

static gboolean show_button( GtkWidget *widget, GdkEventMotion *event ) {

  int set_to_y = 0;
  GtkWidget *parent = gtk_widget_get_parent(widget);
  GtkWidget *overlay = gtk_widget_get_parent(gtk_widget_get_parent(parent));
  
  if (g_object_get_data(overlay, "locked")) {
  
    return TRUE;
  }
  g_object_set_data(overlay, "locked", TRUE);
  
  GtkWidget *button = g_object_get_data(parent, "button");
  GtkBox *box = g_object_get_data(parent, "navigation_btns");
  
  GtkAllocation *allocation = g_new(GtkAllocation, 1);
  gtk_widget_get_allocation(widget, allocation);
  
  if (allocation->y != 0) {
  
    set_to_y = allocation->y - 100 + allocation->height;
  }
  gtk_fixed_move(gtk_widget_get_parent(parent), box, allocation->width - 10, set_to_y);
  
  g_free(allocation);
  
  gtk_widget_set_visible(button, TRUE); 
  gtk_widget_set_visible(box, TRUE); 
  
  gtk_fixed_move(parent, button, event->x, event->y);
  
  return TRUE;
}


static gboolean hide_button( GtkWidget *widget, GdkEventMotion *event ) {
  
  GtkWidget *button = g_object_get_data(widget, "button");
  GtkWidget *box = g_object_get_data(widget, "navigation_btns");
  g_object_set_data(widget, "locked", NULL);
  
  gtk_widget_set_visible(box, FALSE); 
  gtk_widget_set_visible(button, FALSE); 
  return TRUE;
}

static void go_to_addr(GtkEntry *entry, gpointer user_data)
{
  WebKitWebView *view = (WebKitWebView*) user_data;
  
  webkit_web_view_load_uri(view, gtk_entry_get_text(entry));
}




static void back_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  webkit_web_view_go_back((WebKitWebView *) user_data);
}

static void forward_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  webkit_web_view_go_forward((WebKitWebView *) user_data);
}

static void teleport_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  GtkButton *button;
  GtkBox *fixed1 = (GtkBox*) user_data;
  GtkBox *fixed2 = gtk_widget_get_parent(fixed1); 
  GtkWidget *box = g_object_get_data(fixed1, "navigation_btns");
  const char *data = g_object_get_data(widget, "position");
  if (data && 0 == strcmp(data, "down")) {
  
     gtk_button_set_label(widget, "↓");
     g_object_set_data(widget, "position", "up");
     
     gtk_fixed_move(fixed2, fixed1, 0, 0);
     gtk_fixed_move(fixed2, box, 0, 0);
  }
  else {
    
    int h;
    GtkAllocation* allocation = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(fixed2, allocation);
    
    h = allocation->height;
    
    gtk_widget_get_allocation(widget, allocation);
    
    gtk_button_set_label(widget, "↑"); 
    g_object_set_data(widget, "position", "down");
    
    h = h - allocation->height;
    
    gtk_fixed_move(fixed2, fixed1, 0, h);
    
    gtk_widget_get_allocation(fixed2, allocation);
    
    h = allocation->height;
    gtk_widget_get_allocation(box, allocation);
    h = h - allocation->height;
    
    gtk_fixed_move(fixed2, box, 0, h);
    g_free(allocation);
  }
  
  box = g_object_get_data(fixed1, "navigation_btns");
  button = g_object_get_data(fixed1, "button");
  gtk_widget_set_visible(button, FALSE);  
  gtk_widget_set_visible(box, FALSE);  
  g_object_set_data(gtk_widget_get_parent(fixed1), "locked", NULL);
}

gboolean close_app(GtkWidget *widget,GdkEvent *event, gpointer user_data)
{
  exit(0);
}

int main(int argc, char **argv)
{
  
  GtkWidget *mWindow, *button, *back, *forward, *home;
  GtkFixed *box;
  GtkBox *navigation_btns;
  GtkFixed *fixed;
  GtkOverlay *overlay;
  WebKitWebView *webContent;
  GtkEntry *url;
  GtkCssProvider *prov;
  GValue value = G_VALUE_INIT;
  GValue value2 = G_VALUE_INIT;
  
  gtk_init(&argc, &argv);
  
  //webkit_web_context_set_web_extensions_directory (webkit_web_context_get_default (), 
  //                                                 INSTALL_PREFIX "lib/extensions");
  mWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  g_signal_connect(mWindow, "delete-event", close_app, NULL);
  
  gtk_window_set_default_size(mWindow, 800, 600);
  
  overlay = gtk_overlay_new();
  
  webContent = webkit_web_view_new();
  webkit_web_view_load_uri(webContent, "http://dobreprogramy.pl");

  fixed = gtk_fixed_new();
  box = gtk_fixed_new();
  url = gtk_entry_new();
  
  GtkStyleContext *background_ctx = gtk_widget_get_style_context(url);
  
 
  button = gtk_button_new_with_label("↓");
  back = gtk_button_new_with_label("⇦");
  forward = gtk_button_new_with_label("⇨");
  home = gtk_button_new_with_label("⌂");
  
  g_signal_connect(back, "button-press-event", G_CALLBACK(back_clicked), webContent);
  g_signal_connect(forward, "button-press-event", G_CALLBACK(forward_clicked), webContent);
  
  navigation_btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);// gtk_fixed_new();
  
  gtk_box_pack_start(navigation_btns, back, TRUE, FALSE, 0);
  gtk_box_pack_start(navigation_btns, home, TRUE, FALSE, 0);
  gtk_box_pack_start(navigation_btns, forward, TRUE, FALSE, 0);
  
  
  
  gtk_widget_add_events(url, GDK_POINTER_MOTION_MASK);
  
  g_signal_connect(button, "button-press-event", G_CALLBACK(teleport_clicked), box);
  //gtk_widget_add_events(box, GDK_POINTER_MOTION_MASK);
 // g_signal_connect (url, "draw",G_CALLBACK(draw_url), NULL);
  g_signal_connect (url, "motion_notify_event",G_CALLBACK(show_button), NULL);
  g_signal_connect (overlay, "motion_notify_event",G_CALLBACK(hide_button), NULL);
  
  prov = gtk_css_provider_new();
  gtk_css_provider_load_from_data(prov, "* {background-color: rgba(65,10,20, 0.8);}", -1, NULL);
  
  gtk_style_context_add_provider(gtk_widget_get_style_context(url), prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  gtk_style_context_get_property(gtk_widget_get_style_context(url), "font-size", GTK_STATE_FLAG_NORMAL, &value);
  
  g_value_init(&value2, G_TYPE_UINT);
  
  if (!g_value_transform(&value, &value2)) {
  
    exit(1);
  }
  
  int height = g_value_get_uint(&value2);
  

  //gtk_fixed_put(fixed, box, 10, 10);
  //gtk_fixed_put(fixed, button, 10, 10);
  
  gtk_fixed_put(box, url, 0, 0);
  gtk_fixed_put(box, button, 100, 0);
  
  
  gtk_fixed_put(navigation_btns, forward, 0, 0);
  gtk_fixed_put(navigation_btns, home, 0, 20);
  gtk_fixed_put(navigation_btns, back, 0, 40);
  
  g_object_set_data(box, "navigation_btns", navigation_btns);
  g_object_set_data(box, "button", button);
  g_object_set_data(overlay, "button", button);
  g_object_set_data(overlay, "navigation_btns", navigation_btns);
  //gtk_fixed_put(fixed, url, 10, 10);
  gtk_widget_set_size_request(box, 100, height);
  gtk_widget_set_size_request(url, 80, height);
  gtk_widget_set_size_request(button, 20, height);
  gtk_widget_set_size_request(navigation_btns, 20, 100);
  gtk_widget_set_size_request(fixed, 100, height);
  
  gtk_fixed_put(fixed, navigation_btns, 0, 0);
  
  gtk_fixed_put(fixed, box, 0, 0);
  gtk_overlay_add_overlay(overlay, webContent);
  gtk_overlay_add_overlay(overlay, fixed);
  gtk_overlay_set_overlay_pass_through(overlay, fixed, TRUE);
  
  gtk_entry_set_text(url, "http://www.dobreprogramy.pl");
  
  gtk_container_add(mWindow, overlay);
  
  gtk_widget_show_all(mWindow);
  gtk_widget_set_visible(button, FALSE); 
  gtk_widget_set_visible(navigation_btns, FALSE); 

  g_signal_connect (url, "activate", G_CALLBACK (go_to_addr), webContent);
  
  gtk_main();
}
