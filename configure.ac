dnl configure.ac
dnl
dnl xfce4-xkb-plugin - XKB plugin for xfce4-panel
dnl
dnl 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
dnl

m4_define([xfce4_xkb_plugin_version],[0.4.1])

dnl init autoconf
AC_INIT([xfce4-xkb-plugin], [xfce4_xkb_plugin_version()], [sasoiliev@mamul.org])
AC_CONFIG_SRCDIR([configure])

dnl init automake
XKB_VERSION=xfce4_xkb_plugin_version()
AM_INIT_AUTOMAKE([xfce4-xkb-plugin], [$XKB_VERSION])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE

dnl Check for UNIX variants
AC_AIX
AC_ISC_POSIX
AC_MINIX
AM_CONDITIONAL([HAVE_CYGWIN], [test "`uname | grep \"CYGWIN\"`" != ""])

dnl disable static lib
AC_DISABLE_STATIC

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL

dnl Check for X11
XDT_CHECK_LIBX11_REQUIRE

dnl configure the panel plugin
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-1.0], [4.3.20])

dnl Check for i18n support
XDT_I18N([bg hu ja pl])

dnl check for debugging support
XDT_FEATURE_DEBUG()

AC_OUTPUT([
panel-plugin/Makefile
flags/Makefile
Makefile
po/Makefile.in
])
