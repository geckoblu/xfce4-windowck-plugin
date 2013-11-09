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

#ifndef WCK_PLUGIN_H_
#define WCK_PLUGIN_H_

#include <xfconf/xfconf.h>
#include <libxfce4panel/xfce-panel-plugin.h>

/* the website url */
#define PLUGIN_WEBSITE "http://goodies.xfce.org/projects/panel-plugins/xfce4-windowck-plugin"

G_BEGIN_DECLS

XfconfChannel *
wck_properties_get_channel (GObject *object_for_weak_ref, const gchar *channel_name);
void wck_about (XfcePanelPlugin *plugin, const gchar *icon_name);
GtkWidget *show_refresh_item (XfcePanelPlugin *plugin);

G_END_DECLS

#endif /* WCK_PLUGIN_H_ */
