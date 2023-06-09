/*
    gui.h - Part of xsensors

    Copyright (c) 2002-2007 Kris Kersey <augustus@linuxhardware.org>
                  2012-2016 Jeremy Newton (mystro256)

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

enum { VOLT, TEMP, FAN };

#define GTK_ERROR_DIALOG_FLAGS GTK_DIALOG_DESTROY_WITH_PARENT,\
                                GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE

#define NA "Not Available!"

#define COPYRIGHT "© 2012-2023 Jeremy Newton, 2002-2007 Kris Kersey"

#define GPL2PLUS \
    "This program is free software; you can redistribute it and/or modify\n"\
    "it under the terms of the GNU General Public License as published by\n"\
    "the Free Software Foundation; either version 2 of the License, or\n"\
    "(at your option) any later version.\n"\
    "\n"\
    "This program is distributed in the hope that it will be useful,\n"\
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
    "GNU General Public License for more details.\n"\
    "\n"\
    "You should have received a copy of the GNU General Public License\n"\
    "along with this program; if not, write to the Free Software\n"\
    "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, Boston, MA\n"\
    "02110-1301  USA."

/* Public vars */
extern int usedefaulttheme;
extern GtkWidget *mainwindow;
extern GdkPixbuf *theme;

/* Prototypes */
gint destroy_gui( GtkWidget *, gpointer );
#if GTK_MAJOR_VERSION == 2
gboolean draw_callback( GtkWidget *, GdkEventExpose *, gpointer );
#else
gboolean draw_callback( GtkWidget *, cairo_t *, gpointer );
#endif
gboolean about_callback( GtkWidget *, GdkEvent * );
gboolean on_key_press_callback( GtkWidget *, GdkEventKey *, gpointer );
gint start_timer( GtkWidget *, gpointer );
void get_pm_location( gchar, int *, int *, int * );
int start_gui( int, char ** );
