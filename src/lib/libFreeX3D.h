/*******************************************************************
 *
 * FreeX3D support library
 *
 * public API - libFreeX3D.h
 *
 * $Id: libFreeX3D.h,v 1.3 2008/11/03 14:14:12 couannette Exp $
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
int be_collision;
char *keypress_string;

/**
 * Plugin functions
 */

/**
 * Plugin variables
 */
int isBrowserPlugin;
int _fw_pipe;
int _fw_browser_plugin;
unsigned _fw_instance;

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
void setLineWidth(float lwidth);

/**
 * Display variables
 */
int feHeight, feWidth;
int fullscreen;
float gl_linewidth;

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
void setEaiVerbose();

/**
 * EAI variables
 */
int wantEAI;
int EAIverbose;

/**
 * Version embedded
 */
extern const char *libFreeX3D_get_version();


#endif /* __LIBFREEX3D_MAIN_H__ */
