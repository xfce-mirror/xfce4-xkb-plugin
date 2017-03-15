/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-cairo.c
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

#include "xkb-cairo.h"
#include "xkb-util.h"
#include "xfce4-xkb-plugin.h"

#include <librsvg/rsvg.h>

#define XKB_PREFERRED_FONT "Courier New, Courier 10 Pitch, Monospace Bold"

void
xkb_cairo_draw_flag (cairo_t *cr,
                     const gchar *group_name,
                     gint actual_width,
                     gint actual_height,
                     gint variant_markers_count,
                     guint max_variant_markers_count,
                     guint scale,
                     GdkRGBA rgba)
{
    gchar *filename;
    RsvgHandle *handle;
    RsvgDimensionData dim;
    double scalex, scaley;
    gint i;
    double layoutx, layouty, img_width, img_height;
    double radius, diameter;
    guint spacing;

    g_assert (cr != NULL);

    if (!group_name)
        return;

    filename = xkb_util_get_flag_filename (group_name);
    handle = rsvg_handle_new_from_file (filename, NULL);
    g_free (filename);

    if (!handle)
    {
        xkb_cairo_draw_label (cr, group_name,
                actual_width, actual_height,
                variant_markers_count,
                scale,
                rgba);
        return;
    }

    rsvg_handle_get_dimensions (handle, &dim);

    scalex = (double) (actual_width - 4) / dim.width;
    scaley = (double) (actual_height - 4) / dim.height;

    scalex *= scale / 100.0;
    scaley *= scale / 100.0;

    img_width  = dim.width * scalex;
    img_height = dim.height * scaley;

    DBG ("scale x/y: %.3f/%.3f, dim w/h: %d/%d, scaled w/h: %.1f/%.1f",
         scalex, scaley, dim.width, dim.height, scalex*dim.width, scaley*dim.height);

    layoutx = (actual_width - img_width) / 2;
    layouty = (actual_height - img_height) / 2;
    cairo_translate (cr, layoutx, layouty);

    cairo_save (cr);

    cairo_scale (cr, scalex, scaley);
    rsvg_handle_render_cairo (handle, cr);

    cairo_restore (cr);

    DBG ("actual width/height: %d/%d; img w/h: %.1f/%.1f; markers: %d, max markers: %d",
         actual_width, actual_height, img_width, img_height,
         variant_markers_count, max_variant_markers_count);
    DBG ("layout x/y: %.1f/%.1f", layoutx, layouty);

    if (variant_markers_count > 0)
    {
        diameter = 5.0;
        spacing = 1;

        /* check if the flag is too small to draw variant markers inside it */
        if ((diameter + spacing) * (max_variant_markers_count-1) > img_width - 2)
        {
            /* draw markers below the flag */
            diameter = 4;
            spacing  = 0;
            layoutx  = actual_width / 2 + (max_variant_markers_count - 2) * diameter / 2;
            layouty  = (actual_height + img_height) / 2 + diameter + 1;
            DBG ("small flag");
        }
        else
        {
            /* draw markers inside the flag */
            spacing  = 1;
            layoutx += img_width  - diameter / 2 - 1;
            layouty += img_height - diameter / 2 - 1;
            DBG ("large flag");
        }

        radius = diameter / 2.0;

        if (layouty > actual_height - radius)
            layouty = actual_height - radius;
        if (layoutx > actual_width - radius)
            layoutx = actual_width - radius;
    }

    /* draw variant_markers_count circles */
    for (i = 0; i < variant_markers_count; i++)
    {
        gint x, y;

        cairo_set_source_rgb (cr, 0, 0, 0);

        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_width (cr, 1);

        x = layoutx - (diameter + spacing) * i + 0.5;
        y = layouty;

        DBG ("variant center x/y: %d/%d, diameter: %.1f, spacing: %d",
             x, y, diameter, spacing);
        cairo_arc (cr, x, y, radius, 0, 2 * G_PI);

        cairo_set_source_rgb (cr, 0, 0, 0);
        cairo_fill_preserve (cr);
        cairo_set_source_rgb (cr, 1, 1, 1);
        cairo_stroke (cr);
    }

    rsvg_handle_close (handle, NULL);
    g_object_unref (handle);
}

void
xkb_cairo_draw_label (cairo_t *cr,
                      const gchar *group_name,
                      const gint actual_width,
                      const gint actual_height,
                      const gint variant_markers_count,
                      const guint scale,
                      const GdkRGBA rgba)
{
    gchar *normalized_group_name;
    gint pango_width, pango_height;
    double layoutx, layouty, text_width, text_height;
    double scalex, scaley;
    gint i;
    double radius, diameter;

    PangoLayout *layout;
    PangoFontDescription *desc;

    g_assert (cr != NULL);

    DBG ("actual width/height: %d/%d; markers: %d",
         actual_width, actual_height, variant_markers_count);

    layout = pango_cairo_create_layout (cr);
    normalized_group_name = xkb_util_normalize_group_name (group_name, FALSE);

    if (!normalized_group_name ||
        !g_utf8_validate (normalized_group_name, -1, NULL))
    {
        g_object_unref (layout);
        g_free (normalized_group_name);
        return;
    }

    pango_layout_set_text (layout, normalized_group_name, -1);

    desc = pango_font_description_from_string ( XKB_PREFERRED_FONT );
    pango_layout_set_font_description (layout, desc);
    pango_font_description_free (desc);

    gdk_cairo_set_source_rgba (cr, &rgba);
    pango_layout_get_pixel_size (layout, &pango_width, &pango_height);
    DBG ("pango_width/height: %d/%d", pango_width, pango_height);

    scalex = scaley = scale / 100.0;

    DBG ("txt size scale x/y: %.2f/%.2f", scalex, scaley);

    text_height = actual_height * scaley;
    scaley = text_height / pango_height;
    radius = (text_height < 32) ? 1.2 : 2.5;
    diameter = 2 * radius;

    text_width  = actual_width * scalex;
    if (actual_width - text_width < 3 + variant_markers_count * diameter)
    {
        text_width = actual_width - 3 - (variant_markers_count) * diameter;
    }
    else if (scale >= 99.5)
    {
        text_width -= 3;
    }

    scalex =  text_width/pango_width;

    layoutx = (actual_width -
               (text_width + (variant_markers_count ? 3:0) +
                variant_markers_count * diameter)) / 2;
    layouty = (actual_height - text_height) / 2;

    DBG ("text_width/height: %.2f/%.2f", text_width, text_height);
    DBG ("layout x/y: %.2f/%.2f scale x/y: %.2f/%.2f, radius: %.2f",
         layoutx, layouty, scalex, scaley, radius);

    cairo_save (cr);
    cairo_move_to (cr, layoutx, layouty);
    cairo_scale (cr, scalex, scaley);
    pango_cairo_show_layout (cr, layout);
    cairo_restore (cr);

    for (i = 0; i < variant_markers_count; i++)
    {
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_width (cr, 1);
        cairo_arc (cr,
                layoutx + text_width + 3 + (diameter * i),
                layouty + text_height - (text_height / 5),
                radius, 0, 2 * G_PI
        );
        cairo_fill (cr);
    }

    g_free (normalized_group_name);
    g_object_unref (layout);
}

void
xkb_cairo_draw_label_system (cairo_t *cr,
                             const gchar *group_name,
                             const gint actual_width,
                             const gint actual_height,
                             const gint variant_markers_count,
                             const PangoFontDescription *desc,
                             const GdkRGBA rgba)
{
    gchar *normalized_group_name;
    gint pango_width, pango_height;
    double layoutx, layouty;
    gint i;
    double radius, diameter;
    PangoLayout *layout;

    g_assert (cr != NULL);

    DBG ("actual width/height: %d/%d; markers: %d",
         actual_width, actual_height, variant_markers_count);

    layout = pango_cairo_create_layout (cr);
    normalized_group_name = xkb_util_normalize_group_name (group_name, TRUE);

    if (!normalized_group_name ||
        !g_utf8_validate (normalized_group_name, -1, NULL))
    {
        g_object_unref (layout);
        g_free (normalized_group_name);
        return;
    }

    pango_layout_set_text (layout, normalized_group_name, -1);
    pango_layout_set_font_description (layout, desc);

    gdk_cairo_set_source_rgba (cr, &rgba);
    pango_layout_get_pixel_size (layout, &pango_width, &pango_height);
    DBG ("pango_width/height: %d/%d", pango_width, pango_height);

    layoutx = (double) (actual_width - pango_width) / 2;
    layouty = (actual_height - pango_height) / 2;
    radius = pango_height / 10;
    diameter = 2 * radius;

    DBG ("layout x/y: %.2f/%.2f, radius: %.2f", layoutx, layouty, radius);

    cairo_move_to (cr, layoutx, layouty);
    pango_cairo_show_layout (cr, layout);

    for (i = 0; i < variant_markers_count; i++)
    {
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_width (cr, 1);
        cairo_arc (cr,
                layoutx + (pango_width - (2 * variant_markers_count - 2) * diameter) / 2
                    + 2 * diameter * i,
                layouty + pango_height + radius,
                radius, 0, 2 * G_PI
        );
        cairo_fill (cr);
    }

    g_free (normalized_group_name);
    g_object_unref (layout);
}

