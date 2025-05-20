dnl configure.ac
dnl
dnl xfce4-xkb-plugin - XKB plugin for xfce4-panel
dnl
dnl 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
dnl

AC_CONFIG_MACRO_DIRS([m4])
m4_define([copyright_year], [2025])

dnl ***************************
dnl *** Version information ***
dnl ***************************
XDT_VERSION_INIT([0.9.0], [git])


dnl ***************************
dnl *** Initialize autoconf ***
dnl ***************************
AC_COPYRIGHT([Copyright (c) 2004-copyright_year()
        The Xfce development team. All rights reserved.])
AC_INIT([xfce4-xkb-plugin], [xdt_version], [https://gitlab.xfce.org/panel-plugins/xfce4-xkb-plugin/], [xfce4-xkb-plugin])
AC_PREREQ([2.69])
AC_CANONICAL_TARGET()
AC_REVISION([xdt_version_build])
AC_DEFINE([VERSION_FULL], [PACKAGE_VERSION], [Alias for VERSION and PACKAGE_VERSION for meson compatibility])
COPYRIGHT_YEAR=copyright_year()
AC_DEFINE_UNQUOTED([COPYRIGHT_YEAR], ["$COPYRIGHT_YEAR"], [Copyright year])
AC_SUBST([COPYRIGHT_YEAR])

dnl ***************************
dnl *** Initialize automake ***
dnl ***************************
AM_INIT_AUTOMAKE([1.8 dist-bzip2 no-dist-gzip tar-ustar foreign])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl *******************************
dnl *** Check for UNIX variants ***
dnl *******************************
AC_USE_SYSTEM_EXTENSIONS

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************
AC_PROG_CC()
LT_PATH_LD([])
AC_PROG_INSTALL()
LT_PREREQ([2.2.6])
LT_INIT([disable-static])
AM_PROG_CC_C_O()

dnl *********************
dnl *** Check for X11 ***
dnl *********************
XDT_CHECK_LIBX11_REQUIRE()

dnl ******************************
dnl *** Check for i18n support ***
dnl ******************************
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

dnl ***********************************
dnl *** Check for required packages ***
dnl ***********************************
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.50.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.22.0])
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-2.0], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UTIL], [libxfce4util-1.0], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.16.0])
XDT_CHECK_PACKAGE([XFCONF], [libxfconf-0], [4.16.0])
XDT_CHECK_PACKAGE([LIBXKLAVIER], [libxklavier], [5.3])
XDT_CHECK_PACKAGE([LIBRSVG], [librsvg-2.0], [2.40])
XDT_CHECK_PACKAGE([LIBWNCK], [libwnck-3.0], [3.14])
XDT_CHECK_PACKAGE([GARCON], [garcon-1], [4.16.0])

dnl **********************************
dnl *** Optional libnotify Support ***
dnl **********************************
XDT_CHECK_OPTIONAL_PACKAGE([LIBNOTIFY], [libnotify], [0.7.0], [libnotify],
                           [libnotify library])

dnl ***********************************
dnl *** Check for debugging support ***
dnl ***********************************
XDT_FEATURE_DEBUG([xdt_debug_default])

AC_CONFIG_FILES([
panel-plugin/Makefile
flags/Makefile
Makefile
po/Makefile.in
icons/Makefile
icons/16x16/Makefile
icons/32x32/Makefile
icons/128x128/Makefile
icons/scalable/Makefile
])
AC_OUTPUT

dnl ***************************
dnl *** Print configuration ***
dnl ***************************
echo
echo "Build Configuration:"
echo
echo "* Debug Support:      $enable_debug"
echo "* libnotify Support:  ${LIBNOTIFY_FOUND:-no}"
echo
