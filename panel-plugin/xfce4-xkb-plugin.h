/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xfce4-xkb-plugin.h
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

#ifndef _XFCE_XKB_H_
#define _XFCE_XKB_H_

#include "xkb-config.h"

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <glib.h>

typedef enum
{
    DISPLAY_TYPE_IMAGE = 0,
    DISPLAY_TYPE_TEXT = 1
} t_display_type;

typedef enum
{
    DISPLAY_TEXTSIZE_SMALL = 0,
    DISPLAY_TEXTSIZE_MEDIUM = 1,
    DISPLAY_TEXTSIZE_LARGE = 2
} t_display_textsize;

typedef struct
{
    XfcePanelPlugin *plugin;

    t_display_type display_type;  /* display layout as image ot text */
    guint display_text_scale; /* text scale % for text layout */
    guint display_img_scale;  /* image scale % for flag layout */
    t_group_policy group_policy;     /* per-app/window/global policy */

    /* widgets */
    GtkWidget *btn;
    GtkWidget *layout_image;
    GtkWidget *popup;
} t_xkb;

#endif

