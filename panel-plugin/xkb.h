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

#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <glib.h>

/* # define TINY 0
# define SMALL 1
# define MEDIUM 2
# define LARGE 3
*/
#define ICONSIZETINY 24 
#define ICONSIZESMALL 30
#define ICONSIZEMEDIUM 45
#define ICONSIZELARGE 60

typedef enum {
  TEXT = 0,
  IMAGE = 1
} t_display_type;

typedef struct {
  GtkWidget	*ebox;
  GtkWidget *btn;
  GtkWidget *label;
  GtkWidget *image;
  GtkWidget *vbox;
  GtkWidget *def_lang_menu;
  
  gint size;
  
  t_display_type display_type;

  // perapps
  gboolean enable_perapp;
  // default group for "locale per process"
  gint default_group;
} t_xkb;

void set_new_locale(t_xkb *ctrl);
char *initialize_xkb(t_xkb *ctrl);
void deinitialize_xkb();

gint get_group_count();
char * get_symbol_name_by_res_no(int group_res_no);

int do_change_group(int increment, t_xkb *ctrl);
gboolean gio_callback(GIOChannel *source, GIOCondition condition, gpointer data);
int get_connection_number();

// "locale per process" functions
void react_active_window_changed(gint pid, t_xkb *ctrl);
void react_window_closed(gint pid);
int do_set_group(gint group, t_xkb *ctrl);
#endif
