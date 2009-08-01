/*
=INSERT_TEMPLATE_HERE=

$Id: display_win32.h,v 1.2 2009/08/01 09:45:39 couannette Exp $

FreeWRL support library.
Internal header: display (X11/Motif or OSX/Aqua) dependencies.

*/

#ifndef __LIBFREEWRL_DISPLAY_WIN32_H__
#define __LIBFREEWRL_DISPLAY_WIN32_H__

#define IDM_ABOUT      1000

/* #define WIN32_LEAN_AND_MEAN 1 */
#include <windows.h>
#include <winuser.h>

/* #ifdef MESA */
/* #ifdef GLEW */
/* /\* http://glew.sourceforge.net/ *\/ */
/* #include "GL/glew.h"  /\* from glew include *\/ */
/* #include "GL/glut.h" /\* from Mesa include *\/ */
/* #else */
/* #include "GL/glut.h" */
/* #endif */
/* #else */
/* wgl / windows opengl version 1.1 */

/* #include <GL/gl.h>  */
/* #include <GL/glu.h>  */
/* #include <GL/glext.h> */

# include <GL/glew.h>

/* /\* I get unsatisfied link references to functions in the shaders *\/  */
/* #undef GL_VERSION_2_0 */
/* #undef GL_VERSION_1_5 */
/*
#include "C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include\gl\gl.h"
#include "C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include\gl\glu.h"
*/
/* #endif */

/* Windows globals, defines, and prototypes */ 
//static CHAR szAppName[]="Win OpenGL"; 
HWND  ghWnd;   /* on a hunch I made these static so they are once per program */
HDC   ghDC; 
HGLRC ghRC; 
void swapbuffers32(); 
#define SWAPBUFFERS SwapBuffers(ghDC) 
#define BLACK_INDEX     0 
#define RED_INDEX       13 
#define GREEN_INDEX     14 
#define BLUE_INDEX      16 
#define WIDTH           300 
#define HEIGHT          200 
/*********************  Prototypes  ***********************/

LRESULT WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT WINAPI AboutDlgProc( HWND, UINT, WPARAM, LPARAM );

/*******************  Global Variables ********************/

HANDLE ghInstance;
BOOL bSetupPixelFormat(HDC); 
 
/* OpenGL globals, defines, and prototypes */ 
GLvoid resize(GLsizei, GLsizei); 
GLvoid initializeGL(GLsizei, GLsizei); 
GLvoid drawScene(GLvoid); 
int create_main_window_win32();

/*
void updateContext();
float getWidth();
float getHeight();
void  setAquaCursor(int ctype);
int aquaSetConsoleMessage(char* str);
void createAutoReleasePool();
*/
void setMenuButton_collision(int val);
void setMenuButton_texSize(int size);
void setMenuStatus(char* stat);
void setMenuButton_headlight(int val);
void setMenuFps(float fps);
void setMenuButton_navModes(int type);
int getEventsWin32(int*,int,int*,int*); 
int doEventsWin32A();


#endif /* __LIBFREEWRL_DISPLAY_WIN32_H__ */
