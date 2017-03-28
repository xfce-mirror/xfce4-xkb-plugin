/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-settings-dialog.c
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

#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4ui/libxfce4ui.h>

#include "xkb-plugin.h"
#include "xkb-settings-dialog.h"
#include "xkb-util.h"

GtkTreeIter current_iter;
GtkWidget *settings_dialog;
GtkWidget *default_layout_menu;

typedef struct
{
    XfcePanelPlugin *plugin;
    GtkWidget *display_scale_range;
} DialogInstance;

enum combo_enum
{
    DESC = 0,
    NOM,
    COMBO_NUM
};

enum tree_enum
{
    DEFAULT_LAYOUT = 0,
    LAYOUTS,
    VARIANTS,
    TREE_NUM
};

enum enumeration
{
    AVAIL_LAYOUT_TREE_COL_DESCRIPTION = 0,
    AVAIL_LAYOUT_TREE_COL_ID,
    NUM
};


/**************************************************************/

static void
on_settings_close (GtkDialog *dialog,
                   gint response,
                   DialogInstance *instance)
{
    xfce_panel_plugin_unblock_menu (instance->plugin);
    gtk_widget_destroy (GTK_WIDGET (dialog));
    g_free (instance);
}

static void
on_display_type_changed (GtkComboBox *cb,
                         DialogInstance *instance)
{
    gint active = gtk_combo_box_get_active (cb);
    gtk_widget_set_sensitive (instance->display_scale_range,
            active == DISPLAY_TYPE_IMAGE || active == DISPLAY_TYPE_TEXT);
}

void
xkb_plugin_configure_plugin (XfcePanelPlugin *plugin)
{
    GtkWidget *display_type_combo;
    GtkWidget *display_scale_range;
    GtkWidget *display_tooltip_icon_switch;
    GtkWidget *group_policy_combo;
    GtkWidget *vbox, *frame, *bin, *grid, *label;
    XkbXfconf *config;
    DialogInstance *instance;

    xfce_panel_plugin_block_menu (plugin);

    config = xkb_plugin_get_config (XKB_PLUGIN (plugin));

    instance = g_new0 (DialogInstance, 1);
    instance->plugin = plugin;

    settings_dialog = xfce_titled_dialog_new_with_buttons (_("Keyboard Layouts"),
            NULL, 0, "gtk-close", GTK_RESPONSE_OK, NULL);
    gtk_window_set_icon_name (GTK_WINDOW (settings_dialog), "xfce4-settings");

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
    gtk_widget_set_margin_start (vbox, 8);
    gtk_widget_set_margin_end (vbox, 8);
    gtk_widget_set_margin_top (vbox, 8);
    gtk_widget_set_margin_bottom (vbox, 8);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (settings_dialog))), vbox);

    frame = xfce_gtk_frame_box_new (_("Appearance"), &bin);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 2);

    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 18);
    gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
    gtk_widget_set_size_request (grid, -1, -1);
    gtk_container_add (GTK_CONTAINER (bin), grid);

    label = gtk_label_new (_("Show layout as:"));
    gtk_label_set_xalign (GTK_LABEL (label), 0.f);
    gtk_widget_set_hexpand (label, TRUE);
    gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

    display_type_combo = gtk_combo_box_text_new ();
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (display_type_combo), _("image"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (display_type_combo), _("text"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (display_type_combo), _("system"));
    gtk_widget_set_size_request (display_type_combo, 230, -1);
    gtk_grid_attach (GTK_GRID (grid), display_type_combo, 1, 0, 1, 1);

    label = gtk_label_new (_("Widget size:"));
    gtk_label_set_xalign (GTK_LABEL (label), 0.f);
    gtk_widget_set_hexpand (label, TRUE);
    gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);

    display_scale_range = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL,
            DISPLAY_SCALE_MIN, DISPLAY_SCALE_MAX, 1);
    instance->display_scale_range = display_scale_range;
    gtk_scale_set_value_pos (GTK_SCALE (display_scale_range), GTK_POS_RIGHT);
    gtk_widget_set_size_request (display_scale_range, 230, -1);
    gtk_grid_attach (GTK_GRID (grid), display_scale_range, 1, 1, 1, 1);

    label = gtk_label_new (_("Tooltip icon:"));
    gtk_label_set_xalign (GTK_LABEL (label), 0.f);
    gtk_widget_set_hexpand (label, TRUE);
    gtk_grid_attach (GTK_GRID (grid), label, 0, 2, 1, 1);

    display_tooltip_icon_switch = gtk_switch_new ();
    gtk_widget_set_halign (display_tooltip_icon_switch, GTK_ALIGN_END);
    gtk_grid_attach (GTK_GRID (grid), display_tooltip_icon_switch, 1, 2, 1, 1);

    frame = xfce_gtk_frame_box_new (_("Behavior"), &bin);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 2);

    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 18);
    gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
    gtk_widget_set_size_request (grid, -1, -1);
    gtk_container_add (GTK_CONTAINER (bin), grid);

    label = gtk_label_new (_("Manage layout:"));
    gtk_label_set_xalign (GTK_LABEL (label), 0.f);
    gtk_widget_set_hexpand (label, TRUE);
    gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

    group_policy_combo = gtk_combo_box_text_new ();
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (group_policy_combo), _("globally"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (group_policy_combo), _("per window"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (group_policy_combo), _("per application"));
    gtk_widget_set_size_request (group_policy_combo, 230, -1);
    gtk_grid_attach (GTK_GRID (grid), group_policy_combo, 1, 0, 1, 1);

    gtk_widget_show_all (vbox);

    g_signal_connect ((gpointer) settings_dialog, "response",
            G_CALLBACK (on_settings_close), instance);

    /* enable or disable display_scale_range depending on display type */
    g_signal_connect (display_type_combo, "changed",
            G_CALLBACK (on_display_type_changed), instance);
    on_display_type_changed (GTK_COMBO_BOX (display_type_combo), instance);

    g_object_bind_property (G_OBJECT (config), DISPLAY_TYPE,
            G_OBJECT (display_type_combo),
            "active", G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_object_bind_property (G_OBJECT (config), DISPLAY_SCALE,
            G_OBJECT (gtk_range_get_adjustment (GTK_RANGE (display_scale_range))),
            "value", G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_object_bind_property (G_OBJECT (config), DISPLAY_TOOLTIP_ICON,
            G_OBJECT (display_tooltip_icon_switch),
            "active", G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_object_bind_property (G_OBJECT (config), GROUP_POLICY,
            G_OBJECT (group_policy_combo),
            "active", G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    gtk_widget_show (settings_dialog);
}

void
xkb_plugin_show_about (XfcePanelPlugin *plugin)
{
    GtkWidget *about;
    GdkPixbuf *icon;

    const gchar* authors[] = {
        "Alexander Iliev <sasoiliev@mamul.org>",
        "Gauvain Pocentek <gauvainpocentek@gmail.com>",
        "Igor Slepchin <igor.slepchin@gmail.com>",
        NULL
    };

    icon = xfce_panel_pixbuf_from_source ("preferences-desktop-keyboard", NULL, 32);
    about = gtk_about_dialog_new ();
    gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (about),
            _("Keyboard Layouts Plugin"));
    gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (about),
            PACKAGE_VERSION);
    gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (about),
            icon);
    gtk_about_dialog_set_license (GTK_ABOUT_DIALOG (about),
            xfce_get_license_text (XFCE_LICENSE_TEXT_GPL));
    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (about),
            (const gchar**) authors);
    gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (about),
            _("Allows you to configure and use multiple keyboard layouts."));
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (about),
            "http://goodies.xfce.org/");
    gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (about),
            _("Other plugins available here"));
    gtk_dialog_run (GTK_DIALOG (about));
    gtk_widget_destroy (about);
    if (icon)
        g_object_unref (G_OBJECT (icon));
}
