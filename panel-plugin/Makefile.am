
INCLUDES =										\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"						\
	-DFLAGSDIR=\"$(datadir)/xfce4/xkb/flags\"

plugindir = $(libexecdir)/xfce4/panel-plugins

plugin_PROGRAMS = xfce4-xkb-plugin

xfce4_xkb_plugin_SOURCES =								\
	xfce4-xkb-plugin.c								\
	xkb.c																		\
	xkb.h

xfce4_xkb_plugin_CFLAGS =								\
	@LIBXFCE4PANEL_CFLAGS@								\
	@GTK_CFLAGS@									\
	@LIBXFCEGUI4_CFLAGS@								\
	@LIBXFCE4UTIL_CFLAGS@
	

xfce4_xkb_plugin_LDADD = 								\
	@LIBXFCE4PANEL_LIBS@								\
	@GTK_LIBS@									\
	@LIBXFCEGUI4_LIBS@								\
	@LIBXFCE4UTIL_LIBS@

desktop_in_in_files = xkb-plugin.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)

desktopdir = $(datadir)/xfce4/panel-plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =															\
	$(desktop_in_in_files)

DISTCLEANFILES = 									\
	$(desktop_DATA) $(desktop_in_files)

%.desktop.in: %.desktop.in.in
	sed -e "s^@PLUGIN_PATH@^$(libexecdir)/xfce4/panel-plugins^" \
		$< > $@

