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
GtkWidget *uptmwidget = NULL;
GtkWidget *timewidget = NULL;
GtkWidget *warnwidget = NULL;

gint destroy_prefs( GtkWidget *widget, gpointer data )
{
    /*TODO Save preferences to a file*/
    return (FALSE);
}

gint set_tf( GtkWidget *widget, gpointer data )
{
    tf = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget) );
    return (FALSE);
}

gint check_update_time( GtkWidget *widget, gpointer data )
{
    int value = gtk_spin_button_get_value_as_int(
                                               GTK_SPIN_BUTTON( timewidget ) );
    if ( value != update_time )
        gtk_widget_show( warnwidget );
    else
        gtk_widget_hide( warnwidget );

    if ( value == 0 ) {
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(uptmwidget), FALSE );
        gtk_widget_set_sensitive( timewidget, FALSE );
    }

    return (FALSE);
}

gint toggle_updates( GtkWidget *widget, gpointer data )
{
    int state = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(uptmwidget) );
    int value = gtk_spin_button_get_value_as_int(
                                               GTK_SPIN_BUTTON( timewidget ) );
    if ( state ) {
        if ( update_time)
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(timewidget), update_time);
        else
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(timewidget), 1);
        gtk_widget_set_sensitive( timewidget, TRUE );
    } else if ( value ) {
        gtk_spin_button_set_value( GTK_SPIN_BUTTON(timewidget), 0);
        gtk_widget_set_sensitive( timewidget, FALSE );
    }

    return (FALSE);
}

gboolean prefs_callback( GtkWidget *widget, GdkEvent *event )
{
    GtkWidget *notebook, *notelabel, *noteframe,
              *vbox,     *hbox,      *tmpwidget;

    /* Setup main window. */
    prefwindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_transient_for( GTK_WINDOW (prefwindow),
                                  GTK_WINDOW (mainwindow) );
    gtk_window_set_position( GTK_WINDOW (prefwindow),
                             GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_destroy_with_parent( GTK_WINDOW (prefwindow), TRUE );
    gtk_window_set_resizable( GTK_WINDOW (prefwindow), FALSE );
    gtk_window_set_title( GTK_WINDOW (prefwindow), "Preferences" );
    g_signal_connect( G_OBJECT (prefwindow), "delete_event",
                      G_CALLBACK (destroy_prefs), NULL );

    /* Create notebook for prefs. */
    notebook = gtk_notebook_new();
    gtk_container_add( GTK_CONTAINER (prefwindow), notebook );
    gtk_widget_show( notebook );

    /* Create page for general prefs. */
    noteframe = gtk_frame_new( NULL );
    gtk_widget_show( noteframe );

    notelabel = gtk_label_new( "General" );
    gtk_widget_show( notelabel );

#if GTK_MAJOR_VERSION == 2
    hbox = gtk_hbox_new( FALSE, 0 );
    vbox = gtk_vbox_new( FALSE, 0 );
#else
    hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#endif
    gtk_container_add( GTK_CONTAINER (noteframe), hbox );
    gtk_widget_show( hbox );
    gtk_box_pack_start( GTK_BOX (hbox), vbox, FALSE, FALSE, 10 );
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

    /* Add checkbox for enabling updates */
    uptmwidget = gtk_check_button_new_with_label( "Continually update values" );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(uptmwidget), update_time );
    g_signal_connect( G_OBJECT (uptmwidget), "toggled",
                      G_CALLBACK (toggle_updates), NULL );
    gtk_box_pack_start( GTK_BOX (vbox), uptmwidget, FALSE, FALSE, 10 );
    gtk_widget_show( uptmwidget );

    /* Add spinbutton for update time */
#if GTK_MAJOR_VERSION == 2
    hbox = gtk_vbox_new( FALSE, 0 );
#else
    hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
#endif
    gtk_box_pack_start( GTK_BOX (vbox), hbox, FALSE, FALSE, 10 );
    gtk_widget_show( hbox );

    tmpwidget = gtk_label_new( "Update time in number of seconds: " );
    gtk_box_pack_start( GTK_BOX (hbox), tmpwidget, FALSE, FALSE, 0 );
    gtk_widget_show( tmpwidget );

    timewidget = gtk_spin_button_new_with_range( 0, ((guint)-1)/1000, 1 );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(timewidget), update_time );
    gtk_widget_set_sensitive( timewidget, update_time );
    g_signal_connect( G_OBJECT (timewidget), "changed",
                      G_CALLBACK (check_update_time), NULL );
    gtk_box_pack_start( GTK_BOX (hbox), timewidget, FALSE, FALSE, 0 );
    gtk_widget_show( timewidget );

    /* Warning for updates requiring restart */
    warnwidget = gtk_label_new( NULL );
    gtk_label_set_markup( GTK_LABEL(warnwidget),
                          "<span fgcolor='#FF0000'>Some preferences require "
                          "restarting to apply</span>" );
    gtk_box_pack_start( GTK_BOX (vbox), warnwidget, FALSE, FALSE, 10 );

    /* Create page for theme prefs. */
    noteframe = gtk_frame_new( NULL );
    gtk_widget_show( noteframe );

    notelabel = gtk_label_new( "Theme" );
    gtk_widget_show( notelabel );

#if GTK_MAJOR_VERSION == 2
    hbox = gtk_hbox_new( FALSE, 0 );
    vbox = gtk_vbox_new( FALSE, 0 );
#else
    hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#endif
    gtk_container_add( GTK_CONTAINER (noteframe), hbox );
    gtk_widget_show( hbox );
    gtk_box_pack_start( GTK_BOX (hbox), vbox, FALSE, FALSE, 10 );
    gtk_widget_show( vbox );

    gtk_notebook_append_page( GTK_NOTEBOOK (notebook), noteframe, notelabel );

    /* Setup the main components. */
    gtk_widget_show( prefwindow );

    return SUCCESS;
}
