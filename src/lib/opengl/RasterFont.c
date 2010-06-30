/*
  $Id: RasterFont.c,v 1.10 2010/06/30 12:57:42 crc_canada Exp $

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

#include "../vrml_parser/Structs.h"
#include "opengl/RasterFont.h"
#include "opengl/OpenGL_Utils.h"

#include <stdarg.h>

#include "../main/headers.h"
#include "../input/EAIHelpers.h"
#include "../scenegraph/Component_Text.h"
static struct X3D_Text myText;
static struct X3D_FontStyle myFont;
struct Uni_String myString;


static bool rf_initialized = FALSE;

static int xf_color = xf_white;
static vec4f_t xf_colors[3] = { 
    { 1.0f, 1.0f, 1.0f, 1.0f }, 
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    { 0.5f, 0.5f, 0.5f, 1.0f } 
};


void rf_print(const char *text)
{
	/* has text changed? */
	myText.string.p[0]->touched = 0;
	verify_Uni_String (myText.string.p[0],(char *)text);
	if (myText.string.p[0]->touched > 0) {
		/* mark the FontStyle and Text node that things have changed */
		myText._change++;
		myFont._change++;
	}

	render_Text (&myText);
}


void rf_printf(int x, int y, const char *format, ...)
{
    va_list ap;
    char xfont_buffer[5000];

    if (!rf_initialized) {
	ERROR_MSG("xfont not initialized !!! initializing with defaults (fixed white)\n");
	if (!rf_xfont_init("fixed")) {
		return;
	}
	rf_xfont_set_color(xf_white);
    }
    va_start(ap, format);
    vsprintf(xfont_buffer, format, ap);
    va_end(ap);

    FW_GL_RASTERPOS2I(x, y);

    FW_GL_COLOR4FV(xf_colors[xf_color]);

    rf_print(xfont_buffer);
}

void rf_layer2D()
{
    FW_GL_PUSH_ATTRIB(GL_ENABLE_BIT);
    FW_GL_DISABLE(GL_DEPTH_TEST);
    FW_GL_DISABLE(GL_CULL_FACE);
    FW_GL_DISABLE(GL_LIGHTING);

    // On assume être en MODELVIEW
    FW_GL_MATRIX_MODE(GL_PROJECTION);
    FW_GL_PUSH_MATRIX();
    FW_GL_LOAD_IDENTITY();
    FW_GL_ORTHO(0.0, (GLfloat) screenWidth,  // we need a viewport variable: glc.viewport[2],
	    0.0, (GLfloat) screenHeight, // glc.viewport[3],
	    -1, 1);
    // Faire un FW_GL_POP_MATRIX après ...
    FW_GL_MATRIX_MODE(GL_MODELVIEW);
    FW_GL_PUSH_MATRIX();
    FW_GL_LOAD_IDENTITY();
    FW_GL_TRANSLATE_F(0.375f, 0.375f, 0.0f);
}

void rf_leave_layer2D()
{
    FW_GL_POP_ATTRIB();

    FW_GL_MATRIX_MODE(GL_PROJECTION);
    FW_GL_POP_MATRIX();
    FW_GL_MATRIX_MODE(GL_MODELVIEW);
    FW_GL_POP_MATRIX();
}

int rf_xfont_init(const char *fontname)
{
	/* create a new text node, but DO NOT call one of the createNewX3DNode interface, because we only
		want a holder here, this is NOT a scenegraph node. */

	/* write zeroes here - we do not want any pointers, parents, etc, etc. */
	bzero (&myText,sizeof (struct X3D_Text));

	myText.v = &virt_Text;
	myText.fontStyle = NULL;
	myText.solid = TRUE;
	myText.__rendersub = 0;
	myText.origin.c[0] = 0;myText.origin.c[1] = 0;myText.origin.c[2] = 0;;

	/* give this 1 string */
 myText.string.p = MALLOC (sizeof(struct Uni_String)*1);myText.string.p[0] = newASCIIString("Initial String for Status Line");myText.string.n=1; ;

	
	myText.textBounds.c[0] = 0;myText.textBounds.c[1] = 0;;
	myText.length.n=0; myText.length.p=0;
	myText.maxExtent = 0;
	myText.lineBounds.n=0; myText.lineBounds.p=0;
	myText.metadata = NULL;
	myText._defaultContainer = FIELDNAMES_geometry;

	/* create a new FontStyle node here and link it in */
	bzero (&myFont, sizeof (struct X3D_FontStyle));

	myFont._nodeType = NODE_FontStyle; /* needed for scenegraph structure in make_Text */
	myFont.v = &virt_FontStyle;
	myFont.language = newASCIIString("");
	myFont.leftToRight = TRUE;
	myFont.topToBottom = TRUE;
	myFont.style = newASCIIString("PLAIN");
	myFont.size = 20.0f;
	myFont.justify.p = MALLOC (sizeof(struct Uni_String)*1);myFont.justify.p[0] = newASCIIString("BEGIN");myFont.justify.n=1; ;
	myFont.metadata = NULL;
	myFont.spacing = 1;
	myFont.horizontal = TRUE;
	/* myFont.family.p = MALLOC (sizeof(struct Uni_String)*1);myFont.family.p[0] = newASCIIString("SERIF");myFont.family.n=1; */
	myFont.family.p = MALLOC (sizeof(struct Uni_String)*1);myFont.family.p[0] = newASCIIString("TYPEWRITER");myFont.family.n=1; ;
	myFont._defaultContainer = FIELDNAMES_fontStyle;

	myText.fontStyle = &myFont;
    rf_initialized = TRUE;
    return TRUE;
}

void rf_xfont_set_color(e_xfont_color_t index)
{
    ASSERT(index < e_xfont_color_max);
    xf_color = index;
}

void rf_xfont_set_usercolor(vec4f_t color)
{
    xf_colors[xf_user][0] = color[0];
    xf_colors[xf_user][1] = color[1];
    xf_colors[xf_user][2] = color[2];
    xf_colors[xf_user][3] = color[3];
}
