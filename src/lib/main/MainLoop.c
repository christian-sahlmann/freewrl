/*
=INSERT_TEMPLATE_HERE=

$Id: MainLoop.c,v 1.6 2008/12/02 18:17:18 couannette Exp $

CProto ???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h"
#include "headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../world_script/jsUtils.h"
#include "../world_script/CScripts.h"
#include "Snapshot.h"
#include "../scenegraph/LinearAlgebra.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"
#include "../input/EAIheaders.h"

#include "../ui/ui.h"
#include "../opengl/OpenGL_Utils.h"


#ifndef AQUA
# define SENSOR_CURSOR cursor= sensorc;
# define ARROW_CURSOR  cursor = arrowc;
#else
# define SENSOR_CURSOR ccurse = SCURSE;
# define ARROW_CURSOR  ccurse = ACURSE;
#endif


/* do we want OpenGL errors to be printed to the console?? */
int displayOpenGLErrors = FALSE;

/* are we displayed, or iconic? */
static int onScreen = TRUE;


/* do we do event propagation, proximity calcs?? */
static int doEvents = FALSE;

static char debs[300];
/* void debug_print(char *s) {printf ("debug_print:%s\n",s);} */

/* handle X11 requests, windowing calls, etc if on X11 */
#ifndef AQUA
/*      #include <X11/cursorfont.h> */

/*      #ifdef XF86V4 */
/*              #include <X11/extensions/xf86vmode.h> */
/*      #endif */

/*      #include <X11/keysym.h> */
/*      #include <X11/Intrinsic.h> */

        Cursor arrowc;
        Cursor sensorc;
        Cursor curcursor;
        XEvent event;
/*         extern void createGLContext(); */
/*      #ifdef HAVE_MOTIF */
/*      extern XtAppContext Xtcx; */
/*      #endif */
        void handle_Xevents(XEvent event);
#endif

pthread_t DispThrd = 0;
char* threadmsg;
char* PluginFullPath;

int replaceWorld = FALSE;
char  replace_name[FILENAME_MAX];

/* linewidth for lines and points - passed in on command line */
float gl_linewidth = 1.0;

/* what kind of file was just parsed? */
int currentFileVersion = 0;

#ifdef AQUA
        #include <OpenGL.h>
        void eventLoopsetPaneClipRect(int npx, int npy, WindowPtr fwWindow, int ct, int cb, int cr, int cl, int width, int height) ;
        CGLContextObj myglobalContext;
        AGLContext aqglobalContext;
        #define SCURSE 1
        #define ACURSE 0
        int ccurse = ACURSE;
        int ocurse = ACURSE;
        GLboolean cErr;
        static GDHandle gGDevice;

        /* for handling Safari window changes at the top of the display event loop */
        int PaneClipnpx;
        int PaneClipnpy;
        WindowPtr PaneClipfwWindow;
        int PaneClipct;
        int PaneClipcb;
        int PaneClipcr;
        int PaneClipcl;
        int PaneClipwidth;
        int PaneClipheight;
        int PaneClipChanged = FALSE;
#endif

/* we want to run initialize() from the calling thread. NOTE: if initialize creates VRML/X3D nodes, it
   will call the ProdCon methods to do this, and these methods will check to see if nodes, yada, yada,
   yada, until we run out of stack. So, we check to see if we are initializing; if so, don't worry about
   checking for new scripts */
#define INITIALIZE_ANY_SCRIPTS \
        /* any scripts to initialize here? we do it here, because we may just have created new scripts during \
           X3D/VRML parsing. Routing in the Display thread may have noted new scripts, but will ignore them until  \
           we have told it that the scripts are initialized. */ \
        if (max_script_found != max_script_found_and_initialized) { \
                /* printf ("have scripts to initialize in EventLoop old %d new %d\n",max_script_found, max_script_found_and_initialized); */ \
                int i; jsval retval; \
                for (i=max_script_found_and_initialized+1; i <= max_script_found; i++) { \
                        /* printf ("initializing script %d in thread %u\n",i,pthread_self()); */  \
                        JSCreateScriptContext(i); \
                        JSInitializeScriptAndFields(i); \
                        ACTUALRUNSCRIPT(i, "initialize()" ,&retval); \
                        ScriptControl[i]._initialized=TRUE; \
                        /* printf ("initialized script %d\n",i); */ \
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
void setup_viewpoint();
void get_collisionoffset(double *x, double *y, double *z);

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
GLint viewPort2[10];

/* screen width and height. */
int screenWidth=1;
int screenHeight=1;
int clipPlane = 0;

struct X3D_Node* CursorOverSensitive=NULL;      /*  is Cursor over a Sensitive node?*/
struct X3D_Node* oldCOS=NULL;                   /*  which node was cursor over before this node?*/
int NavigationMode=FALSE;               /*  are we navigating or sensing?*/
int ButDown[] = {FALSE,FALSE,FALSE,FALSE,FALSE};

int currentX, currentY;                 /*  current mouse position.*/
int lastMouseEvent = MapNotify;         /*  last event a mouse did; care about Button and Motion events only.*/
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

#ifdef PROFILE
static double timeAA, timeA, timeB, timeC, timeD, timeE, timeF, xxf, oxf;
#endif

int trisThisLoop;

/* do we have some sensitive nodes in scene graph? */
int HaveSensitive = FALSE;

/* Function protos */
void sendDescriptionToStatusBar(struct X3D_Node *CursorOverSensitive);
void do_keyPress(char kp, int type);
void render_collisions(void);
void render_pre(void);
void render(void);
void setup_projection(int pick, int x, int y);
void glPrintError(char *str);
void XEventStereo(void);
void EventLoop(void);
struct X3D_Node*  getRayHit(void);
void get_hyperhit(void);
void sendSensorEvents(struct X3D_Node *COS,int ev, int butStatus, int status);
Boolean pluginRunning;
int isBrowserPlugin = FALSE;

/******************************************************************************/
/* Jens Rieks sent in some changes - some of which uses strndup, which does not
   always exist... */
char *fw_strndup (const char *str, int len) {
        char *retval;
        int ml;
        ml = strlen(str);
        if (ml > len) ml = len;
        retval = (char *) MALLOC (sizeof (char) * (ml+1));
        strncpy (retval,str,ml);
        /* ensure termination */
        retval[ml] = '\0';
        return retval;
}

/* a simple routine to allow the front end to get our version */
const char *getLibVersion() {
        return libFreeX3D_get_version();
}

/* Main eventloop for FreeWRL!!! */
void EventLoop() {
        int counter;

        #ifndef AQUA
        Cursor cursor;
        #endif

        static int loop_count = 0;
        struct timeval waittime;

        struct timeval mytime;
        struct timezone tz; /* unused see man gettimeofday */

        #ifdef AQUA
        if (RUNNINGASPLUGIN) {
                cErr = aglSetCurrentContext(aqglobalContext);
                if (cErr == GL_FALSE) {
                        printf("set current context error!");
                }
        }

        /* window size changed by Safari? */
        if (PaneClipChanged) {
                eventLoopsetPaneClipRect( PaneClipnpx, PaneClipnpy, PaneClipfwWindow,
                        PaneClipct, PaneClipcb, PaneClipcr, PaneClipcl,
                        PaneClipwidth, PaneClipheight);
                PaneClipChanged = FALSE;
        }

        #endif

        /* printf ("start of MainLoop\n"); */

        /* should we do events, or maybe a parser is parsing? */
        doEvents = (!isinputThreadParsing()) && (!isTextureParsing()) && (!isShapeCompilerParsing()) && isInputThreadInitialized();

        /* Set the timestamp */
        gettimeofday (&mytime,&tz);
        TickTime = (double) mytime.tv_sec + (double)mytime.tv_usec/1000000.0;
        
        /* any scripts to do?? */
        INITIALIZE_ANY_SCRIPTS;

        /* has the default background changed? */
        if (cc_changed) doglClearColor();

        OcclusionStartofEventLoop();
        startOfLoopNodeUpdates();

        /* First time through */
        if (loop_count == 0) {
                BrowserStartTime = TickTime;
                lastTime = TickTime;
                #ifdef PROFILE
                /* printf ("time setup for debugging\n"); */ 
                timeAA = timeA = timeB = timeC = timeD = timeE = timeF =0.0;
                #endif
        } else {
                /*  rate limit ourselves to about 65fps.*/
                /* waittime.tv_usec = (TickTime - lastTime - 0.0120)*1000000.0;*/
                waittime.tv_usec = (TickTime - lastTime - 0.0153)*1000000.0;
                if (waittime.tv_usec < 0.0) {
                        waittime.tv_usec = -waittime.tv_usec;
                        /* printf ("waiting %d\n",(int)waittime.tv_usec);*/
                        usleep((unsigned)waittime.tv_usec);
                }
        }
        if (loop_count == 25) {

                BrowserFPS = 25.0 / (TickTime-BrowserStartTime);
                setMenuFps(BrowserFPS); /*  tell status bar to refresh, if it is displayed*/
                /* printf ("fps %f tris %d\n",BrowserFPS,trisThisLoop);  */

                #ifdef PROFILE
                oxf = timeAA + timeA + timeB + timeC + timeD + timeE + timeF;
                if (oxf > 0.01) 
                printf ("fps %f times beg:%lf eve:%lf handle_tick:%lf render_pre:%lf do_first:%lf render:%lf ending:%lf\n",
                                BrowserFPS,
/*
                                timeAA,
                                timeA,timeB,
                                timeC, timeD,
                                timeE,timeF);
*/
                                timeAA/oxf*100.0,
                                timeA/oxf*100.0,timeB/oxf*100.0,
                                timeC/oxf*100.0, timeD/oxf*100.0,
                                timeE/oxf*100.0,timeF/oxf*100.0);

                #endif
                BrowserStartTime = TickTime;
                loop_count = 1;
        } else {
                loop_count++;
        }

        trisThisLoop = 0;

        #ifdef PROFILE
        gettimeofday (&mytime,&tz);
        xxf = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
        timeAA = (double)timeAA +  (double)xxf - TickTime;
        #endif

        /* BrowserAction required? eg, anchors, etc */
        if (BrowserAction) {
                doBrowserAction ();
                BrowserAction = FALSE;  /* action complete */
        }

        if (replaceWorld) {
                Anchor_ReplaceWorld(replace_name);
                replaceWorld= FALSE;
        }

        /* handle any events provided on the command line - Robert Sim */
        if (keypress_string && doEvents) {
                if (keypress_wait_for_settle > 0) {
                        keypress_wait_for_settle--;
                } else {
                        /* dont do the null... */
                        if (*keypress_string) {
                                /* printf ("handling key %c\n",*keypress_string); */
                                do_keyPress(*keypress_string,KeyPress);
                                keypress_string++;
                        } else {
                                keypress_string=NULL;
                        }
                }
        }

        /* if we are not letting Motif handle things, check here for keys, etc */
#ifndef AQUA
#ifndef HAVE_MOTIF
        while (XPending(Xdpy)) {
            XNextEvent(Xdpy, &event);
            handle_Xevents(event);
        }
#ifdef HAVE_GTK2
        printf ("GTK event loop should be here\n");
#endif
#endif
#endif

        #ifdef PROFILE
        gettimeofday (&mytime,&tz);
        oxf = xxf;
        xxf = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
        timeA = (double)timeA +  (double)xxf - oxf;
        #endif

        /* Viewer move viewpoint */
        handle_tick();

        #ifdef PROFILE
        gettimeofday (&mytime,&tz);
        oxf = xxf;
        xxf = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
        timeB = (double)timeB +  (double)xxf - oxf;
        #endif

        /* setup Projection and activate ProximitySensors */
        if (onScreen) render_pre();

        #ifdef PROFILE
        gettimeofday (&mytime,&tz);
        oxf = xxf;
        xxf = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
        timeC = (double)timeC +  (double)xxf - oxf;
        #endif


        /* first events (clock ticks, etc) if we have other things to do, yield */
        if (doEvents) do_first (); else sched_yield();

        #ifdef PROFILE
        gettimeofday (&mytime,&tz);
        oxf = xxf;
        xxf = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
        timeD = (double)timeD +  (double)xxf - oxf;
        #endif

        /* actual rendering */
        if (onScreen) render();

        #ifdef PROFILE
        gettimeofday (&mytime,&tz);
        oxf = xxf;
        xxf = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
        timeE = (double)timeE +  (double)xxf - oxf;
        #endif

        /* handle_mouse events if clicked on a sensitive node */
        if (!NavigationMode && HaveSensitive) {
                setup_projection(TRUE,currentX,currentY);
                setup_viewpoint();
                /* original: render_hier(rootNode,VF_Sensitive); */
                render_hier(rootNode,VF_Sensitive  | VF_Geom); 
                CursorOverSensitive = getRayHit();

                /* for nodes that use an "isOver" eventOut... */
                if (lastOver != CursorOverSensitive) {
                        #ifdef VERBOSE
                        printf ("%lf over changed, lastOver %u cursorOverSensitive %u, butDown1 %d\n",TickTime, lastOver,CursorOverSensitive,ButDown[1]);
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
                if (CursorOverSensitive != NULL) printf ("COS %d (%s)\n",CursorOverSensitive,stringNodeType(CursorOverSensitive->_nodeType));
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
                        SENSOR_CURSOR

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
                                SENSOR_CURSOR
                        } else {
                                ARROW_CURSOR
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
#ifndef AQUA

                if (cursor != curcursor) {
                        curcursor = cursor;
                        XDefineCursor (Xdpy, GLwin, cursor);
                }
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
        }

        /* record the TickTime here, for rate setting. We don't do this earlier, as some
           nodes use the lastTime variable */
        lastTime = TickTime;


        #ifdef PROFILE
        gettimeofday (&mytime,&tz);
        oxf = xxf;
        xxf = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
        timeF = (double)timeF +  (double)xxf - oxf;
        #endif

}

#ifndef AQUA
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
                #ifdef HAVE_NOTOOLKIT
                /* Motif, etc, usually handles this. */
                case ConfigureNotify:
                        setScreenDim (event.xconfigure.width,event.xconfigure.height);
                        break;
                #endif
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
void render_pre() {
        /* 1. Set up projection */
        setup_projection(FALSE,0,0);


        /* 2. Headlight, initialized here where we have the modelview matrix to Identity.
        FIXME: position of light sould actually be offset a little (towards the center)
        when in stereo mode. */
        fwLoadIdentity();

        /*printf("calling get headlight in render_pre\n"); */
        if (get_headlight()) lightState(0,TRUE);


        /* 3. Viewpoint */
        setup_viewpoint();      /*  need this to render collisions correctly*/

        /* 4. Collisions */
        if (be_collision == 1) {
                render_collisions();
                setup_viewpoint(); /*  update viewer position after collision, to*/
                                   /*  give accurate info to Proximity sensors.*/
        }

        /* 5. render hierarchy - proximity */
        if (doEvents) render_hier(rootNode, VF_Proximity);

        glPrintError("GLBackend::render_pre");
}

/* Render the scene */
void render() {
        int count;

        /*  profile*/
        /* double xx,yy,zz,aa,bb,cc,dd,ee,ff;*/
        /* struct timeval mytime;*/
        /* struct timezone tz; unused see man gettimeofday */

        for (count = 0; count < maxbuffers; count++) {
                set_buffer((unsigned)bufferarray[count]);               /*  in Viewer.c*/
                glDrawBuffer((unsigned)bufferarray[count]);

                /*  turn lights off, and clear buffer bits*/
                BackEndClearBuffer();
                BackEndLightsOff();

                /*  turn light #0 off only if it is not a headlight.*/
                if (!get_headlight()) {
                        lightState(0,FALSE);
                }

                /*  Correct Viewpoint, only needed when in stereo mode.*/
                if (maxbuffers > 1) setup_viewpoint();

                /*  Other lights*/
                glPrintError("XEvents::render, before render_hier");

                render_hier(rootNode, VF_otherLight);
                glPrintError("XEvents::render, render_hier(VF_VF_otherLight)");

                /*  4. Nodes (not the blended ones)*/
                render_hier(rootNode, VF_Geom);
                glPrintError("XEvents::render, render_hier(VF_Geom)");

                /*  5. Blended Nodes*/
                if (have_transparency) {

                        /*  turn off writing to the depth buffer*/
                        glDepthMask(FALSE);

                        /*  render the blended nodes*/
                        render_hier(rootNode, VF_Geom | VF_Blend);

                        /*  and turn writing to the depth buffer back on*/
                        glDepthMask(TRUE);
                        glPrintError("XEvents::render, render_hier(VF_Geom)");
                }

        }
        #ifndef AQUA
                glXSwapBuffers(Xdpy,GLwin);
        #else
                if (RUNNINGASPLUGIN) {
                        aglSetCurrentContext(aqglobalContext);
                        aglSwapBuffers(aqglobalContext);
                } else {
                        CGLError err = CGLFlushDrawable(myglobalContext);
                        if (err != kCGLNoError) printf ("CGLFlushDrawable error %d\n",err);
                        updateContext();
                }
        #endif

        glPrintError("XEvents::render");
}



void
get_collisionoffset(double *x, double *y, double *z)
{
        struct point_XYZ res = CollisionInfo.Offset;

        /* uses mean direction, with maximum distance */
        if (CollisionInfo.Count == 0) {
            *x = *y = *z = 0;
        } else {
            if (APPROX(vecnormal(&res, &res),0.0)) {
                        *x = *y = *z = 0;
            } else {
                        vecscale(&res, &res, sqrt(CollisionInfo.Maximum2));
                        *x = res.x;
                        *y = res.y;
                        *z = res.z;
                         /* printf ("get_collisionoffset, %lf %lf %lf\n",*x, *y, *z);  */
            }
        }
}

void render_collisions() {
        struct point_XYZ v;
        CollisionInfo.Offset.x = 0;
        CollisionInfo.Offset.y = 0;
        CollisionInfo.Offset.z = 0;
        CollisionInfo.Count = 0;
        CollisionInfo.Maximum2 = 0.;

        render_hier(rootNode, VF_Collision);
        get_collisionoffset(&(v.x), &(v.y), &(v.z));
        increment_pos(&v);
}

void setup_viewpoint() {
        fwMatrixMode(GL_MODELVIEW); /*  this should be assumed , here for safety.*/
        fwLoadIdentity();
        viewer_togl(fieldofview);
        render_hier(rootNode, VF_Viewpoint);
        glPrintError("XEvents::setup_viewpoint");
}



void setup_projection(int pick, int x, int y) {
        #ifdef AQUA
        if (RUNNINGASPLUGIN) {
                aglSetCurrentContext(aqglobalContext);
        } else {
                CGLSetCurrentContext(myglobalContext);
        }
        #endif

        fwMatrixMode(GL_PROJECTION);
        glViewport(0,clipPlane,screenWidth,screenHeight);
        fwLoadIdentity();
        if(pick) {
                /* picking for mouse events */
                glGetIntegerv(GL_VIEWPORT,viewPort2);
                gluPickMatrix((float)x,(float)viewPort2[3]-y,
                        (float)100,(float)100,viewPort2);
        }

        /* bounds check */
        if ((fieldofview <= 0.0) || (fieldofview > 180.0)) fieldofview=45.0;
        /* glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);  */
        gluPerspective(fieldofview, screenRatio, nearPlane, farPlane); 

        fwMatrixMode(GL_MODELVIEW);

        glPrintError("XEvents::setup_projection");
}

/* handle a keypress. "man freewrl" shows all the recognized keypresses */
void do_keyPress(const char kp, int type) {
        /* does this X3D file have a KeyDevice node? if so, send it to it */
        
        if (KeySensorNodePresent()) {
                sendKeyToKeySensor(kp,type);
        } else {
                if (type == KeyPress) {
                        switch (kp) {
                                case 'e': { set_viewer_type (EXAMINE); break; }
                                case 'w': { set_viewer_type (WALK); break; }
                                case 'd': { set_viewer_type (FLY); break; }
                                case 'f': { set_viewer_type (EXFLY); break; }
                                case 'h': { toggle_headlight(); break;}
                                case '/': { print_viewer(); break; }
                                case 'q': { if (!RUNNINGASPLUGIN) {
                                                  doQuit();
                                            }
                                            break;
                                          }
                                case 'c': {be_collision = !be_collision; 
                                                setMenuButton_collision(be_collision); break; }
                                case 'v': {Next_ViewPoint(); break;}
                                case 'b': {Prev_ViewPoint(); break;}
                                case 's': {setSnapshot(); break;}
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
                gluUnProject(hp.x,hp.y,hp.z,rayHit.modelMatrix,rayHit.projMatrix,viewport,&x,&y,&z);

                /* and save this globally */
                ray_save_posn.c[0] = x; ray_save_posn.c[1] = y; ray_save_posn.c[2] = z;

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
                        stringNodeType(parentNode->_nodeType),datanode,stringNodeType (datanode->_nodeType)); */

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
void sendSensorEvents(struct X3D_Node* COS,int ev, int butStatus, int status) {
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
void get_hyperhit() {
        double x1,y1,z1,x2,y2,z2,x3,y3,z3;
        GLdouble projMatrix[16];

        fwGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
        gluUnProject(r1.x, r1.y, r1.z, rayHitHyper.modelMatrix,
                projMatrix, viewport, &x1, &y1, &z1);
        gluUnProject(r2.x, r2.y, r2.z, rayHitHyper.modelMatrix,
                projMatrix, viewport, &x2, &y2, &z2);
        gluUnProject(hp.x, hp.y, hp.z, rayHit.modelMatrix,
                projMatrix,viewport, &x3, &y3, &z3);

        /* printf ("get_hyperhit in VRMLC %f %f %f, %f %f %f, %f %f %f\n",*/
        /*      x1,y1,z1,x2,y2,z2,x3,y3,z3);*/

        /* and save this globally */
        hyp_save_posn.c[0] = x1; hyp_save_posn.c[1] = y1; hyp_save_posn.c[2] = z1;
        hyp_save_norm.c[0] = x2; hyp_save_norm.c[1] = y2; hyp_save_norm.c[2] = z2;
        ray_save_posn.c[0] = x3; ray_save_posn.c[1] = y3; ray_save_posn.c[2] = z3;
}





/* set stereo buffers, if required */
void XEventStereo() {
        bufferarray[0]=GL_BACK_LEFT;
        bufferarray[1]=GL_BACK_RIGHT;
        maxbuffers=2;
}

/* if we had an opengl error... */
void glPrintError(char *str) {
        if (displayOpenGLErrors) {
                int err;
                while((err = glGetError()) != GL_NO_ERROR)
                        printf("OpenGL Error: \"%s\" in %s\n", gluErrorString((unsigned)err),str);
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

/* set internal variables for screen sizes, and calculate frustum */
void setScreenDim(int wi, int he) {
        screenWidth = wi;
        screenHeight = he;

        if (screenHeight != 0) screenRatio = (double) screenWidth/(double) screenHeight;
        else screenRatio =  screenWidth;
}

/* OSX plugin is telling us the id to refer to */
void setInstance (uintptr_t instance) {
        /* printf ("setInstance, setting to %u\n",instance); */
        _fw_instance = instance;
}

/* osx Safari plugin is telling us where the initial file is */
void setFullPath(const char* file) 
{
    if (!be_collision) {
        char ks = 'c';
        do_keyPress(ks, KeyPress);
    }
    FREE_IF_NZ (BrowserFullPath);
    BrowserFullPath = STRDUP((char *) file);
    /* printf ("setBrowserFullPath is %s (%d)\n",BrowserFullPath,strlen(BrowserFullPath));  */
}


/* handle all the displaying and event loop stuff. */
void displayThread()
{
    /* printf ("displayThread, I am %u \n",pthread_self()); */
    
    /* Create an OpenGL rendering context. */
#ifdef AQUA
    if (RUNNINGASPLUGIN) {
        aglSetCurrentContext(aqglobalContext);
        glpOpenGLInitialize();
        new_tessellation();
        set_viewer_type(EXAMINE);
    } else {
        glpOpenGLInitialize();
        new_tessellation();
    }
#else
    /* make the window, get the OpenGL context */
    openMainWindow(0, NULL);
    createGLContext();
    glpOpenGLInitialize();
    new_tessellation();
#endif
    
    /* see if we want OpenGL errors to be found and printed - note, this creates bottlenecks,
       in the OpenGL pipeline, so we do not do this all the time, only for debugging */
    if (getenv ("FREEWRL_PRINT_OPENGL_ERRORS")!= NULL) {
        displayOpenGLErrors = TRUE;
        printf ("FREEWRL_PRINT_OPENGL_ERRORS set\n");
        printf ("rendering on a \"%s\" chipset\n",glGetString(GL_RENDERER));
    }
    
    /* loop and loop, and loop... */
    while (!quitThread) {
        
        /* FreeWRL SceneGraph */
        EventLoop();
        
#ifndef AQUA
#ifdef HAVE_MOTIF
        /* X11 Windowing calls */
        
        /* any updates to the menu buttons? Because of Linux threading
           issues, we try to make all updates come from 1 thread */
        frontendUpdateButtons();
        
        
        /* do the Xt events here. */
        while (XtAppPending(Xtcx)!= 0) {
            XtAppNextEvent(Xtcx, &event);
            XtDispatchEvent (&event);
        }
#endif
        
#ifdef HAVE_GTK2
        /* X11 GTK windowing calls */
        /* any updates to the menu buttons? Because of Linux threading
           issues, we try to make all updates come from 1 thread */
        frontendUpdateButtons();
        
        /* GTK events here */
        printf ("look for GTK events here\n");
#endif
        
#endif
    }
    
#ifndef AQUA
    if (fullscreen) resetGeometry();
#endif
}

#ifdef AQUA
void initGL() {
        /* printf ("initGL called\n"); */
        if (RUNNINGASPLUGIN) {
                //aqglobalContext = aglGetCurrentContext();
        /* printf ("initGL - runningasplugin...\n"); */
                pluginRunning = TRUE;
                aglSetCurrentContext(aqglobalContext);
        } else {
                myglobalContext = CGLGetCurrentContext();
        }
        /* printf ("initGL call finished\n"); */
}

int getOffset() {
        return offsetof(struct X3D_Group, children);
}

void setCurXY(int cx, int cy) {
        currentX = cx;
        currentY = cy;
}

void setButDown(int button, int value) {
        ButDown[button] = value;
}

#endif


void setLastMouseEvent(int etype) {
        lastMouseEvent = etype;
}


/* load up the world, and run with it! */
void initFreewrl() {
        int tmp = 0;
        setbuf(stdout,0);
        setbuf(stderr,0);
        threadmsg = "event loop";
        quitThread = FALSE;

        /* printf ("initFreewrl called\n"); */

        #ifdef AQUA
        if (pluginRunning) {
        /* printf ("initFreeWRL, setting aglSetCurrentContext %u\n", aqglobalContext); */
                aglSetCurrentContext(aqglobalContext);
        }
        #endif

        /* printf ("initFreewrl, hows the DispThrd? %u\n",DispThrd); */

        if (DispThrd == 0) {
                pthread_create(&DispThrd, NULL, (void *) displayThread, (void*) threadmsg);

                #ifndef AQUA
                while (ISDISPLAYINITIALIZED == FALSE) { usleep(50);
        /* printf ("initFreewrl, waiting for display to become initialized...\n"); */
}
                #endif


                /* shape compiler thread - if we can do this */
                #ifdef DO_MULTI_OPENGL_THREADS
                initializeShapeCompileThread();
                #endif
                initializeInputParseThread();

                while (!isInputThreadInitialized()) {
                        usleep(50);
                }
                initializeTextureThread();
                while (!isTextureinitialized()) {
                        usleep(50);
                }

                /* create the root node */
                if (rootNode == NULL) {
                        rootNode = createNewX3DNode (NODE_Group);

                        /*remove this node from the deleting list*/
                        doNotRegisterThisNodeForDestroy(rootNode);
                }
        /* printf ("initFreewrl, down to here\n"); */
        }

        /* printf ("initFreewrl, bfp %s\n",BrowserFullPath); */

        /* is there a file name to parse? (ie, does the user just want to start off with a blank screen?) */
        if (BrowserFullPath != NULL) 
                if (strlen(BrowserFullPath) > 1) 
                        inputParse(FROMURL, BrowserFullPath, TRUE, FALSE, rootNode, offsetof(struct X3D_Group, children), &tmp, TRUE);
        /* printf ("initFreewrl call finished\n"); */
}


void setSnapSeq() {
#ifdef DOSNAPSEQUENCE
/* need to re-implement this for OSX generating QTVR */
        snapsequence = TRUE;
#endif
}

void closeFreewrl() {
        struct Multi_Node* tn;
        struct X3D_Group* rn;
        /* printf ("closeFreewrl called\n"); */

        #ifdef AQUA
        pluginRunning = FALSE;
        kill_clockEvents();
        EAI_killBindables();
        kill_bindables();
        kill_routing();
        kill_status();
        kill_openGLTextures();
        kill_javascript();

        #endif
        /* kill any remaining children */
        rn = (struct X3D_Group*) rootNode;
        tn =  &(rn->children);
        tn->n = 0;
        quitThread = TRUE;
        viewer_initialized = FALSE;

        if (!RUNNINGASPLUGIN) {
                set_viewer_type (EXAMINE);
        }
        glFlush();
        glFinish();
        screenWidth = screenHeight = 1;
        clipPlane = 0;
        /* printf ("closeFreewrl call finished\n"); */
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
        useShapeThreadIfPossible = x;
}

void setTextures_take_priority (int x) {
        textures_take_priority = x;
}

/* set the global_texSize. Expect a number that is 0 - use max, or negative. eg,
   -512 hopefully sets to size 512x512; this will be bounds checked in the texture
   thread */
void setTexSize(int requestedsize) {
        global_texSize = requestedsize;
}


void setSnapGif() {
#ifdef DOSNAPSEQUENCE
/* need to re-implement this for OSX generating QTVR */

        snapGif = 1;
#endif
}

void setNoCollision() {
        be_collision = 0;
        setMenuButton_collision(be_collision);
}

void setKeyString(const char* kstring) {
        keypress_string = fw_strndup(kstring, 500);
}

void setSeqFile(const char* file) {
#ifdef DOSNAPSEQUENCE
/* need to re-implement this for OSX generating QTVR */

        snapseqB = fw_strndup(file, 500);
        printf("snapseqB is %s\n", snapseqB);
#endif
}

void setSnapFile(const char* file) {
        snapsnapB = fw_strndup(file, 500);
        printf("snapsnapB is %s\n", snapsnapB);
}

void setMaxImages(int max) {
#ifdef DOSNAPSEQUENCE
/* need to re-implement this for OSX generating QTVR */

        if (max <=0)
                max = 100;
        maxSnapImages = max;
#endif

}
void setSeqTemp(const char* file) {
        seqtmp = fw_strndup(file, 500);
        printf("seqtmp is %s\n", seqtmp);
}

/* if we had an exit(EXIT_FAILURE) anywhere in this C code - it means
   a memory error. So... print out a standard message to the
   console. */
void outOfMemory(const char *msg) {
        ConsoleMessage ("FreeWRL has encountered a memory allocation problem\n"\
                        "and is exiting.\nPlease email this file to freewrl-09@rogers.com\n -- %s--",msg);
        sleep(10);
        exit(EXIT_FAILURE);
}



/* quit key pressed, or Plugin sends SIGQUIT */
void doQuit()
{
    STOP_DISPLAY_THREAD;

    kill_oldWorld(TRUE,TRUE,TRUE);

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

#ifdef AQUA

/* MIMIC what happens in handle_Xevents, but without the X events */
void handle_aqua(const int mev, const unsigned int button, int x, int y) {
        int count;

         /* printf ("handle_aqua in MainLoop; but %d x %d y %d screenWidth %d screenHeight %d",
                button, x,y,screenWidth,screenHeight); 
        if (mev == ButtonPress) printf ("ButtonPress\n");
        else if (mev == ButtonRelease) printf ("ButtonRelease\n");
        else if (mev == MotionNotify) printf ("MotionNotify\n");
        else printf ("event %d\n",mev); */

        /* save this one... This allows Sensors to get mouse movements if required. */
        lastMouseEvent = mev;

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
                currentX = x;
                currentY = y;

                if (NavigationMode) {
                        /* find out what the first button down is */
                        count = 0;
                        while ((count < 5) && (!ButDown[count])) count++;
                        if (count == 5) return; /* no buttons down???*/

                        handle (mev, (unsigned) count, (float) ((float)x/screenWidth), (float) ((float)y/screenHeight));
                }
        }
}
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
void createContext(CGrafPtr grafPtr) {
        AGLPixelFormat  fmt;
        GLboolean      mkc, ok;
        const GLint    attribWindow[]   = {AGL_RGBA, AGL_DOUBLEBUFFER, AGL_NO_RECOVERY, AGL_ALL_RENDERERS, AGL_ACCELERATED, AGL_DEPTH_SIZE, 24, AGL_STENCIL_SIZE, 8, AGL_NONE};
        AGLDrawable             aglWin;

        /* printf ("createContext called\n"); */
        if (aqglobalContext) {
                /* printf ("FreeWRL: createContext already made\n"); */
        /* printf ("FreeWRL: createContext already made\n");  */
                aglUpdateContext(aqglobalContext);
                return;
        }

        gGDevice = GetMainDevice();
        fmt = aglChoosePixelFormat(&gGDevice, 1, attribWindow);

        if ((fmt == NULL) || (aglGetError() != AGL_NO_ERROR)) {
                printf("FreeWRL: aglChoosePixelFormat failed!\n");
        }

        aqglobalContext = aglCreateContext(fmt, nil);
        if ((aqglobalContext == nil) || (aglGetError() != AGL_NO_ERROR)) {
                printf("FreeWRL: aglCreateContext failed!\n");
        }

        aglWin = (AGLDrawable)grafPtr;
        ok = aglSetDrawable(aqglobalContext, aglWin);

        if ((!ok) || (aglGetError() != AGL_NO_ERROR)) {
                if (aglGetError() == AGL_BAD_ALLOC) {
                        printf("FreeWRL: Not enough VRAM to initialize the draw context.\n");
                } else {
                        printf("FreeWRL: OGL_InitDrawContext: aglSetDrawable failed!\n");
                }
        }

        mkc = aglSetCurrentContext(aqglobalContext);
        if ((mkc == NULL) || (aglGetError() != AGL_NO_ERROR)) {
                printf("FreeWRL: aglSetCurrentContext failed!\n");
        }

        aglDestroyPixelFormat(fmt);

        //sprintf(debs, "Created context: %p", aqglobalContext);
        //debug_print(debs);

        pluginRunning = TRUE;
        /* printf ("createContext call finished\n"); */
}



void setPaneClipRect(int npx, int npy, WindowPtr fwWindow, int ct, int cb, int cr, int cl, int width, int height) {
        /* record these items so that they get handled in the display thread */
        PaneClipnpx = npx;
        PaneClipnpy = npy;
        PaneClipfwWindow = fwWindow;
        PaneClipct = ct;
        PaneClipcb = cb;
        PaneClipcr = cr;
        PaneClipcl = cl;
        PaneClipwidth = width;
        PaneClipheight = height;

        PaneClipChanged = TRUE;
}

void eventLoopsetPaneClipRect(int npx, int npy, WindowPtr fwWindow, int ct, int cb, int cr, int cl, int width, int height) {
        GLint   bufferRect[4];
        Rect    r;
        int     x,y;
        int     clipHeight;
        int     windowHeight;

        #ifdef VERBOSE
        sprintf(debs, "eventLoopPaneClipRect npx %d npy %d ct %d cb %d cr %d cl %d width %d height %d\n", 
                npx, npy, ct, cb, cr, cl, width, height);
        debug_print(debs);
        #endif

        if (aqglobalContext == nil) return;

        if (!pluginRunning) return;

        cErr = aglSetCurrentContext(aqglobalContext);
        if (cErr == GL_FALSE) {
                printf("FreeWRL: EventLoopPaneClipRect: set current context error!\n");
                return;
        }

        x = npx;


        #ifdef VERBOSE
        debug_print("get window bounds");
        #endif

        GetWindowBounds(fwWindow, kWindowContentRgn, &r);               // get size of actual Mac window

        windowHeight = r.bottom - r.top;

        #ifdef VERBOSE
        sprintf (debs,"window from getWindowBounds, t %d b %d l %d r %d\n",r.top,r.bottom,r.left,r.right);
        debug_print(debs);
        #endif

        clipPlane = cb - npy;
        y = windowHeight - npy - clipPlane;

        clipHeight = cb - ct;

        bufferRect[0] = x;
        bufferRect[1] = y;
        bufferRect[2] = cr - x;
        bufferRect[3] = clipHeight;

        #ifdef VERBOSE
        sprintf (debs,"setting bufferRect  to %d %d %d %d\n",bufferRect[0],bufferRect[1],bufferRect[2],bufferRect[3]);
        debug_print(debs);
        sprintf (debs,"but, screen width/height is %d %d\n",width,height); debug_print(debs);
        #endif

        if ((width != bufferRect[2]) || (height != bufferRect[3])) {
                #ifdef VERBOSE
                debug_print("DIFFERENCE IN SIZES! choosing the largest \n");
                #endif

                if (bufferRect[2] > width) width = bufferRect[2];
                if (bufferRect[3] > height) height = bufferRect[3];
        } else {
                setScreenDim(width, height);

                #ifdef VERBOSE
                debug_print("calling agl buffer rect ... ");
                #endif

                aglSetInteger (aqglobalContext, AGL_BUFFER_RECT, bufferRect);
        }


        /* ok to here... */
        aglEnable (aqglobalContext, AGL_BUFFER_RECT);
        clipPlane = y - npy;
        clipPlane += cb - height;

        clipPlane -= (r.bottom - cb);
        clipPlane += r.top;

        #ifdef VERBOSE
        sprintf(debs, "leaving set clip - set cp to %d\n", clipPlane);
        debug_print(debs);
        #endif
}

/* make a disposeContext but without some of the node destroys. */
void Safari_disposeContext() {
        /* printf ("Safari_disposeContext called\n"); */


        STOP_DISPLAY_THREAD

        cErr = aglSetCurrentContext(nil);
        if (cErr == GL_FALSE) {
                printf("FreeWRL: set current context error!\n");
        }
        cErr = aglSetDrawable(aqglobalContext, nil);
        if (cErr == GL_FALSE) {
                printf("FreeWRL: set current context error!\n");
        }
        cErr = aglDestroyContext(aqglobalContext);
        if (cErr == GL_FALSE) {
                printf("FreeWRL: set current context error!\n");
        }
        aqglobalContext = nil;
        /* printf ("Safari_disposeContext call finished\n"); */
}

/* older code - is this called from the front end? keep it around until
verified that it is no longer required: */

void disposeContext() {
        //debug_print("called dispose context");
        //sprintf(debs, "context is currently %p\n", aqglobalContext);
        //debug_print(debs);

        STOP_DISPLAY_THREAD 

        kill_X3DDefs();
        closeFreewrl();

        cErr = aglSetCurrentContext(nil);
        if (cErr == GL_FALSE) {
                printf("FreeWRL: set current context error!\n");
        }
        cErr = aglSetDrawable(aqglobalContext, nil);
        if (cErr == GL_FALSE) {
                printf("FreeWRL: set current context error!\n");
        }
        cErr = aglDestroyContext(aqglobalContext);
        if (cErr == GL_FALSE) {
                printf("FreeWRL: set current context error!\n");
        }
        aqglobalContext = nil;
}

void sendPluginFD(int fd) {
        /* printf ("sendPluginFD, FreeWRL received %d\n",fd); */
        _fw_browser_plugin = fd;
}
void aquaPrintVersion() {
        printf ("FreeWRL version: %s\n",FWVER); 
        exit(EXIT_SUCCESS);
}
void setPluginPath(char* path) {
        FREE_IF_NZ(PluginFullPath);
        PluginFullPath = strdup(path);
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
        eaiverbose = FALSE;
}
        
/* called from the standalone OSX front end */
void replaceWorldNeeded(char* str) {
        strncpy(&replace_name, (const char*) str, FILENAME_MAX);
        replaceWorld= TRUE; 
}


/* send the description to the statusbar line */
void sendDescriptionToStatusBar(struct X3D_Node *CursorOverSensitive) {
        int tmp;
        char *ns;

        if (CursorOverSensitive == NULL) update_status ("");
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
                                default: {}
                                }
                                /* if there is no description, put the node type on the screen */
                                if (ns == NULL) {ns = "(over sensitive)";}
                                else if (ns[0] == '\0') ns = stringNodeType(SensorEvents[tmp].datanode->_nodeType);
        
                                /* send this string to the screen */
                                update_status(ns);
                        }
                }
        }
}
