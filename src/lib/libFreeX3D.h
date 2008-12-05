/*******************************************************************
 *
 * FreeX3D support library
 *
 * public API - libFreeX3D.h
 *
 * $Id: libFreeX3D.h,v 1.10 2008/12/05 13:20:52 couannette Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_MAIN_H__
#define __LIBFREEX3D_MAIN_H__

/**
 * Version embedded
 */
extern const char *libFreeX3D_get_version();


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
void doQuit();
void Anchor_ReplaceWorld(char *name);

/**
 * General variables
 */

#define RUNNINGASPLUGIN (isBrowserPlugin)

int win_height;
int win_width;
int fullscreen;
char *BrowserFullPath;
pthread_t DispThrd;
int _fw_pipe, _fw_FD;
int _fw_browser_plugin;
int isBrowserPlugin;
uintptr_t _fw_instance;
int be_collision;
char *keypress_string;


#endif /* __LIBFREEX3D_MAIN_H__ */
