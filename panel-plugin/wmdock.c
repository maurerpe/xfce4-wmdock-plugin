/*  Copyright (C) 2021 Paul Maurer
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <stdint.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>

#include "wmdock.h"
#include "wmdock-dialogs.h"

/* prototypes */
static void
wmdock_construct (XfcePanelPlugin *plugin);

static void
wmdock_window_open(WnckScreen   *s,
		   WnckWindow   *w,
		   WmdockPlugin *wmdock);

/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER (wmdock_construct);

#define DOCKAPP_SIZE 64

static WmdockPlugin *
wmdock_new (XfcePanelPlugin *plugin) {
  WmdockPlugin   *wmdock;
  GtkOrientation  orientation;

  /* allocate memory for the plugin structure */
  wmdock = g_slice_new0 (WmdockPlugin);

  /* pointer to plugin */
  wmdock->plugin = plugin;

  /* get the current orientation */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* create some panel widgets */
  wmdock->ebox = gtk_event_box_new ();
  gtk_widget_show (wmdock->ebox);

  wmdock->hvbox = gtk_box_new (orientation, 1);
  gtk_widget_show (wmdock->hvbox);
  gtk_container_add (GTK_CONTAINER (wmdock->ebox), wmdock->hvbox);

  return wmdock;
}

static void
wmdock_free (XfcePanelPlugin *plugin,
             WmdockPlugin    *wmdock) {
  /* destroy the panel widgets */
  gtk_widget_destroy (wmdock->hvbox);

  /* free the plugin structure */
  g_slice_free (WmdockPlugin, wmdock);
}

static void
wmdock_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            WmdockPlugin    *wmdock) {
  /* change the orientation of the box */
  gtk_orientable_set_orientation(GTK_ORIENTABLE(wmdock->hvbox), orientation);
}

static gboolean
wmdock_size_changed (XfcePanelPlugin *plugin,
                     gint             size,
                     WmdockPlugin    *wmdock) {
  GtkOrientation orientation;

  /* get the orientation of the plugin */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* set the widget size */
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
  else
    gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);

  /* we handled the orientation */
  return TRUE;
}

static void
wmdock_construct (XfcePanelPlugin *plugin) {
  WmdockPlugin *wmdock;
  WnckScreen *screen;

  /* setup transation domain */
  xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* create the plugin */
  wmdock = wmdock_new (plugin);

  /* add the ebox to the panel */
  gtk_container_add (GTK_CONTAINER (plugin), wmdock->ebox);

  /* show the panel's right-click menu on this ebox */
  xfce_panel_plugin_add_action_widget (plugin, wmdock->ebox);
  
  /* connect plugin signals */
  screen = wnck_screen_get(0);
  g_signal_connect (screen, "window_opened",
		    G_CALLBACK(wmdock_window_open), wmdock);
  g_signal_connect (G_OBJECT (plugin), "free-data",
                    G_CALLBACK (wmdock_free), wmdock);

  g_signal_connect (G_OBJECT (plugin), "size-changed",
                    G_CALLBACK (wmdock_size_changed), wmdock);

  g_signal_connect (G_OBJECT (plugin), "orientation-changed",
                    G_CALLBACK (wmdock_orientation_changed), wmdock);

  /* show the about menu item and connect signal */
  xfce_panel_plugin_menu_show_about (plugin);
  g_signal_connect (G_OBJECT (plugin), "about",
                    G_CALLBACK (wmdock_about), NULL);
}

static int
is_dockapp(WnckWindow *w) {
  int xpos, ypos, width, height;
  const char *name;
  
  if ((name = wnck_window_get_name(w)) == NULL ||
      (strncasecmp(name, "wm", 2) != 0 &&
       strncasecmp(name, "as", 2) != 0))
    return 0;
  
  wnck_window_get_client_window_geometry(w, &xpos, &ypos, &width, &height);
  if (height != DOCKAPP_SIZE || width != DOCKAPP_SIZE)
      return 0;
  
  return 1;
}

static int
dockapp_new(WmdockPlugin *wmdock, WnckWindow *w) {
  DockApp *dapp;
  
  if ((dapp = malloc(sizeof(*dapp))) == NULL)
    goto err;
  
  dapp->id = wnck_window_get_xid(w);
  
  if ((dapp->sock = gtk_socket_new()) == NULL)
    goto err2;
  gtk_widget_set_size_request(dapp->sock, DOCKAPP_SIZE, DOCKAPP_SIZE);
  gtk_box_pack_start(GTK_BOX(wmdock->hvbox), dapp->sock, FALSE, FALSE, 0);
  gtk_socket_add_id(GTK_SOCKET(dapp->sock), dapp->id);
  gtk_widget_show_all(dapp->sock);
  
  wnck_window_stick(w);
  wnck_window_set_skip_tasklist(w, TRUE);
  wnck_window_set_skip_pager(w, TRUE);
  
  wnck_window_minimize(w);
  wmdock->dapps = g_list_append(wmdock->dapps, dapp);

  return 0;
  
 err2:
  free(dapp);
 err:
  return -1;
}

static void
wmdock_window_open(WnckScreen   *s,
		   WnckWindow   *w,
		   WmdockPlugin *wmdock) {
  gdk_display_flush(gdk_display_get_default());
  
  if (!is_dockapp(w))
    return;
  
  fprintf(stderr, "Found dockapp: %s\n", wnck_window_get_name(w));
  
  dockapp_new(wmdock, w);
}
