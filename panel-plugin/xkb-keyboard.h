/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-keyboard.h
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

#ifndef _xkb_keyboard_H_
#define _xkb_keyboard_H_

#include <gdk/gdk.h>

#include "xkb-properties.h"

G_BEGIN_DECLS

typedef struct _XkbKeyboardClass      XkbKeyboardClass;
typedef struct _XkbKeyboard           XkbKeyboard;

#define TYPE_XKB_KEYBOARD             (xkb_keyboard_get_type ())
#define XKB_KEYBOARD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XKB_KEYBOARD, XkbKeyboard))
#define XKB_KEYBOARD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  TYPE_XKB_KEYBOARD, XkbKeyboardClass))
#define IS_XKB_KEYBOARD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XKB_KEYBOARD))
#define IS_XKB_KEYBOARD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  TYPE_XKB_KEYBOARD))
#define XKB_KEYBOARD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  TYPE_XKB_KEYBOARD, XkbKeyboard))

GType             xkb_keyboard_get_type                     (void)                           G_GNUC_CONST;

XkbKeyboard      *xkb_keyboard_new                          (XkbGroupPolicy group_policy);

gboolean          xkb_keyboard_get_initialized              (XkbKeyboard     *keyboard);
void              xkb_keyboard_set_group_policy             (XkbKeyboard     *keyboard,
                                                             XkbGroupPolicy   group_policy);
gint              xkb_keyboard_get_group_count              (XkbKeyboard     *keyboard);
guint             xkb_keyboard_get_max_group_count          (XkbKeyboard     *keyboard);
const gchar*      xkb_keyboard_get_group_name               (XkbKeyboard     *keyboard,
                                                             XkbDisplayName   display_name,
                                                             gint             group);
gint              xkb_keyboard_get_variant_index            (XkbKeyboard     *keyboard,
                                                             XkbDisplayName   display_name,
                                                             gint             group);
gboolean          xkb_keyboard_set_group                    (XkbKeyboard     *keyboard,
                                                             gint             group);
gboolean          xkb_keyboard_next_group                   (XkbKeyboard     *keyboard);
gboolean          xkb_keyboard_prev_group                   (XkbKeyboard     *keyboard);

GdkPixbuf*        xkb_keyboard_get_pixbuf                   (XkbKeyboard     *keyboard,
                                                             gboolean         tooltip,
                                                             gint             group);
gchar*            xkb_keyboard_get_pretty_layout_name       (XkbKeyboard     *keyboard,
                                                             gint             group);
gint              xkb_keyboard_get_current_group            (XkbKeyboard     *keyboard);

G_END_DECLS

#endif
