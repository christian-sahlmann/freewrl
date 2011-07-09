/*
  $Id: common.h,v 1.3 2011/07/09 01:06:01 dug9 Exp $

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

//#define SENSOR_CURSOR ccurse = SCURSE
//#define ARROW_CURSOR  ccurse = ACURSE

/* Status variables */

//extern int ccurse;
//extern int ocurse;

//extern float myFps;
#define MAXSTAT 200
//extern char myMenuStatus[MAXSTAT];
//extern char messagebar[MAXSTAT];
#define MAXTITLE 200
//extern char window_title[MAXTITLE];

/* Status update functions */

void setMenuFps(float fps);
void setMenuStatus(char *stat);
void setMessageBar();

/* Generic (virtual) update functions */

void setSensorCursor();
void setArrowCursor();
void setWindowTitle0();
void setMessageBar0();
//legacy apps: these virtual functions rely on static variables
void setCursor();
void setWindowTitle();



#endif /* __LIBFREEWRL_UI_COMMON_H__ */
