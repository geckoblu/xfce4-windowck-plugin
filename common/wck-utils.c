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

#include "wck-utils.h"

/* Prototypes */
static WnckWindow *get_root_window(WnckScreen *screen);
static WnckWindow *get_upper_maximized(WckUtils *);
static void track_controled_window (WckUtils *);
static void active_workspace_changed(WnckScreen *, WnckWorkspace *, WckUtils *);
static void active_window_changed(WnckScreen *, WnckWindow *, WckUtils *);
static void track_changed_max_state(WnckWindow *, WnckWindowState, WnckWindowState, WckUtils *);
static void on_umaxed_window_state_changed(WnckWindow *, WnckWindowState, WnckWindowState, WckUtils *);
static void on_viewports_changed(WnckScreen *, WckUtils *);
static void on_window_closed(WnckScreen *, WnckWindow *, WckUtils *);
static void on_window_opened(WnckScreen *, WnckWindow *, WckUtils *);


gboolean wck_signal_handler_disconnect (GObject *object, gulong handler)
{
    if (object && handler > 0)
    {
        if (g_signal_handler_is_connected(object, handler))
        {
            g_signal_handler_disconnect(object, handler);
            return TRUE;
        }
    }
    return FALSE;
}


static WnckWindow *get_root_window (WnckScreen *screen)
{
    GList *winstack = wnck_screen_get_windows_stacked(screen);
    // we can't access data directly because sometimes we will get NULL or not desktop window
    if (winstack && wnck_window_get_window_type (winstack->data) == WNCK_WINDOW_DESKTOP)
        return winstack->data;
    else
        return NULL;
}


/* Trigger when activewindow's workspaces changes */
static void umax_window_workspace_changed (WnckWindow *window,                                                WckUtils *win)
{
        track_controled_window (win);
}


/* Trigger when a specific window's state changes */
static void track_changed_max_state (WnckWindow *window,
                                         WnckWindowState changed_mask,
                                         WnckWindowState new_state,
                                         WckUtils *win)
{
    /* track the window max state only if it isn't the control window */
    if (window != win->controlwindow)
    {
        if (window
            && !wnck_window_is_minimized(window)
            && wnck_window_is_maximized(window))
        {
            track_controled_window (win);
        }
    }
}


/* Triggers when umaxedwindow's state changes */
static void on_umaxed_window_state_changed (WnckWindow *window,
                                          WnckWindowState changed_mask,
                                          WnckWindowState new_state,
                                          WckUtils *win)
{
    /* WARNING : only if window is unmaximized to prevent growing loop !!!*/
    if (!wnck_window_is_maximized(window)
        || wnck_window_is_minimized(window)
        || changed_mask & (WNCK_WINDOW_STATE_ABOVE))
    {
        track_controled_window (win);
    }
    else {
        on_wck_state_changed(win->controlwindow, win->data);
    }
}


/* Returns the highest maximized window */
static WnckWindow *get_upper_maximized (WckUtils *win)
{
    WnckWindow      *umaxedwindow = NULL;

    GList *windows = wnck_screen_get_windows_stacked(win->activescreen);

    while (windows && windows->data)
    {
        if (!win->activeworkspace
            || wnck_window_is_in_viewport(windows->data, win->activeworkspace))
            if (wnck_window_is_maximized(windows->data)
                && !wnck_window_is_minimized(windows->data))
        {
                umaxedwindow = windows->data;
        }
        windows = windows->next;
    }
    /* NULL if no maximized window found */
    return umaxedwindow;
}


/* track the new controled window according to preferences */
static void track_controled_window (WckUtils *win)
{
    WnckWindow      *previous_umax = NULL;
    WnckWindow      *previous_control = NULL;

    previous_control = win->controlwindow;

    if (win->only_maximized)
    {
        previous_umax = win->umaxwindow;
        win->umaxwindow = get_upper_maximized(win);
        win->controlwindow = win->umaxwindow;
    }
    else if (win->activewindow
            && (!win->activeworkspace
                || wnck_window_is_in_viewport(win->activewindow, win->activeworkspace))
            && !wnck_window_is_minimized(win->activewindow)
            && !wnck_window_is_sticky(win->activewindow))
    {
            win->controlwindow = win->activewindow;
    }

    if (!win->umaxwindow || (win->umaxwindow != previous_umax))
    {
        wck_signal_handler_disconnect (G_OBJECT(previous_umax), win->msh);
        wck_signal_handler_disconnect (G_OBJECT(previous_umax), win->mwh);
    }

    if (win->only_maximized)
    {
        if (win->umaxwindow && (win->umaxwindow != previous_umax))
        {
            /* track the new upper maximized window state */
            win->msh = g_signal_connect(G_OBJECT(win->umaxwindow),
                                           "state-changed",
                                           G_CALLBACK (on_umaxed_window_state_changed),
                                           win);
            win->mwh = g_signal_connect(G_OBJECT (win->umaxwindow),
                                        "workspace-changed",
                                        G_CALLBACK (umax_window_workspace_changed),
                                        win);
        }
        else if (win->controlwindow == previous_control)
        {
            /* track previous upper maximized window state on desktop */
            win->umaxwindow = previous_umax;
            if (win->umaxwindow) {
                win->msh = g_signal_connect(G_OBJECT(win->umaxwindow),
                                               "state-changed",
                                               G_CALLBACK (track_changed_max_state),
                                               win);
            }
        }
    }

    if (!win->controlwindow)
        win->controlwindow = get_root_window(win->activescreen);

    if (win->controlwindow != previous_control)
        on_control_window_changed(win->controlwindow, previous_control, win->data);
    else
        on_wck_state_changed(win->controlwindow, win->data);
}


/* Triggers when a new window has been opened */
static void on_window_opened (WnckScreen *screen,
                           WnckWindow *window,
                           WckUtils *win)
{
    // track new maximized window
    if (wnck_window_is_maximized(window))
        track_controled_window (win);
}


/* Triggers when a window has been closed */
static void on_window_closed (WnckScreen *screen,
                           WnckWindow *window,
                           WckUtils *win)
{
    // track closed maximized window
    if (wnck_window_is_maximized(window))
        track_controled_window (win);
}


/* Triggers when a new active window is selected */
static void active_window_changed (WnckScreen *screen,
                                   WnckWindow *previous,
                                   WckUtils *win)
{

    win->activewindow = wnck_screen_get_active_window(screen);

    if (win->activewindow != previous)
    {
        wck_signal_handler_disconnect (G_OBJECT(previous), win->ash);

        track_controled_window (win);
    }

    if (win->activewindow
        && (win->activewindow != previous)
        && (wnck_window_get_window_type (win->activewindow) != WNCK_WINDOW_DESKTOP))
    {
        /* Start tracking the new active window */
        win->ash = g_signal_connect(G_OBJECT (win->activewindow), "state-changed", G_CALLBACK (track_changed_max_state), win);
    }
}


/* Triggers when user changes viewports on Compiz */
// We ONLY need this for Compiz (Marco doesn't use viewports)
static void on_viewports_changed (WnckScreen *screen, WckUtils *win)
{
    reload_wnck (win, win->only_maximized, win->data);
}


/* Triggers when user changes workspace on Marco (?) */
static void active_workspace_changed (WnckScreen *screen,
                                      WnckWorkspace *previous,
                                      WckUtils *win)
{
    reload_wnck (win, win->only_maximized, win->data);
}


void toggle_maximize (WnckWindow *window)
{
    if (window && wnck_window_is_maximized(window))
        wnck_window_unmaximize(window);
    else
        wnck_window_maximize(window);
}


void reload_wnck (WckUtils *win, gboolean only_maximized, gpointer data)
{
    /* disconnect all signal handlers */
    wck_signal_handler_disconnect (G_OBJECT(win->controlwindow), win->ash);
    wck_signal_handler_disconnect (G_OBJECT(win->controlwindow), win->msh);
    wck_signal_handler_disconnect (G_OBJECT(win->controlwindow), win->mwh);

    wck_signal_handler_disconnect (G_OBJECT(win->activescreen), win->sch);
    wck_signal_handler_disconnect (G_OBJECT(win->activescreen), win->soh);
    wck_signal_handler_disconnect (G_OBJECT(win->activescreen), win->svh);
    wck_signal_handler_disconnect (G_OBJECT(win->activescreen), win->swh);

    init_wnck (win, only_maximized, data);
}


void init_wnck (WckUtils *win, gboolean only_maximized, gpointer data)
{
    /* save data */
    win->data = data;

    /* get window proprieties */
    win->activescreen = wnck_screen_get_default();
    win->activeworkspace = wnck_screen_get_active_workspace(win->activescreen);
    if (!win->activeworkspace)
        win->activeworkspace = wnck_screen_get_workspace(win->activescreen, 0);
    win->activewindow = wnck_screen_get_active_window(win->activescreen);
    win->umaxwindow = NULL;
    win->controlwindow = NULL;
    win->only_maximized = only_maximized;

    /* Global window tracking */
    g_signal_connect(win->activescreen, "active-window-changed", G_CALLBACK (active_window_changed), win);

    if (win->only_maximized)
    {
        win->sch = g_signal_connect(win->activescreen, "window-closed", G_CALLBACK (on_window_closed), win);
        win->soh = g_signal_connect(win->activescreen, "window-opened", G_CALLBACK (on_window_opened), win);
    }

        win->svh = g_signal_connect(win->activescreen, "viewports-changed", G_CALLBACK (on_viewports_changed), win);
        win->swh = g_signal_connect(win->activescreen, "active-workspace-changed", G_CALLBACK (active_workspace_changed), win);

    /* Get controled window */
    track_controled_window (win);
    
    if (!win->controlwindow)
        on_control_window_changed (NULL, NULL, win->data);
}
