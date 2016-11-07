/*
    gui.c - Part of xsensors

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

#include "main.h"
#include "prefs.h"

extern int tf;
extern int update_time;
extern char *imagefile;
extern char *home_dir;

GtkWidget *mainwindow = NULL;

GdkPixbuf *theme = NULL;
GdkPixbuf *icon = NULL;

cairo_surface_t *surface = NULL;

/* Destroy the main window. */
gint destroy_gui( GtkWidget *widget, gpointer data )
{
    gtk_main_quit();

    return (FALSE);
}

/* About Dialog */
gboolean about_callback( GtkWidget *widget, GdkEvent *event )
{
    char* authors [4] = {"Kris Kersey",
                         "Jeremy Newton (mystro256)",
                         "with patches from Nanley Chery",NULL};

    gtk_show_about_dialog( GTK_WINDOW (mainwindow),
                           "version", VERSION,
                           "copyright", COPYRIGHT,
                           "comments", "A GTK interface to lm_sensors",
                           "website", "https://github.com/Mystro256/xsensors",
                           "authors", authors,
                           "license", GPL2PLUS,
                           "logo", icon,
                           "title", "About",
                           NULL );

    return SUCCESS;
}

/* Get the position and width of a character */
static void get_pm_location( gchar curInt, int *x, int *y, int *w )
{
    switch ( curInt ) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            *x = 18 * (curInt - '0');
            *y = 0;
            *w = 18;
            break;
        case ' ':
            *x = 198;
            *y = 0;
            *w = 18;
            break;
        case '-':
            *x = 180;
            *y = 0;
            *w = 18;
            break;
        case '.':
        case ',':
            *x = 171;
            *y = 60;
            *w = 6;
            break;
    }
}

static void draw_digits( GtkWidget *widget, cairo_t *cr, const gchar *digits,
                         int highLow )
{
    const gchar *digit = digits;
    int pos = 0, x = 0, y = 0, w = 0;

    while ( *digit ) {
        get_pm_location( *digit, &x, &y, &w );
        cairo_set_source_surface ( cr, surface, pos - x, 0 - ( y + highLow ) );
        cairo_rectangle( cr, pos, 0, w, 30 );
        cairo_fill( cr );
        pos += w;
        digit++;
    }
}

/* Event function to draw graphical numbers. */
#if GTK_MAJOR_VERSION == 2
gboolean expose_event_callback( GtkWidget *widget, GdkEventExpose *event,
                                gpointer data )
#else
gboolean draw_callback( GtkWidget *widget, cairo_t *cr, gpointer data )
#endif
{
    int x = 0;
    int highLow = 0;
    gfloat temp;

    updates *current = data;

    gchar result[7];

#if GTK_MAJOR_VERSION == 2
    cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( widget ) );

#ifdef DEBUG_XSENSORS
    printf( "area.width = %d, area.height = %d\n", event->area.width,
            event->area.height );
#endif
    gdk_window_clear_area( widget->window, event->area.x, event->area.y,
                           event->area.width, event->area.height );
#endif

    switch ( current->feattype ) {
        case FAN:
            if ( current->curvalue < current->curmin )
                highLow = 30;

            /* Display the digits */
            if ( g_snprintf( result, 6, "%5.0f", current->curvalue ) >= 0 )
                draw_digits( widget, cr, result, highLow );

            /* Display RPM */
            cairo_set_source_surface ( cr, surface, 90 - 0,
                                       0 - ( 120 + highLow ) );
            cairo_rectangle( cr, 90, 0, 57, 30 );
            break;
        case TEMP:
            if ( current->curvalue > current->curmax )
                highLow = 30;

            if ( tf == TRUE )
                temp = ( 1.8 * current->curvalue ) + 32;
            else
                temp = current->curvalue;

            /* Display the digits */
            if ( g_snprintf( result, 7, "%6.1f", temp ) >= 0 )
                draw_digits( widget, cr, result, highLow );

            /* Display degree symbol */
            if ( tf == FALSE )
                x = 0;
            else
                x = 57;
            cairo_set_source_surface ( cr, surface, 96 - x,
                                       0 - ( 60 + highLow ) );
            cairo_rectangle( cr, 96, 0, 57, 30 );

            break;
        case VOLT:
            if ( current->curvalue > current->curmax ||
                 current->curvalue < current->curmin )
                highLow = 30;

            /* Display the digits */
            if ( g_snprintf( result, 7, "%6.2f", current->curvalue ) >= 0 )
                draw_digits( widget, cr, result, highLow );

            /* Display V */
            cairo_set_source_surface ( cr, surface, 96 - 114,
                                       0 - ( 60 + highLow ) );
            cairo_rectangle( cr, 96, 0, 57, 30 );
            break;
        default:
            break;
    }
    cairo_fill(cr);
#if GTK_MAJOR_VERSION == 2
    cairo_destroy(cr);
#endif
    return TRUE;
}

/* Free the link list. */
gint free_llist( updates *node )
{
    if ( node != NULL ) {
        free_llist( node->next );
        g_free( node );
    }

    return SUCCESS;
}

/* Find the tail of a non-NULL linked list. */
static updates *llist_tail( updates *node )
{
    if ( node->next == NULL )
        return node;
    else
        return llist_tail( node->next );
}

/* Update the sensor information. */
gint update_sensor_data( gpointer data )
{
    updates *updata = data;

    gfloat percent = 0;

    do {
        if ( sensors_get_value( updata->name, updata->featnum,
             &(updata->curvalue) ) != 0 ) {
            updata->curvalue = 0;
        }

        if ( updata->featminnum != UNDEFMAXMIN ) {
            sensors_get_value( updata->name, updata->featminnum,
                               &(updata->curmin) );
        } else {
            updata->curmin = 0;
        }

        if ( updata->featmaxnum != UNDEFMAXMIN ) {
            sensors_get_value( updata->name, updata->featmaxnum,
                               &(updata->curmax) );
        } else {
            updata->curmax = 10000;
        }

        if (updata->curmax != updata->curmin ) {
            percent = ( ( updata->curvalue - updata->curmin ) /
                        ( updata->curmax - updata->curmin ) );
        } else {
            percent = 0.0;
        }
        /* Negative voltages may have their limits swapped depending on
           the chip wiring */
        if ( updata->feattype == SENSORS_FEATURE_IN &&
             updata->curmin < 0 && updata->curmax < 0 &&
             updata->curmax < updata->curmin ) {
            double tmp = updata->curmax;
            updata->curmax = updata->curmin;
            updata->curmin = tmp;

#ifdef DEBUG_XSENSORS
            printf( "negative voltage, swapping curmin and curmax\n" );
#endif
        }

#ifdef DEBUG_XSENSORS
        printf( "curvalue = %f, curmax = %f, curmin = %f, percent = %f\n",
                updata->curvalue, updata->curmax, updata->curmin, percent );
#endif

        if ( percent < 0 )
            percent = 0;

        if ( percent > 1 )
            percent = 1;

        gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR (updata->pbar),
                                       percent );

        gtk_widget_queue_draw_area( updata->darea, 0, 0, 153, 30 );

        updata = updata->next;

    } while ( updata != NULL );

#ifdef DEBUG_XSENSORS
    printf( "\n" );
#endif

    return (TRUE);
}

/* Start the sensor info update timer. */
gint start_timer( GtkWidget *widget, gpointer data )
{
    /* Setup timer for updates. */
    if( update_time )
        g_timeout_add( update_time * 1000,
                       (GSourceFunc) update_sensor_data,
                       (gpointer) data );

    return SUCCESS;
}

updates *add_sensor_tab( GtkWidget *container, const sensors_chip_name *name )
{
    /* packing boxes */
    GtkWidget *mainbox, *voltbox, *tempbox, *fanbox;

    GtkWidget *currbox = NULL;
    GtkWidget *innerbox = NULL;

    /* notebook stuff */
    GtkWidget *notelabel, *noteframe, *notescroll;

    /* main feature labels */
    GtkWidget *voltlabel, *templabel, *fanlabel;

    /* feature data */
    updates *head = NULL;
    updates *current = NULL;

    const sensors_feature *feature;

    /* several needed ints */
    int i = 0;
    int usedvolt = 0;
    int usedtemp = 0;
    int usedfan = 0;

    /* fields placed in the notebook to display feature info */
    GtkWidget *darea = NULL;
    char *feattext = NULL;
    GtkWidget *featframe = NULL;
    GtkWidget *featpbar = NULL;

    /* Setup main boxes. */
#if GTK_MAJOR_VERSION == 2
    mainbox = gtk_hbox_new( TRUE, 10 );
    voltbox = gtk_vbox_new( FALSE, 0 );
    tempbox = gtk_vbox_new( FALSE, 0 );
    fanbox = gtk_vbox_new( FALSE, 0 );
#else
    mainbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 10 );
    gtk_box_set_homogeneous ( GTK_BOX (mainbox), TRUE );
    voltbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    tempbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    fanbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#endif
    gtk_container_set_border_width( GTK_CONTAINER (mainbox), 10 );
    gtk_widget_show( mainbox );

    /* Create notebook for sensors. */
    noteframe = gtk_frame_new( NULL );
    gtk_container_set_border_width( GTK_CONTAINER (noteframe), 10 );
    gtk_widget_show( noteframe );

    notescroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW (notescroll),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    /*min height of 400px seems sensible*/
    gtk_widget_set_size_request( notescroll, -1, 400 );
    gtk_widget_show( notescroll );

    notelabel = gtk_label_new( name->prefix );
    gtk_widget_show( notelabel );

    gtk_container_add( GTK_CONTAINER (noteframe), notescroll );
#if GTK_MAJOR_VERSION == 2
    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW (notescroll),
                                           mainbox );
#else
    gtk_container_add( GTK_CONTAINER (notescroll), mainbox );
#endif
    gtk_notebook_append_page( GTK_NOTEBOOK (container), noteframe, notelabel );

    /* Create main labels. */
    voltlabel = gtk_label_new( "Voltages:" );
    templabel = gtk_label_new( "Temperatures:" );
    fanlabel = gtk_label_new( "Fans:" );
    gtk_box_pack_start( GTK_BOX (voltbox), voltlabel, FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX (tempbox), templabel, FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX (fanbox), fanlabel, FALSE, FALSE, 0 );

    /* Create labels and entry fields for features. */
    while ( (feature = sensors_get_features( name, &i )) ) {
            updates *new_node = add_node( name, feature );
            if ( new_node == NULL )
                continue;

            featframe = gtk_frame_new( NULL );
            featpbar = gtk_progress_bar_new();
            darea = gtk_drawing_area_new();
#if GTK_MAJOR_VERSION == 2
            innerbox = gtk_vbox_new( FALSE, 0 );
            GdkColor colorWhite = { 0, 0xFFFF, 0xFFFF, 0xFFFF };
            gtk_widget_modify_bg( darea, GTK_STATE_NORMAL, &colorWhite );
#else
            innerbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#endif

            gtk_widget_set_size_request( darea, 36, 30 );

            new_node->darea = darea;
            new_node->pbar = featpbar;

            if ( head == NULL ) {
                head = current = new_node;
            } else {
                current = current->next = new_node;
            }

            /* Connect the event signal handler to redraw the numbers. */
#if GTK_MAJOR_VERSION == 2
            g_signal_connect( G_OBJECT(darea), "expose_event",
                              G_CALLBACK(expose_event_callback), current );
#else
            g_signal_connect( G_OBJECT(darea), "draw",
                              G_CALLBACK(draw_callback), current );
#endif

            feattext = sensors_get_label( name, feature );

            if ( feattext != NULL ) {
                /* We need a temporary variable in case realloc fails */
                char *new_feattext;
#ifdef DEBUG_XSENSORS
                printf( "Adding feature %d, %s.\n", i, feattext );
#endif
                if ( ( new_feattext = realloc( feattext,
                                ( strlen( feattext ) + 2 ) *
                                sizeof( char ) ) ) == NULL ) {
                    fprintf( stderr, "realloc failed in add_sensor_tab()!\n" );
                    free( feattext );
                    GtkWidget *dialog = gtk_message_dialog_new(
                                                GTK_WINDOW (mainwindow),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_CLOSE,
                                                "Memory allocation error!\n\n"
                                                "Failed to create GTK "
                                                "Notebook." );
                    gtk_dialog_run( GTK_DIALOG (dialog) );
                    gtk_widget_destroy( dialog );
                    return NULL;
                }
                feattext = new_feattext;
                strcat( feattext, ":" );

                gtk_frame_set_label( GTK_FRAME (featframe), feattext );

                switch ( current->feattype ) {
                    case VOLT:
                        currbox = voltbox;
                        usedvolt++;
                        break;
                    case TEMP:
                        currbox = tempbox;
                        usedtemp++;
                        break;
                    case FAN:
                        currbox = fanbox;
                        usedfan++;
                        break;
                    default:
                        fprintf( stderr,
                                 "Type not recognized, not packing.\n" );
                        break;
                }
                gtk_box_pack_start( GTK_BOX (currbox), featframe,
                                    FALSE, FALSE, 0 );
                gtk_container_add (GTK_CONTAINER (featframe), innerbox);
                gtk_box_pack_start( GTK_BOX (innerbox), darea,
                                    FALSE, FALSE, 0 );

                gtk_box_pack_start( GTK_BOX (innerbox), featpbar,
                                    FALSE, FALSE, 0 );

            } else {
                gtk_frame_set_label( GTK_FRAME (featframe), feattext );
            }
            gtk_widget_show( featframe );
            gtk_widget_show( innerbox );
            gtk_widget_show( darea );
            gtk_widget_show( featpbar );
            free( feattext );
    }

    if ( usedvolt > 0 ) {
        gtk_widget_show( voltbox );
        gtk_box_pack_start( GTK_BOX (mainbox), voltbox, FALSE, FALSE, 0 );
        gtk_widget_show( voltlabel );
    }

    if ( usedtemp > 0 ) {
        gtk_widget_show( tempbox );
        gtk_box_pack_start( GTK_BOX (mainbox), tempbox, FALSE, FALSE, 0 );
        gtk_widget_show( templabel );
    }

    if ( usedfan > 0 ) {
        gtk_widget_show( fanbox );
        gtk_box_pack_start( GTK_BOX (mainbox), fanbox, FALSE, FALSE, 0 );
        gtk_widget_show( fanlabel );
    }

    return head;
}

static updates *add_sensor_chips( GtkWidget *notebook, const char *pattern )
{
    const sensors_chip_name *name = NULL, *pquery = NULL;

    updates *head = NULL, *tail = NULL, *new_nodes;

    int chipnum = 0;

#ifdef DEBUG_XSENSORS
    sensors_chip_name query;
    if ( pattern ) {
        if ( sensors_parse_chip_name( pattern, &query ) ) {
            fprintf( stderr,
                    "Couldn't parse chip name %s!  Exiting!\n",
                    pattern );
            return NULL;
        }
        pquery = &query;
    }
#endif

    while ( ( name = sensors_get_detected_chips( pquery,
                                                 &chipnum ) ) != NULL ) {
#ifdef DEBUG_XSENSORS
        printf( "Adding tab for %s\n", name->prefix );
#endif
        if ( ( new_nodes = add_sensor_tab( notebook, name ) ) == NULL )
            return head;

        update_sensor_data( new_nodes );
        g_signal_connect( G_OBJECT (mainwindow), "realize",
                          G_CALLBACK (start_timer), new_nodes );

        if ( head == NULL )
            head = new_nodes;
        else
            tail->next = new_nodes;
        tail = llist_tail( new_nodes );
    }

    return head;
}

int start_gui( int argc, char **argv )
{
    struct stat sbuf;
    char title [20];

    GtkWidget *mainbox, *menubar, *tempwgt, *notebook;

    updates *head = NULL;

    gtk_init( &argc, &argv );

    /* Setup main window. */
    mainwindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    g_snprintf( title, 20, "%s %s", PACKAGE, VERSION );
    gtk_window_set_title( GTK_WINDOW (mainwindow), title );
    g_signal_connect( G_OBJECT (mainwindow), "delete_event",
                      G_CALLBACK (destroy_gui), NULL );


    /* Set up the image file used for displaying characters. */
    if ( imagefile == NULL ) {

        /* find max size for string */
        gsize tempsize = sizeof( DATADIR );
        if ( tempsize < ( strlen( home_dir ) + sizeof( "/.local/share") ) )
            tempsize = strlen( home_dir ) + sizeof( "/.local/share");
        tempsize += sizeof( PACKAGE ) + sizeof( "theme.png" );

        /* alloc some memory for imagefile string */
        if ( ( imagefile = g_malloc( sizeof( char ) *
                           ( tempsize ) ) ) == NULL ) {
            fprintf( stderr, "malloc failed!\n" );
            GtkWidget *dialog = gtk_message_dialog_new(
                                                GTK_WINDOW (mainwindow),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_CLOSE,
                                                "Memory allocation error!\n\n"
                                                "Failed import theme." );
            gtk_dialog_run( GTK_DIALOG (dialog) );
            gtk_widget_destroy( dialog );
            exit( 1 );
        }

        /* Check home dir first */
        sprintf( imagefile, "%s/.local/share/%s/theme.png",
                 home_dir, PACKAGE );
        if ( stat( imagefile, &sbuf ) != 0 ) {

            /* Check system dir next */
            sprintf( imagefile, "%s/%s/theme.png", DATADIR, PACKAGE );
            if ( stat( imagefile, &sbuf ) != 0 ) {
                fprintf( stderr, "%s: %s/.local/share/%s/theme.png\n",
                         strerror( errno ) , home_dir, PACKAGE );
                fprintf( stderr, "%s: %s\n",
                         strerror( errno ), imagefile );
                fprintf( stderr,
                      "Image file not found in either location!  Exiting!\n" );
                GtkWidget *dialog = gtk_message_dialog_new(
                                                GTK_WINDOW (mainwindow),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_CLOSE,
                                                "Theme Import error!\n\n"
                                                "Could not find theme.png\n"
                                                "Please make sure it exists in"
                                                " one of these directories:"
                                                "\n\n%s/%s"
                                                "\n%s/.local/%s",
                                                DATADIR, PACKAGE,
                                                home_dir, PACKAGE );
                gtk_dialog_run( GTK_DIALOG (dialog) );
                gtk_widget_destroy( dialog );
                exit( 1 );
            }
        }
    } else {
        if ( stat( imagefile, &sbuf ) != 0 ) {
            fprintf( stderr, "%s: %s\n", strerror( errno ), imagefile );
            fprintf( stderr,
                    "Image file not found in specified location!  Exiting!\n" );
            exit( 1 );
        }
    }
    theme = gdk_pixbuf_new_from_file( imagefile, NULL );
    surface = cairo_image_surface_create_for_data(gdk_pixbuf_get_pixels(theme),
                                               CAIRO_FORMAT_RGB24,
                                               gdk_pixbuf_get_width(theme),
                                               gdk_pixbuf_get_height(theme),
                                               gdk_pixbuf_get_rowstride(theme));


    /* Import 128x128 icon if it's available */
    char iconfile [sizeof( DATADIR ) - 1 + sizeof( PACKAGE ) - 1 +
                            sizeof( "/icons/hicolor/128x128/apps/.png")];
    sprintf( iconfile, "%s/icons/hicolor/128x128/apps/%s.png",
             DATADIR, PACKAGE );
    if ( ( stat( iconfile, &sbuf ) ) == 0 )
        icon = gdk_pixbuf_new_from_file( iconfile, NULL );

    /* Create main vertical box */
#if GTK_MAJOR_VERSION == 2
    mainbox = gtk_vbox_new( FALSE, 0 );
#else
    mainbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#endif
    gtk_container_add( GTK_CONTAINER (mainwindow), mainbox );
    gtk_widget_show( mainbox );

    /* Create menu */
    /* TODO Make into appmenu for gtk3 */
    menubar = gtk_menu_new();

    tempwgt = gtk_menu_item_new_with_label( "Preferences" );
    gtk_menu_shell_append( GTK_MENU_SHELL (menubar), tempwgt );
    g_signal_connect( G_OBJECT (tempwgt), "activate",
                      G_CALLBACK (prefs_callback), NULL );
    gtk_widget_show( tempwgt );

    tempwgt = gtk_separator_menu_item_new();
    gtk_menu_shell_append( GTK_MENU_SHELL (menubar), tempwgt );
    gtk_widget_show( tempwgt );

    tempwgt = gtk_menu_item_new_with_label( "About" );
    gtk_menu_shell_append( GTK_MENU_SHELL (menubar), tempwgt );
    g_signal_connect( G_OBJECT (tempwgt), "activate",
                      G_CALLBACK (about_callback), NULL );
    gtk_widget_show( tempwgt );

    tempwgt = gtk_menu_item_new_with_label( "Quit" );
    gtk_menu_shell_append( GTK_MENU_SHELL (menubar), tempwgt );
    g_signal_connect( tempwgt, "activate",
                      G_CALLBACK (destroy_gui), NULL );
    gtk_widget_show( tempwgt );

    tempwgt = gtk_menu_item_new_with_label( "xsensors" );
    gtk_widget_show( tempwgt );
    gtk_menu_item_set_submenu( GTK_MENU_ITEM (tempwgt), menubar );

    menubar = gtk_menu_bar_new();
    gtk_box_pack_start( GTK_BOX (mainbox), menubar, FALSE, FALSE, 0 );
    gtk_widget_show( menubar );
    gtk_menu_shell_append( GTK_MENU_SHELL (menubar), tempwgt );

    /* Create notebook for sensors. */
    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos( GTK_NOTEBOOK (notebook), GTK_POS_LEFT );
    gtk_box_pack_end( GTK_BOX (mainbox), notebook, TRUE, TRUE, 0 );
    gtk_widget_show( notebook );

#ifdef DEBUG_XSENSORS
    if ( argc >= 2 ) {
        int i;
        for ( i = 1; i < argc; i++ ) {
            if ( argv[i][0] != '-' ) {
                head = add_sensor_chips( notebook, argv[i] );
                if ( head == NULL )
                    return FAILURE;
            } else {
                /* Skip another for all but -f */
                i += ( argv[i][1] != 'f' );
            }
        }
    }

    if ( head == NULL ) 
#endif
    {
        head = add_sensor_chips( notebook, NULL );
        if ( head == NULL )
            return FAILURE;
    }

    /* Setup the main components. */
    gtk_widget_show( mainwindow );

    gtk_main();

    free_llist( head );

    return SUCCESS;
}
