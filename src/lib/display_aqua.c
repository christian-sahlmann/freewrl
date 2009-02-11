/*******************************************************************
 *
 * FreeWRL support library
 *
 * display_aqua.c
 *
 * $Id: display_aqua.c,v 1.3 2009/02/11 15:12:54 istakenv Exp $
 *
 *******************************************************************/

#include "config.h"
#include "system.h"
#include "display.h"
#include "internal.h"

/* display part specific to Mac */

CGLContextObj myglobalContext;
AGLContext aqglobalContext;

GLboolean cErr;

GDHandle gGDevice;

int ccurse = ACURSE;
int ocurse = ACURSE;

/* for handling Safari window changes at the top of the display event loop */
int PaneClipnpx;
int PaneClipnpy;
WindowPtr PaneClipfwWindow;
int PaneClipct;
int PaneClipcb;
int PaneClipcr;
int PaneClipcl;
int PaneClipwidth;
int PaneClipheight;
int PaneClipChanged = FALSE;

int create_main_window_aqua()
{
    return FALSE;
}
