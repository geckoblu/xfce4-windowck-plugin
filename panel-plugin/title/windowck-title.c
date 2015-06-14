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

#include <libxfce4util/libxfce4util.h>
#include <common/wck-utils.h>
#include <common/ui_style.h>
#include <common/theme.h>

#include "windowck.h"
#include "windowck-title.h"

/* Prototypes */
static void on_name_changed(WnckWindow *window, WindowckPlugin *);


void reload_wnck_title (WindowckPlugin *wckp)
{
    /* disconnect controled window name and icon signal handlers */
    wck_signal_handler_disconnect (G_OBJECT(wckp->win->controlwindow), wckp->cnh);
    wck_signal_handler_disconnect (G_OBJECT(wckp->win->controlwindow), wckp->cih);

    reload_wnck (wckp->win, wckp->prefs->only_maximized, wckp);
}


static void on_icon_changed(WnckWindow *controlwindow, WindowckPlugin *wckp)
{
    GdkPixbuf *pixbuf = NULL;
    GdkPixbuf *grayscale = NULL;


    if (!controlwindow)
    {
        xfce_panel_image_clear(XFCE_PANEL_IMAGE (wckp->icon->symbol));
    }
    else if (wckp->prefs->show_on_desktop)
    {
        gtk_widget_set_sensitive (wckp->icon->symbol, TRUE);

        if (wnck_window_get_window_type (controlwindow) == WNCK_WINDOW_DESKTOP)
        {
            if (!wnck_window_is_active(controlwindow))
                gtk_widget_set_sensitive (wckp->icon->symbol, FALSE);

            xfce_panel_image_set_from_source (XFCE_PANEL_IMAGE (wckp->icon->symbol),GTK_STOCK_HOME);
        }
    }

    if (controlwindow
        && wnck_window_get_window_type (controlwindow) != WNCK_WINDOW_DESKTOP)
    {
        /* This only returns a pointer - it SHOULDN'T be unrefed! */
        if (XFCE_PANEL_IS_SMALL)
            pixbuf = wnck_window_get_mini_icon(controlwindow);
        else
            pixbuf = wnck_window_get_icon(controlwindow);

        /* leave when there is no valid pixbuf */
        if (G_UNLIKELY (pixbuf == NULL))
        {
            xfce_panel_image_clear (XFCE_PANEL_IMAGE (wckp->icon->symbol));
            return;
        }

        if (!wnck_window_is_active(controlwindow))
        {
            /* icon color is set to grayscale */
            grayscale = gdk_pixbuf_copy(pixbuf);
            gdk_pixbuf_saturate_and_pixelate(grayscale, grayscale, 0, FALSE);
            if (G_UNLIKELY (grayscale != NULL))
                pixbuf = grayscale;
        }

        xfce_panel_image_set_from_pixbuf(XFCE_PANEL_IMAGE (wckp->icon->symbol), pixbuf);
    }

    if (grayscale != NULL && grayscale != pixbuf)
        g_object_unref (G_OBJECT (grayscale));
}


/* Triggers when controlwindow's name changes */
/* Warning! This function is called very often, so it should only do the most necessary things! */
static void on_name_changed (WnckWindow *controlwindow, WindowckPlugin *wckp)
{
    gint i, n;
    
    const gchar *title_text;

    if (controlwindow
        && ((wnck_window_get_window_type (controlwindow) != WNCK_WINDOW_DESKTOP)
            || wckp->prefs->show_on_desktop))
    {
        const gchar *title_color, *title_font, *subtitle_font;
        gchar **part, *title_markup;

        title_text = wnck_window_get_name(controlwindow);

        if (wnck_window_is_active(controlwindow))
        {
            /* window focused */
            //~ gtk_widget_set_sensitive(GTK_WIDGET(wckp->title), TRUE);
            title_color = wckp->prefs->active_text_color;
        }
        else {
            /* window unfocused */
            //~ gtk_widget_set_sensitive(GTK_WIDGET(wckp->title), FALSE);
            title_color = wckp->prefs->inactive_text_color;
        }

        title_font = wckp->prefs->title_font;
        subtitle_font = wckp->prefs->subtitle_font;

        /* Set tooltips */
        if (wckp->prefs->show_tooltips)
        {
            gtk_widget_set_tooltip_text(GTK_WIDGET(wckp->title), title_text);
        }

        /* get application and instance names */
        if (wckp->prefs->full_name && !wckp->prefs->two_lines)
        {
            title_markup = g_markup_printf_escaped("<span font=\"%s\" color=\"%s\">%s</span>", title_font, title_color, title_text);
        }
        else {
            /* split title text */
            part = g_strsplit (title_text, " - ", 0);
            n=0;
            for (i=0; part[i]; i++) n++;

            if (n > 1)
            {
                if (wckp->prefs->two_lines)
                {
                    gchar *subtitle = malloc( sizeof(gchar) * ( strlen(title_text) + 1 ) );
                    strcpy (subtitle, part[0]);
                    if (wckp->prefs->full_name)
                    {
                        for (i=1; i < n - 1; i++)
                        {
                            strcat (subtitle, " - ");
                            strcat (subtitle, part[i]);
                        }
                    }
                    title_markup = g_markup_printf_escaped("<span font=\"%s\" color=\"%s\">%s</span><span font=\"%s\" color=\"%s\">\n%s</span>", title_font, title_color, part[n-1], subtitle_font, title_color,  subtitle);
                    g_free (subtitle);
                }
                else
                {
                    title_markup = g_markup_printf_escaped("<span font=\"%s\" color=\"%s\">%s</span>", title_font, title_color, part[n-1]);
                }
            }
            else {
                title_markup = g_markup_printf_escaped("<span font=\"%s\" color=\"%s\">%s</span>", title_font, title_color, part[0]);
            }
            g_strfreev(part);
        }

        gtk_label_set_markup(wckp->title, title_markup);

        if (wckp->prefs->title_alignment == LEFT)
        {
            gtk_label_set_justify(wckp->title, GTK_JUSTIFY_LEFT);
        }
        else if (wckp->prefs->title_alignment == CENTER)
        {
            gtk_label_set_justify(wckp->title, GTK_JUSTIFY_CENTER);
        }
        else if (wckp->prefs->title_alignment == RIGHT)
        {
            gtk_label_set_justify(wckp->title, GTK_JUSTIFY_RIGHT);
        }

        g_free (title_markup);
    }
    else {
        /* hide text */
        title_text = "";
        gtk_label_set_text(wckp->title, title_text);
    }
}


void on_wck_state_changed (WnckWindow *controlwindow, gpointer data)
{
    WindowckPlugin *wckp = data;

    on_name_changed (controlwindow, wckp);

    if (wckp->prefs->show_window_menu)
    {
        if (wckp->prefs->show_app_icon)
        {
            on_icon_changed (wckp->win->controlwindow, wckp);
        }
        else
        {
            GdkColor color;

            if (controlwindow
                && ((wnck_window_get_window_type (controlwindow) != WNCK_WINDOW_DESKTOP)
                    || wckp->prefs->show_on_desktop))
            {
                if (wnck_window_is_active(controlwindow)
                    && gdk_color_parse (wckp->prefs->active_text_color, &color))
                {
                    gtk_widget_modify_fg (wckp->icon->symbol, GTK_STATE_NORMAL, &color);
                }
                else if (gdk_color_parse (wckp->prefs->inactive_text_color, &color))
                {
                    gtk_widget_modify_fg (wckp->icon->symbol, GTK_STATE_NORMAL, &color);
                }
            }
        }
    }
}


void on_control_window_changed (WnckWindow *controlwindow, WnckWindow *previous, gpointer data)
{
    WindowckPlugin *wckp = data;

    /* disconect previous window title signal */
    wck_signal_handler_disconnect (G_OBJECT(previous), wckp->cnh);

    on_wck_state_changed (controlwindow, wckp);

    if (!controlwindow
        || ((wnck_window_get_window_type (controlwindow) == WNCK_WINDOW_DESKTOP)
        && !wckp->prefs->show_on_desktop))
    {
        if (gtk_widget_get_visible(GTK_WIDGET(wckp->hvbox)))
            gtk_widget_hide_all(GTK_WIDGET(wckp->hvbox));
    }
    else
    {
        if (!gtk_widget_get_visible(GTK_WIDGET(wckp->hvbox)))
            gtk_widget_show_all(GTK_WIDGET(wckp->hvbox));
    }

    if (controlwindow)
    {
        if (wnck_window_get_window_type (controlwindow) != WNCK_WINDOW_DESKTOP)
        {
            wckp->cnh = g_signal_connect(G_OBJECT(controlwindow), "name-changed", G_CALLBACK(on_name_changed), wckp);
            if (!gtk_widget_get_visible(GTK_WIDGET(wckp->icon->eventbox)))
                gtk_widget_show_all (GTK_WIDGET(wckp->icon->eventbox));
        }
        else if (wckp->prefs->show_on_desktop && !wckp->prefs->show_app_icon)
        {
            if (gtk_widget_get_visible(GTK_WIDGET(wckp->icon->eventbox)))
                gtk_widget_hide_all (GTK_WIDGET(wckp->icon->eventbox));
        }
    }

    if (wckp->prefs->show_app_icon && wckp->prefs->show_window_menu)
    {
        wck_signal_handler_disconnect (G_OBJECT(previous), wckp->cih);

        if (controlwindow)
            wckp->cih = g_signal_connect(G_OBJECT(controlwindow), "icon-changed", G_CALLBACK(on_icon_changed), wckp);
    }
}


void resize_title(WindowckPlugin *wckp)
{
    switch (wckp->prefs->size_mode)
    {
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


gboolean on_title_pressed(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp)
{

    if (!wckp->win->controlwindow)
        return FALSE;

    if (event->button == 1
        && (wnck_window_get_window_type (wckp->win->controlwindow) != WNCK_WINDOW_DESKTOP))
    {
        /* double/tripple click */
        if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_3BUTTON_PRESS)
        {
            toggle_maximize(wckp->win->controlwindow);
        }
        else /* left-click */
        {
            wnck_window_activate(wckp->win->controlwindow, gtk_get_current_event_time());
        }
        return TRUE;
    }

    if (event->button == 3)
    {
        /* right-click */
        wnck_window_activate(wckp->win->controlwindow, gtk_get_current_event_time());

        /* let the panel show the menu */
        return TRUE;
    }

    return FALSE;
}


gboolean on_title_released(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp)
{
    if (!wckp->win->controlwindow)
        return FALSE;

    if (event->button == 2)
    {
        /* middle-click */
        wnck_window_close(wckp->win->controlwindow, GDK_CURRENT_TIME);
        return TRUE;
    }

    return FALSE;
}


gboolean on_icon_released(GtkWidget *title, GdkEventButton *event, WindowckPlugin *wckp)
{
    GtkWidget *menu;

    if ((event->button != 1)
        || (wnck_window_get_window_type (wckp->win->controlwindow) == WNCK_WINDOW_DESKTOP))
        return FALSE;

    menu = wnck_action_menu_new (wckp->win->controlwindow);

    gtk_menu_attach_to_widget(GTK_MENU(menu), GTK_WIDGET(wckp->icon->eventbox), NULL);
    gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
                      xfce_panel_plugin_position_menu,
                      wckp->plugin,
                      1, gtk_get_current_event_time ());

    return TRUE;
}


static void set_title_colors(WindowckPlugin *wckp)
{
    /* get plugin widget style */
    wckp->prefs->active_text_color = get_ui_color  (GTK_WIDGET(wckp->plugin), "fg", "normal");
    wckp->prefs->inactive_text_color = mix_bg_fg (GTK_WIDGET(wckp->plugin), "normal", wckp->prefs->inactive_text_alpha / 100.0, wckp->prefs->inactive_text_shade / 100.0);
}


static void apply_wm_settings (WindowckPlugin *wckp)
{
    const gchar *wm_theme = xfconf_channel_get_string (wckp->wm_channel, "/general/theme", NULL);

    if (G_LIKELY(wm_theme))
    {
        const gchar *wm_title_font = xfconf_channel_get_string (wckp->wm_channel, "/general/title_font", wckp->prefs->title_font);
        wckp->prefs->title_font = g_strdup (wm_title_font);

        on_name_changed (wckp->win->controlwindow, wckp);
    }
}


static void
on_x_chanel_property_changed (XfconfChannel *x_channel, const gchar *property_name, const GValue *value, WindowckPlugin *wckp)
{
    if (g_str_has_prefix(property_name, "/Net/") == TRUE)
    {
        const gchar *name = &property_name[5];
        switch (G_VALUE_TYPE(value))
        {
            case G_TYPE_STRING:
                if (!strcmp (name, "ThemeName"))
                {
                    set_title_colors(wckp);
                    on_name_changed (wckp->win->controlwindow, wckp);
                }
                break;
            default:
                g_warning("The property '%s' is not supported", property_name);
                break;
        }
    }
}


static void on_xfwm_channel_property_changed (XfconfChannel *wm_channel, const gchar *property_name, const GValue *value, WindowckPlugin *wckp)
{
    if (g_str_has_prefix(property_name, "/general/") == TRUE)
    {
        const gchar *name = &property_name[9];
        switch (G_VALUE_TYPE(value))
        {
            case G_TYPE_STRING:
                if (!strcmp (name, "title_font")
					|| !strcmp (name, "show_app_icon"))
                {
                    apply_wm_settings (wckp);
                }
                else if (!strcmp (name, "theme"))
                {
					init_title(wckp);
					reload_wnck_title (wckp);
                }
                break;
            default:
                g_warning("The property '%s' is not supported", property_name);
                break;
        }
    }
}


void init_title (WindowckPlugin *wckp)
{
    set_title_colors(wckp);
    resize_title(wckp);

    gtk_label_set_ellipsize(wckp->title, PANGO_ELLIPSIZE_END);

    if (wckp->prefs->size_mode != SHRINK)
        gtk_misc_set_alignment(GTK_MISC(wckp->title), wckp->prefs->title_alignment / 10.0, 0.5);

    /* get the xfwm4 chanel */
    wckp->wm_channel = wck_properties_get_channel (G_OBJECT (wckp->plugin), "xfwm4");

    /* try to set title settings from the xfwm4 theme */
    if (wckp->wm_channel && wckp->prefs->sync_wm_font)
    {
        apply_wm_settings (wckp);
        g_signal_connect (wckp->wm_channel, "property-changed", G_CALLBACK (on_xfwm_channel_property_changed), wckp);
    }

    gtk_alignment_set_padding(GTK_ALIGNMENT(wckp->alignment), ICON_PADDING, ICON_PADDING, wckp->prefs->title_padding, wckp->prefs->title_padding);
    gtk_box_set_spacing (GTK_BOX(wckp->hvbox), wckp->prefs->title_padding);

    /* get the xsettings chanel to update the gtk theme */
    wckp->x_channel = wck_properties_get_channel (G_OBJECT (wckp->plugin), "xsettings");

    if (wckp->x_channel)
        g_signal_connect (wckp->x_channel, "property-changed", G_CALLBACK (on_x_chanel_property_changed), wckp);
}
