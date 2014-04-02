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
 *  Copyright (C) 2013 Alessio Piccoli <alepic@geckoblu.net>
 *                     Cedric Leporcq  <cedl38@gmail.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <common/theme.h>

#include "windowck.h"
#include "windowck-title.h"
#include "windowck-dialogs.h"
#include "windowck-dialogs_ui.h"

/* the website url */
#define PLUGIN_WEBSITE "http://goodies.xfce.org/projects/panel-plugins/xfce4-windowck-plugin"
#define TITLE_SIZE_MIN 3


static void on_only_maximized_toggled(GtkRadioButton *only_maximized, WindowckPlugin *wckp)
{
    wckp->prefs->only_maximized = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(only_maximized));
    reload_wnck_title (wckp);
}


static void on_show_on_desktop_toggled(GtkToggleButton *show_on_desktop, WindowckPlugin *wckp)
{
    wckp->prefs->show_on_desktop = gtk_toggle_button_get_active(show_on_desktop);

    if (wckp->icon->symbol)
        gtk_widget_set_sensitive (wckp->icon->symbol, TRUE);
    reload_wnck_title (wckp);
}


static void on_titlesize_changed(GtkSpinButton *titlesize, WindowckPlugin *wckp)
{
    wckp->prefs->title_size = gtk_spin_button_get_value(titlesize);
    resize_title(wckp);
}


static void on_size_mode_changed (GtkComboBox *size_mode, WindowckPlugin *wckp)
{
    gint id;
    GtkWidget *titlesize, *width_unit;

    id = gtk_combo_box_get_active (size_mode);

    if (id < 0 || id > 2)
    {
      g_critical ("Trying to set a default size but got an invalid item");
      return;
    }

    titlesize = GTK_WIDGET (gtk_builder_get_object (wckp->prefs->builder, "titlesize"));
    width_unit = GTK_WIDGET (gtk_builder_get_object (wckp->prefs->builder, "width_unit"));

    if (id == 0)
    {
        wckp->prefs->size_mode = SHRINK;
        xfce_panel_plugin_set_shrink (wckp->plugin, FALSE);
        gtk_widget_set_sensitive (titlesize, TRUE);
        gtk_widget_set_sensitive (width_unit, TRUE);
    }
    else if (id == 1)
    {
        wckp->prefs->size_mode = FIXE;
        xfce_panel_plugin_set_shrink (wckp->plugin, TRUE);
        gtk_widget_set_sensitive(titlesize, TRUE);
        gtk_widget_set_sensitive(width_unit, TRUE);
    }
    else if (id == 2)
    {
        wckp->prefs->size_mode = EXPAND;
        xfce_panel_plugin_set_shrink (wckp->plugin, TRUE);
        gtk_widget_set_sensitive (titlesize, FALSE);
        gtk_widget_set_sensitive (width_unit, FALSE);
    }

    /* dynamic resizing */
    /* don't work for title shrinking -> need to restart the applet */
    // TODO: make it working...
    resize_title(wckp);
}


static void on_full_name_toggled(GtkToggleButton *full_name, WindowckPlugin *wckp)
{
    wckp->prefs->full_name = gtk_toggle_button_get_active(full_name);
    on_wck_state_changed (wckp->win->controlwindow, wckp);
}


static void on_two_lines_toggled(GtkToggleButton *two_lines, WindowckPlugin *wckp)
{
    GtkWidget *sync_wm_font, *subtitle_font, *subtitle_font_label;

    sync_wm_font = GTK_WIDGET(gtk_builder_get_object(wckp->prefs->builder, "sync_wm_font"));
    subtitle_font = GTK_WIDGET(gtk_builder_get_object(wckp->prefs->builder, "subtitle_font"));
    subtitle_font_label = GTK_WIDGET(gtk_builder_get_object(wckp->prefs->builder, "subtitle_font_label"));
    wckp->prefs->two_lines = gtk_toggle_button_get_active(two_lines);

    on_wck_state_changed (wckp->win->controlwindow, wckp);

    if (wckp->prefs->two_lines)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sync_wm_font), FALSE);
	}

    gtk_widget_set_sensitive (subtitle_font, wckp->prefs->two_lines);
    gtk_widget_set_sensitive (subtitle_font_label, wckp->prefs->two_lines);
    gtk_widget_set_sensitive (sync_wm_font, !wckp->prefs->two_lines);
}


static void on_show_app_icon_toggled(GtkToggleButton *show_app_icon, WindowckPlugin *wckp)
{
    wckp->prefs->show_app_icon = gtk_toggle_button_get_active(show_app_icon);

    create_symbol (wckp->prefs->show_app_icon, wckp);

    if (!wckp->prefs->show_app_icon)
        wck_signal_handler_disconnect (G_OBJECT(wckp->win->controlwindow), wckp->cih);

    on_wck_state_changed (wckp->win->controlwindow, wckp);
}


static void on_icon_on_right_toggled(GtkToggleButton *icon_on_right, WindowckPlugin *wckp)
{
    wckp->prefs->icon_on_right = gtk_toggle_button_get_active(icon_on_right);

    if (wckp->prefs->icon_on_right)
        gtk_box_reorder_child (GTK_BOX (wckp->hvbox), GTK_WIDGET(wckp->icon->eventbox), 1);
    else
        gtk_box_reorder_child (GTK_BOX (wckp->hvbox), GTK_WIDGET(wckp->icon->eventbox), 0);
}


static void on_show_window_menu_toggled(GtkToggleButton *show_window_menu, WindowckPlugin *wckp)
{
    GtkWidget *show_app_icon;
    GtkWidget *icon_on_right;

    wckp->prefs->show_window_menu = gtk_toggle_button_get_active(show_window_menu);
    show_app_icon = GTK_WIDGET(gtk_builder_get_object(wckp->prefs->builder, "show_app_icon"));
    icon_on_right = GTK_WIDGET(gtk_builder_get_object(wckp->prefs->builder, "icon_on_right"));

    create_symbol (wckp->prefs->show_app_icon, wckp);

    if (wckp->prefs->show_window_menu)
    {
        gtk_widget_set_sensitive (show_app_icon, TRUE);
        gtk_widget_set_sensitive (icon_on_right, TRUE);
        on_wck_state_changed (wckp->win->controlwindow, wckp);
    }
    else
    {
        gtk_widget_set_sensitive (show_app_icon, FALSE);
        gtk_widget_set_sensitive (icon_on_right, FALSE);

        if (wckp->prefs->show_app_icon)
            wck_signal_handler_disconnect (G_OBJECT(wckp->win->controlwindow), wckp->cih);
    }
}


static void on_sync_wm_font_toggled(GtkToggleButton *sync_wm_font, WindowckPlugin *wckp)
{
    GtkFontButton *title_font;

    wckp->prefs->sync_wm_font = gtk_toggle_button_get_active (sync_wm_font);
    init_title (wckp);
    title_font = GTK_FONT_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "title_font"));
    gtk_font_button_set_font_name(title_font, wckp->prefs->title_font);
}


static void on_title_font_set(GtkFontButton *title_font, WindowckPlugin *wckp)
{
    PangoFontDescription *font;

    wckp->prefs->title_font = g_strdup(gtk_font_button_get_font_name(title_font));

    font = pango_font_description_from_string(wckp->prefs->title_font);
    gtk_widget_modify_font(GTK_WIDGET(wckp->title), font);
    pango_font_description_free(font);

    if (wckp->prefs->sync_wm_font)
        xfconf_channel_set_string (wckp->wm_channel, "/general/title_font", wckp->prefs->title_font);
}


static void on_subtitle_font_set(GtkFontButton *subtitle_font, WindowckPlugin *wckp)
{
    PangoFontDescription *font;

    wckp->prefs->subtitle_font = g_strdup(gtk_font_button_get_font_name(subtitle_font));

    font = pango_font_description_from_string(wckp->prefs->subtitle_font);
    gtk_widget_modify_font(GTK_WIDGET(wckp->title), font);
    pango_font_description_free(font);
}


static void on_title_alignment_changed (GtkComboBox *title_alignment, WindowckPlugin *wckp)
{
    gint id;

    id = gtk_combo_box_get_active(title_alignment);

    if (id < 0 || id > 2)
    {
        g_critical ("Trying to set a default size but got an invalid item");
        return;
    }

    if (id == 0)
    {
        wckp->prefs->title_alignment = LEFT;
    }
    else if (id == 1)
    {
        wckp->prefs->title_alignment = CENTER;
    }
    else if (id == 2)
    {
        wckp->prefs->title_alignment = RIGHT;
    }

    gtk_misc_set_alignment(GTK_MISC(wckp->title), wckp->prefs->title_alignment / 10.0, 0.5);
    on_wck_state_changed (wckp->win->controlwindow, wckp);
}


static void on_title_padding_changed(GtkSpinButton *title_padding, WindowckPlugin *wckp)
{
    wckp->prefs->title_padding = gtk_spin_button_get_value(title_padding);
    gtk_alignment_set_padding(GTK_ALIGNMENT(wckp->alignment), ICON_PADDING, ICON_PADDING, wckp->prefs->title_padding, wckp->prefs->title_padding);
    gtk_box_set_spacing (GTK_BOX(wckp->hvbox), wckp->prefs->title_padding);
}


static GtkWidget * build_properties_area(WindowckPlugin *wckp, const gchar *buffer, gsize length)
{
    GError *error = NULL;
    GObject *area = NULL;
    GtkSpinButton *titlesize, *title_padding;
    GtkComboBox *size_mode, *title_alignment;
    GtkToggleButton *sync_wm_font;
    GtkRadioButton *only_maximized, *active_window;
    GtkToggleButton *show_on_desktop, *full_name, *two_lines;
    GtkToggleButton *show_app_icon, *icon_on_right, *show_window_menu;
    GtkFontButton *title_font, *subtitle_font;
    GtkWidget *width_unit, *subtitle_font_label;

    wckp->prefs->builder = gtk_builder_new();

    if (gtk_builder_add_from_string(wckp->prefs->builder, buffer, length, &error)) {
        area = gtk_builder_get_object(wckp->prefs->builder, "alignment0");

        if (G_LIKELY (area != NULL))
        {
            only_maximized = GTK_RADIO_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "only_maximized"));
            active_window = GTK_RADIO_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "active_window"));

            if (G_LIKELY (only_maximized != NULL))
            {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(only_maximized), wckp->prefs->only_maximized);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(active_window), !wckp->prefs->only_maximized);
                g_signal_connect(only_maximized, "toggled", G_CALLBACK(on_only_maximized_toggled), wckp);
            }
            else {
                DBG("No widget with the name \"only_maximized\" found");
            }

            show_on_desktop = GTK_TOGGLE_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "show_on_desktop"));

            if (G_LIKELY (show_on_desktop != NULL)) {
                gtk_toggle_button_set_active(show_on_desktop, wckp->prefs->show_on_desktop);
                g_signal_connect(show_on_desktop, "toggled", G_CALLBACK(on_show_on_desktop_toggled), wckp);
            }
            else {
                DBG("No widget with the name \"show_on_desktop\" found");
            }

            full_name = GTK_TOGGLE_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "full_name"));

            if (G_LIKELY (full_name != NULL))
            {
                gtk_toggle_button_set_active(full_name, wckp->prefs->full_name);
                g_signal_connect(full_name, "toggled", G_CALLBACK(on_full_name_toggled), wckp);
            }
            else {
                DBG("No widget with the name \"full_name\" found");
            }

            two_lines = GTK_TOGGLE_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "two_lines"));

            if (G_LIKELY (two_lines != NULL))
            {
                gtk_toggle_button_set_active(two_lines, wckp->prefs->two_lines);
                g_signal_connect(two_lines, "toggled", G_CALLBACK(on_two_lines_toggled), wckp);
            }
            else {
                DBG("No widget with the name \"two_lines\" found");
            }

            show_app_icon = GTK_TOGGLE_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "show_app_icon"));

            if (G_LIKELY (show_app_icon != NULL))
            {
                gtk_toggle_button_set_active(show_app_icon, wckp->prefs->show_app_icon);
                g_signal_connect(show_app_icon, "toggled", G_CALLBACK(on_show_app_icon_toggled), wckp);
            }
            else {
                DBG("No widget with the name \"show_app_icon\" found");
            }

            icon_on_right = GTK_TOGGLE_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "icon_on_right"));

            if (G_LIKELY (icon_on_right != NULL))
            {
                gtk_toggle_button_set_active(icon_on_right, wckp->prefs->icon_on_right);
                g_signal_connect(icon_on_right, "toggled", G_CALLBACK(on_icon_on_right_toggled), wckp);
            }
            else {
                DBG("No widget with the name \"icon_on_right\" found");
            }

            show_window_menu = GTK_TOGGLE_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "show_window_menu"));

            if (G_LIKELY (show_window_menu != NULL))
            {
                gtk_toggle_button_set_active(show_window_menu, wckp->prefs->show_window_menu);
                g_signal_connect(show_window_menu, "toggled", G_CALLBACK(on_show_window_menu_toggled), wckp);
            }
            else {
                DBG("No widget with the name \"show_window_menu\" found");
            }

            titlesize = GTK_SPIN_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "titlesize"));
            width_unit = GTK_WIDGET(gtk_builder_get_object(wckp->prefs->builder, "width_unit"));

            if (G_LIKELY (titlesize != NULL))
            {
                gtk_spin_button_set_range(titlesize, TITLE_SIZE_MIN, TITLE_SIZE_MAX);
                gtk_spin_button_set_increments(titlesize, 1, 1);
                gtk_spin_button_set_value(titlesize, wckp->prefs->title_size);
                g_signal_connect(titlesize, "value-changed", G_CALLBACK(on_titlesize_changed), wckp);
            }
            else {
                DBG("No widget with the name \"titlesize\" found");
            }

            sync_wm_font = GTK_TOGGLE_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "sync_wm_font"));
            title_font = GTK_FONT_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "title_font"));

            if (G_LIKELY (sync_wm_font != NULL))
            {
                if (wckp->wm_channel)
                {
                    gtk_toggle_button_set_active(sync_wm_font, wckp->prefs->sync_wm_font);
                    g_signal_connect(sync_wm_font, "toggled", G_CALLBACK(on_sync_wm_font_toggled), wckp);
                }
                else {
                    gtk_widget_set_sensitive (GTK_WIDGET(sync_wm_font), FALSE);
                }
            }
            else {
                DBG("No widget with the name \"sync_wm_font\" found");
            }

            if (G_LIKELY (title_font != NULL))
            {
                gtk_font_button_set_font_name(title_font, wckp->prefs->title_font);
                g_signal_connect(title_font, "font-set", G_CALLBACK(on_title_font_set), wckp);
            }
            else {
                DBG("No widget with the name \"title_font\" found");
            }

            subtitle_font = GTK_FONT_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "subtitle_font"));
            subtitle_font_label = GTK_WIDGET(gtk_builder_get_object(wckp->prefs->builder, "subtitle_font_label"));
            if (G_LIKELY (subtitle_font != NULL))
            {
                gtk_font_button_set_font_name(subtitle_font, wckp->prefs->subtitle_font);
                gtk_widget_set_sensitive (GTK_WIDGET(subtitle_font), wckp->prefs->two_lines);
                gtk_widget_set_sensitive (subtitle_font_label, wckp->prefs->two_lines);
                g_signal_connect(subtitle_font, "font-set", G_CALLBACK(on_subtitle_font_set), wckp);
            }
            else {
                DBG("No widget with the name \"title_font\" found");
            }

            title_alignment = GTK_COMBO_BOX(gtk_builder_get_object(wckp->prefs->builder, "title_alignment"));
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

            title_padding = GTK_SPIN_BUTTON(gtk_builder_get_object(wckp->prefs->builder, "title_padding"));
            if (G_LIKELY (title_padding != NULL)) {
                gtk_spin_button_set_range(title_padding, 0, 99);
                gtk_spin_button_set_increments(title_padding, 1, 1);
                gtk_spin_button_set_value(title_padding, wckp->prefs->title_padding);
                g_signal_connect(title_padding, "value-changed", G_CALLBACK(on_title_padding_changed), wckp);
            } else {
                DBG("No widget with the name \"title_padding\" found");
            }

            size_mode = GTK_COMBO_BOX(gtk_builder_get_object(wckp->prefs->builder, "size_mode"));
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
                    gtk_widget_set_sensitive(width_unit, FALSE);
                }

                g_signal_connect(size_mode, "changed", G_CALLBACK(on_size_mode_changed), wckp);
            } else {
                DBG("No widget with the name \"size_mode\" found");
            }

            return GTK_WIDGET(area) ;
        }
        else {
            g_set_error_literal(&error, 0, 0, "No widget with the name \"contentarea\" found");
        }
    }

    g_critical("Faild to construct the wckp->prefs->builder for plugin %s-%d: %s.", xfce_panel_plugin_get_name (wckp->plugin), xfce_panel_plugin_get_unique_id (wckp->plugin), error->message);
    g_error_free(error);
    g_object_unref(G_OBJECT (wckp->prefs->builder) );

    return NULL ;
}


static void windowck_configure_response(GtkWidget *dialog, gint response, WindowckPlugin *wckp)
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
        g_object_set_data (G_OBJECT (wckp->plugin), "dialog", NULL);

        /* unlock the panel menu */
        xfce_panel_plugin_unblock_menu (wckp->plugin);

        /* save the plugin */
        windowck_save (wckp->plugin, wckp);

        /* destroy the properties dialog */
        gtk_widget_destroy (dialog);
    }
}


void windowck_configure(XfcePanelPlugin *plugin, WindowckPlugin *wckp)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *ca;
    const gchar *name;

    /* block the plugin menu */
    xfce_panel_plugin_block_menu (plugin);

    /* create the dialog */
    name = xfce_panel_plugin_get_display_name (plugin);
    dialog = xfce_titled_dialog_new_with_buttons (_(name),
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
                    G_CALLBACK(windowck_configure_response), wckp);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog) );

    ca = build_properties_area (wckp, windowck_dialogs_ui, windowck_dialogs_ui_length);
    if (G_LIKELY (ca != NULL))
        gtk_container_add(GTK_CONTAINER (content_area), ca);
    else
        DBG("Failed to create content area");

    /* show the entire dialog */
    gtk_widget_show (dialog);
}
