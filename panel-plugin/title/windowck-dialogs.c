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
#include "windowck-title.h"
#include "windowck-dialogs.h"
#include "windowck-dialogs_ui.h"

/* the website url */
#define PLUGIN_WEBSITE "http://goodies.xfce.org/projects/panel-plugins/xfce4-windowck-plugin"
#define TITLE_SIZE_MIN 3

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
    wckp->prefs->title_size = gtk_spin_button_get_value(titlesize);
    resizeTitle(wckp);
}

static void on_size_mode_changed (GtkComboBox *size_mode, WindowckPlugin *wckp) {
    gint id;
    GtkSpinButton *titlesize;
    GtkLabel *width_unit;

    id = gtk_combo_box_get_active(size_mode);
    
    if (id < 0 || id > 2) {
      g_critical ("Trying to set a default size but got an invalid item");
      return;
    }

    titlesize = g_object_get_data(G_OBJECT(wckp->plugin), "titlesize");
    width_unit = g_object_get_data(G_OBJECT(wckp->plugin), "width_unit");

    if (id == 0) {
        wckp->prefs->size_mode = SHRINK;
        xfce_panel_plugin_set_shrink (wckp->plugin, FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(titlesize), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(width_unit), TRUE);
    }
    else if (id == 1) {
        wckp->prefs->size_mode = FIXE;
        xfce_panel_plugin_set_shrink (wckp->plugin, TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(titlesize), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(width_unit), TRUE);
    }
    else if (id == 2) {
        wckp->prefs->size_mode = EXPAND;
        xfce_panel_plugin_set_shrink (wckp->plugin, TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(titlesize), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(width_unit), FALSE);
    }

    /* dynamic resizing */
    /* d'ont work for title shrinking -> need to restart the applet */
    resizeTitle(wckp);
}

static void on_custom_font_toggled(GtkToggleButton *custom_font, WindowckPlugin *wckp) {
    GtkFontButton *title_font;

    title_font= g_object_get_data(G_OBJECT(wckp->plugin), "title_font");

    wckp->prefs->custom_font = gtk_toggle_button_get_active(custom_font);
    if (wckp->prefs->custom_font)
        gtk_widget_set_sensitive(GTK_WIDGET(title_font), TRUE );
    else
        gtk_widget_set_sensitive(GTK_WIDGET(title_font), FALSE );
}

static void on_title_font_font_set(GtkFontButton *title_font, WindowckPlugin *wckp) {
    wckp->prefs->title_font = g_strdup(gtk_font_button_get_font_name(title_font));
    updateFont(wckp);
}

static void on_title_alignment_changed (GtkComboBox *title_alignment, WindowckPlugin *wckp) {
    gint id;

    id = gtk_combo_box_get_active(title_alignment);

    if (id < 0 || id > 2) {
        g_critical ("Trying to set a default size but got an invalid item");
        return;
    }

    if (id == 0) {
        wckp->prefs->title_alignment = LEFT;
    }
    else if (id == 1) {
        wckp->prefs->title_alignment = CENTER;
    }
    else if (id == 2) {
        wckp->prefs->title_alignment = RIGHT;
    }

    gtk_misc_set_alignment(GTK_MISC(wckp->title), wckp->prefs->title_alignment / 10.0, 0.5);
}

static void on_title_padding_changed(GtkSpinButton *title_padding, WindowckPlugin *wckp) {
    wckp->prefs->title_padding = gtk_spin_button_get_value(title_padding);
    gtk_alignment_set_padding(GTK_ALIGNMENT(wckp->alignment), 0, 0, wckp->prefs->title_padding, wckp->prefs->title_padding);
}

static GtkWidget * build_properties_area(WindowckPlugin *wckp, const gchar *buffer, gsize length) {
    GError *error = NULL;
    GtkBuilder *builder;
    GObject *area = NULL;
    GtkSpinButton *titlesize, *title_padding;
    GtkComboBox *size_mode, *title_alignment;
    GtkToggleButton *custom_font;
    GtkFontButton *title_font;
    GtkLabel *width_unit;

    builder = gtk_builder_new();
    if (gtk_builder_add_from_string(builder, buffer, length, &error)) {
        area = gtk_builder_get_object(builder, "contentarea");
        if (G_LIKELY (area != NULL)) {

            titlesize = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "titlesize"));
            width_unit = GTK_LABEL(gtk_builder_get_object(builder, "width_unit"));
            g_object_set_data(G_OBJECT(wckp->plugin), "titlesize", titlesize);
            g_object_set_data(G_OBJECT(wckp->plugin), "width_unit", width_unit);
            if (G_LIKELY (titlesize != NULL)) {
                gtk_spin_button_set_range(titlesize, TITLE_SIZE_MIN, TITLE_SIZE_MAX);
                gtk_spin_button_set_increments(titlesize, 1, 1);
                gtk_spin_button_set_value(titlesize, wckp->prefs->title_size);
                g_signal_connect(titlesize, "value-changed", G_CALLBACK(on_titlesize_changed), wckp);
            } else {
                DBG("No widget with the name \"titlesize\" found");
            }

            custom_font = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "custom_font"));
            title_font = GTK_FONT_BUTTON(gtk_builder_get_object(builder, "title_font"));
            g_object_set_data(G_OBJECT(wckp->plugin), "title_font", title_font);

            if (!wckp->prefs->custom_font)
            gtk_widget_set_sensitive(GTK_WIDGET(title_font), FALSE );

            if (G_LIKELY (custom_font != NULL)) {
                gtk_toggle_button_set_active(custom_font, wckp->prefs->custom_font);
                g_signal_connect(custom_font, "toggled", G_CALLBACK(on_custom_font_toggled), wckp);
            } else {
                DBG("No widget with the name \"custom_font\" found");
            }

            if (G_LIKELY (title_font != NULL)) {
                gtk_font_button_set_font_name(title_font, wckp->prefs->title_font);
                g_signal_connect(title_font, "font-set", G_CALLBACK(on_title_font_font_set), wckp);
            } else {
                DBG("No widget with the name \"title_font\" found");
            }

            title_alignment = GTK_COMBO_BOX(gtk_builder_get_object(builder, "title_alignment"));
            if (G_LIKELY (title_alignment != NULL)) {
                /* set active item */
                if ( wckp->prefs->title_alignment == LEFT ) {
                    gtk_combo_box_set_active(title_alignment, 0);
                }
                else if( wckp->prefs->title_alignment == CENTER ) {
                    gtk_combo_box_set_active(title_alignment, 1);
                }
                else if( wckp->prefs->title_alignment == RIGHT ) {
                    gtk_combo_box_set_active(title_alignment, 2);
                }
                g_signal_connect(title_alignment, "changed", G_CALLBACK(on_title_alignment_changed), wckp);
            } else {
                DBG("No widget with the name \"title_alignment\" found");
            }

            title_padding = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "title_padding"));
            if (G_LIKELY (title_padding != NULL)) {
                gtk_spin_button_set_range(title_padding, 0, 99);
                gtk_spin_button_set_increments(title_padding, 1, 1);
                gtk_spin_button_set_value(title_padding, wckp->prefs->title_padding);
                g_signal_connect(title_padding, "value-changed", G_CALLBACK(on_title_padding_changed), wckp);
            } else {
                DBG("No widget with the name \"title_padding\" found");
            }

            size_mode = GTK_COMBO_BOX(gtk_builder_get_object(builder, "size_mode"));
            if (G_LIKELY (size_mode != NULL)) {
                /* set active item */
                if ( wckp->prefs->size_mode == SHRINK ) {
                    gtk_combo_box_set_active(size_mode, 0);
                }
                else if( wckp->prefs->size_mode == FIXE ) {
                    gtk_combo_box_set_active(size_mode, 1);
                }
                else if( wckp->prefs->size_mode == EXPAND ) {
                    gtk_combo_box_set_active(size_mode, 2);
                    gtk_widget_set_sensitive(GTK_WIDGET(titlesize), FALSE);
                    gtk_widget_set_sensitive(GTK_WIDGET(width_unit), FALSE);
                }

                g_signal_connect(size_mode, "changed", G_CALLBACK(on_size_mode_changed), wckp);
            } else {
                DBG("No widget with the name \"size_mode\" found");
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
