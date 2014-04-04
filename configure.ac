dnl Process this file with autoconf to produce a configure script.

AM_CONFIG_HEADER(config.h)

AC_INIT(xsensors, 0.73)
AC_CONFIG_SRCDIR(src/main.c)
AM_INIT_AUTOMAKE


AC_PREREQ([2.50])

AM_MAINTAINER_MODE

AC_PROG_CC
AC_ISC_POSIX
AC_HEADER_STDC

dnl Checks for the sensors library.
AC_CHECK_LIB(sensors, sensors_init,[],
 	AC_MSG_ERROR(You need the sensors library))

AC_CHECK_HEADERS([sensors/sensors.h sensors/error.h])

AM_PATH_GLIB_2_0(2.0.0,,AC_MSG_ERROR([
*** GLib 2.0 is required to build xsensors; please make sure you have the GLib
*** development headers installed. The latest version of GLib is
*** always available at http://www.gtk.org/.]))
AM_PATH_GTK_2_0(2.0.0,,AC_MSG_ERROR([
*** GTK+ 2.0 is required to build xsensors; please make sure you have the GTK+
*** development headers installed. The latest version of GTK+ is
*** always available at http://www.gtk.org/.]))

XSENSORS_CFLAGS="`pkg-config --cflags gtk+-2.0` -Werror"
XSENSORS_LIBS="`pkg-config --libs gtk+-2.0`"
AC_SUBST(XSENSORS_CFLAGS)
AC_SUBST(XSENSORS_LIBS)

AC_DEFINE_UNQUOTED(XSENSORS_ICONDIR, "${prefix}/share/pixmaps")

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

AC_OUTPUT([
	Makefile
	xdg/Makefile
	src/Makefile
])
