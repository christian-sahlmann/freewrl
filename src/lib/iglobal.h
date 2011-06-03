#ifndef INSTANCEGLOBAL
#include "display.h" //for opengl_utils.h which is for rdr_caps
#include "opengl/OpenGL_Utils.h"  //for rdr_caps
#include "list.h" //for resources.h which is for root_res
#include "resources.h" //for root_res

typedef struct iiglobal //InstanceGlobal
{
	struct tdisplay{
		GLenum _global_gl_err;
		bool display_initialized;// = FALSE;

		int win_height;// = 0; /* window */
		int win_width;// = 0;
		long int winToEmbedInto;// = -1;
		int fullscreen;// = FALSE;
		int view_height;// = 0; /* viewport */
		int view_width;// = 0;

		int screenWidth;// = 0; /* screen */
		int screenHeight;// = 0;

		double screenRatio;// = 1.5;

		char *window_title;// = NULL;

		int mouse_x;
		int mouse_y;

		int show_mouse;

		int xPos;// = 0;
		int yPos;// = 0;

		int shutterGlasses;// = 0; /* stereo shutter glasses */
		int quadbuff_stereo_mode;// = 0;

		s_renderer_capabilities_t rdr_caps;

		float myFps;// = (float) 0.0;
		char myMenuStatus[MAXSTAT];
		void *prv;
	}display;
	struct tinternalc {
		bool global_strictParsing;// = FALSE;
		bool global_plugin_print;// = FALSE;
		bool global_occlusion_disable;// = FALSE;
		unsigned global_texture_size;// = 0;
		bool global_print_opengl_errors;// = FALSE;
		bool global_trace_threads;// = FALSE;

		/* having trouble with VBOs, make false unless otherwise told to do so */
		#ifdef SHADERS_2011
			bool global_use_VBOs;// = TRUE;
		#else
			bool global_use_VBOs;// = FALSE;
		#endif /* SHADERS_2011 */
		void *prv;
	} internalc;
	struct tio_http {
		void *prv;
	} io_http;
	struct tresources {
		resource_item_t *root_res; // = NULL;
		void *prv;
	} resources;
	struct tthreads {
		pthread_t mainThread; /* main (default) thread */
		pthread_t DispThrd; /*DEF_THREAD(DispThrd); display thread */
		pthread_t PCthread; /* DEF_THREAD(PCthread)parser thread */
		pthread_t loadThread; /* DEF_THREAD(pthread_t loadThread)texture thread */
		/* Synchronize / exclusion root_res and below */
		pthread_mutex_t mutex_resource_tree; // = PTHREAD_MUTEX_INITIALIZER;

		/* Synchronize / exclusion : resource queue for parser */
		pthread_mutex_t mutex_resource_list; // = PTHREAD_MUTEX_INITIALIZER;
		pthread_cond_t resource_list_condition; // = PTHREAD_COND_INITIALIZER;

		/* Synchronize / exclusion (main<=>texture) */
		pthread_mutex_t mutex_texture_list; // = PTHREAD_MUTEX_INITIALIZER;
		pthread_cond_t texture_list_condition; // = PTHREAD_COND_INITIALIZER;
		void *prv;
	} threads;
	struct tconvert1To2 {
		void *prv;
	} convert1To2;
	struct tSnapshot {
		bool doSnapshot;
		bool doPrintshot;
		int snapGoodCount;
		void *prv;
	} Snapshot;
	struct tEAI_C_CommonFunctions {
		int eaiverbose;// = FALSE;
		void *prv;
	} EAI_C_CommonFunctions;
	struct tEAIEventsIn{
		void *prv;
	} EAIEventsIn;
	struct tEAIHelpers{
		char *outBuffer;
		int outBufferLen;
		void *prv;
	} EAIHelpers;
	struct tEAIServ{
		int	EAIlistenfd;// = -1;			/* listen to this one for an incoming connection*/
		/* EAI input buffer */
		char *EAIbuffer;
		int EAIbufcount;				/* pointer into buffer*/
		int EAIbufsize;				/* current size in bytes of input buffer*/
		char EAIListenerData[8192]; //EAIREADSIZE]; /* this is the location for getting Listenered data back again.*/
		int	EAIMIDIlistenfd;// = -1;		/* listen on this socket for an incoming connection for MIDI EAI */
		void *prv;
	} EAIServ;
	struct tSenseInterps{
		void *prv;
	} SenseInterps;
	struct tConsoleMessage{
		int consMsgCount;
		int Console_writeToHud;
		void *prv;
	} ConsoleMessage;
	struct tMainloop{
		float gl_linewidth;
		/* what kind of file was just parsed? */
		int currentFileVersion;
		double TickTime;
		double lastTime;
		double BrowserFPS;// = 100.0;        /* calculated FPS               */
		double BrowserSpeed;// = 0.0;      /* calculated movement speed    */
		int HaveSensitive;// = FALSE;
		int trisThisLoop;
		int clipPlane;// = 0;
		int currentX[20], currentY[20];                 /*  current mouse position.*/
		void *prv;
	} Mainloop;
	struct tProdCon{
		void* *viewpointnodes;// = NULL;
		int totviewpointnodes;// = 0;
		int currboundvpno;//=0;
		/* bind nodes in display loop, NOT in parsing threadthread */
		void *setViewpointBindInRender;// = NULL;
		void *setFogBindInRender;// = NULL;
		void *setBackgroundBindInRender;// = NULL;
		void *setNavigationBindInRender;// = NULL;
		void *savedParser; //struct VRMLParser* savedParser;

		void *prv;
	} ProdCon;
	struct tColladaParser{
		void *prv;
	}ColladaParser;
} * ttglobal;
#define INSTANCEGLOBAL 1
#endif
ttglobal  iglobal_constructor(); 
void iglobal_destructor(ttglobal);
void set_thread2global(ttglobal fwl, pthread_t any );
ttglobal gglobal(); //gets based on threadID