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

#ifndef __WCKBUTTONS_H__
#define __WCKBUTTONS_H__

#include <common/wck-utils.h>
#include <xfconf/xfconf.h>

G_BEGIN_DECLS

/* indexing of buttons */
typedef enum {
    MINIMIZE_BUTTON = 0,    // minimize button
    MAXIMIZE_BUTTON,    // maximize/unmaximize button
    CLOSE_BUTTON,       // close button

    BUTTONS             // number of buttons
} WindowButtonIndices;

typedef enum {
    BUTTON_STATE_NORMAL =  0,
    BUTTON_STATE_PRELIGHT,
    BUTTON_STATE_PRESSED,

  BUTTON_STATES
} WBButtonSt;

/* we will index images for convenience */
typedef enum {
    IMAGE_MINIMIZE = 0,
    IMAGE_UNMAXIMIZE,
    IMAGE_MAXIMIZE,
    IMAGE_CLOSE,

    IMAGES_BUTTONS
} ImageStates;

/* we will also index image states for convenience */
typedef enum {
    IMAGE_UNFOCUSED = 0,
    IMAGE_FOCUSED,
    IMAGE_PRELIGHT,
    IMAGE_PRESSED,

    IMAGES_STATES
} WBImageIndices;

typedef struct {
    gboolean only_maximized;                // [T/F] Only track maximized windows
    gboolean show_on_desktop;               // [T/F] Show the plugin on desktop
    gchar       *theme;                     // Selected theme path
    gchar      *button_layout;              // Button layout ["XXX"] (example "HMC" : H=Hide, M=Maximize/unMaximize, C=Close)
    gboolean sync_wm_theme;       // [T/F] Try to use xfwm4 active theme if possible.
} WCKPreferences;

/* Definition for our button */
typedef struct {
    GtkEventBox     *eventbox;
    GtkImage        *image;
    gboolean        visible;            // Indicates whether the button is visible
} WindowButton;

/* plugin structure for title and buttons*/
typedef struct {
    XfcePanelPlugin *plugin;

    /* Widgets */
    GtkWidget *ebox;
    GtkWidget *hvbox;

    WindowButton  **button;         // Array of buttons

    WCKPreferences *prefs;          // Main properties
    WckUtils *win;

    gint        setting2;

    GdkPixbuf *pixbufs[IMAGES_STATES][IMAGES_BUTTONS];
    XfconfChannel *wm_channel;      // window manager chanel
} WBPlugin;

void wckbuttons_save (XfcePanelPlugin *plugin, WBPlugin *wb);
void on_wck_state_changed (WnckWindow *controlwindow, WBPlugin *wckp);
void on_control_window_changed(WnckWindow *controlwindow, WnckWindow *previous, WBPlugin *wckp);

G_END_DECLS

#endif /* !__WCKBUTTONS_H__ */
