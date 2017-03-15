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

static void xkb_plugin_popup_menu (GtkButton *btn,
                                   gpointer data);

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

gboolean
xkb_plugin_layout_image_draw (GtkWidget *widget,
                                 cairo_t *cr,
                                 t_xkb *xkb)
{
    const gchar *group_name;
    GtkAllocation allocation;
    GtkStyleContext *style_ctx;
    GtkStateFlags state;
    GdkRGBA rgba;
    gint actual_hsize, actual_vsize;

    gtk_widget_get_allocation (GTK_WIDGET (widget), &allocation);
    actual_hsize = allocation.width;
    actual_vsize = allocation.height;

    state = gtk_widget_get_state_flags (GTK_WIDGET (xkb->btn));
    style_ctx = gtk_widget_get_style_context (GTK_WIDGET (xkb->btn));
    gtk_style_context_get_color (style_ctx, state, &rgba);
    group_name = xkb_config_get_group_name (-1);

    DBG ("img_exposed: actual h/v (%d/%d)",
         actual_hsize, actual_vsize);

    if (xkb->display_type == DISPLAY_TYPE_IMAGE)
    {
        xkb_cairo_draw_flag (cr, group_name,
                actual_hsize, actual_vsize,
                xkb_config_variant_index_for_group (-1),
                xkb_config_get_max_group_count (),
                xkb->display_img_scale,
                xkb->display_text_scale,
                rgba
        );
    }
    else
    {
        xkb_cairo_draw_label (cr, group_name,
                actual_hsize, actual_vsize,
                xkb_config_variant_index_for_group (-1),
                xkb->display_text_scale,
                rgba
        );
    }

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

static void
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
    gint       group       = xkb_config_get_current_group ();
    GdkPixbuf *pixbuf      = xkb_config_get_tooltip_pixbuf (group);
    gchar     *layout_name = xkb_config_get_pretty_layout_name (group);

    gtk_tooltip_set_icon (tooltip, pixbuf);
    gtk_tooltip_set_text (tooltip, layout_name);

    return TRUE;
}

