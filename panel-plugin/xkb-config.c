/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-config.c
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

#include "xkb-config.h"
#include "xkb-util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxklavier/xklavier.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <librsvg/rsvg.h>

#ifndef DEBUG
#define G_DISABLE_ASSERT
#endif

typedef struct
{
    gchar      *group_name;
    gchar      *variant;
    gchar      *pretty_layout_name;
    GdkPixbuf  *tooltip_pixbuf;
} t_group_data;

typedef struct
{
    XklEngine            *engine;

    t_group_data         *group_data;

    t_group_policy        group_policy;
    GHashTable           *variant_index_by_group;

    GHashTable           *application_map;
    GHashTable           *window_map;

    guint                 current_window_id;
    guint                 current_application_id;

    gint                  group_count;
    gint                  current_group;

    XkbCallback           callback;
    gpointer              callback_data;
} t_xkb_config;

static t_xkb_config *config;

static void         xkb_config_xkl_state_changed        (XklEngine *engine,
                                                         XklEngineStateChange change,
                                                         gint group,
                                                         gboolean restore,
                                                         gpointer user_data);

static void         xkb_config_xkl_config_changed       (XklEngine *engine,
                                                         gpointer user_data);

static GdkFilterReturn
                    handle_xevent                       (GdkXEvent * xev,
                                                         GdkEvent * event);

static void         xkb_config_free                     ();
static void         xkb_config_update_from_xkl          ();
static void         xkb_config_initialize_xkb_options   (const XklConfigRec *config_rec);

/* ---------------------- implementation ------------------------- */

gboolean
xkb_config_initialize (t_group_policy group_policy,
               XkbCallback callback,
               gpointer callback_data)
{
    config = g_new0 (t_xkb_config, 1);

    config->group_policy = group_policy;

    config->callback = callback;
    config->callback_data = callback_data;

    config->engine = xkl_engine_get_instance (GDK_DISPLAY ());

    if (!config->engine)
    {
        return FALSE;
    }

    xkb_config_update_from_xkl ();

    xkl_engine_set_group_per_toplevel_window (config->engine, FALSE);

    xkl_engine_start_listen (config->engine, XKLL_TRACK_KEYBOARD_STATE);

    g_signal_connect (config->engine,
            "X-state-changed",
            G_CALLBACK (xkb_config_xkl_state_changed),
            NULL);
    g_signal_connect (config->engine,
            "X-config-changed",
            G_CALLBACK (xkb_config_xkl_config_changed),
            NULL);
    gdk_window_add_filter (NULL, (GdkFilterFunc) handle_xevent, NULL);

    return TRUE;
}

static gchar *
xkb_config_xkb_description (XklConfigItem *config_item)
{
  gchar *ci_description;
  gchar *description;

  ci_description = g_strstrip (config_item->description);

  if (ci_description[0] == 0)
    description = g_strdup (config_item->name);
  else
    description = g_locale_to_utf8 (ci_description, -1, NULL, NULL, NULL);

  return description;
}

static gchar*
xkb_config_create_pretty_layout_name (XklConfigRegistry *registry,
                                      XklConfigItem *config_item,
                                      gchar *layout_name,
                                      gchar *layout_variant)
{
    gchar *pretty_layout_name;

    g_snprintf (config_item->name, sizeof (config_item->name),
                "%s", layout_variant);
    if (xkl_config_registry_find_variant (registry, layout_name, config_item))
    {
        pretty_layout_name = xkb_config_xkb_description (config_item);
    }
    else
    {
        g_snprintf (config_item->name, sizeof (config_item->name),
                    "%s", layout_name);
        if (xkl_config_registry_find_layout (registry, config_item))
        {
            pretty_layout_name = xkb_config_xkb_description (config_item);
        }
        else
        {
            pretty_layout_name = xkb_util_get_layout_string (layout_name,
                                                             layout_variant);
        }
    }

    return pretty_layout_name;
}

static void
xkb_config_initialize_xkb_options (const XklConfigRec *config_rec)
{
    GHashTable *index_variants;
    gchar **group;
    gint val, i;
    gpointer pval;
    gchar *imgfilename;
    XklConfigRegistry *registry;
    XklConfigItem *config_item;

    xkb_config_free ();

    group = config_rec->layouts;
    config->group_count = 0;
    while (*group)
    {
        group++;
        config->group_count++;
    }

    config->window_map = g_hash_table_new (g_direct_hash, NULL);
    config->application_map = g_hash_table_new (g_direct_hash, NULL);
    config->group_data = (t_group_data *) g_new0 (typeof (t_group_data),
                                                  config->group_count);
    config->variant_index_by_group = g_hash_table_new (NULL, NULL);
    index_variants = g_hash_table_new (g_str_hash, g_str_equal);

    registry = xkl_config_registry_get_instance (config->engine);
    xkl_config_registry_load (registry, FALSE);
    config_item = xkl_config_item_new ();

    for (i = 0; i < config->group_count; i++)
    {
        t_group_data *group_data = &config->group_data[i];
        RsvgHandle *handle;

        group_data->group_name = g_strdup (config_rec->layouts[i]);

        group_data->variant = (config_rec->variants[i] == NULL)
            ? g_strdup ("") : g_strdup (config_rec->variants[i]);

        pval = g_hash_table_lookup (
                index_variants,
                group_data->group_name
        );
        val = (pval != NULL) ? GPOINTER_TO_INT (pval) : 0;
        val++;
        g_hash_table_insert (
                config->variant_index_by_group,
                GINT_TO_POINTER (i),
                GINT_TO_POINTER (val)
        );
        g_hash_table_insert (
                index_variants,
                group_data->group_name,
                GINT_TO_POINTER (val)
        );

        imgfilename = xkb_util_get_flag_filename (group_data->group_name);
        handle = rsvg_handle_new_from_file (imgfilename, NULL);
        if (handle)
        {
            GdkPixbuf *tmp = rsvg_handle_get_pixbuf (handle);
            group_data->tooltip_pixbuf =
                gdk_pixbuf_scale_simple (tmp, 30, 22, GDK_INTERP_BILINEAR);
            g_object_unref (tmp);
            rsvg_handle_close (handle, NULL);
            g_object_unref (handle);
        }
        g_free (imgfilename);

        group_data->pretty_layout_name =
            xkb_config_create_pretty_layout_name (registry, config_item,
                                                  group_data->group_name,
                                                  group_data->variant);
    }
    g_object_unref (config_item);
    g_object_unref (registry);
    g_hash_table_destroy (index_variants);
}

static void
xkb_config_free (void)
{
    gint i;

    g_assert (config != NULL);

    if (config->variant_index_by_group)
        g_hash_table_destroy (config->variant_index_by_group);

    if (config->window_map)
        g_hash_table_destroy (config->window_map);

    if (config->application_map)
        g_hash_table_destroy (config->application_map);

    if (config->group_data)
    {
        for (i = 0; i < config->group_count; i++)
        {
            t_group_data *group_data = &config->group_data[i];
            g_free (group_data->group_name);
            g_free (group_data->variant);
            g_free (group_data->pretty_layout_name);
            if (group_data->tooltip_pixbuf)
            {
                g_object_unref (group_data->tooltip_pixbuf);
            }
        }
        g_free (config->group_data);
    }
}

void
xkb_config_finalize (void)
{
    xkl_engine_stop_listen (config->engine, XKLL_TRACK_KEYBOARD_STATE);
    g_object_unref (config->engine);

    xkb_config_free ();
    g_free (config);

    gdk_window_remove_filter (NULL, (GdkFilterFunc) handle_xevent, NULL);
}

gint
xkb_config_get_current_group (void)
{
    return config->current_group;
}

gboolean
xkb_config_set_group (gint group)
{
    g_assert (config != NULL);

    if (G_UNLIKELY (group < 0 || group >= config->group_count))
    {
        return FALSE;
    }

    xkl_engine_lock_group (config->engine, group);
    config->current_group = group;

    return TRUE;
}

gboolean
xkb_config_next_group (void)
{
    xkl_engine_lock_group (config->engine,
            xkl_engine_get_next_group (config->engine));

    return TRUE;
}

gboolean
xkb_config_prev_group (void)
{
    xkl_engine_lock_group (config->engine,
            xkl_engine_get_prev_group (config->engine));

    return TRUE;
}

void
xkb_config_set_group_policy (t_group_policy group_policy)
{
    config->group_policy = group_policy;
}

static void
xkb_config_update_from_xkl (void)
{
    XklConfigRec *config_rec;


    g_assert (config != NULL);

    config_rec = xkl_config_rec_new ();
    xkl_config_rec_get_from_server (config_rec, config->engine);

    xkb_config_initialize_xkb_options (config_rec);

    g_object_unref (config_rec);
}

void
xkb_config_window_changed (guint new_window_id, guint application_id)
{
    gint group;
    gpointer key, value;
    GHashTable *hashtable;
    guint id;

    g_assert (config != NULL);

    id = 0;
    hashtable = NULL;

    switch (config->group_policy)
    {
        case GROUP_POLICY_GLOBAL:
            return;

        case GROUP_POLICY_PER_WINDOW:
            hashtable = config->window_map;
            id = new_window_id;
            config->current_window_id = id;
            break;

        case GROUP_POLICY_PER_APPLICATION:
            hashtable = config->application_map;
            id = application_id;
            config->current_application_id = id;
            break;
    }

    group = 0;

    if (g_hash_table_lookup_extended (hashtable, GINT_TO_POINTER (id), &key, &value))
    {
        group = GPOINTER_TO_INT (value);
    }
    else
    {
        g_hash_table_insert (hashtable,
                             GINT_TO_POINTER (id),
                             GINT_TO_POINTER (group));
    }

    xkb_config_set_group (group);
}

void
xkb_config_application_closed (guint application_id)
{
    g_assert (config != NULL);

    switch (config->group_policy)
    {
        case GROUP_POLICY_GLOBAL:
        case GROUP_POLICY_PER_WINDOW:
            return;

        case GROUP_POLICY_PER_APPLICATION:
            g_hash_table_remove (
                    config->application_map,
                    GINT_TO_POINTER (application_id)
            );

            break;
    }
}

void
xkb_config_window_closed (guint window_id)
{
    g_assert (config != NULL);

    switch (config->group_policy)
    {
        case GROUP_POLICY_GLOBAL:
        case GROUP_POLICY_PER_APPLICATION:
            return;

        case GROUP_POLICY_PER_WINDOW:
            g_hash_table_remove (
                    config->window_map,
                    GINT_TO_POINTER (window_id)
            );

            break;
    }
}

gint
xkb_config_get_group_count (void)
{
    g_assert (config != NULL);

    return config->group_count;
}

guint xkb_config_get_max_group_count (void)
{
    return xkl_engine_get_max_num_groups(config->engine);
}

const gchar*
xkb_config_get_group_name (gint group)
{
    g_assert (config != NULL);

    if (G_UNLIKELY (group >= config->group_count))
        return NULL;

    if (group == -1)
        group = xkb_config_get_current_group ();

    return config->group_data[group].group_name;
}

const gchar*
xkb_config_get_variant (gint group)
{
    g_assert (config != NULL);

    if (G_UNLIKELY (group >= config->group_count))
        return NULL;

    if (group == -1)
        group = xkb_config_get_current_group ();

    return config->group_data[group].variant;
}

void
xkb_config_xkl_state_changed (XklEngine *engine,
                              XklEngineStateChange change,
                              gint group,
                              gboolean restore,
                              gpointer user_data)
{
    if (change == GROUP_CHANGED)
    {
        config->current_group = group;

        switch (config->group_policy)
        {
            case GROUP_POLICY_GLOBAL:
                break;

            case GROUP_POLICY_PER_WINDOW:
                g_hash_table_insert (
                        config->window_map,
                        GINT_TO_POINTER (config->current_window_id),
                        GINT_TO_POINTER (group)
                );
                break;

            case GROUP_POLICY_PER_APPLICATION:
                g_hash_table_insert (
                        config->application_map,
                        GINT_TO_POINTER (config->current_application_id),
                        GINT_TO_POINTER (group)
                );
            break;
        }

        if (config->callback != NULL) config->callback (group, FALSE, config->callback_data);
    }
}

void
xkb_config_xkl_config_changed (XklEngine *engine, gpointer user_data)
{
    xkb_config_update_from_xkl ();

    if (config->callback != NULL)
    {
        xkb_config_set_group (0);
        config->callback (0, TRUE, config->callback_data);
    }
}

gint
xkb_config_variant_index_for_group (gint group)
{
    gpointer presult;
    gint result;

    g_return_val_if_fail (config != NULL, 0);

    if (group == -1) group = xkb_config_get_current_group ();

    presult = g_hash_table_lookup (
            config->variant_index_by_group,
            GINT_TO_POINTER (group)
    );
    if (presult == NULL) return 0;

    result = GPOINTER_TO_INT (presult);
    result = (result <= 0) ? 0 : result - 1;
    return result;
}

GdkFilterReturn
handle_xevent (GdkXEvent * xev, GdkEvent * event)
{
    XEvent *xevent = (XEvent *) xev;

    xkl_engine_filter_events (config->engine, xevent);

    return GDK_FILTER_CONTINUE;
}

XklConfigRegistry*
xkb_config_get_xkl_registry (void)
{
    XklConfigRegistry *registry;

    if (!config) return NULL;

    registry = xkl_config_registry_get_instance (config->engine);
    xkl_config_registry_load (registry, FALSE);

    return registry;
}

GdkPixbuf*
xkb_config_get_tooltip_pixbuf (gint group)
{
    return config->group_data[group].tooltip_pixbuf;
}

gchar*
xkb_config_get_pretty_layout_name (gint group)
{
    return config->group_data[group].pretty_layout_name;
}
