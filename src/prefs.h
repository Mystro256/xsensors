/*
    pref.h - Part of xsensors

    Copyright (c) 2016 Jeremy Newton (mystro256)

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

#include <gdk/gdk.h>
#include <sys/stat.h>

/* Prototypes */
gint destroy_prefs( GtkWidget *, gpointer );
#if GTK_MAJOR_VERSION == 2
gboolean draw_preview( GtkWidget *, GdkEventExpose *, gpointer );
#else
gboolean draw_preview( GtkWidget *, cairo_t *, gpointer );
#endif
gboolean prefs_callback( GtkWidget *, GdkEvent * );
gint set_tf( GtkWidget *, gpointer );
gint toggle_updates( GtkWidget *, gpointer );
gint check_update_time( GtkWidget *, gpointer );
gint open_theme_dialog( GtkWidget *, gpointer );
gint undo_callback( GtkWidget *, gpointer );
gint apply_callback( GtkWidget *, gpointer );
gint setdefault_callback( GtkWidget *, gpointer );
