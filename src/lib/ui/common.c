/*
  $Id: common.c,v 1.14 2012/07/08 19:11:45 dug9 Exp $

  FreeWRL support library.

  See common.h.

*/

#include <config.h>
#include <system.h>
#include <libFreeWRL.h>
#include <iglobal.h>
#include "../ui/common.h"

/* Status variables */
/* cursors are a 'shared resource' meanng you only need one cursor for n windows,
  not per-instance cursors (except multi-touch multi-cursors)
  However cursor style choice could/should be per-window/instance
*/
int ccurse = ACURSE;
int ocurse = ACURSE;

/* typedef struct pcommon{
	float myFps; // = (float) 0.0;
	char myMenuStatus[MAXSTAT];
	char messagebar[MAXSTAT];
	char window_title[MAXTITLE];
	int cursorStyle;
}* ppcommon;
*/

void *common_constructor(){
	void *v = malloc(sizeof(struct pcommon));
	memset(v,0,sizeof(struct pcommon));
	return v;
}
void common_init(struct tcommon *t){
	//public
	//private
	t->prv = common_constructor();
	{
		ppcommon p = (ppcommon)t->prv;
		p->myFps = (float) 0.0;
		//char myMenuStatus[MAXSTAT];
		//char messagebar[MAXSTAT];
		//char window_title[MAXTITLE];
		p->cursorStyle = ACURSE;
		p->promptForURL = 0;
		p->promptForFile = 0;
	}
}
//ppcommon p = (ppcommon)gglobal()->common.prv;

/* Status update functions (generic = all platform) */

void setMenuFps(float fps)
{
	ppcommon p = (ppcommon)gglobal()->common.prv;

	p->myFps = fps;
	setMessageBar();
}

void setMenuStatus(char *stattext)
{
	int loading = FALSE;
	ppcommon p = (ppcommon)gglobal()->common.prv;

        if (fwl_isinputThreadParsing() || 
	    fwl_isTextureParsing() || 
	    (!fwl_isInputThreadInitialized())) loading = TRUE;

	if (loading) {
		snprintf(p->myMenuStatus, sizeof(p->myMenuStatus),
			 "(Loading...)");
	} else {
		snprintf(p->myMenuStatus, sizeof(p->myMenuStatus),
			 "Viewpoint: %s", stattext);
	}
}

#if !defined (_ANDROID)

void setWindowTitle0()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;

	snprintf(p->window_title, sizeof(p->window_title), "FreeWRL");
	setWindowTitle();
}
char *getWindowTitle()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	return p->window_title;
}
#endif //ANDROID

void setMessageBar()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;

	snprintf(&p->messagebar[0], 10, "%10.0f", p->myFps);
	snprintf(&p->messagebar[15], sizeof(p->myMenuStatus)-15, "%s", p->myMenuStatus);
}
char *getMessageBar()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	return p->messagebar;
}
/* the next 4 functions allow statusbarHud to set a flag 
   to indicate the frontend should pop up a dialog to prompt for
   something
*/
int fwl_pollPromptForURL()
{ /* poll from front end / UI in loop */
	ppcommon p = (ppcommon)gglobal()->common.prv;
	return p->promptForURL;
}
int fwl_pollPromptForFile()
{ 
	ppcommon p = (ppcommon)gglobal()->common.prv;
	return p->promptForFile;
}
void fwl_setPromptForURL(int state)
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	p->promptForURL = state; //1 or 0
}
void fwl_setPromptForFile(int state)
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	p->promptForFile = state; //1 or 0
}


void setArrowCursor()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	p->cursorStyle = ACURSE;
}
void setSensorCursor()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	p->cursorStyle = SCURSE;
}
void updateCursorStyle0(int cstyle);
void updateCursorStyle()
{
	/* Multi-window apps - there's only one mouse cursor -it's a shared resource.
	   But the cursor style is owned by each window/freewrl iglobal instance.
	   How does a window know which global instance to ask for style?
	   Simple: when a window gets mouse events, that means the mouse is in 
	   that window. So get the iglobal that goes with that window, and ask 
	   it for the cursor style.
		So a good place to call this updateCursorStyle is near/after 
		freewrl gets a mouse event, like handle_aqua or handle_mev.
	 */
	int cstyle;
	ppcommon p = (ppcommon)gglobal()->common.prv;

	cstyle = p->cursorStyle;
#ifdef _MSC_VER
	updateCursorStyle0(cstyle); /* in fwWindow32 where cursors are loaded */
#else

	ccurse = ocurse = cstyle;

#if !defined (_ANDROID) 
	/* ANDROID - no cursor style right now */
	setCursor(); /*updateCursorStyle0(cstyle); // in fwWindow32 where cursors are loaded */
#endif //ANDROID
#endif
}
