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

#include <pthread.h>

typedef struct {
  GtkWidget	*ebox;
  GtkWidget *btn;
  guint      timeout_id;
  guint      timeout_id2;
} t_xkb;

#define INIT_TIMEOUT  500

t_xkb *plugin;
pthread_t thrd;

char *to_upper(char *src) {
  int i = 0;
  for (i = 0; i < strlen(src); i++) {
    src[i] = toupper(src[i]);
  }
  return src;
}

void *refresh(void *ptr) {
  char *label_text;
  label_text = (char *) ptr;
  gtk_button_set_label((GtkButton *) plugin->btn, to_upper(label_text));
}

void change_group() {
  do_change_group(1, (void*)&refresh);
}

gint init_xkb(t_xkb * xkb) {
  pthread_create( &thrd, NULL, (void*)&catch_the_keys, (void*)&refresh); //xkb->label
  return FALSE;
}

static t_xkb * xkb_new(void) {
  t_xkb *xkb;

  xkb = g_new(t_xkb, 1);

  char *initial_group = initialize_xkb();

  xkb->ebox = gtk_event_box_new();
  gtk_widget_show(xkb->ebox);

  xkb->btn = gtk_button_new_with_label(_(to_upper(initial_group)));
  gtk_button_set_relief(GTK_BUTTON(xkb->btn), GTK_RELIEF_NONE);

  gtk_widget_show(xkb->btn);
  gtk_container_add(GTK_CONTAINER(xkb->ebox), xkb->btn);
  g_signal_connect(xkb->btn, "clicked", G_CALLBACK(change_group), do_change_group);

  xkb->timeout_id = g_timeout_add(INIT_TIMEOUT, (GtkFunction)init_xkb, xkb);

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
  terminate();
  pthread_kill_other_threads_np();
  deinitialize_xkb();

  t_xkb *xkb;

  g_return_if_fail(ctrl != NULL);
  g_return_if_fail(ctrl->data != NULL);

  xkb = (t_xkb *)ctrl->data;

  g_free(xkb);
}

static void xkb_read_config(Control *ctrl, xmlNodePtr parent) {}

static void xkb_write_config(Control *ctrl, xmlNodePtr parent) {}

static void xkb_attach_callback(Control *ctrl, const gchar *signal, GCallback cb,
                                gpointer data) {
  t_xkb *xkb;

  xkb = (t_xkb *)ctrl->data;
  g_signal_connect(xkb->ebox, signal, cb, data);
  g_signal_connect(xkb->btn, signal, cb, data);
}

static void xkb_set_size(Control *ctrl, int size) {}

static void xkb_create_options (Control *ctrl, GtkContainer *con, GtkWidget *done) {}

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
