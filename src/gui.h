/*
    gui.h - Part of xsensors

    Copyright (c) 2002-2007 Kris Kersey <augustus@linuxhardware.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, Boston, MA
    02110-1301  USA.
*/

#include <sys/stat.h>
#include <gdk/gdk.h>

enum { VOLT, TEMP, FAN };

#define NA "Not Available!"

/* Prototypes */
gint destroy_gui( GtkWidget *, gpointer );
#if GTK_MAJOR_VERSION == 2
gboolean expose_event_callback( GtkWidget *, GdkEventExpose *, gpointer );
#elif GTK_MAJOR_VERSION == 3
gboolean draw_callback( GtkWidget *, cairo_t *cr, gpointer );
#endif
gint free_llist( updates * );
gint update_sensor_data( gpointer );
gint start_timer( GtkWidget *, gpointer );
updates *add_sensor_tab( GtkWidget *, const sensors_chip_name * );
int start_gui( int, char ** );
