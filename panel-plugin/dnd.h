/* Copyright (C) 2021 Paul Maurner
 * dnd.h port (C) 2021 tezeta, original by Andre Ellguth
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

#ifndef __DND_H__
#define __DND_H__

G_BEGIN_DECLS

void drag_begin_handl (GtkWidget *, GdkDragContext *, gpointer);
gboolean drag_drop_handl (GtkWidget *, GdkDragContext *, gint, gint, guint, gpointer);
void drag_data_received_handl (GtkWidget *, GdkDragContext *, gint, gint, GtkSelectionData *, guint, guint, gpointer);
void drag_data_get_handl (GtkWidget *, GdkDragContext *, GtkSelectionData *, guint, guint, gpointer);
gboolean drag_failed_handl(GtkWidget *, GdkDragContext *, GtkDragResult, gpointer);

G_END_DECLS

#endif /* __DND_H__ */
