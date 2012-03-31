/*
    chips.h - Part of xsensors

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

#define UNDEFMAXMIN -1

/* Link list to hold sensor feature data.  This is used since the number of 
 * features vary.
 */
typedef struct _updates {
    const sensors_chip_name *name;
    GtkWidget *pbar;
    GtkWidget *darea;
    int featnum;
    int featminnum;
    int featmaxnum;
    int feattype;
    double curvalue;
    double curmin;
    double curmax;
    struct _updates *next;
} updates;

/* Add a node to the sensor's features linked list. */
updates *add_node( const sensors_chip_name *, const sensors_feature * );
