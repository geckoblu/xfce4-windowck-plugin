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
 *  Parts of this code is derived from xfwm4 sources (setting.c)
 *
 */

#include <libxfce4util/libxfce4util.h>
#include <common/ui_style.h>
#include <common/mypixmap.h>

#include "theme.h"

#define XPM_COLOR_SYMBOL_SIZE 22
#define UNITY_TEST_FILE         "close_focused_normal.png"


static gchar *getSystemThemeDir (const gchar *default_theme)
{
    return g_build_filename (DATADIR, "themes", default_theme, "xfwm4", NULL);
}


gchar *test_theme_dir (const gchar *theme, const char *themedir, const gchar *file) {
    gchar *test_file, *abs_path, *path;

    path = g_build_filename (theme, themedir, file, NULL);

    xfce_resource_push_path (XFCE_RESOURCE_THEMES,
                             DATADIR G_DIR_SEPARATOR_S "themes");
    test_file = xfce_resource_lookup (XFCE_RESOURCE_THEMES, path);
    xfce_resource_pop_path (XFCE_RESOURCE_THEMES);

    g_free (path);

    if (test_file)
    {
        abs_path = g_path_get_dirname (test_file);
        g_free (test_file);

        return abs_path;
    }

    return NULL;
}


gchar *
getThemeDir (const gchar *theme, const gchar *default_theme)
{
    const gchar *file;
    gchar *abs_path;
    gint i;

    static const char *themedirs[] = {
      "xfwm4",
      "unity",
      NULL
    };

    if (g_path_is_absolute (theme))
    {
        if (g_file_test (theme, G_FILE_TEST_IS_DIR))
        {
            return g_strdup (theme);
        }
    }

    for (i=0; themedirs[i]; i++)
    {
        if (strcmp(themedirs[i], "unity") == 0)
            file = UNITY_TEST_FILE;
        else
            file = "wckrc";

        abs_path = test_theme_dir (theme, themedirs[i], file);

        if (abs_path)
            return abs_path;
    }

    /* Pfew, really can't find that theme nowhere! */
    if (default_theme)
        return getSystemThemeDir (default_theme);

    return NULL;
}


static gboolean
setGValue (const gchar * lvalue, const GValue *rvalue, Settings *rc)
{
    gint i;

    TRACE ("entering setValue");

    g_return_val_if_fail (lvalue != NULL, FALSE);
    g_return_val_if_fail (rvalue != NULL, FALSE);

    for (i = 0; rc[i].option; i++)
    {
        if (!g_ascii_strcasecmp (lvalue, rc[i].option))
        {
            if (rvalue)
            {
                if (rc[i].value)
                {
                    g_value_unset (rc[i].value);
                    g_value_init (rc[i].value, G_VALUE_TYPE(rvalue));
                }
                else
                {
                    rc[i].value = g_new0(GValue, 1);
                    g_value_init (rc[i].value, G_VALUE_TYPE(rvalue));
                }

                g_value_copy (rvalue, rc[i].value);
                return TRUE;
            }
        }
    }
    return FALSE;
}


static gboolean
setStringValue (const gchar * lvalue, const gchar *value, Settings *rc)
{
    GValue tmp_val = {0, };
    g_value_init(&tmp_val, G_TYPE_STRING);
    g_value_set_static_string(&tmp_val, value);
    return setGValue (lvalue, &tmp_val, rc);
}


/* use xfwm4 buttons naming */
static void get_wm_pixbuf (const gchar *themedir, WBPlugin *wb) {
    gint i,j;
    gchar imagename[30];
    xfwmColorSymbol colsym[ XPM_COLOR_SYMBOL_SIZE + 1 ];

    Settings rc[] = {
        /* Do not change the order of the following parameters */
        {"active_text_color", NULL, G_TYPE_STRING, FALSE},
        {"inactive_text_color", NULL, G_TYPE_STRING, FALSE},
        {"active_text_shadow_color", NULL, G_TYPE_STRING, FALSE},
        {"inactive_text_shadow_color", NULL, G_TYPE_STRING, FALSE},
        {"active_border_color", NULL, G_TYPE_STRING, FALSE},
        {"inactive_border_color", NULL, G_TYPE_STRING, FALSE},
        {"active_color_1", NULL, G_TYPE_STRING, FALSE},
        {"active_hilight_1", NULL, G_TYPE_STRING, FALSE},
        {"active_shadow_1", NULL, G_TYPE_STRING, FALSE},
        {"active_mid_1", NULL, G_TYPE_STRING, FALSE},
        {"active_color_2", NULL, G_TYPE_STRING, FALSE},
        {"active_hilight_2", NULL, G_TYPE_STRING, FALSE},
        {"active_shadow_2", NULL, G_TYPE_STRING, FALSE},
        {"active_mid_2", NULL, G_TYPE_STRING, FALSE},
        {"inactive_color_1", NULL, G_TYPE_STRING, FALSE},
        {"inactive_hilight_1", NULL, G_TYPE_STRING, FALSE},
        {"inactive_shadow_1", NULL, G_TYPE_STRING, FALSE},
        {"inactive_mid_1", NULL, G_TYPE_STRING, FALSE},
        {"inactive_color_2", NULL, G_TYPE_STRING, FALSE},
        {"inactive_hilight_2", NULL, G_TYPE_STRING, FALSE},
        {"inactive_shadow_2", NULL, G_TYPE_STRING, FALSE},
        {"inactive_mid_2", NULL, G_TYPE_STRING, FALSE}
    };

    static const char *ui_part[] = {
        "text",
        "mix_bg_text",
        "dark",
        "dark",
        "fg",
        "fg",
        "bg",
        "light",
        "dark",
        "mid",
        "bg",
        "light",
        "dark",
        "mid",
        "bg",
        "light",
        "dark",
        "mid",
        "bg",
        "light",
        "dark",
        "mid",
        NULL
    };

    static const char *ui_state[] = {
        "active",
        "normal",
        "active",
        "insensitive",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        "normal",
        NULL
    };

    static const char *button_names[] = {
      //~ "menu",
      //~ "stick",
      //~ "shade",
      "hide",
      "maximize-toggled",
      "maximize",
      "close"
    };

    static const char *button_state_names[] = {
      "inactive",
      "active",
      "prelight",
      "pressed"
    };

    while (ui_part[i] && ui_state[i])
    {
        gchar *color;

        color = getUIStyle  (GTK_WIDGET(wb->plugin), ui_part[i], ui_state[i]);
        setStringValue (rc[i].option, color, rc);
        g_free (color);
        ++i;
    }

    for (i = 0; i < XPM_COLOR_SYMBOL_SIZE; i++)
    {
        colsym[i].name = rc[i].option;
        colsym[i].value = g_value_get_string(rc[i].value);
    }
    colsym[XPM_COLOR_SYMBOL_SIZE].name = NULL;
    colsym[XPM_COLOR_SYMBOL_SIZE].value = NULL;

    for (i = 0; i < IMAGES_BUTTONS; i++)
    {
        for (j = 0; j < IMAGES_STATES; j++)
        {
            g_snprintf(imagename, sizeof (imagename), "%s-%s", button_names[i], button_state_names[j]);
            wb->pixbufs[i][j] = xfwmPixbufLoad (themedir, imagename, colsym);
        }
    }
}


static void get_unity_pixbuf (const gchar *themedir, WBPlugin *wb) {
    gint i,j;
    gchar imagename[40];

    static const char *button_names[] = {
      "minimize",
      "unmaximize",
      "maximize",
      "close"
    };

    static const char *button_state_names[] = {
      "unfocused",
      "focused_normal",
      "focused_prelight",
      "focused_pressed"
    };

    for (i = 0; i < IMAGES_BUTTONS; i++)
    {
        for (j = 0; j < IMAGES_STATES; j++)
        {
            g_snprintf(imagename, sizeof (imagename), "%s_%s", button_names[i], button_state_names[j]);
            wb->pixbufs[i][j] = pixbuf_alpha_load (themedir, imagename);
        }
    }
}


gchar *button_layout_filter  (const gchar *button_layout)
{
    gint i, j;
    gchar filtered_layout[4] = {0};

    j = 0;
    for (i = 0; i < strlen (button_layout); i++)
    {
        switch (button_layout[i])
        {
            case 'H':
                filtered_layout[j++] = 'H';
                break;
            case 'M':
                filtered_layout[j++] = 'M';
                break;
            case 'C':
                filtered_layout[j++] = 'C';
        }
    }
    filtered_layout[j] = '\0';

    return g_strdup (filtered_layout);
}


static int getButtonFromLetter (char chr)
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


/* Replace buttons accordingly to button_layout and visible state */
static void replace_buttons (const gchar *button_layout, WBPlugin *wb)
{
    gint i, j, button;

    j = 0;
    for (i = 0; i < strlen (button_layout); i++)
    {
        button= getButtonFromLetter (button_layout[i]);
        if (button >= 0 && wb->button[i]->visible)
        {
            gtk_box_reorder_child (GTK_BOX (wb->hvbox), GTK_WIDGET(wb->button[button]->eventbox), j++);
            gtk_widget_show_all(GTK_WIDGET(wb->button[button]->eventbox));
        }
        else {
            gtk_widget_hide_all(GTK_WIDGET(wb->button[button]->eventbox));
        }
    }
}


/* load the theme according to xfwm4 theme format */
void loadTheme (WBPlugin *wb)
{
    gint i;
    gchar *themedir;

    /* get theme dir */
    themedir = getThemeDir (wb->prefs->theme, DEFAULT_THEME);

    if (strcmp (g_path_get_basename (themedir), "unity") == 0)
        get_unity_pixbuf (themedir, wb);
    else
        get_wm_pixbuf (themedir, wb);

    g_free (themedir);

    /* try to replace missing images */

    for (i = 0; i < BUTTONS; i++)
    {
        if (G_LIKELY(wb->pixbufs[i][IMAGE_FOCUSED]))
            wb->button[i]->visible = TRUE;
        else
            wb->button[i]->visible = FALSE;
    }

    for (i = 0; i < IMAGES_STATES; i++)
    {
        if (!wb->pixbufs[IMAGE_UNMAXIMIZE][i])
            wb->pixbufs[IMAGE_UNMAXIMIZE][i] = wb->pixbufs[IMAGE_MAXIMIZE][i];
    }
    for (i = 0; i < IMAGES_BUTTONS; i++)
    {
        if (!wb->pixbufs[i][IMAGE_UNFOCUSED] || !wb->pixbufs[i][IMAGE_PRESSED])
            wb->pixbufs[i][IMAGE_UNFOCUSED] = wb->pixbufs[i][IMAGE_FOCUSED];

        if (!wb->pixbufs[i][IMAGE_PRELIGHT])
            wb->pixbufs[i][IMAGE_PRELIGHT] = wb->pixbufs[i][IMAGE_PRESSED];
    }

    /* Replace buttons accordingly to button_layout and visible state */
    replace_buttons(wb->prefs->button_layout, wb);
}
