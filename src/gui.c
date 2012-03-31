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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "main.h"

extern int tf;
extern int update_time;
extern char *imagefile;

GtkWidget *mainwindow = NULL;

GdkColor colorWhite = { 0, 0xFFFF, 0xFFFF, 0xFFFF };
    
GdkColormap *cmap = NULL;

GdkPixmap *theme = NULL;

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

static void draw_digits( GtkWidget *widget, const gchar *digits, int highLow )
{
    const gchar *digit = digits;
    int pos = 0, x = 0, y = 0, w = 0;

    while ( *digit ) {
        get_pm_location( *digit, &x, &y, &w );
        gdk_draw_drawable( widget->window,
                           widget->style->fg_gc[ GTK_WIDGET_STATE
                           (widget) ], theme, x, y + highLow,
                           pos, 0, w, 30 );
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

#ifdef DEBUG_XSENSORS
    printf( "area.width = %d, area.height = %d\n", event->area.width,
            event->area.height );
#endif

    gdk_window_clear_area( widget->window, event->area.x, event->area.y, 
                            event->area.width, event->area.height );

    switch ( current->feattype ) {
        case FAN:
            if ( current->curvalue < current->curmin )
                highLow = 30;

            /* Display the digits */
            if ( g_snprintf( result, 6, "%5.0f", current->curvalue ) >= 0 )
               draw_digits( widget, result, highLow );

            /* Display RPM */
            gdk_draw_drawable( widget->window, 
                               widget->style->fg_gc[ GTK_WIDGET_STATE 
                               (widget) ], theme, 0, 120 + highLow, 
                               90, 0, 57, 30 );
            break;
        case TEMP:
            if ( current->curvalue > current->curmax )
                highLow = 30;

            if ( tf == TRUE )
                current->curvalue = ( 1.8 * current->curvalue ) + 32;

            /* Display the digits */
            if ( g_snprintf( result, 7, "%6.1f", current->curvalue ) >= 0 )
               draw_digits( widget, result, highLow );

            /* Display degree symbol */
            if ( tf == FALSE )
                x = 0;
            else
                x = 57;
            gdk_draw_drawable( widget->window, 
                             widget->style->fg_gc[ GTK_WIDGET_STATE 
                             (widget) ], theme, x, 60 + highLow, 
                             96, 0, 57, 30 );
            
            break;
        case VOLT:
            if ( current->curvalue > current->curmax ||
                 current->curvalue < current->curmin )
                highLow = 30;
            
            /* Display the digits */
            if ( g_snprintf( result, 7, "%6.2f", current->curvalue ) >= 0 )
               draw_digits( widget, result, highLow );

            /* Display V */
            gdk_draw_drawable( widget->window, 
                             widget->style->fg_gc[ GTK_WIDGET_STATE 
                             (widget) ], theme, 114, 60 + highLow, 
                             96, 0, 57, 30 );


            break;
        default:
            break;
    }
            
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
    gint timer;

    /* Setup timer for updates. */
    timer = g_timeout_add( update_time * 1000, 
                             (GtkFunction) update_sensor_data, 
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
    updates *current = NULL, *prev = NULL;

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
    mainbox = gtk_hbox_new( TRUE, 10 );
    gtk_container_set_border_width( GTK_CONTAINER (mainbox), 10 );
    gtk_widget_show( mainbox );
    voltbox = gtk_vbox_new( FALSE, 0 );
    tempbox = gtk_vbox_new( FALSE, 0 );
    fanbox = gtk_vbox_new( FALSE, 0 );

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
            innerbox = gtk_vbox_new( FALSE, 0 );
            featpbar = gtk_progress_bar_new();
            darea = gtk_drawing_area_new();
            gtk_widget_modify_bg( darea, GTK_STATE_NORMAL, &colorWhite );
            gtk_widget_set_size_request( darea, 36, 30 );
            
            new_node->darea = darea;
            new_node->pbar = featpbar;

            if ( head == NULL ) {
                prev = head;
                head = current = new_node;
            } else {
                prev = current;
                current = current->next = new_node;
            }

            /* Connect the expose event sinal handler to redraw the numbers. */
            g_signal_connect( G_OBJECT(darea), "expose_event", 
                              G_CALLBACK(expose_event_callback), current );
            
            feattext = sensors_get_label( name, feature );
	    
            if ( feattext != NULL ) {
#ifdef DEBUG_XSENSORS
                printf( "Adding feature %d, %s.\n", i, feattext );
#endif
                if ( ( feattext = realloc( feattext, 
                                ( strlen( feattext ) + 2 ) * 
                                sizeof( char ) ) ) == NULL ) {
                    fprintf( stderr, "realloc failed in add_sensor_tab()!\n" );
                    return NULL;
                }
                if ( strcat( feattext, ":" ) == NULL ) {
                    fprintf( stderr, "strcat failed in add_sensor_tab()!\n" );
                    return NULL;
                }
                
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
            g_free( feattext );
            feattext = NULL;
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

    g_free( feattext );

    return head;
}

int start_gui( int argc, char **argv ) {
    struct stat sbuf;
    char *title = NULL;
    int errone;

    GtkWidget *notebook = NULL;
    
    updates *head = NULL;

    int chipnum = 0;
    const sensors_chip_name *name = NULL;

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

    /* Graphics needed for drawing info. */
    cmap = gtk_widget_get_colormap( mainwindow );

    /* Set up the image file used for displaying characters. */
    if ( imagefile == NULL ) {
        if( ( imagefile = g_malloc( sizeof( char ) * 
                          ( sizeof( DATADIR ) + 20 ) ) ) == NULL ) {
            fprintf( stderr, "malloc failed!\n" );
            exit( 1 );
        }
        sprintf( imagefile, "%s/default.xpm", DATADIR );
        if ( ( errone = stat( imagefile, &sbuf ) ) != 0 ) {
            if ( stat( "./default.xpm", &sbuf ) != 0 ) {
                fprintf( stderr, "%s: %s\n", 
                         strerror( errno ), imagefile );
                fprintf( stderr, "%s: ./default.xpm\n", 
                         strerror( errno ) );
                fprintf( stderr, 
                       "Image file not found in either location!  Exiting!\n" );
                exit( 1 );
            } else {
                theme = gdk_pixmap_colormap_create_from_xpm( NULL, cmap,
                        NULL, NULL, "./images/default.xpm" );
            }
        } else {
            theme = gdk_pixmap_colormap_create_from_xpm( NULL, cmap,
                    NULL, NULL, imagefile );
        }
    } else {
        if ( stat( imagefile, &sbuf ) != 0 ) {
            fprintf( stderr, "%s: %s\n", strerror( errno ), imagefile );
            fprintf( stderr, 
                    "Image file not found in specified location!  Exiting!\n" );
            exit( 1 );
        } else {
            theme = gdk_pixmap_colormap_create_from_xpm( NULL, cmap,
                    NULL, NULL, imagefile );
        }
    }
    
    /* Create notebook for sensors. */
    notebook = gtk_notebook_new( );
    gtk_widget_modify_bg( notebook, GTK_STATE_NORMAL, &colorWhite );
    gtk_notebook_set_tab_pos( GTK_NOTEBOOK (notebook), GTK_POS_LEFT );
    gtk_widget_show( notebook );

    gtk_container_add( GTK_CONTAINER (mainwindow), notebook );

    while ( ( name = sensors_get_detected_chips( NULL, &chipnum ) ) != NULL ) {
        if ( 1 ) {
#ifdef DEBUG_XSENSORS
            printf( "Adding tab for %s\n", name->prefix );
#endif
            if ( ( head = add_sensor_tab( notebook, name ) ) == NULL )
                return FAILURE;
            
            update_sensor_data( head );
            g_signal_connect( G_OBJECT (mainwindow), "realize",
                              G_CALLBACK (start_timer), head );
        }
    }
    
    /* Setup the main components. */
    gtk_widget_show( mainwindow );

    gtk_main();

    free_llist( head );
    free( title );

    return SUCCESS;
}
