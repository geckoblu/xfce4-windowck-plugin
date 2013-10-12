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

#include <common/wck-utils.h>
#include <common/ui_style.h>

#include "windowck.h"
#include "windowck-title.h"

/* Prototypes */
static void on_name_changed(WnckWindow *window, WindowckPlugin *);

void updateFont(WindowckPlugin *wckp) {
    PangoFontDescription *font;
    if (wckp->prefs->custom_font) {
        font = pango_font_description_from_string(wckp->prefs->title_font);
        gtk_widget_modify_font(GTK_WIDGET(wckp->title), font);
        pango_font_description_free(font);
    }
    else {
        gtk_widget_modify_font(GTK_WIDGET(wckp->title), NULL);
    }
}

static void on_icon_changed(WnckWindow *controlwindow, WindowckPlugin *wckp) {
    GdkPixbuf *pixbuf = NULL;
    GdkPixbuf *grayscale = NULL;

    if (controlwindow) {
        /* This only returns a pointer - it SHOULDN'T be unrefed! */
        if (wckp->icon->size == GTK_ICON_SIZE_MENU)
            pixbuf = wnck_window_get_mini_icon(controlwindow);
        else
            pixbuf = wnck_window_get_icon(controlwindow);
    }

    if (controlwindow && !wnck_window_is_active(controlwindow)) {
        /* icon color is set to grayscale */
        grayscale = gdk_pixbuf_copy(pixbuf);
        gdk_pixbuf_saturate_and_pixelate(grayscale, grayscale, 0, FALSE);
        if (G_UNLIKELY (grayscale != NULL))
            pixbuf = grayscale;
    }

    if (controlwindow
        && (wnck_window_get_window_type (controlwindow) != WNCK_WINDOW_DESKTOP)) {
        gtk_image_set_from_pixbuf(wckp->icon->image, pixbuf);
    }
    else if (controlwindow && wckp->prefs->show_on_desktop) {
        gtk_image_set_from_stock(wckp->icon->image,GTK_STOCK_HOME, wckp->icon->size);
    }
    else {
        gtk_image_clear(wckp->icon->image);
    }

    if (grayscale != NULL && grayscale != pixbuf)
        g_object_unref (G_OBJECT (grayscale));
}

/* Triggers when controlwindow's name OR ICON changes */
/* Warning! This function is called very often, so it should only do the most necessary things! */
static void on_name_changed(WnckWindow *controlwindow, WindowckPlugin *wckp) {
    gint i, n;
    const gchar *title_text, *title_markup;
    const gchar *title_color, *title_font;
    gchar *app_name, *instance_name;
    gchar **part;

    if (controlwindow
        && ((wnck_window_get_window_type (controlwindow) != WNCK_WINDOW_DESKTOP)
            || wckp->prefs->show_on_desktop)) {

        title_text = wnck_window_get_name(controlwindow);

        if (wnck_window_is_active(controlwindow)) {
            /* window focused */
            //~ gtk_widget_set_sensitive(GTK_WIDGET(wckp->title), TRUE);
            title_color = wckp->prefs->active_text_color;
        }
        else {
            /* window unfocused */
            //~ gtk_widget_set_sensitive(GTK_WIDGET(wckp->title), FALSE);
            title_color = wckp->prefs->inactive_text_color;
        }

        if (wckp->prefs->custom_font)
            title_font = wckp->prefs->title_font;
        else
            title_font = "";

        updateFont(wckp);

        /* Set tooltips */
        if (wckp->prefs->show_tooltips) {
            gtk_widget_set_tooltip_text(GTK_WIDGET(wckp->title), title_text);
        }

        /* get application and instance names */
        if (wckp->prefs->full_name) {
            title_markup = g_markup_printf_escaped("<span font=\"%s\" color=\"%s\">%s</span>", title_font, title_color, title_text);
        }
        else {

        	/* split title text */
        	part = g_strsplit (title_text, " - ", 0);
        	n=0;
        	for (i=0; part[i]; i++) {
            	n++;
        	}

            if (n > 1) {
                title_markup = g_markup_printf_escaped("<span font=\"%s\" color=\"%s\">%s</span>", title_font, title_color, part[n-1]);
            }
            else {
                title_markup = g_markup_printf_escaped("<span font=\"%s\" color=\"%s\">%s</span>", title_font, title_color, part[0]);
            }
            g_strfreev(part);
        }

        gtk_label_set_markup(wckp->title, title_markup);
    }
    else {
        /* hide text */
        title_text = "";
        gtk_label_set_text(wckp->title, title_text);
    }
}

void on_wck_state_changed (WnckWindow *controlwindow, WindowckPlugin *wckp) {
    on_name_changed (controlwindow, wckp);
    if (wckp->prefs->show_icon)
        on_icon_changed (wckp->win->controlwindow, wckp);
}

void on_control_window_changed (WnckWindow *controlwindow, WnckWindow *previous, WindowckPlugin *wckp) {

    /* disconect previous window title signal */
    wck_signal_handler_disconnect (G_OBJECT(previous), wckp->cnh);

    on_name_changed (controlwindow, wckp);

    if (controlwindow)
    {
        wckp->cnh = g_signal_connect(G_OBJECT(controlwindow), "name-changed", G_CALLBACK(on_name_changed), wckp);
    }

    if (wckp->prefs->show_icon)
    {
        wck_signal_handler_disconnect (G_OBJECT(previous), wckp->cih);

        on_icon_changed (controlwindow, wckp);

        if (controlwindow)
        {
            wckp->cih = g_signal_connect(G_OBJECT(controlwindow), "icon-changed", G_CALLBACK(on_icon_changed), wckp);
        }
    }

}

void resizeTitle(WindowckPlugin *wckp) {
    switch (wckp->prefs->size_mode) {
        case SHRINK:
            gtk_label_set_max_width_chars(wckp->title, wckp->prefs->title_size);
            break;
        case EXPAND:
            gtk_label_set_width_chars(wckp->title, TITLE_SIZE_MAX);
            break;
        default:
            gtk_label_set_width_chars(wckp->title, wckp->prefs->title_size);
    }
}

gboolean on_title_pressed(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp) {

    if (!wckp->win->controlwindow)
        return FALSE;

    if (event->button == 1) {
        /* double/tripple click */
        if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_3BUTTON_PRESS) {
            toggleMaximize(wckp->win->controlwindow);
        }
        else {
        /* left-click */
        wnck_window_activate(wckp->win->controlwindow, gtk_get_current_event_time());
        }
        return TRUE;
    }

    if (event->button == 3) {
        /* right-click */
        wnck_window_activate(wckp->win->controlwindow, gtk_get_current_event_time());

        /* let the panel show the menu */
        return TRUE;
    }

    return FALSE;
}

gboolean on_title_released(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp) {

    if (!wckp->win->controlwindow)
        return FALSE;

    if (event->button == 2) {
        /* middle-click */
        wnck_window_close(wckp->win->controlwindow, GDK_CURRENT_TIME);
        return TRUE;
    }

    return FALSE;
}


gboolean on_icon_released(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp) {

    if (event->button != 1)
        return FALSE;

    GtkWidget *menu;
    menu = wnck_action_menu_new (wckp->win->controlwindow);

    gtk_menu_attach_to_widget(GTK_MENU(menu), GTK_WIDGET(wckp->icon->eventbox), NULL);
    gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
                      xfce_panel_plugin_position_menu,
                      wckp->plugin,
                      1, gtk_get_current_event_time ());

    return TRUE;
}

static void setTitleColors(WindowckPlugin *wckp) {
    gchar *title_color;
    GdkPixbuf *icon_pixbuf;
    GdkColor  color, textColor, bgColor;

    /* get plugin widget style */
    textColor =  GTK_WIDGET(wckp->plugin)->style->text[GTK_STATE_ACTIVE];
    wckp->prefs->active_text_color = gdk_color_to_string(&textColor);

    bgColor =  GTK_WIDGET(wckp->plugin)->style->bg[GTK_STATE_NORMAL];
    textColor =  GTK_WIDGET(wckp->plugin)->style->text[GTK_STATE_NORMAL];
    color = mix(bgColor, textColor, UNFOCUSED_TEXT_ALPHA);
    wckp->prefs->inactive_text_color = gdk_color_to_string(&color);
}

void initTitle (WindowckPlugin *wckp) {

    setTitleColors(wckp);
    wckp->icon->size = GTK_ICON_SIZE_MENU;
    resizeTitle(wckp);

    gtk_label_set_ellipsize(wckp->title, PANGO_ELLIPSIZE_END);

    if (wckp->prefs->size_mode != SHRINK)
        gtk_misc_set_alignment(GTK_MISC(wckp->title), wckp->prefs->title_alignment / 10.0, 0.5);

    gtk_alignment_set_padding(GTK_ALIGNMENT(wckp->alignment), 0, 0, wckp->prefs->title_padding, wckp->prefs->title_padding);
    gtk_box_set_spacing (GTK_BOX(wckp->hvbox), wckp->prefs->title_padding);
}
