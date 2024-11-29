/*  Copyright (C) 2021 Paul Maurer
 *  rcfile.h (C) 2021 tezeta
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

#ifndef __RCFILE_H__
#define __RCFILE_H__

G_BEGIN_DECLS

#define RCKEY_CMDLIST                  (const gchar *) "cmds"
#define RC_LIST_DELIMITER   (const gchar *) ";"

#define RCKEY_WRITECFG (const gchar *) "writeCfg"

/* Prototypes */
void wmdock_read_rc_file (WmdockPlugin *);
void wmdock_write_rc_file (WmdockPlugin *);

G_END_DECLS

#endif /* __RCFILE_H__ */
