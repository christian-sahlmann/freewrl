/*
  $Id: common.c,v 1.3 2011/06/18 13:17:10 crc_canada Exp $

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

void setMenuStatus(char *stat)
{
	int loading = FALSE;

/* - JAS - for testing IPHONE
printf ("setMenuStatus, %d %d %d, true %d\n",fwl_isinputThreadParsing(),
fwl_isTextureParsing(),
!fwl_isInputThreadInitialized(),
TRUE);
*/


        if (fwl_isinputThreadParsing() || 
	    fwl_isTextureParsing() || 
	    (!fwl_isInputThreadInitialized())) loading = TRUE;

	if (loading) {
		snprintf(myMenuStatus, sizeof(myMenuStatus),
			 "(Loading...)");
	} else {
		snprintf(myMenuStatus, sizeof(myMenuStatus),
			 "Viewpoint: %s", stat);
	}
}

void setMessageBar()
{
	snprintf(&messagebar[0], 10, "%10f", myFps);
	snprintf(&messagebar[15], sizeof(myMenuStatus)-15, "%s", myMenuStatus);
}
