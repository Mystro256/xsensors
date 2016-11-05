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

#include "main.h"
#include "prefs.h"
extern int tf;
extern int update_time;
extern GdkPixbuf *theme;
extern GtkWidget *mainwindow;

GtkWidget *prefwindow = NULL;

gint destroy_prefs( GtkWidget *widget, gpointer data )
{
    /*TODO Save preferences to a file*/
    return (FALSE);
}

gint set_tf( GtkWidget *widget, gpointer data )
{
    /*TODO*/
    tf = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget) );
    return (FALSE);
}

gboolean prefs_callback( GtkWidget *widget, GdkEvent *event )
{
    GtkWidget *notebook = NULL;
    GtkWidget *notelabel = NULL;
    GtkWidget *noteframe = NULL;
    GtkWidget *vbox = NULL;
    GtkWidget *tmpwidget = NULL;

    /* Setup main window. */
    prefwindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_transient_for( GTK_WINDOW (prefwindow),
                                  GTK_WINDOW (mainwindow) );
    gtk_window_set_position( GTK_WINDOW (prefwindow),
                             GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_destroy_with_parent( GTK_WINDOW (prefwindow), TRUE );
    gtk_window_set_title( GTK_WINDOW (prefwindow), "Preferences" );
    g_signal_connect( G_OBJECT (prefwindow), "delete_event",
                      G_CALLBACK (destroy_prefs), NULL );

    /* Create notebook for prefs. */
    notebook = gtk_notebook_new();
#if GTK_MAJOR_VERSION == 2
    gtk_widget_modify_bg( notebook, GTK_STATE_NORMAL, &colorWhite );
#endif
    gtk_container_add( GTK_CONTAINER (prefwindow), notebook );
    gtk_widget_show( notebook );

    /* Create page for general prefs. */
    noteframe = gtk_frame_new( NULL );
    gtk_widget_show( noteframe );

    notelabel = gtk_label_new( "General" );
    gtk_widget_show( notelabel );

#if GTK_MAJOR_VERSION == 2
    vbox = gtk_vbox_new( FALSE, 0 );
#else
    vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#endif
    gtk_container_add( GTK_CONTAINER (noteframe), vbox );
    gtk_widget_show( vbox );

    gtk_notebook_append_page( GTK_NOTEBOOK (notebook), noteframe, notelabel );

    /* Add checkbox for temp units */
    tmpwidget = gtk_check_button_new_with_label( "Display all temperatures "
                                                    "in Fahrenheit" );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(tmpwidget), tf );
    g_signal_connect( G_OBJECT (tmpwidget), "toggled",
                      G_CALLBACK (set_tf), NULL );
    gtk_box_pack_start( GTK_BOX (vbox), tmpwidget, FALSE, FALSE, 10 );
    gtk_widget_show( tmpwidget );

    /* Setup the main components. */
    gtk_widget_show( prefwindow );

    return SUCCESS;
}
