/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-xfconf.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xkb-properties.h"
#include "xkb-xfconf.h"

#include <string.h>

#include <xfconf/xfconf.h>

#define DEFAULT_DISPLAY_TYPE                DISPLAY_TYPE_IMAGE
#define DEFAULT_DISPLAY_SCALE               DISPLAY_SCALE_MAX
#define DEFAULT_DISPLAY_TOOLTIP_ICON        TRUE
#define DEFAULT_GROUP_POLICY                GROUP_POLICY_PER_APPLICATION

static void            xkb_xfconf_finalize            (GObject          *object);
static void            xkb_xfconf_get_property        (GObject          *object,
                                                       guint             prop_id,
                                                       GValue           *value,
                                                       GParamSpec       *pspec);
static void            xkb_xfconf_set_property        (GObject          *object,
                                                       guint             prop_id,
                                                       const GValue     *value,
                                                       GParamSpec       *pspec);

struct _XkbXfconfClass
{
    GObjectClass __parent__;
};

struct _XkbXfconf
{
    GObject __parent__;

    guint display_type;
    guint display_scale;
    gboolean display_tooltip_icon;
    guint group_policy;
};

enum
{
    PROP_0,
    PROP_DISPLAY_TYPE,
    PROP_DISPLAY_SCALE,
    PROP_DISPLAY_TOOLTIP_ICON,
    PROP_GROUP_POLICY,
    N_PROPERTIES,
};

enum
{
    CONFIGURATION_CHANGED,
    LAST_SIGNAL
};

static guint xkb_xfconf_signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (XkbXfconf, xkb_xfconf, G_TYPE_OBJECT)

static void
xkb_xfconf_class_init (XkbXfconfClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = xkb_xfconf_finalize;
    gobject_class->get_property = xkb_xfconf_get_property;
    gobject_class->set_property = xkb_xfconf_set_property;

    g_object_class_install_property (gobject_class, PROP_DISPLAY_TYPE,
            g_param_spec_uint (DISPLAY_TYPE, NULL, NULL,
                    DISPLAY_TYPE_IMAGE, DISPLAY_TYPE_SYSTEM, DEFAULT_DISPLAY_TYPE,
                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (gobject_class, PROP_DISPLAY_SCALE,
            g_param_spec_uint (DISPLAY_SCALE, NULL, NULL,
                    DISPLAY_SCALE_MIN, DISPLAY_SCALE_MAX, DEFAULT_DISPLAY_SCALE,
                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (gobject_class, PROP_DISPLAY_TOOLTIP_ICON,
            g_param_spec_boolean (DISPLAY_TOOLTIP_ICON, NULL, NULL,
                    DEFAULT_DISPLAY_TOOLTIP_ICON,
                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (gobject_class, PROP_GROUP_POLICY,
            g_param_spec_uint (GROUP_POLICY, NULL, NULL,
                    GROUP_POLICY_GLOBAL, GROUP_POLICY_PER_APPLICATION, DEFAULT_GROUP_POLICY,
                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    xkb_xfconf_signals[CONFIGURATION_CHANGED] =
            g_signal_new (g_intern_static_string ("configuration-changed"),
                    G_TYPE_FROM_CLASS (gobject_class),
                    G_SIGNAL_RUN_LAST,
                    0, NULL, NULL,
                    g_cclosure_marshal_VOID__VOID,
                    G_TYPE_NONE, 0);
}

static void
xkb_xfconf_init (XkbXfconf *config)
{
    config->display_type = DEFAULT_DISPLAY_TYPE;
    config->display_scale = DEFAULT_DISPLAY_SCALE;
    config->display_tooltip_icon = DEFAULT_DISPLAY_TOOLTIP_ICON;
    config->group_policy = DEFAULT_GROUP_POLICY;
}

static void
xkb_xfconf_finalize (GObject *object)
{
    xfconf_shutdown ();
    G_OBJECT_CLASS (xkb_xfconf_parent_class)->finalize (object);
}

static void
xkb_xfconf_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    XkbXfconf *config = XKB_XFCONF (object);

    switch (prop_id)
    {
        case PROP_DISPLAY_TYPE:
            g_value_set_uint (value, config->display_type);
            break;
        case PROP_DISPLAY_SCALE:
            g_value_set_uint (value, config->display_scale);
            break;
        case PROP_DISPLAY_TOOLTIP_ICON:
            g_value_set_boolean (value, config->display_tooltip_icon);
            break;
        case PROP_GROUP_POLICY:
            g_value_set_uint (value, config->group_policy);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xkb_xfconf_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    XkbXfconf *config = XKB_XFCONF (object);
    guint val_uint;
    gboolean val_boolean;

    switch (prop_id)
    {
        case PROP_DISPLAY_TYPE:
            val_uint = g_value_get_uint (value);
            if (config->display_type != val_uint)
            {
                config->display_type = val_uint;
                g_object_notify (G_OBJECT (config), DISPLAY_TYPE);
                g_signal_emit (G_OBJECT (config), xkb_xfconf_signals [CONFIGURATION_CHANGED], 0);
            }
            break;
        case PROP_DISPLAY_SCALE:
            val_uint = g_value_get_uint (value);
            if (config->display_scale != val_uint)
            {
                config->display_scale = val_uint;
                g_object_notify (G_OBJECT (config), DISPLAY_SCALE);
                g_signal_emit (G_OBJECT (config), xkb_xfconf_signals [CONFIGURATION_CHANGED], 0);
            }
            break;
        case PROP_DISPLAY_TOOLTIP_ICON:
            val_boolean = g_value_get_boolean (value);
            if (config->display_tooltip_icon != val_boolean)
            {
                config->display_tooltip_icon = val_boolean;
                g_object_notify (G_OBJECT (config), DISPLAY_TOOLTIP_ICON);
                g_signal_emit (G_OBJECT (config), xkb_xfconf_signals [CONFIGURATION_CHANGED], 0);
            }
            break;
        case PROP_GROUP_POLICY:
            val_uint = g_value_get_uint (value);
            if (config->group_policy != val_uint)
            {
                config->group_policy = val_uint;
                g_object_notify (G_OBJECT (config), GROUP_POLICY);
                g_signal_emit (G_OBJECT (config), xkb_xfconf_signals [CONFIGURATION_CHANGED], 0);
            }
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

guint
xkb_xfconf_get_display_type (XkbXfconf *config)
{
    g_return_val_if_fail (IS_XKB_XFCONF (config), DEFAULT_DISPLAY_TYPE);
    return config->display_type;
}

guint
xkb_xfconf_get_display_scale (XkbXfconf *config)
{
    g_return_val_if_fail (IS_XKB_XFCONF (config), DEFAULT_DISPLAY_SCALE);
    return config->display_scale;
}

gboolean
xkb_xfconf_get_display_tooltip_icon (XkbXfconf *config)
{
    g_return_val_if_fail (IS_XKB_XFCONF (config), DEFAULT_DISPLAY_TOOLTIP_ICON);
    return config->display_tooltip_icon;
}

guint
xkb_xfconf_get_group_policy (XkbXfconf *config)
{
    g_return_val_if_fail (IS_XKB_XFCONF (config), DEFAULT_GROUP_POLICY);
    return config->group_policy;
}

XkbXfconf *
xkb_xfconf_new (const gchar *property_base)
{
    XkbXfconf *config;
    XfconfChannel *channel;
    gchar *property;

    config = g_object_new (TYPE_XKB_XFCONF, NULL);

    if (xfconf_init (NULL))
    {
        channel = xfconf_channel_get ("xfce4-panel");

        property = g_strconcat (property_base, "/" DISPLAY_TYPE, NULL);
        xfconf_g_property_bind (channel, property, G_TYPE_UINT, config, DISPLAY_TYPE);
        g_free (property);

        property = g_strconcat (property_base, "/" DISPLAY_SCALE, NULL);
        xfconf_g_property_bind (channel, property, G_TYPE_UINT, config, DISPLAY_SCALE);
        g_free (property);

        property = g_strconcat (property_base, "/" DISPLAY_TOOLTIP_ICON, NULL);
        xfconf_g_property_bind (channel, property, G_TYPE_BOOLEAN, config, DISPLAY_TOOLTIP_ICON);
        g_free (property);

        property = g_strconcat (property_base, "/" GROUP_POLICY, NULL);
        xfconf_g_property_bind (channel, property, G_TYPE_UINT, config, GROUP_POLICY);
        g_free (property);
    }

    return config;
}
