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
  
  gint size;
  
  t_display_type display_type;
} t_xkb;

char * initialize_xkb(t_xkb *ctrl);
void deinitialize_xkb();

int do_change_group(int increment, t_xkb *ctrl);
gboolean gio_callback(GIOChannel *source, GIOCondition condition, gpointer data);
int get_connection_number();

#endif
