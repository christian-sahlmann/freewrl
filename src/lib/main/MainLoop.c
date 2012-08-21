/*
  $Id: MainLoop.c,v 1.267 2012/08/21 23:02:19 dug9 Exp $

  FreeWRL support library.
  Main loop : handle events, ...

*/

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <system_js.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>
#include <list.h>
#include <threads.h>
#include <resources.h>

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../world_script/jsUtils.h"
#include "../world_script/JScript.h"
#include "../world_script/CScripts.h"
#include "Snapshot.h"
#include "../scenegraph/LinearAlgebra.h"
#include "../scenegraph/Collision.h"

#include "../scenegraph/Viewer.h"
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"
#include "../input/EAIHeaders.h"

#include "../scenegraph/Component_KeyDevice.h"	/* resolving implicit declarations */
#include "../opengl/Frustum.h"
#include "../input/InputFunctions.h"

#include "../opengl/OpenGL_Utils.h"
#include "../ui/statusbar.h"
#include "../ui/CursorDraw.h"
#include "../scenegraph/RenderFuncs.h"

#include "../ui/common.h"


void (*newResetGeometry) (void) = NULL;

#ifdef WANT_OSC
	#define USE_OSC 1
#else
	#define USE_OSC 0
#endif

#if defined(_ANDROID )
void  setAquaCursor(int ctype) { };

#endif // _ANDROID

#ifdef IPHONE
void  setAquaCursor(int ctype) { };

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#include "MainLoop.h"

double TickTime()
{
	return gglobal()->Mainloop.TickTime;
}
double lastTime()
{
	return gglobal()->Mainloop.lastTime;
}
/* Sensor table. When clicked, we get back from getRayHit the fromnode,
        have to look up type and data in order to properly handle it */
struct SensStruct {
        struct X3D_Node *fromnode;
        struct X3D_Node *datanode;
        void (*interpptr)(void *, int, int, int);
};
struct Touch
{
	int button; /*none down=0, LMB =1, MMB=2, RMB=3*/
	bool isDown; /* false = up, true = down */
	int mev; /* down/press=4, move/drag=6, up/release=5 */
	int ID;  /* for multitouch: 0-20, represents one finger drag. Recycle after an up */
	float angle; /*some multitouch -like smarttech- track the angle of the finger */
	int x;
	int y;
}; 

typedef struct pMainloop{
	//browser
	/* are we displayed, or iconic? */
	int onScreen;// = TRUE;

	/* do we do event propagation, proximity calcs?? */
	int doEvents;// = FALSE;

	#ifdef VERBOSE
	char debs[300];
	#endif

	char* PluginFullPath;
	//
	int num_SensorEvents;// = 0;

	/* Viewport data */
	GLint viewPort2[10];
	GLint viewpointScreenX[2]; /*for stereo where we can adjust the viewpoint position on the screen */
	/* screen width and height. */
	struct X3D_Node* CursorOverSensitive;//=NULL;      /*  is Cursor over a Sensitive node?*/
	struct X3D_Node* oldCOS;//=NULL;                   /*  which node was cursor over before this node?*/
	int NavigationMode;//=FALSE;               /*  are we navigating or sensing?*/
	int ButDown[20][8];// = {{FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE}};

	int currentCursor;// = 0;
	int lastMouseEvent;// = 0/*MapNotify*/;         /*  last event a mouse did; care about Button and Motion events only.*/
	struct X3D_Node* lastPressedOver;// = NULL;/*  the sensitive node that the mouse was last buttonpressed over.*/
	struct X3D_Node* lastOver;// = NULL;       /*  the sensitive node that the mouse was last moused over.*/
	int lastOverButtonPressed;// = FALSE;      /*  catch the 1 to 0 transition for button presses and isOver in TouchSensors */

	int maxbuffers;// = 1;                     /*  how many active indexes in bufferarray*/
	int bufferarray[2];// = {GL_BACK,0};

	double BrowserStartTime;        /* start of calculating FPS     */

	int quitThread;// = FALSE;
	int keypress_wait_for_settle;// = 100;     /* JAS - change keypress to wait, then do 1 per loop */
	char * keypress_string;//=NULL;            /* Robert Sim - command line key sequence */

	struct SensStruct *SensorEvents;// = 0;

    int loop_count;// = 0;
    int slowloop_count;// = 0;
	double waitsec;

	//scene
	//window
	//2D_inputdevice
	int lastDeltax;// = 50;
	int lastDeltay;// = 50;
	int lastxx;
	int lastyy;
	int ntouch;// =0;
	int currentTouch;// = -1;
	struct Touch touchlist[20];
	int EMULATE_MULTITOUCH;// = 1; 

}* ppMainloop;
void *Mainloop_constructor(){
	void *v = malloc(sizeof(struct pMainloop));
	memset(v,0,sizeof(struct pMainloop));
	return v;
}
void Mainloop_init(struct tMainloop *t){
	//public
	/* linewidth for lines and points - passed in on command line */
	t->gl_linewidth= 1.0f;
	//t->TickTime;
	//t->lastTime;
	t->BrowserFPS = 100.0;        /* calculated FPS               */
	t->BrowserSpeed = 0.0;      /* calculated movement speed    */
	t->trisThisLoop = 0;

	/* what kind of file was just parsed? */
	t->currentFileVersion = 0;
	/* do we have some sensitive nodes in scene graph? */
	t->HaveSensitive = FALSE;
	//t->currentX[20];
	//t->currentY[20];                 /*  current mouse position.*/
	t->clipPlane = 0;

	t->tmpFileLocation = MALLOC (char *,5);
	strcpy(t->tmpFileLocation,"/tmp");

	//private
	t->prv = Mainloop_constructor();
	{
		ppMainloop p = (ppMainloop)t->prv;
		//browser
		/* are we displayed, or iconic? */
		p->onScreen = TRUE;

		/* do we do event propagation, proximity calcs?? */
		p->doEvents = FALSE;

		#ifdef VERBOSE
		//static char debs[300];
		#endif

		//char* PluginFullPath;
		p->num_SensorEvents = 0;

		/* Viewport data */
		//p->viewPort2[10];

		/* screen width and height. */
		p->CursorOverSensitive=NULL;      /*  is Cursor over a Sensitive node?*/
		p->oldCOS=NULL;                   /*  which node was cursor over before this node?*/
		p->NavigationMode=FALSE;               /*  are we navigating or sensing?*/
		//p->ButDown[20][8] = {{FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE}}; nulls

		p->currentCursor = 0;
		p->lastMouseEvent = 0/*MapNotify*/;         /*  last event a mouse did; care about Button and Motion events only.*/
		p->lastPressedOver = NULL;/*  the sensitive node that the mouse was last buttonpressed over.*/
		p->lastOver = NULL;       /*  the sensitive node that the mouse was last moused over.*/
		p->lastOverButtonPressed = FALSE;      /*  catch the 1 to 0 transition for button presses and isOver in TouchSensors */

		p->maxbuffers = 1;                     /*  how many active indexes in bufferarray*/
		p->bufferarray[0] = GL_BACK;
		p->bufferarray[1] = 0;
		/* current time and other time related stuff */
		//p->BrowserStartTime;        /* start of calculating FPS     */

		p->quitThread = FALSE;
		p->keypress_wait_for_settle = 100;     /* JAS - change keypress to wait, then do 1 per loop */
		p->keypress_string=NULL;            /* Robert Sim - command line key sequence */

		p->SensorEvents = 0;

        p->loop_count = 0;
        p->slowloop_count = 0;
		//p->waitsec;

		//scene
		//window
		//2D_inputdevice
		p->lastDeltax = 50;
		p->lastDeltay = 50;
		//p->lastxx;
		//p->lastyy;
		p->ntouch =0;
		p->currentTouch = -1;
		//p->touchlist[20];
		p->EMULATE_MULTITOUCH = 0; 

	}
}

//true statics:
int isBrowserPlugin = FALSE; //I can't think of a scenario where sharing this across instances would be a problem

///* are we displayed, or iconic? */
//static int onScreen = TRUE;
//
//
///* do we do event propagation, proximity calcs?? */
//static int doEvents = FALSE;
//
//#ifdef VERBOSE
//static char debs[300];
//#endif
//
//char* PluginFullPath;
//
///* linewidth for lines and points - passed in on command line */
//float gl_linewidth = 1.0f;
//
///* what kind of file was just parsed? */
//int currentFileVersion = 0;

/*
   we want to run initialize() from the calling thread. NOTE: if
   initialize creates VRML/X3D nodes, it will call the ProdCon methods
   to do this, and these methods will check to see if nodes, yada,
   yada, yada, until we run out of stack. So, we check to see if we
   are initializing; if so, don't worry about checking for new scripts
   any scripts to initialize here? we do it here, because we may just
   have created new scripts during  X3D/VRML parsing. Routing in the
   Display thread may have noted new scripts, but will ignore them
   until   we have told it that the scripts are initialized.  printf
   ("have scripts to initialize in fwl_RenderSceneUpdateScene old %d new
   %d\n",max_script_found, max_script_found_and_initialized);
*/

#define INITIALIZE_ANY_SCRIPTS \
        if (tg->CRoutes.max_script_found != tg->CRoutes.max_script_found_and_initialized) { \
				struct CRscriptStruct *ScriptControl = getScriptControl(); \
                int i; jsval retval; \
                for (i=tg->CRoutes.max_script_found_and_initialized+1; i <= tg->CRoutes.max_script_found; i++) { \
                        /* printf ("initializing script %d in thread %u\n",i,pthread_self());  */ \
                        JSCreateScriptContext(i); \
                        JSInitializeScriptAndFields(i); \
			if (ScriptControl[i].scriptOK) ACTUALRUNSCRIPT(i, "initialize()" ,&retval); \
                        /* printf ("initialized script %d\n",i);*/  \
                } \
                tg->CRoutes.max_script_found_and_initialized = tg->CRoutes.max_script_found; \
        }

/* we bind bindable nodes on parse in this thread */
#define SEND_BIND_IF_REQUIRED(node) \
                if (node != NULL) { send_bind_to(X3D_NODE(node),1); node = NULL; }



static void setup_viewpoint();
static void get_collisionoffset(double *x, double *y, double *z);

/* Function protos */
static void sendDescriptionToStatusBar(struct X3D_Node *CursorOverSensitive);
/* void fwl_do_keyPress(char kp, int type); Now in lib.h */
void render_collisions(int Viewer_type);
void slerp_viewpoint();
static void render_pre(void);
static void render(void);
static void setup_projection(int pick, int x, int y);
static struct X3D_Node*  getRayHit(void);
static void get_hyperhit(void);
static void sendSensorEvents(struct X3D_Node *COS,int ev, int butStatus, int status);
#if USE_OSC
void activate_OSCsensors();
#endif


/* libFreeWRL_get_version()

  Q. where do I get this function ?
  A: look in Makefile.am (vtempl will create it automatically in internal_version.c).

*/

/* stop the display thread. Used (when this comment was made) by the OSX Safari plugin; keeps
most things around, just stops display thread, when the user exits a world. */
static void stopDisplayThread()
{
	ttglobal tg = gglobal();
	if (!TEST_NULL_THREAD(tg->threads.DispThrd)) {
		((ppMainloop)(tg->Mainloop.prv))->quitThread = TRUE;
		pthread_join(tg->threads.DispThrd,NULL);
		ZERO_THREAD(tg->threads.DispThrd);
	}
}
#ifndef SIGTERM
#define SIGTERM SIG_TERM
#endif


// stops the Texture loading thread - will either pthread_cancel or will send SIGUSR2 to 
// the thread, depending on platform.

static void stopLoadThread()
{
	ttglobal tg = gglobal();
	if (!TEST_NULL_THREAD(tg->threads.loadThread)) {

		#if defined(HAVE_PTHREAD_CANCEL)
			pthread_cancel(tg->threads.loadThread);
	 	#else

		{	
			int status; 
			char me[200]; 
			sprintf(me,"faking pthread cancel on thread %x",tg->threads.loadThread); 
			ConsoleMessage(me); 
			if ((status = pthread_kill(tg->threads.loadThread, SIGUSR2)) != 0) {
				ConsoleMessage("issue stopping thread");
			}
		} 
		#endif //HAVE_PTHREAD_CANCEL

		pthread_join(tg->threads.loadThread,NULL);
		ZERO_THREAD(tg->threads.loadThread);
	}
}


// stops the source parsing thread - will either pthread_cancel or will send SIGUSR2 to 
// the thread, depending on platform.

static void stopPCThread()
{
	ttglobal tg = gglobal();

	if (!TEST_NULL_THREAD(tg->threads.PCthread)) {
		#if defined(HAVE_PTHREAD_CANCEL)
			pthread_cancel(tg->threads.PCthread);
	 	#else

		{	
			int status; 
			char me[200]; 
			sprintf(me,"faking pthread cancel on thread %x",tg->threads.PCthread); 
			ConsoleMessage(me); 
			if ((status = pthread_kill(tg->threads.PCthread, SIGUSR2)) != 0) {
				ConsoleMessage("issue stopping thread");
			}
		} 
	#endif //HAVE_PTHREAD_CANCEL

		pthread_join(tg->threads.PCthread,NULL);
		ZERO_THREAD(tg->threads.PCthread);
	}
}

//static double waitsec;

#if !defined(_MSC_VER)

//static struct timeval mytime;

/* Doug Sandens windows function; lets make it static here for non-windows */
static double Time1970sec(void) {
		struct timeval mytime;
        gettimeofday(&mytime, NULL);
        return (double) mytime.tv_sec + (double)mytime.tv_usec/1000000.0;
}

#else 

/* #ifdef STRANGE_FUNCTION_FROM_LIBFREEWRL_H */

#include <windows.h>
__inline double Time1970sec()
{
   SYSTEMTIME mytimet; /*winNT and beyond */
   /* the windows getlocaltime has a granularity of 1ms at best. 
   There are a gazillion time functions in windows so I isolated it here in case I got it wrong*/
		/* win32 there are some higher performance timer functions (win95-vista)
		but a system might not support it - lpFrequency returns 0 if not supported
		BOOL QueryPerformanceFrequency( LARGE_INTEGER *lpFrequency );
		BOOL QueryPerformanceCounter( LARGE_INTEGER *lpPerformanceCount );
		*/

   GetLocalTime(&mytimet);
   return (double) mytimet.wHour*3600.0 + (double)mytimet.wMinute*60.0 + (double)mytimet.wSecond + (double)mytimet.wMilliseconds/1000.0;
}

/* #endif */

#endif


#define DJ_KEEP_COMPILER_WARNING 0
#if DJ_KEEP_COMPILER_WARNING
#define TI(_tv) gettimeofdat(&_tv)
#define TID(_tv) ((double)_tv.tv_sec + (double)_tv.tv_usec/1000000.0)
#endif


/* Main eventloop for FreeWRL!!! */
void fwl_RenderSceneUpdateScene() {
        //static int loop_count = 0;
        //static int slowloop_count = 0;
		ttglobal tg = gglobal();
		ppMainloop p = (ppMainloop)tg->Mainloop.prv;

    PRINT_GL_ERROR_IF_ANY("start of renderSceneUpdateScene");
    
        DEBUG_RENDER("start of MainLoop (parsing=%s) (url loaded=%s)\n", 
		     BOOL_STR(fwl_isinputThreadParsing()), BOOL_STR(resource_is_root_loaded()));

        /* should we do events, or maybe a parser is parsing? */
        p->doEvents = (!fwl_isinputThreadParsing()) && (!fwl_isTextureParsing()) && fwl_isInputThreadInitialized();

        /* First time through */
        if (p->loop_count == 0) {
                p->BrowserStartTime = Time1970sec();
				tg->Mainloop.TickTime = p->BrowserStartTime;
                tg->Mainloop.lastTime = tg->Mainloop.TickTime - 0.01; /* might as well not invoke the usleep below */
        } else {
		/* NOTE: front ends now sync with the monitor, meaning, this sleep is no longer needed unless
		   something goes totally wrong */
#ifndef FRONTEND_HANDLES_DISPLAY_THREAD
		/* we see how long it took to do the last loop; now that the frame rate is synced to the
		   vertical retrace of the screens, we should not get more than 60-70fps. We calculate the
		   time here, if it is more than 200fps, we sleep for 1/100th of a second - we should NOT
		   need this, but in case something goes pear-shaped (british expression, there!) we do not
		   consume thousands of frames per second */

               p->waitsec = TickTime() - lastTime();
               if (p->waitsec < 0.005) {
                       usleep(10000);
		}
#endif /* FRONTEND_HANDLES_DISPLAY_THREAD */
        }

        /* Set the timestamp */
	tg->Mainloop.lastTime = tg->Mainloop.TickTime;
	tg->Mainloop.TickTime = Time1970sec();

        /* any scripts to do?? */
#ifdef _MSC_VER
		if(p->doEvents)
#endif /* _MSC_VER */

	#ifdef HAVE_JAVASCRIPT
        INITIALIZE_ANY_SCRIPTS;
	#endif


        /* BrowserAction required? eg, anchors, etc */
        if (tg->RenderFuncs.BrowserAction) {
                tg->RenderFuncs.BrowserAction = doBrowserAction ();
        }

        /* has the default background changed? */
        if (tg->OpenGL_Utils.cc_changed) doglClearColor();

        OcclusionStartofRenderSceneUpdateScene();
        startOfLoopNodeUpdates();

        if (p->loop_count == 25) {

                tg->Mainloop.BrowserFPS = 25.0 / (TickTime()-p->BrowserStartTime);
                setMenuFps((float)tg->Mainloop.BrowserFPS); /*  tell status bar to refresh, if it is displayed*/
                /* printf ("fps %f tris %d, rootnode children %d \n",p->BrowserFPS,p->trisThisLoop, X3D_GROUP(rootNode)->children.n);  */
                /* ConsoleMessage("fps %f tris %d\n",p->BrowserFPS,p->trisThisLoop);   */

		/* printf ("MainLoop, nearPlane %lf farPlane %lf\n",Viewer.nearPlane, Viewer.farPlane);  */

                p->BrowserStartTime = TickTime();
                p->loop_count = 1;
        } else {
                p->loop_count++;
        }

        tg->Mainloop.trisThisLoop = 0;

	if(p->slowloop_count == 1009) p->slowloop_count = 0 ;
	#if USE_OSC
	if ((p->slowloop_count % 256) == 0) {
		/* activate_picksensors() ; */
		/*
		printf("slowloop_count = %d at T=%lf : lastMouseEvent=%d , MotionNotify=%d\n",
			p->slowloop_count, TickTime(), p->lastMouseEvent, MotionNotify) ;
		*/
		activate_OSCsensors() ;
	} else {
		/* deactivate_picksensors() ; */
	}
	#endif /* USE_OSC */

	p->slowloop_count++ ;

        /* handle any events provided on the command line - Robert Sim */
        if (p->keypress_string && p->doEvents) {
                if (p->keypress_wait_for_settle > 0) {
                        p->keypress_wait_for_settle--;
                } else {
                        /* dont do the null... */
                        if (*p->keypress_string) {
                                /* printf ("handling key %c\n",*p->keypress_string); */
#if !defined( AQUA ) && !defined( _MSC_VER )  /*win32 - don't know whats it is suppsoed to do yet */

				DEBUG_XEV("CMD LINE GEN EVENT: %c\n", *p->keypress_string);
                                fwl_do_keyPress(*p->keypress_string,KeyPress);
#endif /* NOT AQUA and NOT WIN32 */

                                p->keypress_string++;
                        } else {
                                p->keypress_string=NULL;
                        }
                }
        }

#if KEEP_X11_INLIB
	/**
	 *   Merge of Bare X11 and Motif/X11 event handling ...
	 */
	/* REMARK: Do we want to process all pending events ? */

#if defined(TARGET_X11)
	/* We are running our own bare window */
	while (XPending(Xdpy)) {
	    XNextEvent(Xdpy, &event);
	    DEBUG_XEV("EVENT through XNextEvent\n");
	    handle_Xevents(event);
	}
#endif /* TARGET_X11 */

    
    PRINT_GL_ERROR_IF_ANY("before xtdispatch");
#if defined(TARGET_MOTIF)
	/* any updates to the menu buttons? Because of Linux threading
	   issues, we try to make all updates come from 1 thread */
	frontendUpdateButtons();
	
	/* do the Xt events here. */
	while (XtAppPending(Xtcx)!= 0) {
	    XAnyEvent *aev;
	    
	    XtAppNextEvent(Xtcx, &event);
	    
	    aev = &event.xany;
	    
#ifdef XEVENT_VERBOSE
	    XButtonEvent *bev;
	    XMotionEvent *mev;
	    
	    switch (event.type) {
	    case MotionNotify:
		mev = &event.xmotion;
		TRACE_MSG("mouse motion event: win=%u, state=%d\n",
			  mev->window, mev->state);
		break;
	    case ButtonPress:
	    case ButtonRelease:
		bev = &event.xbutton;
		TRACE_MSG("mouse button event: win=%u, state=%d\n",
			  bev->window, bev->state);
		break;
	    }
#endif /* XEVENT_VERBOSE */

	    DEBUG_XEV("EVENT through XtDispatchEvent\n");
	    XtDispatchEvent (&event);
	}

#endif /* TARGET_MOTIF */
#endif /* KEEP_X11_INLIB */

#if defined(_MSC_VER) && !defined(GLES2)
	/**
	 *   Win32 event loop
	 *   gives windows message handler a time slice and 
	 *   it calls fwl_handle_aqua and do_keypress from fwWindow32.c 
	 */
	doEventsWin32A(); 
#endif /* _MSC_VER */

        /* Viewer move viewpoint */
        handle_tick();

    PRINT_GL_ERROR_IF_ANY("after handle_tick")
    
        /* setup Projection and activate ProximitySensors */
        if (p->onScreen) 
		{
			render_pre(); 
			slerp_viewpoint();
		}
		

        /* first events (clock ticks, etc) if we have other things to do, yield */
        if (p->doEvents) do_first (); else sched_yield();

	/* ensure depth mask turned on here */
	FW_GL_DEPTHMASK(GL_TRUE);

    PRINT_GL_ERROR_IF_ANY("after depth")
        /* actual rendering */
        if (p->onScreen)
			render();

        /* handle_mouse events if clicked on a sensitive node */
	 //printf("nav mode =%d sensitive= %d\n",p->NavigationMode, tg->Mainloop.HaveSensitive);  
        if (!p->NavigationMode && tg->Mainloop.HaveSensitive) {
                p->currentCursor = 0;
                setup_projection(TRUE,tg->Mainloop.currentX[p->currentCursor],tg->Mainloop.currentY[p->currentCursor]);
                setup_viewpoint();
                render_hier(rootNode(),VF_Sensitive  | VF_Geom); 
                p->CursorOverSensitive = getRayHit();

                /* for nodes that use an "isOver" eventOut... */
                if (p->lastOver != p->CursorOverSensitive) {
                        #ifdef VERBOSE
			  printf ("%lf over changed, p->lastOver %u p->cursorOverSensitive %u, p->butDown1 %d\n",
				TickTime(), (unsigned int) p->lastOver, (unsigned int) p->CursorOverSensitive,
				p->ButDown[p->currentCursor][1]);
                        #endif

                        if (p->ButDown[p->currentCursor][1]==0) {

                                /* ok, when the user releases a button, cursorOverSensitive WILL BE NULL
                                   until it gets sensed again. So, we use the lastOverButtonPressed flag to delay 
                                   sending this flag by one event loop loop. */
                                if (!p->lastOverButtonPressed) {
                                        sendSensorEvents(p->lastOver, overMark, 0, FALSE);
                                        sendSensorEvents(p->CursorOverSensitive, overMark, 0, TRUE);
                                        p->lastOver = p->CursorOverSensitive;
                                }
                                p->lastOverButtonPressed = FALSE;
                        } else {
                                p->lastOverButtonPressed = TRUE;
                        }

                }
                #ifdef VERBOSE
                if (p->CursorOverSensitive != NULL) 
			printf("COS %d (%s)\n",
			       (unsigned int) p->CursorOverSensitive,
			       stringNodeType(p->CursorOverSensitive->_nodeType));
                #endif /* VERBOSE */

                /* did we have a click of button 1? */

                if (p->ButDown[p->currentCursor][1] && (p->lastPressedOver==NULL)) {
                        /* printf ("Not Navigation and 1 down\n"); */
                        /* send an event of ButtonPress and isOver=true */
                        p->lastPressedOver = p->CursorOverSensitive;
                        sendSensorEvents(p->lastPressedOver, ButtonPress, p->ButDown[p->currentCursor][1], TRUE);
                }

                if ((p->ButDown[p->currentCursor][1]==0) && p->lastPressedOver!=NULL) {
                        /* printf ("Not Navigation and 1 up\n");  */
                        /* send an event of ButtonRelease and isOver=true;
                           an isOver=false event will be sent below if required */
                        sendSensorEvents(p->lastPressedOver, ButtonRelease, p->ButDown[p->currentCursor][1], TRUE);
                        p->lastPressedOver = NULL;
                }

                if (p->lastMouseEvent == MotionNotify) {
                        /* printf ("Not Navigation and motion - going into sendSensorEvents\n"); */
                        /* TouchSensor hitPoint_changed needs to know if we are over a sensitive node or not */
                        sendSensorEvents(p->CursorOverSensitive,MotionNotify, p->ButDown[p->currentCursor][1], TRUE);

                        /* PlaneSensors, etc, take the last sensitive node pressed over, and a mouse movement */
                        sendSensorEvents(p->lastPressedOver,MotionNotify, p->ButDown[p->currentCursor][1], TRUE);
                	p->lastMouseEvent = 0 ;
                }



                /* do we need to re-define cursor style?        */
                /* do we need to send an isOver event?          */
                if (p->CursorOverSensitive!= NULL) {
					//SENSOR_CURSOR;
					setSensorCursor();

                        /* is this a new node that we are now over?
                           don't change the node pointer if we are clicked down */
                        if ((p->lastPressedOver==NULL) && (p->CursorOverSensitive != p->oldCOS)) {
                                sendSensorEvents(p->oldCOS,MapNotify,p->ButDown[p->currentCursor][1], FALSE);
                                sendSensorEvents(p->CursorOverSensitive,MapNotify,p->ButDown[p->currentCursor][1], TRUE);
                                p->oldCOS=p->CursorOverSensitive;
                                sendDescriptionToStatusBar(p->CursorOverSensitive);
                        }

                } else {
                        /* hold off on cursor change if dragging a sensor */
                        if (p->lastPressedOver!=NULL) {
							//SENSOR_CURSOR;
							setSensorCursor();
                        } else {
							//ARROW_CURSOR;
							setArrowCursor();
                        }

                        /* were we over a sensitive node? */
                        if ((p->oldCOS!=NULL)  && (p->ButDown[p->currentCursor][1]==0)) {
                                sendSensorEvents(p->oldCOS,MapNotify,p->ButDown[p->currentCursor][1], FALSE);
                                /* remove any display on-screen */
                                sendDescriptionToStatusBar(NULL);
                                p->oldCOS=NULL;
                        }
                }

                //if (ccurse != ocurse) {
                //        ocurse = ccurse;
                //        setCursor();
                //}
        } /* (!NavigationMode && HaveSensitive) */
		else
			setArrowCursor();


	#if !defined(FRONTEND_DOES_SNAPSHOTS)
        /* handle snapshots */
        if (tg->Snapshot.doSnapshot) {
                Snapshot();
        }
	#endif //FRONTEND_DOES_SNAPSHOTS

        /* do OcclusionCulling, etc */
        OcclusionCulling();
        
        if (p->doEvents) {
                /* and just parsed nodes needing binding? */
                SEND_BIND_IF_REQUIRED(tg->ProdCon.setViewpointBindInRender)
                SEND_BIND_IF_REQUIRED(tg->ProdCon.setFogBindInRender)
                SEND_BIND_IF_REQUIRED(tg->ProdCon.setBackgroundBindInRender)
                SEND_BIND_IF_REQUIRED(tg->ProdCon.setNavigationBindInRender)


                /* handle ROUTES - at least the ones not generated in do_first() */
                propagate_events();

                /* Javascript events processed */
                process_eventsProcessed();

		#if !defined(EXCLUDE_EAI)
		/*
		 * Actions are now separate so that file IO is not tightly coupled
		 * via shared buffers and file descriptors etc. 'The core' now calls
		 * the fwlio_SCK* funcs to get data into the system, and calls the fwl_EAI*
		 * funcs to give the data to the EAI,nd the fwl_MIDI* funcs for MIDI
		 *
		 * Although the MIDI code and the EAI code are basically the same
		 * and one could compress them into a loop, for the moment keep
		 * them seperate to serve as a example for any extensions...
		 */

                /* handle_EAI(); */
		{
		int socketVerbose = fwlio_RxTx_control(CHANNEL_EAI, RxTx_GET_VERBOSITY)  ;
		if ( socketVerbose <= 1 || (socketVerbose > 1 && ((p->slowloop_count % 256) == 0)) ) {
			if(fwlio_RxTx_control(CHANNEL_EAI, RxTx_REFRESH) == 0) {
				/* Nothing to be done, maybe not even running */
				if ( socketVerbose > 1 ) {
					printf("%s:%d Nothing to be done\n",__FILE__,__LINE__) ;
				}
			} else {
				if ( socketVerbose > 1 ) {
					printf("%s:%d Test RxTx_PENDING\n",__FILE__,__LINE__) ;
				}
				if(fwlio_RxTx_control(CHANNEL_EAI, RxTx_PENDING) > 0) {
					char *tempEAIdata;
					if ( socketVerbose != 0 ) {
						printf("%s:%d Something pending\n",__FILE__,__LINE__) ;
					}
					tempEAIdata = fwlio_RxTx_getbuffer(CHANNEL_EAI) ;
					if(tempEAIdata != (char *)NULL) {
						char * replyData;
						int EAI_StillToDo;
						if ( socketVerbose != 0 ) {
							printf("%s:%d Something for EAI to do with buffer addr %p\n",__FILE__,__LINE__,tempEAIdata ) ;
						}
						/*
						 * Every incoming command has a reply,
						 * and the reply is synchronous.
						 */
						replyData = fwl_EAI_handleBuffer(tempEAIdata);
						free(tempEAIdata) ;
						EAI_StillToDo = 1;
						do {
							if(replyData != NULL && strlen(replyData) != 0) {
								fwlio_RxTx_sendbuffer(__FILE__,__LINE__,CHANNEL_EAI, replyData) ;
								free(replyData) ;
								/*
								 * Note: fwlio_RxTx_sendbuffer() can also be called async
								 * due to a listener trigger within routing, but it is
								 * is up to that caller to clean out its own buffers.
								 */
							}
							EAI_StillToDo = fwl_EAI_allDone();
							if(EAI_StillToDo) {
								if ( socketVerbose != 0 ) {
									printf("%s:%d Something still in EAI buffer? %d\n",__FILE__,__LINE__,EAI_StillToDo ) ;
								}
								replyData = fwl_EAI_handleRest();
							}
						} while(EAI_StillToDo) ;
					}
				}
			}
			/* handle_MIDI(); */
			//socketVerbose = fwlio_RxTx_control(CHANNEL_MIDI, RxTx_GET_VERBOSITY)  ;
			if(fwlio_RxTx_control(CHANNEL_MIDI, RxTx_REFRESH) == 0) {
				/* Nothing to be done, maybe not even running */
				if ( socketVerbose > 1 ) {
					printf("%s:%d Nothing to be done\n",__FILE__,__LINE__) ;
				}
			} else {
				if ( socketVerbose > 1 ) {
					printf("%s:%d Test RxTx_PENDING\n",__FILE__,__LINE__) ;
				}
				if(fwlio_RxTx_control(CHANNEL_MIDI, RxTx_PENDING) > 0) {
					char *tempMIDIdata;
					if ( socketVerbose != 0 ) {
						printf("%s:%d Something pending\n",__FILE__,__LINE__) ;
					}
					tempMIDIdata = fwlio_RxTx_getbuffer(CHANNEL_MIDI) ;
					if(tempMIDIdata != (char *)NULL) {
						char * replyData;
						int EAI_StillToDo;
						if ( socketVerbose != 0 ) {
							printf("%s:%d Something for MIDI to do with buffer addr %p\n",__FILE__,__LINE__,tempMIDIdata ) ;
						}
						replyData = fwl_MIDI_handleBuffer(tempMIDIdata);
						free(tempMIDIdata) ;
						EAI_StillToDo = 1;
						do {
							if(replyData != NULL && strlen(replyData) != 0) {
								fwlio_RxTx_sendbuffer(__FILE__,__LINE__,CHANNEL_MIDI, replyData) ;
								free(replyData) ;
							}
							EAI_StillToDo = fwl_EAI_allDone();
							if(EAI_StillToDo) {
								if ( socketVerbose != 0 ) {
									printf("%s:%d Something still in EAI buffer? %d\n",__FILE__,__LINE__,EAI_StillToDo ) ;
								}
								replyData = fwl_EAI_handleRest();
							}
						} while(EAI_StillToDo) ;
					}
				}
			}
		}
		}
  		#endif //EXCLUDE_EAI
          }
  }

#if !defined( AQUA ) && !defined( _MSC_VER ) && !defined(GLES2)
void handle_Xevents(XEvent event) {

        XEvent nextevent;
        char buf[10];
        KeySym ks;
        int count;
		ppMainloop p;
		ttglobal tg = gglobal();
		p = (ppMainloop)tg->Mainloop.prv;
        p->lastMouseEvent=event.type;
		
        #ifdef VERBOSE
        switch (event.type) {
                case ConfigureNotify: printf ("Event: ConfigureNotify\n"); break;
                case ClientMessage: printf ("Event: ClientMessage\n"); break;
                case KeyPress: printf ("Event: KeyPress\n"); break;
                case KeyRelease: printf ("Event: KeyRelease\n"); break;
                case ButtonPress: printf ("Event: ButtonPress\n"); break;
                case ButtonRelease: printf ("Event: ButtonRelease\n"); break;
                case MotionNotify: printf ("Event: MotionNotify\n"); break;
                case MapNotify: printf ("Event: MapNotify\n"); break;
                case UnmapNotify: printf ("Event: *****UnmapNotify\n"); break;
                default: printf ("event, unknown %d\n", event.type);
        }
        #endif

        switch(event.type) {
//#ifdef HAVE_NOTOOLKIT
                /* Motif, etc, usually handles this. */
                case ConfigureNotify:
			/*  printf("%s,%d ConfigureNotify  %d %d\n",__FILE__,__LINE__,event.xconfigure.width,event.xconfigure.height); */
                        fwl_setScreenDim (event.xconfigure.width,event.xconfigure.height);
                        break;
//#endif
                case ClientMessage:
			if (event.xclient.data.l[0] == WM_DELETE_WINDOW && !RUNNINGASPLUGIN) {
				#ifdef VERBOSE
				printf("---XClient sent wmDeleteMessage, quitting freewrl\n");
				#endif
				fwl_doQuit();
			}
			break;
                case KeyPress:
                case KeyRelease:
                        XLookupString(&event.xkey,buf,sizeof(buf),&ks,0);
                        /*  Map keypad keys in - thanks to Aubrey Jaffer.*/
                        switch(ks) {
                           /*  the non-keyboard arrow keys*/
                           case XK_Left: ks = XK_j; break;
                           case XK_Right: ks = XK_l; break;
                           case XK_Up: ks = XK_p; break;
                           case XK_Down: ks = XK_semicolon; break;
                           case XK_KP_0:
                           case XK_KP_Insert:
                                ks = XK_a; break;
                           case XK_KP_Decimal:
                           case XK_KP_Delete:
                                ks = XK_z; break;
                           case XK_KP_7:
                           case XK_KP_Home:
                                 ks = XK_7; break;
                           case XK_KP_9:
                           case XK_KP_Page_Up:
                                ks = XK_9; break;
                           case XK_KP_8:
                           case XK_KP_Up:
                                ks = XK_k; break;
                           case XK_KP_2:
                           case XK_KP_Down:
                                ks = XK_8; break;
                           case XK_KP_4:
                           case XK_KP_Left:
                                ks = XK_u; break;
                           case XK_KP_6:
                           case XK_KP_Right:
                                ks = XK_o; break;
                           case XK_Num_Lock: ks = XK_h; break;
                           default: break;
                           }

                        /* doubt that this is necessary */
                        buf[0]=(char)ks;buf[1]='\0';

			DEBUG_XEV("Key type = %s\n", (event.type == KeyPress ? "KEY PRESS" : "KEY  RELEASE"));
                        fwl_do_keyPress((char)ks,event.type);
                        break;

                case ButtonPress:
                case ButtonRelease:
                        /* printf("got a button press or button release\n"); */
                        /*  if a button is pressed, we should not change state,*/
                        /*  so keep a record.*/
						if(handleStatusbarHud(event.type, &tg->Mainloop.clipPlane))break;
                        if (event.xbutton.button>=5) break;  /* bounds check*/
                        p->ButDown[p->currentCursor][event.xbutton.button] = (event.type == ButtonPress);

                        /* if we are Not over an enabled sensitive node, and we do NOT
                           already have a button down from a sensitive node... */
						/* printf("cursoroversensitive is %u lastPressedOver %u\n", p->CursorOverSensitive,p->lastPressedOver); */
                        if ((p->CursorOverSensitive==NULL) && (p->lastPressedOver==NULL))  {
                                p->NavigationMode=p->ButDown[p->currentCursor][1] || p->ButDown[p->currentCursor][3];
                                handle (event.type,event.xbutton.button,
                                        (float) ((float)event.xbutton.x/tg->display.screenWidth),
                                        (float) ((float)event.xbutton.y/tg->display.screenHeight));
                        }
                        break;

                case MotionNotify:
#if KEEP_X11_INLIB
                        /* printf("got a motion notify\n"); */
                        /*  do we have more motion notify events queued?*/
                        if (XPending(Xdpy)) {
                                XPeekEvent(Xdpy,&nextevent);
                                if (nextevent.type==MotionNotify) { break;
                                }
                        }
#endif /* KEEP_X11_INLIB */

                        /*  save the current x and y positions for picking.*/
                        tg->Mainloop.currentX[p->currentCursor] = event.xbutton.x;
                        tg->Mainloop.currentY[p->currentCursor] = event.xbutton.y;
                        /* printf("navigationMode is %d\n", NavigationMode); */
						if(handleStatusbarHud(6, &tg->Mainloop.clipPlane))break;
                        if (p->NavigationMode) {
                                /*  find out what the first button down is*/
                                count = 0;
                                while ((count < 5) && (!p->ButDown[p->currentCursor][count])) count++;
                                if (count == 5) return; /*  no buttons down???*/

                                handle (event.type,(unsigned)count,
                                        (float)((float)event.xbutton.x/tg->display.screenWidth),
                                        (float)((float)event.xbutton.y/tg->display.screenHeight));
                        }
                        break;
        }
}
#endif

/* get setup for rendering. */
#ifdef DJTRACK_PICKSENSORS
void do_pickSensors();
int enabled_picksensors();
#endif

static void render_pre() {
	ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;

        /* 1. Set up projection */
        setup_projection(FALSE,0,0);


        /* 2. Headlight, initialized here where we have the modelview matrix to Identity.
        FIXME: position of light sould actually be offset a little (towards the center)
        when in stereo mode. */

        /*printf("calling get headlight in render_pre\n"); */
        if (fwl_get_headlight()) lightState(HEADLIGHT_LIGHT,TRUE);


        /* 3. Viewpoint */
        setup_viewpoint();      /*  need this to render collisions correctly*/

        /* 4. Collisions */
        if (fwl_getCollision() == 1) {
                render_collisions(Viewer()->type);
                setup_viewpoint(); /*  update viewer position after collision, to*/
                                   /*  give accurate info to Proximity sensors.*/
        }

        /* 5. render hierarchy - proximity */
        if (p->doEvents) 
		{
			render_hier(rootNode(), VF_Proximity);
#ifdef DJTRACK_PICKSENSORS
			{
				/* find pickingSensors, record their world transform and picktargets */
				save_viewpoint2world();
				render_hier(rootNode(), VF_PickingSensor | VF_Other);
				if( enabled_picksensors() )
				{
					/* find picktargets, transform to world and do pick test and save results */
					render_hier(rootNode(), VF_inPickableGroup | VF_Other );
					/* record results of picks to picksensor node fields and event outs*/
					do_pickSensors();
				}
			}
#endif
		}
		//drawStatusBar();
		PRINT_GL_ERROR_IF_ANY("GLBackend::render_pre");
}
void setup_projection(int pick, int x, int y) 
{
	GLDOUBLE fieldofview2;
	GLint xvp = 0;
	GLint scissorxl,scissorxr;
	ppMainloop p;
	X3D_Viewer *viewer;
	ttglobal tg = gglobal();
	GLsizei screenwidth2 = tg->display.screenWidth;
	GLDOUBLE aspect2 = tg->display.screenRatio;
	p = (ppMainloop)tg->Mainloop.prv;
	viewer = Viewer();

	PRINT_GL_ERROR_IF_ANY("XEvents::start of setup_projection");

	scissorxl = 0;
	scissorxr = screenwidth2;
	fieldofview2 = viewer->fieldofview;
	if(viewer->type==VIEWER_YAWPITCHZOOM)
		fieldofview2*=viewer->fovZoom;
	if(viewer->isStereo)
	{
		double expansion;
		GLint xl,xr,iexpand;
		bool expand = viewer->screendist > .5f;
		expansion = viewer->screendist - .5;
		expansion = fabs(expansion);
		iexpand = expansion * screenwidth2;
		//assume: the viewpoint is centered in the viewport
		//there are 2 viewports, one for left and one for right
		//so if you want to spread the screen eyebase out, 
		//you need to expand the viewport(s) horizontally by 2x 
		// in the direction you want it to move
		//for example to move the left viewpoint left, you expand the left viewport
		//on the left side by 2x (and move the right side of the right viewport to the right)
		//to move the left viewpoint right, move the right side of the left viewport
		//to the right by 2x.
		//except in sidebyside, that would cause an over-write in the middle, and changes
		//to aspect2 ratio can change the field of view
		//so for sidebyside, we make the viewports normal screenwidth2 wide and 
		//use scissor test to crop to the viewports
		xl = 0;
		xr = screenwidth2;
		if(viewer->sidebyside)
		{
			int l,f;
			xr -= screenwidth2/4;
			xl -= screenwidth2/4;
			scissorxr = screenwidth2/2;
			if(viewer->iside ==1)
			{
				xl += screenwidth2/2;
				xr += screenwidth2/2;
				scissorxl += screenwidth2/2;
				scissorxr += screenwidth2/2;
			}
		}
		if(expand)
		{
			if(viewer->iside ==1)
				xr = xr + iexpand;
			else
				xl = xl - iexpand;
		}else{
			if(viewer->iside ==1)
				xl = xl - iexpand;
			else
				xr = xr + iexpand;
		}
		aspect2 = (double)(xr - xl)/(double)(tg->display.screenHeight - tg->Mainloop.clipPlane);
		xvp = xl;
		screenwidth2 = xr-xl;
	}
	if(0) //if(viewer->sidebyside) //old method
	{
		screenwidth2 = (int)((screenwidth2 * .5)+.5);
		aspect2 = aspect2 * .5;
		if(viewer->iside == 1) xvp = (GLint)screenwidth2;
	}

	FW_GL_MATRIX_MODE(GL_PROJECTION);
	/* >>> statusbar hud */
	if(tg->Mainloop.clipPlane != 0)
	{   /* scissor used to prevent mainloop from glClear()ing the statusbar area
		 which is updated only every 10-25 loops */
		//FW_GL_SCISSOR(0,tg->Mainloop.clipPlane,tg->display.screenWidth,tg->display.screenHeight);
		FW_GL_SCISSOR(scissorxl,tg->Mainloop.clipPlane,scissorxr-scissorxl,tg->display.screenHeight-tg->Mainloop.clipPlane);
		FW_GL_ENABLE(GL_SCISSOR_TEST);
	}
	/* <<< statusbar hud */
	p->viewpointScreenX[viewer->iside] = xvp + screenwidth2/2;
	FW_GL_VIEWPORT(xvp, tg->Mainloop.clipPlane, screenwidth2, tg->display.screenHeight-tg->Mainloop.clipPlane);

	FW_GL_LOAD_IDENTITY();
	if(pick) {
		/* picking for mouse events */
		FW_GL_GETINTEGERV(GL_VIEWPORT,p->viewPort2);
		FW_GLU_PICK_MATRIX((float)x,(float)p->viewPort2[3]-y + tg->Mainloop.clipPlane, (float)100,(float)100,p->viewPort2);
	}

	/* ortho projection or perspective projection? */
	if (Viewer()->ortho) {
		double minX, maxX, minY, maxY;
		double numerator;

		minX = viewer->orthoField[0];
		minY = viewer->orthoField[1];
		maxX = viewer->orthoField[2];
		maxY = viewer->orthoField[3];

		if (tg->display.screenHeight != 0) {
			numerator = (maxY - minY) * ((float) tg->display.screenWidth) / ((float) tg->display.screenHeight);
			maxX = numerator/2.0f; 
			minX = -(numerator/2.0f);
		}

		FW_GL_ORTHO (minX, maxX, minY, maxY,
			viewer->nearPlane,viewer->farPlane);
		
	} else {
		/* bounds check */
		if ((fieldofview2 <= 0.0) || (fieldofview2 > 180.0)) 
			fieldofview2=45.0;
		/* glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);  */
		FW_GLU_PERSPECTIVE(fieldofview2, aspect2, viewer->nearPlane,viewer->farPlane); 
	}
	FW_GL_MATRIX_MODE(GL_MODELVIEW);

	PRINT_GL_ERROR_IF_ANY("XEvents::setup_projection");

}

/* Render the scene */
static void render() 
{
#if defined(FREEWRL_SHUTTER_GLASSES) || defined(FREEWRL_STEREO_RENDERING)
    int count,i;
	static double shuttertime;
	static int shutterside;
#endif

	ppMainloop p;
	ttglobal tg = gglobal();
	p = (ppMainloop)tg->Mainloop.prv;

#if defined(FREEWRL_SHUTTER_GLASSES) || defined(FREEWRL_STEREO_RENDERING)
	/*  profile*/
    /* double xx,yy,zz,aa,bb,cc,dd,ee,ff;*/
    /* struct timeval mytime;*/
    /* struct timezone tz; unused see man gettimeofday */

    for (count = 0; count < p->maxbuffers; count++) {

        /*set_buffer((unsigned)bufferarray[count],count); */              /*  in Viewer.c*/

		Viewer()->buffer = (unsigned)p->bufferarray[count]; 
		Viewer()->iside = count;
#ifndef GLES2
		FW_GL_DRAWBUFFER((unsigned)p->bufferarray[count]);
#endif
        /*  turn lights off, and clear buffer bits*/

		if(Viewer()->isStereo)
		{
			if(Viewer()->shutterGlasses == 2) /* flutter mode - like --shutter but no GL_STEREO so alternates */
			{
				if(TickTime() - shuttertime > 2.0)
				{
					shuttertime = TickTime();
					if(shutterside > 0) shutterside = 0;
					else shutterside = 1;
				}
				if(count != shutterside) continue;
			}
			if(Viewer()->anaglyph) //haveAnaglyphShader)
			{
				//set the channels for backbuffer clearing
				if(count == 0)
					Viewer_anaglyph_clearSides(); //clear all channels 
				else
					Viewer_anaglyph_setSide(count); //clear just the channels we're going to draw to
			}
			setup_projection(0, 0, 0);
			BackEndClearBuffer(2);
			if(Viewer()->anaglyph) 
				Viewer_anaglyph_setSide(count); //set the channels for scenegraph drawing
			setup_viewpoint(); 
		}
		else 
			BackEndClearBuffer(2);
		BackEndLightsOff();

#else

	BackEndClearBuffer(2); // no stereo, no shutter glasses: simple clear

#endif // SHUTTER GLASSES or STEREO	

	/*  turn light #0 off only if it is not a headlight.*/
	if (!fwl_get_headlight()) {
		lightState(HEADLIGHT_LIGHT,FALSE);
	}

	/*  Other lights*/
	PRINT_GL_ERROR_IF_ANY("XEvents::render, before render_hier");
	
	render_hier(rootNode(), VF_globalLight);
	PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_globalLight)");
	
	/*  4. Nodes (not the blended ones)*/
	render_hier(rootNode(), VF_Geom);
	PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_Geom)");
	
	/*  5. Blended Nodes*/
	if (tg->RenderFuncs.have_transparency) {
		/*  render the blended nodes*/
		render_hier(rootNode(), VF_Geom | VF_Blend);
		PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_Geom)");
	}
	
#if defined(FREEWRL_SHUTTER_GLASSES) || defined(FREEWRL_STEREO_RENDERING)
		if (Viewer()->isStereo) {
			cursorDraw(1,p->viewpointScreenX[count],0,0.0f); //draw a fiducial mark where centre of viewpoint is
			if (Viewer()->anaglyph)
				glColorMask(1,1,1,1); /*restore, for statusbarHud etc*/
		}
	} /* for loop */

	if (Viewer()->isStereo) {
		Viewer()->iside = Viewer()->dominantEye; /*is used later in picking to set the cursor pick box on the (left=0 or right=1) viewport*/
	}

#endif
	if(p->EMULATE_MULTITOUCH) {
        int i;
    
		for(i=0;i<20;i++)
			if(p->touchlist[i].isDown > 0)
				cursorDraw(p->touchlist[i].ID,p->touchlist[i].x,p->touchlist[i].y,p->touchlist[i].angle); 
    }
#ifndef STATUSBAR_HUD
	/* status bar, if we have one */
	drawStatusBar();

	/* swap the rendering area */
	FW_GL_SWAPBUFFERS;
        PRINT_GL_ERROR_IF_ANY("XEvents::render");
#endif
}


#if defined(GLES2)
static int currentViewerLandPort = 0;
static int rotatingCCW = FALSE;
static double currentViewerAngle = 0.0;
static double requestedViewerAngle = 0.0;
#endif // GLES2


static void setup_viewpoint() {
	

        FW_GL_MATRIX_MODE(GL_MODELVIEW); /*  this should be assumed , here for safety.*/
        FW_GL_LOAD_IDENTITY();

	#if defined(GLES2)
    
    // has a change happened? 
    if (Viewer()->screenOrientation != currentViewerLandPort) {
        // 4 possible values; 0, 90, 180, 270
        // 
        rotatingCCW = FALSE; // assume, unless told otherwise 
        switch (currentViewerLandPort) {
            case 0: {
                rotatingCCW= (Viewer()->screenOrientation == 270);
                break;
            }
            case 90: {
                rotatingCCW = (Viewer()->screenOrientation == 0);
                break;
            }
                
            case 180: {
                rotatingCCW = (Viewer()->screenOrientation != 270);
                break;
            }
                
            case 270: {
                rotatingCCW = (Viewer()->screenOrientation != 0);
                break;
                
            }
                
                
        }
        
        currentViewerLandPort = Viewer()->screenOrientation;
        requestedViewerAngle = (double)Viewer()->screenOrientation;
        
    }
    
    if (!(APPROX(currentViewerAngle,requestedViewerAngle))) {
        
        if (rotatingCCW) {
            //printf ("ccw, cva %lf req %lf\n",currentViewerAngle, requestedViewerAngle);
            currentViewerAngle -= 10.0;
            if (currentViewerAngle < -5.0) currentViewerAngle = 360.0;
        } else {
            //printf ("cw, cva %lf req %lf\n",currentViewerAngle, requestedViewerAngle);
            currentViewerAngle +=10.0;
            if (currentViewerAngle > 365.0) currentViewerAngle = 0.0; 
        }
        
    }
        FW_GL_ROTATE_D (currentViewerAngle,0.0,0.0,1.0);
        
            
	#endif // screen rotate

        viewer_togl(Viewer()->fieldofview);
        render_hier(rootNode(), VF_Viewpoint);
        PRINT_GL_ERROR_IF_ANY("XEvents::setup_viewpoint");

	/* 
	{ GLDOUBLE projMatrix[16]; 
	fw_glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	printf ("\n");
	printf ("setup_viewpoint, proj  %lf %lf %lf\n",projMatrix[12],projMatrix[13],projMatrix[14]);
	fw_glGetDoublev(GL_MODELVIEW_MATRIX, projMatrix);
	printf ("setup_viewpoint, model %lf %lf %lf\n",projMatrix[12],projMatrix[13],projMatrix[14]);
	printf ("setup_viewpoint, currentPos %lf %lf %lf\n",        Viewer.currentPosInModel.x, 
	        Viewer.currentPosInModel.y ,
	        Viewer.currentPosInModel.z);
	}
	*/
	

}

extern void dump_scene (FILE *fp, int level, struct X3D_Node* node); // in GeneratedCode.c
void dump_scenegraph()
{
#ifdef FW_DEBUG
	dump_scene(stdout, 0, (struct X3D_Node*) rootNode);
#endif
}

void sendKeyToKeySensor(const char key, int upDown);
/* handle a keypress. "man freewrl" shows all the recognized keypresses */
#ifdef _MSC_VER
#define KEYPRESS 1
#define KEYDOWN 2
#define KEYUP 3
#else
#define KEYDOWN 2
#endif

void fwl_do_keyPress(const char kp, int type) {
		int lkp;
		ttglobal tg = gglobal();
        /* does this X3D file have a KeyDevice node? if so, send it to it */
	//printf("fwl_do_keyPress: %c%d\n",kp,type); 
        if (KeySensorNodePresent()) {
                sendKeyToKeySensor(kp,type);
        } else {
#ifdef _MSC_VER
			if(type == KEYPRESS) 
#else
			if(type == KEYDOWN) 
#endif
			{
						lkp = kp;
						//if(kp>='A' && kp <='Z') lkp = tolower(kp);
                        switch (lkp) {
                                case 'e': { fwl_set_viewer_type (VIEWER_EXAMINE); break; }
                                case 'w': { fwl_set_viewer_type (VIEWER_WALK); break; }
                                case 'd': { fwl_set_viewer_type (VIEWER_FLY); break; }
                                case 'f': { fwl_set_viewer_type (VIEWER_EXFLY); break; }
                                case 'y': { fwl_set_viewer_type (VIEWER_YAWPITCHZOOM); break; }
                                case 'h': { fwl_toggle_headlight(); break;}
                                case '/': { print_viewer(); break; }
                                case '\\': { dump_scenegraph(); break; }
                                case '$': resource_tree_dump(0, tg->resources.root_res); break;
                                case '*': resource_tree_list_files(0, tg->resources.root_res); break;
                                case 'q': { if (!RUNNINGASPLUGIN) {
                                                  fwl_doQuit();
                                            }
                                            break;
                                          }
                                case 'c': { toggle_collision(); break;}
                                case 'v': {fwl_Next_ViewPoint(); break;}
                                case 'b': {fwl_Prev_ViewPoint(); break;}

#if !defined(FRONTEND_DOES_SNAPSHOTS)
                                case 's': {fwl_toggleSnapshot(); break;}
				case 'x': {Snapshot(); break;} /* thanks to luis dias mas dec16,09 */
#endif //FRONTEND_DOES_SNAPSHOTS

                                default: 
#ifdef _MSC_VER
									break;
#else
									{handle_key(kp);}
#endif
        
                        }
                } else {
#ifdef _MSC_VER
					if(type == KEYDOWN)
							{handle_key(kp);}  //keydown for fly
					if(type == KEYUP)
#endif
                        handle_keyrelease(kp); //keyup for fly
                }
        }
}



/* go to a viewpoint, hopefully it is one that is in our current list */
void fwl_gotoViewpoint (char *findThisOne) {
	int i;
	int whichnode = -1;
	struct tProdCon *t = &gglobal()->ProdCon;

	/* did we have a "#viewpoint" here? */
	if (findThisOne != NULL) {
		for (i=0; i<vectorSize(t->viewpointNodes); i++) {
			switch ((vector_get(struct X3D_Node*, t->viewpointNodes,i)->_nodeType)) {
				case NODE_Viewpoint:
					if (strcmp(findThisOne,
						X3D_VIEWPOINT(vector_get(struct X3D_Node *,t->viewpointNodes,i))->description->strptr) == 0) {
						whichnode = i;
					}
					break;


				case NODE_GeoViewpoint:
					if (strcmp(findThisOne,
						X3D_GEOVIEWPOINT(vector_get(struct X3D_Node *,t->viewpointNodes,i))->description->strptr) == 0) {
						whichnode = i;
					}
					break;

				case NODE_OrthoViewpoint:
					if (strcmp(findThisOne,
						X3D_ORTHOVIEWPOINT(vector_get(struct X3D_Node *,t->viewpointNodes,i))->description->strptr) == 0) {
						whichnode = i;
					}
					break;


			}	
		}

		
		/* were we successful at finding this one? */
		if (whichnode != -1) {
			/* set the initial viewpoint for this file */
			t->setViewpointBindInRender = vector_get(struct X3D_Node *,t->viewpointNodes,whichnode);
		}
    	}	
}

struct X3D_Node* getRayHit() {
        double x,y,z;
        int i;
		ppMainloop p;
		ttglobal tg = gglobal();
		p = (ppMainloop)tg->Mainloop.prv;

        if(tg->RenderFuncs.hitPointDist >= 0) {
			struct currayhit * rh = (struct currayhit *)tg->RenderFuncs.rayHit;
                FW_GLU_UNPROJECT(tg->RenderFuncs.hp.x,tg->RenderFuncs.hp.y,tg->RenderFuncs.hp.z,rh->modelMatrix,rh->projMatrix,viewport,&x,&y,&z);

                /* and save this globally */
                tg->RenderFuncs.ray_save_posn.c[0] = (float) x; tg->RenderFuncs.ray_save_posn.c[1] = (float) y; tg->RenderFuncs.ray_save_posn.c[2] = (float) z;

                /* we POSSIBLY are over a sensitive node - lets go through the sensitive list, and see
                   if it exists */

                /* is the sensitive node not NULL? */
                if (rh->hitNode == NULL) return NULL;
        
                
		/*
                printf ("rayhit, we are over a node, have node %p (%s), posn %lf %lf %lf",
                        rayHit.hitNode,stringNodeType(rayHit.hitNode->_nodeType), x,y,z);
                printf (" dist %f \n",rayHit.hitNode->_dist);
		*/
                

                for (i=0; i<p->num_SensorEvents; i++) {
                        if (p->SensorEvents[i].fromnode == rh->hitNode) {
                                /* printf ("found this node to be sensitive - returning %u\n",rayHit.hitNode); */
                                return ((struct X3D_Node*) rh->hitNode);
                        }
                }
        }

        /* no rayhit, or, node was "close" (scenegraph-wise) to a sensitive node, but is not one itself */
        return(NULL);
}


/* set a node to be sensitive, and record info for this node */
void setSensitive(struct X3D_Node *parentNode, struct X3D_Node *datanode) {
        void (*myp)(unsigned *);
	int i;
		ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;

        switch (datanode->_nodeType) {
                /* sibling sensitive nodes - we have a parent node, and we use it! */
                case NODE_TouchSensor: myp = (void *)do_TouchSensor; break;
                case NODE_GeoTouchSensor: myp = (void *)do_GeoTouchSensor; break;
                case NODE_PlaneSensor: myp = (void *)do_PlaneSensor; break;
                case NODE_CylinderSensor: myp = (void *)do_CylinderSensor; break;
                case NODE_SphereSensor: myp = (void *)do_SphereSensor; break;
                case NODE_ProximitySensor: /* it is time sensitive only, NOT render sensitive */ return; break;
                case NODE_GeoProximitySensor: /* it is time sensitive only, NOT render sensitive */ return; break;

                /* Anchor is a special case, as it has children, so this is the "parent" node. */
                case NODE_Anchor: myp = (void *)do_Anchor; parentNode = datanode; break;
                default: return;
        }
        /* printf ("setSensitive parentNode %p  type %s data %p type %s\n",parentNode,
                        stringNodeType(parentNode->_nodeType),datanode,stringNodeType (datanode->_nodeType)); */

	/* is this node already here? */
	/* why would it be duplicate? When we parse, we add children to a temp group, then we
	   pass things over to a rootNode; we could possibly have this duplicated */
	for (i=0; i<p->num_SensorEvents; i++) {
		if ((p->SensorEvents[i].fromnode == parentNode) &&
		    (p->SensorEvents[i].datanode == datanode) &&
		    (p->SensorEvents[i].interpptr == (void *)myp)) {
			/* printf ("setSensitive, duplicate, returning\n"); */
			return;
		}
	}

        if (datanode == 0) {
                printf ("setSensitive: datastructure is zero for type %s\n",stringNodeType(datanode->_nodeType));
                return;
        }

        /* record this sensor event for clicking purposes */
        p->SensorEvents = REALLOC(p->SensorEvents,sizeof (struct SensStruct) * (p->num_SensorEvents+1));

        /* now, put the function pointer and data pointer into the structure entry */
        p->SensorEvents[p->num_SensorEvents].fromnode = parentNode;
        p->SensorEvents[p->num_SensorEvents].datanode = datanode;
        p->SensorEvents[p->num_SensorEvents].interpptr = (void *)myp;

        /* printf ("saved it in num_SensorEvents %d\n",p->num_SensorEvents);  */
        p->num_SensorEvents++;
}

/* we have a sensor event changed, look up event and do it */
/* note, (Geo)ProximitySensor events are handled during tick, as they are time-sensitive only */
static void sendSensorEvents(struct X3D_Node* COS,int ev, int butStatus, int status) {
        int count;
		int butStatus2;
		ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;

        /* if we are not calling a valid node, dont do anything! */
        if (COS==NULL) return;

        for (count = 0; count < p->num_SensorEvents; count++) {
                if (p->SensorEvents[count].fromnode == COS) {
						butStatus2 = butStatus;
                        /* should we set/use hypersensitive mode? */
                        if (ev==ButtonPress) {
                                gglobal()->RenderFuncs.hypersensitive = p->SensorEvents[count].fromnode;
                                gglobal()->RenderFuncs.hyperhit = 0;
                        } else if (ev==ButtonRelease) {
                                gglobal()->RenderFuncs.hypersensitive = 0;
                                gglobal()->RenderFuncs.hyperhit = 0;
								butStatus2 = 1;
                        } else if (ev==MotionNotify) {
                                get_hyperhit();
                        }


                        p->SensorEvents[count].interpptr(p->SensorEvents[count].datanode, ev,butStatus2, status);
                        /* return; do not do this, incase more than 1 node uses this, eg,
                                an Anchor with a child of TouchSensor */
                }
        }
}


/* If we have a sensitive node, that is clicked and moved, get the posn
   for use later                                                                */
static void get_hyperhit() {
        double x1,y1,z1,x2,y2,z2,x3,y3,z3;
        GLDOUBLE projMatrix[16];
		struct currayhit *rhh, *rh;
		ttglobal tg = gglobal();
		rhh = (struct currayhit *)tg->RenderFuncs.rayHitHyper;
		rh = (struct currayhit *)tg->RenderFuncs.rayHit;

        FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, projMatrix);
        FW_GLU_UNPROJECT(r1.x, r1.y, r1.z, rhh->modelMatrix,
                projMatrix, viewport, &x1, &y1, &z1);
        FW_GLU_UNPROJECT(r2.x, r2.y, r2.z, rhh->modelMatrix,
                projMatrix, viewport, &x2, &y2, &z2);
        FW_GLU_UNPROJECT(tg->RenderFuncs.hp.x, tg->RenderFuncs.hp.y, tg->RenderFuncs.hp.z, rh->modelMatrix,
                projMatrix,viewport, &x3, &y3, &z3);

        /* printf ("get_hyperhit in VRMLC %f %f %f, %f %f %f, %f %f %f\n",*/
        /*      x1,y1,z1,x2,y2,z2,x3,y3,z3);*/

        /* and save this globally */
        tg->RenderFuncs.hyp_save_posn.c[0] = (float) x1; tg->RenderFuncs.hyp_save_posn.c[1] = (float) y1; tg->RenderFuncs.hyp_save_posn.c[2] = (float) z1;
        tg->RenderFuncs.hyp_save_norm.c[0] = (float) x2; tg->RenderFuncs.hyp_save_norm.c[1] = (float) y2; tg->RenderFuncs.hyp_save_norm.c[2] = (float) z2;
        tg->RenderFuncs.ray_save_posn.c[0] = (float) x3; tg->RenderFuncs.ray_save_posn.c[1] = (float) y3; tg->RenderFuncs.ray_save_posn.c[2] = (float) z3;
}

/* set stereo buffers, if required */
void setStereoBufferStyle(int itype) /*setXEventStereo()*/
{
	ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;
	if(itype==0)
	{
		/* quad buffer crystal eyes style */
		p->bufferarray[0]=GL_BACK_LEFT;
		p->bufferarray[1]=GL_BACK_RIGHT;
		p->maxbuffers=2;
	}
	else if(itype==1)
	{
		/*sidebyside and anaglyph type*/
		p->bufferarray[0]=GL_BACK;
		p->bufferarray[1]=GL_BACK;
		p->maxbuffers=2;
	}
	printf("maxbuffers=%d\n",p->maxbuffers);
}

/* go to the first viewpoint */
/* ok, is this ViewpointGroup active or not? */
int vpGroupActive(struct X3D_ViewpointGroup *vp_parent) {

	/* ok, if this is not a ViewpointGroup, we are ok */
	if (vp_parent->_nodeType != NODE_ViewpointGroup) return TRUE;

	if (vp_parent->__proxNode != NULL) {
	        /* if size == 0,,0,0 we always do the render */
	        if ((APPROX(0.0,vp_parent->size.c[0])) && (APPROX(0.0,vp_parent->size.c[1])) && (APPROX(0.0,vp_parent->size.c[2]))) {
	                printf ("size is zero\n");
	                return TRUE;
	        }

		return X3D_PROXIMITYSENSOR(vp_parent->__proxNode)->isActive;
	}
	return TRUE;
}

/* find if there is another valid viewpoint */
static int moreThanOneValidViewpoint( void) {
	int count;
	struct tProdCon *t = &gglobal()->ProdCon;

	if (vectorSize(t->viewpointNodes)<=1) return FALSE;

	for (count=0; count < vectorSize(t->viewpointNodes); count++) {
		if (count != t->currboundvpno) {
			struct Vector *me = vector_get(struct X3D_Node*, t->viewpointNodes,count)->_parentVector;

			/* ok, we have a viewpoint; is its parent a ViewpointGroup? */
			if (me != NULL) {

			    if (vectorSize(me) > 0) {
				struct X3D_Node * vp_parent;

				POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, vector_get( struct X3D_Node *,
					vector_get(struct X3D_Node *,t->viewpointNodes,count)->_parentVector, 0),
					vp_parent);
				/* printf ("parent found, it is a %s\n",stringNodeType(vp_parent->_nodeType)); */

				/* sigh, find if the ViewpointGroup is active or not */
				return vpGroupActive((struct X3D_ViewpointGroup *)vp_parent);
			   }
			}
		}
	}
	return FALSE;
}


/* go to the last viewpoint */
void fwl_Last_ViewPoint() {
	if (moreThanOneValidViewpoint()) {

		int vp_to_go_to;
		int ind;
		struct tProdCon *t = &gglobal()->ProdCon;

		/* go to the next viewpoint. Possibly, quite possibly, we might
		   have to skip one or more if they are in a ViewpointGroup that is
		   out of proxy */
		vp_to_go_to = vectorSize(t->viewpointNodes);	
		for (ind = 0; ind < vectorSize(t->viewpointNodes); ind--) {
			struct X3D_Node *cn;

			vp_to_go_to--;
                	if (vp_to_go_to<0) vp_to_go_to=vectorSize(t->viewpointNodes)-1;
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, vector_get(struct X3D_Node*, t->viewpointNodes,vp_to_go_to),cn);

			/* printf ("NVP, %d of %d, looking at %d\n",ind, totviewpointnodes,vp_to_go_to);
			printf ("looking at node :%s:\n",X3D_VIEWPOINT(cn)->description->strptr); */

			if (vpGroupActive((struct X3D_ViewpointGroup *) cn)) {
				if(0){
					/* whew, we have other vp nodes */
					send_bind_to(vector_get(struct X3D_Node*, t->viewpointNodes,t->currboundvpno),0);
					t->currboundvpno = vp_to_go_to;
					if (t->currboundvpno>=vectorSize(t->viewpointNodes)) t->currboundvpno=0;
					send_bind_to(vector_get(struct X3D_Node*, t->viewpointNodes,t->currboundvpno),1);

				}else{
					/* dug9 - using the display-thread-synchronous gotoViewpoint style
						to help order-senstive slerp_viewpoint() process */
					/* set the initial viewpoint for this file */
					t->setViewpointBindInRender = vector_get(struct X3D_Node*, 
						t->viewpointNodes,vp_to_go_to);
					t->currboundvpno = vp_to_go_to;
					if (t->currboundvpno>=vectorSize(t->viewpointNodes)) t->currboundvpno=0;
				}
			return;
			}
		}
        }
}



/* go to the first viewpoint */
void fwl_First_ViewPoint() {
	if (moreThanOneValidViewpoint()) {

		int vp_to_go_to;
		int ind;
		struct tProdCon *t = &gglobal()->ProdCon;

		/* go to the next viewpoint. Possibly, quite possibly, we might
		   have to skip one or more if they are in a ViewpointGroup that is
		   out of proxy */
		vp_to_go_to = -1;	
		for (ind = 0; ind < vectorSize(t->viewpointNodes); ind++) {
			struct X3D_Node *cn;

			vp_to_go_to++;
                	if (vp_to_go_to<0) vp_to_go_to=vectorSize(t->viewpointNodes)-1;
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, vector_get(
				struct X3D_Node* , t->viewpointNodes,vp_to_go_to),cn);

			/* printf ("NVP, %d of %d, looking at %d\n",ind, totviewpointnodes,vp_to_go_to);
			printf ("looking at node :%s:\n",X3D_VIEWPOINT(cn)->description->strptr); */

			if (vpGroupActive((struct X3D_ViewpointGroup *) cn)) {
				if(0){
                	/* whew, we have other vp nodes */
                	send_bind_to(vector_get(struct X3D_Node*,t->viewpointNodes,t->currboundvpno),0);
                	t->currboundvpno = vp_to_go_to;
                	if (t->currboundvpno>=vectorSize(t->viewpointNodes)) t->currboundvpno=0;
                	send_bind_to(vector_get(struct X3D_Node*,t->viewpointNodes,t->currboundvpno),1);
				}else{
					/* dug9 - using the display-thread-synchronous gotoViewpoint style
						to help order-senstive slerp_viewpoint() process */
					/* set the initial viewpoint for this file */
					t->setViewpointBindInRender = vector_get(struct X3D_Node*,t->viewpointNodes,vp_to_go_to);
                	t->currboundvpno = vp_to_go_to;
                	if (t->currboundvpno>=vectorSize(t->viewpointNodes)) t->currboundvpno=0;

				}

			return;
			}
		}
        }
}
/* go to the next viewpoint */
void fwl_Prev_ViewPoint() {
	if (moreThanOneValidViewpoint()) {

		int vp_to_go_to;
		int ind;
		struct tProdCon *t = &gglobal()->ProdCon;

		/* go to the next viewpoint. Possibly, quite possibly, we might
		   have to skip one or more if they are in a ViewpointGroup that is
		   out of proxy */
		vp_to_go_to = t->currboundvpno;	
		for (ind = 0; ind < vectorSize(t->viewpointNodes); ind--) {
			struct X3D_Node *cn;

			vp_to_go_to--;
                	if (vp_to_go_to<0) vp_to_go_to=vectorSize(t->viewpointNodes)-1;
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, vector_get(struct X3D_Node*, t->viewpointNodes,vp_to_go_to),cn);

			/* printf ("NVP, %d of %d, looking at %d\n",ind, totviewpointnodes,vp_to_go_to);
			printf ("looking at node :%s:\n",X3D_VIEWPOINT(cn)->description->strptr); */

			if (vpGroupActive((struct X3D_ViewpointGroup *) cn)) {

				if(0){
                	/* whew, we have other vp nodes */
                	send_bind_to(vector_get(struct X3D_Node*,t->viewpointNodes,t->currboundvpno),0);
                	t->currboundvpno = vp_to_go_to;
                	if (t->currboundvpno>=vectorSize(t->viewpointNodes)) t->currboundvpno=0;
                	send_bind_to(vector_get(struct X3D_Node*,t->viewpointNodes,t->currboundvpno),1);
				}else{
					/* dug9 - using the display-thread-synchronous gotoViewpoint style
						to help order-senstive slerp_viewpoint() process */
					/* set the initial viewpoint for this file */
					t->setViewpointBindInRender = vector_get(struct X3D_Node*,
						t->viewpointNodes,vp_to_go_to);
                	t->currboundvpno = vp_to_go_to;
                	if (t->currboundvpno>=vectorSize(t->viewpointNodes)) t->currboundvpno=0;
				}


			return;
			}
		}
        }
}

/* go to the next viewpoint */
void fwl_Next_ViewPoint() {
	if (moreThanOneValidViewpoint()) {

		int vp_to_go_to;
		int ind;
		struct tProdCon *t = &gglobal()->ProdCon;

		/* go to the next viewpoint. Possibly, quite possibly, we might
		   have to skip one or more if they are in a ViewpointGroup that is
		   out of proxy */
		vp_to_go_to = t->currboundvpno;	
		for (ind = 0; ind < vectorSize(t->viewpointNodes); ind++) {
			struct X3D_Node *cn;

			vp_to_go_to++;
                	if (vp_to_go_to>=vectorSize(t->viewpointNodes)) vp_to_go_to=0;
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, vector_get(
				struct X3D_Node*, t->viewpointNodes,vp_to_go_to),cn);

			/* printf ("NVP, %d of %d, looking at %d\n",ind, totviewpointnodes,vp_to_go_to);
			printf ("looking at node :%s:\n",X3D_VIEWPOINT(cn)->description->strptr); */

			if (vpGroupActive((struct X3D_ViewpointGroup *) cn)) {
                		/* whew, we have other vp nodes */
				/* dug9 - using the display-thread-synchronous gotoViewpoint style
					to help order-senstive slerp_viewpoint() process */
				/* set the initial viewpoint for this file */
				t->setViewpointBindInRender = vector_get(
					struct X3D_Node*,t->viewpointNodes,vp_to_go_to);
                		t->currboundvpno = vp_to_go_to;
                		if (t->currboundvpno>=vectorSize(t->viewpointNodes)) t->currboundvpno=0;

			return;
			}
		}
        }
}

/* initialization for the OpenGL render, event processing sequence. Should be done in threat that has the OpenGL context */
void fwl_initializeRenderSceneUpdateScene() {
    
#ifndef AQUA
	ttglobal tg = gglobal();
#endif
    
	/* printf ("fwl_initializeRenderSceneUpdateScene start\n"); */

#if KEEP_X11_INLIB
	/* Hmm. display_initialize is really a frontend function. The frontend should call it before calling fwl_initializeRenderSceneUpdateScene */
	/* Initialize display */
	if (!fv_display_initialize()) {
	       ERROR_MSG("initFreeWRL: error in display initialization.\n");
	       exit(1);
	}
#endif /* KEEP_X11_INLIB */

	new_tessellation();

	fwl_set_viewer_type(VIEWER_EXAMINE);
	
	viewer_postGLinit_init();

	#ifndef AQUA
	if (tg->display.fullscreen && newResetGeometry != NULL) newResetGeometry();
	#endif

	/* printf ("fwl_initializeRenderSceneUpdateScene finish\n"); */
	// on OSX, this function is not called by the thread that holds the OpenGL
	// context. Unsure if only Windows can do this one, but for now,
	// do NOT do this on OSX. 
#ifndef TARGET_AQUA
	drawStatusBar(); //just to get it initialized
#endif
}

void finalizeRenderSceneUpdateScene() {
    	kill_oldWorld(TRUE,TRUE,__FILE__,__LINE__);
}


/* iphone front end handles the displayThread internally */
#ifndef FRONTEND_HANDLES_DISPLAY_THREAD
/* handle all the displaying and event loop stuff. */
void updateButtonStatus()
{
	//checks collision, headlight and navmode 
	//-these can be set by either the UI (this statusbar), keyboard hits, or from 
	// events inside vrml. 
	// Here we take our UI current state from the scene state. 
	// For FRONTEND_HANDLES_DISPLAY_THREAD configurations, the frontend should do 
	// the equivalent of the following once per frame (poll state and set UI)
	int headlight, collision, navmode;
	//poll model state:
	headlight = fwl_get_headlight();
	collision = fwl_getCollision();
	navmode = fwl_getNavMode();
	//update UI(view):
	setMenuButton_navModes(navmode);
	setMenuButton_headlight(headlight);
	setMenuButton_collision(collision);
}
int android_get_unread_message_count();
char *android_get_last_message(int whichOne); 
void hudSetConsoleMessage(char *buffer);
#if defined(STATUSBAR_HUD)
void updateConsoleStatus()
{
	//polls ConsoleMessage.c for accumulated messages and updates statusbarHud.c via hudSetConsoleMessage
	int nlines,i;
	char *buffer;
	nlines = android_get_unread_message_count(); //poll model
	for(i=0;i<nlines;i++)
	{
		buffer = android_get_last_message(nlines-i-1); //poll model
		hudSetConsoleMessage(buffer); //update UI(view)
		free(buffer);
	}
}
#endif
void checkFileLoadRequest()
{
	/* Checks flags and pops up a dialog for the user to enter a new 
	   scene URL or File path.
		I heard Android calls from UI thread:
		fwl_Android_replaceWorldNeeded();
	   then calls from render thread:
	    fwl_initializeRenderSceneUpdateScene();
		pthread_create(&loadFileThread, NULL, (void*)fileLoadThread, (void*)currentFile) 
		fv_display_initialize();
	   In this native case for statusbarHud UI (which is in the rendering thread)
	   the UI flags that it wants the dialog box to come up in the rendering thread. 
	   Then the dialog and replaceworld are called from rendering thread here.
	*/

	char *fname;
	ttglobal tg = gglobal();
	fname = NULL;

	//poll state:
	if(fwl_pollPromptForURL())
	{
		fwl_setPromptForURL(0);
		#if defined(_MSC_VER) || defined(QNX)
			fname = frontend_pick_URL();
		#endif
	}
	else if(fwl_pollPromptForFile())
	{
		fwl_setPromptForFile(0);
		#if defined(_MSC_VER) || defined(QNX)
			fname = frontend_pick_file();
		#endif
	}
	// update 
	if(fname)
	{
		kill_oldWorld(TRUE,TRUE,__FILE__,__LINE__);
		Anchor_ReplaceWorld(fname);
		free(fname);
	}

}
void _displayThread()
{
	ENTER_THREAD("display");

#if KEEP_FV_INLIB
	/* Hmm. display_initialize is really a frontend function. The frontend should call it before calling _displayThread */
	/* Initialize display */
	if (!fv_display_initialize()) {
		ERROR_MSG("initFreeWRL: error in display initialization.\n");
		exit(1);
	}
#endif /* KEEP_FV_INLIB */

	fwl_initializeRenderSceneUpdateScene();
	/* loop and loop, and loop... */
	while (!((ppMainloop)(gglobal()->Mainloop.prv))->quitThread) {
		//PRINTF("event loop\n");
		fwl_RenderSceneUpdateScene();
		//Controller code
		//-MVC - Model-View-Controller design pattern: do the Controller poll-model-and-update-UI here
		//-reason for MVC: put controller in UI(view) technology so no callbacks are needed from Model to UI(View)
		//-here everything is in C so we don't need MVC style, but we are preparing MVC in C
		// to harmonize with Android, IOS etc where the UI(View) and Controller are in Objective-C or Java and Model(state) in C
		updateButtonStatus(); //poll Model & update UI(View)
#if defined(STATUSBAR_HUD)
		updateConsoleStatus(); //poll Model & update UI(View)
#endif
		checkFileLoadRequest();
		/* status bar, if we have one */
		drawStatusBar();  // UI/View 

		/* swap the rendering area */
		FW_GL_SWAPBUFFERS;
			PRINT_GL_ERROR_IF_ANY("XEvents::render");
	} 
	/* when finished: */
	finalizeRenderSceneUpdateScene();

}
#endif /* FRONTEND_HANDLES_DISPLAY_THREAD */


void fwl_setLastMouseEvent(int etype) {
	ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;
	//printf ("fwl_setLastMouseEvent called\n");
        p->lastMouseEvent = etype;
}

void fwl_initialize_parser()
{
	/* JAS 
		if (gglobal() == NULL) ConsoleMessage ("fwl_initialize_parser, gglobal() NULL");
		if ((gglobal()->Mainloop.prv) == NULL) ConsoleMessage ("fwl_initialize_parser, gglobal()->Mainloop.prv NULL");
	*/

        ((ppMainloop)(gglobal()->Mainloop.prv))->quitThread = FALSE;

	/* create the root node */
	if (rootNode() == NULL) {
		setRootNode( createNewX3DNode (NODE_Group) );
		/*remove this node from the deleting list*/
		doNotRegisterThisNodeForDestroy(X3D_NODE(rootNode()));
	}
}

void fwl_init_SnapSeq() {
#ifdef DOSNAPSEQUENCE
/* need to re-implement this for OSX generating QTVR */
        set_snapsequence(TRUE);
#endif
}


void fwl_set_LineWidth(float lwidth) {
        gglobal()->Mainloop.gl_linewidth = lwidth;
}

void fwl_set_KeyString(const char* kstring)
{
	ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;
    p->keypress_string = strdup(kstring);
}

/* if we had an exit(EXIT_FAILURE) anywhere in this C code - it means
   a memory error. So... print out a standard message to the
   console. */
void outOfMemory(const char *msg) {
        ConsoleMessage ("FreeWRL has encountered a memory allocation problem\n"\
                        "and is exiting. -- %s--",msg);
        usleep(10 * 1000);
        exit(EXIT_FAILURE);
}

#if defined (_ANDROID)

// we are loading a new file, or just not visible anymore
void fwl_Android_doQuitInstance()
{

    kill_oldWorld(TRUE,TRUE,__FILE__,__LINE__); //must be done from this thread
	stopLoadThread();
	stopPCThread();

    /* set geometry to normal size from fullscreen */
    if (newResetGeometry != NULL) newResetGeometry();

    /* kill any remaining children */
    killErrantChildren();
    
#ifdef DEBUG_MALLOC
    void scanMallocTableOnQuit(void);
    scanMallocTableOnQuit();
#endif
	/* tested on win32 console program July9,2011 seems OK */
	iglobal_destructor(gglobal());
}
#else
void fwl_doQuitInstance()
{

#if !defined(FRONTEND_HANDLES_DISPLAY_THREAD)
    	stopDisplayThread();
#endif

    kill_oldWorld(TRUE,TRUE,__FILE__,__LINE__); //must be done from this thread
	stopLoadThread();
	stopPCThread();

    /* set geometry to normal size from fullscreen */
#ifndef AQUA
    if (newResetGeometry != NULL) newResetGeometry();
#endif

    /* kill any remaining children */
    killErrantChildren();
    
#ifdef DEBUG_MALLOC
    void scanMallocTableOnQuit(void);
    scanMallocTableOnQuit();
#endif
	/* tested on win32 console program July9,2011 seems OK */
	iglobal_destructor(gglobal());

}
#endif //ANDROID


/* quit key pressed, or Plugin sends SIGQUIT */
void fwl_doQuit()
{
#if defined(_ANDROID)
	fwl_Android_doQuitInstance();
#else //ANDROID
	fwl_doQuitInstance();
#endif //ANDROID
    exit(EXIT_SUCCESS);
}

// tmp files are on a per-invocation basis on Android, and possibly other locations.
// note that the "tempnam" function will accept NULL as the directory on many platforms,
// so this function does not really need to be called on many platforms.
void fwl_tmpFileLocation(char *tmpFileLocation) {
	ttglobal tg;
	if (tmpFileLocation == NULL) return;
	tg = gglobal();
	FREE_IF_NZ(tg->Mainloop.tmpFileLocation);
	tg->Mainloop.tmpFileLocation = MALLOC(char *,strlen(tmpFileLocation)+1);
	strcpy(tg->Mainloop.tmpFileLocation,tmpFileLocation);
}

void close_internetHandles();
int iglobal_instance_count();
void fwl_closeGlobals()
{
	//"last one out shut off the lights"
	//when there are no freewrl iglobal instances left, then call this to shut
	//down anything that's of per-process / per-application / static-global-shared
	//dug9 - not used yet as of Aug 3, 2011
	//if you call from the application main thread / message pump ie on_key > doQuit
	//then in theory there should be a way to iterate through all 
	//instances, quitting each one in a nice way, say on freewrlDie or 
	//(non-existant yet) doQuitAll or doQuitInstanceOrAllIfNoneLeft
	//for i = 1 to iglobal_instance_count
	//  set instance through window handle or index (no function yet to
	//       get window handle by index, or set instance by index )
	//  fwl_doQuitInstance
	//then call fwl_closeGlobals
	if(iglobal_instance_count() == 0)
	{
		close_internetHandles();
		//console window?
	}
}
void freewrlDie (const char *format) {
        ConsoleMessage ("Catastrophic error: %s\n",format);
        fwl_doQuit();
}


#if defined(AQUA) || defined(_MSC_VER) || defined(GLES2)

//int ntouch =0;
//int currentTouch = -1;
/* MIMIC what happens in handle_Xevents, but without the X events */
void fwl_handle_aqua_multi(const int mev, const unsigned int button, int x, int y, int ID) {
        int count;
		ppMainloop p;
		ttglobal tg = gglobal();
		p = (ppMainloop)tg->Mainloop.prv;

  /* printf ("fwl_handle_aqua in MainLoop; but %d x %d y %d screenWidth %d screenHeight %d",
                button, x,y,tg->display.screenWidth,tg->display.screenHeight);  
        if (mev == ButtonPress) printf ("ButtonPress\n");
        else if (mev == ButtonRelease) printf ("ButtonRelease\n");
        else if (mev == MotionNotify) printf ("MotionNotify\n");
        else printf ("event %d\n",mev); */

        /* save this one... This allows Sensors to get mouse movements if required. */
        p->lastMouseEvent = mev;
        /* save the current x and y positions for picking. */
		tg->Mainloop.currentX[p->currentCursor] = x;
		tg->Mainloop.currentY[p->currentCursor] = y;
		p->touchlist[ID].x = x;
		p->touchlist[ID].y = y;
		p->touchlist[ID].button = button;
		p->touchlist[ID].isDown = (mev == ButtonPress || mev == MotionNotify);
		p->touchlist[ID].ID = ID; /*will come in handy if we change from array[] to accordian list*/
		p->touchlist[ID].mev = mev;
		p->touchlist[ID].angle = 0.0f;
		p->currentTouch = ID;


		if( handleStatusbarHud(mev, &tg->Mainloop.clipPlane) )return; /* statusbarHud options screen should swallow mouse clicks */

        if ((mev == ButtonPress) || (mev == ButtonRelease)) {
                /* record which button is down */
                p->ButDown[p->currentCursor][button] = (mev == ButtonPress);
                /* if we are Not over an enabled sensitive node, and we do NOT already have a 
                   button down from a sensitive node... */

                if ((p->CursorOverSensitive ==NULL) && (p->lastPressedOver ==NULL)) {
                        p->NavigationMode=p->ButDown[p->currentCursor][1] || p->ButDown[p->currentCursor][3];
                        handle(mev, button, (float) ((float)x/tg->display.screenWidth), (float) ((float)y/tg->display.screenHeight));
                }
        }

        if (mev == MotionNotify) {
                /* save the current x and y positions for picking. */
                // above currentX[currentCursor] = x;
                //currentY[currentCursor] = y;

                if (p->NavigationMode) {
                        /* find out what the first button down is */
                        count = 0;
                        while ((count < 8) && (!p->ButDown[p->currentCursor][count])) count++;
                        if (count == 8) return; /* no buttons down???*/

                        handle (mev, (unsigned) count, (float) ((float)x/tg->display.screenWidth), (float) ((float)y/tg->display.screenHeight));
                }
        }
}
//int lastDeltax = 50;
//int lastDeltay = 50;
//int lastxx;
//int lastyy;
void emulate_multitouch(const int mev, const unsigned int button, int x, int y)
{
	/* goal: when MMB draw a slave cursor pinned to last_distance,last_angle from real cursor 
		Note: if using a RMB+LMB = MMB chord with 2 button mice, you need to emulate in your code
			and pass in button 2 here, after releasing your single button first ie:
			fwl_handle_aqua(ButtonRelease, 1, x, y); 
			fwl_handle_aqua(ButtonRelease, 3, x, y); 
	*/
	if( button == 2 ) 
	{
		ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;
		if( mev == ButtonPress )
		{
			p->lastxx = x - p->lastDeltax;
			p->lastyy = y - p->lastDeltay;
		}else if(mev == MotionNotify || mev == ButtonRelease ){
			p->lastDeltax = x - p->lastxx;
			p->lastDeltay = y - p->lastyy;
		}
		fwl_handle_aqua_multi(mev, 1, x, y, 0);
		fwl_handle_aqua_multi(mev, 1, p->lastxx, p->lastyy, 1);
	}else{
		/* normal, no need to emulate if there's no MMB or LMB+RMB */
		fwl_handle_aqua_multi(mev,button,x,y,0);
	}
}
/* old function should still work, with single mouse and ID=0 */
void fwl_handle_aqua(const int mev, const unsigned int button, int x, int y) {
    ttglobal tg = gglobal();

	/* printf ("fwl_handle_aqua, type %d, screen wid:%d height:%d, orig x,y %d %d\n",
            mev,tg->display.screenWidth, tg->display.screenHeight,x,y); */

	// do we have to worry about screen orientations (think mobile devices)
	#if defined (IPHONE) || defined (_ANDROID) // || defined(GLES2)
	{ 

        // iPhone - with home button on bottom, in portrait mode, 
        // top left hand corner is x=0, y=0; 
        // bottom left, 0, 468)
        // while standard opengl is (0,0) in lower left hand corner...
		int ox = x;
		int oy = y;

		// these make sense for walk navigation
		if (Viewer()->type == VIEWER_WALK) {
			switch (Viewer()->screenOrientation) {
				case 0: 
					x = tg->display.screenHeight-x;
	                
					break;
				case 90: 
					x = oy;
					y = ox;
					break;
				case 180:
					x = x;
					y = -y;
					break;
				case 270:
					x = tg->display.screenWidth - oy;
					y = tg->display.screenHeight - ox;
					break;
				default:{}
			}

		// these make sense for examine navigation
		} else if (Viewer()->type == VIEWER_EXAMINE) {
			switch (Viewer()->screenOrientation) {
				case 0: 
					break;
				case 90: 
					x = tg->display.screenWidth - oy;
					y = ox;
					break;
				case 180:
					x = tg->display.screenWidth -x;
					y = tg->display.screenHeight -y;
					break;
				case 270:
					// nope x = tg->display.screenWidth - oy;
					// nope y = tg->display.screenHeight - ox;

					x = tg->display.screenHeight - oy;
					y = tg->display.screenWidth - ox;

					//printf ("resulting in x %d  y %d\n",x,y);
					break;
				default:{}
			}

		}
	}

	#endif

	if(((ppMainloop)(tg->Mainloop.prv))->EMULATE_MULTITOUCH)
		emulate_multitouch(mev,button,x, y);
	else
	{
		fwl_handle_aqua_multi(mev,button,x,y,0);

		updateCursorStyle(); 
	}
}

#endif

void fwl_setCurXY(int cx, int cy) {
	ttglobal tg = gglobal();
	ppMainloop p = (ppMainloop)tg->Mainloop.prv;
	/* printf ("fwl_setCurXY, have %d %d\n",p->currentX[p->currentCursor],p->currentY[p->currentCursor]); */
        tg->Mainloop.currentX[p->currentCursor] = cx;
        tg->Mainloop.currentY[p->currentCursor] = cy;
}

void fwl_setButDown(int button, int value) {
	ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;
	/* printf ("fwl_setButDown called\n"); */
        p->ButDown[p->currentCursor][button] = value;
}


/* mobile devices - set screen orientation */
/* "0" is "normal" orientation; degrees clockwise; note that face up and face down not 
   coded; assume only landscape/portrait style orientations */

void fwl_setOrientation (int orient) {
	switch (orient) {
		case 0: 
		case 90:
		case 180:
		case 270:
			{
			Viewer()->screenOrientation = orient;
			break;
		}
		default: {
			ConsoleMessage ("invalid orientation %d\n",orient);
			Viewer()->screenOrientation = 0;
		}
	}
}

	

void setIsPlugin() {

        RUNNINGASPLUGIN = TRUE;
                
        // Save local working directory
        /*
        {
        FILE* tmpfile;
        char tmppath[512];
        system("pwd > /tmp/freewrl_filename");
        tmpfile = fopen("/tmp/freewrl_filename", "r");
        
        if (tmpfile) {
                fgets(tmppath, 512, tmpfile);
        }
        BrowserFullPath = STRDUP(tmppath);      
        fclose(tmpfile);
        //system("rm /tmp/freewrl_filename");   
        tmpfile = fopen("/tmp/after", "w");
        if (tmpfile) {
                fprintf(tmpfile, "%s\n", BrowserFullPath);
        }
        fclose(tmpfile);
        }
        */
        
}

#ifdef AQUA

int aquaPrintVersion() {
	printf ("FreeWRL version: %s\n", libFreeWRL_get_version()); 
	exit(EXIT_SUCCESS);
}

#endif

/* if we are visible, draw the OpenGL stuff, if not, don not bother */
void setDisplayed (int state) {
	ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;

        #ifdef VERBOSE
        if (state) printf ("WE ARE DISPLAYED\n");
        else printf ("we are now iconic\n");
        #endif
        p->onScreen = state;
}

void fwl_init_EaiVerbose() {
        //eaiverbose = TRUE;
#if !defined(EXCLUDE_EAI)
	gglobal()->EAI_C_CommonFunctions.eaiverbose = TRUE;
	fwlio_RxTx_control(CHANNEL_EAI, RxTx_MOREVERBOSE); /* RxTx_SILENT */
#endif

}

#if defined (_ANDROID)

void fwl_Android_replaceWorldNeeded() {
	// ConsoleMessage ("remove old world, but leave threads intact");
	rootNode()->children.n = 0;



	int i;
	#ifndef AQUA
        char mystring[20];
	#endif
	struct VRMLParser *globalParser = (struct VRMLParser *)gglobal()->CParse.globalParser;

	/* get rid of sensor events */
	resetSensorEvents();


	/* make the root_res equal NULL - this throws away all old resource info */

	gglobal()->resources.root_res = NULL;

	Android_reset_viewer_to_defaults();
        struct tProdCon *t = &gglobal()->ProdCon;
	// if we have a bound vp; if the old world did not have a vp, there will be nothing to send_bind_to
	if (vectorSize(t->viewpointNodes) > t->currboundvpno) {
		send_bind_to(vector_get(struct X3D_Node*, t->viewpointNodes,t->currboundvpno),0);
	}


	/* mark all rootNode children for Dispose */
	for (i=0; i<rootNode()->children.n; i++) {
		markForDispose(rootNode()->children.p[i], TRUE);
	}

	/* stop rendering */
	rootNode()->children.n = 0;

	/* close the Console Message system, if required. */
	closeConsoleMessage();

	/* occlusion testing - zero total count, but keep MALLOC'd memory around */
	zeroOcclusion();

	/* clock events - stop them from ticking */
	kill_clockEvents();

	/* kill DEFS, handles */
	//if we do this here, we have a problem, as the parser is already killed and cleaned up.
	//EAI_killBindables();


	kill_bindables();
	killKeySensorNodeList();


	/* stop routing */
	kill_routing();

	/* tell the statusbar that it needs to reinitialize */
	kill_status();

	/* free scripts */
	#ifdef HAVE_JAVASCRIPT
	kill_javascript();
	#endif

	#if !defined(EXCLUDE_EAI)
	/* free EAI */
	if (kill_EAI) {
	       	/* shutdown_EAI(); */
		fwlio_RxTx_control(CHANNEL_EAI, RxTx_STOP) ;
	}
	#endif //EXCLUDE_EAI

	#ifndef AQUA
		sprintf (mystring, "QUIT");
		Sound_toserver(mystring);
	#endif


	/* reset any VRML Parser data */
	if (globalParser != NULL) {
		parser_destroyData(globalParser);
		//globalParser = NULL;
		gglobal()->CParse.globalParser = NULL;
	}

	kill_X3DDefs();

	/* tell statusbar that we have none */
	viewer_default();
	setMenuStatus("NONE");
}
#endif


#if !defined(_ANDROID)

// JAS - Do not know if these are still required.

/* called from the standalone OSX front end and the OSX plugin */
void fwl_replaceWorldNeeded(char* str)
{
	ConsoleMessage ("replaceWorldNeeded called");

#ifdef OLDCODE
OLDCODE ttglobal tg = gglobal();
OLDCODE setAnchorsAnchor( NULL );
OLDCODE FREE_IF_NZ(tg->RenderFuncs.OSX_replace_world_from_console);
OLDCODE tg->RenderFuncs.OSX_replace_world_from_console = STRDUP(str);
OLDCODE tg->RenderFuncs.BrowserAction = TRUE;
OLDCODE FREE_IF_NZ(tg->RenderFuncs.OSX_last_world_url_for_reload);
OLDCODE tg->RenderFuncs.OSX_last_world_url_for_reload = STRDUP(str);
#endif //OLDCODE
}


void fwl_reload()
{
	ConsoleMessage("fwl_reload called");
#ifdef OLDCODE
OLDCODE char *oldworld;
OLDCODE ttglobal tg = gglobal();
OLDCODE 
OLDCODE oldworld = STRDUP(tg->RenderFuncs.OSX_last_world_url_for_reload);
OLDCODE fwl_replaceWorldNeeded(oldworld);
OLDCODE FREE_IF_NZ(oldworld);
#endif //OLDCODE
}

#endif //NOT _ANDROID


/* OSX the Plugin is telling the displayThread to stop and clean everything up */
void stopRenderingLoop(void) {
	ttglobal tg = gglobal();
	//printf ("stopRenderingLoop called\n");

#if !defined(FRONTEND_HANDLES_DISPLAY_THREAD)
    	stopDisplayThread();
#endif

    	//killErrantChildren();
	/* lets do an equivalent to replaceWorldNeeded, but with NULL for the new world */

        setAnchorsAnchor( NULL );
        tg->RenderFuncs.BrowserAction = TRUE;
	#ifdef OLDCODE
        OLDCODE FREE_IF_NZ(tg->RenderFuncs.OSX_replace_world_from_console);
	#endif //OLDCODE
	// printf ("stopRenderingLoop finished\n");
}


/* send the description to the statusbar line */
void sendDescriptionToStatusBar(struct X3D_Node *CursorOverSensitive) {
        int tmp;
        char *ns;
		ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;

        if (CursorOverSensitive == NULL) update_status (NULL);
        else {

                ns = NULL;
                for (tmp=0; tmp<p->num_SensorEvents; tmp++) {
                        if (p->SensorEvents[tmp].fromnode == CursorOverSensitive) {
                                switch (p->SensorEvents[tmp].datanode->_nodeType) {
                                        case NODE_Anchor: ns = ((struct X3D_Anchor *)p->SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_PlaneSensor: ns = ((struct X3D_PlaneSensor *)p->SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_SphereSensor: ns = ((struct X3D_SphereSensor *)p->SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_TouchSensor: ns = ((struct X3D_TouchSensor *)p->SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_GeoTouchSensor: ns = ((struct X3D_GeoTouchSensor *)p->SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_CylinderSensor: ns = ((struct X3D_CylinderSensor *)p->SensorEvents[tmp].datanode)->description->strptr; break;
                                        default: {printf ("sendDesc; unknown node type %d\n",p->SensorEvents[tmp].datanode->_nodeType);}
                                }
                                /* if there is no description, put the node type on the screen */
                                if (ns == NULL) {ns = "(over sensitive)";}
                                else if (ns[0] == '\0') ns = (char *)stringNodeType(p->SensorEvents[tmp].datanode->_nodeType);
        
                                /* send this string to the screen */
                                update_status(ns);
                        }
                }
        }
}


/* We have a new file to load, lets get rid of the old world sensor events, and run with it */
void resetSensorEvents(void) {
	ppMainloop p = (ppMainloop)gglobal()->Mainloop.prv;

	if (p->oldCOS != NULL) 	
		sendSensorEvents(p->oldCOS,MapNotify,p->ButDown[p->currentCursor][1], FALSE);
       /* remove any display on-screen */
       sendDescriptionToStatusBar(NULL);
	p->CursorOverSensitive=NULL; 

	p->oldCOS=NULL;
	p->lastMouseEvent = 0;
	p->lastPressedOver = NULL;
	p->lastOver = NULL;
	FREE_IF_NZ(p->SensorEvents);
	p->num_SensorEvents = 0;
	gglobal()->RenderFuncs.hypersensitive = NULL;
	gglobal()->RenderFuncs.hyperhit = 0;
	/* Cursor - ensure it is not the "sensitive" cursor */
/*	ARROW_CURSOR; */

}

