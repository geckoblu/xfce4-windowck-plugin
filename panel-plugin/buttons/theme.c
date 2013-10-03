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
 *  This stuff is mostly derived from xfwm4 sources (setting.c)
 *
 */

#include <libxfce4util/libxfce4util.h>
#include <common/ui_style.h>
#include <common/mypixmap.h>

#include "theme.h"

#define XPM_COLOR_SYMBOL_SIZE 22
#define THEMERC "themerc"

gchar *
getSystemThemeDir (void)
{
    return g_build_filename (DATADIR, "themes", DEFAULT_THEME, "xfwm4", NULL);
}

gchar *
getThemeDir (const gchar * theme, const gchar * file)
{
    static const char *themedir[] = {
      "windowck",
      "xfwm4"
    };

    if (!theme)
    {
        return g_build_filename (DATADIR, "themes", DEFAULT_THEME, "xfwm4",
                                 NULL);
    }
    else if (g_path_is_absolute (theme))
    {
        if (g_file_test (theme, G_FILE_TEST_IS_DIR))
        {
            return g_strdup (theme);
        }
        else
        {
            return getSystemThemeDir ();
        }
    }
    else
    {
        gchar *test_file, *path;
        gint i;

        for (i=0; i<2; i++) {

            path = g_build_filename (theme, themedir[i], file, NULL);

            xfce_resource_push_path (XFCE_RESOURCE_THEMES,
                                     DATADIR G_DIR_SEPARATOR_S "themes");
            test_file = xfce_resource_lookup (XFCE_RESOURCE_THEMES, path);
            xfce_resource_pop_path (XFCE_RESOURCE_THEMES);

            g_free (path);

            if (test_file)
            {
                path = g_path_get_dirname (test_file);
                g_free (test_file);
                return path;
            }
        }
    }

    /* Pfew, really can't find that theme nowhere! */
    return getSystemThemeDir ();
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

gboolean
setStringValue (const gchar * lvalue, const gchar *value, Settings *rc)
{
    GValue tmp_val = {0, };
    g_value_init(&tmp_val, G_TYPE_STRING);
    g_value_set_static_string(&tmp_val, value);
    return setGValue (lvalue, &tmp_val, rc);
}

/* load the theme according to xfwm4 theme format */
void loadTheme (WBPlugin *wb)
{
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

    /* use xfwm4 buttons naming */
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

    gint i,j;
    gchar imagename[30];
    gchar *theme;
    const char *spec;
    xfwmColorSymbol colsym[ XPM_COLOR_SYMBOL_SIZE + 1 ];

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

    /* get theme filename */
    theme = getThemeDir (wb->prefs->theme, THEMERC);

    for (i = 0; i < IMAGES_BUTTONS; i++)
    {
        for (j = 0; j < IMAGES_STATES; j++)
        {
            g_snprintf(imagename, sizeof (imagename), "%s-%s", button_names[i], button_state_names[j]);
            wb->pixbufs[i][j] = xfwmPixbufLoad (theme, imagename, colsym);
        }
    }

    /* try to replace missing images */
    if (wb->pixbufs[IMAGE_UNMAXIMIZE][IMAGE_FOCUSED] == NULL)
        wb->pixbufs[IMAGE_UNMAXIMIZE][IMAGE_FOCUSED] = wb->pixbufs[IMAGE_MAXIMIZE][IMAGE_FOCUSED];
    for (i = 0; i < IMAGES_BUTTONS; i++)
    {
        if (wb->pixbufs[i][IMAGE_UNFOCUSED] == NULL)
            wb->pixbufs[i][IMAGE_UNFOCUSED] = wb->pixbufs[IMAGE_FOCUSED][IMAGE_PRELIGHT];

        if (wb->pixbufs[i][IMAGE_PRESSED] == NULL)
            wb->pixbufs[i][IMAGE_PRESSED] = wb->pixbufs[i][IMAGE_FOCUSED];

        if (wb->pixbufs[i][IMAGE_PRELIGHT] == NULL)
            wb->pixbufs[i][IMAGE_PRELIGHT] = wb->pixbufs[i][IMAGE_PRESSED];
    }
}
