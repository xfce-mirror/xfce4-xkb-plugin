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
#include <libxfcegui4/libxfcegui4.h>

#include "xfce4-xkb-plugin.h"
#include "xfce4-xkb-plugin-private.h"
#include "xkb-settings-dialog.h"
#include "xkb-util.h"

GtkWidget *settings_dialog;
GtkWidget *default_layout_menu;

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

gchar       *xkb_settings_layout_dialog_run     ();
void         xkb_settings_update_from_ui        (t_xkb *xkb);

/**************************************************************/

void
on_settings_close (GtkDialog *dialog, gint response, t_xkb *xkb)
{
    xfce_panel_plugin_unblock_menu (xkb->plugin);

    xkb_settings_update_from_ui (xkb);

    xfce_xkb_save_config (xkb->plugin, xkb);

    gtk_widget_destroy (GTK_WIDGET (dialog));
}

void 
on_display_type_changed (GtkComboBox *cb, t_xkb *xkb) 
{
    xkb->display_type = gtk_combo_box_get_active (cb);
    xkb_refresh_gui (xkb);
}

void 
on_group_policy_changed (GtkComboBox *cb, t_xkb *xkb) 
{
    xkb->settings->group_policy = gtk_combo_box_get_active (cb);
}

/* from the gnome control center keyboard applet */
char *
xci_desc_to_utf8 (XklConfigItem * ci)
{
    char *sd = g_strstrip (ci->description);
    return sd[0] == 0 ? g_strdup (ci->name) : g_strdup(sd);
}
/**/

void
xkb_settings_fill_layout_tree_model_with_config (t_xkb *xkb)
{
    gint layout_nb = 0;

    t_xkb_kbd_config *config = xkb->settings->kbd_config;   

    char **layouts = g_strsplit_set (config->layouts, ",", 0);
    char **variants = g_strsplit_set (config->variants, ",", 0);

    while (layouts[layout_nb] != NULL)
    {
        gtk_list_store_append (xkb->layout_store, &iter);
        gtk_list_store_set (xkb->layout_store, &iter,
                            DEFAULT_LAYOUT, (layout_nb == xkb->settings->default_group),
                            LAYOUTS, layouts[layout_nb],
                            VARIANTS, variants[layout_nb],
                            -1);
        layout_nb += 1;
    }
}

static void
xkb_settings_add_toggle_options_to_list (XklConfigRegistry * config_registry,
                                         XklConfigItem * config_item,
                                         t_xkb *xkb)
{
    char *utf_option_name = xci_desc_to_utf8 (config_item);
    gtk_list_store_append (xkb->toggle_options_store, &iter);
    gtk_list_store_set (xkb->toggle_options_store, &iter, 
                      DESC, utf_option_name,
                      NOM, config_item->name, -1);
    g_free (utf_option_name);
}

static void
xkb_settings_add_combo_key_position_options_to_list (XklConfigRegistry * config_registry,
                                                     XklConfigItem * config_item,
                                                     t_xkb *xkb)
{
    char *utf_option_name = xci_desc_to_utf8 (config_item);
    gtk_list_store_append (xkb->compose_key_options_store, &iter);
    gtk_list_store_set (xkb->compose_key_options_store, &iter,
                        DESC, utf_option_name,
                        NOM, config_item->name, -1);
    g_free (utf_option_name);
}

static void
xkb_settings_add_kbd_model_to_list (XklConfigRegistry * config_registry,
                                    XklConfigItem * config_item,
                                    t_xkb *xkb)
{
    char *utf_model_name = xci_desc_to_utf8 (config_item);
    gtk_list_store_append (xkb->combo_store, &iter);
    gtk_list_store_set (xkb->combo_store, &iter, 
                      DESC, utf_model_name,
                      NOM, config_item->name, -1);
    g_free (utf_model_name);
}

void
xkb_settings_set_toggle_option_combo_default_value (t_xkb *xkb)
{
    gchar *id;

    t_xkb_kbd_config *config = xkb->settings->kbd_config;

    /* dirty insurance hack */
    if (config->toggle_option == NULL)
        return;

    model = GTK_TREE_MODEL (xkb->toggle_options_store);
    gtk_tree_model_get_iter_first (model, &iter);
    gtk_tree_model_get (model, &iter, NOM, &id, -1);
    if (strcmp (id, config->toggle_option) == 0 )
    {
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (xkb->toggle_options_combo), &iter);
    }
    else
    {
        while (gtk_tree_model_iter_next(model, &iter))
        {
            gtk_tree_model_get (model, &iter, NOM, &id, -1);

            if (strcmp (id, config->toggle_option) == 0)
            {
                gtk_combo_box_set_active_iter (GTK_COMBO_BOX (xkb->toggle_options_combo), &iter);
                break;
            }
        }
    }
    
    g_free (id);
}

void
xkb_settings_set_compose_key_position_combo_default_value (t_xkb *xkb)
{
    gchar *id;

    t_xkb_kbd_config *config = xkb->settings->kbd_config;

    if (config->compose_key_position == NULL)
        return;

    model = GTK_TREE_MODEL (xkb->compose_key_options_store);
    gtk_tree_model_get_iter_first (model, &iter);
    gtk_tree_model_get (model, &iter, NOM, &id, -1);
    if (strcmp (id, config->compose_key_position) == 0)
    {
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (xkb->compose_key_options_combo), &iter);
    }
    else
    {
        while (gtk_tree_model_iter_next (model, &iter))
        {
            gtk_tree_model_get (model, &iter, NOM, &id, -1);

            if (strcmp (id, config->compose_key_position) == 0)
            {
                gtk_combo_box_set_active_iter (GTK_COMBO_BOX (xkb->compose_key_options_combo), &iter);
                break;
            }
        }
    }

    g_free (id);
}

void
xkb_settings_set_kbd_combo_default_value (t_xkb *xkb)
{
    gchar *id;
    t_xkb_kbd_config *config = xkb->settings->kbd_config;

    model = GTK_TREE_MODEL (xkb->combo_store);
    gtk_tree_model_get_iter_first (model, &iter);
    gtk_tree_model_get (model, &iter, NOM, &id, -1);
    if (strcmp (id, config->model) == 0 )
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (xkb->kbd_model_combo), &iter);
    else
    {
        while (gtk_tree_model_iter_next(model, &iter))
        {
            gtk_tree_model_get (model, &iter, NOM, &id, -1);

            if (strcmp (id, config->model) == 0)
            {
                gtk_combo_box_set_active_iter (GTK_COMBO_BOX (xkb->kbd_model_combo), &iter);
                break;
            }
        }
    }
    
    g_free (id);
}

static gint
xkb_settings_get_group_count (t_xkb *xkb)
{
    gint count = 1;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (xkb->layout_tree_view));
    gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter); 
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter))
        count++;
    return count;
}

static void
xkb_settings_show_hide_layout_buttons (t_xkb *xkb)
{
    gint nb = xkb_settings_get_group_count (xkb);
    gint max_nb = xkb_config_get_max_layout_number ();
    gtk_widget_set_sensitive (xkb->add_layout_btn, 
            (nb < max_nb && !xkb->settings->never_modify_config));
    gtk_widget_set_sensitive (xkb->rm_layout_btn, 
            (nb > 1 && !xkb->settings->never_modify_config));
}

static void
xkb_settings_edit_layout_btn_show (GtkTreeView *tree_view,
                                   t_xkb *xkb)
{
    GtkTreePath *p;
    GtkTreeViewColumn *c;
    gtk_tree_view_get_cursor (GTK_TREE_VIEW (tree_view), &p, &c);
    gtk_widget_set_sensitive (xkb->edit_layout_btn, 
            (p != NULL && !xkb->settings->never_modify_config));
}

static void
xkb_settings_edit_layout (GtkWidget *widget, t_xkb *xkb)
{
    gchar *c;
    gboolean is_default;

    c = xkb_settings_layout_dialog_run ();
    if (c != NULL)
    {
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (xkb->layout_tree_view));
        gchar **strings;
        strings = g_strsplit_set(c, ",", 0);

        gtk_tree_selection_get_selected (selection, &model, &iter);
        gtk_tree_model_get (model, &iter, DEFAULT_LAYOUT, &is_default, -1);
        gtk_list_store_set (xkb->layout_store, &iter,
                            DEFAULT_LAYOUT, is_default,
                            LAYOUTS, strings[0],
                            VARIANTS, strings[1],
                            -1);
        xkb_settings_update_from_ui (xkb);
        g_free(strings);
    }
    g_free(c);
    xkb_settings_edit_layout_btn_show (GTK_TREE_VIEW (xkb->layout_tree_view), xkb);
    
}

static void
xkb_settings_add_layout (GtkWidget *widget, t_xkb *xkb)
{
    gchar *c;
    c = xkb_settings_layout_dialog_run();
    if (c != NULL)
    {
        gchar **strings;
        strings = g_strsplit_set(c, ",", 0);
        gtk_list_store_append (xkb->layout_store, &iter);
        gtk_list_store_set (xkb->layout_store, &iter,
                        DEFAULT_LAYOUT, FALSE,
                        LAYOUTS, strings[0],
                        VARIANTS, strings[1],
                        -1);
        xkb_settings_show_hide_layout_buttons (xkb);
        xkb_settings_update_from_ui (xkb);
        g_free (strings);
    }
    g_free (c);
    xkb_settings_edit_layout_btn_show (GTK_TREE_VIEW (xkb->layout_tree_view), xkb);
}
    
static void
xkb_settings_rm_layout (GtkWidget *widget, t_xkb *xkb)
{
    gboolean is_default;

    selection = gtk_tree_view_get_selection (
                    GTK_TREE_VIEW (xkb->layout_tree_view));
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, DEFAULT_LAYOUT, &is_default, -1);
        gtk_list_store_remove (xkb->layout_store, &iter);
        if (is_default)
        {
            if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
            {
                gtk_list_store_set (GTK_LIST_STORE (model), &iter, DEFAULT_LAYOUT, TRUE);
            }
        }
        gtk_widget_set_sensitive (xkb->add_layout_btn, TRUE);
        xkb_settings_show_hide_layout_buttons (xkb);
    }
    xkb_settings_edit_layout_btn_show (GTK_TREE_VIEW (xkb->layout_tree_view), xkb);
    xkb_settings_update_from_ui (xkb);
}

static void
xkb_settings_default_layout_toggled (GtkCellRendererToggle *renderer,
                                     gchar *path,
                                     t_xkb *xkb)
{
    /* warning, super dumb code - set all layout toggle values to
       false, then set the toggled one to true */

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (xkb->layout_tree_view));
    gtk_tree_model_get_iter_first (model, &iter);
    do
    {
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, DEFAULT_LAYOUT, FALSE, -1);
    } while (gtk_tree_model_iter_next (model, &iter));


    if (gtk_tree_model_get_iter_from_string (model, &iter, path))
    {
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, DEFAULT_LAYOUT, TRUE, -1);
    }
    xkb_settings_update_from_ui (xkb);
}

gboolean
xkb_settings_config_modification_disabled_tooltip (GtkWidget *widget,
                                                   gint x, gint y,
                                                   gboolean keyboard_mode,
                                                   GtkTooltip *tooltip,
                                                   t_xkb *xkb)
{
    if (xkb->settings->never_modify_config)
    {
        gtk_tooltip_set_text (GTK_TOOLTIP (tooltip),
            _("XKB configuration modifications are\n"
            "disabled from the config file.\n\n"
            "See the README file for more information."));

        return TRUE;
    }

    return FALSE;
}

void
xfce_xkb_configure (XfcePanelPlugin *plugin,
                    t_xkb *xkb)
{
    GtkWidget *display_type_optmenu, *group_policy_combo;
    GtkWidget *vbox, *display_type_frame, *group_policy_frame;
          

    GtkCellRenderer *renderer, *renderer2;
    GtkWidget *vbox1, *vbox2, *hbox, *frame;
    XklConfigRegistry *registry;

    xfce_panel_plugin_block_menu (plugin); 

    registry = xkb_config_get_xkl_registry ();

    settings_dialog = xfce_titled_dialog_new_with_buttons (_("Keyboard Layouts"),
            NULL, GTK_DIALOG_NO_SEPARATOR,
            GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);
    gtk_window_set_icon_name (GTK_WINDOW (settings_dialog), "xfce4-settings");

    vbox = gtk_vbox_new (FALSE, 2);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (settings_dialog)->vbox), vbox);

    renderer = gtk_cell_renderer_text_new ();
    renderer2 = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE (renderer2), TRUE);
    g_object_set (G_OBJECT (renderer2), "activatable", TRUE, NULL);

    xkb->combo_store = gtk_list_store_new (COMBO_NUM, G_TYPE_STRING, G_TYPE_STRING);
    xkb->toggle_options_store = gtk_list_store_new (COMBO_NUM, G_TYPE_STRING, G_TYPE_STRING);
    xkb->compose_key_options_store = gtk_list_store_new (COMBO_NUM, G_TYPE_STRING, G_TYPE_STRING);
    
    vbox1 = gtk_vbox_new (FALSE, 5);
    gtk_container_add (GTK_CONTAINER (vbox), vbox1);
    gtk_widget_show (vbox1);

    frame = xfce_framebox_new (_("Keyboard model:"), TRUE);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

    xkb->kbd_model_combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (xkb->combo_store));
    gtk_widget_set_size_request (xkb->kbd_model_combo, 230, -1);
    xfce_framebox_add (XFCE_FRAMEBOX (frame), xkb->kbd_model_combo);
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (xkb->kbd_model_combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (xkb->kbd_model_combo), renderer, "text", 0);
    
    xkl_config_registry_foreach_model (registry,
                       (ConfigItemProcessFunc) xkb_settings_add_kbd_model_to_list,
                           xkb);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (xkb->combo_store),
                                        0, GTK_SORT_ASCENDING);

    xkb_settings_set_kbd_combo_default_value (xkb);
    gtk_widget_show (xkb->kbd_model_combo);

    gtk_widget_set_sensitive (GTK_WIDGET (xkb->kbd_model_combo), !xkb->settings->never_modify_config);
    g_object_set (G_OBJECT (xkb->kbd_model_combo), "has-tooltip", TRUE, NULL);
    g_signal_connect (xkb->kbd_model_combo, "query-tooltip", G_CALLBACK (xkb_settings_config_modification_disabled_tooltip), xkb);
    
    /* toggle layout option */
    frame = xfce_framebox_new (_("Change layout option:"), TRUE);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

    xkb->toggle_options_combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (xkb->toggle_options_store));
    gtk_widget_set_size_request (xkb->toggle_options_combo, 230, -1);
    xfce_framebox_add (XFCE_FRAMEBOX (frame), xkb->toggle_options_combo);
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (xkb->toggle_options_combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (xkb->toggle_options_combo), renderer, "text", 0);
    xkl_config_registry_foreach_option (registry,
                                        "grp",
                                        (ConfigItemProcessFunc) xkb_settings_add_toggle_options_to_list,
                                        xkb);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (xkb->toggle_options_store),
                                          0, GTK_SORT_ASCENDING);

    xkb_settings_set_toggle_option_combo_default_value (xkb);
    gtk_widget_show (xkb->toggle_options_combo);

    gtk_widget_set_sensitive (GTK_WIDGET (xkb->toggle_options_combo), !xkb->settings->never_modify_config);
    g_object_set (G_OBJECT (xkb->toggle_options_combo), "has-tooltip", TRUE, NULL);
    g_signal_connect (xkb->toggle_options_combo, "query-tooltip", G_CALLBACK (xkb_settings_config_modification_disabled_tooltip), xkb);

    /* compose key position option */
    frame = xfce_framebox_new (_("Compose key position:"), TRUE);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

    xkb->compose_key_options_combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (xkb->compose_key_options_store));
    gtk_widget_set_size_request (xkb->compose_key_options_combo, 230, -1);
    xfce_framebox_add (XFCE_FRAMEBOX (frame), xkb->compose_key_options_combo);
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (xkb->compose_key_options_combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (xkb->compose_key_options_combo), renderer, "text", 0);
    xkl_config_registry_foreach_option (registry,
                                        "Compose key",
                                        (ConfigItemProcessFunc) xkb_settings_add_combo_key_position_options_to_list,
                                        xkb);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (xkb->compose_key_options_store),
                                          0, GTK_SORT_ASCENDING);

    xkb_settings_set_compose_key_position_combo_default_value (xkb);
    gtk_widget_show (xkb->compose_key_options_combo);

    gtk_widget_set_sensitive (GTK_WIDGET (xkb->compose_key_options_combo), !xkb->settings->never_modify_config);
    g_object_set (G_OBJECT (xkb->compose_key_options_combo), "has-tooltip", TRUE, NULL);
    g_signal_connect (xkb->compose_key_options_combo, "query-tooltip", G_CALLBACK (xkb_settings_config_modification_disabled_tooltip), xkb);

    
    /* the actual layouts */
    frame = xfce_framebox_new (_("Keyboard layouts:"), TRUE);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
    
    hbox = gtk_hbox_new (FALSE, 5);
    xfce_framebox_add (XFCE_FRAMEBOX (frame), hbox);
    gtk_widget_show (hbox);
    
    // TreeView
    xkb->layout_tree_view = gtk_tree_view_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (xkb->layout_tree_view),
                                               -1,      
                                               "Default",  
                                               renderer2,
                                               "active", DEFAULT_LAYOUT,
                                               NULL);
                                               
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (xkb->layout_tree_view),
                                               -1,      
                                               "Layout",  
                                               renderer,
                                               "text", LAYOUTS,
                                               NULL);
                                               
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (xkb->layout_tree_view),
                                               -1,      
                                               "Variant",  
                                               renderer,
                                               "text", VARIANTS,
                                               NULL);
    
    xkb->layout_store = gtk_list_store_new (TREE_NUM, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);
    xkb_settings_fill_layout_tree_model_with_config (xkb);
    
    gtk_tree_view_set_model (GTK_TREE_VIEW (xkb->layout_tree_view),
                            GTK_TREE_MODEL (xkb->layout_store));
    
    gtk_box_pack_start (GTK_BOX (hbox), xkb->layout_tree_view, TRUE, TRUE, 0);
    gtk_widget_set_size_request (xkb->layout_tree_view, -1, 112);
    gtk_widget_show (xkb->layout_tree_view);


    vbox2 = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);
    gtk_widget_show (vbox2);

    xkb->add_layout_btn = gtk_button_new_from_stock (GTK_STOCK_ADD);
    gtk_box_pack_start (GTK_BOX (vbox2), xkb->add_layout_btn, FALSE, FALSE, 0);
    gtk_widget_show (xkb->add_layout_btn);
    g_object_set (G_OBJECT (xkb->add_layout_btn), "has-tooltip", TRUE, NULL);
    g_signal_connect (xkb->add_layout_btn, "query-tooltip", G_CALLBACK (xkb_settings_config_modification_disabled_tooltip), xkb);


    xkb->edit_layout_btn = gtk_button_new_from_stock (GTK_STOCK_EDIT);
    gtk_box_pack_start (GTK_BOX (vbox2), xkb->edit_layout_btn, FALSE, FALSE, 0);
    gtk_widget_show (xkb->edit_layout_btn);
    g_object_set (G_OBJECT (xkb->edit_layout_btn), "has-tooltip", TRUE, NULL);
    g_signal_connect (xkb->edit_layout_btn, "query-tooltip", G_CALLBACK (xkb_settings_config_modification_disabled_tooltip), xkb);

    xkb->rm_layout_btn = gtk_button_new_from_stock (GTK_STOCK_DELETE);
    gtk_box_pack_start (GTK_BOX (vbox2), xkb->rm_layout_btn, FALSE, FALSE, 0);
    gtk_widget_show (xkb->rm_layout_btn);
    g_object_set (G_OBJECT (xkb->rm_layout_btn), "has-tooltip", TRUE, NULL);
    g_signal_connect (xkb->rm_layout_btn, "query-tooltip", G_CALLBACK (xkb_settings_config_modification_disabled_tooltip), xkb);

    xkb_settings_show_hide_layout_buttons (xkb);
    xkb_settings_edit_layout_btn_show (GTK_TREE_VIEW (xkb->layout_tree_view), xkb);

    /*****/
    display_type_frame = xfce_framebox_new (_("Show layout as:"), TRUE);
    gtk_widget_show (display_type_frame);
    gtk_box_pack_start (GTK_BOX (vbox), display_type_frame, TRUE, TRUE, 2);

    display_type_optmenu = gtk_combo_box_new_text ();
    gtk_combo_box_append_text (GTK_COMBO_BOX (display_type_optmenu), _("image"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (display_type_optmenu), _("text"));
    gtk_widget_set_size_request (display_type_optmenu, 230, -1);
    xfce_framebox_add (XFCE_FRAMEBOX (display_type_frame), display_type_optmenu);

    group_policy_frame = xfce_framebox_new (_("Manage layout:"), TRUE);
    gtk_widget_show (group_policy_frame);
    gtk_box_pack_start (GTK_BOX (vbox), group_policy_frame, TRUE, TRUE, 2);

    group_policy_combo = gtk_combo_box_new_text ();
    gtk_combo_box_append_text (GTK_COMBO_BOX (group_policy_combo), _("globally"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (group_policy_combo), _("per window"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (group_policy_combo), _("per application"));
    gtk_widget_set_size_request (group_policy_combo, 230, -1);
    xfce_framebox_add (XFCE_FRAMEBOX (group_policy_frame), group_policy_combo);
    gtk_widget_show (group_policy_combo);

    gtk_widget_show_all (vbox);

    g_signal_connect ((gpointer) settings_dialog, "response",
            G_CALLBACK (on_settings_close), xkb);

    gtk_combo_box_set_active (GTK_COMBO_BOX (display_type_optmenu), xkb->display_type);

    gtk_combo_box_set_active (GTK_COMBO_BOX (group_policy_combo), xkb->settings->group_policy);

    g_signal_connect (display_type_optmenu, "changed", G_CALLBACK (on_display_type_changed), xkb);
    g_signal_connect (group_policy_combo, "changed", G_CALLBACK (on_group_policy_changed), xkb);

    g_signal_connect (xkb->add_layout_btn, "clicked", G_CALLBACK (xkb_settings_add_layout), xkb);
    g_signal_connect (xkb->rm_layout_btn, "clicked", G_CALLBACK (xkb_settings_rm_layout), xkb);
    g_signal_connect (xkb->edit_layout_btn, "clicked",  G_CALLBACK (xkb_settings_edit_layout), xkb);
    g_signal_connect (xkb->layout_tree_view, "cursor-changed", G_CALLBACK (xkb_settings_edit_layout_btn_show), xkb);

    g_signal_connect (renderer2, "toggled", G_CALLBACK (xkb_settings_default_layout_toggled), xkb);

    gtk_widget_show (settings_dialog);
}

void
xfce_xkb_about (XfcePanelPlugin *plugin)
{
    GtkWidget *about;
    const gchar* authors[2] = {
        "Alexander Iliev <sasoiliev@mamul.org>", 
        "Gauvain Pocentek <gauvainpocentek@gmail.com>", 
        NULL
    };

    about = gtk_about_dialog_new ();
    gtk_about_dialog_set_name (GTK_ABOUT_DIALOG (about), 
            _("Keyboard Layouts Plugin"));
    gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (about), 
            NULL);
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
}

static void
xkb_settings_add_variant_to_available_layouts_tree (XklConfigRegistry * config_registry,
                                                    XklConfigItem * config_item, 
                                                    GtkTreeStore *treestore)
{
  char *utf_variant_name = xci_desc_to_utf8 (config_item);

  gtk_tree_store_append (treestore, &child, &iter);
  gtk_tree_store_set (treestore, &child, 
              AVAIL_LAYOUT_TREE_COL_DESCRIPTION, utf_variant_name, 
              AVAIL_LAYOUT_TREE_COL_ID, config_item->name, -1);
  g_free (utf_variant_name);
}

static void
xkb_settings_add_layout_to_available_layouts_tree (XklConfigRegistry * config_registry,
                                                   XklConfigItem * config_item, 
                                                   GtkTreeStore *treestore)
{
  char *utf_layout_name = xci_desc_to_utf8 (config_item);

  gtk_tree_store_append (treestore, &iter, NULL);
  gtk_tree_store_set (treestore, &iter, 
              AVAIL_LAYOUT_TREE_COL_DESCRIPTION, utf_layout_name, 
              AVAIL_LAYOUT_TREE_COL_ID, config_item->name, -1);
  g_free (utf_layout_name);

  xkl_config_registry_foreach_layout_variant (config_registry, config_item->name,
                   (ConfigItemProcessFunc)xkb_settings_add_variant_to_available_layouts_tree, 
                               treestore);
}

gchar *
xkb_settings_layout_dialog_run ()
{
    GtkWidget *dialog;
    GtkTreeStore *treestore;
    GtkWidget *tree_view = gtk_tree_view_new ();
    GtkCellRenderer *renderer;
    GtkWidget *scrolledw;
    XklConfigRegistry *registry;

    registry = xkb_config_get_xkl_registry ();

    dialog = xfce_titled_dialog_new_with_buttons(_("Add layout"),
                            GTK_WINDOW (settings_dialog),
                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_STOCK_CANCEL,
                            GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,
                            GTK_RESPONSE_OK,
                            NULL);

    gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-keyboard");

    treestore = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    
    xkl_config_registry_foreach_layout (registry, (ConfigItemProcessFunc)
            xkb_settings_add_layout_to_available_layouts_tree, treestore);

    renderer = gtk_cell_renderer_text_new ();
    
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (NULL,
                                    renderer,
                                    "text",
                                    AVAIL_LAYOUT_TREE_COL_DESCRIPTION,
                                    NULL);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view),
               GTK_TREE_MODEL (treestore));
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (treestore),
                                        0, GTK_SORT_ASCENDING);

    scrolledw = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), scrolledw);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledw),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_widget_show (scrolledw);
    
    gtk_container_add (GTK_CONTAINER (scrolledw), tree_view);
    gtk_widget_show (GTK_WIDGET (tree_view));
    
    gtk_window_set_default_size(GTK_WINDOW (dialog), 400, 400);
    gtk_widget_show (dialog);

    int response = gtk_dialog_run (GTK_DIALOG (dialog));
    
    if (response == GTK_RESPONSE_OK)
    {
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
        gchar *id;
        gchar *strings[2];
                
        gtk_tree_selection_get_selected (selection, &model, &iter);
        gtk_tree_model_get (model, &iter, AVAIL_LAYOUT_TREE_COL_ID, &id, -1);
        
        path = gtk_tree_model_get_path (model, &iter);
        if (gtk_tree_path_get_depth (path) == 1)
        {
            strings[0] = id;
            strings[1] = "";
        }
        else
        {
            strings[1] = id;
            gtk_tree_path_up(path);
            gtk_tree_model_get_iter(model, &iter, path);
            gtk_tree_model_get (model, &iter, AVAIL_LAYOUT_TREE_COL_ID, &id, -1);
            strings[0] = id;
        }

        gtk_widget_destroy (dialog);
        return g_strconcat(strings[0], ",", strings[1], NULL);
        
    }
    gtk_widget_destroy (dialog);
    return NULL;
}

void
xkb_settings_update_from_ui (t_xkb *xkb)
{
    gchar *layouts, *variants, *kbdmodel, *toggle_option, 
          *compose_key_position, *tmp;
    t_xkb_kbd_config *kbd_config = xkb->settings->kbd_config;
    gboolean is_default;
    gint i = 0;

    model = GTK_TREE_MODEL (xkb->combo_store);
    gtk_combo_box_get_active_iter (GTK_COMBO_BOX (xkb->kbd_model_combo), &iter);
    gtk_tree_model_get (model, &iter, NOM, &kbdmodel, -1);
    kbd_config->model = kbdmodel;

    model = GTK_TREE_MODEL (xkb->toggle_options_store);
    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (xkb->toggle_options_combo), &iter))
    {
        gtk_tree_model_get (model, &iter, NOM, &toggle_option, -1);
        g_free (kbd_config->toggle_option);
        kbd_config->toggle_option = g_strdup (toggle_option);
    }

    model = GTK_TREE_MODEL (xkb->compose_key_options_store);
    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (xkb->compose_key_options_combo), &iter))
    {
        gtk_tree_model_get (model, &iter, NOM, &compose_key_position, -1);
        g_free (kbd_config->compose_key_position);
        kbd_config->compose_key_position = g_strdup (compose_key_position);
    }

    model = GTK_TREE_MODEL (xkb->layout_store);
    gtk_tree_model_get_iter_first (model, &iter);
    gtk_tree_model_get (model, &iter, DEFAULT_LAYOUT, &is_default, LAYOUTS, &layouts, VARIANTS, &variants, -1);
    if (is_default) xkb->settings->default_group = i;
    kbd_config->layouts = layouts;
    if (variants != NULL)
        kbd_config->variants = variants;
    else
        kbd_config->variants = "";
    
    i = 1;
    while (gtk_tree_model_iter_next (model, &iter))
    {
        gtk_tree_model_get (model, &iter, DEFAULT_LAYOUT, &is_default, LAYOUTS, &layouts, VARIANTS, &variants, -1);
        if (is_default) xkb->settings->default_group = i;
        i++;

        kbd_config->layouts = g_strdup(g_strconcat(kbd_config->layouts, ",", layouts, NULL));
        if (variants != NULL)
            kbd_config->variants = g_strdup(g_strconcat(kbd_config->variants, ",", variants, NULL));
        else
            kbd_config->variants = g_strdup(g_strconcat(kbd_config->variants, ",", NULL));
    }
    xkb_config_update_settings (xkb->settings);
    xkb_refresh_gui (xkb);
}

