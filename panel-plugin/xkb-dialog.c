/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-dialog.c
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

#include <libxfce4ui/libxfce4ui.h>

#include "xkb-plugin.h"
#include "xkb-properties.h"
#include "xkb-dialog.h"



static gboolean
xkb_dialog_transform_scale_range_for_display_type (GBinding     *binding,
                                                   const GValue *from_value,
                                                   GValue       *to_value,
                                                   gpointer      user_data)
{
  gint display_type = g_value_get_int (from_value);
  g_value_set_boolean (to_value,
                       display_type == DISPLAY_TYPE_IMAGE || display_type == DISPLAY_TYPE_TEXT);
  return TRUE;
}



static gboolean
xkb_dialog_transform_scale_range_for_caps_lock_indicator (GBinding     *binding,
                                                          const GValue *from_value,
                                                          GValue       *to_value,
                                                          gpointer      user_data)
{
  gint display_type = g_value_get_int (from_value);
  g_value_set_boolean (to_value,
                       display_type == DISPLAY_TYPE_SYSTEM);
  return TRUE;
}



static gboolean
xkb_dialog_set_style_warning_tooltip (GtkWidget *widget,
                                      gint        x,
                                      gint        y,
                                      gboolean    keyboard_mode,
                                      GtkTooltip *tooltip)
{
  if (!gtk_widget_get_sensitive (widget))
    {
      gtk_tooltip_set_text (tooltip,
                            _("This option is not available for current layout style"));
      gtk_tooltip_set_icon_from_icon_name (tooltip,
                                           "dialog-warning-symbolic",
                                           GTK_ICON_SIZE_SMALL_TOOLBAR);

      return TRUE;
    }

  return FALSE;
}



static gboolean
xkb_dialog_layoutdefault_tooltip (GtkWidget *widget,
				  gint        x,
				  gint        y,
				  gboolean    keyboard_mode,
				  GtkTooltip *tooltip)
{
  gtk_tooltip_set_text (
    tooltip,
    _("Enter a comma-separated list of window classes which will default to this layout."));
  return TRUE;
}



void
xkb_dialog_configure_plugin (XfcePanelPlugin *plugin,
                             XkbXfconf       *config)
{
  GtkWidget *settings_dialog;
  GtkWidget *display_type_combo;
  GtkWidget *display_name_combo;
  GtkWidget *display_scale_range;
  GtkWidget *caps_lock_indicator_switch;
  GtkWidget *display_tooltip_icon_switch;
  GtkWidget *group_policy_combo;
  GtkWidget *layoutdefault_entry[MAX_LAYOUTS];
  GtkWidget *vbox, *frame, *bin, *grid, *label;
  GString   *labtext;
  gint       grid_vertical;
  guint      i;
  const gchar *prop_names[MAX_LAYOUTS];

  prop_names[1] = LAYOUT1_DEFAULTS;
  prop_names[2] = LAYOUT2_DEFAULTS;
  prop_names[3] = LAYOUT3_DEFAULTS;

  xfce_panel_plugin_block_menu (plugin);

  settings_dialog = xfce_titled_dialog_new_with_buttons (_("Keyboard Layouts"),
                                                         NULL, 0, "gtk-close",
                                                         GTK_RESPONSE_OK, NULL);
  gtk_window_set_icon_name (GTK_WINDOW (settings_dialog), "xfce4-settings");

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (settings_dialog))), vbox);

  grid_vertical = 0;

  frame = xfce_gtk_frame_box_new (_("Appearance"), &bin);
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  gtk_alignment_set_padding (GTK_ALIGNMENT (bin), 6, 0, 12, 0);
  G_GNUC_END_IGNORE_DEPRECATIONS
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
  gtk_widget_set_size_request (grid, -1, -1);
  gtk_container_add (GTK_CONTAINER (bin), grid);

  label = gtk_label_new (_("Show layout as:"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.f);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 0, grid_vertical, 1, 1);

  display_type_combo = gtk_combo_box_text_new ();
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (display_type_combo), _("image"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (display_type_combo), _("text"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (display_type_combo), _("system"));
  gtk_widget_set_size_request (display_type_combo, 230, -1);
  gtk_grid_attach (GTK_GRID (grid), display_type_combo, 1, grid_vertical, 1, 1);

  grid_vertical++;

  label = gtk_label_new (_("Layout name:"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.f);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 0, grid_vertical, 1, 1);

  display_name_combo = gtk_combo_box_text_new ();
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (display_name_combo), _("country"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (display_name_combo), _("language"));
  gtk_widget_set_size_request (display_name_combo, 230, -1);
  gtk_grid_attach (GTK_GRID (grid), display_name_combo, 1, grid_vertical, 1, 1);

  grid_vertical++;

  label = gtk_label_new (_("Widget size:"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.f);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 0, grid_vertical, 1, 1);

  display_scale_range = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL,
                                                  DISPLAY_SCALE_MIN, DISPLAY_SCALE_MAX, 1);
  gtk_scale_set_value_pos (GTK_SCALE (display_scale_range), GTK_POS_RIGHT);
  gtk_widget_set_size_request (display_scale_range, 230, -1);
  gtk_grid_attach (GTK_GRID (grid), display_scale_range, 1, grid_vertical, 1, 1);

  grid_vertical++;

  label = gtk_label_new (_("Caps Lock indicator:"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.f);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 0, grid_vertical, 1, 1);

  caps_lock_indicator_switch = gtk_switch_new ();
  gtk_widget_set_halign (caps_lock_indicator_switch, GTK_ALIGN_END);
  gtk_widget_set_valign (caps_lock_indicator_switch, GTK_ALIGN_CENTER);
  gtk_grid_attach (GTK_GRID (grid), caps_lock_indicator_switch, 1, grid_vertical, 1, 1);

  grid_vertical++;

  label = gtk_label_new (_("Tooltip icon:"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.f);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 0, grid_vertical, 1, 1);

  display_tooltip_icon_switch = gtk_switch_new ();
  gtk_widget_set_halign (display_tooltip_icon_switch, GTK_ALIGN_END);
  gtk_widget_set_valign (display_tooltip_icon_switch, GTK_ALIGN_CENTER);
  gtk_grid_attach (GTK_GRID (grid), display_tooltip_icon_switch, 1, grid_vertical, 1, 1);

  grid_vertical = 0;

  frame = xfce_gtk_frame_box_new (_("Behavior"), &bin);
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  gtk_alignment_set_padding (GTK_ALIGNMENT (bin), 6, 0, 12, 0);
  G_GNUC_END_IGNORE_DEPRECATIONS
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
  gtk_widget_set_size_request (grid, -1, -1);
  gtk_container_add (GTK_CONTAINER (bin), grid);

  label = gtk_label_new (_("Manage layout:"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.f);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 0, grid_vertical, 1, 1);

  group_policy_combo = gtk_combo_box_text_new ();
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (group_policy_combo), _("globally"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (group_policy_combo), _("per window"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (group_policy_combo), _("per application"));
  gtk_widget_set_size_request (group_policy_combo, 230, -1);
  gtk_grid_attach (GTK_GRID (grid), group_policy_combo, 1, grid_vertical, 1, 1);

  grid_vertical++;

  label = gtk_label_new (_("Window classes which default to ..."));
  gtk_label_set_xalign (GTK_LABEL (label), 0.f);
  gtk_grid_attach (GTK_GRID (grid), label, 0, grid_vertical, 2, 1);

  grid_vertical++;
  for (i = 1; i < MAX_LAYOUTS; ++i,++grid_vertical) {
    labtext = g_string_new (_("... layout "));
    g_string_append_printf (labtext, "%d:", i);
    label = gtk_label_new (labtext->str);
    g_string_free (labtext, TRUE);
    gtk_label_set_xalign (GTK_LABEL (label), 0.1f);
    gtk_grid_attach (GTK_GRID (grid), label, 0, grid_vertical, 1, 1);
    layoutdefault_entry[i] = gtk_entry_new();
    gtk_widget_set_hexpand (layoutdefault_entry[i], TRUE);
    gtk_grid_attach (GTK_GRID (grid), layoutdefault_entry[i], 1, grid_vertical, 1, 1);
  }

  gtk_widget_show_all (vbox);

  g_signal_connect_swapped (settings_dialog, "response",
                            G_CALLBACK (xfce_panel_plugin_unblock_menu), plugin);
  g_signal_connect (settings_dialog, "response",
                    G_CALLBACK (gtk_widget_destroy), NULL);

  g_object_bind_property (G_OBJECT (config), DISPLAY_TYPE,
                          G_OBJECT (display_type_combo), "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_bind_property (G_OBJECT (config), DISPLAY_NAME,
                          G_OBJECT (display_name_combo), "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_bind_property (G_OBJECT (config), DISPLAY_SCALE,
                          G_OBJECT (gtk_range_get_adjustment (GTK_RANGE (display_scale_range))), "value",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_bind_property (G_OBJECT (config), CAPS_LOCK_INDICATOR,
                          G_OBJECT (caps_lock_indicator_switch), "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_bind_property (G_OBJECT (config), DISPLAY_TOOLTIP_ICON,
                          G_OBJECT (display_tooltip_icon_switch), "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_bind_property (G_OBJECT (config), GROUP_POLICY,
                          G_OBJECT (group_policy_combo), "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_bind_property_full (G_OBJECT (display_type_combo), "active",
                               G_OBJECT (display_scale_range), "sensitive",
                               G_BINDING_SYNC_CREATE,
                               xkb_dialog_transform_scale_range_for_display_type,
                               NULL, NULL, NULL);

  g_object_bind_property_full (G_OBJECT (display_type_combo), "active",
                               G_OBJECT (caps_lock_indicator_switch), "sensitive",
                               G_BINDING_SYNC_CREATE,
                               xkb_dialog_transform_scale_range_for_caps_lock_indicator,
                               NULL, NULL, NULL);

  for (i = 1; i < MAX_LAYOUTS; ++i) {
    g_object_bind_property (G_OBJECT (config), prop_names[i],
			    G_OBJECT (layoutdefault_entry[i]), "text",
			    G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
    gtk_widget_set_has_tooltip (layoutdefault_entry[i], TRUE);
    g_signal_connect (layoutdefault_entry[i], "query-tooltip",
		      G_CALLBACK (xkb_dialog_layoutdefault_tooltip), NULL);
  }

  gtk_widget_set_has_tooltip (display_scale_range, TRUE);
  g_signal_connect (display_scale_range, "query-tooltip",
                    G_CALLBACK (xkb_dialog_set_style_warning_tooltip), NULL);

  gtk_widget_set_has_tooltip (caps_lock_indicator_switch, TRUE);
  g_signal_connect (caps_lock_indicator_switch, "query-tooltip",
                    G_CALLBACK (xkb_dialog_set_style_warning_tooltip), NULL);

  gtk_widget_show (settings_dialog);
}



void
xkb_dialog_about_show (void)
{
  GdkPixbuf *icon;

  const gchar* authors[] =
    {
      "Alexander Iliev <sasoiliev@mamul.org>",
      "Gauvain Pocentek <gauvainpocentek@gmail.com>",
      "Igor Slepchin <igor.slepchin@gmail.com>",
      NULL
    };

  icon = xfce_panel_pixbuf_from_source ("preferences-desktop-keyboard", NULL, 32);

  gtk_show_about_dialog (NULL,
                         "logo", icon,
                         "program-name", _("Keyboard Layouts Plugin"),
                         "version", PACKAGE_VERSION,
                         "comments", _("Allows you to configure and use multiple keyboard layouts."),
                         "website", "https://docs.xfce.org/panel-plugins/xfce4-xkb-plugin",
                         "license", xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
                         "authors", authors,
                         NULL);

  if (icon)
    g_object_unref (G_OBJECT (icon));
}
