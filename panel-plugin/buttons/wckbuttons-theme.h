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
 *  This stuff is mostly derived from xfwm4 sources (setting.c)
 *
 */

#ifndef __WCKBUTTONS_THEME_H__
#define __WCKBUTTONS_THEME_H__

#include <gtk/gtk.h>
#include "wckbuttons.h"

/* default settings */
#define DEFAULT_THEME "Windowck"

G_BEGIN_DECLS

typedef struct {
    gchar  *option;
    GValue *value;
    GType   type;
    gboolean required;
} Settings;


void init_theme (WBPlugin *wb);
void load_theme (const gchar *theme, WBPlugin *wb);
void replace_buttons (const gchar *button_layout, WBPlugin *wb);
gchar *get_rc_button_layout (const gchar *theme);
gchar *button_layout_filter  (const gchar *button_layout, const gchar *default_layout);
gchar *opposite_layout_filter  (const gchar *string);

G_END_DECLS

#endif /* !__WCKBUTTONS_THEME_H__ */
