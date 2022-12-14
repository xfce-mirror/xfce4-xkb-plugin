/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-plugin.h
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

#include <libxfce4panel/libxfce4panel.h>

#include "xkb-xfconf.h"

G_BEGIN_DECLS

typedef struct _XkbPluginClass      XkbPluginClass;
typedef struct _XkbPlugin           XkbPlugin;

#define TYPE_XKB_PLUGIN             (xkb_plugin_get_type ())
#define XKB_PLUGIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XKB_PLUGIN, XkbPlugin))
#define XKB_PLUGIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  TYPE_XKB_PLUGIN, XkbPluginClass))
#define IS_XKB_PLUGIN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XKB_PLUGIN))
#define IS_XKB_PLUGIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  TYPE_XKB_PLUGIN))
#define XKB_PLUGIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  TYPE_XKB_PLUGIN, XkbPlugin))

GType             xkb_plugin_get_type                     (void)                       G_GNUC_CONST;

void              xkb_plugin_register_type                (XfcePanelTypeModule   *type_module);
void              xkb_plugin_configure_layout             (GtkWidget             *widget);

G_END_DECLS

#endif
