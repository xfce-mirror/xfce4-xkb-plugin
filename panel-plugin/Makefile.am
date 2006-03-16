plugindir = $(libexecdir)/xfce4/panel-plugins

plugin_PROGRAMS = xfce4-xkb-plugin

xfce4_xkb_plugin_SOURCES =								\
	xfce4-xkb-plugin.c											\
	xkb.c																		\
	xkb.h

xfce4_xkb_plugin_CFLAGS =									\
	@LIBXFCE4PANEL_CFLAGS@          				\
	-DFLAGSDIR=\"$(datadir)/xfce4/xkb/flags\"

xfce4_xkb_plugin_LDADD = 									\
	@LIBXFCE4PANEL_LIBS@										\
	@LIBS@

desktop_in_files = xkb-plugin.desktop.in
desktop_files = $(desktop_in_files:.desktop.in=.desktop)

desktopdir = $(datadir)/xfce4/panel-plugins
desktop_DATA = $(desktop_files)

EXTRA_DIST =															\
	$(desktop_in_files)

DISTCLEANFILES = 													\
	$(desktop_DATA) $(desktop_in_files)

%.desktop: %.desktop.in
	sed -e "s^@PLUGIN_PATH@^$(libexecdir)/xfce4/panel-plugins^" \
		$< > $@

