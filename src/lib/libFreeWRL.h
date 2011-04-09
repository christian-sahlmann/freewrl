/*
  $Id: libFreeWRL.h,v 1.28 2011/04/09 00:33:19 davejoubert Exp $

  FreeWRL library API (public)

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


#ifndef __LIBFREEWRL_API_H__
#define __LIBFREEWRL_API_H__

#ifdef COMPILING_IPHONE_FRONT_END
	/* Ok, ok, ok. I know. Another definition. But, Objective-C gives lots of
 	   errors if the whole file is included, and also, we only need a couple of
	   definitions to keep the front end as separate from the library as possible... */

	void fwl_initializeRenderSceneUpdateScene(void);
	char *frontEndWantsFileName(void);
	void RenderSceneUpdateScene(void);
	

#else /* COMPILING_IPHONE_FRONT_END */
/**
 * Version embedded
 */
const char *libFreeWRL_get_version();

/**
 * Initialization
 */
typedef struct freewrl_params {
	/* Put here global parameters, parsed in main program
	   and needed to initialize libFreeWRL
	   example: width, height, fullscreen, multithreading, eai...
	*/
	int width;
	int height;
	long int winToEmbedInto;
	bool fullscreen;
	bool multithreading;
	bool eai;
	bool verbose;
	int collision;	/* do collision detection? */

} freewrl_params_t;

/* FreeWRL parameters */
extern freewrl_params_t fw_params;

bool initFreeWRL(freewrl_params_t *params);
void startFreeWRL(const char *url);
void closeFreeWRL();
void terminateFreeWRL();

/**
 * General functions
 */
int ConsoleMessage(const char *fmt, ...);
//#endif

void create_EAI();
void create_MIDIEAI();
/* void doQuit(); */

/* void Anchor_ReplaceWorld(char *name); */
bool Anchor_ReplaceWorld();

#define VIEWER_NONE 0	  /* would have conflicted with another NONE definition */
#define VIEWER_EXAMINE 1
#define VIEWER_WALK 2
#define VIEWER_EXFLY 3
#define VIEWER_FLY 4
#define VIEWER_YAWPITCHZOOM 5
void set_viewer_type(const int type);

/* Hmm.., this is actually a frontend call void fv_setGeometry_from_cmdline(const char *gstring);*/
/* void setSnapFile(const char* file); */
/* void setSnapTmp(const char* file); */
void setEaiVerbose();
void setScreenDist(const char *optArg);
void setStereoParameter(const char *optArg);
void setShutter(void);
/* void setXEventStereo(); */
void setEyeDist(const char *optArg);

void setAnaglyphParameter(const char *optArg);
void setSideBySide(void);
void setStereoBufferStyle(int);
/* void initStereoDefaults(void); */

/* void setLineWidth(float lwidth); */
/* void setSnapGif(); */
/* void setPrintShot(); */

/**
 * General variables
 */

#define RUNNINGASPLUGIN (isBrowserPlugin)

extern char *BrowserFullPath;

extern int _fw_pipe, _fw_FD;
extern int _fw_browser_plugin;
extern int isBrowserPlugin;
extern uintptr_t _fw_instance;
extern char *keypress_string;

#ifdef HAVE_LIBCURL
extern int with_libcurl;
#endif

/* void askForRefreshOK(); */
/* int checkRefresh(); */
/* void resetRefresh(); */

#endif /* COMPILING_IPHONE_FRONT_END */

/* ** NEW DJ ** */

void fwl_set_strictParsing(bool flag);
void fwl_set_plugin_print(bool flag);
void fwl_set_occlusion_disable(bool flag);
void fwl_set_print_opengl_errors(bool flag);
void fwl_set_trace_threads(bool flag);
void fwl_set_use_VBOs(bool flag);
void fwl_set_texture_size(unsigned int texture_size);

/* ** REPLACE DJ ** */
void fwl_init_StereoDefaults(void);
void fwl_init_SnapGif(void);
void fwl_init_PrintShot();
void fwl_set_SnapFile(const char* file);
void fwl_set_SnapTmp(const char* file);
void fwl_init_SnapSeq(); /* Was in main/headers.h */
void fwl_toggleSnapshot();
void fwl_set_LineWidth(float lwidth);
void fwl_set_KeyString(const char *str);
void fwl_set_SeqFile(const char* file);
void fwl_set_MaxImages(int max); 
void fwl_setCurXY(int x, int y);
void fwl_do_keyPress(char kp, int type);
void fwl_doQuit();

void fwl_askForRefreshOK();
int  fwl_checkRefresh();
void fwl_resetRefresh();

void fwl_Next_ViewPoint(void);
void fwl_Prev_ViewPoint(void);
void fwl_First_ViewPoint(void);
void fwl_Last_ViewPoint(void);

void fwl_OSX_initializeParameters(const char* initialURL);
void fwl_frontEndReturningData(unsigned char *dataPointer, int len);
int fwl_display_initialize(void);
void fwl_setScreenDim(int wi, int he);
void fwl_setLastMouseEvent(int etype);
void fwl_handle_aqua(const int mev, const unsigned int button, int x, int y);

#endif /* __LIBFREEWRL_API_H__ */
