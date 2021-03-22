/* Copyright (C) 2021 Paul Maurner
 * dnd.c port (C) 2021 tezeta, original by Andre Ellguth
 * Drag'n'Drop functions
 * 
 * License:
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this package; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>

#include "wmdock.h"
#include "misc.h"
#include "dnd.h"

#define _BYTE 8
#define DOCKAPP_SIZE 64

extern WmdockPlugin *wmdock;

void drag_begin_handl (GtkWidget *widget, GdkDragContext *context,
    gpointer dapp)
{
  GdkPixbuf *gdkPb = NULL, *gdkPbScaled = NULL;
  gint width = 0, height = 0;

  gtk_widget_get_size_request(GTK_WIDGET(DOCKAPP(dapp)->sock), &width, &height);

  if((gdkPb = gdk_pixbuf_get_from_window (gtk_widget_get_window(GTK_WIDGET(DOCKAPP(dapp)->sock)),
          0, 0, width, height))) {
    gdkPbScaled = gdk_pixbuf_scale_simple(gdkPb, DOCKAPP_SIZE / 2, DOCKAPP_SIZE / 2, GDK_INTERP_BILINEAR);
    gtk_drag_set_icon_pixbuf (context, gdkPbScaled ? gdkPbScaled : gdkPb, 0, 0);

    g_object_unref (G_OBJECT(gdkPb));
    g_object_unref (G_OBJECT(gdkPbScaled));
  }
}

gboolean drag_failed_handl(GtkWidget *widget, GdkDragContext *context,
    GtkDragResult result, gpointer dapp)
{
  if(result == GTK_DRAG_RESULT_NO_TARGET && dapp) {
    fprintf(stderr,"dnd.c: dockapp removal requested: %s\n",DOCKAPP(dapp)->name);
    free_dockapp(DOCKAPP(dapp)->sock,(DockApp *) dapp);
  }
  return TRUE;
}

gboolean drag_drop_handl (GtkWidget *widget, GdkDragContext *context,
    gint x, gint y, guint time, gpointer dapp)
{
  gboolean        is_valid_drop_site;
  GdkAtom         target_type;

  is_valid_drop_site = TRUE;

  if (gdk_drag_context_list_targets(context))
  {
    target_type = GDK_POINTER_TO_ATOM
      (g_list_nth_data (gdk_drag_context_list_targets(context), 0));

    gtk_drag_get_data (widget,context, target_type, time);
  }

  else
  {
    is_valid_drop_site = FALSE;
  }

  return  is_valid_drop_site;
}



void drag_data_received_handl (GtkWidget *widget,
    GdkDragContext *context, gint x, gint y,
    GtkSelectionData *selection_data,
    guint target_type, guint time,
    gpointer dapp)
{
  glong *_idata;
  gboolean dnd_success = FALSE;
  GList *dappsSrc = NULL;
  GList *dappsDst = NULL;

  if(target_type == 0) {
    _idata = (glong*) gtk_selection_data_get_data(selection_data);

    dnd_success = TRUE;

    if(dapp) {
      dappsSrc = g_list_nth(wmdock->dapps, *_idata);
      dappsDst = g_list_find(wmdock->dapps, (DockApp *) dapp);

      if(dappsSrc->data != dappsDst->data) {
        dappsDst->data = dappsSrc->data;
        dappsSrc->data = dapp;

        gtk_box_reorder_child(GTK_BOX(wmdock->hvbox),
            GTK_WIDGET(DOCKAPP(dappsSrc->data)->tile),
            g_list_index (wmdock->dapps, dappsSrc->data));
        gtk_box_reorder_child(GTK_BOX(wmdock->hvbox),
            GTK_WIDGET(DOCKAPP(dappsDst->data)->tile),
            g_list_index (wmdock->dapps, dappsDst->data));
        fprintf(stderr,"dnd.c: dockapp reorder completed\n");
        wmdock_write_rc_file(wmdock);

      }

    }

  }
  gtk_drag_finish (context, dnd_success, FALSE, time);

}



void drag_data_get_handl (GtkWidget *widget, GdkDragContext *context,
    GtkSelectionData *selection_data,
    guint target_type, guint time,
    gpointer dapp)
{
  gint index;

  if(target_type == 0 && dapp) {
    index = g_list_index (wmdock->dapps, (DockApp *) dapp);

    gtk_selection_data_set (selection_data, gtk_selection_data_get_target(selection_data),
        sizeof(index) * _BYTE,
        (guchar*) &index, sizeof (index));
  }
}
