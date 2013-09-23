/*
 *  Copyright (C) 2013 Cedric Leporcq <cedl38 at gmail dot com>
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

#include "wck-utils.h"

/* Prototypes */
static WnckWindow *getRootWindow(WnckScreen *screen);
static WnckWindow *getUpperMaximized(WckUtils *);
static void trackControledWindow (WckUtils *);
static void active_workspace_changed(WnckScreen *, WnckWorkspace *, WckUtils *);
static void active_window_changed(WnckScreen *, WnckWindow *, WckUtils *);
static void active_window_state_changed(WnckWindow *, WnckWindowState, WnckWindowState, WckUtils *);
static void on_umaxed_window_state_changed(WnckWindow *, WnckWindowState, WnckWindowState, WckUtils *);
static void on_viewports_changed(WnckScreen *, WckUtils *);
static void on_window_closed(WnckScreen *, WnckWindow *, WckUtils *);
static void on_window_opened(WnckScreen *, WnckWindow *, WckUtils *);

static WnckWindow *getRootWindow (WnckScreen *screen) {
    GList *winstack = wnck_screen_get_windows_stacked(screen);
    // we can't access data directly because sometimes we will get NULL or not desktop window
    if (winstack && wnck_window_get_window_type (winstack->data) == WNCK_WINDOW_DESKTOP)
        return winstack->data;
    else
        return NULL;
}

/* Trigger when activewindow's state changes */
static void active_window_workspace_changed (WnckWindow *window,                                                WckUtils *win) {
        trackControledWindow (win);
}

/* Trigger when activewindow's state changes */
static void active_window_state_changed (WnckWindow *window,
                                         WnckWindowState changed_mask,
                                         WnckWindowState new_state,
                                         WckUtils *win) {
    if ((changed_mask & (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY | WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY | WNCK_WINDOW_STATE_MINIMIZED))
        || !win->only_maximized) {
        trackControledWindow (win);
    }
}

/* Triggers when umaxedwindow's state changes */
static void on_umaxed_window_state_changed (WnckWindow *window,
                                          WnckWindowState changed_mask,
                                          WnckWindowState new_state,
                                          WckUtils *win) {
    /* WARNING : only if window is unmaximized to prevent growing loop !!!*/
    if (!wnck_window_is_maximized(window)
        || wnck_window_is_minimized(window)) {
        trackControledWindow (win);
    }
    else {
        on_wck_state_changed(win->controlwindow, win->data);
    }
}

/* Returns the highest maximized window */
static WnckWindow *getUpperMaximized (WckUtils *win) {
    WnckWindow      *umaxedwindow = NULL;

    GList *windows = wnck_screen_get_windows_stacked(win->activescreen);

    while (windows && windows->data) {
        if (wnck_window_is_in_viewport(windows->data, win->activeworkspace)
        && wnck_window_is_maximized(windows->data)
        && !wnck_window_is_minimized(windows->data)) {
            umaxedwindow = windows->data;
        }
        windows = windows->next;
    }
    //* NULL if no maximized window found
    return umaxedwindow;
}

static void trackControledWindow (WckUtils *win) {
    WnckWindow      *previous;
    gint i;

    previous = win->controlwindow;

    if (previous) {
        if (win->csh
            && (g_signal_handler_is_connected(G_OBJECT(previous), win->csh)))
            g_signal_handler_disconnect(G_OBJECT(previous), win->csh);
        if (win->awh
            && (g_signal_handler_is_connected(G_OBJECT(previous), win->awh)))  g_signal_handler_disconnect(G_OBJECT(previous), win->awh);
    }

    if (win->activewindow
        && (wnck_window_is_maximized(win->activewindow)
            || !win->only_maximized)) {
        win->controlwindow = win->activewindow;
    }
    else if (win->only_maximized) {

        win->controlwindow = getUpperMaximized(win);

        if (win->controlwindow) {
            if (win->controlwindow) {
                // start tracking the new umaxed window
                win->csh = g_signal_connect(G_OBJECT(win->controlwindow),
                                                       "state-changed",
                                                       G_CALLBACK (on_umaxed_window_state_changed),
                                                       win);
                win->awh = g_signal_connect(G_OBJECT (win->controlwindow), "workspace-changed", G_CALLBACK (active_window_workspace_changed), win);
            }
        }

    }

    if (!win->controlwindow) {
        win->controlwindow = getRootWindow(win->activescreen);
    }

    on_control_window_changed(win->controlwindow, previous, win->data);
}

/* Triggers when a new window has been opened */
static void on_window_opened (WnckScreen *screen,
                           WnckWindow *window,
                           WckUtils *win) {
    // track new maximized window
    if (wnck_window_is_maximized(window)) {
        trackControledWindow (win);
    }
}

/* Triggers when a window has been closed */
static void on_window_closed (WnckScreen *screen,
                           WnckWindow *window,
                           WckUtils *win) {

    // track closed maximized window
    if (wnck_window_is_maximized(window)) {
        trackControledWindow (win);
    }
}

/* Triggers when a new active window is selected */
static void active_window_changed (WnckScreen *screen,
                                   WnckWindow *previous,
                                   WckUtils *win) {
    gint i;

    // Start tracking the new active window:
    if (win->activewindow && win->ash) {
        if (g_signal_handler_is_connected(G_OBJECT(win->activewindow), win->ash))
            g_signal_handler_disconnect(G_OBJECT(win->activewindow), win->ash);
    }

    win->activewindow = wnck_screen_get_active_window(screen);

    if (win->activewindow) {
        /* track the new controled window according to preferences */
        if ((wnck_window_is_maximized(win->activewindow))
            || !win->only_maximized
            || !wnck_window_is_in_viewport(win->controlwindow, win->activeworkspace)) {
            trackControledWindow (win);
        }
        else {
            on_wck_state_changed(win->controlwindow, win->data);
        }
        // if we got NULL it would start flickering (but we shouldn't get NULL anymore)
        win->ash = g_signal_connect(G_OBJECT (win->activewindow), "state-changed", G_CALLBACK (active_window_state_changed), win);
    }
}

/* Triggers when user changes viewports on Compiz */
// We ONLY need this for Compiz (Marco doesn't use viewports)
static void on_viewports_changed (WnckScreen *screen, WckUtils *win)
{
    win->activeworkspace = wnck_screen_get_active_workspace(screen);
    if (!win->activeworkspace)
        win->activeworkspace = wnck_screen_get_workspace(win->activescreen, 0);

    win->activewindow = wnck_screen_get_active_window(screen);
    trackControledWindow(win);
}

/* Triggers when user changes workspace on Marco (?) */
static void active_workspace_changed (WnckScreen *screen,
                                      WnckWorkspace *previous,
                                      WckUtils *win) {
    win->activeworkspace = wnck_screen_get_active_workspace(screen);
    if (!win->activeworkspace)
        win->activeworkspace = wnck_screen_get_workspace(win->activescreen, 0);

    win->activewindow = wnck_screen_get_active_window(screen);
    trackControledWindow(win);
}

void toggleMaximize (WckUtils *win) {
    if (win->controlwindow) {
		if (wnck_window_is_maximized(win->controlwindow))
			wnck_window_maximize(win->controlwindow);
		else
			wnck_window_unmaximize(win->controlwindow);
    }
}

void initWnck (WckUtils *win, gboolean only_maximized, gpointer data) {

    if (win->activescreen) {
        if (win->sch) {
            if (g_signal_handler_is_connected(G_OBJECT(win->activescreen), win->sch))
                g_signal_handler_disconnect(G_OBJECT(win->activescreen), win->sch);
        }
        if (win->soh) {
            if (g_signal_handler_is_connected(G_OBJECT(win->activescreen), win->soh))
                g_signal_handler_disconnect(G_OBJECT(win->activescreen), win->soh);
        }
        if (win->svh) {
            if (g_signal_handler_is_connected(G_OBJECT(win->activescreen), win->svh))
                g_signal_handler_disconnect(G_OBJECT(win->activescreen), win->svh);
        }
        if (win->swh) {
            if (g_signal_handler_is_connected(G_OBJECT(win->activescreen), win->swh))
                g_signal_handler_disconnect(G_OBJECT(win->activescreen), win->swh);
        }
    }

    /* save data */
    win->data = data;

    /* get window proprieties */
    win->activescreen = wnck_screen_get_default();
    win->activeworkspace = wnck_screen_get_active_workspace(win->activescreen);
    if (!win->activeworkspace)
        win->activeworkspace = wnck_screen_get_workspace(win->activescreen, 0);
    win->activewindow = wnck_screen_get_active_window(win->activescreen);
    win->only_maximized = only_maximized;

    // Global window tracking
    g_signal_connect(win->activescreen, "active-window-changed", G_CALLBACK (active_window_changed), win);

    if (win->only_maximized) {
        win->sch = g_signal_connect(win->activescreen, "window-closed", G_CALLBACK (on_window_closed), win);
        win->soh = g_signal_connect(win->activescreen, "window-opened", G_CALLBACK (on_window_opened), win);
        win->svh = g_signal_connect(win->activescreen, "viewports-changed", G_CALLBACK (on_viewports_changed), win);
        win->swh = g_signal_connect(win->activescreen, "active-workspace-changed", G_CALLBACK (active_workspace_changed), win);
    }

    trackControledWindow (win);
}
