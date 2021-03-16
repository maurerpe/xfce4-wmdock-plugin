#ifndef __RCFILE_H__
#define __RCFILE_H__

#include "wmdock.h"

#define RCKEY_CMDLIST                  (const gchar *) "cmds"
#define RC_LIST_DELIMITER   (const gchar *) ";"

/* Prototypes */
void wmdock_read_rc_file (WmdockPlugin *);
void wmdock_write_rc_file (WmdockPlugin *);

#endif /* __RCFILE_H__ */
