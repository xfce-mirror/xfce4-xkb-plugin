/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-keyboard.c
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

#include "xkb-keyboard.h"
#include "xkb-util.h"

#include <gdk/gdkx.h>
#include <libxklavier/xklavier.h>
#include <libwnck/libwnck.h>
#include <librsvg/rsvg.h>

typedef struct
{
    gchar                *country_name;
    gint                  country_index;
    gchar                *language_name;
    gint                  language_index;
    gchar                *variant;
    gchar                *pretty_layout_name;
    GdkPixbuf            *display_pixbuf;
    GdkPixbuf            *tooltip_pixbuf;
} XkbGroupData;

struct _XkbKeyboardClass
{
    GObjectClass __parent__;
};

struct _XkbKeyboard
{
    GObject __parent__;
    
    XklEngine            *engine;
    XklConfigRec         *last_config_rec;

    WnckScreen           *wnck_screen;

    guint                 config_timeout_id;

    XkbGroupData         *group_data;

    XkbGroupPolicy        group_policy;

    GHashTable           *application_map;
    GHashTable           *window_map;

    guint                 current_window_id;
    guint                 current_application_id;

    gint                  group_count;
    gint                  current_group;

    gulong                active_window_changed_handler_id;
    gulong                application_closed_handler_id;
    gulong                window_closed_handler_id;
};

static void              xkb_keyboard_active_window_changed   (WnckScreen            *screen,
                                                               WnckWindow            *previously_active_window,
                                                               XkbKeyboard           *keyboard);
static void              xkb_keyboard_application_closed      (WnckScreen            *screen,
                                                               WnckApplication       *application,
                                                               XkbKeyboard           *keyboard);
static void              xkb_keyboard_window_closed           (WnckScreen            *screen,
                                                               WnckWindow            *window,
                                                               XkbKeyboard           *keyboard);

static void              xkb_keyboard_xkl_state_changed        (XklEngine *engine,
                                                                XklEngineStateChange change,
                                                                gint group,
                                                                gboolean restore,
                                                                gpointer user_data);

static void              xkb_keyboard_xkl_config_changed       (XklEngine *engine,
                                                                gpointer user_data);

static GdkFilterReturn   xkb_keyboard_handle_xevent            (GdkXEvent * xev,
                                                                GdkEvent * event,
                                                                gpointer user_data);

static void              xkb_keyboard_free                     (XkbKeyboard *keyboard);
static void              xkb_keyboard_finalize                 (GObject *object);
static gboolean          xkb_keyboard_update_from_xkl          (XkbKeyboard *keyboard);
static void              xkb_keyboard_initialize_xkb_options   (XkbKeyboard *keyboard,
                                                                const XklConfigRec *config_rec);

enum
{
    STATE_CHANGED,
    LAST_SIGNAL
};

static guint xkb_keyboard_signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (XkbKeyboard, xkb_keyboard, G_TYPE_OBJECT)

static void
xkb_keyboard_class_init (XkbKeyboardClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = xkb_keyboard_finalize;

    xkb_keyboard_signals[STATE_CHANGED] =
            g_signal_new (g_intern_static_string ("state-changed"),
                    G_TYPE_FROM_CLASS (gobject_class),
                    G_SIGNAL_RUN_LAST,
                    0, NULL, NULL,
                    g_cclosure_marshal_VOID__BOOLEAN,
                    G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
xkb_keyboard_init (XkbKeyboard *keyboard)
{
    keyboard->engine = NULL;
    keyboard->last_config_rec = NULL;

    keyboard->config_timeout_id = 0;

    keyboard->group_data = NULL;
    keyboard->group_policy = GROUP_POLICY_GLOBAL;

    keyboard->application_map = NULL;
    keyboard->window_map = NULL;

    keyboard->current_window_id = 0;
    keyboard->current_application_id = 0;

    keyboard->group_count = 0;
    keyboard->current_group = 0;

    keyboard->active_window_changed_handler_id = 0;
    keyboard->application_closed_handler_id = 0;
    keyboard->window_closed_handler_id = 0;
}

XkbKeyboard *
xkb_keyboard_new (XkbGroupPolicy group_policy)
{
    XkbKeyboard *keyboard;

    keyboard = g_object_new (TYPE_XKB_KEYBOARD, NULL);

    keyboard->group_policy = group_policy;

    keyboard->engine = xkl_engine_get_instance (gdk_x11_get_default_xdisplay ());

    keyboard->wnck_screen = wnck_screen_get_default ();

    if (keyboard->engine)
    {
        xkb_keyboard_update_from_xkl (keyboard);

        xkl_engine_set_group_per_toplevel_window (keyboard->engine, FALSE);

        xkl_engine_start_listen (keyboard->engine, XKLL_TRACK_KEYBOARD_STATE);

        g_signal_connect (keyboard->engine,
                "X-state-changed",
                G_CALLBACK (xkb_keyboard_xkl_state_changed),
                keyboard);
        g_signal_connect (keyboard->engine,
                "X-config-changed",
                G_CALLBACK (xkb_keyboard_xkl_config_changed),
                keyboard);
        gdk_window_add_filter (NULL, xkb_keyboard_handle_xevent, keyboard);

        keyboard->active_window_changed_handler_id =
                g_signal_connect (G_OBJECT (keyboard->wnck_screen), "active-window-changed",
                        G_CALLBACK (xkb_keyboard_active_window_changed), keyboard);
        keyboard->application_closed_handler_id =
                g_signal_connect (G_OBJECT (keyboard->wnck_screen), "application-closed",
                        G_CALLBACK (xkb_keyboard_application_closed), keyboard);
        keyboard->window_closed_handler_id =
                g_signal_connect (G_OBJECT (keyboard->wnck_screen), "window-closed",
                        G_CALLBACK (xkb_keyboard_window_closed), keyboard);
    }

    return keyboard;
}

gboolean
xkb_keyboard_get_initialized (XkbKeyboard *keyboard)
{
    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), FALSE);
    return G_LIKELY (keyboard->engine != NULL);
}

static gchar *
xkb_keyboard_xkb_description (XklConfigItem *config_item)
{
    gchar *ci_description;
    gchar *description;

    ci_description = g_strstrip (config_item->description);

    if (ci_description[0] == 0)
        description = g_strdup (config_item->name);
    else
        description = g_strdup (ci_description);

    return description;
}

static gchar*
xkb_keyboard_create_pretty_layout_name (XklConfigRegistry *registry,
                                        XklConfigItem *config_item,
                                        gchar *layout_name,
                                        gchar *layout_variant)
{
    gchar *pretty_layout_name;

    g_snprintf (config_item->name, sizeof (config_item->name),
                "%s", layout_variant);
    if (xkl_config_registry_find_variant (registry, layout_name, config_item))
    {
        pretty_layout_name = xkb_keyboard_xkb_description (config_item);
    }
    else
    {
        g_snprintf (config_item->name, sizeof (config_item->name),
                "%s", layout_name);
        if (xkl_config_registry_find_layout (registry, config_item))
        {
            pretty_layout_name = xkb_keyboard_xkb_description (config_item);
        }
        else
        {
            pretty_layout_name = xkb_util_get_layout_string (layout_name,
                                                             layout_variant);
        }
    }

    return pretty_layout_name;
}

static gchar *
xkb_keyboard_obtain_language_name (XklConfigRegistry *registry,
                                   XklConfigItem *config_item,
                                   gchar *layout_name)
{
    g_snprintf (config_item->name, sizeof (config_item->name),
            "%s", layout_name);

    if (xkl_config_registry_find_layout (registry, config_item))
    {
        return g_strdup (config_item->short_description);
    }

    return g_strdup (layout_name);
}

static void
xkb_keyboard_initialize_xkb_options (XkbKeyboard *keyboard,
                                     const XklConfigRec *config_rec)
{
    GHashTable *country_indexes, *language_indexes;
    gchar **group;
    gint val, i;
    gpointer pval;
    gchar *imgfilename;
    XklConfigRegistry *registry;
    XklConfigItem *config_item;

    xkb_keyboard_free (keyboard);

    group = config_rec->layouts;
    keyboard->group_count = 0;
    while (*group)
    {
        group++;
        keyboard->group_count++;
    }

    keyboard->window_map = g_hash_table_new (g_direct_hash, NULL);
    keyboard->application_map = g_hash_table_new (g_direct_hash, NULL);
    keyboard->group_data = (XkbGroupData *) g_new0 (XkbGroupData,
            keyboard->group_count);
    country_indexes = g_hash_table_new (g_str_hash, g_str_equal);
    language_indexes = g_hash_table_new (g_str_hash, g_str_equal);

    registry = xkl_config_registry_get_instance (keyboard->engine);
    xkl_config_registry_load (registry, FALSE);
    config_item = xkl_config_item_new ();

    for (i = 0; i < keyboard->group_count; i++)
    {
        XkbGroupData *group_data = &keyboard->group_data[i];
        RsvgHandle *handle;

        group_data->country_name = g_strdup (config_rec->layouts[i]);

        group_data->variant = (config_rec->variants[i] == NULL)
            ? g_strdup ("") : g_strdup (config_rec->variants[i]);

        group_data->pretty_layout_name = xkb_keyboard_create_pretty_layout_name (registry,
                config_item, group_data->country_name, group_data->variant);

        group_data->language_name = xkb_keyboard_obtain_language_name (registry,
                config_item, group_data->country_name);

        #define MODIFY_INDEXES(table, name, index) \
            pval = g_hash_table_lookup ( \
                    table, \
                    group_data->name \
            ); \
            val = (pval != NULL) ? GPOINTER_TO_INT (pval) : 0; \
            val++; \
            group_data->index = val; \
            g_hash_table_insert ( \
                    table, \
                    group_data->name, \
                    GINT_TO_POINTER (val) \
            );

        MODIFY_INDEXES (country_indexes, country_name, country_index);
        MODIFY_INDEXES (language_indexes, language_name, language_index);

        #undef MODIFY_INDEXES

        imgfilename = xkb_util_get_flag_filename (group_data->country_name);
        handle = rsvg_handle_new_from_file (imgfilename, NULL);
        if (handle)
        {
            group_data->display_pixbuf = rsvg_handle_get_pixbuf (handle);
            group_data->tooltip_pixbuf = gdk_pixbuf_scale_simple (group_data->display_pixbuf,
                    30, 22, GDK_INTERP_BILINEAR);
            rsvg_handle_close (handle, NULL);
            g_object_unref (handle);
        }
        g_free (imgfilename);
    }
    g_object_unref (config_item);
    g_object_unref (registry);
    g_hash_table_destroy (country_indexes);
    g_hash_table_destroy (language_indexes);
}

static void
xkb_keyboard_free (XkbKeyboard *keyboard)
{
    gint i;

    if (keyboard->window_map)
        g_hash_table_destroy (keyboard->window_map);

    if (keyboard->application_map)
        g_hash_table_destroy (keyboard->application_map);

    if (keyboard->group_data)
    {
        for (i = 0; i < keyboard->group_count; i++)
        {
            XkbGroupData *group_data = &keyboard->group_data[i];
            g_free (group_data->country_name);
            g_free (group_data->language_name);
            g_free (group_data->variant);
            g_free (group_data->pretty_layout_name);

            if (group_data->display_pixbuf)
                g_object_unref (group_data->display_pixbuf);

            if (group_data->tooltip_pixbuf)
                g_object_unref (group_data->tooltip_pixbuf);
        }
        g_free (keyboard->group_data);
    }
}

static void
xkb_keyboard_finalize (GObject *object)
{
    XkbKeyboard *keyboard = XKB_KEYBOARD (object);

    if (keyboard->engine)
    {
        xkl_engine_stop_listen (keyboard->engine, XKLL_TRACK_KEYBOARD_STATE);
        g_object_unref (keyboard->engine);

        gdk_window_remove_filter (NULL, xkb_keyboard_handle_xevent, keyboard);
    }

    xkb_keyboard_free (keyboard);

    if (keyboard->last_config_rec != NULL)
        g_object_unref (keyboard->last_config_rec);

    if (keyboard->config_timeout_id != 0)
        g_source_remove (keyboard->config_timeout_id);

    if (keyboard->active_window_changed_handler_id > 0)
        g_signal_handler_disconnect (keyboard->wnck_screen, keyboard->active_window_changed_handler_id);

    if (keyboard->application_closed_handler_id > 0)
        g_signal_handler_disconnect (keyboard->wnck_screen, keyboard->application_closed_handler_id);

    if (keyboard->window_closed_handler_id > 0)
        g_signal_handler_disconnect (keyboard->wnck_screen, keyboard->window_closed_handler_id);

    G_OBJECT_CLASS (xkb_keyboard_parent_class)->finalize (object);
}

gboolean
xkb_keyboard_set_group (XkbKeyboard *keyboard, gint group)
{
    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), FALSE);

    if (G_UNLIKELY (keyboard->engine == NULL || group < 0 || group >= keyboard->group_count))
    {
        return FALSE;
    }

    xkl_engine_lock_group (keyboard->engine, group);
    keyboard->current_group = group;

    return TRUE;
}

gboolean
xkb_keyboard_next_group (XkbKeyboard *keyboard)
{
    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), FALSE);

    if (G_UNLIKELY (keyboard->engine == NULL))
    {
        return FALSE;
    }

    xkl_engine_lock_group (keyboard->engine,
            xkl_engine_get_next_group (keyboard->engine));

    return TRUE;
}

gboolean
xkb_keyboard_prev_group (XkbKeyboard *keyboard)
{
    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), FALSE);

    if (G_UNLIKELY (keyboard->engine == NULL))
    {
        return FALSE;
    }

    xkl_engine_lock_group (keyboard->engine,
            xkl_engine_get_prev_group (keyboard->engine));

    return TRUE;
}

void
xkb_keyboard_set_group_policy (XkbKeyboard *keyboard,
                               XkbGroupPolicy group_policy)
{
    g_return_if_fail (IS_XKB_KEYBOARD (keyboard));
    keyboard->group_policy = group_policy;
}

static gboolean
xkb_keyboard_xkl_config_rec_equals (const XklConfigRec * rec1,
                                    const XklConfigRec * rec2)
{
    gint i = 0;

    g_return_val_if_fail (XKL_IS_CONFIG_REC (rec1), FALSE);
    g_return_val_if_fail (XKL_IS_CONFIG_REC (rec2), FALSE);

    #define STRING_ARRAYS_NOT_EQUAL_RETURN(array1, array2) \
        for (i = 0; array1[i] || array2[i]; i++) \
        { \
            if (!array1[i] || !array2[i] || \
                    g_ascii_strcasecmp (array1[i], array2[i]) != 0) \
            { \
                return FALSE; \
            } \
        }

    STRING_ARRAYS_NOT_EQUAL_RETURN (rec1->layouts, rec2->layouts);
    STRING_ARRAYS_NOT_EQUAL_RETURN (rec1->variants, rec2->variants);

    #undef STRING_ARRAYS_NOT_EQUAL_RETURN

    return TRUE;
}

static gboolean
xkb_keyboard_update_from_xkl (XkbKeyboard *keyboard)
{
    XklConfigRec *config_rec;

    config_rec = xkl_config_rec_new ();
    xkl_config_rec_get_from_server (config_rec, keyboard->engine);

    if (keyboard->last_config_rec == NULL ||
            !xkb_keyboard_xkl_config_rec_equals (config_rec, keyboard->last_config_rec))
    {
        xkb_keyboard_initialize_xkb_options (keyboard, config_rec);

        if (keyboard->last_config_rec != NULL)
            g_object_unref (keyboard->last_config_rec);

        keyboard->last_config_rec = config_rec;

        return TRUE;
    }
    else
    {
        g_object_unref (config_rec);

        return FALSE;
    }
}

static void
xkb_keyboard_active_window_changed (WnckScreen  *screen,
                                    WnckWindow  *previously_active_window,
                                    XkbKeyboard *keyboard)
{
    gint group = 0;
    gpointer key, value;
    GHashTable *hashtable = NULL;
    guint id = 0;
    WnckWindow *window;
    guint window_id, application_id;

    g_return_if_fail (IS_XKB_KEYBOARD (keyboard));

    window = wnck_screen_get_active_window (screen);

    if (!WNCK_IS_WINDOW (window))
        return;

    window_id = wnck_window_get_xid (window);
    application_id = wnck_window_get_pid (window);

    switch (keyboard->group_policy)
    {
        case GROUP_POLICY_GLOBAL:
            return;

        case GROUP_POLICY_PER_WINDOW:
            hashtable = keyboard->window_map;
            id = window_id;
            keyboard->current_window_id = id;
            break;

        case GROUP_POLICY_PER_APPLICATION:
            hashtable = keyboard->application_map;
            id = application_id;
            keyboard->current_application_id = id;
            break;
    }

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

    xkb_keyboard_set_group (keyboard, group);
}

static void
xkb_keyboard_application_closed (WnckScreen      *screen,
                                 WnckApplication *application,
                                 XkbKeyboard     *keyboard)
{
    guint application_id;

    g_return_if_fail (IS_XKB_KEYBOARD (keyboard));

    application_id = wnck_application_get_pid (application);

    switch (keyboard->group_policy)
    {
        case GROUP_POLICY_GLOBAL:
        case GROUP_POLICY_PER_WINDOW:
            return;

        case GROUP_POLICY_PER_APPLICATION:
            g_hash_table_remove (
                    keyboard->application_map,
                    GINT_TO_POINTER (application_id)
            );

            break;
    }
}

static void
xkb_keyboard_window_closed (WnckScreen  *screen,
                            WnckWindow  *window,
                            XkbKeyboard *keyboard)
{
    guint window_id;

    g_return_if_fail (IS_XKB_KEYBOARD (keyboard));

    window_id = wnck_window_get_xid (window);

    switch (keyboard->group_policy)
    {
        case GROUP_POLICY_GLOBAL:
        case GROUP_POLICY_PER_APPLICATION:
            return;

        case GROUP_POLICY_PER_WINDOW:
            g_hash_table_remove (
                    keyboard->window_map,
                    GINT_TO_POINTER (window_id)
            );

            break;
    }
}

gint
xkb_keyboard_get_group_count (XkbKeyboard *keyboard)
{
    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), 0);

    return keyboard->group_count;
}

guint
xkb_keyboard_get_max_group_count (XkbKeyboard *keyboard)
{
    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), 0);

    if (G_UNLIKELY (keyboard->engine == NULL))
    {
        return 0;
    }

    return xkl_engine_get_max_num_groups(keyboard->engine);
}

const gchar*
xkb_keyboard_get_group_name (XkbKeyboard *keyboard,
                             XkbDisplayName display_name,
                             gint group)
{
    XkbGroupData *group_data;

    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), NULL);

    if (group == -1)
        group = xkb_keyboard_get_current_group (keyboard);

    if (G_UNLIKELY (group < 0 || group >= keyboard->group_count))
        return NULL;

    group_data = &keyboard->group_data[group];

    switch (display_name)
    {
        case DISPLAY_NAME_COUNTRY:
            return group_data->country_name;

        case DISPLAY_NAME_LANGUAGE:
            return group_data->language_name;

        default:
            return "";
    }
}

gint
xkb_keyboard_get_variant_index (XkbKeyboard *keyboard,
                                XkbDisplayName display_name,
                                gint group)
{
    XkbGroupData *group_data;

    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), 0);

    if (group == -1)
        group = xkb_keyboard_get_current_group (keyboard);

    if (G_UNLIKELY (group < 0 || group >= keyboard->group_count))
        return 0;

    group_data = &keyboard->group_data[group];

    switch (display_name)
    {
        case DISPLAY_NAME_COUNTRY:
            return group_data->country_index - 1;

        case DISPLAY_NAME_LANGUAGE:
            return group_data->language_index - 1;

        default:
            return 0;
    }
}

static void
xkb_keyboard_xkl_state_changed (XklEngine *engine,
                                XklEngineStateChange change,
                                gint group,
                                gboolean restore,
                                gpointer user_data)
{
    XkbKeyboard *keyboard = user_data;

    if (change == GROUP_CHANGED)
    {
        keyboard->current_group = group;

        switch (keyboard->group_policy)
        {
            case GROUP_POLICY_GLOBAL:
                break;

            case GROUP_POLICY_PER_WINDOW:
                g_hash_table_insert (
                        keyboard->window_map,
                        GINT_TO_POINTER (keyboard->current_window_id),
                        GINT_TO_POINTER (group)
                );
                break;

            case GROUP_POLICY_PER_APPLICATION:
                g_hash_table_insert (
                        keyboard->application_map,
                        GINT_TO_POINTER (keyboard->current_application_id),
                        GINT_TO_POINTER (group)
                );
            break;
        }

        g_signal_emit (G_OBJECT (keyboard),
                xkb_keyboard_signals [STATE_CHANGED],
                0, FALSE);
    }
}

static gboolean
xkb_keyboard_xkl_config_changed_timeout (gpointer user_data)
{
    XkbKeyboard *keyboard = user_data;
    gboolean     updated;

    updated = xkb_keyboard_update_from_xkl (keyboard);

    if (updated)
    {
        xkb_keyboard_set_group (keyboard, 0);

        g_signal_emit (G_OBJECT (keyboard),
                xkb_keyboard_signals [STATE_CHANGED],
                0, TRUE);
    }

    keyboard->config_timeout_id = 0;

    return G_SOURCE_REMOVE;
}

static void
xkb_keyboard_xkl_config_changed (XklEngine *engine,
                                 gpointer user_data)
{
    XkbKeyboard *keyboard = user_data;

    if (keyboard->config_timeout_id != 0)
        g_source_remove (keyboard->config_timeout_id);

    keyboard->config_timeout_id = g_timeout_add (100, xkb_keyboard_xkl_config_changed_timeout, keyboard);
}

static GdkFilterReturn
xkb_keyboard_handle_xevent (GdkXEvent * xev, GdkEvent * event, gpointer user_data)
{
    XkbKeyboard *keyboard = user_data;
    XEvent *xevent = (XEvent *) xev;

    xkl_engine_filter_events (keyboard->engine, xevent);

    return GDK_FILTER_CONTINUE;
}

GdkPixbuf *
xkb_keyboard_get_pixbuf (XkbKeyboard *keyboard,
                         gboolean tooltip,
                         gint group)
{
    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), NULL);

    if (group == -1)
        group = xkb_keyboard_get_current_group (keyboard);

    if (G_UNLIKELY (group < 0 || group >= keyboard->group_count))
        return 0;

    if (tooltip)
        return keyboard->group_data[group].tooltip_pixbuf;
    else
        return keyboard->group_data[group].display_pixbuf;
}

gchar*
xkb_keyboard_get_pretty_layout_name (XkbKeyboard *keyboard,
                                     gint group)
{
    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), NULL);

    if (group == -1)
        group = xkb_keyboard_get_current_group (keyboard);

    if (G_UNLIKELY (group < 0 || group >= keyboard->group_count))
        return 0;

    return keyboard->group_data[group].pretty_layout_name;
}

gint
xkb_keyboard_get_current_group (XkbKeyboard *keyboard)
{
    g_return_val_if_fail (IS_XKB_KEYBOARD (keyboard), 0);
    return keyboard->current_group;
}
