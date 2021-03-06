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

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "windowck.h"
#include "windowck-dialogs.h"
#include "windowck-dialogs_ui.h"

/* the website url */
#define PLUGIN_WEBSITE "http://goodies.xfce.org/projects/panel-plugins/xfce4-windowck-plugin"

static void windowck_configure_response(GtkWidget *dialog, gint response, WindowckPlugin *wckp) {
    gboolean result;

    if (response == GTK_RESPONSE_HELP) {
        /* show help */
        result = g_spawn_command_line_async("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL );

        if (G_UNLIKELY(result == FALSE))
            g_warning(_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
    } else {
        /* remove the dialog data from the plugin */
        g_object_set_data(G_OBJECT(wckp->plugin), "dialog", NULL );

        /* unlock the panel menu */
        xfce_panel_plugin_unblock_menu(wckp->plugin);

        /* save the plugin */
        windowck_save(wckp->plugin, wckp);

        /* destroy the properties dialog */
        gtk_widget_destroy(dialog);
    }
}

static void on_titlesize_changed(GtkSpinButton *titlesize, WindowckPlugin *wckp) {
    gint size = gtk_spin_button_get_value(titlesize);
    wckp->prefs->title_size = size;
    gtk_label_set_width_chars(wckp->title, wckp->prefs->title_size);
}

static GtkWidget * build_properties_area(WindowckPlugin *wckp, const gchar *buffer, gsize length) {
    GError *error = NULL;
    GtkBuilder *builder;
    GObject *area = NULL;
    GtkSpinButton *titlesize;

    builder = gtk_builder_new();
    if (gtk_builder_add_from_string(builder, buffer, length, &error)) {
        area = gtk_builder_get_object(builder, "contentarea");
        if (G_LIKELY (area != NULL)) {

            titlesize = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "titlesize"));
            if (G_LIKELY (titlesize != NULL)) {
                gtk_spin_button_set_range(titlesize, 1, 999);
                gtk_spin_button_set_increments(titlesize, 1, 1);
                gtk_spin_button_set_value(titlesize, wckp->prefs->title_size);
                g_signal_connect(titlesize, "value-changed", G_CALLBACK(on_titlesize_changed), wckp);
            } else {
                DBG("No widget with the name \"titlesize\" found");
            }

            return GTK_WIDGET(area) ;
        } else {
            g_set_error_literal(&error, 0, 0, "No widget with the name \"contentarea\" found");
        }
    }

    g_critical("Faild to construct the builder for plugin %s-%d: %s.", xfce_panel_plugin_get_name (wckp->plugin), xfce_panel_plugin_get_unique_id (wckp->plugin), error->message);
    g_error_free(error);
    g_object_unref(G_OBJECT (builder) );

    return NULL ;
}

void windowck_configure(XfcePanelPlugin *plugin, WindowckPlugin *wckp) {
    GtkWidget *dialog;
    GtkBuilder *builder;
    GtkWidget *content_area;
    GtkWidget *ca;
    GObject *titlesize;

    /* block the plugin menu */
    xfce_panel_plugin_block_menu(plugin);

    /* create the dialog */
    dialog = xfce_titled_dialog_new_with_buttons(_("Window Controls"), GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR, GTK_STOCK_HELP, GTK_RESPONSE_HELP, GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL );

    /* center dialog on the screen */
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    /* set dialog icon */
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "xfce4-settings");

    /* link the dialog to the plugin, so we can destroy it when the plugin
     * is closed, but the dialog is still open */
    g_object_set_data(G_OBJECT(plugin), "dialog", dialog);

    /* connect the reponse signal to the dialog */
    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(windowck_configure_response), wckp);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog) );

    ca = build_properties_area(wckp, windowck_dialogs_ui, windowck_dialogs_ui_length);
    if (G_LIKELY (ca != NULL))
        gtk_container_add(GTK_CONTAINER (content_area), ca);
    else
        DBG("Failed to create content area");

    /* show the entire dialog */
    gtk_widget_show(dialog);
}

void windowck_about(XfcePanelPlugin *plugin) {
    /* about dialog code. you can use the GtkAboutDialog
     * or the XfceAboutInfo widget */
    GdkPixbuf *icon;
    const gchar *auth[] = { "Alessio Piccoli <alepic@geckoblu.net>", "\nThis code is heavily based on original 'Window Applets' code of Andrej Belcijan.\nSee http://gnome-look.org/content/show.php?content=103732 for details." };

    icon = xfce_panel_pixbuf_from_source("preferences-system-windows", NULL, 32);

    gtk_show_about_dialog(NULL, "logo", icon, "license", xfce_get_license_text(XFCE_LICENSE_TEXT_GPL), "version", PACKAGE_VERSION, "program-name", PACKAGE_NAME, "comments", _("Put the maximized window title on the panel."), "website", PLUGIN_WEBSITE, "copyright", _("Copyright (c) 2013\n"), "authors", auth, NULL );
    // TODO: add translators.

    if (icon)
        g_object_unref(G_OBJECT(icon) );
}
