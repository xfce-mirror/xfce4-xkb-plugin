@SET_MAKE@
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS = \
	panel-plugin \
	flags \
	po \
	icons

distuninstallcheck_listfiles = 						\
	find . -type f -print | grep -v ./share/icons/hicolor/icon-theme.cache

distclean-local:
	rm -rf *.cache *~

.PHONY: ChangeLog

ChangeLog: 
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp \
	&& mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) \
	|| (touch ChangeLog; echo 'Git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

EXTRA_DIST =								\
	README.md							\
	meson.build							\
	meson_options.txt						\
	po/meson.build							\
	xfce-revision.h.in 						\
	$(NULL)
