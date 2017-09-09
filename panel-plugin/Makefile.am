# $Id$

plugindir = \
	$(libdir)/xfce4/panel/plugins

plugin_LTLIBRARIES = \
	libxkb.la

libxkb_la_SOURCES = \
	xkb-plugin.h \
	xkb-plugin.c \
	xkb-properties.h \
	xkb-keyboard.h \
	xkb-keyboard.c \
	xkb-modifier.h \
	xkb-modifier.c \
	xkb-dialog.h \
	xkb-dialog.c \
	xkb-xfconf.h \
	xkb-xfconf.c \
	xkb-cairo.h \
	xkb-cairo.c \
	xkb-util.h \
	xkb-util.c

libxkb_la_CPPFLAGS = \
	-I$(top_srcdir) \
	$(PLATFORM_CPPFLAGS)

libxkb_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(XFCONF_CFLAGS) \
	$(LIBXKLAVIER_CFLAGS) \
	$(LIBRSVG_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(GARCON_CFLAGS) \
	$(PLATFORM_CFLAGS) \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DFLAGSRELDIR=\"xfce4/xkb/flags\" \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE

libxkb_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

libxkb_la_LIBADD = \
	$(GTK_LIBS) \
	$(LIBXFCE4PANEL_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(XFCONF_LIBS) \
	$(LIBXKLAVIER_LIBS) \
	$(LIBWNCK_LIBS) \
	$(GARCON_LIBS) \
	$(LIBRSVG_LIBS) \
	-lX11

#
# Desktop file
#
desktopdir = $(datadir)/xfce4/panel/plugins
desktop_in_files = xkb.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(desktop_in_files)

CLEANFILES = \
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

