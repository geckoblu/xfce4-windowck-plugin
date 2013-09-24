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

#ifndef WINDOWCK_TITLE_H_
#define WINDOWCK_TITLE_H_

#include "windowck.h"

void initTitle (WindowckPlugin *wckp);
void resizeTitle(WindowckPlugin *wckp);
void expandTitle(WindowckPlugin *wckp);
float alignTitle(WindowckPlugin *wckp);
void on_wck_state_changed (WnckWindow *controlwindow, WindowckPlugin *wckp);
void on_windowck_size_allocated(GtkWidget *widget, GtkAllocation *allocation,  WindowckPlugin *wckp);
void on_control_window_changed(WnckWindow *controlwindow, WnckWindow *previous, WindowckPlugin *wckp);
void updateFont(WindowckPlugin *wckp);
gboolean title_clicked(GtkWidget *, GdkEventButton *, WindowckPlugin *);

#endif /* WINDOWCK_TITLE_H_ */
