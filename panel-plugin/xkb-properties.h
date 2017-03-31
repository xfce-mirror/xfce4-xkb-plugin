/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-properties.h
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

#ifndef _XKB_PROPERTIES_H_
#define _XKB_PROPERTIES_H_

#define DISPLAY_TYPE "display-type"
#define DISPLAY_SCALE "display-scale"
#define DISPLAY_TOOLTIP_ICON "display-tooltip-icon"
#define GROUP_POLICY "group-policy"

typedef enum
{
    DISPLAY_TYPE_IMAGE              = 0,
    DISPLAY_TYPE_TEXT               = 1,
    DISPLAY_TYPE_SYSTEM             = 2
} XkbDisplayType;

#define DISPLAY_SCALE_MIN             0
#define DISPLAY_SCALE_MAX             100

typedef enum
{
    GROUP_POLICY_GLOBAL             = 0,
    GROUP_POLICY_PER_WINDOW         = 1,
    GROUP_POLICY_PER_APPLICATION    = 2
} XkbGroupPolicy;

#endif
