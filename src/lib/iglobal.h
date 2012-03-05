/*
The globals have 'private' and 'public' facilities
1. if the variables are used only in one source file, they go 
    in a ppFileName struct in Filename.c
2. if the variables are used in other source files (via extern) then either:
2a. add getter and setter functions and keep private, or
2b. add directly to iiglobal struct in iglobal.h, in the struct tFileName sub-struct
2bi. in this case if its a pointer to a complex type, put as void* in iglobal and 
     add casting to the code (so iglobal.h doesn't have to #include a bunch of headers)
 
Variable initialization:
1. for private: in a //private section in FileName_init(..) p->variable = const value
2. for public: in a //public section in FileName_init(..) t->variable = const value
 
Variable use:
1. for private: ppFileName p = (ppFileName)gglobal()->FileName.prv;
-- p->variable = ...
2. for public:  gglobal()->FileName.variable = ...

*/

#define MAXSTAT 200

#ifndef INSTANCEGLOBAL
#include "display.h" //for opengl_utils.h which is for rdr_caps
#include "opengl/OpenGL_Utils.h"  //for rdr_caps
#include "list.h" //for resources.h which is for root_res
#include "resources.h" //for root_res
#include <threads.h> //for threads
#include "vrml_parser/Structs.h" //for SFColor
#include "world_script/JScript.h" //for jsval
#include "x3d_parser/X3DParser.h" //for PARENTSTACKSIZE
#include "ui/common.h" // for ppcommon
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
	struct tSensInterps{
		void *prv;
	} SensInterps;
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
		struct Vector *viewpointNodes;// = NULL;
		int currboundvpno;//=0;
		/* bind nodes in display loop, NOT in parsing threadthread */
		struct X3D_Node *setViewpointBindInRender;// = NULL;
		struct X3D_Node *setFogBindInRender;// = NULL;
		struct X3D_Node *setBackgroundBindInRender;// = NULL;
		struct X3D_Node *setNavigationBindInRender;// = NULL;
		void *savedParser; //struct VRMLParser* savedParser;
		void *prv;
	} ProdCon;
	struct tColladaParser{
		void *prv;
	}ColladaParser;
	struct tFrustum{
		int OccFailed;//. = FALSE;
		void *prv;
	} Frustum;
	struct tLoadTextures{
		/* is the texture thread up and running yet? */
		int TextureThreadInitialized;// = FALSE;
		void *prv;
	}LoadTextures;
	struct tOpenGL_Utils{
		/* is this 24 bit depth? 16? 8?? Assume 24, unless set on opening */
		int displayDepth;// = 24;
		//static float cc_red = 0.0f, cc_green = 0.0f, cc_blue = 0.0f, cc_alpha = 1.0f;
		int cc_changed;// = FALSE;
		void *prv;
	}OpenGL_Utils;
	struct tRasterFont{
		void *prv;
	}RasterFont;
	struct tRenderTextures{
		void *textureParameterStack[10]; //MAX_MULTITEXTURE];
		void *prv;
	}RenderTextures;
	struct tTextures{
		/* for texture remapping in TextureCoordinate nodes */
		GLuint	*global_tcin;
		int	global_tcin_count;
		void 	*global_tcin_lastParent;
		GLuint defaultBlankTexture;
		void *prv;
	}Textures;
	struct tPluginSocket{
		void *prv;
	}PluginSocket;
	struct tpluginUtils{
		void *prv;
	}pluginUtils;
	struct tcollision{
		void *prv;
	}collision;
	struct tComponent_EnvironSensor{
		void *prv;
	}Component_EnvironSensor;
	struct tComponent_Geometry3D{
		void *prv;
	}Component_Geometry3D;
	struct tComponent_Geospatial{
		void *prv;
	}Component_Geospatial;
	struct tComponent_HAnim{
		void *prv;
	}Component_HAnim;
	struct tComponent_KeyDevice{
		void *prv;
	}Component_KeyDevice;

#ifdef OLDCODE
iOLDCODE	struct tComponent_Networking{
iOLDCODE		void *ReWireNamenames;
iOLDCODE		int ReWireNametableSize;
iOLDCODE		void *ReWireDevices;
iOLDCODE		int ReWireDevicetableSize;
iOLDCODE		void *prv;
iOLDCODE	}Component_Networking;
#endif // OLDCODE

#ifdef DJTRACK_PICKSENSORS
	struct tComponent_Picking{
		void *prv;
	}Component_Picking;
#endif
	struct tComponent_Shape{
		void *prv;
	}Component_Shape;
	struct tComponent_Sound{
		int sound_from_audioclip;// = 0;
		/* is the sound engine started yet? */
		int SoundEngineStarted;// = FALSE;
		void *prv;
	}Component_Sound;
	struct tComponent_Text{
		void *prv;
	}Component_Text;
	struct tComponent_VRML1{
		void *prv;
	}Component_VRML1;
	struct tRenderFuncs{
		char *OSX_last_world_url_for_reload;
		char *OSX_replace_world_from_console;
		/* Any action for the Browser to do? */
		int BrowserAction;// = FALSE;
		double hitPointDist; /* distance in ray: 0 = r1, 1 = r2, 2 = 2*r2-r1... */
		/* used to save rayhit and hyperhit for later use by C functions */
		struct SFColor hyp_save_posn, hyp_save_norm, ray_save_posn;
		void *hypersensitive;//= 0; 
		int hyperhit;// = 0;
		struct point_XYZ hp;
		void *prv;
		void *rayHit;
		void *rayHitHyper;
		struct point_XYZ t_r1,t_r2,t_r3; /* transformed ray */
		int	lightingOn;		/* do we need to restore lighting in Shape? */
		int	have_transparency;//=FALSE;/* did any Shape have transparent material? */
		/* material node usage depends on texture depth; if rgb (depth1) we blend color field
		   and diffusecolor with texture, else, we dont bother with material colors */
		int last_texture_type;// = NOTEXTURE;
		/* texture stuff - see code. Need array because of MultiTextures */
		GLuint boundTextureStack[10];//MAX_MULTITEXTURE];
		int textureStackTop;
	}RenderFuncs;
	struct tStreamPoly{
		void *prv;
	}StreamPoly;
	struct tTess{
		int *global_IFS_Coords;
		int global_IFS_Coord_count;//=0;
#if !defined(IPHONE) && !defined(_ANDROID) && !defined(GLES2)
		GLUtriangulatorObj *global_tessobj;
#else
		int global_tessobj;
#endif /* IPHONE */

		void *prv;
	}Tess;
	struct tViewer{
		void *prv;
	}Viewer;
	struct tstatusbar{
		void *prv;
	}statusbar;
	struct tCParse{
		void* globalParser;
		void *prv;
	}CParse;
	struct tCParseParser{
		void *prv;
	}CParseParser;
	struct tCProto{
		void *prv;
	}CProto;
	struct tCRoutes{
		/* EAI needs the extra parameter, so we put it globally when a RegisteredListener is clicked. */
		int CRoutesExtra;// = 0;
		jsval JSglobal_return_val;
		void *JSSFpointer;
		int *scr_act;// = 0;				/* this script has been sent an eventIn */
		int max_script_found;// = -1;			/* the maximum script number found */
		int max_script_found_and_initialized;// = -1;	/* the maximum script number found */

		void *prv;
	}CRoutes;
	struct tCScripts{
		void *prv;
	}CScripts;
	struct tJScript{
		int jsnameindex; //= -1;
		int MAXJSparamNames;// = 0;

		void *prv;
	}JScript;
	struct tjsUtils{
		void *prv;
	}jsUtils;
	struct tjsVRMLBrowser{
		/* for setting field values to the output of a CreateVrml style of call */
		/* it is kept at zero, unless it has been used. Then it is reset to zero */
		jsval JSCreate_global_return_val;
		void *prv;
	}jsVRMLBrowser;
	struct tjsVRMLClasses{
		void *prv;
	}jsVRMLClasses;
	struct tBindable{
		struct sNaviInfo naviinfo;
        	struct Vector *background_stack;
        	struct Vector *viewpoint_stack;
        	struct Vector *navigation_stack;
        	struct Vector *fog_stack;
		void *prv;
	}Bindable;
	struct tX3DParser{
		int parentIndex;// = -1;
		struct X3D_Node *parentStack[PARENTSTACKSIZE];
		char *CDATA_Text;// = NULL;
		int CDATA_Text_curlen;// = 0;
		void *prv;
	}X3DParser;
	struct tX3DProtoScript{
		void *prv;
	}X3DProtoScript;
	struct tcommon{
		void *prv;
	}common;
} * ttglobal;
#define INSTANCEGLOBAL 1
#endif
ttglobal  iglobal_constructor(); 
void iglobal_destructor(ttglobal);
void set_thread2global(ttglobal fwl, pthread_t any , char *desc);
ttglobal gglobal(); //gets based on threadID, errors out if no threadID
ppcommon gglobal_common(); // lets the front end get the myMenuStatus without hassle
ttglobal gglobal0(); //will return null if thread not yet initialized
ttglobal gglobalH(void *handle); //use window handle
ttglobal gglobalH0(void *handle); //test if window handle is in the table yet
