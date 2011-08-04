#include <internal.h> //needed for opengl_utils.h included in iglobal.h
//#include <config.h>
//#include <system.h>
//#include <system_threads.h>
//#include <internal.h>
//#include <display.h>
//#include <threads.h>
//#include <libFreeWRL.h>
//
//#include "vrml_parser/Structs.h"
//#include "opengl/Textures.h"
//#include "opengl/RasterFont.h"
//#include "opengl/OpenGL_Utils.h"
#include <iglobal.h>


void display_init(struct tdisplay* d);
void internalc_init(struct tinternalc* ic);
void io_http_init(struct tio_http* t);
void threads_init(struct tthreads* t);
void convert1To2_init(struct tconvert1To2* t);
void Snapshot_init(struct tSnapshot *);
void EAI_C_CommonFunctions_init(struct tEAI_C_CommonFunctions*);
void EAIEventsIn_init(struct tEAIEventsIn* t);
void EAIHelpers_init(struct tEAIHelpers* t);
void EAIServ_init(struct tEAIServ* t);
void SensInterps_init(struct tSensInterps *t);
void ConsoleMessage_init(struct tConsoleMessage *t);
void Mainloop_init(struct tMainloop *t);
void ProdCon_init(struct tProdCon *t);
void ColladaParser_init(struct tColladaParser *t);
void Frustum_init(struct tFrustum *t);
void LoadTextures_init(struct tLoadTextures *t);
void OpenGL_Utils_init(struct tOpenGL_Utils *t);
void RasterFont_init(struct tRasterFont *t);
void RenderTextures_init(struct tRenderTextures *t);
void Textures_init(struct tTextures *t);
void PluginSocket_init(struct tPluginSocket *t);
void pluginUtils_init(struct tpluginUtils *t);
void collision_init(struct tcollision *t);
void Component_EnvironSensor_init(struct tComponent_EnvironSensor *t);
void Component_Geometry3D_init(struct tComponent_Geometry3D *t);
void Component_Geospatial_init(struct tComponent_Geospatial *t);
void Component_HAnim_init(struct tComponent_HAnim *t);
void Component_KeyDevice_init(struct tComponent_KeyDevice *t);

#ifdef OLDCODE
OLDCODEvoid Component_Networking_init(struct tComponent_Networking *t);
#endif

#ifdef DJTRACK_PICKSENSORS
void Component_Picking_init(struct tComponent_Picking *t);
#endif

void Component_Shape_init(struct tComponent_Shape *t);
void Component_Sound_init(struct tComponent_Sound *t);

#if !(defined(IPHONE) || defined(_ANDROID))
void Component_Text_init(struct tComponent_Text *t);
#endif

void Component_VRML1_init(struct tComponent_VRML1 *t);
void RenderFuncs_init(struct tRenderFuncs *t);
void StreamPoly_init(struct tStreamPoly *t);
void Tess_init(struct tTess *t);
void Viewer_init(struct tViewer *t);

#if defined(STATUSBAR_HUD)
void statusbar_init(struct tstatusbar *t);
#endif

void CParse_init(struct tCParse *t);
void CParseParser_init(struct tCParseParser *t);
void CProto_init(struct tCProto *t);
void CRoutes_init(struct tCRoutes *t);
void CScripts_init(struct tCScripts *t);
void JScript_init(struct tJScript *t);

#ifdef HAVE_JAVASCRIPT
void jsUtils_init(struct tjsUtils *t);
void jsVRMLBrowser_init(struct tjsVRMLBrowser *t);
void jsVRMLClasses_init(struct tjsVRMLClasses *t);
#endif
void Bindable_init(struct tBindable *t);
void X3DParser_init(struct tX3DParser *t);
void X3DProtoScript_init(struct tX3DProtoScript *t);
void common_init(struct tcommon *t);

//static ttglobal iglobal; //<< for initial development witn single instance
ttglobal  iglobal_constructor() //(mainthreadID,parserthreadID,texturethreadID...)
{
	//using Johns threadID method would:
	//1. create global struct
	// - malloc
	// - initialize any that have initializers
	//2. add 3 items to the thread2global[] list
	//3. for each of those 3 items:
	//   - set thread2global[threadID] = global
	pthread_t uiThread;
	ttglobal iglobal = malloc(sizeof(struct iiglobal));
	memset(iglobal,0,sizeof(struct iiglobal)); //set to zero/null by default

	//call initializer for each sub-struct
	display_init(&iglobal->display);
	internalc_init(&iglobal->internalc);
	io_http_init(&iglobal->io_http);
	//resources_init
	threads_init(&iglobal->threads);
	convert1To2_init(&iglobal->convert1To2);
	Snapshot_init(&iglobal->Snapshot);
	EAI_C_CommonFunctions_init(&iglobal->EAI_C_CommonFunctions);
	EAIEventsIn_init(&iglobal->EAIEventsIn);
	EAIHelpers_init(&iglobal->EAIHelpers);
	EAIServ_init(&iglobal->EAIServ);
	SensInterps_init(&iglobal->SensInterps);
	ConsoleMessage_init(&iglobal->ConsoleMessage);
	Mainloop_init(&iglobal->Mainloop);
	ProdCon_init(&iglobal->ProdCon);
	ColladaParser_init(&iglobal->ColladaParser);
	Frustum_init(&iglobal->Frustum);
	LoadTextures_init(&iglobal->LoadTextures);
	OpenGL_Utils_init(&iglobal->OpenGL_Utils);
	RasterFont_init(&iglobal->RasterFont);
	RenderTextures_init(&iglobal->RenderTextures);
	Textures_init(&iglobal->Textures);
	PluginSocket_init(&iglobal->PluginSocket);
	pluginUtils_init(&iglobal->pluginUtils);
	collision_init(&iglobal->collision);
	Component_EnvironSensor_init(&iglobal->Component_EnvironSensor);
	Component_Geometry3D_init(&iglobal->Component_Geometry3D);
	Component_Geospatial_init(&iglobal->Component_Geospatial);
	Component_HAnim_init(&iglobal->Component_HAnim);
	Component_KeyDevice_init(&iglobal->Component_KeyDevice);
#ifdef OLDCODE
OLDCODE	Component_Networking_init(&iglobal->Component_Networking);
#endif // OLDCODE
#ifdef DJTRACK_PICKSENSORS
	Component_Picking_init(&iglobal->Component_Picking);
#endif
	Component_Shape_init(&iglobal->Component_Shape);
	Component_Sound_init(&iglobal->Component_Sound);
#if !(defined(IPHONE) || defined(_ANDROID))
	Component_Text_init(&iglobal->Component_Text);
#endif
	Component_VRML1_init(&iglobal->Component_VRML1);
	RenderFuncs_init(&iglobal->RenderFuncs);
	StreamPoly_init(&iglobal->StreamPoly);
	Tess_init(&iglobal->Tess);
	Viewer_init(&iglobal->Viewer);
#if defined(STATUSBAR_HUD)
	statusbar_init(&iglobal->statusbar);
#endif
	CParse_init(&iglobal->CParse);
	CParseParser_init(&iglobal->CParseParser);
	CProto_init(&iglobal->CProto);
	CRoutes_init(&iglobal->CRoutes);
	CScripts_init(&iglobal->CScripts);
	JScript_init(&iglobal->JScript);

#ifdef HAVE_JAVASCRIPT
	jsUtils_init(&iglobal->jsUtils);
	jsVRMLBrowser_init(&iglobal->jsVRMLBrowser);
	jsVRMLClasses_init(&iglobal->jsVRMLClasses);
#endif
	Bindable_init(&iglobal->Bindable);
	X3DParser_init(&iglobal->X3DParser);
	X3DProtoScript_init(&iglobal->X3DProtoScript);
	common_init(&iglobal->common);

	uiThread = pthread_self();
	set_thread2global(iglobal, uiThread ,"UI thread");
	return iglobal;
}

void remove_iglobal_from_table(ttglobal tg);
void iglobal_destructor(ttglobal tg)
{
	/* you should have stopped any worker threads for this instance */
	//call individual destructors in reverse order to constructor
	FREE_IF_NZ(tg->common.prv);
	FREE_IF_NZ(tg->X3DProtoScript.prv);
	FREE_IF_NZ(tg->X3DParser.prv);
	FREE_IF_NZ(tg->Bindable.prv);
#ifdef HAVE_JAVASCRIPT
	FREE_IF_NZ(tg->jsVRMLClasses.prv);
	FREE_IF_NZ(tg->jsVRMLBrowser.prv);
	FREE_IF_NZ(tg->jsUtils.prv);
#endif
	FREE_IF_NZ(tg->JScript.prv);
	FREE_IF_NZ(tg->CScripts.prv);
	FREE_IF_NZ(tg->CRoutes.prv);
	FREE_IF_NZ(tg->CProto.prv);
	FREE_IF_NZ(tg->CParseParser.prv);
	FREE_IF_NZ(tg->CParse.prv);
	FREE_IF_NZ(tg->statusbar.prv);
	FREE_IF_NZ(tg->Viewer.prv);
	FREE_IF_NZ(tg->Tess.prv);
	FREE_IF_NZ(tg->StreamPoly.prv);
	FREE_IF_NZ(tg->Component_Sound.prv);
	FREE_IF_NZ(tg->RenderFuncs.prv);
	FREE_IF_NZ(tg->Component_VRML1.prv);
	FREE_IF_NZ(tg->Component_Text.prv);
	FREE_IF_NZ(tg->Component_Shape.prv);
#ifdef DJTRACK_PICKSENSORS
	FREE_IF_NZ(tg->Component_Picking.prv);
#endif
#ifdef OLDCODE
OLDCODE	FREE_IF_NZ(tg->Component_Networking.prv);
#endif
	FREE_IF_NZ(tg->Component_KeyDevice.prv);
	FREE_IF_NZ(tg->Component_HAnim.prv);
	FREE_IF_NZ(tg->Component_Geospatial.prv);
	FREE_IF_NZ(tg->Component_Geometry3D.prv);
	FREE_IF_NZ(tg->Component_EnvironSensor.prv);
	FREE_IF_NZ(tg->collision.prv);
	FREE_IF_NZ(tg->pluginUtils.prv);
	FREE_IF_NZ(tg->PluginSocket.prv);
	FREE_IF_NZ(tg->Textures.prv);
	FREE_IF_NZ(tg->RenderTextures.prv);
	FREE_IF_NZ(tg->RasterFont.prv);
	FREE_IF_NZ(tg->OpenGL_Utils.prv);
	FREE_IF_NZ(tg->LoadTextures.prv);
	FREE_IF_NZ(tg->Frustum.prv);
	FREE_IF_NZ(tg->ColladaParser.prv);
	FREE_IF_NZ(tg->ProdCon.prv);
	FREE_IF_NZ(tg->Mainloop.prv);
	FREE_IF_NZ(tg->ConsoleMessage.prv);
	FREE_IF_NZ(tg->SensInterps.prv);
	FREE_IF_NZ(tg->EAIServ.prv);
	FREE_IF_NZ(tg->EAIHelpers.prv);
	FREE_IF_NZ(tg->EAIEventsIn.prv);
	FREE_IF_NZ(tg->EAI_C_CommonFunctions.prv);
	FREE_IF_NZ(tg->Snapshot.prv);
	FREE_IF_NZ(tg->convert1To2.prv);
	FREE_IF_NZ(tg->threads.prv);
	FREE_IF_NZ(tg->resources.prv);
	FREE_IF_NZ(tg->io_http.prv);
	FREE_IF_NZ(tg->internalc.prv);
	FREE_IF_NZ(tg->display.prv);

	//destroy iglobal
	free(tg);
	remove_iglobal_from_table(tg);
}
#define MAXINSTANCES 25 //max # freewrl windows on a web page / InternetExplorer session
struct t2g {
	pthread_t thread;
	void *handle;
	ttglobal iglobal;
};
static struct t2g thread2global[MAXINSTANCES*5];
static int nglobalthreads = 0;
static void *currentHandle = NULL; /* leave null if single-window application */
/* for multi-window process -such as 2 freewrl widgets on a web page, or
   a console program that creates 2+ popup windows -each with their
   own freewrl instance - then in your window event handler, before
   calling into the freewrl library call fwl_setCurrentHandle(windowHandle)
   and after handling the event call fwl_clearCurrentHandle()
   This allows each window to get/talk to the right freewrl instance
   ASSUMPTION: 1 window per 1 freewrl instance  (window 1:1 iglobal)
	
   */
int iglobal_instance_count();
int fwl_setCurrentHandle(void *handle)
{
	//ConsoleMessage("number of instances = %d\n",iglobal_instance_count());
	currentHandle = handle;
	if(gglobalH0(handle)) return 1; /* let caller know it's in the table */
	return 0; /* let caller know its not in the table yet */
}
void fwl_clearCurrentHandle()
{
	currentHandle = NULL;
}
void set_thread2global(ttglobal fwl, pthread_t any ,char *type)
{
	thread2global[nglobalthreads].thread = any;
	thread2global[nglobalthreads].iglobal = fwl;
	thread2global[nglobalthreads].handle = NULL;
	if(!strcmp(type,"UI thread"))
	{
		/*for the primary thread, we use a 'dipthong' key (threadID,windowHandle)
		because multiple popupWindows or ActiveX controls in the same
		process share the same message loop / event handling thread
		so only the window handle can distinguish the frewrl instances */
		thread2global[nglobalthreads].handle = currentHandle;
	}
	nglobalthreads++;
        //printf ("set_thread2global, thread %p desc: %s\n",any, type);

}
void remove_iglobal_from_table(ttglobal tg)
{
	/*  called from iglobal_destructor. tg should be zombie pointer.
		Perhaps I should lock this table because other threads -in active instances-
		can be doing gglobal(), however they should still be able to find their 
		iglobal.
	*/
	int i,j;
	for(i=0,j=0;i<nglobalthreads;i++)
	{
		memcpy(&thread2global[j],&thread2global[i],sizeof(struct t2g));
		if(thread2global[i].iglobal != tg) j++;
	}
	nglobalthreads = j;
}




#if !(defined(IPHONE) || defined(_ANDROID) || defined(AQUA))
ttglobal gglobal0()
{
	//using Johns threadID method, would:
	//1. detect the current thread and
	//2. lookup in table: global = thread2global[threadID] 
	//3. return if found, else ???
	ttglobal iglobal;
	int i,ifound;
	pthread_t tt = pthread_self();
	iglobal = NULL;
	ifound = 0;
	for(i=0;i<nglobalthreads;i++)
#ifdef _MSC_VER
		if(tt.p == thread2global[i].thread.p){
#else
		if(tt == thread2global[i].thread){
#endif
			/* for primary thread, test to see which window handle is set */
			if(thread2global[i].handle != NULL)
				if(thread2global[i].handle != currentHandle) continue;
			/* for worker threads the threadID is sufficient */
			iglobal = thread2global[i].iglobal;
			ifound = 1;
			break;
		}
	return iglobal;
}
ttglobal gglobalH0(void *handle)
{
	/* same as gglobal0 except use window handle instead of thread */
	ttglobal iglobal;
	int i,ifound;
	iglobal = NULL;
	ifound = 0;
	for(i=0;i<nglobalthreads;i++)
	{
		/* for primary thread, test to see which window handle is set */
		if(thread2global[i].handle != currentHandle) continue;
		iglobal = thread2global[i].iglobal;
		ifound = 1;
		break;
	}
	return iglobal;
}
int iglobal_instance_count()
{
	int i,count;
	void *lastig;
	if(nglobalthreads == 0) return 0;
	count = 0;
	lastig = NULL;
	for(i=0;i<nglobalthreads;i++)
	{
		if(thread2global[i].iglobal != lastig)
		{
			lastig = thread2global[i].iglobal;
			count++;
		}
	}
	return count;
}

#else
int iglobal_instance_count()
{ return 1; }

// on systems where there can only be 1 window running, and when the GUI
// handles the window, simplify the calls so that we do not have to
// register the ui window handling thread (and including associated .h files)
//    AND
// also gets rid of us having to register the OpenGL display thread from the
// front end - when OpenGL calls are required in the FreeWRL library, we know
// the global data area to use.
//
//  SHOULD 
// add a call to register the OpenGL display thread to allow the front end to
// register its thread.

ttglobal gglobal0()
{
	// printf ("gglobal - assuming only 1 thread here\n");
	if (nglobalthreads >=1) {
		return thread2global[0].iglobal;
	} 
	return NULL;
}
ttglobal gglobalH0(void *handle)
{
	return gglobal0();
}
#endif // ANDROID AND IPHONE



ttglobal gglobal()
{
	ttglobal iglobal = gglobal0();
	if(iglobal == NULL)
	{
		printf("ouch - no state for this thread - hit a key to exit\n");
		getchar();
		//let it bomb exit(-1);
	}
	return iglobal;
}
ttglobal gglobalH(void *handle)
{
	ttglobal iglobal = gglobalH0(handle);
	if(iglobal == NULL)
	{
		printf("ouch - no state for this thread - hit a key to exit\n");
		getchar();
		//let it bomb exit(-1);
	}
	return iglobal;
}