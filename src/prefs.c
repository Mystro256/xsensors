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
extern int usedefaulttheme;
extern GdkPixbuf *theme;
extern GtkWidget *mainwindow;
extern char *home_dir;
extern cairo_surface_t *surface;

GdkPixbuf *temptheme = NULL;
GtkWidget *prefwindow = NULL;
GtkWidget *uptmwidget = NULL;
GtkWidget *timewidget = NULL;
GtkWidget *warnwidget = NULL;
GtkWidget *undowidget = NULL;
GtkWidget *applywidget = NULL;
GtkWidget *defaultwidget = NULL;
#if GTK_MAJOR_VERSION == 2
GtkWidget *drawwidget = NULL;
#endif

/* Event function to draw theme preview. */
#if GTK_MAJOR_VERSION == 2
gboolean draw_preview( GtkWidget *widget, GdkEventExpose *event,
                       gpointer data )
#else
gboolean draw_preview( GtkWidget *widget, cairo_t *cr, gpointer data )
#endif
{
#if GTK_MAJOR_VERSION == 2
    cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( widget ) );
    gdk_window_clear_area( widget->window, event->area.x, event->area.y,
                           event->area.width, event->area.height );
#endif

    const gchar digits[] = "3.40V*28.7C*83.6F*\n"
                           "3.40V*28.7C*83.6F*\n"
                           "591R *591R * -* -";
    const gchar *digit = digits;
    int highLow = 0;
    int posx = 0, posy = 0, x = 0, y = 0, w = 0;

    while ( *digit ) {
        switch ( *digit ) {
            case '*':
                highLow = highLow ? 0 : 30; break;
            case '\n':
                posx = 0; posy += 30; break;
            default:
                get_pm_location( *digit, &x, &y, &w );
                gdk_cairo_set_source_pixbuf( cr, temptheme, posx - x,
                                             posy -( y + highLow ) );
                cairo_rectangle( cr, posx, posy, w, 30 );
                cairo_fill( cr );
                posx += w;
        }
        digit++;
    }
#if GTK_MAJOR_VERSION == 2
    cairo_destroy( cr );
#endif
    return TRUE;
}

gint destroy_prefs( GtkWidget *widget, gpointer data )
{
    struct stat sbuf;
    FILE * fileconf;
    char temp_str[ 100 ];

    sprintf( temp_str, "%s/.local/share/%s/",
             home_dir, PACKAGE );
    if ( stat( temp_str, &sbuf ) != 0 ) {
        if ( mkdir( temp_str, 0700 ) != 0 ) {
            fprintf( stderr, "Unable to create configuration!\n"
                     "Failed to create directory %s\n"
                     "Error Number: %d", temp_str, errno );
            GtkWidget *dialog = gtk_message_dialog_new(
                                          GTK_WINDOW (prefwindow),
                                          GTK_ERROR_DIALOG_FLAGS,
                                          "Unable to create configuration!\n\n"
                                          "Failed to create directory %s\n"
                                          "Error Number: %d",
                                          temp_str, errno );
            gtk_dialog_run( GTK_DIALOG (dialog) );
            gtk_widget_destroy( dialog );
            return (FALSE);
        }
    }

    strcat( temp_str, "custom.ini" );
    fileconf = fopen( temp_str, "w" );
    if ( fileconf == NULL ) {
        fprintf( stderr, "Could not save preferences!\n"
                 "Failed to save preferences to %s\n"
                 , temp_str );
        GtkWidget *dialog = gtk_message_dialog_new( GTK_WINDOW (prefwindow),
                                          GTK_ERROR_DIALOG_FLAGS,
                                          "Could not save preferences!\n\n"
                                          "Failed to save preferences to %s\n"
                                          , temp_str );
        gtk_dialog_run( GTK_DIALOG (dialog) );
        gtk_widget_destroy( dialog );
        return (FALSE);
    }

    fprintf( fileconf, "[%s]\n", PACKAGE );
    fputs( "; Display all temperatures in Fahrenheit (0 or 1):\n", fileconf );
    fprintf( fileconf, "use_fahrenheit=%d\n", tf );
    fputs( "; Specify the update time in number of seconds "
           "(eg. 0, 1, 2), 0 for no update:\n", fileconf );
    fprintf( fileconf, "update_time=%d\n",
             gtk_spin_button_get_value_as_int(
                GTK_SPIN_BUTTON( timewidget ) ) );

    if ( temptheme != theme )
        g_object_unref( temptheme );

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
                                               GTK_SPIN_BUTTON(timewidget) );
    if ( state ) {
        if ( update_time)
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(timewidget),
                                       update_time );
        else
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(timewidget), 1);
        gtk_widget_set_sensitive( timewidget, TRUE );
    } else if ( value ) {
        gtk_spin_button_set_value( GTK_SPIN_BUTTON(timewidget), 0);
        gtk_widget_set_sensitive( timewidget, FALSE );
    }

    return (FALSE);
}

gint open_theme_dialog( GtkWidget *widget, gpointer data )
{
    GtkWidget *dialog;
    GtkFileFilter *filter;
    dialog = gtk_file_chooser_dialog_new( "Open New Theme",
                                          GTK_WINDOW (prefwindow),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          "_Cancel", GTK_RESPONSE_CANCEL,
                                          "_Select", GTK_RESPONSE_ACCEPT,
                                          NULL );
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pixbuf_formats( filter );
    gtk_file_filter_set_name( filter, "Supported File Types" );
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER (dialog), filter );
    if ( gtk_dialog_run( GTK_DIALOG (dialog) ) == GTK_RESPONSE_ACCEPT )
    {
        char *filename;
        GdkPixbuf *temp;

        filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (dialog) );
        temp = gdk_pixbuf_new_from_file_at_scale( filename, 216, 180, FALSE,
                                                  NULL );
        gtk_widget_destroy( dialog );
        if ( temp ) {
            if ( temptheme != theme )
                g_object_unref( temptheme );
            temptheme = temp;
#if GTK_MAJOR_VERSION == 2
            gtk_widget_queue_draw_area( drawwidget, 0, 0, 351, 90 );
#endif
            gtk_widget_set_sensitive( undowidget, TRUE );
            gtk_widget_set_sensitive( applywidget, TRUE );
            gtk_widget_set_sensitive( defaultwidget, TRUE );
        } else {
            fprintf( stderr, "Unable to import theme: %s\n", filename );
            GtkWidget *warning = gtk_message_dialog_new(
                                          GTK_WINDOW (prefwindow),
                                          GTK_ERROR_DIALOG_FLAGS,
                                          "Unable to import theme:\n%s",
                                          filename );
            gtk_dialog_run( GTK_DIALOG (warning) );
            gtk_widget_destroy( warning );
        }
        g_free( filename );
    } else {
        gtk_widget_destroy( dialog );
    }
    return (FALSE);
}

gint undo_callback( GtkWidget *widget, gpointer data )
{
    temptheme = theme;
#if GTK_MAJOR_VERSION == 2
    gtk_widget_queue_draw_area( drawwidget, 0, 0, 351, 90 );
#endif
    gtk_widget_set_sensitive( undowidget, FALSE );
    gtk_widget_set_sensitive( applywidget, FALSE );
    gtk_widget_set_sensitive( defaultwidget, !usedefaulttheme );
    return (FALSE);
}

gint apply_callback( GtkWidget *widget, gpointer data )
{
    struct stat sbuf;
    char *filename;

    /* alloc some memory for filename string */
    if ( ( filename = g_malloc( sizeof( char ) *
                       ( strlen( home_dir ) + sizeof( PACKAGE ) +
                         sizeof( "/.local/share/theme.tiff" ) ) ) ) == NULL ) {
        fputs( "malloc failed!\n", stderr );
        GtkWidget *dialog = gtk_message_dialog_new(
                                                GTK_WINDOW (mainwindow),
                                                GTK_ERROR_DIALOG_FLAGS,
                                                "Memory allocation error!\n\n"
                                                "Failed apply theme." );
        gtk_dialog_run( GTK_DIALOG (dialog) );
        gtk_widget_destroy( dialog );
        exit( 1 );
    }

    sprintf( filename, "%s/.local/share/%s/theme.tiff", home_dir, PACKAGE );
    if ( stat( filename, &sbuf ) == 0 )
        remove( filename );/*TODO ERROR*/

    if ( gtk_widget_get_sensitive( defaultwidget ) )
        if ( gdk_pixbuf_save( temptheme, filename, "tiff", NULL, NULL)
                == FALSE ) {
            fprintf( stderr, "Could not save theme!\n"
                     "Failed to save theme to %s\n"
                     , filename );
            GtkWidget *dialog = gtk_message_dialog_new(
                                                GTK_WINDOW (prefwindow),
                                                GTK_ERROR_DIALOG_FLAGS,
                                                "Could not save theme!\n\n"
                                                "Failed to save theme to %s\n"
                                                , filename );
            gtk_dialog_run( GTK_DIALOG (dialog) );
            gtk_widget_destroy( dialog );
            return (FALSE);
        }

    theme = temptheme;
#if GTK_MAJOR_VERSION == 2
    gtk_widget_queue_draw_area( drawwidget, 0, 0, 351, 90 );
#endif
    gtk_widget_set_sensitive( undowidget, FALSE );
    gtk_widget_set_sensitive( applywidget, FALSE );
    usedefaulttheme = !gtk_widget_get_sensitive( defaultwidget );
    return (FALSE);
}

gint setdefault_callback( GtkWidget *widget, gpointer data )
{
    char *filename;
    GdkPixbuf *temp;

    /* alloc some memory for filename string */
    if ( ( filename = g_malloc( sizeof( char ) *
                       ( sizeof( DATADIR ) + sizeof( PACKAGE ) +
                         sizeof( "theme.tiff" ) ) ) ) == NULL ) {
        fputs( "malloc failed!\n", stderr );
        GtkWidget *dialog = gtk_message_dialog_new(
                                                GTK_WINDOW (mainwindow),
                                                GTK_ERROR_DIALOG_FLAGS,
                                                "Memory allocation error!\n\n"
                                                "Failed import theme." );
        gtk_dialog_run( GTK_DIALOG (dialog) );
        gtk_widget_destroy( dialog );
        exit( 1 );
    }

    sprintf( filename, "%s/%s/theme.tiff", DATADIR, PACKAGE );
    temp = gdk_pixbuf_new_from_file( filename, NULL );
    if ( temp ) {
        if ( temptheme != theme )
            g_object_unref( temptheme );
        temptheme = temp;
#if GTK_MAJOR_VERSION == 2
        gtk_widget_queue_draw_area( drawwidget, 0, 0, 351, 90 );
#endif
        gtk_widget_set_sensitive( undowidget, !usedefaulttheme );
        gtk_widget_set_sensitive( applywidget, !usedefaulttheme );
        gtk_widget_set_sensitive( defaultwidget, FALSE );
    } else {
        fprintf( stderr, "Unable to import default theme: %s\n", filename );
        GtkWidget *dialog = gtk_message_dialog_new(
                                         GTK_WINDOW (prefwindow),
                                         GTK_ERROR_DIALOG_FLAGS,
                                         "Unable to import default theme:\n%s",
                                         filename );
        gtk_dialog_run( GTK_DIALOG (dialog) );
        gtk_widget_destroy( dialog );
    }
    free( filename );

    return (FALSE);
}

gboolean prefs_callback( GtkWidget *widget, GdkEvent *event )
{
    GtkWidget *notebook, *notelabel, *noteframe,
              *vbox,     *hbox,      *tmpwidget;

    /* Set temptheme to theme */
    temptheme = theme;

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
    gtk_box_pack_start( GTK_BOX (hbox), vbox, FALSE, FALSE, 8 );
    gtk_widget_show( vbox );

    gtk_notebook_append_page( GTK_NOTEBOOK (notebook), noteframe, notelabel );

    /* Add checkbox for temp units */
    tmpwidget = gtk_check_button_new_with_label( "Display all temperatures "
                                                    "in Fahrenheit" );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(tmpwidget), tf );
    g_signal_connect( G_OBJECT (tmpwidget), "toggled",
                      G_CALLBACK (set_tf), NULL );
    gtk_box_pack_start( GTK_BOX (vbox), tmpwidget, FALSE, FALSE, 16 );
    gtk_widget_show( tmpwidget );

    /* Add checkbox for enabling updates */
    uptmwidget = gtk_check_button_new_with_label(
                                                "Continually update values" );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(uptmwidget), update_time );
    g_signal_connect( G_OBJECT (uptmwidget), "toggled",
                      G_CALLBACK (toggle_updates), NULL );
    gtk_box_pack_start( GTK_BOX (vbox), uptmwidget, FALSE, FALSE, 8 );
    gtk_widget_show( uptmwidget );

    /* Add spinbutton for update time */
    tmpwidget = gtk_label_new( "Update time in number of seconds: " );
    gtk_box_pack_start( GTK_BOX (vbox), tmpwidget, FALSE, FALSE, 0 );
    gtk_widget_show( tmpwidget );

    timewidget = gtk_spin_button_new_with_range( 0, ((guint)-1)/1000, 1 );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(timewidget), update_time );
    gtk_widget_set_sensitive( timewidget, update_time );
    g_signal_connect( G_OBJECT (timewidget), "changed",
                      G_CALLBACK (check_update_time), NULL );
    gtk_box_pack_start( GTK_BOX (vbox), timewidget, FALSE, FALSE, 0 );
    gtk_widget_show( timewidget );

    /* Warning for updates requiring restart */
    warnwidget = gtk_label_new( NULL );
    gtk_label_set_markup( GTK_LABEL(warnwidget),
                          "<span fgcolor='#FF0000'>Restart application for "
                          "change to take effect</span>" );
    gtk_box_pack_start( GTK_BOX (vbox), warnwidget, FALSE, FALSE, 8 );

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
    gtk_box_pack_start( GTK_BOX (hbox), vbox, FALSE, FALSE, 8 );
    gtk_widget_show( vbox );

    gtk_notebook_append_page( GTK_NOTEBOOK (notebook), noteframe, notelabel );

    /* Warning for updates requiring restart */
    tmpwidget = gtk_label_new( "Preview theme:" );
    gtk_box_pack_start( GTK_BOX (vbox), tmpwidget, FALSE, FALSE, 8 );
    gtk_widget_show( tmpwidget );

    /* Theme Preview */
    tmpwidget = gtk_drawing_area_new();
    gtk_widget_set_size_request( tmpwidget, 351, 90 );
#if GTK_MAJOR_VERSION == 2
    GdkColor colorWhite = { 0, 0xFFFF, 0xFFFF, 0xFFFF };
    gtk_widget_modify_bg( tmpwidget, GTK_STATE_NORMAL, &colorWhite );
    drawwidget = tmpwidget;
    g_signal_connect( G_OBJECT(tmpwidget), "expose_event",
#else
    g_signal_connect( G_OBJECT(tmpwidget), "draw",
#endif
                              G_CALLBACK(draw_preview), NULL );
    gtk_box_pack_start( GTK_BOX (vbox), tmpwidget, FALSE, FALSE, 8 );
    gtk_widget_show( tmpwidget );

    /* Change theme */
    tmpwidget = gtk_button_new_with_label( "Select theme" );
    gtk_box_pack_start( GTK_BOX (vbox), tmpwidget, FALSE, FALSE, 8 );
    g_signal_connect( G_OBJECT (tmpwidget), "clicked",
                      G_CALLBACK (open_theme_dialog), NULL );
    gtk_widget_show( tmpwidget );

    /* Use default theme */
    defaultwidget = gtk_button_new_with_label( "Use default theme" );
    gtk_box_pack_start( GTK_BOX (vbox), defaultwidget, FALSE, FALSE, 8 );
    g_signal_connect( G_OBJECT (defaultwidget), "clicked",
                      G_CALLBACK (setdefault_callback), NULL );
    gtk_widget_set_sensitive( defaultwidget, !usedefaulttheme );
    gtk_widget_show( defaultwidget );

    /* Add Undo/Apply buttons */
#if GTK_MAJOR_VERSION == 2
    hbox = gtk_hbox_new( FALSE, 0 );
#else
    hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
#endif
    gtk_box_pack_start( GTK_BOX (vbox), hbox, FALSE, FALSE, 8 );
    gtk_widget_show( hbox );

    undowidget = gtk_button_new_with_label( "Undo" );
    gtk_box_pack_start( GTK_BOX (hbox), undowidget, FALSE, FALSE, 0 );
    g_signal_connect( G_OBJECT (undowidget), "clicked",
                      G_CALLBACK (undo_callback), NULL );
    gtk_widget_set_sensitive( undowidget, FALSE );
    gtk_widget_show( undowidget );

    tmpwidget = gtk_label_new( NULL );
    gtk_box_pack_start( GTK_BOX (hbox), tmpwidget, TRUE, TRUE, 0 );
    gtk_widget_show( tmpwidget );

    applywidget = gtk_button_new_with_label( "Apply theme" );
    gtk_box_pack_end( GTK_BOX (hbox), applywidget, FALSE, FALSE, 0 );
    g_signal_connect( G_OBJECT (applywidget), "clicked",
                      G_CALLBACK (apply_callback), NULL );
    gtk_widget_set_sensitive( applywidget, FALSE );
    gtk_widget_show( applywidget );

    /* Setup the main components. */
    gtk_widget_show( prefwindow );

    return SUCCESS;
}
