/*
// ====================================================================
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
*/

#include "xkb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/XKBlib.h>

#include <gtk/gtk.h>
#include <glib.h>

Display *dsp;

int group_title_source;
int group_code_count;
Bool flexy_groups;
char **group_codes;
char **custom_names;

static int base_event_code;
static int base_error_code;

static int device_id;

int current_group_xkb_no, current_group_res_no;
int group_count;

char *group_names[XkbNumKbdGroups];
char *symbol_names[XkbNumKbdGroups];

#include <panel/xfce.h>
//int default_group = 0;
GHashTable* pGroupHash = NULL;

gint get_group_count() { 
  return group_count;
}
char *to_upper(char *src) {
  int i = 0;
  for (i = 0; i < strlen(src); i++) {
    src[i] = toupper(src[i]);
  }
  return src;
}
char *to_lower(char *src) {
  int i = 0;
  for (i = 0; i < strlen(src); i++) {
    src[i] = tolower(src[i]);
  }
  return src;
}

static int group_lookup(int source_value, char *from_texts[], char *to_texts[], int count) {
  if (flexy_groups) {
    const char *source_text = from_texts[source_value];

    if (source_text != NULL) {
      const char *target_text;
      int i;

      for (i=0; i<count; i++) {
        target_text = to_texts[i];
        if (strcasecmp(source_text, target_text) == 0) {
          source_value = i;
          break;
        }
      }
    }
  }

  return source_value;

}

static int group_xkb_to_res(int group_xkb_no) {
  return group_lookup(group_xkb_no, symbol_names, group_codes, group_code_count);
}

static int group_no_res_to_xkb(int group_res_no) {
  return group_lookup(group_res_no, group_codes, symbol_names, group_count);
}

static char * get_group_name_by_res_no(int group_res_no) {
  return group_names[group_no_res_to_xkb(group_res_no)];
}

char * get_symbol_name_by_res_no(int group_res_no) {
  return symbol_names[group_no_res_to_xkb(group_res_no)];
}

const char *get_current_group_name() {
  return get_symbol_name_by_res_no(current_group_xkb_no);
}

void accomodate_group_xkb(void) {
	XkbStateRec xkb_state;
	XkbGetState(dsp, device_id, &xkb_state);
	current_group_xkb_no = xkb_state.group;

	current_group_res_no = group_xkb_to_res(current_group_xkb_no);
}

int do_init_xkb() {
  const Atom *group_source;
  Bool status;
  int major, minor, oppcode;
  int i;
  XkbStateRec xkb_state;
  XkbDescRec *kbd_desc_ptr = NULL;
  const Atom *tmp_group_source;
  Atom cur_group_atom;
  Atom sym_name_atom;
  char *ptr;
  char *sym_name;
  char *ptr1;
  int  count;

  // create hash asap, so it'll be ready when events arrive
  pGroupHash = g_hash_table_new(g_direct_hash, NULL);


  /* Initialize the Xkb extension */
  status = XkbQueryExtension(dsp, &oppcode,
    &base_event_code, &base_error_code, &major, &minor);

  device_id = XkbUseCoreKbd;

  kbd_desc_ptr = XkbAllocKeyboard();
  if (kbd_desc_ptr == NULL) {
    fprintf(stderr, "Failed to get keyboard description\n");
    goto HastaLaVista;
  }

  kbd_desc_ptr->dpy = dsp;
  if (device_id != XkbUseCoreKbd) kbd_desc_ptr->device_spec = device_id;

  XkbGetControls(dsp, XkbAllControlsMask, kbd_desc_ptr);
  XkbGetNames(dsp, XkbSymbolsNameMask, kbd_desc_ptr);
  XkbGetNames(dsp, XkbGroupNamesMask, kbd_desc_ptr);

  if (kbd_desc_ptr->names == NULL) {
    fprintf(stderr, "Failed to get keyboard description\n");
    goto HastaLaVista;
  }

  group_source = kbd_desc_ptr->names->groups;

  /* And more bug patches ! */
  if (kbd_desc_ptr->ctrls != NULL) {
    group_count = kbd_desc_ptr->ctrls->num_groups;
  } else {
    for (group_count=0;
         group_count<XkbNumKbdGroups && group_source[group_count] != None;
         group_count++);
  }

  if (group_count == 0) group_count=1;

  for (i = 0; i < group_count; i++) {
    group_names[i] = NULL;
    symbol_names[i] = NULL;
  }

  tmp_group_source = kbd_desc_ptr->names->groups;

  for (i = 0; i < group_count; i++) {
    if ((cur_group_atom = tmp_group_source[i]) != None) {
      group_names[i] = ptr = XGetAtomName(dsp, cur_group_atom);
      if (ptr != NULL && (ptr=strchr(ptr, '(')) != NULL)
        *ptr = '\0';
    }
  }
  sym_name_atom = kbd_desc_ptr->names->symbols;
  if (sym_name_atom == None ||
      (sym_name = XGetAtomName(dsp, sym_name_atom)) == NULL) return 0;

  count = 0;

  for(ptr = strtok(sym_name, "+"); ptr != NULL; ptr = strtok(NULL, "+")) {
    ptr1 = strchr(ptr, '(');
    if (ptr1 != NULL) *ptr1 = '\0';
    ptr1 = strchr(ptr, '_');
    if (ptr1 != NULL && !isupper(*(ptr1+1))) *ptr1 = '\0';
    ptr1 = strchr(ptr, ':');
    if (ptr1 != NULL) *ptr1 = '\0';

    ptr1 = strrchr(ptr, '/');
    if (ptr1 != NULL) {
      /* Filter out cases like pc/pc */
      if (memcmp(ptr, ptr1+1, ptr1-ptr) == 0) continue;

      ptr = ptr1+1;
    }

    if (strncmp(ptr, "group", 5) == 0) continue;

    symbol_names[count++] = to_upper(strdup(ptr));
  }

  if (count == 1 && group_names[0] == NULL &&
      strcmp(symbol_names[0], "jp") == 0) {
    group_count = 2;
    symbol_names[1] = symbol_names[0];
    symbol_names[0] = strdup("us");
    group_names[0] = strdup("US/ASCII");
    group_names[1] = strdup("Japanese");
  } else {
    if (count<group_count) {
      int j=count, k=group_count;
      while(--j>=0) symbol_names[--k] = symbol_names[j];
      while(--k>=0) symbol_names[k] = strdup("en_US");
    }
  }

  count = (group_title_source == 2) ? group_code_count : group_count;

  for (i = 0; i < count; i++) {
    if (flexy_groups && group_codes[i] == NULL) {
      fprintf(stderr, "\nCode is not specified for Group %i !\n", i+1);
      fprintf(stderr, "Flexy mode is ignored\n");
      flexy_groups = False;
    }

    switch(group_title_source) {
    case 1: /* Group name */
      if (group_names[i] == NULL) {
        char *name = get_symbol_name_by_res_no(i);
        if (name == NULL) name = "U/A";
        fprintf(stderr, "\nGroup Name %i is undefined, set to '%s' !\n", i+1, name);
        group_names[i] = strdup(name);
      }
      break;

    case 2: /* Gustom name */
      if (custom_names[i] == NULL) {
        const char *name = get_symbol_name_by_res_no(i);
        if (name == NULL) name = get_group_name_by_res_no(i);
        if (name == NULL) name = "U/A";
        fprintf(stderr, "\nCustom Name %i is undefined, set to '%s' !\n", i+1, name);
        custom_names[i] = strdup(name);
      }
      break;

    default: /* Symbolic name (0), No title source but can be used for images (3) */
      if (symbol_names[i] == NULL) {
        fprintf(stderr, "\nGroup Symbol %i is undefined, set to 'U/A' !\n", i+1);
        symbol_names[i] = strdup("U/A");
      }
      break;
    }
  }

  XkbGetState(dsp, device_id, &xkb_state);
  current_group_xkb_no = xkb_state.group;


 status = True;

HastaLaVista:
  if (kbd_desc_ptr) XkbFreeKeyboard(kbd_desc_ptr, 0, True);
  return status;
}

char *get_current_gourp_flag_name(char *filename) {
  char *group;
  
  group = strdup(get_symbol_name_by_res_no(current_group_xkb_no));
  strcpy(filename, FLAGSDIR);
  strcat(filename, "/");
  strcat(filename, group);
  strcat(filename, ".png");
  to_lower(filename);
  
  free(group);
  
  return filename;
}

gboolean temporary_changed_display_type = FALSE;

gboolean is_current_group_flag_available() {
  char filename[255];
  gboolean result = FALSE;
  GdkPixbuf *tmp = gdk_pixbuf_new_from_file(get_current_gourp_flag_name(filename), NULL);
  result = (gboolean) (tmp != NULL);
  g_object_unref(tmp);
  return result;
}

void set_new_locale(t_xkb *ctrl) {
  t_xkb *plugin = (t_xkb *) ctrl;
  char filename[255];
  int size;
  GdkPixbuf *pixbuf, *tmp;

  /* Set the label   */
  gtk_label_set_label((GtkLabel *) plugin->label, get_symbol_name_by_res_no(current_group_xkb_no));
  
  /* Set the image */
  size = plugin->size - 4;
  tmp = gdk_pixbuf_new_from_file(get_current_gourp_flag_name(filename), NULL);
  if (tmp == NULL) { /* could not be loaded for some reason */
    if (plugin->display_type == IMAGE) {
      temporary_changed_display_type = TRUE;
      gtk_widget_hide(plugin->image);
      gtk_widget_show(plugin->label);
    }
  } else { /* loaded successfully */
    temporary_changed_display_type = TRUE;
	pixbuf = gdk_pixbuf_scale_simple(tmp, size, size - (int) (size / 3), GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf((GtkImage *) plugin->image, pixbuf);
    g_object_unref(G_OBJECT(tmp));
    g_object_unref(G_OBJECT(pixbuf));
    
    if (plugin->display_type == IMAGE) { 
      /* the image for the previous active layout could not be loaded */
      gtk_widget_hide(plugin->label);
      gtk_widget_show(plugin->image);
    }
  }

  /* Part of the image may remain visible after image or display type change */
  gtk_widget_queue_draw_area(plugin->btn, 0, 0, plugin->size, plugin->size);

  // "locale per process"
  // TBF:: bad here, it's not really a "window" related file
  NetkWindow* win = netk_screen_get_active_window(netk_screen_get_default());
  if (pGroupHash && win)
  {
	  gint pid = netk_window_get_pid(win);
		printf("Storing locale %s for %d\n", get_symbol_name_by_res_no(current_group_xkb_no), pid);
	
		g_hash_table_insert(pGroupHash, GINT_TO_POINTER(pid), GINT_TO_POINTER(current_group_xkb_no));
  }
}

void handle_xevent(t_xkb *ctrl) {
  XkbEvent evnt;

  XNextEvent(dsp, &evnt.core);
  if (evnt.type == base_event_code) {
    int new_group_no;

    if (evnt.any.xkb_type == XkbStateNotify &&
        (new_group_no = evnt.state.group) != current_group_xkb_no) {
      current_group_xkb_no = new_group_no;
      accomodate_group_xkb();
      set_new_locale(ctrl);
    }
  }
}

char * initialize_xkb(t_xkb *ctrl) {
  XkbEvent evnt;
  XkbStateRec state;
  int event_code, error_rtrn, major, minor, reason_rtrn;
  char * display_name;
  char *group;

  major = XkbMajorVersion;
  minor = XkbMinorVersion;

  display_name = "";
  XkbIgnoreExtension(False);
  dsp = XkbOpenDisplay(display_name, &event_code, &error_rtrn, &major, &minor, &reason_rtrn);

  switch (reason_rtrn) {
  case XkbOD_BadLibraryVersion:
    printf("Bad XKB library version.\n");
    return NULL;
  case XkbOD_ConnectionRefused:
    printf("Connection to X server refused.\n");
    return NULL;
  case XkbOD_BadServerVersion:
    printf("Bad X server version.\n");
    return NULL;
  case XkbOD_NonXkbServer:
    printf("XKB not present.\n");
    return NULL;
  case XkbOD_Success:
    break;
  }

  if (do_init_xkb() != True) return "N/A";

  group = get_symbol_name_by_res_no(current_group_xkb_no);

  XkbSelectEventDetails(dsp, XkbUseCoreKbd, XkbStateNotify,
                        XkbAllStateComponentsMask, XkbGroupStateMask);

  XkbGetState(dsp, device_id, &state);
  current_group_xkb_no = (current_group_xkb_no != state.group) ? state.group : current_group_xkb_no;
  accomodate_group_xkb();

  if (ctrl != NULL) set_new_locale(ctrl);

  return group;
}

static void deinit_group_names() {
	int i;
	for (i=0; i< group_count; i++) {
		if (group_names[i] != NULL) {
		  free(group_names[i]);
		  group_names[i] = NULL;
		}
		if (symbol_names[i] != NULL) {
		  free(symbol_names[i]);
		  symbol_names[i] = NULL;
		}
	}
}

void deinitialize_xkb() {
	deinit_group_names();
  XCloseDisplay(dsp);
  dsp = NULL;

  g_hash_table_destroy(pGroupHash);
  pGroupHash = NULL;
}

int get_connection_number() {
  return ConnectionNumber(dsp);
}

/* Sets the kb layout to the next layout */
int do_change_group(int increment, t_xkb *ctrl) {
  if (group_count <= 1) return 0;
  XkbLockGroup(dsp, device_id,
    (current_group_xkb_no + group_count + increment) % group_count); // why not simply (current_group_xkb_no + increment) % group_count ?
  handle_xevent(ctrl);
  return 1;
}

int do_set_group(int group, t_xkb *ctrl) {
  if (group >= group_count)
    return 0;
  XkbLockGroup(dsp, device_id, group);
	accomodate_group_xkb();
	set_new_locale(ctrl);
	
  return 1;
}

gboolean gio_callback(GIOChannel *source, GIOCondition condition, gpointer data) {
  handle_xevent((t_xkb *) data);
  return TRUE;
}

void react_active_window_changed(gint pid, t_xkb *ctrl)
{
	gpointer pKey=0, pVal=0;
	gint new_group_xkb_no = ctrl->default_group;
	
	if (pGroupHash && g_hash_table_lookup_extended(pGroupHash, GINT_TO_POINTER(pid), &pKey, &pVal))
    	new_group_xkb_no = GPOINTER_TO_INT(pVal);

	do_set_group(new_group_xkb_no, ctrl);
}

void react_application_closed(gint pid)
{
  if (pGroupHash) {
    g_hash_table_remove(pGroupHash, GINT_TO_POINTER(pid));
  }
}
