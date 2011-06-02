/*
  $Id: statusbar.c,v 1.33 2011/06/02 19:50:49 dug9 Exp $

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
#include "../opengl/OpenGL_Utils.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"

#include <float.h>

#include "../x3d_parser/Bindable.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"

#include "../opengl/RasterFont.h"

#define DJ_KEEP_COMPILER_WARNING 0

#define MAX_BUFFER_SIZE 4096
static char buffer[MAX_BUFFER_SIZE] = "\0";
void render_init(void);

#if DJ_KEEP_COMPILER_WARNING
#define STATUS_LEN 2000
#endif

/* make sure that on a re-load that we re-init */
void kill_status (void) {
	/* hopefully, by this time, rendering has been stopped */
}


/* trigger a update */
void update_status(char* msg)
{
	if (!msg) {
		buffer[0] = '\0';
	} else {
		strncpy(buffer, msg, MAX_BUFFER_SIZE);
	}
}
void hudSetConsoleMessage(char *buffer){}
void handleButtonOver(){}
void handleOptionPress(){}
void handleButtonPress(){}

void setMenuButton_collision(int val){}
void setMenuButton_texSize(int size){}
void setMenuButton_headlight(int val){}
void setMenuButton_navModes(int type){}
void setMenuStatus(char *stat) {}
void setMenuFps (float fps) {}

int handleStatusbarHud(int mev, int* clipplane)
{ return 0; }
void setup_projection(int pick, int x, int y) 
{
	ttglobal tg = gglobal();
	GLsizei screenwidth2 = tg->display.screenWidth;
	GLDOUBLE aspect2 = tg->display.screenRatio;
	GLint xvp = 0;
	if(Viewer.sidebyside) 
	{
		screenwidth2 = (int)((screenwidth2 * .5)+.5);
		aspect2 = aspect2 * .5;
		if(Viewer.iside == 1) xvp = (GLint)screenwidth2;
	}

        FW_GL_MATRIX_MODE(GL_PROJECTION);
	FW_GL_VIEWPORT(0,0,screenwidth2,tg->display.screenHeight);
        FW_GL_LOAD_IDENTITY();

        /* bounds check */
        if ((Viewer.fieldofview <= 0.0) || (Viewer.fieldofview > 180.0)) Viewer.fieldofview=45.0;
        /* glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);  */
        FW_GLU_PERSPECTIVE(Viewer.fieldofview, aspect2, Viewer.nearPlane, Viewer.farPlane); 

        FW_GL_MATRIX_MODE(GL_MODELVIEW);
        PRINT_GL_ERROR_IF_ANY("XEvents::setup_projection");
}

/**
 *   drawStatusBar: update the status text on top of the 3D view
 *                  using a 2D projection and raster characters.
 */
void drawStatusBar()
{
#if !(defined(IPHONE) || defined(_ANDROID))
	ttglobal tg = gglobal();
	/* dont do this if we can not display; note that when we start, we can send along
	   invalid data to the OpenGL drivers when doing ortho calcs */
	if ((tg->display.screenWidth > 5) && (tg->display.screenHeight > 5)) {
		/* update fps message (maybe extend this with other "text widgets" */

		rf_xfont_set_color(xf_white);
		rf_layer2D();

		rf_printf(15, 15, buffer);
		rf_leave_layer2D();
	}
#endif
}
