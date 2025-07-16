/* vim: set backspace=2 ts=4 softtabstop=4 sw=4 cinoptions=>4 expandtab autoindent smartindent: */
/* xkb-util.c
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

#include <string.h>

#include "xkb-util.h"



gchar*
xkb_util_get_flag_filename (const gchar* group_name)
{
  gchar* filename;

  if (!group_name)
    return NULL;

  filename = g_strconcat (g_get_user_data_dir (), "/", FLAGSRELDIR, "/", group_name, ".svg", NULL);

  if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    {
      g_free (filename);
      filename = g_strconcat (DATADIR, "/", FLAGSRELDIR, "/", group_name, ".svg", NULL);
    }

  return filename;
}



gchar*
xkb_util_get_layout_string (const gchar *group_name,
                            const gchar *variant)
{
  gchar *layout;

  if (!group_name)
    return NULL;

  if (variant && strlen (variant) > 0)
    layout = g_strconcat (group_name, " (", variant, ")", NULL);
  else
    layout = g_strconcat (group_name, NULL);

  return layout;
}



gchar*
xkb_util_normalize_group_name (const gchar *group_name,
                               gboolean     capitalize)
{
  const gchar *c;
  gchar       *result;
  gint         cut_length;
  gint         index_of_na = -1;

  if (!group_name)
    return NULL;

  cut_length = strlen (group_name);

  if (cut_length > 3)
    {
      for (c = group_name; *c; c++)
        {
          if (!((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z')))
            {
              index_of_na = c - group_name;
              break;
            }
        }

      cut_length = (index_of_na != -1 && index_of_na <= 3) ? index_of_na : 3;
    }

  if (capitalize)
    result = g_ascii_strup (group_name, cut_length);
  else
    result = g_strndup (group_name, cut_length);

  return result;
}
