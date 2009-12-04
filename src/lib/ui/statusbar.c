/*
  $Id: statusbar.c,v 1.16 2009/12/04 21:11:00 crc_canada Exp $

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
#include <list.h>
#include <io_files.h>
#include <resources.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"

#include <float.h>

#include "../x3d_parser/Bindable.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"

#include "glut.h"

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

void setup_projection(int pick, int x, int y) 
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

        FW_GL_MATRIX_MODE(GL_PROJECTION);
	glViewport(0,0,screenwidth2,screenHeight);
        FW_GL_LOAD_IDENTITY();

        /* bounds check */
        if ((fieldofview <= 0.0) || (fieldofview > 180.0)) fieldofview=45.0;
        /* glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);  */
        gluPerspective(fieldofview, aspect2, nearPlane, farPlane); 

        FW_GL_MATRIX_MODE(GL_MODELVIEW);

        PRINT_GL_ERROR_IF_ANY("XEvents::setup_projection");

}
void drawStatusBar() {

char *p; 

if (!sb_hasString) {
	glDisable(GL_STENCIL_TEST);
	return;
}

setup_projection(FALSE,screenWidth,screenHeight);
        FW_GL_MATRIX_MODE(GL_MODELVIEW); /*  this should be assumed , here for safety.*/
        FW_GL_LOAD_IDENTITY();

glClearColor (0.0f, 0.0f, 1.0f, 0.0f);
glClearStencil(0);
glEnable(GL_STENCIL_TEST);
glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
glStencilFunc(GL_NEVER, 0x0, 0x0);
glStencilOp(GL_INCR, GL_INCR, GL_INCR);
glColor3f(1.0f, 1.0f, 1.0f);


glTranslatef(0,0,-nearPlane*2.0);
glTranslatef (-2,0,-5);
glScalef (0.005, 0.005, 0.0);
  for (p = buffer; *p; p++)
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *p);


           
    // Now, allow drawing, except where the stencil pattern is 0x1
    // and do not make any further changes to the stencil buffer
    glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}
