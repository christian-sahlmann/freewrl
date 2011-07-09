/*
  $Id: common.c,v 1.8 2011/07/09 16:07:43 dug9 Exp $

  FreeWRL support library.

  See common.h.

*/

#include <config.h>
#include <system.h>
#include <libFreeWRL.h>

#include "../ui/common.h"

/* Status variables */

int ccurse = ACURSE;
int ocurse = ACURSE;

float myFps = (float) 0.0;
char myMenuStatus[MAXSTAT];
char messagebar[MAXSTAT];
char window_title[MAXTITLE];

/* Status update functions (generic = all platform) */

void setMenuFps(float fps)
{
	myFps = fps;
	setMessageBar();
}

void setMenuStatus(char *stattext)
{
	int loading = FALSE;

        if (fwl_isinputThreadParsing() || 
	    fwl_isTextureParsing() || 
	    (!fwl_isInputThreadInitialized())) loading = TRUE;

	if (loading) {
		snprintf(myMenuStatus, sizeof(myMenuStatus),
			 "(Loading...)");
	} else {
		snprintf(myMenuStatus, sizeof(myMenuStatus),
			 "Viewpoint: %s", stattext);
	}
}
void setWindowTitle0()
{
	snprintf(window_title, sizeof(window_title), "FreeWRL");
	setWindowTitle();
}
void setMessageBar()
{
	snprintf(&messagebar[0], 10, "%10f", myFps);
	snprintf(&messagebar[15], sizeof(myMenuStatus)-15, "%s", myMenuStatus);
}
char *getMessageBar()
{
	return messagebar;
}
#ifndef _MSC_VER
void setArrowCursor()
{
	if (ocurse != ACURSE) {
		ccurse = ocurse = ACURSE;
		setCursor();
	}
}
void setSensorCursor()
{
	if (ocurse != SCURSE) {
		ccurse = ocurse = SCURSE;
		setCursor();
	}
}
#endif