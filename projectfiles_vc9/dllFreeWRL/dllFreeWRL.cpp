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
void fwl_handle_aqua(const int mev, const unsigned int button, int x, int y);
//void do_keyPress(const char kp, int type);
//void initializeRenderSceneUpdateScene();
void fwl_initializeRenderSceneUpdateScene();
//void fwl_RenderSceneUpdateScene();
void finalizeRenderSceneUpdateScene();
void resize_GL(int width, int height);
void fwl_setScreenDim(int wi, int he);
void closeFreeWRL(void);
void fwl_resource_push_single_request(const char *request);
void initConsoleH(DWORD pid);
extern int Console_writePrimitive;
char *strBackslash2fore(char *str);
//extern int screenWidth; /*used in mainloop render_pre setup_projection*/
//extern int screenHeight;
void fwl_setConsole_writePrimitive(int ibool);

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
extern "C"{
int fv_display_initialize(void);
//static freewrl_params_t *fv_params;
}
void CdllFreeWRL::onInit(void *handle,int width, int height){
	struct freewrl_params *params;
	//_putenv("FREEWRL_NO_VBOS=1"); 

	if( !fwl_setCurrentHandle(handle) ){
		/* Before we parse the command line, setup the FreeWRL default parameters */
		params = (freewrl_params_t*) malloc( sizeof(freewrl_params_t));

		//params = (struct freewrl_params*) malloc( sizeof(struct freewrl_params));
		/* Default values */
		params->width = width; //600;
		params->height = height; //400;
		params->eai = 0;
		params->fullscreen = 0;
		params->winToEmbedInto = (int)handle;
		swDebugf("Hi just before fwl_initFreeWRL\n");
		//fwl_init_instanceh(); //before setting any structs we need a struct allocated
		swDebugf("onInit: do consoleMessages come out\n");
		swDebugf("onInit sure");
		void *fwl = fwl_init_instance(); //before setting any structs we need a struct allocated
		fwl_ConsoleSetup(MC_DEF_AQUA , MC_TARGET_AQUA , MC_HAVE_MOTIF , MC_TARGET_MOTIF , MC_MSC_HAVE_VER , 0);

		if (!fwl_initFreeWRL(params)) {
			//ERROR_MSG("main: aborting during initialization.\n");
			//exit(1);
		}
		fwl_setConsole_writePrimitive( 1 );
		DWORD pid = GetCurrentProcessId() ;
		initConsoleH(pid);

		swDebugf("after fwl_initFreeWRL\n");
		//initStereoDefaults();
		//fwl_startFreeWRL("C:\\source2\\tests\\1.wrl");
		//swDebugf("after fwl_startFreeWRL\n");
		//fwl_initializeRenderSceneUpdateScene()
		swDebugf("onInit: before fv_display_initialize\n");
#ifdef FRONTEND_HANDLES_DISPLAY_THREAD
		fv_display_initialize();
		swDebugf("onInit: before initializeRenderSceneUpdateScene\n");
		fwl_initializeRenderSceneUpdateScene();
		swDebugf("onInit: after initializeRenderSceneUpdateScene\n");
#endif

		//fwl_resource_push_single_request("C:\\source2\\tests\\1.wrl");
	}
	fwl_clearCurrentHandle();

}
void CdllFreeWRL::onLoad(void *handle, char* scene_url)
{
	//swDebugf("onLoad before finalizeRenderSceneUpdateScene\n");
	//finalizeRenderSceneUpdateScene();
	//swDebugf("onLoad after finalizeRenderSceneUpdateScene\n");
	char * url;
/*
	swDebugf("onLoad: before fv_display_initialize\n");
	fv_display_initialize();
	swDebugf("onLoad before initializeRenderSceneUpdateScene\n");
	fwl_initializeRenderSceneUpdateScene();
	swDebugf("onLoad after initializeRenderSceneUpdateScene\n");
*/
	if(fwl_setCurrentHandle(handle)){
		url = strdup(scene_url);
		url = strBackslash2fore(url);
		swDebugf("onLoad have url=[%s]\n",url);
		//fwl_resource_push_single_request(this->url); //"C:\\source2\\tests\\1.wrl");
		//fwl_resource_push_single_request_IE_main_scene(this->url);
		fwl_replaceWorldNeeded(url);
		//fwl_OSX_initializeParameters("http://freewrl.sourceforge.net/test3.wrl");
		swDebugf("onLoad after push_single_request url=[%s]\n",url);
	}
	fwl_clearCurrentHandle();

}

void CdllFreeWRL::onResize(void *handle, int width,int height){
	//screenWidth = width; /*used in mainloop render_pre setup_projection*/
	//screenHeight = height;
	//resize_GL(width, height); 
	if(fwl_setCurrentHandle(handle)){

		fwl_setScreenDim(width,height);
		//ConsoleMessage("dll_onResize w=%d h=%d\n",width,height);
	}
	fwl_clearCurrentHandle();
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

void CdllFreeWRL::onMouse(void *handle, int mouseAction,int mouseButton,int x, int y){

	/*void fwl_handle_aqua(const int mev, const unsigned int button, int x, int y);*/
	/* butnum=1 left butnum=3 right (butnum=2 middle, not used by freewrl) */
	//fwl_handle_aqua(mev,butnum,mouseX,mouseY); 
	if(fwl_setCurrentHandle(handle)){
		fwl_handle_aqua(mouseAction,mouseButton,x,y); 
	}
	fwl_clearCurrentHandle();
	//ConsoleMessage("%d %d %d %d\r",mouseAction,mouseButton,x,y);
}
void CdllFreeWRL::onKey(void *handle, int keyAction,int keyValue){

	int kp = keyValue;
	int ka = keyAction;
	if(fwl_setCurrentHandle(handle)){
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
			fwl_do_keyPress(kp, ka); 
			break; 

		case KEYPRESS: //WM_CHAR:
			fwl_do_keyPress(kp,ka);
			break;
		}
	}
	fwl_clearCurrentHandle();
}
void CdllFreeWRL::onTick(void *handle, int interval){
	//fwl_setCurrentHandle(handle);
	//fwl_RenderSceneUpdateScene();
	//fwl_clearCurrentHandle();

}
void CdllFreeWRL::onClose(void *handle)
{
    
	/* when finished: */
	if(fwl_setCurrentHandle(handle)){
		//finalizeRenderSceneUpdateScene(); //doesn't like killoldworld

		//closeFreeWRL();
		fwl_doQuitInstance();
		//terminateFreeWRL();
	}
	fwl_clearCurrentHandle();
}
char* CdllFreeWRL::downloadFileName(void *handle)
{
// you need to build the libreewrl with #ifdef FRONTEND_GETS_FILES  defined (otherwise you'll get stubs)
// and	#ifdef COMPILING_ACTIVEX_FRONTEND  defined for this dllFreeWRL build (otherwise header commented out)

#ifdef COMPILING_ACTIVEX_FRONTEND
	if(fwl_setCurrentHandle(handle)){
		return fwg_frontEndWantsFileName();
	}
	fwl_clearCurrentHandle();
#else
	return NULL;
#endif
}
void CdllFreeWRL::downloadComplete(void *handle, char *localfile, int iret)
{
	if(fwl_setCurrentHandle(handle)){
		fwg_frontEndReturningLocalFile(localfile, iret);
	}
	fwl_clearCurrentHandle();

}
void CdllFreeWRL::print(void *handle, char *str)
{
	if(fwl_setCurrentHandle(handle)){
		swDebugf(str);
	}
	fwl_clearCurrentHandle();
}
