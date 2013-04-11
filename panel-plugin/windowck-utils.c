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

#include "windowck.h"
#include "windowck-utils.h"

/* Prototypes */
static WnckWindow *getRootWindow(WnckScreen *);
static WnckWindow *getUpperMaximized(WindowckPlugin *);
static void active_workspace_changed(WnckScreen *, WnckWorkspace *, WindowckPlugin *);
static void active_window_changed(WnckScreen *, WnckWindow *, WindowckPlugin *);
static void active_window_state_changed(WnckWindow *, WnckWindowState, WnckWindowState, WindowckPlugin *);
static void active_window_nameicon_changed(WnckWindow *, WindowckPlugin *);
static void umaxed_window_state_changed(WnckWindow *, WnckWindowState, WnckWindowState, WindowckPlugin *);
static void umaxed_window_nameicon_changed(WnckWindow *, WindowckPlugin *);
static void viewports_changed(WnckScreen *, WindowckPlugin *);
static void window_closed(WnckScreen *, WnckWindow *, WindowckPlugin *);
static void window_opened(WnckScreen *, WnckWindow *, WindowckPlugin *);
static void updateTitle(WindowckPlugin *);

void initWnck(WindowckPlugin *wckp) {
    wckp->activescreen = wnck_screen_get_default();
    wnck_screen_force_update(wckp->activescreen);

    wckp->activeworkspace = wnck_screen_get_active_workspace(wckp->activescreen);
    wckp->activewindow = wnck_screen_get_active_window(wckp->activescreen);
    wckp->umaxedwindow = getUpperMaximized(wckp);
    wckp->rootwindow = getRootWindow(wckp->activescreen);

    // Global window tracking  <-- this thing is crashing with compiz !!!
    g_signal_connect(wckp->activescreen, "active-window-changed", G_CALLBACK (active_window_changed), wckp);

    g_signal_connect(wckp->activescreen, "viewports-changed", G_CALLBACK (viewports_changed), wckp);
    g_signal_connect(wckp->activescreen, "active-workspace-changed", G_CALLBACK (active_workspace_changed), wckp);
    g_signal_connect(wckp->activescreen, "window-closed", G_CALLBACK (window_closed), wckp);
    g_signal_connect(wckp->activescreen, "window-opened", G_CALLBACK (window_opened), wckp);

    updateTitle(wckp);
}

/* Safely returns the bottom-most (root) window */
static WnckWindow *
getRootWindow(WnckScreen *screen) {
    GList *winstack = wnck_screen_get_windows_stacked(screen);
    if (winstack)
        return winstack->data;
    else
        return NULL ;
}

static WnckWindow *getUpperMaximized(WindowckPlugin *wckp) {
    if (!wckp->prefs->only_maximized)
        return wckp->activewindow;

    GList *windows = wnck_screen_get_windows_stacked(wckp->activescreen);
    WnckWindow *returnwindow = NULL;

    while (windows && windows->data) {
        if (wnck_window_is_maximized(windows->data)) {
            // if(wnck_window_is_on_workspace(windows->data, wckp->activeworkspace))
            if (!wnck_window_is_minimized(windows->data))
                if (wnck_window_is_in_viewport(windows->data, wckp->activeworkspace))
                    returnwindow = windows->data;
        }
        windows = windows->next;
    }

    // Start tracking the new umaxed window
    if (wckp->umaxedwindow) {
        if (g_signal_handler_is_connected(G_OBJECT(wckp->umaxedwindow), wckp->umaxed_handler_state))
            g_signal_handler_disconnect(G_OBJECT(wckp->umaxedwindow), wckp->umaxed_handler_state);
        if (g_signal_handler_is_connected(G_OBJECT(wckp->umaxedwindow), wckp->umaxed_handler_name))
            g_signal_handler_disconnect(G_OBJECT(wckp->umaxedwindow), wckp->umaxed_handler_name);
        //if (g_signal_handler_is_connected(G_OBJECT(wckp->umaxedwindow), wckp->umaxed_handler_icon))
        //    g_signal_handler_disconnect(G_OBJECT(wckp->umaxedwindow), wckp->umaxed_handler_icon);
    }

    if (returnwindow) {
        // maxed window was found
        wckp->umaxed_handler_state = g_signal_connect(G_OBJECT(returnwindow), "state-changed", G_CALLBACK(umaxed_window_state_changed), wckp);
        wckp->umaxed_handler_name = g_signal_connect(G_OBJECT(returnwindow), "name-changed", G_CALLBACK(umaxed_window_nameicon_changed), wckp);
        //wckp->umaxed_handler_icon = g_signal_connect(G_OBJECT(returnwindow), "icon-changed", G_CALLBACK(umaxed_window_nameicon_changed), wckp);
    } else {
        // maxed window was not found
        returnwindow = wckp->rootwindow; //return wckp->rootwindow;
    }
    return returnwindow;
    // WARNING: if this function returns NULL, applet won't detect clicks!
}

/* Triggers when a new active window is selected */
static void active_window_changed(WnckScreen *screen, WnckWindow *previous, WindowckPlugin *wckp) {
    // Start tracking the new active window:
    if (wckp->activewindow) {
        if (g_signal_handler_is_connected(G_OBJECT(wckp->activewindow), wckp->active_handler_state))
            g_signal_handler_disconnect(G_OBJECT(wckp->activewindow), wckp->active_handler_state);
        if (g_signal_handler_is_connected(G_OBJECT(wckp->activewindow), wckp->active_handler_name))
            g_signal_handler_disconnect(G_OBJECT(wckp->activewindow), wckp->active_handler_name);
        //if (g_signal_handler_is_connected(G_OBJECT(wckp->activewindow), wckp->active_handler_icon))
        //    g_signal_handler_disconnect(G_OBJECT(wckp->activewindow), wckp->active_handler_icon);
    }

    wckp->activewindow = wnck_screen_get_active_window(screen);
    wckp->umaxedwindow = getUpperMaximized(wckp); // returns wbapplet->activewindow if not only_maximized
    wckp->rootwindow = getRootWindow(wckp->activescreen);

    if (wckp->activewindow) {
        wckp->active_handler_state = g_signal_connect(G_OBJECT (wckp->activewindow), "state-changed", G_CALLBACK (active_window_state_changed), wckp);
        wckp->active_handler_name = g_signal_connect(G_OBJECT (wckp->activewindow), "name-changed", G_CALLBACK (active_window_nameicon_changed), wckp);
        //wckp->active_handler_icon = g_signal_connect(G_OBJECT (wckp->activewindow), "icon-changed", G_CALLBACK (active_window_nameicon_changed), wckp);

        wckp->focused = TRUE;

        updateTitle(wckp);
    }
}

// Updates the images according to preferences and the window situation
// Warning! This function is called very often, so it should only do the most necessary things!
void updateTitle(WindowckPlugin *wckp) {
    WnckWindow *controlledwindow;
    gchar *title_text, *title_color, *title_font;
    GdkPixbuf *icon_pixbuf;

    if (wckp->prefs->only_maximized) {
        controlledwindow = wckp->umaxedwindow;
    } else {
        controlledwindow = wckp->activewindow;
    }

    if (controlledwindow == NULL )
        return;

    if (controlledwindow == wckp->rootwindow) {
        // we're on desktop
        if (wckp->prefs->hide_on_unmaximized) {
            // hide everything
            icon_pixbuf = NULL;
            title_text = "";
        } else {
            icon_pixbuf = NULL; // Hide the icon
            title_text = ("Desktop");
        }
    } else {
        icon_pixbuf = wnck_window_get_icon(controlledwindow); // This only returns a pointer - it SHOULDN'T be unrefed!
        title_text = (gchar*) wnck_window_get_name(controlledwindow);
    }

    if (controlledwindow == wckp->activewindow) {
        // window focuseD
        gtk_widget_set_sensitive(GTK_WIDGET(wckp->title), TRUE);
    } else {
        // window unfocused
        gtk_widget_set_sensitive(GTK_WIDGET(wckp->title), FALSE);
    }

    // Set tooltips
    if (wckp->prefs->show_tooltips) {
        //gtk_widget_set_tooltip_text(GTK_WIDGET(wckp->icon), title_text);
        gtk_widget_set_tooltip_text(GTK_WIDGET(wckp->title), title_text);
    }

    gtk_label_set_text(wckp->title, title_text);

//  if (icon_pixbuf == NULL )
//    {
//      gtk_image_clear(wckp->icon);
//    }
//  else
//    {
//      // We're updating window info (Careful! We've had pixbuf memory leaks here)
//      GdkPixbuf *ipb1 = gdk_pixbuf_scale_simple(icon_pixbuf, ICON_WIDTH, ICON_HEIGHT, GDK_INTERP_BILINEAR);
//      if (controlledwindow == wckp->rootwindow)
//        g_object_unref(icon_pixbuf); //this is stupid beyond belief, thanks to the retarded GTK framework
//      GdkPixbuf *ipb2 = gdk_pixbuf_rotate_simple(ipb1, wckp->angle);
//      g_object_unref(ipb1);   // Unref ipb1 to get it cleared from memory (we still need ipb2)
//
//      // Saturate icon when window is not focused
//      if (controlledwindow != wckp->activewindow)
//        gdk_pixbuf_saturate_and_pixelate(ipb2, ipb2, 0, FALSE);
//
//      // Apply pixbuf to icon widget
//      gtk_image_set_from_pixbuf(wckp->icon, ipb2);
//      g_object_unref(ipb2);   // Unref ipb2 to get it cleared from memory
//    }
}

/* Triggers when user changes viewports (Compiz) */
static void viewports_changed(WnckScreen *screen, WindowckPlugin *wckp) {
    wckp->activeworkspace = wnck_screen_get_active_workspace(screen);
    wckp->activewindow = wnck_screen_get_active_window(screen);
    wckp->rootwindow = getRootWindow(wckp->activescreen); //?
    wckp->umaxedwindow = getUpperMaximized(wckp);

    // active_window_changed will do it too, but this one will be sooner
    updateTitle(wckp);
}

/* Triggers when a new window has been opened */
// in case a new maximized non-active window appears
static void window_opened(WnckScreen *screen, WnckWindow *window, WindowckPlugin *wckp) {

    wckp->umaxedwindow = getUpperMaximized(wckp);

    updateTitle(wckp);
}

/* Triggers when a window has been closed */
// in case the last maximized window was closed
static void window_closed(WnckScreen *screen, WnckWindow *window, WindowckPlugin *wckp) {

    wckp->umaxedwindow = getUpperMaximized(wckp);

    updateTitle(wckp); // required when closing window in the background
}

/* Triggers when.... ? not sure... (Metacity?) */
static void active_workspace_changed(WnckScreen *screen, WnckWorkspace *previous, WindowckPlugin *wckp) {
    wckp->activeworkspace = wnck_screen_get_active_workspace(screen);
    /*
     wckp->activewindow = wnck_screen_get_active_window(screen);
     // wckp->rootwindow = getRootWindow(wckp->activescreen); //?
     wckp->umaxedwindow = getUpperMaximized(wckp);

     updateTitle(wckp);
     */
}

/* Triggers when umaxedwindow's name OR ICON changes */
static void umaxed_window_nameicon_changed(WnckWindow *window, WindowckPlugin *wckp) {
    updateTitle(wckp);
}

/* Triggers when umaxedwindow's state changes */
static void umaxed_window_state_changed(WnckWindow *window, WnckWindowState changed_mask, WnckWindowState new_state, WindowckPlugin *wckp) {
    wckp->umaxedwindow = getUpperMaximized(wckp);
    wckp->rootwindow = getRootWindow(wckp->activescreen);

    updateTitle(wckp);
}

/* Trigger when activewindow's state changes */
static void active_window_state_changed(WnckWindow *window, WnckWindowState changed_mask, WnckWindowState new_state, WindowckPlugin *wckp) {

    wckp->umaxedwindow = getUpperMaximized(wckp);
    wckp->rootwindow = getRootWindow(wckp->activescreen);

    if (new_state & (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY | WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY)) {
        wckp->focused = TRUE;
    } else {
        if (wckp->prefs->only_maximized) {
            wckp->focused = FALSE;
        }
    }

    updateTitle(wckp);
}

/* Triggers when activewindow's name changes */
static void active_window_nameicon_changed(WnckWindow *window, WindowckPlugin *wckp) {
    updateTitle(wckp);
}

gboolean title_clicked(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp) {
    // only allow left and right mouse button
    //if (event->button != 1 && event->button != 3) return FALSE;

    WnckWindow *controlledwindow;

    if (wckp->prefs->only_maximized) {
        controlledwindow = wckp->umaxedwindow;
    } else {
        controlledwindow = wckp->activewindow;
    }

    if (!controlledwindow)
        return FALSE;

    // single click (left/right)
    if (event->button == 1) {
        // left-click
        wnck_window_activate(controlledwindow, gtk_get_current_event_time());
        if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_3BUTTON_PRESS) {
            // double/tripple click
            //if (event->type==GDK_2BUTTON_PRESS) {
            if (wnck_window_is_maximized(controlledwindow)) {
                wnck_window_unmaximize(controlledwindow);
            } else {
                wnck_window_maximize(controlledwindow);
            }
        }
    } else if (event->button == 3) {
        // right-click
        wnck_window_activate(controlledwindow, gtk_get_current_event_time());

        // let the panel show the menu
        return FALSE;
    } else {
        return FALSE;
    }
    return TRUE;
}
