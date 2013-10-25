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
 *  Copyright (C) 2013 Alessio Piccoli <alepic@geckoblu.net>
 *                     Cedric Leporcq  <cedl38@gmail.com>
 *
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
    GtkEventBox     *eventbox;
    GtkImage        *image;
    gushort            size;
} WindowIcon;

typedef struct {
    gboolean only_maximized;           // [T/F] Only track maximized windows
    gboolean show_on_desktop;      // [T/F] Show the plugin on desktop
    //gboolean hide_icon;                // [T/F] Hide the icon
    gboolean hide_title;               // [T/F] Hide the title
    gboolean show_icon;                 // [T/F] Show the window icon
    gboolean icon_on_right;          // [T/F] Place icon on the right
    //gboolean show_window_menu;       // [T/F] Show window action menu on right click
    gboolean full_name;                // [T/F] Show full name
    gboolean show_tooltips;            // [T/F] Show tooltips

    SizeMode size_mode;               // Size mode : Length=[MINIMAL,FIXE,EXPAND]

    gint title_size;                   // Title size in chars
    gint title_padding;                 // Title padding

    gboolean custom_font;              // [T/F] Use custom font
    gchar *title_font;                  // Custom title font
    gchar *active_text_color;           // active text color
    gchar *inactive_text_color;         // inactive text color

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
    WindowIcon  *icon;              // Icon widget

    WCKPreferences     *prefs;
    WckUtils *win;

    gulong cnh;                         // controled window name handler id
    gulong cih;                         // controled window icon handler id

} WindowckPlugin;

void windowck_save(XfcePanelPlugin *plugin, WindowckPlugin *wckp);

G_END_DECLS

#endif /* !__WINDOWCK_H__ */
