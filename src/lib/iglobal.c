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
void SenseInterps_init(struct tSenseInterps *t);
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
	SenseInterps_init(&iglobal->SenseInterps);
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

	uiThread = pthread_self();
	set_thread2global(iglobal, uiThread );
	return iglobal;
}
void iglobal_destructor(ttglobal tg)
{
	//call individual destructors in reverse order to constructor
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
	FREE_IF_NZ(tg->SenseInterps.prv);
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
	thread2global[nglobalthreads].thread = any;
	thread2global[nglobalthreads].iglobal = fwl;
	nglobalthreads++;
}

ttglobal gglobal()
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
	if(ifound == 0)
	{
		printf("ouch - no state for this thread - hit a key to exit\n");
		getchar();
	}
	return iglobal;
}
