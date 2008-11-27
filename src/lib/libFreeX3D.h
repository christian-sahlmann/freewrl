/*******************************************************************
 *
 * FreeX3D support library
 *
 * public API - libFreeX3D.h
 *
 * $Id: libFreeX3D.h,v 1.8 2008/11/27 00:27:17 couannette Exp $
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
int win_height;
int win_width;
int fullscreen;
char *BrowserFullPath;
pthread_t DispThrd;


#endif /* __LIBFREEX3D_MAIN_H__ */
