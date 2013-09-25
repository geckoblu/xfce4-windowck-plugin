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

#ifndef __WINDOWCK_H__
#define __WINDOWCK_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <common/wck-utils.h>

G_BEGIN_DECLS

typedef enum Alignment
{
    LEFT = 0, CENTER = 5, RIGHT = 10
} Alignment;

typedef enum SizeMode
{
    SHRINK = 1, FIXE = 2, EXPAND = 3
} SizeMode;

typedef struct {
    gboolean only_maximized;           // [T/F] Only track maximized windows
    gboolean show_on_desktop;      // [T/F] Show the plugin on desktop
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

    gboolean custom_font;              // [T/F] Use custom font
    gchar *title_font;                  // Custom title font
    //gchar *title_active_color;       // Custom active title color
    //gchar *title_inactive_color;     // Custom inactive title color

    gint title_alignment;            // Title alignment [0=left, 5=center, 10=right]
} WCKPreferences;

/* plugin structure */
typedef struct {
    XfcePanelPlugin *plugin;

    /* Widgets */
    GtkWidget *ebox;
    GtkWidget *hvbox;
    GtkWidget *alignment;
    GtkLabel *title;

    /* Variables */
    WCKPreferences     *prefs;
    WckUtils *win;

    gulong cnh;                         // controled window name handler id
    gulong cih;                         // controled window icon handler id

} WindowckPlugin;

void windowck_save(XfcePanelPlugin *plugin, WindowckPlugin *wckp);

G_END_DECLS

#endif /* !__WINDOWCK_H__ */
