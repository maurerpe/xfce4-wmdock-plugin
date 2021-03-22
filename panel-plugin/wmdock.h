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

#ifndef __WMDOCK_H__
#define __WMDOCK_H__

G_BEGIN_DECLS

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

/* Dockapp item */
typedef struct {
  GtkWidget     *sock;
  GtkWidget     *tile;
  WnckWindow	*window;
  unsigned long  id;
  const char          *name;
  const char		*cmd;

  int           xpos;
  int           ypos;
  int           width;
  int           height;
} DockApp;

/* plugin structure */
typedef struct {
  XfcePanelPlugin *plugin;
  
  /* panel widgets */
  GtkWidget       *ebox;
  GtkWidget       *hvbox;
  
  GList           *dapps;
} WmdockPlugin;

int is_dockapp(WnckWindow *);
int dockapp_new(WnckWindow *);
void free_dockapp(GtkWidget *, DockApp *);
#define DOCKAPP(__dapp) ((DockApp *) __dapp)

G_END_DECLS

#endif /* !__WMDOCK_H__ */
