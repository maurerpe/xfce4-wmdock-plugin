/*  Copyright (C) 2021 Paul Maurer
 *  misc.c (C) 2021 tezeta
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <libxfce4ui/libxfce4ui.h>

#include <gtk/gtk.h>
#include <gtk/gtkx.h>

#include "misc.h"

gchar *wmdock_get_dockapp_cmd(WnckWindow *w)
{
  gchar *cmd = NULL;
  int wpid = 0;
  int argc = 0;
  int fcnt, i;
  char **argv;
  FILE *procfp = NULL;
  char buf[BUF_MAX];

  XGetCommand(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
    wnck_window_get_xid(w), &argv, &argc);

  if(argc > 0) {
    argv = (char **) realloc(argv, sizeof(char *) * (argc + 1));
    argv[argc] = NULL;
    cmd = g_strjoinv (" ", argv);
    XFreeStringList(argv);
  } else {
    /* Try to get the command line from the proc fs. */
    wpid = wnck_window_get_pid (w);

    if(wpid) {
      sprintf(buf, "/proc/%d/cmdline", wpid);

      procfp = fopen(buf, "r");

      if(procfp) {
        fcnt = read(fileno(procfp), buf, BUF_MAX);

        cmd = g_malloc(fcnt+2);
        if(!cmd) return (NULL);

        for(i = 0; i < fcnt; i++) {
          if(buf[i] == 0)
            *(cmd+i) = ' ';
          else
            *(cmd+i) = buf[i];
        }
        *(cmd+(i-1)) = 0;

        fclose(procfp);
      }
    }
  }

  if(!cmd) {
    /* If nothing helps fallback to the window name. */
    cmd = g_strdup(wnck_window_get_name(w));
  }

  return(cmd);
}

gboolean wmdock_startup_dockapp(const gchar *cmd)
{
  gboolean ret;
  GError *err = NULL;

  ret = xfce_spawn_command_line(gdk_screen_get_default(), cmd, 0, 0, 1, &err);

  if(err) g_clear_error (&err);

  return(ret);
}

