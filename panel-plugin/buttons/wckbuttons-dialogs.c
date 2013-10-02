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

#include <string.h>
#include <gtk/gtk.h>

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "wckbuttons.h"
#include "wckbuttons-dialogs.h"
#include "wckbuttons-dialogs_ui.h"

/* the website url */
#define PLUGIN_WEBSITE "http://goodies.xfce.org/projects/panel-plugins/xfce4-wckbuttons-plugin"

static void on_only_maximized_toggled(GtkRadioButton *only_maximized, WBPlugin *wb) {
    wb->prefs->only_maximized = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(only_maximized));
    initWnck(wb->win, wb->prefs->only_maximized, wb);
}

static void on_show_on_desktop_toggled(GtkToggleButton *show_on_desktop, WBPlugin *wb) {
    wb->prefs->show_on_desktop = gtk_toggle_button_get_active(show_on_desktop);
    initWnck(wb->win, wb->prefs->only_maximized, wb);
}

static GtkWidget * build_properties_area(WBPlugin *wb, const gchar *buffer, gsize length) {
    GError *error = NULL;
    GtkBuilder *builder;
    GObject *area = NULL;
    GtkRadioButton *only_maximized, *active_window;
    GtkToggleButton *show_on_desktop;

    builder = gtk_builder_new();
    if (gtk_builder_add_from_string(builder, buffer, length, &error)) {
        area = gtk_builder_get_object(builder, "alignment0");
        if (G_LIKELY (area != NULL)) {

            only_maximized = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "only_maximized"));
            active_window = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "active_window"));
            if (G_LIKELY (only_maximized != NULL)) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(only_maximized), wb->prefs->only_maximized);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(active_window), !wb->prefs->only_maximized);
                g_signal_connect(only_maximized, "toggled", G_CALLBACK(on_only_maximized_toggled), wb);
            } else {
                DBG("No widget with the name \"only_maximized\" found");
            }

            show_on_desktop = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "show_on_desktop"));
            if (G_LIKELY (show_on_desktop != NULL)) {
                gtk_toggle_button_set_active(show_on_desktop, wb->prefs->show_on_desktop);
                g_signal_connect(show_on_desktop, "toggled", G_CALLBACK(on_show_on_desktop_toggled), wb);
            } else {
                DBG("No widget with the name \"show_on_desktop\" found");
            }

            return GTK_WIDGET(area) ;
        } else {
            g_set_error_literal(&error, 0, 0, "No widget with the name \"contentarea\" found");
        }
    }

    g_critical("Faild to construct the builder for plugin %s-%d: %s.", xfce_panel_plugin_get_name (wb->plugin), xfce_panel_plugin_get_unique_id (wb->plugin), error->message);
    g_error_free(error);
    g_object_unref(G_OBJECT (builder) );

    return NULL ;
}

static void
wckbuttons_configure_response (GtkWidget    *dialog,
                           gint          response,
                           WBPlugin *wb)
{
    gboolean result;

    if (response == GTK_RESPONSE_HELP)
    {
        /* show help */
        result = g_spawn_command_line_async ("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

        if (G_UNLIKELY (result == FALSE))
            g_warning (_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
    }
    else
    {
        /* remove the dialog data from the plugin */
        g_object_set_data (G_OBJECT (wb->plugin), "dialog", NULL);

        /* unlock the panel menu */
        xfce_panel_plugin_unblock_menu (wb->plugin);

        /* save the plugin */
        wckbuttons_save (wb->plugin, wb);

        /* destroy the properties dialog */
        gtk_widget_destroy (dialog);
    }
}



void
wckbuttons_configure (XfcePanelPlugin *plugin,
                  WBPlugin    *wb)
{
    GtkWidget *dialog;
    GtkBuilder *builder;
    GtkWidget *content_area;
    GtkWidget *ca;

    /* block the plugin menu */
    xfce_panel_plugin_block_menu (plugin);

    /* create the dialog */
    dialog = xfce_titled_dialog_new_with_buttons (_("Windowck Buttons"),
                                                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
                                                GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                                                GTK_STOCK_HELP, GTK_RESPONSE_HELP,
                                                GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                                                NULL);

    /* center dialog on the screen */
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

    /* set dialog icon */
    gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-settings");

    /* link the dialog to the plugin, so we can destroy it when the plugin
    * is closed, but the dialog is still open */
    g_object_set_data (G_OBJECT (plugin), "dialog", dialog);

    /* connect the reponse signal to the dialog */
    g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK(wckbuttons_configure_response), wb);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog) );

    ca = build_properties_area(wb, wckbuttons_dialogs_ui, wckbuttons_dialogs_ui_length);
    if (G_LIKELY (ca != NULL))
        gtk_container_add(GTK_CONTAINER (content_area), ca);
    else
        DBG("Failed to create content area");

    /* show the entire dialog */
    gtk_widget_show (dialog);
}



void
wckbuttons_about (XfcePanelPlugin *plugin)
{
    /* about dialog code. you can use the GtkAboutDialog
    * or the XfceAboutInfo widget */
    GdkPixbuf *icon;
    const gchar *auth[] = { "Alessio Piccoli <alepic@geckoblu.net>", "Cedric Leporcq <cedl38@gmail.com>", "\nThis code is based on original 'Window Applets' code of Andrej Belcijan.\nSee http://gnome-look.org/content/show.php?content=103732 for details." };

    icon = xfce_panel_pixbuf_from_source("wckbuttons-plugin", NULL, 32);

    gtk_show_about_dialog(NULL, "logo", icon, "license", xfce_get_license_text(XFCE_LICENSE_TEXT_GPL), "version", PACKAGE_VERSION, "program-name", PACKAGE_NAME, "comments", _("Put the maximized window buttons on the panel."), "website", PLUGIN_WEBSITE, "copyright", _("Copyright (c) 2013\n"), "authors", auth, NULL );
    // TODO: add translators.

    if (icon)
        g_object_unref(G_OBJECT(icon) );
}
