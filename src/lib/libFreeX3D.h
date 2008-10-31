/*******************************************************************
 *
 * FreeX3D support library
 *
 * public API - libFreeX3D.h
 *
 * $Id: libFreeX3D.h,v 1.2 2008/10/31 19:30:36 couannette Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_MAIN_H__
#define __LIBFREEX3D_MAIN_H__


/**
 * Initialization
 */
void initFreewrl();
void closeFreewrl();

/**
 * General functions
 */
int ConsoleMessage(const char *fmt, ...);

/**
 * General variables
 */
char *BrowserFullPath;

/**
 * Network functions
 */
int checkNetworkFile(char *fn);
void setFullPath(const char* file);
void makeAbsoluteFileName(char *filename, char *pspath,char *thisurl);

/**
 * Network variables
 */

/**
 * Display functions
 */
void resetGeometry();

/**
 * Display variables
 */
int fullscreen;

/**
 * Threading functions
 */
#define STOP_DISPLAY_THREAD \
        if (DispThrd != NULL) { \
                quitThread = TRUE; \
                pthread_join(DispThrd,NULL); \
                DispThrd = NULL; \
        }

void displayThread();

/**
 * Threading variables
 */
pthread_t DispThrd;
int quitThread;

/**
 * EAI functions
 */
void create_EAI();
void shutdown_EAI(void);

/**
 * EAI variables
 */
int wantEAI;

/**
 * Version embedded
 */
extern const char *libFreeX3D_get_version();


#endif /* __LIBFREEX3D_MAIN_H__ */
