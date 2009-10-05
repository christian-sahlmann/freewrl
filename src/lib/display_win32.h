/*
=INSERT_TEMPLATE_HERE=

$Id: display_win32.h,v 1.5 2009/10/05 15:07:23 crc_canada Exp $

FreeWRL support library.
Internal header: display (X11/Motif or OSX/Aqua) dependencies.

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
#define SENSOR_CURSOR sensor_cursor32();
#define ARROW_CURSOR arrow_cursor32();


#endif /* __LIBFREEWRL_DISPLAY_WIN32_H__ */
