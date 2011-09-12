/*
  $Id: common.h,v 1.8 2011/09/12 17:28:44 crc_canada Exp $

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

//extern float myFps;
#define MAXSTAT 200
//extern char myMenuStatus[MAXSTAT];
//extern char messagebar[MAXSTAT];
#define MAXTITLE 200
//extern char window_title[MAXTITLE];

/* textual status messages */
typedef struct pcommon{
        float myFps; // = (float) 0.0;
        char myMenuStatus[MAXSTAT];
        char messagebar[MAXSTAT];
        char window_title[MAXTITLE];
        int cursorStyle;
}* ppcommon;

/* Status update functions */

void setMenuFps(float fps);
void setMenuStatus(char *stat);
void setMessageBar();

/* Generic (virtual) update functions */

void setCursor();
void setArrowCursor();
void setSensorCursor();
void setWindowTitle0();
void setWindowTitle();
char *getMessageBar();
char *getWindowTitle();
void updateCursorStyle();

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#endif /* __LIBFREEWRL_UI_COMMON_H__ */
