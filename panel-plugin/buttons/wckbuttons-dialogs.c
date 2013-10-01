/*  $Id$
 *
 *  Copyright (C) 2012 John Doo <john@foo.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "wckbuttons.h"
#include "wckbuttons-dialogs.h"

/* the website url */
#define PLUGIN_WEBSITE "http://goodies.xfce.org/projects/panel-plugins/xfce4-wckbuttons-plugin"


static void
wckbuttons_configure_response (GtkWidget    *dialog,
                           gint          response,
                           WBPlugin *wb)
{
  gboolean result;

  if (response == GTK_RESPONSE_HELP)
    {
      /* show help */
      result = g_spawn_command_line_async ("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

      if (G_UNLIKELY (result == FALSE))
        g_warning (_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
    }
  else
    {
      /* remove the dialog data from the plugin */
      g_object_set_data (G_OBJECT (wb->plugin), "dialog", NULL);

      /* unlock the panel menu */
      xfce_panel_plugin_unblock_menu (wb->plugin);

      /* save the plugin */
      wckbuttons_save (wb->plugin, wb);

      /* destroy the properties dialog */
      gtk_widget_destroy (dialog);
    }
}



void
wckbuttons_configure (XfcePanelPlugin *plugin,
                  WBPlugin    *wb)
{
  GtkWidget *dialog;

  /* block the plugin menu */
  xfce_panel_plugin_block_menu (plugin);

  /* create the dialog */
  dialog = xfce_titled_dialog_new_with_buttons (_("Windowck Buttons"),
                                                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
                                                GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                                                GTK_STOCK_HELP, GTK_RESPONSE_HELP,
                                                GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                                                NULL);

  /* center dialog on the screen */
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

  /* set dialog icon */
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-settings");

  /* link the dialog to the plugin, so we can destroy it when the plugin
   * is closed, but the dialog is still open */
  g_object_set_data (G_OBJECT (plugin), "dialog", dialog);

  /* connect the reponse signal to the dialog */
  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK(wckbuttons_configure_response), wb);

  /* show the entire dialog */
  gtk_widget_show (dialog);
}



void
wckbuttons_about (XfcePanelPlugin *plugin)
{
  /* about dialog code. you can use the GtkAboutDialog
   * or the XfceAboutInfo widget */
    GdkPixbuf *icon;
    const gchar *auth[] = { "Alessio Piccoli <alepic@geckoblu.net>", "Cedric Leporcq <cedl38@gmail.com>", "\nThis code is based on original 'Window Applets' code of Andrej Belcijan.\nSee http://gnome-look.org/content/show.php?content=103732 for details." };

    icon = xfce_panel_pixbuf_from_source("wckbuttons-plugin", NULL, 32);

    gtk_show_about_dialog(NULL, "logo", icon, "license", xfce_get_license_text(XFCE_LICENSE_TEXT_GPL), "version", PACKAGE_VERSION, "program-name", PACKAGE_NAME, "comments", _("Put the maximized window buttons on the panel."), "website", PLUGIN_WEBSITE, "copyright", _("Copyright (c) 2013\n"), "authors", auth, NULL );
    // TODO: add translators.

    if (icon)
        g_object_unref(G_OBJECT(icon) );
}
