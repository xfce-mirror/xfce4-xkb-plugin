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

static void         xkb_populate_popup_menu             (t_xkb *xkb);

static void         xkb_destroy_popup_menu              (t_xkb *xkb);

static void         xfce_xkb_configure_layout           (GtkWidget *widget,
                                                         gpointer user_data);


/* ================================================================== *
 *                        Implementation                              *
 * ================================================================== */

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (xfce_xkb_construct);

static void
xfce_xkb_construct (XfcePanelPlugin *plugin)
{
    GtkWidget *configure_layouts;
    GtkIconTheme *theme;
    GtkWidget *image;
    GdkPixbuf *pixbuf;

    t_xkb *xkb = xkb_new (plugin);

    xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    xfce_panel_plugin_set_small (plugin, TRUE);

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

    configure_layouts =
        gtk_image_menu_item_new_with_label (_("Keyboard settings"));

    theme = gtk_icon_theme_get_for_screen (gdk_screen_get_default());
    pixbuf = gtk_icon_theme_load_icon (theme, "preferences-desktop-keyboard",
                                       GTK_ICON_SIZE_MENU, 0, NULL);
    if (pixbuf != NULL)
    {
        image = gtk_image_new ();
        gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (configure_layouts),
                                       image);
        g_object_unref (pixbuf);
    }

    gtk_widget_show (configure_layouts);
    xfce_panel_plugin_menu_insert_item (plugin,
                                        GTK_MENU_ITEM (configure_layouts));

    g_signal_connect (G_OBJECT (configure_layouts), "activate",
                      G_CALLBACK (xfce_xkb_configure_layout), NULL);
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

static void
xkb_state_changed (gint current_group, gboolean config_changed,
                   gpointer user_data)
{
    t_xkb *xkb = (t_xkb*) user_data;

    xkb_refresh_gui (xkb);

    if (config_changed)
    {
        xkb_populate_popup_menu (xkb);
    }
}

static void
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
    gchar *filename;
    WnckScreen *wnck_screen;

    xkb = panel_slice_new0 (t_xkb);
    xkb->plugin = plugin;

    filename = xfce_panel_plugin_lookup_rc_file (plugin);
    if ((!filename) || (!xkb_load_config (xkb, filename)))
    {
        xkb_load_default (xkb);
    }
    g_free (filename);

    xkb->btn = gtk_button_new ();
    gtk_button_set_relief (GTK_BUTTON (xkb->btn), GTK_RELIEF_NONE);
    gtk_container_add (GTK_CONTAINER (xkb->plugin), xkb->btn);
    xfce_panel_plugin_add_action_widget (xkb->plugin, xkb->btn);
    xkb->button_state = GTK_STATE_NORMAL;

    gtk_widget_show (xkb->btn);
    g_signal_connect (xkb->btn, "clicked", G_CALLBACK (xkb_plugin_button_clicked), xkb);
    g_signal_connect (xkb->btn, "scroll-event",
                      G_CALLBACK (xkb_plugin_button_scrolled), NULL);

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

    if (xkb_config_initialize (xkb->group_policy, xkb_state_changed, xkb))
    {
        xkb_refresh_gui (xkb);
        xkb_populate_popup_menu (xkb);
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

    gtk_widget_destroy (xkb->layout_image);
    gtk_widget_destroy (xkb->btn);
    xkb_destroy_popup_menu (xkb);

    panel_slice_free (t_xkb, xkb);
}

void
xfce_xkb_save_config (XfcePanelPlugin *plugin, t_xkb *xkb)
{
    gchar* filename;
    XfceRc* rcfile;


    filename = xfce_panel_plugin_save_location (plugin, TRUE);
    if (!filename)
        return;


    rcfile = xfce_rc_simple_open (filename, FALSE);
    if (!rcfile)
    {
        g_free (filename);
        return;
    }

    xfce_rc_set_group (rcfile, NULL);

    xfce_rc_write_int_entry (rcfile, "display_type", xkb->display_type);
    xfce_rc_write_int_entry (rcfile, "display_textsize", xkb->display_textsize);
    xfce_rc_write_int_entry (rcfile, "group_policy", xkb->group_policy);

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
        xkb->display_textsize = xfce_rc_read_int_entry (rcfile, "display_textsize", DISPLAY_TEXTSIZE_LARGE);
        xkb->group_policy = xfce_rc_read_int_entry (rcfile, "group_policy", GROUP_POLICY_PER_APPLICATION);

        xfce_rc_close (rcfile);

        return TRUE;
    }

    return FALSE;
}

static void
xkb_load_default (t_xkb *xkb)
{
    xkb->display_type = DISPLAY_TYPE_IMAGE;
    xkb->display_textsize = DISPLAY_TEXTSIZE_LARGE;
    xkb->group_policy = GROUP_POLICY_PER_APPLICATION;
}

static gboolean
xkb_calculate_sizes (t_xkb *xkb, GtkOrientation orientation, gint panel_size)
{
    guint nrows;

    nrows       = xfce_panel_plugin_get_nrows (xkb->plugin);
    panel_size /= nrows;
    TRACE ("calculate_sizes(%p: %d,%d)", xkb, panel_size, nrows);

    switch (orientation)
    {
        case GTK_ORIENTATION_HORIZONTAL:
            xkb->vsize = panel_size;
            if (nrows > 1)
            {
                xkb->hsize = xkb->vsize;
            }
            else
            {
                xkb->hsize = (int) (1.33 * panel_size);
            }

            gtk_widget_set_size_request (xkb->btn, xkb->hsize, xkb->vsize);
            break;
        case GTK_ORIENTATION_VERTICAL:
            xkb->hsize = panel_size;
            if (nrows > 1)
            {
                xkb->vsize = xkb->hsize;
            }
            else
            {
                xkb->vsize = (int) (0.75 * panel_size);
            }
            if (xkb->vsize < 10) xkb->vsize = 10;
            gtk_widget_set_size_request (xkb->btn, xkb->hsize, xkb->vsize);
            break;
        default:
            break;
    }

    DBG ("size requested: h/v (%p: %d/%d)", xkb, xkb->hsize, xkb->vsize);

    xkb_refresh_gui (xkb);
    return TRUE;
}

static void
xkb_destroy_popup_menu (t_xkb *xkb)
{
    if (xkb->popup)
    {
        gtk_widget_destroy (xkb->popup);
        g_object_ref_sink (xkb->popup);
        g_object_unref (xkb->popup);
        xkb->popup = NULL;
    }
}

static void
xkb_populate_popup_menu (t_xkb *xkb)
{
    gint i, group_count;
    RsvgHandle *handle;
    GdkPixbuf *pixbuf, *tmp;
    gchar *imgfilename;
    GtkWidget *image;
    GtkWidget *menu_item;

    if (G_UNLIKELY (xkb == NULL)) return;

    xkb_destroy_popup_menu (xkb);
    xkb->popup = gtk_menu_new ();

    group_count = xkb_config_get_group_count ();
    for (i = 0; i < group_count; i++)
    {
        gchar *layout_string;

        imgfilename = xkb_util_get_flag_filename (xkb_config_get_group_name (i));
        handle = rsvg_handle_new_from_file (imgfilename, NULL);
        g_free (imgfilename);

        if (handle)
        {
            tmp = rsvg_handle_get_pixbuf (handle);
        }

        layout_string = xkb_config_get_pretty_layout_name (i);

        menu_item = gtk_image_menu_item_new_with_label (layout_string);

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

            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

            rsvg_handle_close (handle, NULL);
            g_object_unref (handle);
        }

        gtk_widget_show (menu_item);

        gtk_menu_shell_append (GTK_MENU_SHELL (xkb->popup), menu_item);
    }
}

void
xkb_refresh_gui (t_xkb *xkb)
{
    GdkDisplay * display;

    /* Part of the image may remain visible after display type change */
    gtk_widget_queue_draw_area (xkb->btn, 0, 0,
            xkb->button_hsize, xkb->button_vsize);

    display = gdk_display_get_default();
    if (display)
    {
        gtk_tooltip_trigger_tooltip_query(display);
    }
}

static void
xfce_xkb_configure_layout (GtkWidget *widget,
                           gpointer user_data)
{
    gchar *desktop_file = xfce_resource_lookup (XFCE_RESOURCE_DATA,
                                 "applications/xfce-keyboard-settings.desktop");
    gchar command[1024];
    snprintf (command, sizeof (command), "exo-open %s", desktop_file);
    command[sizeof (command) - 1] = '\0';
    xfce_spawn_command_line_on_screen (gdk_screen_get_default (),
                                       command, False, True, NULL);
    g_free (desktop_file);
}
