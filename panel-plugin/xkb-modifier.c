/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-modifier.c
 * Copyright (C) 2017 Alexander Iliev <sasoiliev@mamul.org>
 *
 * Parts of this program comes from the XfKC tool:
 * Copyright (C) 2006 Gauvain Pocentek <gauvainpocentek@gmail.com>
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

#include "xkb-modifier.h"

#include <gdk/gdk.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

struct _XkbModifierClass
{
  GObjectClass         __parent__;
};

struct _XkbModifier
{
  GObject              __parent__;

  gint                 xkb_event_type;
  gboolean             caps_lock_enabled;
};

static GdkFilterReturn   xkb_modifier_handle_xevent            (GdkXEvent            *xev,
                                                                GdkEvent             *event,
                                                                gpointer              user_data);

static void              xkb_modifier_finalize                 (GObject              *object);

enum
{
  MODIFIER_CHANGED,
  LAST_SIGNAL
};

static guint xkb_modifier_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (XkbModifier, xkb_modifier, G_TYPE_OBJECT)



static void
xkb_modifier_class_init (XkbModifierClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xkb_modifier_finalize;

  xkb_modifier_signals[MODIFIER_CHANGED] =
    g_signal_new (g_intern_string ("modifier-changed"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}



static void
xkb_modifier_init (XkbModifier *modifier)
{
  modifier->xkb_event_type = 0;
  modifier->caps_lock_enabled = FALSE;
}



XkbModifier *
xkb_modifier_new (void)
{
  XkbModifier *modifier;
  Display     *display;
  XkbDescRec  *xkb_desc;
  gint         i;
  guint        states, caps_lock_mask;
  gchar       *atom_name;

  modifier = g_object_new (TYPE_XKB_MODIFIER, NULL);

  /* obtain xkb_event type and caps lock state */
  display = XOpenDisplay (NULL);
  if (display != NULL)
    {
      xkb_desc = XkbGetKeyboard (display, XkbAllComponentsMask, XkbUseCoreKbd);
      if (xkb_desc != NULL)
        {
          for (i = 0; i < XkbNumIndicators; i++)
            {
              if (xkb_desc->names->indicators[i])
                {
                  atom_name = XGetAtomName (display, xkb_desc->names->indicators[i]);
                  if (g_strcmp0 (atom_name, "Caps Lock") == 0)
                    {
                      if (XkbGetIndicatorState (display, XkbUseCoreKbd, &states) == Success)
                        {
                          caps_lock_mask = 1 << i;
                          modifier->caps_lock_enabled = (states & caps_lock_mask) == caps_lock_mask;
                        }

                      break;
                    }
                }
             }

          XkbFreeKeyboard(xkb_desc, 0, True);
        }

      XkbQueryExtension(display, NULL, &modifier->xkb_event_type, NULL, NULL, NULL);
      XCloseDisplay(display);
    }

  gdk_window_add_filter (NULL, xkb_modifier_handle_xevent, modifier);

  return modifier;
}



static void
xkb_modifier_finalize (GObject *object)
{
  XkbModifier *modifier = XKB_MODIFIER (object);

  gdk_window_remove_filter (NULL, xkb_modifier_handle_xevent, modifier);

  G_OBJECT_CLASS (xkb_modifier_parent_class)->finalize (object);
}



static GdkFilterReturn
xkb_modifier_handle_xevent (GdkXEvent *xev,
                            GdkEvent  *event,
                            gpointer   user_data)
{
  XkbModifier         *modifier = user_data;
  Display             *display;
  XkbStateNotifyEvent *state_event = xev;
  guint                modifier_flags;

  if (modifier->xkb_event_type != 0 &&
      state_event->type == modifier->xkb_event_type &&
      state_event->xkb_type == XkbStateNotify &&
      state_event->changed & XkbModifierLockMask)
    {
      display = XOpenDisplay (NULL);

      if (display != NULL)
        {
          modifier_flags = XkbKeysymToModifiers (display, XK_Caps_Lock);
          modifier->caps_lock_enabled = (state_event->locked_mods & modifier_flags) == modifier_flags;
          XCloseDisplay (display);

          g_signal_emit (G_OBJECT (modifier),
                         xkb_modifier_signals[MODIFIER_CHANGED],
                         0, FALSE);
        }
    }

  return GDK_FILTER_CONTINUE;
}



gboolean
xkb_modifier_get_caps_lock_enabled (XkbModifier *modifier)
{
  g_return_val_if_fail (IS_XKB_MODIFIER (modifier), 0);

  return modifier->caps_lock_enabled;
}
