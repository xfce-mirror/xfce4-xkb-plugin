/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-callbacks.h
 * Copyright (C) 2008 Alexander Iliev <sasoiliev@mamul.org>
 *
 * Parts of this program comes from the XfKC tool:
 * Copyright (C) 2006 Gauvain Pocentek <gauvainpocentek@gmail.com>
 *
 * A part of this file comes from the gnome keyboard capplet (control-center):
 * Copyright (C) 2003 Sergey V. Oudaltsov <svu@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _XKB_CALLBACKS_H_
#define _XKB_CALLBACKS_H_

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libwnck/libwnck.h>

#include "xfce4-xkb-plugin.h"

gboolean        xkb_plugin_layout_image_exposed     (GtkWidget *widget,
                                 GdkEventExpose *event,
                                 t_xkb *xkb);

gboolean        xkb_plugin_button_entered       (GtkWidget *widget,
                                 GdkEventCrossing *event,
                                 t_xkb *xkb);

gboolean        xkb_plugin_button_left          (GtkWidget *widget,
                                 GdkEventCrossing *event,
                                 t_xkb *xkb);

void            xkb_plugin_button_size_allocated    (GtkWidget *button,
                                 GtkAllocation *allocation,
                                 t_xkb *xkb);

void            xkb_plugin_active_window_changed    (WnckScreen *screen,
                                 WnckWindow *previously_active_window,
                                 t_xkb *xkb);

void            xkb_plugin_application_closed       (WnckScreen *screen,
                                 WnckApplication *app,
                                 t_xkb *xkb);

void            xkb_plugin_window_closed        (WnckScreen *screen,
                                 WnckWindow *window,
                                 t_xkb *xkb);

void            xkb_plugin_button_clicked       (GtkButton *btn,
                                 gpointer data);

gboolean        xkb_plugin_button_scrolled      (GtkWidget *btn,
                                 GdkEventScroll *event,
                                 gpointer data);

gboolean        xkb_plugin_set_tooltip          (GtkWidget *widget,
                                 gint x,
                                 gint y,
                                 gboolean keyboard_mode,
                                 GtkTooltip *tooltip,
                                 t_xkb *xkb);

#endif

