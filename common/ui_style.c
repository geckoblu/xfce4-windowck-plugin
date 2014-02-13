/*      $Id$

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2, or (at your option)
        any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., Inc., 51 Franklin Street, Fifth Floor, Boston,
        MA 02110-1301, USA.


        xfwm4    - (c) 2002-2011 Olivier Fourdan
        xfwm4-windowck-plugin - (c) 2013 Cedric leporcq
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <pango/pango-font.h>

#include <libxfce4util/libxfce4util.h>
#include "ui_style.h"

char *states[] = {
    "normal", "active", "prelight", "selected", "insensitive", NULL
};

char *names[] = {
    "fg", "bg", "text", "base", "light", "dark", "mid", NULL
};

typedef enum {
    GTKSTYLE_FG = 0,
    GTKSTYLE_BG,
    GTKSTYLE_TEXT,
    GTKSTYLE_BASE,
    GTKSTYLE_LIGHT,
    GTKSTYLE_DARK,
    GTKSTYLE_MID,
    MIX_BG_TEXT,

    COLOR_NAMES
} ColorNames;

GdkColor shade (GdkColor  color, float b)
{
    color.red = color.red * b;
    color.green = color.green * b;
    color.blue = color.blue * b;

    return color;
}

GdkColor mix (GdkColor  color2, GdkColor  color1, float a)
{
    GdkColor color;

    color.red = color1.red * a + color2.red * (1 - a);
    color.green = color1.green * a + color2.green * (1 - a);
    color.blue = color1.blue * a + color2.blue * (1 - a);

    return color;
}

static gint
state_value (const gchar * s)
{
    gint u = 0;

    while ((states[u]) && (strcmp (states[u], s)))
    {
        u++;
    }
    if (states[u])
    {
        return (u);
    }
    return (0);
}

static gint
name_value (const gchar * s)
{
    gint u = 0;

    while ((names[u]) && (strcmp (names[u], s)))
    {
        u++;
    }
    if (names[u])
    {
        return (u);
    }
    return (0);
}

GdkColor
query_color (GtkWidget * win, GdkColor c)
{
    GdkColor real_color;
    GdkColormap *cmap;

    cmap = gtk_widget_get_colormap (GTK_WIDGET (win));
    if (cmap && GDK_IS_COLORMAP (cmap))
    {
        gdk_colormap_query_color (cmap, c.pixel, &real_color);
        return real_color;
    }
    else
    {
        return c;
    }
}

static gchar *
print_color (GtkWidget * win, GdkColor color)
{
    gchar *s;
    s = g_new (gchar, 14);
    g_snprintf (s, 14, "#%04x%04x%04x", color.red, color.green,
                    color.blue);
    return s;
}

static gchar *
print_colors (GtkWidget * win, GdkColor * x, int n)
{
    GdkColor color;
    color = query_color (win, x[n]);
    return print_color(win, color);
}

static gchar *
print_rc_style (GtkWidget * win, const gchar * name, const gchar * state,
                GtkStyle * style)
{
    gchar *s;
    gint n, m;

    g_return_val_if_fail (state != NULL, NULL);
    g_return_val_if_fail (name != NULL, NULL);

    n = state_value (state);
    m = name_value (name);

    switch (m)
    {
        case GTKSTYLE_FG:
            s = print_colors (win, style->fg, n);
            break;
        case GTKSTYLE_BG:
            s = print_colors (win, style->bg, n);
            break;
        case GTKSTYLE_TEXT:
            s = print_colors (win, style->text, n);
            break;
        case GTKSTYLE_BASE:
            s = print_colors (win, style->base, n);
            break;
        case GTKSTYLE_LIGHT:
            s = print_colors (win, style->light, n);
            break;
        case GTKSTYLE_DARK:
            s = print_colors (win, style->dark, n);
            break;
        default:
        case GTKSTYLE_MID:
            s = print_colors (win, style->mid, n);
            break;
    }
    return (s);
}

static GtkStyle *
get_ui_style (GtkWidget * win)
{
    GtkStyle *style;

    TRACE ("entering get_ui_style");

    style = gtk_rc_get_style (win);
    if (!style)
    {
        style = gtk_widget_get_style (win);
    }
    return (style);
}

gchar *
get_ui_color (GtkWidget * win, const gchar * name, const gchar * state)
{
    GtkStyle *style;
    gchar *s;

    TRACE ("entering get_ui_color");

    g_return_val_if_fail (win != NULL, NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (win), NULL);
    g_return_val_if_fail (GTK_WIDGET_REALIZED (win), NULL);

    style = get_ui_style (win);
    s = print_rc_style (win, name, state, style);
    TRACE ("%s[%s]=%s", name, state, s);
    return (s);
}

gchar *
mix_bg_fg (GtkWidget * win, const gchar * state, float alpha, float beta)
{
    GdkColor color, bgColor, fgColor;
    GtkStyle *style;
    gchar *s;
    gint n;

    TRACE ("entering mix_bg_fg_ui");

    g_return_val_if_fail (win != NULL, NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (win), NULL);
    g_return_val_if_fail (GTK_WIDGET_REALIZED (win), NULL);

    style = get_ui_style (win);
    n = state_value (state);

    bgColor = query_color (win, style->bg[n]);
    fgColor = query_color (win, style->fg[n]);
    color = shade (mix (bgColor, fgColor, alpha), beta);
    s = print_color (win, color);

    TRACE ("mix_bg_fg[%s]=%s", state, s);
    return (s);
}

static GdkGC *
_getUIStyle_gc (const gchar * name, const gchar * state, GtkStyle * style)
{
    GdkGC *gc;
    gint n, m;

    g_return_val_if_fail (state != NULL, NULL);
    g_return_val_if_fail (name != NULL, NULL);
    g_return_val_if_fail (style != NULL, NULL);
    g_return_val_if_fail (GTK_IS_STYLE(style), NULL);

    n = state_value (state);
    m = name_value (name);

    switch (m)
    {
        case GTKSTYLE_FG:
            gc = style->fg_gc[n];
            break;
        case GTKSTYLE_BG:
            gc = style->bg_gc[n];
            break;
        case GTKSTYLE_TEXT:
            gc = style->text_gc[n];
            break;
        case GTKSTYLE_BASE:
            gc = style->base_gc[n];
            break;
        case GTKSTYLE_LIGHT:
            gc = style->light_gc[n];
            break;
        case GTKSTYLE_DARK:
            gc = style->dark_gc[n];
            break;
        default:
        case GTKSTYLE_MID:
            gc = style->mid_gc[n];
            break;
    }

    return (gc);
}

GdkGC *
getUIStyle_gc (GtkWidget * win, const gchar * name, const gchar * state)
{
    GtkStyle *style;

    TRACE ("entering getUIStyle_gc");

    g_return_val_if_fail (win != NULL, NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (win), NULL);
    g_return_val_if_fail (GTK_WIDGET_REALIZED (win), NULL);

    style = gtk_rc_get_style (win);
    if (!style)
    {
        style = gtk_widget_get_style (win);
    }
    if (!style)
    {
        style = gtk_widget_get_default_style ();
    }
    return (_getUIStyle_gc (name, state, style));
}

PangoFontDescription *
getUIPangoFontDesc (GtkWidget * win)
{
    TRACE ("entering getUIPangoFontDesc");

    g_return_val_if_fail (win != NULL, NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (win), NULL);
    g_return_val_if_fail (GTK_WIDGET_REALIZED (win), NULL);

    return (win->style->font_desc);
}

PangoContext *
getUIPangoContext (GtkWidget * win)
{
    TRACE ("entering getUIPangoContext");

    g_return_val_if_fail (win != NULL, NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (win), NULL);
    g_return_val_if_fail (GTK_WIDGET_REALIZED (win), NULL);

    return (gtk_widget_get_pango_context (win));
}
