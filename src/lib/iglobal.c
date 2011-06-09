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
void Component_Picking_init(struct tComponent_Picking *t);
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
void statusbar_init(struct tstatusbar *t);
void CParse_init(struct tCParse *t);

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
	statusbar_init(&iglobal->statusbar);
	CParse_init(&iglobal->CParse);

	uiThread = pthread_self();
	set_thread2global(iglobal, uiThread );
	return iglobal;
}
void iglobal_destructor(ttglobal tg)
{
	//call individual destructors in reverse order to constructor
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
}
#define MAXINSTANCES 25 //max # freewrl windows on a web page / InternetExplorer session
struct t2g {
	pthread_t thread;
	ttglobal iglobal;
};
static struct t2g thread2global[MAXINSTANCES*5];
static int nglobalthreads = 0;
void set_thread2global(ttglobal fwl, pthread_t any )
{
printf ("set_thread2global, thread %p\n",any);
	thread2global[nglobalthreads].thread = any;
	thread2global[nglobalthreads].iglobal = fwl;
	nglobalthreads++;
}

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
			iglobal = thread2global[i].iglobal;
			ifound = 1;
			break;
		}
	return iglobal;
}
ttglobal gglobal()
{
	ttglobal iglobal = gglobal0();
	if(iglobal == NULL)
	{
#ifdef AQUA
		printf("ouch - no state for this thread -nglobalthreads %d looking for %p\n",nglobalthreads,pthread_self());
return thread2global[0].thread;

#endif
		printf("ouch - no state for this thread - hit a key to exit\n");
		getchar();
		//let it bomb exit(-1);
	}
	return iglobal;
}
