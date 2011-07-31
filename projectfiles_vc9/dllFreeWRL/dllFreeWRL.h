// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DLLFREEWRL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DLLFREEWRL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DLLFREEWRL_EXPORTS
#define DLLFREEWRL_API __declspec(dllexport)
#else
#define DLLFREEWRL_API __declspec(dllimport)
#endif

// This class is exported from the dllFreeWRL.dll
class DLLFREEWRL_API CdllFreeWRL {
public:
	CdllFreeWRL(void);
	// TODO: add your methods here.
	static enum KeyAction {KEYDOWN=2,KEYUP=3,KEYPRESS=1};
	//#define KeyChar         1  //KeyPress
	//#define KeyPress        2  //KeyDown
	//#define KeyRelease      3  //KeyUp

	static enum MouseAction {MOUSEMOVE=6,MOUSEDOWN=4,MOUSEUP=5};
	//	mev = ButtonPress; //4 down
	//	mev = ButtonRelease; //3 up
	//	mev = MotionNotify; //6 move
	static enum MouseButton {LEFT=1,MIDDLE=2,RIGHT=3,NONE=0}; 		
	/* butnum=1 left butnum=3 right (butnum=2 middle, not used by freewrl) */

	static void onInit(void* handle,int width, int height);
	static void onLoad(void *handle, char* scene_url);
    static void onResize(void *handle, int height,int width);
    static void onMouse(void *handle, int mouseAction,int mouseButton,int x, int y);
    static void onKey(void *handle, int keyAction,int keyValue);
    static void onTick(void *handle, int interval);
	static void onClose(void *handle);
	static char* downloadFileName(void *handle);
	static void downloadComplete(void *handle, char *localfile, int iret);
	static void print(void *handle, char *str);
private:
	static char *url;
};

extern DLLFREEWRL_API int ndllFreeWRL;

DLLFREEWRL_API int fndllFreeWRL(void);
