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
#include "windowck-title.h"
#include <common/wck-utils.h>

#define UNFOCUSED_TEXT_ALPHA 0.5

/* Prototypes */
static void on_name_changed(WnckWindow *window, WindowckPlugin *);

static void mix(GdkColor  *color2, GdkColor  *color1, float a, GdkColor  *color) {
  color->red = color1->red * a + color2->red * (1 - a);
  color->green = color1->green * a + color2->green * (1 - a);
  color->blue = color1->blue * a + color2->blue * (1 - a);
}

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

/* Triggers when controlwindow's name OR ICON changes */
/* Warning! This function is called very often, so it should only do the most necessary things! */
static void on_name_changed(WnckWindow *controlwindow, WindowckPlugin *wckp) {
    gchar *title_text, *title_color, *title_font;
    GdkColor  color, fgColor, bgColor;

    /* get plugin widget style */
    fgColor =  GTK_WIDGET(wckp->plugin)->style->fg[GTK_STATE_NORMAL];
    bgColor =  GTK_WIDGET(wckp->plugin)->style->bg[GTK_STATE_NORMAL];

    if (controlwindow
        && ((wnck_window_get_window_type (controlwindow) != WNCK_WINDOW_DESKTOP)
            || wckp->prefs->show_on_desktop)) {

        title_text = (gchar*) wnck_window_get_name(controlwindow);
        if (wnck_window_is_active(controlwindow)) {
            // window focused
            //~ gtk_widget_set_sensitive(GTK_WIDGET(wckp->title), TRUE);
        title_color = gdk_color_to_string(&fgColor);
        }
        else {
            // window unfocused
            //~ gtk_widget_set_sensitive(GTK_WIDGET(wckp->title), FALSE);
            mix(&bgColor, &fgColor, UNFOCUSED_TEXT_ALPHA, &color);
            title_color = gdk_color_to_string(&color);
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

       title_text = g_markup_printf_escaped("<span font=\"%s\" color=\"%s\">%s</span>", title_font, title_color, title_text);
        gtk_label_set_markup(wckp->title, title_text);
    }
    else {
        /* hide text */
        title_text = "";
        gtk_label_set_text(wckp->title, title_text);
    }
}

void on_wck_state_changed (WnckWindow *controlwindow, WindowckPlugin *wckp) {
    on_name_changed (controlwindow, wckp);
}

void on_control_window_changed (WnckWindow *controlwindow, WnckWindow *previous, WindowckPlugin *wckp) {

    /* disconect previous window title signal */
    if (previous && wckp->cnh) {
        if (g_signal_handler_is_connected(G_OBJECT(previous), wckp->cnh))
            g_signal_handler_disconnect(G_OBJECT(previous), wckp->cnh);
    }

    on_name_changed (controlwindow, wckp);

    if (controlwindow) {
        wckp->cnh = g_signal_connect(G_OBJECT(controlwindow), "name-changed", G_CALLBACK(on_name_changed), wckp);
    }
}

void expandTitle(WindowckPlugin *wckp) {
    if (wckp->width > wckp->prefs->width) {
        /* expand title to max size (in chars) */
        wckp->prefs->title_size_max = wckp->prefs->title_size_max * wckp->width / wckp->prefs->width - 1; /* add - 1 to fix title overflow */
    }
    else {
        wckp->prefs->title_size_max = wckp->prefs->title_size;
    }

    /* resize title label (in chars) */
    gtk_label_set_width_chars(wckp->title, wckp->prefs->title_size_max);
}

void resizeTitle(WindowckPlugin *wckp) {
    if (wckp->prefs->size_mode == SHRINK ) {
        gtk_label_set_max_width_chars(wckp->title, wckp->prefs->title_size);
    }
    else if (wckp->prefs->size_mode == EXPAND ) {
        expandTitle(wckp);
    }
    else {
        gtk_label_set_width_chars(wckp->title, wckp->prefs->title_size);
    }
}

float alignTitle(WindowckPlugin *wckp) {
    /* correction on title alignement for expand option */
    if (wckp->prefs->title_size_max != 0
        && wckp->prefs->size_mode == EXPAND
        && wckp->prefs->title_alignment == CENTER)
        return MIN (wckp->prefs->title_alignment / 10.0 + 0.5 / wckp->prefs->title_size_max, 1);
    else
        return wckp->prefs->title_alignment / 10.0;
}

void on_windowck_size_allocated(GtkWidget *widget, GtkAllocation *allocation,  WindowckPlugin *wckp) {
    if (wckp->width != allocation->width) {
        wckp->width = allocation->width;
        expandTitle(wckp);
    }
}

void on_title_size_allocated(GtkWidget *widget, GtkAllocation *allocation,  WindowckPlugin *wckp) {
    if (wckp->prefs->width != allocation->width) {
        wckp->prefs->width = allocation->width;
    }
}

gboolean title_clicked(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp) {
    // only allow left and right mouse button
    //if (event->button != 1 && event->button != 3) return FALSE;

    if (!wckp->win->controlwindow)
        return FALSE;

    // single click (left/right)
    if (event->button == 1) {
        // left-click
        wnck_window_activate(wckp->win->controlwindow, gtk_get_current_event_time());
        if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_3BUTTON_PRESS) {
            // double/tripple click
            //if (event->type==GDK_2BUTTON_PRESS) {
            toggleMaximize(wckp->win);
        }
    } else if (event->button == 3) {
        // right-click
        wnck_window_activate(wckp->win->controlwindow, gtk_get_current_event_time());

        // let the panel show the menu
        return FALSE;
    } else {
        return FALSE;
    }
    return TRUE;
}

void initTitle (WindowckPlugin *wckp) {

    resizeTitle(wckp);

    if (wckp->prefs->size_mode != SHRINK)
    	gtk_misc_set_alignment(GTK_MISC(wckp->title), alignTitle(wckp), 0.5);

    gtk_misc_set_padding(GTK_MISC(wckp->title), wckp->prefs->title_padding, 0);

    /* start tracking the applet and title size */
    if (wckp->prefs->size_mode == EXPAND) {
    /* update wckp->width */
    g_signal_connect(G_OBJECT (wckp->plugin), "size-allocate", G_CALLBACK(on_windowck_size_allocated), wckp);

    /* update wckp->pref->width */
    g_signal_connect(G_OBJECT (wckp->title), "size-allocate", G_CALLBACK(on_title_size_allocated), wckp);
    }
}
