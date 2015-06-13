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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-hvbox.h>

#include "wckbuttons.h"
#include "wckbuttons-dialogs.h"
#include "wckbuttons-theme.h"

/* default settings */
#define DEFAULT_ONLY_MAXIMIZED TRUE
#define DEFAULT_SHOW_ON_DESKTOP FALSE
#define DEFAULT_SYNC_WM_THEME TRUE
#define DEFAULT_BUTTON_LAYOUT "HMC"
#define DEFAULT_INACTIVE_TEXT_ALPHA 60
#define DEFAULT_INACTIVE_TEXT_SHADE 110

#define BUTTONS_SIGNALS_CONNECT(name, id) \
        g_signal_connect (G_OBJECT (wb->button[id]->eventbox), "button-press-event", G_CALLBACK (name##_button_pressed), wb); \
        g_signal_connect (G_OBJECT (wb->button[id]->eventbox), "button-release-event", G_CALLBACK (name##_button_release), wb); \
        g_signal_connect (G_OBJECT (wb->button[id]->eventbox), "enter-notify-event", ((GCallback) (name##_button_hover_enter)), wb); \
        g_signal_connect (G_OBJECT (wb->button[id]->eventbox), "leave-notify-event", G_CALLBACK (name##_button_hover_leave), wb);


/* prototypes */
static void
wckbuttons_construct (XfcePanelPlugin *plugin);


void
wckbuttons_save (XfcePanelPlugin *plugin,
             WBPlugin    *wb)
{
    XfceRc *rc;
    gchar  *file;

    /* get the config file location */
    file = xfce_panel_plugin_save_location (plugin, TRUE);

    if (G_UNLIKELY (file == NULL))
    {
       DBG ("Failed to open config file");
       return;
    }

    /* open the config file, read/write */
    rc = xfce_rc_simple_open (file, FALSE);
    g_free (file);

    if (G_LIKELY (rc != NULL))
    {
        /* save the settings */
        DBG(".");
        xfce_rc_write_bool_entry(rc, "only_maximized", wb->prefs->only_maximized);
        xfce_rc_write_bool_entry(rc, "show_on_desktop", wb->prefs->show_on_desktop);
        xfce_rc_write_bool_entry(rc, "sync_wm_theme", wb->prefs->sync_wm_theme);
        if (wb->prefs->button_layout)
            xfce_rc_write_entry (rc, "button_layout", wb->prefs->button_layout);

        if (wb->prefs->theme)
            xfce_rc_write_entry (rc, "theme", wb->prefs->theme);

        xfce_rc_write_int_entry  (rc, "inactive_text_alpha", wb->prefs->inactive_text_alpha);
        xfce_rc_write_int_entry  (rc, "inactive_text_shade", wb->prefs->inactive_text_shade);

        /* close the rc file */
        xfce_rc_close (rc);
    }
}


static void
wckbuttons_read (WBPlugin *wb)
{
    XfceRc      *rc;
    gchar       *file;
    const gchar *button_layout, *theme;

    /* allocate memory for the preferences structure */
    wb->prefs = g_slice_new0(WBPreferences);

    /* get the plugin config file location */
    file = xfce_panel_plugin_save_location (wb->plugin, TRUE);

    if (G_LIKELY (file != NULL))
    {
        /* open the config file, readonly */
        rc = xfce_rc_simple_open (file, TRUE);

        /* cleanup */
        g_free (file);

        if (G_LIKELY (rc != NULL))
        {
            /* read the settings */
            wb->prefs->only_maximized = xfce_rc_read_bool_entry(rc, "only_maximized", DEFAULT_ONLY_MAXIMIZED);
            wb->prefs->show_on_desktop = xfce_rc_read_bool_entry(rc, "show_on_desktop", DEFAULT_SHOW_ON_DESKTOP);
            wb->prefs->sync_wm_theme = xfce_rc_read_bool_entry(rc, "sync_wm_theme", DEFAULT_SYNC_WM_THEME);
            button_layout = xfce_rc_read_entry (rc, "button_layout", DEFAULT_BUTTON_LAYOUT);
            wb->prefs->button_layout = button_layout_filter (button_layout, DEFAULT_BUTTON_LAYOUT);
            theme = xfce_rc_read_entry (rc, "theme", DEFAULT_THEME);
            wb->prefs->theme = g_strdup (theme);

            wb->prefs->inactive_text_alpha = xfce_rc_read_int_entry (rc, "inactive_text_alpha", DEFAULT_INACTIVE_TEXT_ALPHA);
            wb->prefs->inactive_text_shade = xfce_rc_read_int_entry (rc, "inactive_text_shade", DEFAULT_INACTIVE_TEXT_SHADE);

            /* cleanup */
            xfce_rc_close (rc);

            /* leave the function, everything went well */
            return;
        }
    }

    /* something went wrong, apply default values */
    DBG ("Applying default settings");

    wb->prefs->only_maximized = DEFAULT_ONLY_MAXIMIZED;
    wb->prefs->show_on_desktop = DEFAULT_SHOW_ON_DESKTOP;
    wb->prefs->sync_wm_theme = DEFAULT_SYNC_WM_THEME;
    wb->prefs->button_layout = DEFAULT_BUTTON_LAYOUT;
    wb->prefs->theme = DEFAULT_THEME;
    wb->prefs->inactive_text_alpha = DEFAULT_INACTIVE_TEXT_ALPHA;
    wb->prefs->inactive_text_shade = DEFAULT_INACTIVE_TEXT_SHADE;
}

static WindowButton **create_buttons (WBPlugin *wb)
{
    WindowButton **button = g_new(WindowButton*, BUTTONS);
    gint i;

    for (i=0; i<BUTTONS; i++)
    {
        button[i] = g_new0 (WindowButton, 1);
        button[i]->eventbox = GTK_EVENT_BOX (gtk_event_box_new());
        button[i]->image = GTK_IMAGE (gtk_image_new());

        gtk_widget_set_can_focus (GTK_WIDGET(button[i]->eventbox), TRUE);

        gtk_container_add (GTK_CONTAINER (button[i]->eventbox), GTK_WIDGET(button[i]->image));
        gtk_event_box_set_visible_window (button[i]->eventbox, FALSE);
        gtk_box_pack_start (GTK_BOX (wb->hvbox), GTK_WIDGET(button[i]->eventbox), TRUE, TRUE, 0);

        /* Add hover events to eventboxes */
        gtk_widget_add_events (GTK_WIDGET (button[i]->eventbox), GDK_ENTER_NOTIFY_MASK); //add the "enter" signal
        gtk_widget_add_events (GTK_WIDGET (button[i]->eventbox), GDK_LEAVE_NOTIFY_MASK); //add the "leave" signal
    }
    return button;
}


static WBPlugin *
wckbuttons_new (XfcePanelPlugin *plugin)
{
    WBPlugin   *wb;
    GtkOrientation  orientation;

    /* allocate memory for the plugin structure */
    wb = g_slice_new0 (WBPlugin);

    /* pointer to plugin */
    wb->plugin = plugin;

    /* read the user settings */
    wckbuttons_read (wb);

    /* get the current orientation */
    orientation = xfce_panel_plugin_get_orientation (plugin);

    /* create some panel widgets */
    wb->ebox = gtk_event_box_new ();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(wb->ebox), FALSE);
    gtk_widget_set_name(wb->ebox, "XfceWckButtonsPlugin");

    wb->hvbox = xfce_hvbox_new (orientation, FALSE, 2);

    /* create buttons */
    wb->button = create_buttons (wb);

    gtk_widget_show (wb->ebox);
    gtk_widget_show (wb->hvbox);
    gtk_container_add (GTK_CONTAINER (wb->ebox), wb->hvbox);

    return wb;
}


static void wckbuttons_free (XfcePanelPlugin *plugin, WBPlugin    *wb)
{
    GtkWidget *dialog;

    /* check if the dialog is still open. if so, destroy it */
    dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
    if (G_UNLIKELY (dialog != NULL))
    gtk_widget_destroy (dialog);

    /* destroy the panel widgets */
    gtk_widget_destroy (wb->hvbox);

    /* cleanup the settings */
    if (G_LIKELY (wb->prefs->button_layout != NULL))
    g_free (wb->prefs->button_layout);

    /* free the plugin structure */
    g_slice_free(WckUtils, wb->win);
    g_slice_free(WBPreferences, wb->prefs);
    g_slice_free (WBPlugin, wb);
}


static void
wckbuttons_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            WBPlugin    *wb)
{
  /* change the orienation of the box */
  xfce_hvbox_set_orientation (XFCE_HVBOX (wb->hvbox), orientation);
}


static gboolean
wckbuttons_size_changed (XfcePanelPlugin *plugin,
                     gint             size,
                     WBPlugin    *wb)
{
    GtkOrientation orientation;

    /* get the orientation of the plugin */
    orientation = xfce_panel_plugin_get_orientation (plugin);

    /* set the widget size */
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
    else
    gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);

    /* we handled the orientation */
    return TRUE;
}

static void set_maximize_button_image (WBPlugin *wb, gushort image_state)
{
    if (wb->win->controlwindow && wnck_window_is_maximized(wb->win->controlwindow)) {
        gtk_image_set_from_pixbuf (wb->button[MAXIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_UNMAXIMIZE][image_state]);
    } else {
        gtk_image_set_from_pixbuf (wb->button[MAXIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MAXIMIZE][image_state]);
    }
}

void on_wck_state_changed (WnckWindow *controlwindow, gpointer data)
{
    WBPlugin *wb = data;

    gushort image_state;

    if (controlwindow && (wnck_window_is_active(controlwindow)))
        image_state = 1;
    else
        image_state = 0;

    /* update buttons images */
    gtk_image_set_from_pixbuf (wb->button[MINIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MINIMIZE][image_state]);

    set_maximize_button_image (wb, image_state);

    gtk_image_set_from_pixbuf (wb->button[CLOSE_BUTTON]->image, wb->pixbufs[IMAGE_CLOSE][image_state]);
}

void on_control_window_changed (WnckWindow *controlwindow, WnckWindow *previous, gpointer data)
{
    WBPlugin *wb = data;
    gint i;

    if (!controlwindow
        || ((wnck_window_get_window_type (controlwindow) == WNCK_WINDOW_DESKTOP)
        && !wb->prefs->show_on_desktop))
    {
        if (gtk_widget_get_visible(GTK_WIDGET(wb->hvbox)))
            gtk_widget_hide_all(GTK_WIDGET(wb->hvbox));
    }
    else
    {
        if (!gtk_widget_get_visible(GTK_WIDGET(wb->hvbox)))
            gtk_widget_show_all(GTK_WIDGET(wb->hvbox));
    }

    if (controlwindow)
    {
        if (wnck_window_get_window_type (controlwindow) != WNCK_WINDOW_DESKTOP)
        {
            for (i=0; i<BUTTONS; i++)
                gtk_widget_set_sensitive(GTK_WIDGET(wb->button[i]->eventbox), TRUE);

            on_wck_state_changed (controlwindow, wb);
            if (!gtk_widget_get_visible(GTK_WIDGET(wb->hvbox)))
                gtk_widget_show_all(GTK_WIDGET(wb->hvbox));
        }
        else if (wb->prefs->show_on_desktop)
        {
            gtk_widget_set_sensitive(GTK_WIDGET(wb->button[MINIMIZE_BUTTON]->eventbox), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(wb->button[MAXIMIZE_BUTTON]->eventbox), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(wb->button[CLOSE_BUTTON]->eventbox), TRUE);

            on_wck_state_changed (controlwindow, wb);
        }
    }
}


/* Called when we release the click on a button */
static gboolean on_minimize_button_release (GtkWidget *event_box,
                               GdkEventButton *event,
                               WBPlugin *wb)
{

    if (event->button != 1) return FALSE;

    wnck_window_minimize(wb->win->controlwindow);

    return TRUE;
}


/* Called when we click on a button */
static gboolean on_minimize_button_pressed (GtkWidget *event_box,
                             GdkEventButton *event,
                             WBPlugin *wb)
{

    if (event->button != 1) return FALSE;

    gtk_image_set_from_pixbuf (wb->button[MINIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MINIMIZE][IMAGE_PRESSED]);

    return TRUE;
}


/* Makes the button stop 'glowing' when the mouse leaves it */
static gboolean on_minimize_button_hover_leave (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb)
{
    gboolean image_state;
    image_state = wnck_window_is_active(wb->win->controlwindow);

    gtk_image_set_from_pixbuf (wb->button[MINIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MINIMIZE][image_state]);
    return TRUE;
}


/* Makes the button 'glow' when the mouse enters it */
static gboolean on_minimize_button_hover_enter (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb)
{
    gtk_image_set_from_pixbuf (wb->button[MINIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MINIMIZE][IMAGE_PRELIGHT]);

    return TRUE;
}


/* Called when we release the click on a button */
static gboolean on_maximize_button_release (GtkWidget *event_box,
                               GdkEventButton *event,
                               WBPlugin *wb)
{
    if (event->button != 1) return FALSE;

    toggle_maximize(wb->win->controlwindow);

    return TRUE;
}


/* Called when we click on a button */
static gboolean on_maximize_button_pressed (GtkWidget *event_box,
                             GdkEventButton *event,
                             WBPlugin *wb)
{
    if (event->button != 1) return FALSE;

    set_maximize_button_image (wb, IMAGE_PRESSED);

    return TRUE;
}


/* Makes the button stop 'glowing' when the mouse leaves it */
static gboolean on_maximize_button_hover_leave (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb)
{
    if (wb->win->controlwindow) {
        set_maximize_button_image (wb, wnck_window_is_active(wb->win->controlwindow));
    }

    return TRUE;
}


/* Makes the button 'glow' when the mouse enters it */
static gboolean on_maximize_button_hover_enter (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb)
{
    set_maximize_button_image (wb, IMAGE_PRELIGHT);

    return TRUE;
}


/* Called when we release the click on a button */
static gboolean on_close_button_release (GtkWidget *event_box,
                               GdkEventButton *event,
                               WBPlugin *wb)
{
    if (event->button != 1) return FALSE;

    wnck_window_close(wb->win->controlwindow, GDK_CURRENT_TIME);

    return TRUE;
}


/* Called when we click on a button */
static gboolean on_close_button_pressed (GtkWidget *event_box,
                             GdkEventButton *event,
                             WBPlugin *wb)
{
    if (event->button != 1) return FALSE;

    gtk_image_set_from_pixbuf (wb->button[CLOSE_BUTTON]->image, wb->pixbufs[IMAGE_CLOSE][IMAGE_PRESSED]);

    return TRUE;
}


/* Makes the button stop 'glowing' when the mouse leaves it */
static gboolean on_close_button_hover_leave (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb)
{
    gboolean image_state;
    image_state = wnck_window_is_active(wb->win->controlwindow);

    gtk_image_set_from_pixbuf (wb->button[CLOSE_BUTTON]->image, wb->pixbufs[IMAGE_CLOSE][image_state]);
    return TRUE;
}


/* Makes the button 'glow' when the mouse enters it */
static gboolean on_close_button_hover_enter (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb)
{
    gtk_image_set_from_pixbuf (wb->button[CLOSE_BUTTON]->image, wb->pixbufs[IMAGE_CLOSE][IMAGE_PRELIGHT]);

    return TRUE;
}


static void on_refresh_item_activated (GtkMenuItem *refresh, WBPlugin *wb)
{
    wckbuttons_read (wb);
    init_theme(wb);
    reload_wnck (wb->win, wb->prefs->only_maximized, wb);
}


static void
wckbuttons_construct (XfcePanelPlugin *plugin)
{
    WBPlugin *wb;
    GtkWidget *refresh;

    /* setup transation domain */
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    /* create the plugin */
    wb = wckbuttons_new (plugin);

    /* add the ebox to the panel */
    gtk_container_add (GTK_CONTAINER (plugin), wb->ebox);

    /* show the panel's right-click menu on this ebox */
    xfce_panel_plugin_add_action_widget (plugin, wb->ebox);

    /* connect plugin signals */
    g_signal_connect (G_OBJECT (plugin), "free-data",
                    G_CALLBACK (wckbuttons_free), wb);

    g_signal_connect (G_OBJECT (plugin), "save",
                    G_CALLBACK (wckbuttons_save), wb);

    g_signal_connect (G_OBJECT (plugin), "size-changed",
                    G_CALLBACK (wckbuttons_size_changed), wb);

    g_signal_connect (G_OBJECT (plugin), "orientation-changed",
                    G_CALLBACK (wckbuttons_orientation_changed), wb);

    /* show the configure menu item and connect signal */
    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (G_OBJECT (plugin), "configure-plugin",
                    G_CALLBACK (wckbuttons_configure), wb);

    /* show the about menu item and connect signal */
    xfce_panel_plugin_menu_show_about (plugin);
    g_signal_connect (G_OBJECT (plugin), "about",
                    G_CALLBACK (wck_about), "wckbuttons-plugin");


    /* add custom menu items */
    refresh = show_refresh_item (plugin);
    g_signal_connect (G_OBJECT (refresh), "activate", G_CALLBACK (on_refresh_item_activated), wb);

    /* start tracking windows */
    wb->win = g_slice_new0 (WckUtils);
    init_wnck(wb->win, wb->prefs->only_maximized, wb);

    /* get theme */
    init_theme(wb);

    /* start tracking buttons events*/
    BUTTONS_SIGNALS_CONNECT(on_minimize, MINIMIZE_BUTTON);
    BUTTONS_SIGNALS_CONNECT(on_maximize, MAXIMIZE_BUTTON);
    BUTTONS_SIGNALS_CONNECT(on_close, CLOSE_BUTTON);
}


/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER (wckbuttons_construct);
