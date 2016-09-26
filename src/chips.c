/*
    chips.c - Part of xsensors

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

/* Add a node to the feature linked list at the passed node pointer. */
updates *add_node( const sensors_chip_name *name,
                   const sensors_feature *feature ) {
    const sensors_subfeature *subfeature;
    updates *node;

    if( ( node = g_malloc( sizeof( updates ) ) ) == NULL ) {
        fprintf( stderr, "malloc failed!\n" );
        return NULL;
    }

    switch ( feature->type ) {
	case SENSORS_FEATURE_IN:
	    node->feattype = VOLT;
	    break;
	case SENSORS_FEATURE_TEMP:
	    node->feattype = TEMP;
	    break;
	case SENSORS_FEATURE_FAN:
	    node->feattype = FAN;
	    break;
	default:
	    g_free( node );
	    return NULL;
    }

    node->featnum = UNDEFMAXMIN;
    node->featminnum = UNDEFMAXMIN;
    node->featmaxnum = UNDEFMAXMIN;
    switch ( node->feattype ) {
	case VOLT:
	    if ( (subfeature = sensors_get_subfeature (name, feature, SENSORS_SUBFEATURE_IN_INPUT)) )
		node->featnum = subfeature->number;
	    if ( (subfeature = sensors_get_subfeature (name, feature, SENSORS_SUBFEATURE_IN_MIN)) )
		node->featminnum = subfeature->number;
	    if ( (subfeature = sensors_get_subfeature (name, feature, SENSORS_SUBFEATURE_IN_MAX)) )
		node->featmaxnum = subfeature->number;
	    break;
	case TEMP:
	    if ( (subfeature = sensors_get_subfeature (name, feature, SENSORS_SUBFEATURE_TEMP_INPUT)) )
		node->featnum = subfeature->number;
	    if ( (subfeature = sensors_get_subfeature (name, feature, SENSORS_SUBFEATURE_TEMP_MIN)) )
		node->featminnum = subfeature->number;
	    if ( (subfeature = sensors_get_subfeature (name, feature, SENSORS_SUBFEATURE_TEMP_MAX)) )
		node->featmaxnum = subfeature->number;
	    break;
	case FAN:
	    if ( (subfeature = sensors_get_subfeature (name, feature, SENSORS_SUBFEATURE_FAN_INPUT)) )
		node->featnum = subfeature->number;
	    if ( (subfeature = sensors_get_subfeature (name, feature, SENSORS_SUBFEATURE_FAN_MIN)) )
		node->featminnum = subfeature->number;
	    break;
    }

    if ( node->featnum == UNDEFMAXMIN ) {
	g_free( node );
	return NULL;
    }

    node->name = name;
    node->next = NULL;

    return node;
}
