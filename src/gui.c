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

extern int tf;
extern int update_time;
extern char *imagefile;

GtkWidget *mainwindow = NULL;

#if GTK_MAJOR_VERSION == 2
GdkColor colorWhite = { 0, 0xFFFF, 0xFFFF, 0xFFFF };
#elif GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION < 16
/*FIXME*/
#endif

GdkPixbuf *theme = NULL;

cairo_surface_t *surface = NULL;

/* Destroy the main window. */
gint destroy_gui( GtkWidget *widget, gpointer data ) {

    gtk_main_quit();

    return (FALSE);
}

/* Get the position and width of a character */
static void get_pm_location( gchar curInt, int *x, int *y, int *w ) {
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

static void draw_digits( GtkWidget *widget, cairo_t *cr, const gchar *digits, int highLow )
{
    const gchar *digit = digits;
    int pos = 0, x = 0, y = 0, w = 0;

    while ( *digit ) {
        get_pm_location( *digit, &x, &y, &w );
        cairo_set_source_surface (cr, surface, pos-x, 0-(y + highLow));
		cairo_rectangle(cr, pos, 0, w, 30);
		cairo_fill(cr);
        pos += w;
        digit++;
    }
}

/* Expose event function to draw graphical numbers. */
gboolean expose_event_callback( GtkWidget *widget, GdkEventExpose *event,
                                gpointer data) {
    int x = 0;
    int highLow = 0;

    updates *current = data;

    gchar result[7];

    cairo_t *cr = gdk_cairo_create( gtk_widget_get_window(widget) );

#ifdef DEBUG_XSENSORS
    printf( "area.width = %d, area.height = %d\n", event->area.width,
            event->area.height );
#endif

#if GTK_MAJOR_VERSION == 2
    gdk_window_clear_area( widget->window, event->area.x, event->area.y,
                            event->area.width, event->area.height );
#elif GTK_MAJOR_VERSION == 3
/*FIXME*/
#endif

    switch ( current->feattype ) {
        case FAN:
            if ( current->curvalue < current->curmin )
                highLow = 30;

            /* Display the digits */
            if ( g_snprintf( result, 6, "%5.0f", current->curvalue ) >= 0 )
               draw_digits( widget, cr, result, highLow );

            /* Display RPM */
            cairo_set_source_surface (cr, surface, 90-0, 0-(120 + highLow));
     	    cairo_rectangle(cr, 90, 0, 57, 30);
            break;
        case TEMP:
            if ( current->curvalue > current->curmax )
                highLow = 30;

            if ( tf == TRUE )
                current->curvalue = ( 1.8 * current->curvalue ) + 32;

            /* Display the digits */
            if ( g_snprintf( result, 7, "%6.1f", current->curvalue ) >= 0 )
               draw_digits( widget, cr, result, highLow );

            /* Display degree symbol */
            if ( tf == FALSE )
                x = 0;
            else
                x = 57;
            cairo_set_source_surface (cr, surface, 96-x, 0-(60 + highLow));
     	    cairo_rectangle(cr, 96, 0, 57, 30);

            break;
        case VOLT:
            if ( current->curvalue > current->curmax ||
                 current->curvalue < current->curmin )
                highLow = 30;

            /* Display the digits */
            if ( g_snprintf( result, 7, "%6.2f", current->curvalue ) >= 0 )
               draw_digits( widget, cr, result, highLow );

            /* Display V */
            cairo_set_source_surface (cr, surface, 96-114, 0-(60 + highLow));
     	    cairo_rectangle(cr, 96, 0, 57, 30);
            break;
        default:
            break;
    }
    cairo_fill(cr);
    cairo_destroy(cr);
    return TRUE;
}

/* Free the link list. */
gint free_llist( updates *node ) {

    if ( node != NULL ) {
        free_llist( node->next );
        g_free( node );
    }

    return SUCCESS;
}

/* Find the tail of a non-NULL linked list. */
static updates *llist_tail( updates *node ) {

    if ( node->next == NULL )
        return node;
    else
        return llist_tail( node->next );
}

/* Update the sensor information. */
gint update_sensor_data( gpointer data ) {
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
gint start_timer( GtkWidget *widget, gpointer data ) {

    /* Setup timer for updates. */
    g_timeout_add( update_time * 1000,
                             (GSourceFunc) update_sensor_data,
			     (gpointer) data );

    return SUCCESS;
}

updates *add_sensor_tab( GtkWidget *container, const sensors_chip_name *name ) {
    /* packing boxes */
    GtkWidget *mainbox = NULL;
    GtkWidget *voltbox = NULL;
    GtkWidget *tempbox = NULL;
    GtkWidget *fanbox = NULL;
    GtkWidget *currbox = NULL;
    GtkWidget *innerbox = NULL;

    /* notebook stuff */
    GtkWidget *notelabel = NULL;
    GtkWidget *noteframe = NULL;

    /* main feature labels */
    GtkWidget *voltlabel = NULL;
    GtkWidget *templabel = NULL;
    GtkWidget *fanlabel = NULL;

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
#elif GTK_MAJOR_VERSION == 3
    mainbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 10 );
#endif
    gtk_container_set_border_width( GTK_CONTAINER (mainbox), 10 );
    gtk_widget_show( mainbox );

#if GTK_MAJOR_VERSION == 2
    voltbox = gtk_vbox_new( FALSE, 0 );
    tempbox = gtk_vbox_new( FALSE, 0 );
    fanbox = gtk_vbox_new( FALSE, 0 );
#elif GTK_MAJOR_VERSION == 3
    voltbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    tempbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    fanbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#endif

    /* Create notebook for sensors. */
    noteframe = gtk_frame_new( NULL );
    gtk_container_set_border_width( GTK_CONTAINER (noteframe), 10 );
    gtk_widget_show( noteframe );

    notelabel = gtk_label_new( name->prefix );
    gtk_widget_show( notelabel );

    gtk_container_add( GTK_CONTAINER (noteframe), mainbox );
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
#if GTK_MAJOR_VERSION == 2
            innerbox = gtk_vbox_new( FALSE, 0 );
#elif GTK_MAJOR_VERSION == 3
            innerbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#endif
            featpbar = gtk_progress_bar_new();
            darea = gtk_drawing_area_new();
#if GTK_MAJOR_VERSION == 2
            gtk_widget_modify_bg( darea, GTK_STATE_NORMAL, &colorWhite );
#elif GTK_MAJOR_VERSION == 3
/*FIXME*/
#endif

            gtk_widget_set_size_request( darea, 36, 30 );

            new_node->darea = darea;
            new_node->pbar = featpbar;

            if ( head == NULL ) {
                head = current = new_node;
            } else {
                current = current->next = new_node;
            }

            /* Connect the expose event sinal handler to redraw the numbers. */
            g_signal_connect( G_OBJECT(darea), "expose_event",
                              G_CALLBACK(expose_event_callback), current );

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

static updates *add_sensor_chips( GtkWidget *notebook, const char *pattern ) {
    const sensors_chip_name *name = NULL, *pquery = NULL;
    sensors_chip_name query;

    updates *head = NULL, *tail = NULL, *new_nodes;

    int chipnum = 0;

    if ( pattern ) {
        if ( sensors_parse_chip_name( pattern, &query ) ) {
            fprintf( stderr,
                    "Couldn't parse chip name %s!  Exiting!\n",
                    pattern );
            return NULL;
        }
        pquery = &query;
    }

    while ( ( name = sensors_get_detected_chips( pquery, &chipnum ) ) != NULL ) {
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

int start_gui( int argc, char **argv ) {
    struct stat sbuf;
    char *title = NULL;
    int i, errone;

    GtkWidget *notebook = NULL;

    updates *head = NULL;

    gtk_init( &argc, &argv );

    if ( ( title = g_malloc( 15 * sizeof( char ) ) ) == NULL ) {
            fprintf( stderr, "malloc failed!\n" );
            exit( 1 );
    }

    /* Setup main window. */
    mainwindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    title = strcpy( title, "xsensors " );
    title = strcat( title, VERSION );
    gtk_window_set_title( GTK_WINDOW (mainwindow), title );
    g_signal_connect( G_OBJECT (mainwindow), "delete_event",
                      G_CALLBACK (destroy_gui), NULL );


    /* Set up the image file used for displaying characters. */
    if ( imagefile == NULL ) {
        if( ( imagefile = g_malloc( sizeof( char ) *
                          ( sizeof( DATADIR ) + 20 ) ) ) == NULL ) {
            fprintf( stderr, "malloc failed!\n" );
            exit( 1 );
        }
        sprintf( imagefile, "%s/xsensors.xpm", DATADIR );
        if ( ( errone = stat( imagefile, &sbuf ) ) != 0 ) {
            if ( stat( "./xsensors.xpm", &sbuf ) != 0 ) {
                fprintf( stderr, "%s: %s\n",
                         strerror( errno ), imagefile );
                fprintf( stderr, "%s: ./xsensors.xpm\n",
                         strerror( errno ) );
                fprintf( stderr,
                       "Image file not found in either location!  Exiting!\n" );
                exit( 1 );
            } else {
                theme = gdk_pixbuf_new_from_file("./images/xsensors.xpm", NULL );
            }
        } else {
            theme = gdk_pixbuf_new_from_file(imagefile, NULL );
        }
    } else {
        if ( stat( imagefile, &sbuf ) != 0 ) {
            fprintf( stderr, "%s: %s\n", strerror( errno ), imagefile );
            fprintf( stderr,
                    "Image file not found in specified location!  Exiting!\n" );
            exit( 1 );
        } else {
            theme = gdk_pixbuf_new_from_file(imagefile, NULL );
        }
    }
    surface = cairo_image_surface_create_for_data(gdk_pixbuf_get_pixels(theme),
                                        CAIRO_FORMAT_RGB24,
										gdk_pixbuf_get_width(theme),
										gdk_pixbuf_get_height(theme),
										gdk_pixbuf_get_rowstride(theme));

    /* Create notebook for sensors. */
    notebook = gtk_notebook_new( );
#if GTK_MAJOR_VERSION == 2
    gtk_widget_modify_bg( notebook, GTK_STATE_NORMAL, &colorWhite );
#elif GTK_MAJOR_VERSION == 3
/*FIXME*/
#endif
    gtk_notebook_set_tab_pos( GTK_NOTEBOOK (notebook), GTK_POS_LEFT );
    gtk_widget_show( notebook );

    gtk_container_add( GTK_CONTAINER (mainwindow), notebook );

    if ( argc < 2 || (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'f')) {
        head = add_sensor_chips( notebook, NULL );
        if ( head == NULL )
            return FAILURE;
    } else {
        for ( i = 1; i < argc; i++ ) {
            if ( argv[i][0] != '-' || argv[i][1] != 'f') {
                head = add_sensor_chips( notebook, argv[i] );
                if ( head == NULL )
                    return FAILURE;
            }
        }
    }

    /* Setup the main components. */
    gtk_widget_show( mainwindow );

    gtk_main();

    free_llist( head );
    free( title );

    return SUCCESS;
}
