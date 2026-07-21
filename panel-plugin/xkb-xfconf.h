/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-xfconf.h
 * Copyright (C) 2008 Alexander Iliev <sasoiliev@mamul.org>
 *
 * Parts of this program comes from the XfKC tool:
 * Copyright (C) 2006 Gauvain Pocentek <gauvainpocentek@gmail.com>
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

#ifndef _XKB_XFCONF_H_
#define _XKB_XFCONF_H_

#include <glib-object.h>
#include "xkb-properties.h"

G_BEGIN_DECLS

#define XKB_TYPE_XFCONF (xkb_xfconf_get_type ())
G_DECLARE_FINAL_TYPE (XkbXfconf, xkb_xfconf, XKB, XFCONF, GObject)

XkbXfconf      *xkb_xfconf_new                             (const gchar   *property_base);

XkbDisplayType  xkb_xfconf_get_display_type                (XkbXfconf     *config);
XkbDisplayName  xkb_xfconf_get_display_name                (XkbXfconf     *config);
guint           xkb_xfconf_get_display_scale               (XkbXfconf     *config);
gboolean        xkb_xfconf_get_caps_lock_indicator         (XkbXfconf     *config);
#ifdef HAVE_LIBNOTIFY
gboolean        xkb_xfconf_get_show_notifications          (XkbXfconf     *config);
#endif
gboolean        xkb_xfconf_get_display_tooltip_icon        (XkbXfconf     *config);
XkbGroupPolicy  xkb_xfconf_get_group_policy                (XkbXfconf     *config);
const gchar *   xkb_xfconf_get_layout_defaults             (XkbXfconf     *config,
                                                            guint          layout);

G_END_DECLS

#endif
