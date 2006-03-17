/*
//====================================================================
//  xfce4-xkb-plugin - XFCE4 Xkb Layout Indicator panel plugin
// -------------------------------------------------------------------
//  Alexander Iliev <sasoiliev@mamul.org>
//  20-Feb-04
// -------------------------------------------------------------------
//  Parts of this code belong to Michael Glickman <wmalms@yahooo.com>
//  and his program wmxkb.
//  WARNING: DO NOT BOTHER Michael Glickman WITH QUESTIONS ABOUT THIS
//           PROGRAM!!! SEND INSTEAD EMAILS TO <sasoiliev@mamul.org>
//====================================================================
*/

#include "xkb.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/i18n.h>
#include <libxfcegui4/dialogs.h>
#include <panel/plugins.h>
#include <panel/xfce.h>

#include <stdio.h>
#include <ctype.h>

#include <glib.h>

t_xkb *plugin;
GIOChannel *channel;
guint source_id;

gulong win_change_hanler, win_close_hanler;


static void active_window_changed(NetkScreen *screen, gpointer data) {
	NetkWindow* win = netk_screen_get_active_window(screen);
	if (!win)
		return;

	react_active_window_changed(netk_window_get_pid(win), plugin);
}

static void application_closed(NetkScreen *screen, NetkApplication* app, gpointer data) {
  react_application_closed(netk_application_get_pid(app));	
}

void change_group() {
  do_change_group(1, plugin);
}

static void xkb_refresh_gui(t_xkb *data) {
  t_xkb *plugin = (t_xkb *) data;

  switch (plugin->display_type) {
    case TEXT:
      gtk_widget_hide(plugin->image);
      gtk_widget_show(plugin->label);
      break;
    case IMAGE:
      if (is_current_group_flag_available()) {
        gtk_widget_hide(plugin->label);
        gtk_widget_show(plugin->image);
      }
      break;
    default: break;
  }

  /* Part of the image may remain visible after display type change */
  gtk_widget_queue_draw_area(plugin->btn, 0, 0, plugin->size, plugin->size);
}

static t_xkb * xkb_new(void) {
  t_xkb *xkb;
  char *initial_group;
  
  xkb = g_new(t_xkb, 1);

  xkb->size = ICONSIZETINY;
  xkb->display_type = TEXT;

  /*  defaults. xfce removes plugin's settings on plugin removal, so on first run
      it displays wrong flag/text  */
  xkb->display_type = 1; // IMAGE
  xkb->enable_perapp = TRUE;
  xkb->default_group = 0;


  xkb->ebox = gtk_event_box_new();
  gtk_widget_show(xkb->ebox);

  xkb->btn = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(xkb->btn), GTK_RELIEF_NONE);
  gtk_widget_show(xkb->btn);
  gtk_container_add(GTK_CONTAINER(xkb->ebox), xkb->btn);
  g_signal_connect(xkb->btn, "clicked", G_CALLBACK(change_group), do_change_group);
  
  xkb->vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(xkb->btn), xkb->vbox);
  
  xkb->label = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(xkb->vbox), xkb->label);
  xkb->image = gtk_image_new();
  gtk_container_add(GTK_CONTAINER(xkb->vbox), xkb->image);
/*  gtk_box_pack_start(GTK_BOX(xkb->vbox), xkb->label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(xkb->vbox), xkb->image, TRUE, TRUE, 0); */

  gtk_widget_show(xkb->vbox);

  initial_group = initialize_xkb(xkb);

  xkb_refresh_gui(xkb);


  channel = g_io_channel_unix_new(get_connection_number());
  source_id = g_io_add_watch(channel, G_IO_IN | G_IO_PRI, (GIOFunc) &gio_callback, (gpointer) xkb);

  plugin = xkb;

  // track signals about window change
	NetkScreen* netk_screen = netk_screen_get_default ();
	win_change_hanler = g_signal_connect( G_OBJECT (netk_screen), "active_window_changed", 
		G_CALLBACK(active_window_changed), NULL);

	win_close_hanler = g_signal_connect( G_OBJECT (netk_screen), "application_closed", 
    	G_CALLBACK(application_closed), NULL);

  return(xkb);
}

static gboolean xkb_control_new(Control *ctrl) {
  t_xkb *xkb;

  xkb = xkb_new();

  gtk_container_add(GTK_CONTAINER(ctrl->base), xkb->ebox);

  ctrl->data = (gpointer)xkb;
  ctrl->with_popup = FALSE;

  gtk_widget_set_size_request(ctrl->base, -1, -1);

  return(TRUE);
}

static void xkb_free(Control *ctrl) {
  t_xkb *xkb;

  NetkScreen* netk_screen = netk_screen_get_default ();
	g_signal_handler_disconnect(netk_screen, win_change_hanler);
	g_signal_handler_disconnect(netk_screen, win_close_hanler);

  g_source_remove(source_id);
  deinitialize_xkb();


  g_return_if_fail(ctrl != NULL);
  g_return_if_fail(ctrl->data != NULL);

  xkb = (t_xkb *)ctrl->data;

  g_free(xkb);
}

static void xkb_read_config(Control *ctrl, xmlNodePtr node) {
  xmlChar *value;
  t_xkb *plugin = ctrl->data;

  

  for (node = node->children; node; node = node->next) {
    if (xmlStrEqual(node->name, (const xmlChar *)"XKBLayoutSwitch")) {
      if ((value = xmlGetProp(node, (const xmlChar *)"displayType"))) {
        plugin->display_type = atoi(value);
        g_free(value);
      }
      else
        plugin->display_type = 1; // IMAGE

      if ((value = xmlGetProp(node, (const xmlChar *)"enablePerapp"))) {
        plugin->enable_perapp = atoi(value);
        g_free(value);
      }
      else
        plugin->enable_perapp = TRUE;
      
      if ((value = xmlGetProp(node, (const xmlChar *)"defaultGroup"))) {
        plugin->default_group = atoi(value);
        g_free(value);
      }
      else
        plugin->default_group = 0; // first one specified in XF86Config
      
      break;
    }
  }
  xkb_refresh_gui(plugin);
}

static void xkb_write_config(Control *ctrl, xmlNodePtr parent) {
  t_xkb *plugin = (t_xkb *) ctrl->data;
  xmlNodePtr root;
  char value[11];

  root = xmlNewTextChild(parent, NULL, "XKBLayoutSwitch", NULL);
  g_snprintf(value, 10, "%d", plugin->display_type);
  xmlSetProp(root, "displayType", value);
  g_snprintf(value, 10, "%d", plugin->enable_perapp);
  xmlSetProp(root, "enablePerapp", value);
  g_snprintf(value, 10, "%d", plugin->default_group);
  xmlSetProp(root, "defaultGroup", value);
}

static void xkb_attach_callback(Control *ctrl, const gchar *signal, GCallback cb, gpointer data) {
  t_xkb *xkb;

  xkb = (t_xkb *)ctrl->data;
  g_signal_connect(xkb->ebox, signal, cb, data);
  g_signal_connect(xkb->btn, signal, cb, data);
}

static void xkb_set_size(Control *ctrl, int size) {
  t_xkb *xkb = (t_xkb *) ctrl->data;
  if (size == TINY) {
    xkb->size = ICONSIZETINY;
  } else if (size == SMALL) {
    xkb->size = ICONSIZESMALL;
  } else if (size == MEDIUM) {
    xkb->size = ICONSIZEMEDIUM;
  } else {
    xkb->size = ICONSIZELARGE;
  }
  gtk_widget_set_size_request(xkb->btn, xkb->size, xkb->size);
  set_new_locale(xkb);
}

static void xkb_display_type_changed(GtkOptionMenu *om, gpointer *data) {
  t_xkb *xkb = (t_xkb *) data;
  xkb->display_type = gtk_option_menu_get_history(om);
  xkb_refresh_gui(xkb);
}

static void xkb_enable_perapp_changed(GtkToggleButton *tb, gpointer *data) {
  t_xkb *xkb = (t_xkb *) data;
  xkb->enable_perapp = gtk_toggle_button_get_active(tb);
  gtk_widget_set_sensitive(xkb->def_lang_menu, xkb->enable_perapp);
}

static void xkb_def_lang_changed(GtkComboBox *cb, gpointer *data) {
  t_xkb *xkb = (t_xkb *) data;
  xkb->default_group = gtk_combo_box_get_active(cb);
}

static void xkb_create_options (Control *ctrl, GtkContainer *con, GtkWidget *done) {
  t_xkb *xkb = (t_xkb *) ctrl->data;
  GtkWidget *vbox, *hbox, *label, *menu, *opt_menu;

  vbox = gtk_vbox_new(FALSE, 2);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(con), vbox);

  hbox = gtk_hbox_new(FALSE, 2);
  gtk_widget_show(hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  //gtk_container_add(GTK_CONTAINER(con), hbox);

  menu = gtk_menu_new();
  gtk_menu_append(GTK_MENU(menu), gtk_menu_item_new_with_label("text"));
  gtk_menu_append(GTK_MENU(menu), gtk_menu_item_new_with_label("image"));
  opt_menu = gtk_option_menu_new();
  gtk_option_menu_set_menu(GTK_OPTION_MENU(opt_menu), menu);

  if (xkb->display_type == TEXT)
    gtk_option_menu_set_history(GTK_OPTION_MENU(opt_menu), 0);
  else
    gtk_option_menu_set_history(GTK_OPTION_MENU(opt_menu), 1);

  label = gtk_label_new("Display layout as:");
/*  gtk_container_add(GTK_CONTAINER(hbox), label);
  gtk_container_add(GTK_CONTAINER(hbox), opt_menu); */
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), opt_menu, TRUE, TRUE, 2);

	// per app
  GtkWidget *frame1, *alignment1, *vbox1, *perapp_checkbutton, *hbox3,
  	*label4, *def_lang_menu, *label5;


  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (vbox), frame1, TRUE, TRUE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (frame1), alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 2, 2, 12, 2);

  vbox1 = gtk_vbox_new (FALSE, 2);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (alignment1), vbox1);

  perapp_checkbutton = gtk_check_button_new_with_mnemonic( _("_Remember input locale for each application"));
  gtk_widget_show (perapp_checkbutton);
  gtk_box_pack_start (GTK_BOX (vbox1), perapp_checkbutton, FALSE, FALSE, 2);
  gtk_toggle_button_set_active((GtkToggleButton*)perapp_checkbutton, xkb->enable_perapp);

  hbox3 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, TRUE, TRUE, 2);

  label4 = gtk_label_new (_("Default group:"));
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (hbox3), label4, FALSE, FALSE, 2);

  xkb->def_lang_menu = gtk_combo_box_new_text ();
  gtk_widget_show (xkb->def_lang_menu);
  gtk_box_pack_start (GTK_BOX (hbox3), xkb->def_lang_menu, FALSE, TRUE, 2);
  printf("We've got %d groups:\n", get_group_count());
  int x;
  for (x=0; x<get_group_count(); x++)
    gtk_combo_box_append_text(GTK_COMBO_BOX (xkb->def_lang_menu), get_symbol_name_by_res_no(x));

  gtk_combo_box_set_active((GtkComboBox*)xkb->def_lang_menu, xkb->default_group);
  
  label5 = gtk_label_new (_("Per applcation settings"));
  gtk_widget_show (label5);
  gtk_frame_set_label_widget (GTK_FRAME (frame1), label5);
  gtk_label_set_use_markup (GTK_LABEL (label5), TRUE);
	
	
  gtk_widget_show_all(vbox);

  g_signal_connect(opt_menu, "changed", G_CALLBACK(xkb_display_type_changed), xkb);
  g_signal_connect(perapp_checkbutton, "toggled", G_CALLBACK(xkb_enable_perapp_changed), xkb);
  g_signal_connect(xkb->def_lang_menu, "changed", G_CALLBACK(xkb_def_lang_changed), xkb);
}

G_MODULE_EXPORT void xfce_control_class_init(ControlClass *cc) {
  /* these are required */
  cc->name = "xkb";
  cc->caption = _("XKB Layout Switcher");

  cc->create_control = (CreateControlFunc)xkb_control_new;

  cc->free = xkb_free;
  cc->attach_callback = xkb_attach_callback;

  cc->read_config = xkb_read_config;
  cc->write_config = xkb_write_config;
  cc->create_options = xkb_create_options;

  cc->set_size = xkb_set_size;

  /* unused in the sample:
   * ->set_orientation
   * ->set_theme
   */
}

/* required! defined in panel/plugins.h */
XFCE_PLUGIN_CHECK_INIT
