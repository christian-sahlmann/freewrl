/*
  $Id: common.h,v 1.2 2011/06/10 22:28:33 dug9 Exp $

  FreeWRL support library.

Purpose:
  Common UI for all platforms.

Data:
  Handle internal FreeWRL library variables related to UI.

Functions:
  Update internal FreeWRL library variables related to UI.
  NO PLATFORM SPECIFIC CODE HERE. ALL GENERIC CODE.

*/

#ifndef __LIBFREEWRL_UI_COMMON_H__
#define __LIBFREEWRL_UI_COMMON_H__


/* Generic declarations */

#define SCURSE 1
#define ACURSE 0

#define SENSOR_CURSOR ccurse = SCURSE
#define ARROW_CURSOR  ccurse = ACURSE

/* Status variables */

extern int ccurse;
extern int ocurse;

extern float myFps;
#define MAXSTAT 200
extern char myMenuStatus[MAXSTAT];
extern char messagebar[MAXSTAT];
#define MAXTITLE 200
extern char window_title[MAXTITLE];

/* Status update functions */

void setMenuFps(float fps);
void setMenuStatus(char *stat);
void setMessageBar();

/* Generic (virtual) update functions */

void setCursor();
void setWindowTitle();

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#endif /* __LIBFREEWRL_UI_COMMON_H__ */
