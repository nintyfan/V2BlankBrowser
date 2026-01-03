/*
 *    V2BlankBrowser - browser with respect your monitor
 *    Copyright (C) 2021-2026  Lach Sławomir <nintyfan19@gmail.com>
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
 * //  *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <glib-2.0/glib.h>

#include <stddef.h>

static gboolean download_starting(WebKitDownload *download,
                                  gchar          *suggested_filename,
                                  gpointer        user_data)
{
  char *buffer;
  //struct download_information *fi = (struct download_information*) user_data;
  const char *name = webkit_uri_request_get_uri(webkit_download_get_request(download));

  
  GtkWidget *download_localtionD = gtk_file_chooser_dialog_new ("Select destination directory",
                                                                NULL,
                                                                GTK_FILE_CHOOSER_ACTION_SAVE,
                                                                "Select destination",
                                                                GTK_RESPONSE_ACCEPT,
                                                                "Cancel", GTK_RESPONSE_CANCEL, NULL);
  
  gtk_widget_show_all(download_localtionD);
  gint result =  gtk_dialog_run((GtkDialog*) download_localtionD);
  
  const char *location = NULL;
  
  if (GTK_RESPONSE_ACCEPT == result) {
    
    location = gtk_file_chooser_get_filename((GtkFileChooser*)download_localtionD);
    
    gtk_widget_destroy(download_localtionD);
  }
  else {
  
    gtk_widget_destroy(download_localtionD);
    webkit_download_cancel(download);
    return FALSE;
  }
  
  if (NULL == location) {
  
    webkit_download_cancel(download);
    return FALSE;
  }
  
  int length = snprintf(NULL, 0, "file:///%s", location) + 1;
  buffer = malloc(length);
  
  snprintf(buffer, length, "file:///%s", location);
  
  puts(buffer);
  
  webkit_download_set_destination(download, buffer);
  
  free(buffer);
  
  return TRUE;
}

static void download_started(WebKitWebContext *context,
                             WebKitDownload   *download,
                             gpointer          user_data)
{
  (void) user_data;
  
  g_signal_connect(download, "decide-destination", (GCallback) download_starting, NULL);
}


void download_manager_init()
{
  g_signal_connect(webkit_web_context_get_default (), "download-started", (GCallback) download_started, NULL);
}
