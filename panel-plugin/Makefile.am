plugindir = @XFCE4_PANEL_PLUGINSDIR@

plugin_LTLIBRARIES =							\
	libxkb.la

libxkb_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-module								\
	-L/usr/X11R6/lib

libxkb_la_SOURCES =							\
	xfce4-xkb-plugin.c xkb.c xkb.h

libxkb_la_CFLAGS =							\
	@XFCE4_PANEL_CFLAGS@                                            \
        -I/usr/include/gtk-2.0                                          \
        -I/usr/include/glib-2.0                                         \
        -I/usr/lib/gtk-2.0/include                                      \
        -I/usr/lib/glib-2.0/include                                     \
        -I/usr/include/pango-1.0                                        \
        -I/usr/include/atk-1.0                                          \
	-DFLAGSDIR=\"$(datadir)/xfce4/xkb/flags\"

libxkb_la_LIBADD = 							\
	@XFCE4_PANEL_LIBS@ -lX11 -lXext -lXpm
