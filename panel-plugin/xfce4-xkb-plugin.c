//====================================================================
//  xfce4-xkb-plugin - XFCE4 Xkb Layout Indicator panel plugin
// -------------------------------------------------------------------
//  Alexander Iliev <sasoiliev@mail.bg>
//  20-Feb-04
// -------------------------------------------------------------------
//  Parts of this code belong to Michael Glickman <wmalms@yahooo.com>
//  and his program wmxkb.
//  WARNING: DO NOT BOTHER Michael Glickman WITH QUESTIONS ABOUT THIS
//           PROGRAM!!! SEND INSTEAD EMAILS TO <sasoiliev@mail.bg>
//====================================================================

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

typedef enum {
  TEXT = 0,
  IMAGE = 1
} t_display_type;

typedef struct {
  GtkWidget	*ebox;
  GtkWidget *btn;
  
  t_display_type display_type;
} t_xkb;

t_xkb *plugin;
GIOChannel *channel;
guint source_id;

void change_group() {
  do_change_group(1, (GtkButton *) plugin->btn);
}

static t_xkb * xkb_new(void) {
  t_xkb *xkb;

  xkb = g_new(t_xkb, 1);

  xkb->ebox = gtk_event_box_new();
  gtk_widget_show(xkb->ebox);

  xkb->btn = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(xkb->btn), GTK_RELIEF_NONE);

  char *initial_group = initialize_xkb((GtkButton *) xkb->btn);
//_(initial_group)

  gtk_widget_show(xkb->btn);
  gtk_container_add(GTK_CONTAINER(xkb->ebox), xkb->btn);
  g_signal_connect(xkb->btn, "clicked", G_CALLBACK(change_group), do_change_group);

  channel = g_io_channel_unix_new(get_connection_number());
  source_id = g_io_add_watch(channel, G_IO_IN | G_IO_PRI, (GIOFunc) &gio_callback, (gpointer) xkb->btn);

  plugin = xkb;

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
  g_source_remove(source_id);
  deinitialize_xkb();

  t_xkb *xkb;

  g_return_if_fail(ctrl != NULL);
  g_return_if_fail(ctrl->data != NULL);

  xkb = (t_xkb *)ctrl->data;

  g_free(xkb);
}

static void xkb_read_config(Control *ctrl, xmlNodePtr parent) {
  
}

static void xkb_write_config(Control *ctrl, xmlNodePtr parent) {
  
}

static void xkb_attach_callback(Control *ctrl, const gchar *signal, GCallback cb,
                                gpointer data) {
  t_xkb *xkb;

  xkb = (t_xkb *)ctrl->data;
  g_signal_connect(xkb->ebox, signal, cb, data);
  g_signal_connect(xkb->btn, signal, cb, data);
}

static void xkb_set_size(Control *ctrl, int size) {}

static void xkb_display_type_changed(GtkOptionMenu *om, gpointer *data) {
  t_xkb *xkb = (t_xkb *) data;
  xkb->display_type = gtk_option_menu_get_history(om);
  printf("in xkb_display_type_changed: %i \n", xkb->display_type);
}

static void xkb_create_options (Control *ctrl, GtkContainer *con, GtkWidget *done) {
  t_xkb *xkb = (t_xkb *) ctrl->data;
  GtkWidget *hbox, *label, *menu, *opt_menu;

  hbox = gtk_hbox_new(FALSE, 2);
  gtk_widget_show(hbox);
  gtk_container_add(GTK_CONTAINER(con), hbox);

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
  gtk_container_add(GTK_CONTAINER(hbox), label);
  gtk_container_add(GTK_CONTAINER(hbox), opt_menu);
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), opt_menu, TRUE, TRUE, 2);

  gtk_widget_show_all(hbox);

  g_signal_connect(opt_menu, "changed", G_CALLBACK(xkb_display_type_changed), xkb);
}

G_MODULE_EXPORT void xfce_control_class_init(ControlClass *cc) {
  /* these are required */
  cc->name		= "xkb";
  cc->caption		= _("XKB Layout Switcher");

  cc->create_control	= (CreateControlFunc)xkb_control_new;

  cc->free		= xkb_free;
  cc->attach_callback	= xkb_attach_callback;

  /* options; don't define if you don't have any ;) */
  cc->read_config	= xkb_read_config;
  cc->write_config	= xkb_write_config;
  cc->create_options	= xkb_create_options;

  /* unused in the sample:
   * ->set_orientation
   * ->set_theme
   */
}

/* required! defined in panel/plugins.h */
XFCE_PLUGIN_CHECK_INIT
