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

char * initialize_xkb();
void catch_the_keys(void *ptr(void *));
const char *get_current_group_name();
void terminate();
int do_change_group(int increment, void *ptr(void *));
void deinitialize_xkb();

#endif
