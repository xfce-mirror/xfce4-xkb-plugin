//====================================================================
//  xfce4-xkb-plugin - XFCE4 Xkb Layout Indicator panel plugin
// -------------------------------------------------------------------
//  Alexander Iliev <sasoiliev@mail.bg>
//  20-Feb-04
// -------------------------------------------------------------------
//  Parts of this code belong to Michael Glickman <wmalms@yahooo.com>
//  and his program wmxkb.
//  WARNING: DO NOT BOTHER Michael Glickman WITH QUESTIONS ABOUT THIS
//           PROGRAM!!! SEND INSTEAD EMAILS TO <sasoiliev@mail.bg>
//====================================================================

#ifndef _XFCE_XKB_H_
#define _XFCE_XKB_H_

#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <glib.h>

char * initialize_xkb(GtkWidget *ctrl);
void deinitialize_xkb();

int do_change_group(int increment, GtkWidget *ctrl);

gboolean gio_callback(GIOChannel *source, GIOCondition condition, gpointer data);

int get_connection_number();

#endif
