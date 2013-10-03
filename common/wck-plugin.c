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

void
wck_about (XfcePanelPlugin *plugin, const gchar *icon_name)
{
    /* about dialog code. you can use the GtkAboutDialog
    * or the XfceAboutInfo widget */
    GdkPixbuf *icon;

    const gchar *auth[] = {
        "Alessio Piccoli <alepic@geckoblu.net>",
        "Cedric Leporcq <cedl38@gmail.com>",
        "\nThis code is based on original 'Window Applets' from Andrej Belcijan.\nSee http://gnome-look.org/content/show.php?content=103732 for details.",
        NULL };

    icon = xfce_panel_pixbuf_from_source(icon_name, NULL, 32);

    gtk_show_about_dialog (NULL,
            "logo", icon,
            "license", xfce_get_license_text(XFCE_LICENSE_TEXT_GPL),
            "version", PACKAGE_VERSION,
            "program-name", xfce_panel_plugin_get_display_name (plugin),
            "comments", xfce_panel_plugin_get_comment (plugin),
            "website", PLUGIN_WEBSITE,
            "copyright", _("Copyright \302\251 2013\n"),
            "authors", auth,
            NULL );
    // TODO: add translators.

    if (icon)
        g_object_unref(G_OBJECT(icon) );
}
