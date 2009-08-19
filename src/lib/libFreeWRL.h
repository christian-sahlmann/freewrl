/*******************************************************************
 *
 * FreeWRL support library
 *
 * public API - libFreeWRL.h
 *
 * $Id: libFreeWRL.h,v 1.6 2009/08/19 04:16:49 dug9 Exp $
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

#if defined(_MSC_VER)
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_   /* to block loading of older winsock. Or #define WIN32_LEAN_AND_MEAN */
#endif

#include <windows.h>
__inline double Time1970sec()
{
   SYSTEMTIME mytimet; /*winNT and beyond */
   /* the windows getlocaltime has a granularity of 1ms at best. 
   There are a gazillion time functions in windows so I isolated it here in case I got it wrong*/
		/* win32 there are some higher performance timer functions (win95-vista)
		but a system might not support it - lpFrequency returns 0 if not supported
		BOOL QueryPerformanceFrequency( LARGE_INTEGER *lpFrequency );
		BOOL QueryPerformanceCounter( LARGE_INTEGER *lpPerformanceCount );
		*/

   GetLocalTime(&mytimet);
   return (double) mytimet.wHour*3600.0 + (double)mytimet.wMinute*60.0 + (double)mytimet.wSecond + (double)mytimet.wMilliseconds/1000.0;
}

#else
inline double Time1970sec()
{
  struct timeval tv;
  (void) gettimeofday(&tv, (struct timezone *)NULL);
  return (tv.tv_sec + tv.tv_usec / 1000000.0);
}
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
/* void setXEventStereo(); */
void setEyeDist(const char *optArg);

void setAnaglyphParameter(const char *optArg);
void setSideBySide(void);
void setStereoBufferStyle(int);

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

#ifdef HAVE_LIBCURL
extern int with_libcurl;
#endif

#if defined(_MSC_VER)
#define FALSE 0
#define TRUE 1
#endif
#endif /* __LIBFREEWRL_MAIN_H__ */
