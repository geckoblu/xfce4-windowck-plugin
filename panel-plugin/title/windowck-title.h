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

#ifndef WINDOWCK_TITLE_H_
#define WINDOWCK_TITLE_H_

#include "windowck.h"

#define TITLE_SIZE_MAX 999 /* title size max for expand option in characters */

void init_title (WindowckPlugin *wckp);
void resize_title(WindowckPlugin *wckp);
void reload_wnck_title (WindowckPlugin *wckp);
void update_font(WindowckPlugin *wckp);
gboolean on_title_pressed(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp);
gboolean on_title_released(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp);
gboolean on_icon_released(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp);

#endif /* WINDOWCK_TITLE_H_ */
