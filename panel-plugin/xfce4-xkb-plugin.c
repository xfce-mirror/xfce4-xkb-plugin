/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xfce4-xkb-plugin.c
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

#include <stdio.h>
#include <ctype.h>

#include <libwnck/libwnck.h>

#include <librsvg/rsvg.h>

#include "xfce4-xkb-plugin.h"
#include "xfce4-xkb-plugin-private.h"
#include "xkb-settings-dialog.h"
#include "xkb-util.h"
#include "xkb-cairo.h"
#include "xkb-callbacks.h"

/* ------------------------------------------------------------------ *
 *                     Panel Plugin Interface                         *
 * ------------------------------------------------------------------ */

static void         xfce_xkb_construct                  (XfcePanelPlugin *plugin);

static void         xfce_xkb_orientation_changed        (XfcePanelPlugin *plugin,
                                                        GtkOrientation orientation,
                                                        t_xkb *xkb);

static gboolean     xfce_xkb_set_size                   (XfcePanelPlugin *plugin,
                                                         gint size,
                                                         t_xkb *xkb);

static void         xfce_xkb_free_data                  (XfcePanelPlugin *plugin,
                                                         t_xkb *xkb);

/* ----------------------------------------------------------------- *
 *                           XKB Stuff                               *
 * ----------------------------------------------------------------- */

static t_xkb *      xkb_new                             (XfcePanelPlugin *plugin);

static void         xkb_free                            (t_xkb *xkb);

static gboolean     xkb_calculate_sizes                 (t_xkb *xkb,
                                                         GtkOrientation orientation,
                                                         gint panel_size);

static gboolean     xkb_load_config                     (t_xkb *xkb,
                                                         const gchar *filename);

static void         xkb_load_default                    (t_xkb *xkb);

static void         xkb_initialize_menu                 (t_xkb *xkb);


/* ================================================================== *
 *                        Implementation                              *
 * ================================================================== */

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (xfce_xkb_construct);

static void
xfce_xkb_construct (XfcePanelPlugin *plugin)
{
    t_xkb *xkb = xkb_new (plugin);
    xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    g_signal_connect (plugin, "orientation-changed",
            G_CALLBACK (xfce_xkb_orientation_changed), xkb);

    g_signal_connect (plugin, "size-changed",
            G_CALLBACK (xfce_xkb_set_size), xkb);

    g_signal_connect (plugin, "free-data",
            G_CALLBACK (xfce_xkb_free_data), xkb);

    g_signal_connect (plugin, "save",
            G_CALLBACK (xfce_xkb_save_config), xkb);

    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (plugin, "configure-plugin",
            G_CALLBACK (xfce_xkb_configure), xkb);

    xfce_panel_plugin_menu_show_about (plugin);
    g_signal_connect (plugin, "about",
            G_CALLBACK (xfce_xkb_about), xkb);

}

static void
xfce_xkb_orientation_changed (XfcePanelPlugin *plugin,
                              GtkOrientation orientation,
                              t_xkb *xkb)
{
    xkb_calculate_sizes (xkb, orientation, xfce_panel_plugin_get_size (plugin));
}

static gboolean
xfce_xkb_set_size (XfcePanelPlugin *plugin, gint size,
                   t_xkb *xkb)
{
    return xkb_calculate_sizes (xkb, xfce_panel_plugin_get_orientation (plugin), size);
}

static void
xfce_xkb_free_data (XfcePanelPlugin *plugin, t_xkb *xkb)
{
    xkb_free (xkb);
}

/* ----------------- xkb plugin stuff -----------------------*/

void
xkb_state_changed (gint current_group, gboolean config_changed, 
                   gpointer user_data)
{
    t_xkb *xkb = (t_xkb*) user_data;

    xkb_refresh_gui (xkb);

    if (config_changed)
    {
        xkb_initialize_menu (xkb);
    }
}

void
xkb_plugin_set_group (GtkMenuItem *item,
              gpointer data)
{
    gint group = GPOINTER_TO_INT (data);
    xkb_config_set_group (group);
}

static t_xkb *
xkb_new (XfcePanelPlugin *plugin)
{
    t_xkb *xkb;
    gint i;
    gchar *filename;
    WnckScreen *wnck_screen;
    GtkWidget *menu_item;

    xkb = panel_slice_new0 (t_xkb);
    xkb->settings = g_new0 (t_xkb_settings, 1);
    xkb->plugin = plugin;

    filename = xfce_panel_plugin_save_location (plugin, TRUE);
    if ((!filename) || (!xkb_load_config (xkb, filename)))
    {
        xkb_load_default (xkb);
    }

    xkb->btn = gtk_button_new ();
    gtk_button_set_relief (GTK_BUTTON (xkb->btn), GTK_RELIEF_NONE);
    gtk_container_add (GTK_CONTAINER (xkb->plugin), xkb->btn);
    xfce_panel_plugin_add_action_widget (xkb->plugin, xkb->btn);

    gtk_widget_show (xkb->btn);
    g_signal_connect (xkb->btn, "clicked", G_CALLBACK (xkb_plugin_button_clicked), xkb);
    g_object_set (G_OBJECT (xkb->btn), "has-tooltip", TRUE, NULL);
    g_signal_connect (xkb->btn, "query-tooltip",
            G_CALLBACK (xkb_plugin_set_tooltip), xkb);

    g_signal_connect (G_OBJECT (xkb->btn), "enter-notify-event",
            G_CALLBACK (xkb_plugin_button_entered), xkb);
    g_signal_connect (G_OBJECT (xkb->btn), "leave-notify-event",
            G_CALLBACK (xkb_plugin_button_left), xkb);
    g_signal_connect (G_OBJECT (xkb->btn), "size-allocate",
            G_CALLBACK (xkb_plugin_button_size_allocated), xkb);

    xkb->layout_image = gtk_image_new ();
    gtk_container_add (GTK_CONTAINER (xkb->btn), xkb->layout_image);
    g_signal_connect (G_OBJECT (xkb->layout_image), "expose-event",
            G_CALLBACK (xkb_plugin_layout_image_exposed), xkb);
    gtk_widget_show (GTK_WIDGET (xkb->layout_image));

    if (xkb_config_initialize (xkb->settings, xkb_state_changed, xkb))
    {
        xkb_refresh_gui (xkb);

        xkb_initialize_menu (xkb);
    }

    wnck_screen = wnck_screen_get_default ();
    g_signal_connect (G_OBJECT (wnck_screen), "active-window-changed",
            G_CALLBACK (xkb_plugin_active_window_changed), xkb);
    g_signal_connect (G_OBJECT (wnck_screen), "window-closed",
            G_CALLBACK (xkb_plugin_window_closed), xkb);
    g_signal_connect (G_OBJECT (wnck_screen), "application-closed",
            G_CALLBACK (xkb_plugin_application_closed), xkb);

    return xkb;
}

static void
xkb_free (t_xkb *xkb)
{
    xkb_config_finalize ();

    if (xkb->settings->kbd_config)
        g_free (xkb->settings->kbd_config);

    g_free (xkb->settings);

    gtk_widget_destroy (xkb->layout_image);
    gtk_widget_destroy (xkb->btn);
    gtk_widget_destroy (xkb->popup);

    panel_slice_free (t_xkb, xkb);
}

void
xfce_xkb_save_config (XfcePanelPlugin *plugin, t_xkb *xkb)
{
    gchar* filename;
    XfceRc* rcfile;

    xkb_config_update_settings (xkb->settings);
    xkb_initialize_menu (xkb);

    filename = xfce_panel_plugin_save_location (plugin, TRUE);
    if (!filename)
    {
        return;
    }

    rcfile = xfce_rc_simple_open (filename, FALSE);
    if (!rcfile)
    {
        return;
    }

    xfce_rc_set_group (rcfile, NULL);

    xfce_rc_write_int_entry (rcfile, "display_type", xkb->display_type);
    xfce_rc_write_int_entry (rcfile, "group_policy", xkb->settings->group_policy);
    xfce_rc_write_int_entry (rcfile, "default_group", xkb->settings->default_group);
    xfce_rc_write_bool_entry (rcfile, "never_modify_config", xkb->settings->never_modify_config);

    if (xkb->settings->kbd_config != NULL)
    {
        xfce_rc_write_entry (rcfile, "model", xkb->settings->kbd_config->model);
        xfce_rc_write_entry (rcfile, "layouts", xkb->settings->kbd_config->layouts);
        xfce_rc_write_entry (rcfile, "variants", xkb->settings->kbd_config->variants);

        if (xkb->settings->kbd_config->toggle_option == NULL)
            xfce_rc_write_entry (rcfile, "toggle_option", "");
        else xfce_rc_write_entry (rcfile, "toggle_option", xkb->settings->kbd_config->toggle_option);

        if (xkb->settings->kbd_config->compose_key_position == NULL)
            xfce_rc_write_entry (rcfile, "compose_key_position", "");
        else xfce_rc_write_entry (rcfile, "compose_key_position", xkb->settings->kbd_config->compose_key_position);
    }

    xfce_rc_close (rcfile);
    g_free (filename);
}

static gboolean
xkb_load_config (t_xkb *xkb, const gchar *filename)
{
    XfceRc* rcfile;
    if ((rcfile = xfce_rc_simple_open (filename, TRUE)))
    {
        xfce_rc_set_group (rcfile, NULL);

        xkb->display_type = xfce_rc_read_int_entry (rcfile, "display_type", DISPLAY_TYPE_IMAGE);
        xkb->settings->group_policy = xfce_rc_read_int_entry (rcfile, "group_policy", GROUP_POLICY_PER_APPLICATION);

        if (xkb->settings->group_policy != GROUP_POLICY_GLOBAL)
        {
            xkb->settings->default_group = xfce_rc_read_int_entry (rcfile, "default_group", 0);
        }

        xkb->settings->never_modify_config = xfce_rc_read_bool_entry (rcfile, "never_modify_config", FALSE);

        if (xkb->settings->kbd_config == NULL)
        {
            xkb->settings->kbd_config = g_new0 (t_xkb_kbd_config, 1);
        }
        xkb->settings->kbd_config->model = g_strdup (xfce_rc_read_entry (rcfile, "model", NULL));
        xkb->settings->kbd_config->layouts = g_strdup (xfce_rc_read_entry (rcfile, "layouts", NULL));
        xkb->settings->kbd_config->variants = g_strdup (xfce_rc_read_entry (rcfile, "variants", NULL));
        xkb->settings->kbd_config->toggle_option = g_strdup (xfce_rc_read_entry (rcfile, "toggle_option", NULL));
        xkb->settings->kbd_config->compose_key_position = g_strdup (xfce_rc_read_entry (rcfile, "compose_key_position", NULL));

        xfce_rc_close (rcfile);

        return TRUE;
    }

    return FALSE;
}

static void
xkb_load_default (t_xkb *xkb)
{
    xkb->display_type = DISPLAY_TYPE_IMAGE;
    xkb->settings->group_policy = GROUP_POLICY_PER_APPLICATION;
    xkb->settings->default_group = 0;
    xkb->settings->kbd_config = NULL;
}

static gboolean
xkb_calculate_sizes (t_xkb *xkb, GtkOrientation orientation, gint panel_size)
{

    switch (orientation)
    {
        case GTK_ORIENTATION_HORIZONTAL:
            xkb->vsize = panel_size;
            xkb->hsize = (int) (1.33 * panel_size);
            gtk_widget_set_size_request (xkb->btn, xkb->hsize, xkb->vsize);
            break;
        case GTK_ORIENTATION_VERTICAL:
            xkb->hsize = panel_size;
            xkb->vsize = (int) (0.75 * panel_size);
            if (xkb->vsize < 10) xkb->vsize = 10;
            gtk_widget_set_size_request (xkb->btn, xkb->hsize, xkb->vsize);
            break;
        default:
            break;
    }

    xkb_refresh_gui (xkb);
    return TRUE;
}

static void
xkb_initialize_menu (t_xkb *xkb)
{
    if (G_UNLIKELY (xkb == NULL)) return;

    gint i;
    RsvgHandle *handle;
    GdkPixbuf *pixbuf, *tmp;
    gchar *imgfilename;
    GtkWidget *image;
    GtkWidget *menu_item;

    xkb->popup = gtk_menu_new ();
    for (i = 0; i < xkb_config_get_group_count (); i++)
    {
        imgfilename = xkb_util_get_flag_filename (xkb_config_get_group_name (i));

        handle = rsvg_handle_new_from_file (imgfilename, NULL);
        if (handle)
        {
            tmp = rsvg_handle_get_pixbuf (handle);
        }

        menu_item = gtk_image_menu_item_new_with_label (
                xkb_util_get_layout_string (
                    xkb_config_get_group_name (i),
                    xkb_config_get_variant (i)
                    )
                );
        g_signal_connect (G_OBJECT (menu_item), "activate",
                G_CALLBACK (xkb_plugin_set_group), GINT_TO_POINTER (i));

        if (handle)
        {
            image = gtk_image_new ();
            pixbuf = gdk_pixbuf_scale_simple (tmp, 15, 10, GDK_INTERP_BILINEAR);
            gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
            gtk_widget_show (image);
            g_object_unref (G_OBJECT (tmp));
            g_object_unref (G_OBJECT (pixbuf));

            gtk_image_menu_item_set_image (menu_item, image);
        }

        gtk_widget_show (menu_item);

        gtk_menu_shell_append (GTK_MENU_SHELL (xkb->popup), menu_item);
    }

}

void
xkb_refresh_gui (t_xkb *xkb)
{
    /* Part of the image may remain visible after display type change */
    gtk_widget_queue_draw_area (xkb->btn, 0, 0,
            xkb->button_hsize, xkb->button_vsize);
}

