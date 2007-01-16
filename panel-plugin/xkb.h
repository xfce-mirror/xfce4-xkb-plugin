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

#ifndef _XFCE_XKB_H_
#define _XFCE_XKB_H_

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <glib.h>

#define ICONSIZETINY 24 
#define ICONSIZESMALL 30
#define ICONSIZEMEDIUM 45
#define ICONSIZELARGE 60

typedef enum 
{
  IMAGE = 0,
  TEXT = 1
} t_display_type;

typedef struct 
{
  XfcePanelPlugin *plugin;

  /* options */
  gint size;                    /* plugin size */
  t_display_type display_type;  /* display layout as image ot text */
  gboolean enable_perapp;       /* enable per application (window) layout) */
  gint default_group;           /* default group for "locale per process" */

  /* widgets */
  GtkWidget *ebox;
  GtkWidget *btn;
  GtkWidget *label;
  GtkWidget *image;
  GtkWidget *vbox;
  GtkWidget *def_lang_menu;
} t_xkb;

typedef struct
{
  t_xkb *xkb;

  GtkWidget *dialog;

  /* display type menu */
  GtkWidget *display_type_optmenu;    
  GtkWidget *display_type_menu_label_text;
  GtkWidget *display_type_menu_label_image;
 
  /* layout per application options */
  GtkWidget *per_app_frame;
  GtkWidget *per_app_vbox;
  GtkWidget *per_app_checkbutton;
  GtkWidget *per_app_default_layout_menu;
} t_xkb_options_dlg;

void set_new_locale(t_xkb *ctrl);
const char *initialize_xkb(t_xkb *ctrl);
void deinitialize_xkb();
void react_application_closed(gint pid);
gint get_group_count();
const char * get_symbol_name_by_res_no(int group_res_no);

int do_change_group(int increment, t_xkb *ctrl);
gboolean gio_callback(GIOChannel *source, GIOCondition condition, gpointer data);
int get_connection_number();
gboolean is_current_group_flag_available(void);
/* "locale per process" functions */
void react_active_window_changed(gint pid, t_xkb *ctrl);
void react_window_closed(gint pid);
int do_set_group(gint group, t_xkb *ctrl);

#endif

