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
#include "theme.h"

/* default settings */
#define DEFAULT_ONLY_MAXIMIZED TRUE
#define DEFAULT_SHOW_ON_DESKTOP FALSE
#define DEFAULT_BUTTON_LAYOUT "HMC"
#define DEFAULT_SETTING2 1


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
      if (wb->prefs->button_layout)
        xfce_rc_write_entry    (rc, "button_layout", wb->prefs->button_layout);

      if (wb->prefs->theme)
        xfce_rc_write_entry    (rc, "theme", wb->prefs->theme);

      xfce_rc_write_int_entry  (rc, "setting2", wb->setting2);

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
  wb->prefs = g_slice_new0(WCKPreferences);

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
        button_layout = xfce_rc_read_entry (rc, "button_layout", DEFAULT_BUTTON_LAYOUT);
        wb->prefs->button_layout = g_strdup (button_layout);
        theme = xfce_rc_read_entry (rc, "theme", DEFAULT_THEME);
        wb->prefs->theme = g_strdup (theme);

          wb->setting2 = xfce_rc_read_int_entry (rc, "setting2", DEFAULT_SETTING2);

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
    wb->prefs->button_layout = g_strdup (DEFAULT_BUTTON_LAYOUT);
    wb->prefs->theme = g_strdup (DEFAULT_THEME);
  wb->setting2 = DEFAULT_SETTING2;
}

WindowButton **createButtons (WBPlugin *wb) {
  WindowButton **button = g_new(WindowButton*, BUTTONS);
  gint i;

  for (i=0; i<BUTTONS; i++) {
      button[i] = g_new0 (WindowButton, 1);
      button[i]->eventbox = GTK_EVENT_BOX (gtk_event_box_new());
      button[i]->image = GTK_IMAGE (gtk_image_new());

      // Woohooo! This line eliminates MatePanelPlugin borders - no more workarounds!
      gtk_widget_set_can_focus (GTK_WIDGET(button[i]->eventbox), TRUE);

      gtk_container_add (GTK_CONTAINER (button[i]->eventbox), GTK_WIDGET(button[i]->image));
      gtk_event_box_set_visible_window (button[i]->eventbox, FALSE);

      /* Add hover events to eventboxes */
      gtk_widget_add_events (GTK_WIDGET (button[i]->eventbox), GDK_ENTER_NOTIFY_MASK); //add the "enter" signal
      gtk_widget_add_events (GTK_WIDGET (button[i]->eventbox), GDK_LEAVE_NOTIFY_MASK); //add the "leave" signal
  }
  return button;
}

static int
getButtonFromLetter (char chr)
{

    TRACE ("entering getButtonFromLetter");

    switch (chr)
    {
        case 'H':
                return MINIMIZE_BUTTON;
        case 'M':
                return MAXIMIZE_BUTTON;
        case 'C':
                return CLOSE_BUTTON;
        default:
            return -1;
    }
}

// Places our buttons in correct order
void placeButtons(WBPlugin *wb) {
  gint i, button;

    for (i = 0; i < strlen (wb->prefs->button_layout); i++)
    {
        button= getButtonFromLetter (wb->prefs->button_layout[i]);
        if (button >= 0)
        {
            wb->button[button]->visible = TRUE;
            gtk_box_pack_start (GTK_BOX (wb->hvbox), GTK_WIDGET(wb->button[button]->eventbox), TRUE, TRUE, 0);
            gtk_widget_show_all(GTK_WIDGET(wb->button[button]->eventbox));
        }
        else {
            wb->button[button]->visible = FALSE;
        }
    }
}

static WBPlugin *
wckbuttons_new (XfcePanelPlugin *plugin)
{
    WBPlugin   *wb;
  GtkOrientation  orientation;
  GtkWidget      *label;

  /* allocate memory for the plugin structure */
  wb = g_slice_new0 (WBPlugin);

  /* pointer to plugin */
  wb->plugin = plugin;

  /* read the user settings */
  wckbuttons_read (wb);

  /* load images */
    loadTheme(wb);

  /* get the current orientation */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* create some panel widgets */
  wb->ebox = gtk_event_box_new ();
  wb->hvbox = xfce_hvbox_new (orientation, FALSE, 2);

  /* create buttons */
  wb->button = createButtons (wb);

  /* place buttons accordingly to button_layout pref */
  placeButtons(wb);

  gtk_widget_show (wb->ebox);
  gtk_widget_show (wb->hvbox);
  gtk_container_add (GTK_CONTAINER (wb->ebox), wb->hvbox);

  return wb;
}



static void
wckbuttons_free (XfcePanelPlugin *plugin, WBPlugin    *wb)
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
  g_slice_free(WCKPreferences, wb->prefs);
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

setMaximizeButtonImage (WBPlugin *wb, gushort image_state) {
    if (wb->win->controlwindow && wnck_window_is_maximized(wb->win->controlwindow)) {
        gtk_image_set_from_pixbuf (wb->button[MAXIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_UNMAXIMIZE][image_state]);
    } else {
        gtk_image_set_from_pixbuf (wb->button[MAXIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MAXIMIZE][image_state]);
    }
}

void on_wck_state_changed (WnckWindow *controlwindow, WBPlugin *wb) {
    gushort image_state;

    if (controlwindow && (wnck_window_is_active(controlwindow)))
        image_state = 1;
    else
        image_state = 0;

    /* update buttons images */

    gtk_image_set_from_pixbuf (wb->button[MINIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MINIMIZE][image_state]);

    setMaximizeButtonImage (wb, image_state);

    gtk_image_set_from_pixbuf (wb->button[CLOSE_BUTTON]->image, wb->pixbufs[IMAGE_CLOSE][image_state]);
}

void on_control_window_changed (WnckWindow *controlwindow, WnckWindow *previous, WBPlugin *wb) {

    if (controlwindow
        && (wnck_window_get_window_type (controlwindow) != WNCK_WINDOW_DESKTOP)) {
        gtk_widget_set_sensitive(GTK_WIDGET(wb->hvbox), TRUE);
        on_wck_state_changed (controlwindow, wb);
        if (!gtk_widget_get_visible(GTK_WIDGET(wb->hvbox)))
            gtk_widget_show_all(GTK_WIDGET(wb->hvbox));
    }
    else if (wb->prefs->show_on_desktop) {
        gtk_widget_set_sensitive(GTK_WIDGET(wb->hvbox), FALSE);
        on_wck_state_changed (controlwindow, wb);
        if (!gtk_widget_get_visible(GTK_WIDGET(wb->hvbox)))
            gtk_widget_show_all(GTK_WIDGET(wb->hvbox));
    }
    else {
        if (gtk_widget_get_visible(GTK_WIDGET(wb->hvbox)))
            gtk_widget_hide_all(GTK_WIDGET(wb->hvbox));
    }
}

/* Called when we release the click on a button */
static gboolean on_minimize_button_release (GtkWidget *event_box,
                               GdkEventButton *event,
                               WBPlugin *wb) {

    if (event->button != 1) return FALSE;

    wnck_window_minimize(wb->win->controlwindow);

	return TRUE;
}

/* Called when we click on a button */
static gboolean on_minimize_button_pressed (GtkWidget *event_box,
                             GdkEventButton *event,
                             WBPlugin *wb) {

	if (event->button != 1) return FALSE;

	gtk_image_set_from_pixbuf (wb->button[MINIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MINIMIZE][IMAGE_PRESSED]);

	return TRUE;
}

/* Makes the button stop 'glowing' when the mouse leaves it */
static gboolean on_minimize_button_hover_leave (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb) {
    gboolean image_state;
    image_state = wnck_window_is_active(wb->win->controlwindow);

	gtk_image_set_from_pixbuf (wb->button[MINIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MINIMIZE][image_state]);
	return TRUE;
}

/* Makes the button 'glow' when the mouse enters it */
static gboolean on_minimize_button_hover_enter (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb) {

	gtk_image_set_from_pixbuf (wb->button[MINIMIZE_BUTTON]->image, wb->pixbufs[IMAGE_MINIMIZE][IMAGE_PRELIGHT]);

	return TRUE;
}

/* Called when we release the click on a button */
static gboolean on_maximize_button_release (GtkWidget *event_box,
                               GdkEventButton *event,
                               WBPlugin *wb) {

    if (event->button != 1) return FALSE;

    toggleMaximize(wb->win);

	return TRUE;
}

/* Called when we click on a button */
static gboolean on_maximize_button_pressed (GtkWidget *event_box,
                             GdkEventButton *event,
                             WBPlugin *wb) {

	if (event->button != 1) return FALSE;

    setMaximizeButtonImage (wb, IMAGE_PRESSED);

	return TRUE;
}

/* Makes the button stop 'glowing' when the mouse leaves it */
static gboolean on_maximize_button_hover_leave (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb) {

    if (wb->win->controlwindow) {
        setMaximizeButtonImage (wb, wnck_window_is_active(wb->win->controlwindow));
    }

	return TRUE;
}

/* Makes the button 'glow' when the mouse enters it */
static gboolean on_maximize_button_hover_enter (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb) {

    setMaximizeButtonImage (wb, IMAGE_PRELIGHT);

	return TRUE;
}

/* Called when we release the click on a button */
static gboolean on_close_button_release (GtkWidget *event_box,
                               GdkEventButton *event,
                               WBPlugin *wb) {

    if (event->button != 1) return FALSE;

    wnck_window_close(wb->win->controlwindow, GDK_CURRENT_TIME);

	return TRUE;
}

/* Called when we click on a button */
static gboolean on_close_button_pressed (GtkWidget *event_box,
                             GdkEventButton *event,
                             WBPlugin *wb) {

	if (event->button != 1) return FALSE;

	gtk_image_set_from_pixbuf (wb->button[CLOSE_BUTTON]->image, wb->pixbufs[IMAGE_CLOSE][IMAGE_PRESSED]);

	return TRUE;
}

/* Makes the button stop 'glowing' when the mouse leaves it */
static gboolean on_close_button_hover_leave (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb) {
    gboolean image_state;
    image_state = wnck_window_is_active(wb->win->controlwindow);

	gtk_image_set_from_pixbuf (wb->button[CLOSE_BUTTON]->image, wb->pixbufs[IMAGE_CLOSE][image_state]);
	return TRUE;
}

/* Makes the button 'glow' when the mouse enters it */
static gboolean on_close_button_hover_enter (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBPlugin *wb) {

	gtk_image_set_from_pixbuf (wb->button[CLOSE_BUTTON]->image, wb->pixbufs[IMAGE_CLOSE][IMAGE_PRELIGHT]);

	return TRUE;
}

void initButtonsEvents (WBPlugin *wb) {
    /* Connect buttons to their respective callback functions */
    if (wb->button[MINIMIZE_BUTTON]->visible) {
        g_signal_connect (G_OBJECT (wb->button[MINIMIZE_BUTTON]->eventbox), "button-press-event", G_CALLBACK (on_minimize_button_pressed), wb);
        g_signal_connect (G_OBJECT (wb->button[MINIMIZE_BUTTON]->eventbox), "button-release-event", G_CALLBACK (on_minimize_button_release), wb);
        g_signal_connect (G_OBJECT (wb->button[MINIMIZE_BUTTON]->eventbox), "enter-notify-event", G_CALLBACK (on_minimize_button_hover_enter), wb);
        g_signal_connect (G_OBJECT (wb->button[MINIMIZE_BUTTON]->eventbox), "leave-notify-event", G_CALLBACK (on_minimize_button_hover_leave), wb);
    }
    if (wb->button[MAXIMIZE_BUTTON]->visible) {
        g_signal_connect (G_OBJECT (wb->button[MAXIMIZE_BUTTON]->eventbox), "button-press-event", G_CALLBACK (on_maximize_button_pressed), wb);
        g_signal_connect (G_OBJECT (wb->button[MAXIMIZE_BUTTON]->eventbox), "button-release-event", G_CALLBACK (on_maximize_button_release), wb);
        g_signal_connect (G_OBJECT (wb->button[MAXIMIZE_BUTTON]->eventbox), "enter-notify-event", G_CALLBACK (on_maximize_button_hover_enter), wb);
        g_signal_connect (G_OBJECT (wb->button[MAXIMIZE_BUTTON]->eventbox), "leave-notify-event", G_CALLBACK (on_maximize_button_hover_leave), wb);
    }
    if (wb->button[MAXIMIZE_BUTTON]->visible) {
        g_signal_connect (G_OBJECT (wb->button[CLOSE_BUTTON]->eventbox), "button-press-event", G_CALLBACK (on_close_button_pressed), wb);
        g_signal_connect (G_OBJECT (wb->button[CLOSE_BUTTON]->eventbox), "button-release-event", G_CALLBACK (on_close_button_release), wb);
        g_signal_connect (G_OBJECT (wb->button[CLOSE_BUTTON]->eventbox), "enter-notify-event", G_CALLBACK (on_close_button_hover_enter), wb);
        g_signal_connect (G_OBJECT (wb->button[CLOSE_BUTTON]->eventbox), "leave-notify-event", G_CALLBACK (on_close_button_hover_leave), wb);
    }
}

static void
wckbuttons_construct (XfcePanelPlugin *plugin)
{
    WBPlugin *wb;

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
                    G_CALLBACK (wckbuttons_about), NULL);

    /* start tracking windows */
    wb->win = g_slice_new0 (WckUtils);
    initWnck(wb->win, wb->prefs->only_maximized, wb);

    /* start tracking buttons events*/
    initButtonsEvents(wb);
}

/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER (wckbuttons_construct);
