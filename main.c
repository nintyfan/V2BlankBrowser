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
#if 0


static void load_handler(WebKitWebView  *web_view,
                         WebKitLoadEvent load_event,
                         gpointer        user_data)
{
  GtkLabel *tabLabel;
  WebKitWebResource *resource = webkit_web_view_get_main_resource(web_view);
  const char const *uri = NULL;
  int page_num;
  bool is_settings;
  
  if (resource ) {
    
    uri =webkit_web_resource_get_uri(resource); 
  }
  if (WEBKIT_LOAD_FINISHED == load_event) {
    
    struct connection *conn = get_connection_by_web_view(web_view);
    if (NULL == conn) return;
    const char *title = webkit_web_view_get_title(conn->wv);
    if (NULL != title && '\0' != title[0])
      gtk_label_set_text(conn->page_title, title);
    else if (0 == strcmp(conn->params[0], "/bin/false") && -1 != conn->pid) {
    
      char buffer2[PATH_MAX];
      char buffer[PATH_MAX];
      
      snprintf(buffer, PATH_MAX, "/proc/%d/exe", conn->pid);
      
      readlink(buffer, buffer2, PATH_MAX);
      
      gtk_label_set_text(conn->page_title, buffer2);
    }
    else 
      gtk_label_set_text(conn->page_title, conn->params[0]);
    ++conn->ref_count;
    webkit_web_view_run_javascript(conn->wv,  "function set_form_data(method, form) { form = JSON.parse(form); var els = document.querySelector('form[action=\"' + method + '\"]'); let rel = els; if (null == rel) return; for(let field of rel.elements) {if (field.name) {field.value = form[field.name];}}};", NULL, js_ready, conn);
    gtk_main();
    ++conn->ref_count;
    webkit_web_view_run_javascript(conn->wv,  "function get_form_data(method, prefix) { var els = document.querySelector('form[action=\"' + method + '\"]'); let rel = els; if (null == rel) return; var result=prefix + '?'; for(let field of rel.elements) {if (field.name) {if (field.type == 'checkbox') {  if (field.checked) result += field.name + '=1' + '\\u0026'; } else result += field.name + '=' + field.value + '\\u0026' ;}}  if (result.substr(result.length - 1, result.length) == '\\u0026') result = result.substr(0, result.length - 1);  window.location.assign(result); };", NULL, js_ready, conn);
    gtk_main();
    
    struct connection_message *msg = conn->msg;
    struct connection_message **prev = &conn->msg;
    while (NULL != msg) {
       
      const char *method = dbus_message_get_member(msg->msg);
      const char *interface = dbus_message_get_interface(msg->msg);
      ((struct connection*) user_data)->flags &= ~BONSOLE_FLAG_LOCKED;
      *prev = msg->next;
      real_handle_dbus_msg(msg->connection, msg->msg, msg->user_data);
      
      dbus_message_unref(msg->msg);
      free(msg);
      msg = *prev;
      if ((((struct connection*) user_data)->flags & BONSOLE_FLAG_LOCKED) == BONSOLE_FLAG_LOCKED) return;
    }
    
    conn->msg = NULL;
    if (-1 == conn->pid) free_connection(conn);
    
    ((struct connection*) user_data)->flags &= ~BONSOLE_FLAG_LOCKED;
  }
}
static void toggle_term_visibility(GtkWidget *self, GtkWidget *data2)
{
  data2 = gtk_widget_get_parent(data2);
  gtk_widget_set_no_show_all(data2, FALSE);
  if (gtk_widget_get_visible(data2)) {
  
    
    //gtk_widget_hide(data2);
    gtk_widget_hide(data2);
  }
  else {
    
    //gtk_widget_show_all(data2);
     gtk_widget_show_all(data2);
  }
}

static void send_input(GtkEntry *entry, gpointer  user_data)
{
  const char *text;
  struct connection *conn = (struct connection*) user_data;
  
  text = gtk_entry_get_text(entry);
  
  write(conn->masterFd, text, strlen(text));
  write(conn->masterFd, "\n", 1);
}

static struct connection *command_init(char **params, pid_t regenerate)
{
  struct termios termios_p;
  int slaveFd;
  
  struct connection *conn = (struct connection*) malloc(sizeof(*conn));
  conn->msg = NULL;
  conn->ref_count = 0;
  conn->dbus_name = NULL;
  
  conn->next = programs_conn;
  
  bool ok = true;
  int fdm = posix_openpt(O_RDWR | O_NOCTTY);
  if (fdm < 0)
  {
    
    ok  = 0;
  }
  int rc;
  if (ok) {
    rc = grantpt(fdm);
    if (rc != 0)
    {
      
      ok = 0;
    }
  }
  
  rc = unlockpt(fdm);
  
  
  if ((tcgetattr (fdm, &termios_p)) < 0) {
    
    ok = 0;
  }
  else {
    memset(&termios_p,0,sizeof(termios_p));  
    termios_p.c_oflag |= ONLCR;
    termios_p.c_lflag &= ~ECHO;
    termios_p.c_cc[VMIN]  = 1;
    tcsetattr(fdm, TCSANOW, &termios_p);
  }
  
  ok = ((NULL != ptsname(fdm)) && ok);
  if (!ok) {
    
    return NULL;
  }
  pid_t pid;
  if (-1 == regenerate)
  pid = bonsole_spawn_process(params, fdm, conn);
  else
    pid = regenerate;
  int mesh2wr2 = 0;
  write(conn->comm_ch, &mesh2wr2, sizeof(mesh2wr2));
  // TODO: Skorzystać z tego
  //g_unix_fd_add (fd, flags, gio_dautom_event_handler, NULL);
  // Start the process
  conn->flags = 0;
  if (-1 == regenerate)
    g_unix_fd_add (fdm, G_IO_IN , terminal_input_handler, conn);
  // SEND KEY
  conn->pid = pid;
  conn->params = params;
  
  return conn;
  
}

static GtkWidget *generate_UI_fo_command(struct connection *conn)
{
  
  GtkWidget *btnBox;
  GtkWidget *termBtn;
  GtkWidget *termContainer;
  GtkWidget *box;
  GtkWidget *status_label;
  
  GtkWidget *web_view = webkit_web_view_new();
  conn->wv = web_view;
  
  webkit_settings_set_enable_developer_extras(webkit_web_view_get_settings(web_view), true);
  
  
  
  gtk_widget_set_vexpand(GTK_WIDGET(web_view), true);
  
  
  
  conn->term = gtk_text_view_new();
  
  gtk_text_view_set_editable(conn->term, FALSE);
  g_signal_connect(web_view, "load-changed", load_handler, conn);
  //free(params);
  
  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  
  status_label = gtk_label_new("Running...");
  conn->status_label = status_label;
  
  btnBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  termBtn = gtk_button_new_with_label("Terminal");
  
  termContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  
  GtkWidget *command_prompt = gtk_entry_new();
  
  gtk_box_pack_start(termContainer, conn->term, TRUE, TRUE, 0);
  gtk_box_pack_start(termContainer, command_prompt, FALSE, FALSE, 0);
  gtk_widget_set_no_show_all(termContainer, TRUE);
  
  g_signal_connect(command_prompt, "activate", send_input, conn);
  
  gtk_label_set_angle(gtk_bin_get_child(termBtn), 90);
  
  gtk_box_pack_start(btnBox, termBtn, FALSE, FALSE, 0);
  gtk_box_pack_start(box, box2, TRUE, TRUE, 0);
  gtk_box_pack_start(box, conn->status_label, FALSE, FALSE, 0);
  
  gtk_box_pack_start(box2, conn->wv, TRUE, TRUE, 0);
  gtk_box_pack_start(box2, btnBox, FALSE, FALSE, 0);
  gtk_box_pack_start(box2, termContainer, FALSE, FALSE, 0);
  gtk_widget_set_size_request(conn->term, 100, 100);
  
  
  
  g_object_set_data(box, "connection_str", conn);
  
  g_signal_connect(termBtn, "clicked", toggle_term_visibility, conn->term);
  programs_conn = conn;
  webkit_web_view_load_html(web_view, "<html><head><script type=\"text/javascript\">setTimeout(function () {var el = document.querySelector('body'); el.innerHTML = '<h1>It looks like this app not display page until specified amount of time</h1><dl><dt>Maybe it\\\'s classical console application</dt><dd>In this case, open terminal by clicking on terminal button on the right side of this window</dd><dt>App will start in some time</dt><dd>In this case you must wait</dd></dl>';}, 10000);</script></head><body></body></html>", "about:first_loading");
  gtk_widget_show_all(web_view);
  return box;
}

#endif

static gboolean show_button( GtkWidget *widget, GdkEventMotion *event ) {

  GtkWidget *parent = gtk_widget_get_parent(widget);
  
  GtkWidget *button = g_object_get_data(parent, "button");
  
  gtk_widget_set_visible(button, TRUE); 
  
  return TRUE;
}


static gboolean hide_button( GtkWidget *widget, GdkEventMotion *event ) {
  
  
  return TRUE;
}

static void go_to_addr(GtkEntry *entry, gpointer user_data)
{
  WebKitWebView *view = (WebKitWebView*) user_data;
  
  webkit_web_view_load_uri(view, gtk_entry_get_text(entry));
}

int main(int argc, char **argv)
{
  
  GtkWidget *mWindow, *button;
  GtkFixed *box;
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
  
  //g_signal_connect(mWindow, "delete-event", ask_for_close, &pcd);
  //g_signal_connect(mWindow, "destroy", exit_app, NULL);
  
  gtk_window_set_default_size(mWindow, 800, 600);
  
  overlay = gtk_overlay_new();
  
  webContent = webkit_web_view_new();
  webkit_web_view_load_uri(webContent, "http://dobreprogramy.pl");

  box = gtk_fixed_new();
  url = gtk_entry_new();
  
  GtkStyleContext *background_ctx = gtk_widget_get_style_context(url);
  
 
  button = gtk_button_new_with_label("^");

  gtk_widget_add_events(url, GDK_POINTER_MOTION_MASK);
  //gtk_widget_add_events(box, GDK_POINTER_MOTION_MASK);
 // g_signal_connect (url, "draw",G_CALLBACK(draw_url), NULL);
  g_signal_connect (url, "motion_notify_event",G_CALLBACK(show_button), button);
  //g_signal_connect (box, "motion_notify_event",G_CALLBACK(hide_button), button);
  
  prov = gtk_css_provider_new();
  gtk_css_provider_load_from_data(prov, "* {background-color: rgba(65,10,20, 0.8);}", -1, NULL);
  
  gtk_style_context_add_provider(gtk_widget_get_style_context(url), prov, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  gtk_style_context_get_property(gtk_widget_get_style_context(url), "font-size", GTK_STATE_FLAG_NORMAL, &value);
  
  g_value_init(&value2, G_TYPE_UINT);
  
  if (!g_value_transform(&value, &value2)) {
  
    exit(1);
  }
  
  int height = g_value_get_int(&value2);
  
  fixed = gtk_fixed_new();
  //gtk_fixed_put(fixed, box, 10, 10);
  //gtk_fixed_put(fixed, button, 10, 10);
  
  gtk_fixed_put(box, url, 0, 0);
  gtk_fixed_put(box, button, 100, 0);
  
  g_object_set_data(box, "button", button);
  //gtk_fixed_put(fixed, url, 10, 10);
  gtk_widget_set_size_request(box, 100, height);
  gtk_widget_set_size_request(url, 80, height);
  gtk_widget_set_size_request(button, 20, height);
  
  gtk_overlay_add_overlay(overlay, webContent);
  gtk_overlay_add_overlay(overlay, box);
  gtk_overlay_set_overlay_pass_through(overlay, box, TRUE);
  
  gtk_entry_set_text(url, "http://www.dobreprogramy.pl");
  
  gtk_container_add(mWindow, overlay);
  
  gtk_widget_show_all(mWindow);
  gtk_widget_set_visible(button, FALSE);  

  g_signal_connect (url, "activate", G_CALLBACK (go_to_addr), webContent);
  
  gtk_main();
}
