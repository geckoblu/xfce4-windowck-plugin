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
 *  This code is derived from original 'Window Applets' from Andrej Belcijan.
 */

#ifndef WCK_UTILS_H_
#define WCK_UTILS_H_

#ifndef WNCK_I_KNOW_THIS_IS_UNSTABLE
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#endif
#include <libwnck/libwnck.h>
#include <libxfce4panel/xfce-panel-plugin.h>

G_BEGIN_DECLS

/* Wnck structure */
typedef struct {
    WnckScreen *activescreen;          // Active screen
    WnckWorkspace *activeworkspace;    // Active workspace
    WnckWindow *controlwindow;          // Controled window according to only_maximized option
    WnckWindow *activewindow;          // Active window
    WnckWindow *umaxwindow;            // Upper maximized window

    gulong msh;                         // upper maximized window state handler id
    gulong ash;                         // active state handler id
    gulong mwh;                         // upper maximized workspace handler id
    gulong sch;                         // window closed handler id
    gulong soh;                         // window opened handler id
    gulong svh;                         // viewport changed handler id
    gulong swh;                         // workspace changed handler id

    gboolean only_maximized;           // [T/F] Only track maximized windows

    gpointer data;
} WckUtils;

void init_wnck (WckUtils *win, gboolean only_maximized, gpointer data);
void on_wck_state_changed (WnckWindow *controlwindow, gpointer data);
void on_control_window_changed(WnckWindow *controlwindow, WnckWindow *previous, gpointer data);
void reload_wnck (WckUtils *win, gboolean only_maximized, gpointer data);
void toggle_maximize (WnckWindow *window);
gboolean wck_signal_handler_disconnect (GObject *object, gulong handler);

G_END_DECLS

#endif /* WCK_UTILS_H_ */
