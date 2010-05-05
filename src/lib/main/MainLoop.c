/*
  $Id: MainLoop.c,v 1.121 2010/05/05 11:28:08 davejoubert Exp $

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
#include "../scenegraph/RenderFuncs.h"

#include "../scenegraph/Viewer.h"
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"
#include "../input/EAIHeaders.h"

#include "../scenegraph/Component_KeyDevice.h"	/* resolving implicit declarations */
#include "../opengl/Frustum.h"
#include "../input/InputFunctions.h"

#include "../opengl/OpenGL_Utils.h"
#include "../ui/statusbar.h"

#ifdef AQUA
#include "../ui/aquaInt.h"
#endif
#ifdef IPHONE
int ccurse;
int ocurse;
void  setAquaCursor(int ctype) { };
typedef struct
{
   // Handle to a program object
   GLuint programObject;

} UserData;
typedef struct
{
    GLfloat   m[4][4];
} ESMatrix;

typedef struct
{
   /// Put your user data here...
   void*       userData;

   /// Window width
   GLint       width;

   /// Window height
   GLint       height;

} ESContext;

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#include "MainLoop.h"


#define IS_WORLD_LOADED ((root_res != NULL) && (root_res->status == ress_parsed))
/* extern int isURLLoaded(void);	/\* initial scene loaded? Robert Sim *\/ */

/* are we displayed, or iconic? */
static int onScreen = TRUE;

/* Coordinate screen refresh with aqua */
static int askForRefresh = FALSE;
static int refreshOK = FALSE;

/* do we do event propagation, proximity calcs?? */
static int doEvents = FALSE;

#ifdef VERBOSE
static char debs[300];
#endif

char* PluginFullPath;

/* linewidth for lines and points - passed in on command line */
float gl_linewidth = 1.0f;

/* what kind of file was just parsed? */
int currentFileVersion = 0;

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
   ("have scripts to initialize in EventLoop old %d new
   %d\n",max_script_found, max_script_found_and_initialized);
*/

#define INITIALIZE_ANY_SCRIPTS \
        if (max_script_found != max_script_found_and_initialized) { \
                int i; jsval retval; \
                for (i=max_script_found_and_initialized+1; i <= max_script_found; i++) { \
                        /* printf ("initializing script %d in thread %u\n",i,pthread_self());  */ \
                        JSCreateScriptContext(i); \
                        JSInitializeScriptAndFields(i); \
			if (ScriptControl[i].scriptOK) ACTUALRUNSCRIPT(i, "initialize()" ,&retval); \
                        /* printf ("initialized script %d\n",i);*/  \
                } \
                max_script_found_and_initialized = max_script_found; \
        }

/* we bind bindable nodes on parse in this thread */
#define SEND_BIND_IF_REQUIRED(node) \
                if (node != NULL) { send_bind_to(X3D_NODE(node),1); node = NULL; }


int quitThread = FALSE;
char * keypress_string=NULL;            /* Robert Sim - command line key sequence */
int keypress_wait_for_settle = 100;     /* JAS - change keypress to wait, then do 1 per loop */
extern int viewer_initialized;

void Next_ViewPoint(void);              /*  switch to next viewpoint -*/
static void setup_viewpoint();
static void get_collisionoffset(double *x, double *y, double *z);

/* Sensor table. When clicked, we get back from getRayHit the fromnode,
        have to look up type and data in order to properly handle it */
struct SensStruct {
        struct X3D_Node *fromnode;
        struct X3D_Node *datanode;
        void (*interpptr)(void *, int, int, int);
};
struct SensStruct *SensorEvents = 0;
int num_SensorEvents = 0;

/* Viewport data */
static GLint viewPort2[10];

/* screen width and height. */
int clipPlane = 0;
struct X3D_Node* CursorOverSensitive=NULL;      /*  is Cursor over a Sensitive node?*/
struct X3D_Node* oldCOS=NULL;                   /*  which node was cursor over before this node?*/
int NavigationMode=FALSE;               /*  are we navigating or sensing?*/
int ButDown[] = {FALSE,FALSE,FALSE,FALSE,FALSE};

int currentX, currentY;                 /*  current mouse position.*/
int lastMouseEvent = 0/*MapNotify*/;         /*  last event a mouse did; care about Button and Motion events only.*/
struct X3D_Node* lastPressedOver = NULL;/*  the sensitive node that the mouse was last buttonpressed over.*/
struct X3D_Node* lastOver = NULL;       /*  the sensitive node that the mouse was last moused over.*/
int lastOverButtonPressed = FALSE;      /*  catch the 1 to 0 transition for button presses and isOver in TouchSensors */

int maxbuffers = 1;                     /*  how many active indexes in bufferarray*/
int bufferarray[] = {GL_BACK,0};

/* current time and other time related stuff */
double TickTime;
double lastTime;
double BrowserStartTime;        /* start of calculating FPS     */
double BrowserFPS = 0.0;        /* calculated FPS               */
double BrowserSpeed = 0.0;      /* calculated movement speed    */

int trisThisLoop;

/* do we have some sensitive nodes in scene graph? */
int HaveSensitive = FALSE;

/* Function protos */
static void sendDescriptionToStatusBar(struct X3D_Node *CursorOverSensitive);
void do_keyPress(char kp, int type);
static void render_collisions(void);
static void render_pre(void);
static void render(void);
static void setup_projection(int pick, int x, int y);
static struct X3D_Node*  getRayHit(void);
static void get_hyperhit(void);
static void sendSensorEvents(struct X3D_Node *COS,int ev, int butStatus, int status);
int isBrowserPlugin = FALSE;

/* libFreeWRL_get_version()

  Q. where do I get this function ?
  A: look in Makefile.am (vtempl will create it automatically in internal_version.c).

*/

/* stop the display thread. Used (when this comment was made) by the OSX Safari plugin; keeps
most things around, just stops display thread, when the user exits a world. */
static void stopDisplayThread()
{
	if (!TEST_NULL_THREAD(DispThrd)) {
		quitThread = TRUE;
		pthread_join(DispThrd,NULL);
		ZERO_THREAD(DispThrd);
	}
}

static double waitsec;

#if !defined(_WIN32)

static struct timeval mytime;

/* Doug Sandens windows function; lets make it static here for non-windows */
static double Time1970sec(void) {
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

#define TI(_tv) gettimeofdat(&_tv)
#define TID(_tv) ((double)_tv.tv_sec + (double)_tv.tv_usec/1000000.0)


/* Main eventloop for FreeWRL!!! */
#ifndef IPHONE
void EventLoop() {
        static int loop_count = 0;

#if defined(TARGET_X11) || defined(TARGET_MOTIF)
        Cursor cursor;
#endif
	while (refreshOK) {
		usleep(10);
	}


        DEBUG_RENDER("start of MainLoop (parsing=%s) (url loaded=%s)\n", 
		     BOOL_STR(isinputThreadParsing()), BOOL_STR(IS_WORLD_LOADED));

        /* should we do events, or maybe a parser is parsing? */
        doEvents = (!isinputThreadParsing()) && (!isTextureParsing()) && isInputThreadInitialized();

        /* Set the timestamp */
	TickTime = Time1970sec();

        /* any scripts to do?? */
        INITIALIZE_ANY_SCRIPTS;


        /* BrowserAction required? eg, anchors, etc */
        if (BrowserAction) {
                BrowserAction = doBrowserAction ();
        }

        /* has the default background changed? */
        if (cc_changed) doglClearColor();

        OcclusionStartofEventLoop();
        startOfLoopNodeUpdates();

        /* First time through */
        if (loop_count == 0) {
                BrowserStartTime = TickTime;
                lastTime = TickTime;
        } else {
		/* calculate how much to wait so that we are running around 100fps. Adjust the constant
		   in the line below to raise/lower this frame rate */
		/* NOTE: OSX front end now syncs with the monitor, meaning, this sleep is no longer needed */
/* Johns - OSX plugin sometimes runs away, so for now, keep this here #ifndef TARGET_AQUA */

#ifndef IPHONE
               waitsec = 7000.0 + TickTime - lastTime;
               if (waitsec > 0.0) {
                       /* printf ("waiting %lf\n",waitsec); */
                       usleep((unsigned)waitsec);
		}
#endif
        }

        if (loop_count == 25) {

                BrowserFPS = 25.0 / (TickTime-BrowserStartTime);
                setMenuFps((float)BrowserFPS); /*  tell status bar to refresh, if it is displayed*/
                /* printf ("fps %f tris %d, rootnode children %d \n",BrowserFPS,trisThisLoop, X3D_GROUP(rootNode)->children.n);  */
                /* ConsoleMessage("fps %f tris %d\n",BrowserFPS,trisThisLoop);   */

		/* printf ("MainLoop, nearPlane %lf farPlane %lf\n",nearPlane, farPlane);  */

                BrowserStartTime = TickTime;
                loop_count = 1;
        } else {
                loop_count++;
        }

        trisThisLoop = 0;

        /* handle any events provided on the command line - Robert Sim */
        if (keypress_string && doEvents) {
                if (keypress_wait_for_settle > 0) {
                        keypress_wait_for_settle--;
                } else {
                        /* dont do the null... */
                        if (*keypress_string) {
                                /* printf ("handling key %c\n",*keypress_string); */
#if !defined( AQUA ) && !defined( WIN32 )  /*win32 - don't know whats it is suppsoed to do yet */

                                do_keyPress(*keypress_string,KeyPress);
#endif
                                keypress_string++;
                        } else {
                                keypress_string=NULL;
                        }
                }
        }

	/**
	 *   Merge of Bare X11 and Motif/X11 event handling ...
	 */
	/* REMARK: Do we want to process all pending events ? */

#if defined(TARGET_X11)
	/* We are running our own bare window */
	while (XPending(Xdpy)) {
	    XNextEvent(Xdpy, &event);
	    handle_Xevents(event);
	}
#endif

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
#endif //XEVENT_VERBOSE

#if 0 // no need for this hack now that events are correctly propagated through Motif
	    /**
	     *   Quick hack to make events propagate
	     *   to the drawing area.... :)
	     */
	    if (aev->window == GLwin) {
		handle_Xevents(event);
	    } else {
		XtDispatchEvent (&event);
	    }
#else // clean implementation
	    XtDispatchEvent (&event);
#endif
	}

#endif //defined(TARGET_MOTIF)

#if defined(_MSC_VER) //TARGET_WIN32)
	/**
	 *   Win32 event loop
	 *   gives windows message handler a time slice and 
	 *   it calls handle_aqua and do_keypress from fwWindow32.c 
	 */
	doEventsWin32A(); 
#endif

        /* Viewer move viewpoint */
        handle_tick();

        /* setup Projection and activate ProximitySensors */
        if (onScreen) render_pre(); 

        /* first events (clock ticks, etc) if we have other things to do, yield */
        if (doEvents) do_first (); else sched_yield();

	/* ensure depth mask turned on here */
	FW_GL_DEPTHMASK(GL_TRUE);

        /* actual rendering */
        if (onScreen)
			render();

        /* handle_mouse events if clicked on a sensitive node */
		/*printf("nav mode =%d sensitive= %d\n",NavigationMode, HaveSensitive); */
        if (!NavigationMode && HaveSensitive) {
                setup_projection(TRUE,currentX,currentY);
                setup_viewpoint();
                /* original: render_hier(rootNode,VF_Sensitive); */
                render_hier(rootNode,VF_Sensitive  | VF_Geom); 
                CursorOverSensitive = getRayHit();

                /* for nodes that use an "isOver" eventOut... */
                if (lastOver != CursorOverSensitive) {
                        #ifdef VERBOSE
                        printf ("%lf over changed, lastOver %u cursorOverSensitive %u, butDown1 %d\n",
				TickTime, (unsigned int) lastOver, (unsigned int) CursorOverSensitive,
				ButDown[1]);
                        #endif

                        if (ButDown[1]==0) {

                                /* ok, when the user releases a button, cursorOverSensitive WILL BE NULL
                                   until it gets sensed again. So, we use the lastOverButtonPressed flag to delay 
                                   sending this flag by one event loop loop. */
                                if (!lastOverButtonPressed) {
                                        sendSensorEvents(lastOver, overMark, 0, FALSE);
                                        sendSensorEvents(CursorOverSensitive, overMark, 0, TRUE);
                                        lastOver = CursorOverSensitive;
                                }
                                lastOverButtonPressed = FALSE;
                        } else {
                                lastOverButtonPressed = TRUE;
                        }

                }
                #ifdef VERBOSE
                if (CursorOverSensitive != NULL) 
			printf("COS %d (%s)\n",
			       (unsigned int) CursorOverSensitive,
			       stringNodeType(CursorOverSensitive->_nodeType));
                #endif

                /* did we have a click of button 1? */

                if (ButDown[1] && (lastPressedOver==NULL)) {
                        /* printf ("Not Navigation and 1 down\n"); */
                        /* send an event of ButtonPress and isOver=true */
                        lastPressedOver = CursorOverSensitive;
                        sendSensorEvents(lastPressedOver, ButtonPress, ButDown[1], TRUE);
                }

                if ((ButDown[1]==0) && lastPressedOver!=NULL) {
                        /* printf ("Not Navigation and 1 up\n");  */
                        /* send an event of ButtonRelease and isOver=true;
                           an isOver=false event will be sent below if required */
                        sendSensorEvents(lastPressedOver, ButtonRelease, ButDown[1], TRUE);
                        lastPressedOver = NULL;
                }

                if (lastMouseEvent == MotionNotify) {
                        /* printf ("Not Navigation and motion - going into sendSensorEvents\n"); */
                        /* TouchSensor hitPoint_changed needs to know if we are over a sensitive node or not */
                        sendSensorEvents(CursorOverSensitive,MotionNotify, ButDown[1], TRUE);

                        /* PlaneSensors, etc, take the last sensitive node pressed over, and a mouse movement */
                        sendSensorEvents(lastPressedOver,MotionNotify, ButDown[1], TRUE);
                }



                /* do we need to re-define cursor style?        */
                /* do we need to send an isOver event?          */
                if (CursorOverSensitive!= NULL) {
		    SENSOR_CURSOR;

                        /* is this a new node that we are now over?
                           don't change the node pointer if we are clicked down */
                        if ((lastPressedOver==NULL) && (CursorOverSensitive != oldCOS)) {
                                sendSensorEvents(oldCOS,MapNotify,ButDown[1], FALSE);
                                sendSensorEvents(CursorOverSensitive,MapNotify,ButDown[1], TRUE);
                                oldCOS=CursorOverSensitive;
                                sendDescriptionToStatusBar(CursorOverSensitive);
                        }

                } else {
                        /* hold off on cursor change if dragging a sensor */
                        if (lastPressedOver!=NULL) {
			    SENSOR_CURSOR;
                        } else {
			    ARROW_CURSOR;
                        }

                        /* were we over a sensitive node? */
                        if ((oldCOS!=NULL)  && (ButDown[1]==0)) {
                                sendSensorEvents(oldCOS,MapNotify,ButDown[1], FALSE);
                                /* remove any display on-screen */
                                sendDescriptionToStatusBar(NULL);
                                oldCOS=NULL;
                        }
                }

                /* do we have to change cursor? */
#if !defined( AQUA ) && !defined( WIN32 )


                if (cursor != curcursor) {
                        curcursor = cursor;
                        XDefineCursor (Xdpy, GLwin, cursor);
                }
#elif defined( WIN32 )
				/*win32 - dont know what goes here */
#else
                if (ccurse != ocurse) {
                        ocurse = ccurse;
                        setAquaCursor(ccurse);
                }
#endif
        }


        /* handle snapshots */
        if (doSnapshot) {
                Snapshot();
        }


        /* do OcclusionCulling, etc */
        OcclusionCulling();
        
        if (doEvents) {
                /* and just parsed nodes needing binding? */
                SEND_BIND_IF_REQUIRED(setViewpointBindInRender)
                SEND_BIND_IF_REQUIRED(setFogBindInRender)
                SEND_BIND_IF_REQUIRED(setBackgroundBindInRender)
                SEND_BIND_IF_REQUIRED(setNavigationBindInRender)


                /* handle ROUTES - at least the ones not generated in do_first() */
                propagate_events();

                /* Javascript events processed */
                process_eventsProcessed();

                /* EAI */
                handle_EAI();
		handle_MIDIEAI();
        }

        /* record the TickTime here, for rate setting. We don't do this earlier, as some
           nodes use the lastTime variable */
        lastTime = TickTime;
	if (askForRefresh) {
		refreshOK = TRUE;
		askForRefresh = FALSE;
	}
}
#else
GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = malloc (sizeof(char) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}

///
// Initialize the shader and program object
//
int Init ( ESContext *esContext )
{
   esContext->userData = malloc(sizeof(UserData));

   UserData *userData = esContext->userData;
   GLbyte vShaderStr[] =  
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;\n"\
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
      "}                                            \n";

   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Bind vPosition to attribute 0   
   glBindAttribLocation ( programObject, 0, "vPosition" );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = malloc (sizeof(char) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         
         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return GL_FALSE;
   }

   // Store the program object
   userData->programObject = programObject;

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void EventLoop() {}
void Draw ( ESContext *esContext )
{
   UserData *userData = esContext->userData;
   GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f, 
                           -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f };
      
   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

   // Load the vertex data
   glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
   glEnableVertexAttribArray ( 0 );

   glDrawArrays ( GL_TRIANGLES, 0, 3 );
}

#endif
#if !defined( AQUA ) && !defined( WIN32 )
void handle_Xevents(XEvent event) {

        XEvent nextevent;
        char buf[10];
        KeySym ks;
        int count;

        lastMouseEvent=event.type;

        #ifdef VERBOSE
        switch (event.type) {
                case ConfigureNotify: printf ("Event: ConfigureNotify\n"); break;
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
                        setScreenDim (event.xconfigure.width,event.xconfigure.height);
                        break;
//#endif
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

                        do_keyPress((char)ks,event.type);
                        break;

                case ButtonPress:
                case ButtonRelease:
                        /* printf("got a button press or button release\n"); */
                        /*  if a button is pressed, we should not change state,*/
                        /*  so keep a record.*/
						if(handleStatusbarHud(event.type, &clipPlane))break;
                        if (event.xbutton.button>=5) break;  /* bounds check*/
                        ButDown[event.xbutton.button] = (event.type == ButtonPress);

                        /* if we are Not over an enabled sensitive node, and we do NOT
                           already have a button down from a sensitive node... */
                        /* printf("cursoroversensitive is %u lastPressedOver %u\n", CursorOverSensitive,lastPressedOver); */
                        if ((CursorOverSensitive==NULL) && (lastPressedOver==NULL))  {
                                NavigationMode=ButDown[1] || ButDown[3];
                                handle (event.type,event.xbutton.button,
                                        (float) ((float)event.xbutton.x/screenWidth),
                                        (float) ((float)event.xbutton.y/screenHeight));
                        }
                        break;

                case MotionNotify:
                        /* printf("got a motion notify\n"); */
                        /*  do we have more motion notify events queued?*/
                        if (XPending(Xdpy)) {
                                XPeekEvent(Xdpy,&nextevent);
                                if (nextevent.type==MotionNotify) { break;
                                }
                        }

                        /*  save the current x and y positions for picking.*/
                        currentX = event.xbutton.x;
                        currentY = event.xbutton.y;
                        /* printf("navigationMode is %d\n", NavigationMode); */
						if(handleStatusbarHud(6, &clipPlane))break;
                        if (NavigationMode) {
                                /*  find out what the first button down is*/
                                count = 0;
                                while ((count < 5) && (!ButDown[count])) count++;
                                if (count == 5) return; /*  no buttons down???*/

                                handle (event.type,(unsigned)count,
                                        (float)((float)event.xbutton.x/screenWidth),
                                        (float)((float)event.xbutton.y/screenHeight));
                        }
                        break;
        }
}
#endif

/* get setup for rendering. */
static void render_pre() {
        /* 1. Set up projection */
        setup_projection(FALSE,0,0);


        /* 2. Headlight, initialized here where we have the modelview matrix to Identity.
        FIXME: position of light sould actually be offset a little (towards the center)
        when in stereo mode. */
        FW_GL_LOAD_IDENTITY();

        /*printf("calling get headlight in render_pre\n"); */
        if (get_headlight()) lightState(HEADLIGHT_LIGHT,TRUE);


        /* 3. Viewpoint */
        setup_viewpoint();      /*  need this to render collisions correctly*/

        /* 4. Collisions */
        if (fw_params.collision == 1) {
                render_collisions();
                setup_viewpoint(); /*  update viewer position after collision, to*/
                                   /*  give accurate info to Proximity sensors.*/
        }

        /* 5. render hierarchy - proximity */
        if (doEvents) render_hier(rootNode, VF_Proximity);

        PRINT_GL_ERROR_IF_ANY("GLBackend::render_pre");
}
void setup_projection(int pick, int x, int y) 
{
	GLsizei screenwidth2 = screenWidth;
	GLDOUBLE aspect2 = screenRatio;
	GLDOUBLE fieldofview2;
	GLint xvp = 0;
	fieldofview2 = fieldofview;
	if(getViewerType()==VIEWER_YAWPITCHZOOM)
		fieldofview2*=fovZoom;
	if(Viewer.sidebyside) 
	{
		screenwidth2 = (int)((screenwidth2 * .5)+.5);
		aspect2 = aspect2 * .5;
		if(Viewer.iside == 1) xvp = (GLint)screenwidth2;
	}

        FW_GL_MATRIX_MODE(GL_PROJECTION);
		/* >>> statusbar hud */
		if(clipPlane != 0)
		{   /* scissor used to prevent mainloop from glClear()ing the statusbar area
			 which is updated only every 10-25 loops */
			FW_GL_SCISSOR(0,clipPlane,screenWidth,screenHeight);
			FW_GL_ENABLE(GL_SCISSOR_TEST);
		}
		/* <<< statusbar hud */

		FW_GL_VIEWPORT(xvp,clipPlane,screenwidth2,screenHeight);
#ifdef AQUA
#if !defined(IPHONE) 
                myglobalContext = CGLGetCurrentContext();
		CGLSetCurrentContext(myglobalContext);
#endif
#endif
		FW_GL_VIEWPORT(xvp, clipPlane, screenwidth2, screenHeight);
        FW_GL_LOAD_IDENTITY();
        if(pick) {
                /* picking for mouse events */
                FW_GL_GETINTEGERV(GL_VIEWPORT,viewPort2);
                FW_GLU_PICK_MATRIX((float)x,(float)viewPort2[3]-y, (float)100,(float)100,viewPort2);
        }

        /* bounds check */
        if ((fieldofview2 <= 0.0) || (fieldofview2 > 180.0)) 
			fieldofview2=45.0;
        /* glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);  */

        FW_GLU_PERSPECTIVE(fieldofview2, aspect2, nearPlane, farPlane); 
        FW_GL_MATRIX_MODE(GL_MODELVIEW);

        PRINT_GL_ERROR_IF_ANY("XEvents::setup_projection");

}


/* Render the scene */
static void render() 
{
#if defined(FREEWRL_SHUTTER_GLASSES) || defined(FREEWRL_STEREO_RENDERING)

    int count;
	static double shuttertime;
	static int shutterside;

	/*  profile*/
    /* double xx,yy,zz,aa,bb,cc,dd,ee,ff;*/
    /* struct timeval mytime;*/
    /* struct timezone tz; unused see man gettimeofday */

    for (count = 0; count < maxbuffers; count++) {

        /*set_buffer((unsigned)bufferarray[count],count); */              /*  in Viewer.c*/

		Viewer.buffer = (unsigned)bufferarray[count]; 
		Viewer.iside = count;
		FW_GL_DRAWBUFFER((unsigned)bufferarray[count]);

        /*  turn lights off, and clear buffer bits*/

		if(Viewer.isStereo)
		{
			if(Viewer.shutterGlasses == 2) /* flutter mode - like --shutter but no GL_STEREO so alternates */
			{
				if(TickTime - shuttertime > 2.0)
				{
					shuttertime = TickTime;
					if(shutterside > 0) shutterside = 0;
					else shutterside = 1;
				}
				if(count != shutterside) continue;
			}
			if(Viewer.anaglyph) //haveAnaglyphShader)
				USE_SHADER(Viewer.programs[Viewer.iprog[count]]);
			setup_projection(0, 0, 0);
			if(Viewer.sidebyside && count >0)
				BackEndClearBuffer(1);
			else
				BackEndClearBuffer(2);
			setup_viewpoint(); 
		}
		else 
			BackEndClearBuffer(2);
		BackEndLightsOff();

#else

	BackEndClearBuffer(2); // no stereo, no shutter glasses: simple clear

#endif // SHUTTER GLASSES or STEREO	

	/*  turn light #0 off only if it is not a headlight.*/
	if (!get_headlight()) {
		lightState(HEADLIGHT_LIGHT,FALSE);
	}

	/*  Other lights*/
	PRINT_GL_ERROR_IF_ANY("XEvents::render, before render_hier");
	
	render_hier(rootNode, VF_globalLight);
	PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_globalLight)");
	
	/*  4. Nodes (not the blended ones)*/
	render_hier(rootNode, VF_Geom);
	PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_Geom)");
	
	/*  5. Blended Nodes*/
	if (have_transparency) {
		/*  render the blended nodes*/
		render_hier(rootNode, VF_Geom | VF_Blend);
		PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_Geom)");
	}
	
#if defined(FREEWRL_SHUTTER_GLASSES) || defined(FREEWRL_STEREO_RENDERING)

		if (Viewer.isStereo) {
			if (Viewer.anaglyph) //haveAnaglyphShader) 
			{
				if (count==0) {
					USE_SHADER(0);
					FW_GL_ACCUM(GL_LOAD,1.0); 
				}
				else if(count==1) {
					USE_SHADER(0);
					FW_GL_ACCUM(GL_ACCUM,1.0); 
					FW_GL_ACCUM(GL_RETURN,1.0);
				}
			}
		}

	} /* for loop */

	if (Viewer.isStereo) {
		Viewer.iside = Viewer.dominantEye; /*is used later in picking to set the cursor pick box on the (left=0 or right=1) viewport*/
	}

#endif

	/* status bar, if we have one */
	drawStatusBar();

	/* swap the rendering area */
	FW_GL_SWAPBUFFERS;
        PRINT_GL_ERROR_IF_ANY("XEvents::render");
}

static void get_collisionoffset(double *x, double *y, double *z)
{
		struct point_XYZ xyz;
        struct point_XYZ res = CollisionInfo.Offset;
		/* collision.offset should be in collision space coordinates: fly/examine: avatar space, walk: BVVA space */
        /* uses mean direction, with maximum distance */

		/* xyz is in collision space- fly/examine: avatar space, walk: BVVA space */
		xyz.x = xyz.y = xyz.z = 0.0;

		if(CollisionInfo.Count > 0 && !APPROX(vecnormal(&res, &res),0.0) )
				vecscale(&xyz, &res, sqrt(CollisionInfo.Maximum2));

		/* for WALK + collision */
		if(FallInfo.walking)
		{
			if(FallInfo.canFall && FallInfo.isFall ) 
			{
				/* canFall == true if we aren't climbing, isFall == true if there's no climb, and there's geom to fall to  */
				double floatfactor = .1;
				if(FallInfo.allowClimbing) floatfactor = 0.0; /*popcycle method */
				if(FallInfo.smoothStep)
					xyz.y = max(FallInfo.hfall,-FallInfo.fallStep) + naviinfo.height*floatfactor; 
				else
					xyz.y = FallInfo.hfall + naviinfo.height*floatfactor; //.1; 
				if(FallInfo.verticalOnly)
				{
					xyz.x = 0.0;
					xyz.z = 0.0;
				}
			}
			if(FallInfo.isClimb && FallInfo.allowClimbing)
			{
				/* stepping up normally handled by cyclindrical collision, but there are settings to use this climb instead */
				if(FallInfo.smoothStep)
					xyz.y = min(FallInfo.hclimb,FallInfo.fallStep);
				else
					xyz.y = FallInfo.hclimb; 
				if(FallInfo.verticalOnly)
				{
					xyz.x = 0.0;
					xyz.z = 0.0;
				}
			}
			if(FallInfo.isPenetrate)
			{
				/*over-ride everything else*/
				xyz = FallInfo.pencorrection;
			}
		}
		/* now convert collision-space deltas to avatar space via collision2avatar- fly/examine: identity (do nothing), walk:BVVA2A */
		transform3x3(&xyz,&xyz,FallInfo.collision2avatar);
		/* now xyz is in avatar space, ready to be added to avatar viewer.pos */
		*x = xyz.x;
		*y = xyz.y;
		*z = xyz.z;
		/* another transform possible: from avatar space into navigation space. fly/examine: identity walk: A2BVVA*/
}
struct point_XYZ viewer_get_lastP();
static void render_collisions() {
        struct point_XYZ v;
		int viewerType;
		viewerType = getViewerType();
		if(viewerType == VIEWER_YAWPITCHZOOM) return; //no collisions
		

        CollisionInfo.Offset.x = 0;
        CollisionInfo.Offset.y = 0;
        CollisionInfo.Offset.z = 0;
        CollisionInfo.Count = 0;
        CollisionInfo.Maximum2 = 0.;

		/* popcycle shaped avatar collision volume: ground to stepheight is a ray, stepheight to avatarheight is a cylinder, 
		   and a sphere on top?
		   -keeps cyclinder from dragging in terrain mesh, easy to harmonize fall and climb math so not fighting (now a ray both ways)
		   -2 implementations: analytical cyclinder, sampler
		   The sampler method intersects line segments radiating from the the avatar axis with shape facets - misses small shapes but good
		   for walls and floors; intersection math is simple: line intersect plane.
		*/
		FallInfo.fallHeight = 100.0; /* when deciding to fall, how far down do you look for a landing surface before giving up and floating */
		FallInfo.fallStep = 1.0; /* maximum height to fall on one frame */
		FallInfo.walking = getViewerType() == VIEWER_WALK; //viewer_type == VIEWER_WALK;
		FallInfo.canFall = FallInfo.walking; /* && COLLISION (but we wouldn't be in here if not). Will be set to 0 if a climb is found. */
		FallInfo.hits = 0; /* counter (number of vertical hits) set to zero here once per frame */
		FallInfo.isFall = 0; /*initialize here once per frame - flags if there's a fall found */
		FallInfo.isClimb = 0; /* initialize here each frame */
		FallInfo.smoothStep = 1; /* [1] setting - will only fall by fallstep on a frame rather than the full hfall */
		FallInfo.allowClimbing = 1; /* [0] - setting - 0=climbing done by collision with cyclinder 1=signals popcycle avatar collision volume and allows single-footpoint climbing  */
		FallInfo.verticalOnly = 0; /* [0] - setting - will completely over-ride/skip cylindrical collision and do only fall/climb */
		FallInfo.gravityVectorMethod = 1; //[1] - setting -  0=global Y down gravity 1= bound viewpoint Y down gravity as per specs
		FallInfo.fastTestMethod = 2; //[2] - setting -0=old method - uses fast cylinder test 1= MBB shape space 2= MBB avatar space 3=ignor fast cylinder test and keep going 
		FallInfo.walkColliderMethod = 3; /* 0=sphere 1=normal_cylinder 2=disp_ 3=sampler */
		FallInfo.canPenetrate = 1; /* setting - 0= don't check for wall penetration 1= check for wall penetration */
		FallInfo.isPenetrate = 0; /* set to zero once per loop and will come back 1 if there was a penetration detected and corrected */

		/* at this point we know the navigation mode and the pose of the avatar, and of the boundviewpoint 
		   so pre-compute some handy matrices for collision calcs - the avatar2collision and back (a tilt matrix for gravity direction)
		*/
		if(FallInfo.walking)
		{
			if(FallInfo.gravityVectorMethod==1)
			{
				/*bound viewpoint vertical aligned gravity as per specs*/
				avatar2BoundViewpointVerticalAvatar(FallInfo.avatar2collision, FallInfo.collision2avatar);
			}
			if(FallInfo.gravityVectorMethod==0)
			{
				/* Y-up-world aligned gravity */
				double modelMatrix[16];
				struct point_XYZ tupvBoundViewpoint, tupvWorld = {0,1,0};
				FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);
				transform3x3(&tupvBoundViewpoint,&tupvWorld,modelMatrix);
				matrotate2v(FallInfo.avatar2collision,tupvWorld,tupvBoundViewpoint);
				matrotate2v(FallInfo.collision2avatar,tupvBoundViewpoint,tupvWorld);
			}
		}
		else
		{
			/* when flying or examining, no gravity - up is your avatar's up */
			loadIdentityMatrix(FallInfo.avatar2collision);
			loadIdentityMatrix(FallInfo.collision2avatar);
		}

		/* wall penetration detection and correction initialization */
		if(FallInfo.walking && FallInfo.canPenetrate)
		{
			/* set up avatar to last valid avatar position vector in avatar space */
			double plen = 0.0;
			struct point_XYZ lastpos;  
			lastpos = viewer_get_lastP(); /* in viewer/avatar space */
			transform(&lastpos,&lastpos,FallInfo.avatar2collision); /* convert to collision space */
			/* if vector length == 0 can't penetrate - don't bother to check */
			plen = sqrt(vecdot(&lastpos,&lastpos));
			if(APPROX(plen,0.0))
				FallInfo.canPenetrate = 0;
			else
			{
				/* precompute MBB/extent etc in collision space for penetration vector */
				struct point_XYZ pos = {0.0,0.0,0.0};
				FallInfo.penMin[0] = min(pos.x,lastpos.x);
				FallInfo.penMin[1] = min(pos.y,lastpos.y);
				FallInfo.penMin[2] = min(pos.z,lastpos.z);
				FallInfo.penMax[0] = max(pos.x,lastpos.x);
				FallInfo.penMax[1] = max(pos.y,lastpos.y);
				FallInfo.penMax[2] = max(pos.z,lastpos.z);
				FallInfo.penRadius = plen;
				vecnormal(&lastpos,&lastpos);
				FallInfo.penvec = lastpos;
				FallInfo.pendisp = 0.0; /* used to sort penetration intersections to pick one closest to last valid avatar position */
			}
		}

        render_hier(rootNode, VF_Collision);
		if(!FallInfo.isPenetrate) 
		{
			/* we don't clear if we just solved a penetration, otherwise we'll get another penetration going back, from the correction.
			   No pen? Then clear here to start over on the next loop.
			*/
			viewer_lastP_clear(); 
		}
        get_collisionoffset(&(v.x), &(v.y), &(v.z));

	/* if (!APPROX(v.x,0.0) || !APPROX(v.y,0.0) || !APPROX(v.z,0.0)) {
		printf ("MainLoop, rendercollisions, offset %f %f %f\n",v.x,v.y,v.z);
	} */
		/* v should be in avatar coordinates*/
        increment_pos(&v);
}


static void setup_viewpoint() {
	
	/* GLDOUBLE projMatrix[16]; */

        FW_GL_MATRIX_MODE(GL_MODELVIEW); /*  this should be assumed , here for safety.*/
        FW_GL_LOAD_IDENTITY();

        viewer_togl(fieldofview);
        render_hier(rootNode, VF_Viewpoint);
        PRINT_GL_ERROR_IF_ANY("XEvents::setup_viewpoint");

	/*
	fw_glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	printf ("\n");
	printf ("setup_viewpoint, proj  %lf %lf %lf\n",projMatrix[12],projMatrix[13],projMatrix[14]);
	fw_glGetDoublev(GL_MODELVIEW_MATRIX, projMatrix);
	printf ("setup_viewpoint, model %lf %lf %lf\n",projMatrix[12],projMatrix[13],projMatrix[14]);
	printf ("setup_viewpoint, currentPos %lf %lf %lf\n",        Viewer.currentPosInModel.x, 
	        Viewer.currentPosInModel.y ,
	        Viewer.currentPosInModel.z);
	*/

}

extern void dump_scene (int level, struct X3D_Node* node); // in GeneratedCode.c
void dump_scenegraph()
{
#ifdef FW_DEBUG
	dump_scene(0, rootNode);
#endif
}

void sendKeyToKeySensor(const char key, int upDown);
/* handle a keypress. "man freewrl" shows all the recognized keypresses */
void do_keyPress(const char kp, int type) {
        /* does this X3D file have a KeyDevice node? if so, send it to it */
        /* printf("%c%d\n",kp,type); */
        if (KeySensorNodePresent()) {
                sendKeyToKeySensor(kp,type);
        } else {
                if (type == KeyPress) {
                        switch (kp) {
                                case 'e': { set_viewer_type (VIEWER_EXAMINE); break; }
                                case 'w': { set_viewer_type (VIEWER_WALK); break; }
                                case 'd': { set_viewer_type (VIEWER_FLY); break; }
                                case 'f': { set_viewer_type (VIEWER_EXFLY); break; }
                                case 'y': { set_viewer_type (VIEWER_YAWPITCHZOOM); break; }
                                case 'h': { toggle_headlight(); break;}
                                case '/': { print_viewer(); break; }
			        case '\\': { dump_scenegraph(); break; }
				case '$': resource_tree_dump(0, root_res); break;
                                case 'q': { if (!RUNNINGASPLUGIN) {
                                                  doQuit();
                                            }
                                            break;
                                          }
                                case 'c': { toggle_collision(); break;}
                                case 'v': {Next_ViewPoint(); break;}
                                case 'b': {Prev_ViewPoint(); break;}
                                case 's': {setSnapshot(); break;}
								case 'x': {Snapshot(); break;} /* thanks to luis dias mas dec16,09 */
                                default: {handle_key(kp);}
        
                        }
                } else {
                        handle_keyrelease(kp);
                }
        }
}

struct X3D_Node* getRayHit() {
        double x,y,z;
        int i;
        if(hpdist >= 0) {
                FW_GLU_UNPROJECT(hp.x,hp.y,hp.z,rayHit.modelMatrix,rayHit.projMatrix,viewport,&x,&y,&z);

                /* and save this globally */
                ray_save_posn.c[0] = (float) x; ray_save_posn.c[1] = (float) y; ray_save_posn.c[2] = (float) z;

                /* we POSSIBLY are over a sensitive node - lets go through the sensitive list, and see
                   if it exists */

                /* is the sensitive node not NULL? */
                if (rayHit.node == NULL) return NULL;
        
                /*
                printf ("rayhit, we are over a node, have node %u (%s), posn %lf %lf %lf",
                        rayHit.node,stringNodeType(rayHit.node->_nodeType), x,y,z);
                printf (" dist %f ",rayHit.node->_dist);
                */

                for (i=0; i<num_SensorEvents; i++) {
                        if (SensorEvents[i].fromnode == rayHit.node) {
                                /* printf ("found this node to be sensitive - returning %u\n",rayHit.node); */
                                return ((struct X3D_Node*) rayHit.node);
                        }
                }
        }

        /* no rayhit, or, node was "close" (scenegraph-wise) to a sensitive node, but is not one itself */
        return(NULL);
}


/* set a node to be sensitive, and record info for this node */
void setSensitive(struct X3D_Node *parentNode, struct X3D_Node *datanode) {
        void (*myp)(unsigned *);

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
        /* printf ("set_sensitive ,parentNode %d  type %s data %d type %s\n",parentNode,
                        stringNodeType(parentNode->_nodeType),datanode,stringNodeType (datanode->_nodeType));  */

        /* record this sensor event for clicking purposes */
        SensorEvents = REALLOC(SensorEvents,sizeof (struct SensStruct) * (num_SensorEvents+1));

        if (datanode == 0) {
                printf ("setSensitive: datastructure is zero for type %s\n",stringNodeType(datanode->_nodeType));
                return;
        }

        /* now, put the function pointer and data pointer into the structure entry */
        SensorEvents[num_SensorEvents].fromnode = parentNode;
        SensorEvents[num_SensorEvents].datanode = datanode;
        SensorEvents[num_SensorEvents].interpptr = (void *)myp;

        /* printf ("saved it in num_SensorEvents %d\n",num_SensorEvents); */
        num_SensorEvents++;
}

/* we have a sensor event changed, look up event and do it */
/* note, (Geo)ProximitySensor events are handled during tick, as they are time-sensitive only */
static void sendSensorEvents(struct X3D_Node* COS,int ev, int butStatus, int status) {
        int count;

        /* if we are not calling a valid node, dont do anything! */
        if (COS==NULL) return;

        for (count = 0; count < num_SensorEvents; count++) {
                if (SensorEvents[count].fromnode == COS) {
                        /* should we set/use hypersensitive mode? */
                        if (ev==ButtonPress) {
                                hypersensitive = SensorEvents[count].fromnode;
                                hyperhit = 0;
                        } else if (ev==ButtonRelease) {
                                hypersensitive = 0;
                                hyperhit = 0;
                        } else if (ev==MotionNotify) {
                                get_hyperhit();
                        }


                        SensorEvents[count].interpptr(SensorEvents[count].datanode, ev,butStatus, status);
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

        FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, projMatrix);
        FW_GLU_UNPROJECT(r1.x, r1.y, r1.z, rayHitHyper.modelMatrix,
                projMatrix, viewport, &x1, &y1, &z1);
        FW_GLU_UNPROJECT(r2.x, r2.y, r2.z, rayHitHyper.modelMatrix,
                projMatrix, viewport, &x2, &y2, &z2);
        FW_GLU_UNPROJECT(hp.x, hp.y, hp.z, rayHit.modelMatrix,
                projMatrix,viewport, &x3, &y3, &z3);

        /* printf ("get_hyperhit in VRMLC %f %f %f, %f %f %f, %f %f %f\n",*/
        /*      x1,y1,z1,x2,y2,z2,x3,y3,z3);*/

        /* and save this globally */
        hyp_save_posn.c[0] = (float) x1; hyp_save_posn.c[1] = (float) y1; hyp_save_posn.c[2] = (float) z1;
        hyp_save_norm.c[0] = (float) x2; hyp_save_norm.c[1] = (float) y2; hyp_save_norm.c[2] = (float) z2;
        ray_save_posn.c[0] = (float) x3; ray_save_posn.c[1] = (float) y3; ray_save_posn.c[2] = (float) z3;
}

/* set stereo buffers, if required */
void setStereoBufferStyle(int itype) /*setXEventStereo()*/
{
	if(itype==0)
	{
		/* quad buffer crystal eyes style */
		bufferarray[0]=GL_BACK_LEFT;
		bufferarray[1]=GL_BACK_RIGHT;
		maxbuffers=2;
	}
	else if(itype==1)
	{
		/*sidebyside and anaglyph type*/
		bufferarray[0]=GL_BACK;
		bufferarray[1]=GL_BACK;
		maxbuffers=2;
	}
}

/* go to the first viewpoint */
void First_ViewPoint() {
        if (totviewpointnodes>=1) {

                /* whew, we have other vp nodes */
                /*
                if (currboundvpno != 0) {
                */
                        /* have to do some work */
                        send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),0);
                        currboundvpno = 0;
                        send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),1);
                /*
                }
                */
        }
}
/* go to the first viewpoint */
void Last_ViewPoint() {
        if (totviewpointnodes>=1) {
                /* whew, we have other vp nodes */
                /*
                if (currboundvpno != (totviewpointnodes-1)) {
                */
                        /* have to do some work */
                        send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),0);
                        currboundvpno = totviewpointnodes-1;
                        send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),1);
                /*
                }
                */
        }
}
/* go to the previous viewpoint */
void Prev_ViewPoint() {
        if (totviewpointnodes>=1) {
                /* whew, we have other vp nodes */
                send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),0);
                currboundvpno--;
                if (currboundvpno<0) currboundvpno=totviewpointnodes-1;
                send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),1);
        }
}

/* go to the next viewpoint */
void Next_ViewPoint() {
        if (totviewpointnodes>=1) {
                /* whew, we have other vp nodes */
                send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),0);
                currboundvpno++;
                if (currboundvpno>=totviewpointnodes) currboundvpno=0;
                send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),1);
        }
}

/* handle all the displaying and event loop stuff. */
void _displayThread()
{
	ENTER_THREAD("display");

	/* Initialize display */
	if (!display_initialize()) {
		ERROR_MSG("initFreeWRL: error in display initialization.\n");
		exit(1);
	}

	/* Context has been created,
	   make it current to this thread */
	bind_GLcontext();

	new_tessellation();
	
	set_viewer_type(VIEWER_EXAMINE);
	
	viewer_postGLinit_init();

#ifndef AQUA
	if (fullscreen) resetGeometry();
#endif
    
	/* loop and loop, and loop... */
	while (!quitThread) {
		EventLoop();
	} 
    	kill_oldWorld(TRUE,TRUE,__FILE__,__LINE__);
}

void setLastMouseEvent(int etype) {
        lastMouseEvent = etype;
}

void initialize_parser()
{
        quitThread = FALSE;

	/* create the root node */
	if (rootNode == NULL) {
		rootNode = createNewX3DNode (NODE_Group);	
		/*remove this node from the deleting list*/
		doNotRegisterThisNodeForDestroy(rootNode);
	}
}

void setSnapSeq() {
#ifdef DOSNAPSEQUENCE
/* need to re-implement this for OSX generating QTVR */
        snapsequence = TRUE;
#endif
}

void setEAIport(int pnum) {
        EAIport = pnum;
}

void setWantEAI(int flag) {
        EAIwanted = TRUE;
}

void setLineWidth(float lwidth) {
        gl_linewidth = lwidth;
}

void setUseShapeThreadIfPossible(int x) {
#ifdef DO_MULTI_OPENGL_THREADS
	useShapeThreadIfPossible = x;
#endif
}

void setTextures_take_priority (int x) {
        textures_take_priority = x;
}

/* set the opengl_has_textureSize. Expect a number that is 0 - use max, or negative. eg,
   -512 hopefully sets to size 512x512; this will be bounds checked in the texture
   thread */
void setTexSize(int requestedsize) {
/*         opengl_has_textureSize = requestedsize; */	
}


void setKeyString(const char* kstring)
{
    keypress_string = strdup(kstring);
}

void setSeqFile(const char* file)
{
#if defined(DOSNAPSEQUENCE)
    /* need to re-implement this for OSX generating QTVR */
    snapseqB = strdup(file);
    printf("snapseqB is %s\n", snapseqB);
#else
    WARN_MSG("Call to setSeqFile when Snapshot Sequence not compiled in.\n");
#endif
}

void setSnapFile(const char* file)
{
    snapsnapB = strdup(file);
    TRACE_MSG("snapsnapB set to %s\n", snapsnapB);
}

void setMaxImages(int max)
{
#if defined(DOSNAPSEQUENCE)
    /* need to re-implement this for OSX generating QTVR */
    if (max <=0)
	max = 100;
    maxSnapImages = max;
#else
    WARN_MSG("Call to setMaxImages when Snapshot Sequence not compiled in.\n");
#endif
}

void setSnapTmp(const char* file)
{
    seqtmp = strdup(file);
    TRACE_MSG("seqtmp set to %s\n", seqtmp);
}

/* if we had an exit(EXIT_FAILURE) anywhere in this C code - it means
   a memory error. So... print out a standard message to the
   console. */
void outOfMemory(const char *msg) {
        ConsoleMessage ("FreeWRL has encountered a memory allocation problem\n"\
                        "and is exiting.\nPlease email this file to freewrl-09@rogers.com\n -- %s--",msg);
        usleep(10 * 1000);
        exit(EXIT_FAILURE);
}

/* quit key pressed, or Plugin sends SIGQUIT */
void doQuit()
{
    stopDisplayThread();

    kill_oldWorld(TRUE,TRUE,__FILE__,__LINE__);

    /* set geometry to normal size from fullscreen */
#ifndef AQUA
    resetGeometry();
#endif

    /* kill any remaining children */
    killErrantChildren();
    
#ifdef DEBUG_MALLOC
    void scanMallocTableOnQuit(void);
    scanMallocTableOnQuit();
#endif

    exit(EXIT_SUCCESS);
}

void freewrlDie (const char *format) {
        ConsoleMessage ("Catastrophic error: %s\n",format);
        doQuit();
}


#if defined(AQUA) || defined(WIN32)

/* MIMIC what happens in handle_Xevents, but without the X events */
void handle_aqua(const int mev, const unsigned int button, int x, int y) {
        int count;

         /* printf ("handle_aqua in MainLoop; but %d x %d y %d screenWidth %d screenHeight %d",
                button, x,y,screenWidth,screenHeight); 
        if (mev == ButtonPress) printf ("ButtonPress\n");
        else if (mev == ButtonRelease) printf ("ButtonRelease\n");
        else if (mev == MotionNotify) printf ("MotionNotify\n");
        else printf ("event %d\n",mev);  */

        /* save this one... This allows Sensors to get mouse movements if required. */
        lastMouseEvent = mev;
        /* save the current x and y positions for picking. */
		currentX = x;
		currentY = y;

		if( handleStatusbarHud(mev, &clipPlane) )return; /* statusbarHud options screen should swallow mouse clicks */

        if ((mev == ButtonPress) || (mev == ButtonRelease)) {
                /* record which button is down */
                ButDown[button] = (mev == ButtonPress);
                /* if we are Not over an enabled sensitive node, and we do NOT already have a 
                   button down from a sensitive node... */

                if ((CursorOverSensitive ==NULL) && (lastPressedOver ==NULL)) {
                        NavigationMode=ButDown[1] || ButDown[3];
                        handle(mev, button, (float) ((float)x/screenWidth), (float) ((float)y/screenHeight));
                }
        }

        if (mev == MotionNotify) {
                /* save the current x and y positions for picking. */
                // above currentX = x;
                //currentY = y;

                if (NavigationMode) {
                        /* find out what the first button down is */
                        count = 0;
                        while ((count < 5) && (!ButDown[count])) count++;
                        if (count == 5) return; /* no buttons down???*/

                        handle (mev, (unsigned) count, (float) ((float)x/screenWidth), (float) ((float)y/screenHeight));
                }
        }
}
#endif
#ifdef AQUA

#if !defined(IPHONE) 
void initGL() {
        /* printf ("OSX initGL called\n");  */
        myglobalContext = CGLGetCurrentContext();
        /* printf ("initGL call finished - myglobalContext %u\n",myglobalContext); */
}

int getOffset() {
        return (int) offsetof(struct X3D_Group, children);
}

void setCurXY(int cx, int cy) {
	/* printf ("setCurXY, have %d %d\n",currentX,currentY);  */
        currentX = cx;
        currentY = cy;
}

void setButDown(int button, int value) {
	/* printf ("old qua code, setButDown...\n"); */
        ButDown[button] = value;
}

#endif

void setIsPlugin() {

        RUNNINGASPLUGIN = TRUE;
        setUseShapeThreadIfPossible(0);
                
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
ConsoleMessage ("setIsPlugin, BrowserFullPath :%s:");
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

int aquaPrintVersion() {
	printf ("FreeWRL version: %s\n", libFreeWRL_get_version()); 
	exit(EXIT_SUCCESS);
}

#endif

/* if we are visible, draw the OpenGL stuff, if not, don not bother */
void setDisplayed (int state) {
        #ifdef VERBOSE
        if (state) printf ("WE ARE DISPLAYED\n");
        else printf ("we are now iconic\n");
        #endif
        onScreen = state;
}

void setEaiVerbose() {
        eaiverbose = TRUE;
}

void askForRefreshOK() {
	askForRefresh = TRUE;
}

int checkRefresh() {
	return refreshOK;
}

void resetRefresh() {
	refreshOK = FALSE;
}

/* called from the standalone OSX front end and the OSX plugin */
void replaceWorldNeeded(char* str)
{
	//printf ("replaceWorldneeded called - file %s\n",str); 
        AnchorsAnchor = NULL;
        FREE_IF_NZ(OSX_replace_world_from_console);
	OSX_replace_world_from_console = STRDUP(str);
        BrowserAction = TRUE;
}

/* OSX the Plugin is telling the displayThread to stop and clean everything up */
void stopRenderingLoop(void) {

    	stopDisplayThread();
    	//killErrantChildren();
	/* lets do an equivalent to replaceWorldNeeded, but with NULL for the new world */
#ifdef AQUA
#if !defined(IPHONE) 
	myglobalContext = NULL;
#endif
#endif
        AnchorsAnchor = NULL;
        BrowserAction = TRUE;
        FREE_IF_NZ(OSX_replace_world_from_console);
}


/* send the description to the statusbar line */
void sendDescriptionToStatusBar(struct X3D_Node *CursorOverSensitive) {
        int tmp;
        char *ns;

        if (CursorOverSensitive == NULL) update_status (NULL);
        else {

                ns = NULL;
                for (tmp=0; tmp<num_SensorEvents; tmp++) {
                        if (SensorEvents[tmp].fromnode == CursorOverSensitive) {
                                switch (SensorEvents[tmp].datanode->_nodeType) {
                                        case NODE_Anchor: ns = ((struct X3D_Anchor *)SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_PlaneSensor: ns = ((struct X3D_PlaneSensor *)SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_SphereSensor: ns = ((struct X3D_SphereSensor *)SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_TouchSensor: ns = ((struct X3D_TouchSensor *)SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_GeoTouchSensor: ns = ((struct X3D_GeoTouchSensor *)SensorEvents[tmp].datanode)->description->strptr; break;
                                        case NODE_CylinderSensor: ns = ((struct X3D_CylinderSensor *)SensorEvents[tmp].datanode)->description->strptr; break;
				default: {printf ("sendDesc; unknown node type %d\n",SensorEvents[tmp].datanode->_nodeType);}
                                }
                                /* if there is no description, put the node type on the screen */
                                if (ns == NULL) {ns = "(over sensitive)";}
                                else if (ns[0] == '\0') ns = (char *)stringNodeType(SensorEvents[tmp].datanode->_nodeType);
        
                                /* send this string to the screen */
                                update_status(ns);
                        }
                }
        }
}
