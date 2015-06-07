/*  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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
 *
 *  Copyright (C) 2013 Cedric Leporcq  <cedl38@gmail.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>
#include <libxfce4ui/libxfce4ui.h>

#include "wck-plugin.h"

XfconfChannel *
wck_properties_get_channel (GObject *object_for_weak_ref, const gchar *channel_name)
{
  GError        *error = NULL;
  XfconfChannel *channel;

  g_return_val_if_fail (G_IS_OBJECT (object_for_weak_ref), NULL);

  if (!xfconf_init (&error))
    {
      g_critical ("Failed to initialize Xfconf: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  //~ channel = xfconf_channel_get (XFCE_PANEL_CHANNEL_NAME);
  channel = xfconf_channel_get (channel_name);
  g_object_weak_ref (object_for_weak_ref, (GWeakNotify) xfconf_shutdown, NULL);

  return channel;
}


void
wck_about (XfcePanelPlugin *plugin, const gchar *icon_name)
{
    /* about dialog code. you can use the GtkAboutDialog
    * or the XfceAboutInfo widget */
    GdkPixbuf *icon;

    const gchar *auth[] =
    {
        "Alessio Piccoli <alepic@geckoblu.net>",
        "Cedric Leporcq <cedl38@gmail.com>",
        "Felix Krull <f_krull@gmx.de>",
        "Pavel Zlámal <zlamal@cesnet.cz>",
        "",
        "This code is derived from 'Window Applets' from Andrej Belcijan.",
        "See http://gnome-look.org/content/show.php?content=103732 for details.",
        NULL
    };

    icon = xfce_panel_pixbuf_from_source(icon_name, NULL, 32);

    gtk_show_about_dialog (NULL,
            "logo", icon,
            "license", xfce_get_license_text(XFCE_LICENSE_TEXT_GPL),
            "version", PACKAGE_VERSION,
            "program-name", xfce_panel_plugin_get_display_name (plugin),
            "comments", xfce_panel_plugin_get_comment (plugin),
            "website", PLUGIN_WEBSITE,
            "copyright", "Copyright \302\251 2013-2015\n",
            "authors", auth,
            NULL );
    // TODO: add translators.

    if (icon)
        g_object_unref(G_OBJECT(icon) );
}


GtkWidget *show_refresh_item (XfcePanelPlugin *plugin)
{
    GtkWidget *refresh;
    refresh = gtk_image_menu_item_new_from_stock  (GTK_STOCK_REFRESH, NULL);
    xfce_panel_plugin_menu_insert_item(plugin, GTK_MENU_ITEM(refresh));
    gtk_widget_show(GTK_WIDGET(refresh));

    return refresh;
}
