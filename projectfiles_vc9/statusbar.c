/*
  $Id: statusbar.c,v 1.2 2009/12/05 19:47:31 dug9 Exp $

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


#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>
#include "main/headers.h"
#include "vrml_parser/Structs.h"
#include "scenegraph/Viewer.h"


/* raster font drawing taken from redbook p.311 / fonts.c in www.opengl-redbook.com/code */

GLubyte space[] = 
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

GLubyte letters[][13] = {
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18}, 
{0x00, 0x00, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe}, 
{0x00, 0x00, 0x7e, 0xe7, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e}, 
{0x00, 0x00, 0xfc, 0xce, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xce, 0xfc}, 
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff}, 
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xff}, 
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xcf, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e}, 
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e}, 
{0x00, 0x00, 0x7c, 0xee, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, 
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc3}, 
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0}, 
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xff, 0xe7, 0xc3}, 
{0x00, 0x00, 0xc7, 0xc7, 0xcf, 0xcf, 0xdf, 0xdb, 0xfb, 0xf3, 0xf3, 0xe3, 0xe3}, 
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e}, 
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe}, 
{0x00, 0x00, 0x3f, 0x6e, 0xdf, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c}, 
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe}, 
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0xe0, 0xc0, 0xc0, 0xe7, 0x7e}, 
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff}, 
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
{0x00, 0x00, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3}, 
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3}, 
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x7e, 0x0c, 0x06, 0x03, 0x03, 0xff}
};

GLuint fontOffset;

void makeRasterFont(void)
{
   GLuint i, j;
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   fontOffset = glGenLists (128);
   for (i = 0,j = 'A'; i < 26; i++,j++) {
      glNewList(fontOffset + j, GL_COMPILE);
      glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, letters[i]);
      glEndList();
   }
   glNewList(fontOffset + ' ', GL_COMPILE);
   glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, space);
   glEndList();
}

static int fontInitialized = 0;
static int currentx, currenty;
void initFont(void)
{
   glShadeModel (GL_FLAT);
   makeRasterFont();
   fontInitialized = 1;
}

void printString(char *s)
{
   glPushAttrib (GL_LIST_BIT);
   glListBase(fontOffset);
   glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *) s);
   glPopAttrib ();
}

static int sb_hasString = FALSE;
static struct Uni_String *myline;

static char buffer[200];
void render_init(void);

#define STATUS_LEN 2000

/* make sure that on a re-load that we re-init */
void kill_status (void) {
	/* hopefully, by this time, rendering has been stopped */
	sb_hasString = FALSE;
}


/* trigger a update */
void update_status(char* msg) {

	if (msg==NULL) sb_hasString = FALSE;
	else {
		sb_hasString = TRUE;
		strcpy (buffer,msg);
	}
}


int setup_projection2() 
{
	GLsizei screenwidth2 = screenWidth;
	GLdouble aspect2 = screenRatio;
	GLint xvp = 0;
	if(Viewer.sidebyside) 
	{
		screenwidth2 = (int)((screenwidth2 * .5)+.5);
		aspect2 = aspect2 * .5;
		if(Viewer.iside == 1) xvp = (GLint)screenwidth2;
	}

	glViewport(xvp,0,screenwidth2,screenHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(xvp,xvp+screenwidth2, 0.0, screenHeight, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	return xvp;
}
void display(void)
{
	/* sample from redbook fonts.c , not used */
   GLfloat white[3] = { 1.0, 1.0, 1.0 };

   glClear(GL_COLOR_BUFFER_BIT);
   glColor3fv(white);

   glRasterPos2i(20, 60);
   printString("THE QUICK BROWN FOX JUMPS");
   glRasterPos2i(20, 40);
   printString("OVER A LAZY DOG");
   //glFlush ();
}
extern int currentX, currentY;
static int useStencil = 1;
static int followCursor = 0;
static int windowPos = 1;
void drawStatusBar() 
{
	char *p; 
	int xvp;
	float c[4];
	int ic[4];
	if (!sb_hasString) {
		if(useStencil) glDisable(GL_STENCIL_TEST);
		return;
	}
	if(!fontInitialized) initFont();
	xvp = 0;
	if(windowPos)
	{
		if( Viewer.iside == 1) xvp = screenWidth/2.0;
	}else{
		xvp = setup_projection2(); 
		FW_GL_LOAD_IDENTITY();
	}

	if(useStencil)
	{
		//you call drawStatusBar() with stencil before you start rendering, and the stencil bits won't be over-written
		glGetFloatv(GL_COLOR_CLEAR_VALUE,c);
		glClearColor (1.0f, 1.0f, 1.0f, 0.0f);
		glClearStencil(0);
		glEnable(GL_STENCIL_TEST);
		//glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_NEVER, 0x0, 0x0);
		glStencilOp(GL_INCR, GL_INCR, GL_INCR);
		glColor3f(1.0f, 1.0f, 1.0f);
	}else{
		// you must call drawStatusBar() from render() just before swapbuffers 
		glDepthMask(FALSE);
		glDisable(GL_DEPTH_TEST);
		glColor3f(1.0f,1.0f,1.0f);
	}
	//display();
	if(windowPos)
	{
		//glWindowPos seems to set the bitmap color correctly in windows
		if( followCursor)
			glWindowPos2i(currentX,screenHeight - currentY); 
		else
			glWindowPos2i(xvp+5,screenHeight -15);
	}else{
		//glRasterPos does not seem to set the bitmap color correctly.
		if( followCursor)
			glRasterPos2i(currentX,screenHeight - currentY); 
		else
			glRasterPos2i(xvp+5,screenHeight -15);
	}
	p = buffer;
	//glGetFloatv(GL_CURRENT_RASTER_COLOR,c);
	//printf("c=%f %f %f",c[0],c[1],c[2]);
	//glGetIntegerv(GL_CURRENT_RASTER_INDEX,ic);
	//printf("ic=%d ",ic[0]);
	//glColor3f(1.0,1.0,1.0);
	printString(p); 

	if(useStencil)
	{
		// Now, allow drawing, except where the stencil pattern is 0x1
		// and do not make any further changes to the stencil buffer
		glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		//glClearColor(c[0],c[1],c[2],c[3]); //, 1.0f, 1.0f, 0.0f);
	}else{
		glDepthMask(TRUE);
		glEnable(GL_DEPTH_TEST);
		glFlush();

	}
}
