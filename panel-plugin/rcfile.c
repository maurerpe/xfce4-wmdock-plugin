/*  Copyright (C) 2021 Paul Maurer
 *  rcfile.c (C) 2021 tezeta
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

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>

#include "wmdock.h"
#include "rcfile.h"
#include "misc.h"

#define DOCKAPP_SIZE 64

void wmdock_read_rc_file (WmdockPlugin *wmdock)
{
  gchar     *file = NULL;
  XfceRc    *rc = NULL;
  gchar **rcCmds = NULL;
  gboolean writeCfg = TRUE;
  gint      i = 0;
  gpointer launched = NULL;
  XfcePanelPlugin *plugin = NULL;
  GList *windows;
  WnckHandle *handle;
  WnckScreen *screen;

  handle = wnck_handle_new(WNCK_CLIENT_TYPE_APPLICATION);
  screen = wnck_handle_get_default_screen(handle);

  plugin = wmdock->plugin;
  if (!(file = xfce_panel_plugin_lookup_rc_file (plugin))) return;

  rc = xfce_rc_simple_open (file, TRUE);
  g_free(file);
  if(!rc)
    return;

  rcCmds = xfce_rc_read_list_entry(rc, RCKEY_CMDLIST, RC_LIST_DELIMITER);
  writeCfg = xfce_rc_read_bool_entry(rc, RCKEY_WRITECFG, TRUE);
  xfce_rc_close (rc);

  wmdock->writeCfg = writeCfg;

  if(G_LIKELY(rcCmds != NULL)) {
    if(!(launched = g_malloc0(sizeof (DockApp *) * (g_strv_length(rcCmds)))))
      return;

    /* launch dockapps */
    for (i=0;rcCmds[i]; i++) {
      wmdock_startup_dockapp(rcCmds[i]);
    }

    g_usleep(1 * G_USEC_PER_SEC);

    /* create tiles for dockapps */
    for (i=0;rcCmds[i]; i++) {
      fprintf(stderr,"rcfile.c: Restoring saved dockapp: %s\n", rcCmds[i]);

      /* capture dockapp into tile */
      wnck_screen_force_update(screen);
      for (windows = wnck_screen_get_windows(screen); windows != NULL; windows = windows->next) {
        WnckWindow *window = WNCK_WINDOW(windows->data);
        if ((strcmp(rcCmds[i], wmdock_get_dockapp_cmd(window)) == 0) && (is_dockapp(window))) {
          dockapp_new(window);
          break;
        }
      }
    }
  }

  g_free(launched);
}


void wmdock_write_rc_file (WmdockPlugin *wmdock)
{
  gchar       *file = NULL;
  XfceRc      *rc;
  gchar       **cmdList = NULL;
  GList       *dapps;
  DockApp *dapp = NULL;
  gint        i = 0;

  if (!wmdock->writeCfg)
    return;

  if (!(file = xfce_panel_plugin_save_location (wmdock->plugin, TRUE))) return;

  rc = xfce_rc_simple_open (file, FALSE);
  g_free (file);

  if (!rc)
    return;

  xfce_rc_write_bool_entry(rc, RCKEY_WRITECFG, wmdock->writeCfg);
  if(g_list_length (wmdock->dapps) > 0) {
    cmdList = g_malloc0(sizeof (gchar *) * (g_list_length (wmdock->dapps) + 1));
    fprintf(stderr,"rcfile.c: Saving dockapps to config file: ");
    
    /* iterate dockapps and add them in order to the list */
    for(dapps = g_list_first(wmdock->dapps) ; dapps; dapps = g_list_next(dapps)) {
      dapp = dapps->data;
        
      if((i = g_list_index(wmdock->dapps, (gconstpointer) dapp)) == -1)
        continue;
      cmdList[i] = dapp->cmd ? g_strdup(dapp->cmd) : NULL;
      fprintf(stderr,"%s;",cmdList[i]);
    }
    fprintf(stderr,"\n");
    xfce_rc_write_list_entry(rc, RCKEY_CMDLIST, cmdList, RC_LIST_DELIMITER);
    g_strfreev(cmdList);
  
  } else {
    fprintf(stderr,"rcfile.c: No dockapps exist, removing commands entry");
    xfce_rc_delete_entry(rc, RCKEY_CMDLIST, FALSE);
  }

  xfce_rc_close(rc);
}
