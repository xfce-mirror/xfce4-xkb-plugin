/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-callbacks.c
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

#include "xkb-callbacks.h"
#include "xkb-cairo.h"
#include "xkb-util.h"

void
xkb_plugin_active_window_changed (WnckScreen *screen,
                                  WnckWindow *previously_active_window,
                                  t_xkb *xkb)
{
    WnckWindow *window;
    guint window_id, application_id;

    window = wnck_screen_get_active_window (screen);
    if (!WNCK_IS_WINDOW (window)) return;
    window_id = wnck_window_get_xid (window);
    application_id = wnck_window_get_pid (window);

    xkb_config_window_changed (window_id, application_id);
}

void
xkb_plugin_application_closed (WnckScreen *screen,
                               WnckApplication *app,
                               t_xkb *xkb)
{
    guint application_id;

    application_id = wnck_application_get_pid (app);

    xkb_config_application_closed (application_id);
}

void
xkb_plugin_window_closed (WnckScreen *screen,
                          WnckWindow *window,
                          t_xkb *xkb)
{
    guint window_id;

    window_id = wnck_window_get_xid (window);

    xkb_config_window_closed (window_id);
}

void
xkb_plugin_button_size_allocated (GtkWidget *button,
                                  GtkAllocation *allocation,
                                  t_xkb *xkb)
{
    xkb->button_hsize = allocation->width;
    xkb->button_vsize = allocation->height;

    DBG ("size_allocated: h/v (%p: %d/%d)",
         xkb, xkb->button_hsize, xkb->button_vsize);
}

gboolean
xkb_plugin_button_entered (GtkWidget *widget,
                           GdkEventCrossing *event,
                           t_xkb *xkb)
{
    xkb->button_state = GTK_STATE_PRELIGHT;
    return FALSE;
}

gboolean
xkb_plugin_button_left (GtkWidget *widget,
                        GdkEventCrossing *event,
                        t_xkb *xkb)
{
    xkb->button_state = GTK_STATE_NORMAL;
    return FALSE;
}

gboolean
xkb_plugin_layout_image_exposed (GtkWidget *widget,
                                 GdkEventExpose *event,
                                 t_xkb *xkb)
{
    const gchar *group_name;
    cairo_t *cr;
    GtkStyle *style;
    GdkColor fgcolor;
    gint actual_hsize, actual_vsize, panel_size;
    gint vsize;

    actual_hsize = (xkb->button_hsize > xkb->hsize) ? xkb->button_hsize : xkb->hsize;
    actual_vsize = (xkb->button_vsize > xkb->vsize) ? xkb->button_vsize : xkb->vsize;

    vsize = MIN (xkb->vsize, (int) (xkb->hsize * 0.75));

    panel_size   = xfce_panel_plugin_get_size (xkb->plugin);
    panel_size  /= xfce_panel_plugin_get_nrows (xkb->plugin);

    style = gtk_widget_get_style (GTK_WIDGET (xkb->btn));
    fgcolor = style->fg[xkb->button_state];
    group_name = xkb_config_get_group_name (-1);

    DBG ("img_exposed: actual h/v (%d/%d), xkb h/v (%d/%d), panel sz (%d)",
         actual_hsize, actual_vsize,
         xkb->hsize, xkb->vsize, panel_size);

    cr = gdk_cairo_create ((GTK_WIDGET (xkb->layout_image))->window);

    if (xkb->display_type == DISPLAY_TYPE_IMAGE)
    {
        xkb_cairo_draw_flag (cr, group_name,
                panel_size,
                actual_hsize, actual_vsize,
                xkb->hsize, vsize,
                xkb_config_variant_index_for_group (-1),
                xkb->display_textsize,
                fgcolor
        );
    }
    else
    {
        xkb_cairo_draw_label (cr, group_name,
                panel_size,
                actual_hsize, actual_vsize,
                xkb->hsize, vsize,
                xkb_config_variant_index_for_group (-1),
                xkb->display_textsize,
                fgcolor
        );
    }

    cairo_destroy (cr);

    return FALSE;
}

void
xkb_plugin_button_clicked (GtkButton *btn,
                           gpointer data)
{
    if (xkb_config_get_group_count () > 2)
    {
        xkb_plugin_popup_menu (btn, data);
    }
    else
    {
        xkb_config_next_group ();
    }
}

gboolean
xkb_plugin_button_scrolled (GtkWidget *btn,
                            GdkEventScroll *event,
                            gpointer data)
{
    switch (event->direction)
    {
      case GDK_SCROLL_UP:
      case GDK_SCROLL_RIGHT:
          xkb_config_next_group ();
          return TRUE;
      case GDK_SCROLL_DOWN:
      case GDK_SCROLL_LEFT:
          xkb_config_prev_group ();
          return TRUE;
      default:
        return FALSE;
    }

    return FALSE;
}

void
xkb_plugin_popup_menu (GtkButton *btn,
                       gpointer data)
{
    t_xkb *xkb = (t_xkb *) data;
    gtk_menu_popup (GTK_MENU (xkb->popup),
            NULL, NULL, NULL, NULL, 0,
            gtk_get_current_event_time ());
}

gboolean
xkb_plugin_set_tooltip (GtkWidget *widget,
                        gint x,
                        gint y,
                        gboolean keyboard_mode,
                        GtkTooltip *tooltip,
                        t_xkb *xkb)
{
    RsvgHandle *handle;
    GdkPixbuf  *pixbuf, *tmp;
    gchar      *imgfilename;
    gchar      *text;

    imgfilename = xkb_util_get_flag_filename (xkb_config_get_group_name (-1));
    handle = rsvg_handle_new_from_file (imgfilename, NULL);
    g_free (imgfilename);

    if (handle)
    {
        tmp = rsvg_handle_get_pixbuf (handle);
        pixbuf = gdk_pixbuf_scale_simple (tmp, 24, 24, GDK_INTERP_BILINEAR);
        gtk_tooltip_set_icon (tooltip, pixbuf);

        g_object_unref (pixbuf);
        g_object_unref (tmp);
        g_object_unref (handle);
    }
    else
        gtk_tooltip_set_icon (tooltip, NULL);

    text = xkb_util_get_layout_string (xkb_config_get_group_name (-1),
                                       xkb_config_get_variant (-1));

    gtk_tooltip_set_text (tooltip, text);
    g_free (text);

    return TRUE;
}

