@SET_MAKE@

SUBDIRS = panel-plugin flags po

EXTRA_DIST =								\
	README								\
	intltool-extract.in						\
	intltool-merge.in						\
	intltool-update.in

distclean-local:
	rm -rf *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz
