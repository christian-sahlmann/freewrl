/*******************************************************************
 *
 * FreeX3D support library
 *
 * public API - libFreeX3D.h
 *
 * $Id: libFreeX3D.h,v 1.13 2008/12/19 22:09:17 sdumoulin Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_MAIN_H__
#define __LIBFREEX3D_MAIN_H__

/**
 * Version embedded
 */
const char *libFreeX3D_get_version();


/**
 * Initialization
 */
int initFreeX3D();
void closeFreeX3D();

/**
 * General functions
 */
int ConsoleMessage(const char *fmt, ...);
int checkNetworkFile(char *fn);
void setFullPath(const char* file);
void makeAbsoluteFileName(char *filename, char *pspath,char *thisurl);
void create_EAI();
void create_MIDIEAI();
void doQuit();
void Anchor_ReplaceWorld(char *name);

/**
 * General variables
 */

#define RUNNINGASPLUGIN (isBrowserPlugin)

extern int win_height;
extern int win_width;
extern int fullscreen;
extern char *BrowserFullPath;
extern pthread_t DispThrd;
extern int _fw_pipe, _fw_FD;
extern int _fw_browser_plugin;
extern int isBrowserPlugin;
extern uintptr_t _fw_instance;
extern int be_collision;
extern char *keypress_string;


#endif /* __LIBFREEX3D_MAIN_H__ */
