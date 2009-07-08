/*******************************************************************
 *
 * FreeWRL support library
 *
 * public API - libFreeWRL.h
 *
 * $Id: libFreeWRL.h,v 1.2.2.1 2009/07/08 21:55:04 couannette Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEWRL_MAIN_H__
#define __LIBFREEWRL_MAIN_H__

/**
 * Version embedded
 */
const char *libFreeWRL_get_version();

/**
 * Initialization
 */
void initFreewrl(); /* FIXME: clean up those scatered entry points... */
void closeFreewrl();
int initFreeWRL();
void closeFreeWRL();
void startFreeWRL();

/**
 * General functions
 */
#if defined(WIN32)
# define ConsoleMessage printf
#else
int ConsoleMessage(const char *fmt, ...);
#endif

int checkNetworkFile(char *fn);
void setFullPath(const char* file);
void makeAbsoluteFileName(char *filename, char *pspath,char *thisurl);
void create_EAI();
void create_MIDIEAI();
void doQuit();
void Anchor_ReplaceWorld(char *name);
void setTexSize(int requestedsize);
void setGeometry_from_cmdline(const char *gstring);
void setSnapFile(const char* file);
void setSnapTmp(const char* file);
void setEaiVerbose();
void setScreenDist(const char *optArg);
void setStereoParameter(const char *optArg);
void setShutter(void);
void setXEventStereo();
void setEyeDist(const char *optArg);
void setLineWidth(float lwidth);
void setSnapGif();

/**
 * General variables
 */

#define RUNNINGASPLUGIN (isBrowserPlugin)

extern int win_height;
extern int win_width;
extern int fullscreen;
extern char *BrowserFullPath;

extern int _fw_pipe, _fw_FD;
extern int _fw_browser_plugin;
extern int isBrowserPlugin;
extern uintptr_t _fw_instance;
extern int be_collision;
extern char *keypress_string;


#endif /* __LIBFREEWRL_MAIN_H__ */
