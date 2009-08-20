/*
=INSERT_TEMPLATE_HERE=

$Id: fwWindow.c,v 1.13 2009/08/20 00:37:52 couannette Exp $

FreeWRL main window.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include <float.h>

#include <X11/cursorfont.h>

#include "ui.h"
#include "../plugin/pluginUtils.h"

#if defined(TARGET_MOTIF)
# include "fwMotifWindow.h"
#else
# include "fwBareWindow.h"
#endif

static int oldx = 0, oldy = 0;

/* #undef XTDEBUG */

/* count XLib errors - do not continuously go on for decades... */
static int XLIB_errors = 0;
/* static int screen; */
static int quadbuff_stereo_mode;

char *GL_VER = NULL;
char *GL_VEN = NULL;
char *GL_REN = NULL;

/*
   from similar code in white_dune 8-)
   test for best visual you can get
   with best attribut list
   with maximal possible colorsize
   with maximal possible depth
 */

static int legal_depth_list[] = { 32, 24, 16, 15, 8, 4, 1 };

static int  default_attributes0[] =
   {
   GLX_DEPTH_SIZE,	 24,
   GLX_RED_SIZE,	   8,
   GLX_DOUBLEBUFFER,       GL_TRUE,
#ifdef GLX_STEREO
   GLX_STEREO,	     GL_TRUE,
#endif
   GLX_RGBA,	       GL_TRUE,
   0
   };

static int  default_attributes1[] =
   {
   GLX_DEPTH_SIZE,	 16,
   GLX_RED_SIZE,	   8,
   GLX_DOUBLEBUFFER,       GL_TRUE,
   GLX_RGBA,	       GL_TRUE,
   0
   };

static int  default_attributes2[] =
   {
   GLX_DEPTH_SIZE,	 16,
   GLX_RED_SIZE,	   8,
   GLX_RGBA,	       GL_TRUE,
   0
   };

static int  default_attributes3[] =
   {
   GLX_RGBA,	       GL_TRUE,
   0
   };

#define MAXSTAT 200
float myFps = 0.0;
char myMenuStatus[MAXSTAT];

void handle_Xevents(XEvent event);
XVisualInfo *find_best_visual(int shutter,int *attributes,int len);
static int catch_XLIB (Display *disp, XErrorEvent *err);

void setMenuStatus(char *stat) {
	strncpy (myMenuStatus, stat, MAXSTAT);
	setMessageBar();
}
void setMenuFps (float fps) {
	myFps = fps;
	setMessageBar();
}

void getVisual(void) {
	int *attributes = default_attributes3;
	int len=0;

	Xvi = find_best_visual(shutterGlasses,attributes,len);
	if(!Xvi) { 
		printf ("FreeWRL can not find an appropriate visual from GLX\n");
		exit(-1);
	}

	if ((shutterGlasses) && (quadbuff_stereo_mode==0)) {
		fprintf(stderr, "Warning: No quadbuffer stereo visual found !");
		fprintf(stderr, "On SGI IRIX systems read 'man setmon' or 'man xsetmon'\n");
	}
}

void createGLContext(void) {
    GLenum err;

	/* create a GLX context */
	#ifdef DO_MULTI_OPENGL_THREADS
	GLcx = glXCreateContext(Xdpy, Xvi, 0, GL_FALSE);
	#else
	GLcx = glXCreateContext(Xdpy, Xvi, 0, GL_TRUE);
	#endif

	if (GLcx == NULL) {
		printf ("FreeWRL - Could not create rendering context\n");
	}

	/* we have to wait until the main widget is realized to get the GLwin */
	while (ISDISPLAYINITIALIZED == FALSE) {
		/* printf ("MainWidgetRealized = FALSE, sleeping...\n"); */
		sleep (1);
	}

	/* get window id for later calls - we use more window refs than widget refs */
	GET_GLWIN;

	/* tell the X window system that we desire the following
	   attributes for this window */

	XSelectInput (Xdpy, GLwin, event_mask);

	/* lets make sure everything is sync'd up */
	XFlush(Xdpy);
~	glXMakeCurrent (Xdpy, GLwin,  GLcx);

	/* save this info for later use */
        GL_REN = (char *)glGetString(GL_RENDERER);
        GL_VER = (char *)glGetString(GL_VERSION);
        GL_VEN = (char *)glGetString(GL_VENDOR);

	err = glewInit();
	if (GLEW_OK != err)
	{
	    /* Problem: glewInit failed, something is seriously wrong. */
	    printf("Error: %s\n", glewGetErrorString(err));
	}
	printf( "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	/* Set up the OpenGL state. This'll get overwritten later... */
	glClearDepth (1.0);
	glClearColor (0.0, 0.0, 1.0, 0.0);
	FW_GL_MATRIX_MODE (GL_PROJECTION);
	glFrustum (-1.0, 1.0, -1.0, 1.0, 1.0, 20);
	FW_GL_MATRIX_MODE (GL_MODELVIEW);

	/* Mesa 6.4.1 on AMD64 will segfault. Check for this. */
/* 	if (sizeof(void*) == 8) { */
		/* running on a 64 bit system */
		if (strstr (GL_VER, "Mesa 6.4.1") != NULL) {
			printf ("Warning - Mesa located, needs to be version 6.4.2 or above, have %s\n",glGetString(GL_VERSION));
			printf ("get an update from http://mesa3d.org, or install an NVidia driver\n");
			printf ("FreeWRL will crash because of bugs in this version of Mesa\nget a new version from http://mesa3d.org, or install an NVidia driver and card.\n");
			ConsoleMessage ("FreeWRL will crash because of bugs in this version of Mesa\nget a new version from http://mesa3d.org, or install an NVidia driver and card.\n");
		}
/* 	} */
}


void openMainWindow (int argc, char **argv)
{
    int bestMode, i;

#ifdef DO_MULTI_OPENGL_THREADS
    XInitThreads();
#endif


    /* start up a XLib error handler to catch issues with FreeWRL. There
       should not be any issues, but, if there are, we'll most likely just
       throw our hands up, and continue */
    XSetErrorHandler(catch_XLIB);
    
    
    /* zero status stuff */
    myMenuStatus[0] = '\0';
    
    OPEN_TOOLKIT_MAINWINDOW;

    bestMode = -1;
    Xscreen = DefaultScreen(Xdpy);
    
    arrowc = XCreateFontCursor(Xdpy, XC_left_ptr);
    sensorc = XCreateFontCursor(Xdpy, XC_diamond_cross);
    
#ifdef HAVE_XF86_VMODE
    XF86VidModeGetAllModeLines(Xdpy, Xscreen, &vmode_nb_modes, &vmode_modes);
    
    bestMode = 0;
    for (i=0; i < vmode_nb_modes; i++) {
	if ((vmode_modes[i]->hdisplay == win_width) && (vmode_modes[i]->vdisplay == win_height)) {
	    bestMode = i;
	    break;
	}
    }
    /* There is no mode equivalent to the geometry specified */
    if (bestMode == -1) {
	fullscreen = 0;
	printf("No video mode for geometry %d x %d found.  Please use the --geo flag to specify an appropriate geometry, or add the required video mode\n", win_width, win_height);
    }
    XF86VidModeGetViewPort(Xdpy, DefaultScreen(Xdpy), &oldx, &oldy);
#endif /* HAVE_XF86_VMODE */
    
    /* Find an OpenGL-capable RGB visual with depth buffer. */
    getVisual();
    
    CREATE_TOOLKIT_MAIN_WINDOW;
    
    if (RUNNINGASPLUGIN) {
	sendXwinToPlugin();
    }
}

XVisualInfo *find_best_visual(int shutter,int *attributes,int len)
{
   XVisualInfo *vi=NULL;
   int attrib;
   int startattrib=0;
   int *attrib_mem;

   attrib_mem=(int *)MALLOC (len*sizeof(int)+sizeof(default_attributes0));


   quadbuff_stereo_mode=0;
   if (!shutter)
      startattrib=1;
   else
      {
#ifdef STEREOCOMMAND
      system(STEREOCOMMAND);
#endif
      }
   for (attrib=startattrib;attrib<2;attrib++) {
      int idepth;
      for (idepth=0;idepth<sizeof(legal_depth_list)/sizeof(int);idepth++) {
	 int redsize;
	 for (redsize=8;redsize>=4;redsize--) {
	    int i;
	    int* attribs_pointer=default_attributes0;
	    int  attribs_size=sizeof(default_attributes0)/sizeof(int);
	    if (attrib==1) {
	       attribs_pointer=default_attributes1;
	       attribs_size=sizeof(default_attributes1)/sizeof(int);
	    }
	    if (attrib==2) {
	       attribs_pointer=default_attributes2;
	       attribs_size=sizeof(default_attributes2)/sizeof(int);
	    }
	    if (attrib==3) {
	       attribs_pointer=default_attributes3;
	       attribs_size=sizeof(default_attributes3)/sizeof(int);
	    }
	    attribs_pointer[1]=legal_depth_list[idepth];
	    if ((attrib==0) || (attrib==1))
	       attribs_pointer[3]=redsize;

	    for (i=0;i<len;i++)
	       attrib_mem[i]=attributes[i];
	    for (i=0;i<attribs_size;i++)
	       attrib_mem[i+len]=attribs_pointer[i];

      	    /* get an appropriate visual */
	    vi = glXChooseVisual(Xdpy, Xscreen, attrib_mem);
	    if (vi) {
	       if (attrib==0) {
		  quadbuff_stereo_mode=1;
	       }
	    /* save the display depth for snapshots, etc */
	    displayDepth = legal_depth_list[idepth];

	    FREE_IF_NZ (attrib_mem);
	    return vi;
	    }
	 }
      }
   }
   FREE_IF_NZ(attrib_mem);
   return(NULL);
}

void resetGeometry()
{
#ifdef HAVE_XF86_VMODE
    int oldMode, i;

    if (fullscreen) {
	XF86VidModeGetAllModeLines(Xdpy, Xscreen, &vmode_nb_modes, &vmode_modes);
	oldMode = 0;
	
	for (i=0; i < vmode_nb_modes; i++) {
	    if ((vmode_modes[i]->hdisplay == oldx) && (vmode_modes[i]->vdisplay==oldy)) {
		oldMode = i;
		break;
	    }
	}
	
	XF86VidModeSwitchToMode(Xdpy, Xscreen, vmode_modes[oldMode]);
	XF86VidModeSetViewPort(Xdpy, Xscreen, 0, 0);
	XFlush(Xdpy);
    }
#endif /* HAVE_XF86_VMODE */
}


static int catch_XLIB (Display *disp, XErrorEvent *err) {
	printf ("FreeWRL caught an XLib error on Display:%s We are just going to ignore error:%d request:%d and continue on\n",
		XDisplayName(NULL), err->error_code, err->request_code);

	XLIB_errors++;
	if (XLIB_errors > 20) {
		printf ("FreeWRL - too many XLib errors, exiting...\n");
		exit(0);
	}
	return 0;

}
