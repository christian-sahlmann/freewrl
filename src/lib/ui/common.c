/*
  $Id: common.c,v 1.5 2011/07/09 01:06:01 dug9 Exp $

  FreeWRL support library.

  See common.h.

*/

#include <config.h>
#include <system.h>
#include <libFreeWRL.h>
#include <iglobal.h>
#include "../ui/common.h"

/* Status variables */


//these public static are just for legacy applications using COMMON_OPTIONA
int ccurse = ACURSE;
int ocurse = ACURSE;
//float myFps = (float) 0.0;
char myMenuStatus[MAXSTAT];
char messagebar[MAXSTAT];
char window_title[MAXTITLE];

//these are for all 3 options
typedef struct pcommon{
	int ccurse;// = ACURSE;
	int ocurse;// = ACURSE;

	float myFps;// = (float) 0.0;
	char myMenuStatus[MAXSTAT];
	char messagebar[MAXSTAT];
	char window_title[MAXTITLE];
	void (*cccfn)(int icType);
	void (*wtcfn)(char *title);
	void (*mbcfn)(char *status);

}* ppcommon;
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
		p->ccurse = ACURSE;
		p->ocurse = ACURSE;
		p->myFps = (float) 0.0;
		p->myMenuStatus[0] = '\0';
		p->cccfn = NULL;
		p->mbcfn = NULL;
		p->wtcfn = NULL;
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
void setMenuStatus0(char *stat0);
void setWindowTitle00(char *title);
void setMenuStatus(char *stat)
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
			 "Viewpoint: %s", stat);
	}
	setMessageBar();
}

void setMessageBar()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	snprintf(&p->messagebar[0], 10, "%10f", p->myFps);
	snprintf(&p->messagebar[15], sizeof(p->myMenuStatus)-15, "%s", p->myMenuStatus);
	setMessageBar0(p->messagebar);
}
#ifdef _MSC_VER
#define COMMON_OPTIONB
#else
#define COMMON_OPTIONA
#endif

#ifdef COMMON_OPTIONA
//Option A, define these in your statically linked C app:
//extern char *myMenuStatus; // use this directly
//extern char *window_title;
//extern int ocurse;
//extern int ccurse;
//and functions:
//setCursor(){}  // use ccurse/ocurse in here
//setWindowTitle(){} // use window_title in here
void setWindowTitle0(char *title)
{
	strcpy(window_title,title);
	setWindowTitle();
}

void setMessageBar0(char *stat0)
{
	strcpy(messagebar,stat0);
}

void setSensorCursor()
{
	ocurse = ccurse;
	ccurse = SCURSE;
	setCursor();
}
void setArrowCursor()
{
	//legacy apps
	ocurse = ccurse;
	ccurse = ACURSE;
	setCursor();
}
#endif

#ifdef COMMON_OPTIONB
//Option B, define 3 functions in your statically linked C app:
//setSensorCursor(){}
//setArrowCursor(){}
//setMessageBar0(char *status){}
//and to get the library's window title call the following from 
//the same thread you initialize with
//char * fwl_getWindowTitle();
#endif
#if defined(COMMON_OPTIONB) || defined(COMMON_OPTIONC)
void setWindowTitle0(){}
char *fwl_getWindowTitle()
{
	return "FreeWRL";
}
#endif

#ifdef COMMON_OPTIONC
//not tested as of July 8, 2011
//Option C, define C functions in you're dynamically or statically linked C app:
//setfwCursor(int ictype){}
//setfwMessageBar(char *status){}
//then (after some initialization, so gglobal is set) 
//call the following and pass the functions in as callbacks:
//fwl_setCursorChangeCallback(setfwCursor);
//fwl_setMessageBarCallback(setfwMessageBar);

void fwl_setCursorChangeCallback( void (*cccfn)(int icType)  )
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	p->cccfn = cccfn;
}
void fwl_setMessageBarCallback( void (*mbcfn)(char *status) )
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	p->mbcfn = mbcfn;
}
void setSensorCursor()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	if(p->cccfn)
		p->cccfn(1);
}
void setArrowCursor()
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	if(p->cccfn)
		p->cccfn(0);
}
void setMessageBar0(char *stat0)
{
	ppcommon p = (ppcommon)gglobal()->common.prv;
	if(p->mbcfn)
		p->mbcfn(p->messagebar);
}
#endif