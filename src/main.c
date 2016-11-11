/*
    main.c - Part of xsensors

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
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

/* getopt stuff */
extern int      optopt;
extern int      optind;
extern char    *optarg;
extern int      opterr;

int             getopt (int, char *const *, const char *);
/* end getopt stuff */

char *strdup(const char *s);

/* Our public vars */
int tf = FALSE;
int update_time = 1;
char *imagefile = NULL;
const char *home_dir = NULL;

enum { UPDATE_TIME_SUCCESS, UPDATE_TIME_NAN, UPDATE_TIME_NEG };

static int get_updatetime( const char * temp_str ) {
    if ( temp_str[0] == '0' && temp_str[1] < '0' ) {
        update_time = 0;
    } else {
        int temp = atoi( temp_str );
        if ( temp == 0 )
            return UPDATE_TIME_NAN;
        else if ( temp < 0 )
            return UPDATE_TIME_NEG;
        update_time = temp;
    }

    return UPDATE_TIME_SUCCESS;
}

/* Print the help message. */
int help_msg( void )
{
    printf( "\nUsage: xsensors [options]\n\n"
            "Options:\n"
            "--------\n\n"
            "-f\t\tDisplay all temperatures in Fahrenheit.\n"
            "-h\t\tDisplay this help text and exit.\n"
            "-c filename\tSpecify the libsensors configuration file.\n"
            "-i filename\tSpecify the image file to use as a theme.\n"
            "-t time\t\tSpecify the update time in number of seconds.\n"
            "\t\tSet this to a negative number for default time.\n"
            "\t\tSet this to zero for no update.\n"
            "-v\t\tDisplay version number.\n"
            "\n" );

    return SUCCESS;
}

void read_config( void )
{
    FILE * fileconf;
    char temp_str[ 100 ];
    sprintf( temp_str, "%s/.local/share/%s/custom.ini",
             home_dir, PACKAGE );

    fileconf = fopen( temp_str, "r" );
    if ( fileconf == NULL )
        return;

    /*
     Possible config values:
        use_fahrenheit=0 or 1
        update_time=(uint)
    */

    while ( fgets( temp_str, 100, fileconf ) != NULL ) {
        if ( temp_str[0] != '[' && temp_str[0] != ';' ) {
            if ( strlen( temp_str ) > 15 &&
                    strncmp( temp_str, "use_fahrenheit=", 15 ) == 0 ) {
                if ( temp_str[15] == '1' )
                    tf = TRUE;
                else if ( temp_str[15] != '0' )
                    fputs( "Warning: invalid custom.ini entry!\n"
                           "use_fahrenheit can only "
                           "have a value of 0 or 1.\n", stderr );
            } else if ( strlen( temp_str ) > 12 &&
                    strncmp( temp_str, "update_time=", 12 ) == 0 ) {
                switch ( get_updatetime( &temp_str[12] ) ) {
                    case UPDATE_TIME_NAN :
                        fputs( "Warning: invalid custom.ini entry!\n"
                               "update_time does not appear to be "
                               "a valid number.\n", stderr ); break;
                    case UPDATE_TIME_NEG :
                        fputs("Warning: invalid custom.ini entry!\n"
                              "update_time should be a positive "
                              "number.\n", stderr ); break;
                }
            }
        }
    }
    fclose( fileconf );
}

/* Main. */
int main( int argc, char **argv )
{
    int c = 0;
    char *sens_config = NULL;
    FILE *sens_conf_file = NULL;
    char *temp_str = NULL;

    /* Get homedir */
    if ( ( home_dir = getenv( "HOME" ) ) == NULL )
        home_dir = getpwuid(getuid())->pw_dir;

    read_config();

    /* Process arguements. */
    while ( ( c = getopt( argc, argv, "fhc:i:t:v" ) ) != EOF ) {
        switch (c) {
            case 'f':
                tf = TRUE;
                break;
            case 'h':
                if ( help_msg() == SUCCESS )
                    return EXIT_SUCCESS;
                else
                    return EXIT_FAILURE;
            case 'c':
                if ( ( sens_config = strdup( optarg ) ) == NULL )
                    fputs( "strdup failed! Something is very wrong!\n",
                           stderr );
                break;
            case 'i':
                if ( ( imagefile = strdup( optarg ) ) == NULL )
                    fputs( "strdup failed! Something is very wrong!\n",
                           stderr );
                break;
            case 't':
                if ( ( temp_str =  strdup( optarg ) ) == NULL )
                    fputs( "strdup failed! Something is very wrong!\n",
                           stderr );
                if ( get_updatetime( temp_str ) == UPDATE_TIME_NAN )
                    fputs( "Warning!!\nSpecified update time does "
                           "not appear to be a valid number\n", stderr );
                break;
            case 'v':
                printf( "\nXsensors version %s\n\n", VERSION );
                return EXIT_SUCCESS;
            case '?':
                return EXIT_FAILURE;
            default:
                fputs( "Something has gone wrong!\n", stderr );
                return EXIT_FAILURE;
        }
    }

    /* Open the config file if specified. */
    if ( sens_config &&
            ( sens_conf_file = fopen( sens_config, "r" ) ) == NULL ) {
        fprintf( stderr, "Error opening config file: %s\n"
                 , sens_config );
        return EXIT_FAILURE;
    }


    /* Initialize the sensors library. */
    int errorno = sensors_init( sens_conf_file );
    if ( errorno != SUCCESS ) {
        fprintf( stderr, "Could not initialize sensors!\n"
                 "Is everything installed properly?\n"
                 "Error Number: %d", errorno );
        if ( !sens_config ) {
            gtk_init( &argc, &argv );
            GtkWidget *dialog = gtk_message_dialog_new( NULL,
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                          "Could not initialize sensors!\n\n"
                                          "Is everything installed properly?\n"
                                          "Error Number: %d", errorno );
            gtk_dialog_run( GTK_DIALOG (dialog) );
            gtk_widget_destroy( dialog );
        }
        return EXIT_FAILURE;
    }

    /* This will start the GUI. */
    if ( start_gui( argc, argv ) != SUCCESS )
        fputs( "GUI failed!\n", stderr );

    /* Clean up the sensors library. */
    sensors_cleanup();

    if ( sens_config != NULL )
        free( sens_config );
    if ( imagefile != NULL )
        free( imagefile );

    /* Close the config file. */
    if ( sens_conf_file && fclose( sens_conf_file ) != SUCCESS ) {
        fputs( "Something has gone wrong closing the config file!\n", stderr );
        return EXIT_FAILURE;
    }

    free( temp_str );

    return EXIT_SUCCESS;
}
