/*
 *  Copyright (C) 2013 Alessio Piccoli <alepic@geckoblu.net>
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
 */

/*
 * This code is heavily based on original 'Window Applets' code of Andrej Belcijan.
 * See http://gnome-look.org/content/show.php?content=103732 for details.
 */

#ifndef __WINDOWCKTITLE_H__
#define __WINDOWCKTITLE_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#ifndef WNCK_I_KNOW_THIS_IS_UNSTABLE
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#endif
#include <libwnck/libwnck.h>

G_BEGIN_DECLS

typedef enum SizeMode
{
    SHRINK = 1, FIXE = 2, EXPAND = 3
} SizeMode;

typedef enum Alignment
{
    LEFT = 0, CENTER = 5, RIGHT = 10
} Alignment;

typedef struct {
    gboolean only_maximized;           // [T/F] Only track maximized windows
    gboolean hide_on_unmaximized;      // [T/F] Hide when no maximized windows present
    //gboolean hide_icon;                // [T/F] Hide the icon
    gboolean hide_title;               // [T/F] Hide the title
    //gboolean swap_title_icon;          // [T/F] Swap title/icon (TRUE icon on the right
    //gboolean expand_applet;          // [T/F] Expand the applet TODO: rename to expand_title ?
    //gboolean custom_style;           // [T/F] Use custom style
    //gboolean show_window_menu;       // [T/F] Show window action menu on right click
    gboolean show_tooltips;            // [T/F] Show tooltips

    SizeMode size_mode;               // Size mode : Length=[MINIMAL,FIXE,EXPAND]

    gint title_size;                   // Title size in chars
    gint title_padding;                 // Title padding

    //gchar *title_active_font;        // Custom active title font
    //gchar *title_active_color;       // Custom active title color
    //gchar *title_inactive_font;      // Custom inactive title font
    //gchar *title_inactive_color;     // Custom inactive title color

    gint title_alignment;           // Title alignment [0=left, 5=center, 10=right]

    gint width;                     // Title size in pixels
    gint  title_size_max;           // Title size max in chars

} WCKPreferences;

/* plugin structure */
typedef struct {
    XfcePanelPlugin *plugin;

    /* Widgets */
    GtkWidget *ebox;
    GtkWidget *hvbox;
    GtkLabel *title;

    /* Variables */
    WCKPreferences *prefs;              // Main properties

    WnckScreen *activescreen;          // Active screen
    WnckWorkspace *activeworkspace;    // Active workspace
    WnckWindow *umaxedwindow;          // Upper-most maximized window
    WnckWindow *activewindow;          // Active window
    WnckWindow *rootwindow;            // Root window (desktop)

    gulong active_handler_state;       // activewindow's statechange event handler ID
    gulong active_handler_name;        // activewindow's namechange event handler ID
    gulong active_handler_icon;        // activewindow's iconchange event handler ID
    gulong umaxed_handler_state;       // umaxedwindow's statechange event handler ID
    gulong umaxed_handler_icon;        // umaxedwindow's iconchange event handler ID
    gulong umaxed_handler_name;        // umaxedwindow's manechange event handler ID

    gboolean focused;                  // [T/F] Window state (focused or unfocused)
    gint  width;                       // with of the plugin
} WindowckPlugin;

void windowck_save(XfcePanelPlugin *plugin, WindowckPlugin *wckp);
void resize_title(WindowckPlugin *wckp);
void expand_title(WindowckPlugin *wckp);

G_END_DECLS

#endif /* !__WINDOWCKTITLE_H__ */
