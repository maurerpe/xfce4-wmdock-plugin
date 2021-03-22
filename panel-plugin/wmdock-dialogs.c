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

#include <string.h>
#include <gtk/gtk.h>

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>

#include "wmdock.h"
#include "wmdock-dialogs.h"

/* the website url */
#define PLUGIN_WEBSITE "https://www.github.com/maurerpe/xfce4-wmdock-plugin"



void
wmdock_about (XfcePanelPlugin *plugin)
{
  /* about dialog code. you can use the GtkAboutDialog
   * or the XfceAboutInfo widget */
  GdkPixbuf *icon;

  const gchar *auth[] =
    {
      "Paul Maurer",
      "Tezeta",
      NULL
    };

  icon = xfce_panel_pixbuf_from_source ("xfce4-wmdock", NULL, 32);
  gtk_show_about_dialog (NULL,
                         "logo",         icon,
                         "license",      xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
                         "version",      PACKAGE_VERSION,
                         "program-name", PACKAGE_NAME,
                         "comments",     _("xfce4 panel plugin that swallows wm dockapps"),
                         "website",      PLUGIN_WEBSITE,
                         "copyright",    _("Copyright \xc2\xa9 2021 Paul Maurer\n"),
                         "authors",      auth,
                         NULL);

  if (icon)
    g_object_unref (G_OBJECT (icon));
}
