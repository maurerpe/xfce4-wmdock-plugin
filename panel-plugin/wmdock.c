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
#include <libxfce4ui/libxfce4ui.h>

#include "wmdock.h"
#include "wmdock-dialogs.h"
#include "dnd.h"
#include "rcfile.h"
#include "misc.h"

#include "tile.xpm"

/* globals */
WmdockPlugin *wmdock 		= NULL;
static GdkPixbuf *tile_pixbuf		= NULL;
static cairo_surface_t *tile_surface	= NULL;
static GtkTargetEntry targetList[] = {
		{ "INTEGER", 0, 0 }
};

static guint nTargets = G_N_ELEMENTS (targetList);

/* prototypes */
static void
wmdock_construct (XfcePanelPlugin *plugin);

static void
wmdock_window_open(WnckScreen   *s,
		   WnckWindow   *w
		   );

/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER (wmdock_construct);

#define DOCKAPP_SIZE 64

static WmdockPlugin *
wmdock_new (XfcePanelPlugin *plugin) {
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
wmdock_free (XfcePanelPlugin *plugin) {
  /* destroy the panel widgets */
  gtk_widget_destroy (wmdock->hvbox);

  /* free the plugin structure */
  g_slice_free (WmdockPlugin, wmdock);
}

static void
wmdock_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation
			    ) {
  /* change the orientation of the box */
  gtk_orientable_set_orientation(GTK_ORIENTABLE(wmdock->hvbox), orientation);
}

static gboolean
wmdock_size_changed (XfcePanelPlugin *plugin,
                     gint             size
                     ) {
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
  WnckHandle *handle;
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
  handle = wnck_handle_new(WNCK_CLIENT_TYPE_APPLICATION);
  screen = wnck_handle_get_screen(handle, 0);
  wmdock_read_rc_file(wmdock);
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

/* event utility functions */
static void update_tile(cairo_t *cr) {
  tile_surface = gdk_cairo_surface_create_from_pixbuf(tile_pixbuf, 0, NULL);
  cairo_set_source_surface(cr, tile_surface, 0, 0);

  cairo_paint(cr);

  cairo_surface_destroy(tile_surface);
}

static void *update_rc_delayed(DockApp *dapp) {
  /* wait a sec before updating the rcfile 
   * workaround for dockapps being removed on logout */
  g_usleep(1 * G_USEC_PER_SEC);
  wmdock_write_rc_file(wmdock);
  return NULL;
}

/* event functions */
void free_dockapp(GtkWidget *widget, DockApp *dapp) {
  fprintf(stderr,"wmdock.c: Remove %s\n",dapp->name);

  /* remove dockapp from list */
  wmdock->dapps = g_list_remove_all(wmdock->dapps, dapp);

  /* attempt to close the dockapp in case it's still running */
  wnck_window_close(dapp->window, gtk_get_current_event_time());
 
  gtk_widget_destroy(GTK_WIDGET(dapp->tile));

  /* use a new thread to save the list of dockapps in background */
  g_thread_try_new(NULL, (GThreadFunc)update_rc_delayed, wmdock, NULL);
  free(dapp);
}

static gboolean init_tile(GtkWidget *widget, cairo_t *cr)
{
  update_tile(cr);
  return FALSE;
}

/* init functions */
int
is_dockapp(WnckWindow *w) {
  int xpos, ypos, width, height;
  const char *name;
  
  if ((name = wnck_window_get_name(w)) == NULL ||
      (strncasecmp(name, "wm", 2) != 0 &&
       strncasecmp(name, "as", 2) != 0))
    return 0;
  
  wnck_window_get_client_window_geometry(w, &xpos, &ypos, &width, &height);
  //some dockapps don't have 64x64 geometry (wmclock), if dockapps are smaller than 64px allow them
  if (height > DOCKAPP_SIZE || width > DOCKAPP_SIZE)
      return 0;
  
  return 1;
}

static void setup_dnd(DockApp *dapp, void *user_data)
{
  gtk_drag_dest_set (GTK_WIDGET(dapp->sock), GTK_DEST_DEFAULT_MOTION, targetList,
    nTargets, GDK_ACTION_MOVE);

  gtk_drag_source_set (GTK_WIDGET(dapp->sock), GDK_BUTTON1_MASK, targetList,
    nTargets, GDK_ACTION_MOVE);

  g_signal_connect (dapp->sock, "drag-begin", G_CALLBACK (drag_begin_handl), dapp);
  g_signal_connect (dapp->sock, "drag-data-get", G_CALLBACK (drag_data_get_handl), dapp);
  g_signal_connect (dapp->sock, "drag-data-received", G_CALLBACK(drag_data_received_handl), dapp);
  g_signal_connect (dapp->sock, "drag-drop", G_CALLBACK (drag_drop_handl), dapp);
  g_signal_connect (dapp->sock, "drag-failed", G_CALLBACK (drag_failed_handl), dapp);
}

static GtkWidget *tile_from_sock(DockApp *dapp) {
  GtkWidget *_tile = gtk_fixed_new();

  gtk_widget_set_size_request(dapp->sock, dapp->width, dapp->height);

  /* center dockapps that aren't 64x64 */
  gtk_fixed_put(GTK_FIXED(_tile),dapp->sock, (DOCKAPP_SIZE-dapp->width)/2, (DOCKAPP_SIZE-dapp->height)/2);

  /* setup tile image */
  tile_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**) tile_xpm);

  /* apply tile to sock and tile bg */
  g_signal_connect(G_OBJECT(dapp->sock), "draw", G_CALLBACK(init_tile), NULL);
  g_signal_connect(G_OBJECT(_tile), "draw", G_CALLBACK(init_tile), NULL);

  g_signal_connect(G_OBJECT(dapp->sock), "plug-removed", G_CALLBACK(free_dockapp), dapp);

  gtk_widget_show_all(_tile);

  return _tile;
}

int
dockapp_new(WnckWindow *w) {
 DockApp *dapp;
  
  if ((dapp = malloc(sizeof(*dapp))) == NULL)
    goto err;
 
  /* keep the window for later use */
  dapp->window = w;

  dapp->name = wnck_window_get_name(dapp->window);
  dapp->id = wnck_window_get_xid(dapp->window);
  dapp->cmd = wmdock_get_dockapp_cmd(dapp->window);

  if ((dapp->sock = gtk_socket_new()) == NULL)
    goto err2;
  
  wnck_window_get_client_window_geometry(dapp->window, &dapp->xpos, &dapp->ypos, &dapp->width, &dapp->height);

  dapp->tile = tile_from_sock(dapp);
  gtk_widget_set_size_request(dapp->tile, DOCKAPP_SIZE, DOCKAPP_SIZE);

  gtk_box_pack_start(GTK_BOX(wmdock->hvbox), dapp->tile, FALSE, FALSE, 0);

  wnck_window_stick(dapp->window);
  wnck_window_set_skip_tasklist(dapp->window, TRUE);
  wnck_window_set_skip_pager(dapp->window, TRUE);
  wnck_window_set_window_type(dapp->window, WNCK_WINDOW_DOCK);

  wnck_window_minimize(dapp->window);
  wmdock->dapps = g_list_append(wmdock->dapps, dapp);
  gtk_socket_add_id(GTK_SOCKET(dapp->sock), dapp->id);
  g_list_foreach(wmdock->dapps, (GFunc) setup_dnd, NULL);
 
  return 0;
  
 err2:
  free(dapp);
 err:
  return -1;
}

static void
wmdock_window_open(WnckScreen   *s,
		   WnckWindow   *w
		   ) {
  gdk_display_flush(gdk_display_get_default());
  
  if (!is_dockapp(w))
    return;
  
  fprintf(stderr, "wmdock.c: Found dockapp: %s\n", wnck_window_get_name(w));

  dockapp_new(w);
  wmdock_write_rc_file(wmdock);
}
