#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

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

  ret = xfce_spawn_command_line_on_screen(gdk_screen_get_default(), cmd, 0, 0, &err);

  if(err) g_clear_error (&err);

  return(ret);
}

