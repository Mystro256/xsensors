/*
    main.h - Part of xsensors

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sensors/sensors.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "chips.h"
#include "gui.h"
#include "../config.h"

/* Pretty identifiers. */
enum { SUCCESS, FAILURE };

/* Prototypes */
int help_msg( void );
