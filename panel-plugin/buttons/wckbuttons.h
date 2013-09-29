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

#ifndef __WCKBUTTONS_H__
#define __WCKBUTTONS_H__

G_BEGIN_DECLS

/* indexing of buttons */
typedef enum {
	MINIMIZE_BUTTON = 0,	// minimize button
	MAXIMIZE_BUTTON,	// maximize/unmaximize button
	CLOSE_BUTTON,		// close button

	BUTTONS				// number of buttons
} WindowButtonIndices;

typedef enum {
	BUTTON_STATE_NORMAL	=  0,
	BUTTON_STATE_PRELIGHT,
	BUTTON_STATE_PRESSED,

  BUTTON_STATES
} WBButtonSt;

typedef struct {
    gchar      *button_layout;    // Button layout ["XXX"] (example "HMC" : H=Hide, M=Maximize/unMaximize, C=Close)
} WCKPreferences;

/* Definition for our button */
typedef struct {
	GtkEventBox 	*eventbox;
	GtkImage 		*image;
} WindowButton;

/* plugin structure for title and buttons*/
typedef struct {
    XfcePanelPlugin *plugin;

    /* Widgets */
    GtkWidget *ebox;
    GtkWidget *hvbox;

    WindowButton  *icon;			    // Icon widget
    WindowButton  **button;			    // Array of buttons
    gboolean  *button_hidden;			// Indicates whether a button is hidden
    gushort        font_width;

    WCKPreferences *prefs;              // Main properties
    gint        setting2;
    gboolean    setting3;

    GdkPixbuf *pixbufs[IMAGES_STATES][IMAGES_BUTTONS];
} WBPlugin;

void
wckbuttons_save (XfcePanelPlugin *plugin,
             WBPlugin    *wb);

G_END_DECLS

#endif /* !__WCKBUTTONS_H__ */
