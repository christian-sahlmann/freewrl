// dllFreeWRL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "dllFreeWRL.h"

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <wtypes.h>
static HANDLE hStdErr = NULL;

static void
swInitConsole(void)
{
    BOOL ac = AllocConsole();
    if (ac)
        hStdErr = GetStdHandle(STD_ERROR_HANDLE);
}
void swDebugf(LPCSTR formatstring, ...)
{
    char buff[2500];
    DWORD cWritten;
	int nSize;
   memset(buff, 0, sizeof(buff));
   va_list args;
   va_start(args, formatstring);
   
   nSize = _vsnprintf( buff, sizeof(buff) - 1, formatstring, args); // C4996
    if (hStdErr == NULL)
        swInitConsole(); 
    //_vsnprintf(buff, BUFSIZE, fmt, ap);
    //OutputDebugStringA(buff);

    /* not C console - more low level windows SDK API */
    WriteConsoleA(hStdErr, buff, strlen(buff),&cWritten, NULL);
}

extern "C"
{
#include "libFreeWRL.h"
//#include <main/headers.h>
void handle_aqua(const int mev, const unsigned int button, int x, int y);
void do_keyPress(const char kp, int type);
void initializeRenderSceneUpdateScene();
void RenderSceneUpdateScene();
void finalizeRenderSceneUpdateScene();
void resize_GL(int width, int height);
void setScreenDim(int wi, int he);
void closeFreeWRL(void);
void resource_push_single_request(const char *request);
void initConsoleH(DWORD pid);
extern int Console_writePrimitive;
//extern int screenWidth; /*used in mainloop render_pre setup_projection*/
//extern int screenHeight;

}

#include <malloc.h>
#include <WinUser.h>


// This is an example of an exported variable
DLLFREEWRL_API int ndllFreeWRL=0;

// This is an example of an exported function.
DLLFREEWRL_API int fndllFreeWRL(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see dllFreeWRL.h for the class definition
CdllFreeWRL::CdllFreeWRL()
{
	return;
}
	//enum class KeyAction {KEYDOWN,KEYUP,KEYPRESS};
	//enum class MouseAction {MOUSEMOVE,MOUSEDOWN,MOUSEUP};
	//enum class MouseButton {LEFT,MIDDLE,RIGHT,NONE};

void CdllFreeWRL::onInit(unsigned long handle,int width, int height){
	freewrl_params_t *params;
	//struct freewrl_params *params;
	//_putenv("FREEWRL_NO_VBOS=1"); 
	/* Before we parse the command line, setup the FreeWRL default parameters */
	params = (freewrl_params_t*) malloc( sizeof(freewrl_params_t));

	//params = (struct freewrl_params*) malloc( sizeof(struct freewrl_params));
	/* Default values */
	params->width = width; //600;
	params->height = height; //400;
	params->eai = 0;
	params->fullscreen = 0;
	params->winToEmbedInto = (int)handle;
	swDebugf("Hi just before initFreeWRL\n");
	Console_writePrimitive = 1;
	DWORD pid = GetCurrentProcessId() ;
	initConsoleH(pid);

	ConsoleMessage("do consoleMessages come out\n");
	swDebugf("sure");
	if (!initFreeWRL(params)) {
		//ERROR_MSG("main: aborting during initialization.\n");
		//exit(1);
	}
	swDebugf("after initFreeWRL\n");
	//initStereoDefaults();
	//startFreeWRL("C:\\source2\\tests\\1.wrl");
	//swDebugf("after startFreeWRL\n");
	initializeRenderSceneUpdateScene();
	//swDebugf("after initializeRenderSceneUpdateScene\n");
	resource_push_single_request("C:\\source2\\tests\\1.wrl");

}
void CdllFreeWRL::onResize(int width,int height){
	//screenWidth = width; /*used in mainloop render_pre setup_projection*/
	//screenHeight = height;
	//resize_GL(width, height); 
	setScreenDim(width,height);
	//ConsoleMessage("dll_onResize w=%d h=%d\n",width,height);
}
//#define KeyChar         1
//#if defined(AQUA) || defined(WIN32)
//#define KeyPress        2
//#define KeyRelease      3
//#define ButtonPress     4
//#define ButtonRelease   5
//#define MotionNotify    6
//#define MapNotify       19
//#endif

void CdllFreeWRL::onMouse(int mouseAction,int mouseButton,int x, int y){

	/*void handle_aqua(const int mev, const unsigned int button, int x, int y);*/
	/* butnum=1 left butnum=3 right (butnum=2 middle, not used by freewrl) */
	//handle_aqua(mev,butnum,mouseX,mouseY); 
	handle_aqua(mouseAction,mouseButton,x,y); 
	//ConsoleMessage("%d %d %d %d\r",mouseAction,mouseButton,x,y);
}
void CdllFreeWRL::onKey(int keyAction,int keyValue){

	int kp = keyValue;
	int ka = keyAction;
	switch(keyAction)
	{
	case KEYDOWN:
		if(kp & 1 << 30) 
			break; //ignor - its an auto-repeat
	case KEYUP: 
		switch (kp) 
		{ 
			case VK_OEM_1:
				kp = ';'; //could be : or ; but tolower won't lowercase it, but returns same character if it can't
				break;
			default:
				break;
		}
		do_keyPress(kp, ka); 
		break; 

	case KEYPRESS: //WM_CHAR:
		do_keyPress(kp,ka);
		break;
	}
}
void CdllFreeWRL::onTick(int interval){
	RenderSceneUpdateScene();

}
void CdllFreeWRL::onClose()
{
    
	/* when finished: */
	//finalizeRenderSceneUpdateScene(); //doesn't like killoldworld

	closeFreeWRL();
	//terminateFreeWRL();
}
