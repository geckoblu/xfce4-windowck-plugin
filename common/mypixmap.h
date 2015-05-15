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


        xfwm4-windowck-plugin - (c) 2013 Cedric leporcq
 */

#include <libxfce4util/libxfce4util.h>
#include <gdk/gdk.h>

typedef struct
{
    gchar *name;
    const gchar *value;
}
xfwmColorSymbol;

GdkPixbuf *             pixbuf_load (const gchar *, const gchar *, xfwmColorSymbol *);
GdkPixbuf *	            pixbuf_alpha_load (const gchar * dir, const gchar * file);
