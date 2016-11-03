AM_CPPFLAGS = 								\
	-I$(top_srcdir)							\
	$(XSENSORS_CFLAGS)						\
	-DG_LOG_DOMAIN=\"GnomeCPUInfoApplet\"				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"		\
	-DDATADIR=\""$(datadir)/pixmaps"\"					\
	-DG_DISABLE_DEPRECATED		-DGTK_DISABLE_DEPRECATED	\
	-DGDK_DISABLE_DEPRECATED	-DGNOME_DISABLE_DEPRECATED	\
	-ansi	-Wall	-Wmissing-prototypes	-Wmissing-declarations \
    -Werror -Wall

bin_PROGRAMS = xsensors

xsensors_SOURCES =	\
	main.c  \
    gui.c   \
    prefs.c \
    chips.c

xsensors_LDADD = $(XSENSORS_LIBS)
