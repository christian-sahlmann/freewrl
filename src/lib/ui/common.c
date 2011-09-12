/*
  $Id: common.c,v 1.11 2011/09/12 17:28:44 crc_canada Exp $

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
void setArrowCursor()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	p->cursorStyle = ACURSE;

	//if (ocurse != ACURSE) {
	//	ccurse = ocurse = ACURSE;
	//	setCursor();
	//}
}
void setSensorCursor()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	p->cursorStyle = SCURSE;

	//if (ocurse != SCURSE) {
	//	ccurse = ocurse = SCURSE;
	//	setCursor();
	//}
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
	setCursor(); /*updateCursorStyle0(cstyle); /* in fwWindow32 where cursors are loaded */
#endif
}
