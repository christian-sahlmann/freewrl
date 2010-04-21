/*
  $Id: libFreeWRL.h,v 1.23 2010/04/21 17:09:04 sdumoulin Exp $

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

/**
 * Version embedded
 */
const char *libFreeWRL_get_version();

/**
 * Initialization
 */
#ifdef IPHONE
//int Init(ESContext *esContext);
//GLuint LoadShader(GLenum type, const char* shaderSrc);
//void Draw(ESContext* esContext);
#endif
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
#ifdef _MSC_VER
freewrl_params_t fw_params;
#else
extern freewrl_params_t fw_params;
#endif

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
void doQuit();

/* void Anchor_ReplaceWorld(char *name); */
bool Anchor_ReplaceWorld();

#define VIEWER_NONE 0	  /* would have conflicted with another NONE definition */
#define VIEWER_EXAMINE 1
#define VIEWER_WALK 2
#define VIEWER_EXFLY 3
#define VIEWER_FLY 4
#define VIEWER_YAWPITCHZOOM 5
void set_viewer_type(const int type);

void setTexSize(int requestedsize);
void setGeometry_from_cmdline(const char *gstring);
void setSnapFile(const char* file);
void setSnapTmp(const char* file);
void setEaiVerbose();
void setScreenDist(const char *optArg);
void setStereoParameter(const char *optArg);
void setShutter(void);
/* void setXEventStereo(); */
void setEyeDist(const char *optArg);

void setAnaglyphParameter(const char *optArg);
void setSideBySide(void);
void setStereoBufferStyle(int);
void initStereoDefaults(void);

void setLineWidth(float lwidth);
void setSnapGif();
void setPrintShot();

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

void askForRefreshOK();
int checkRefresh();
void resetRefresh();

#endif /* __LIBFREEWRL_API_H__ */
