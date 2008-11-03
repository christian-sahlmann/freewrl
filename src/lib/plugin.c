/*******************************************************************
 *
 * FreeX3D support library
 *
 * plugin.c
 *
 * $Id: plugin.c,v 1.1 2008/11/03 14:14:12 couannette Exp $
 *
 *******************************************************************/

#include "config.h"
#include "system.h"
#include "display.h"
#include "internal.h"


int isBrowserPlugin = FALSE; /* are we running as plugin child ? */
int _fw_pipe = 0; /* pipe to communicate with plugin */
int _fw_browser_plugin = 0; /* socket to communicate with plugin */
unsigned _fw_instance = 0; /* ??? */
