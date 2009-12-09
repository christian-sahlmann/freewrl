/*
  $Id: RasterFont.c,v 1.4 2009/12/09 14:34:27 crc_canada Exp $

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

#include "opengl/RasterFont.h"

#include <stdarg.h>

#include "../input/EAIHelpers.h"
#include "../scenegraph/Component_Text.h"
static struct X3D_Text myText;
static struct X3D_FontStyle myFont;
struct Uni_String myString;


static bool rf_initialized = FALSE;

static int xf_color = xf_white;
static vec4f_t xf_colors[3] = { 
    { 1.0, 1.0, 1.0, 1.0 }, 
    { 0.0, 0.0, 0.0, 1.0 }, 
    { 0.5, 0.5, 0.5, 1.0 } 
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

    glRasterPos2i(x, y);

    glColor4fv(xf_colors[xf_color]);

    rf_print(xfont_buffer);
}

void rf_layer2D()
{
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);

    // On assume être en MODELVIEW
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (GLfloat) screenWidth,  // we need a viewport variable: glc.viewport[2],
	    0.0, (GLfloat) screenHeight, // glc.viewport[3],
	    -1, 1);
    // Faire un glPopMatrix après ...
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.375, 0.375, 0.);
}

void rf_leave_layer2D()
{
    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static int rf_xfont_init(const char *fontname)
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
	myText.__oldmetadata = 0;
	myText._defaultContainer = FIELDNAMES_geometry;

	/* create a new FontStyle node here and link it in */
	bzero (&myFont, sizeof (struct X3D_FontStyle));

	myFont._nodeType = NODE_FontStyle; /* needed for scenegraph structure in make_Text */
	myFont.v = &virt_FontStyle;
	myFont.language = newASCIIString("");
	myFont.leftToRight = TRUE;
	myFont.topToBottom = TRUE;
	myFont.style = newASCIIString("PLAIN");
	myFont.size = 20.0;
	myFont.justify.p = MALLOC (sizeof(struct Uni_String)*1);myFont.justify.p[0] = newASCIIString("BEGIN");myFont.justify.n=1; ;
	myFont.metadata = NULL;
	myFont.spacing = 1;
	myFont.__oldmetadata = 0;
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

static void rf_xfont_set_usercolor(vec4f_t color)
{
    xf_colors[xf_user][0] = color[0];
    xf_colors[xf_user][1] = color[1];
    xf_colors[xf_user][2] = color[2];
    xf_colors[xf_user][3] = color[3];
}
