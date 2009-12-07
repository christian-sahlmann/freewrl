/*
  $Id: RasterFont.c,v 1.1 2009/12/07 23:23:21 couannette Exp $

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

#include "RasterFont.h"

#include <stdarg.h>


static bool rf_initialized = FALSE;
static char xfont_buffer[5000];
unsigned int xfont_list_base = 0;

static int xf_color = xf_white;
static vec4f_t xf_colors[3] = { 
    { 1.0, 1.0, 1.0, 1.0 }, 
    { 0.0, 0.0, 0.0, 1.0 }, 
    { 0.5, 0.5, 0.5, 1.0 } 
};


void rf_print(const char *text)
{
    ASSERT(text);
    if (strlen(text) > 0) {
	glPushAttrib(GL_LIST_BIT);
	glListBase(xfont_list_base);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, (GLubyte *) text);
	glPopAttrib();
    }
}

void rf_printf(int x, int y, const char *format, ...)
{
    if (!rf_initialized) {
	ERROR_MSG("xfont not initialized !!! initializing with defaults (fixed white)\n");
	if (!rf_xfont_init("fixed")) {
		return;
	}
	rf_xfont_set_color(xf_white);
    }
    va_list ap;
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

int rf_xfont_init(const char *fontname)
{
    int ret;
    Font xfont;

    if (!fontname) {
	    fontname = "-*-fixed-medium-r-*-*-*-*-*-*-*-*-*-*";
    }
    xfont_list_base = glGenLists(256); // I used to have this stored internally... dri_next_list_range_available(256);

    TRACE_MSG("Loading XFont %s\n", fontname);
    xfont = XLoadFont(Xdpy, fontname);
    if (xfont == BadAlloc || xfont == BadName) {
	    ERROR_MSG("rf_xfont_init: XLoadFont error");
	return FALSE;
    }

    glXUseXFont(xfont, 0, 256, xfont_list_base);

    ret = XUnloadFont(Xdpy, xfont);
    if (ret == BadFont) {
	// should not happen
	glDeleteLists(xfont_list_base, 256);
	ERROR_MSG("rf_xfont_init: XUnloadFont error");
    }
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

#if 0
void rf_mvar_print(int x, int y, s_mvar_t *mvar)
{
    static char buffer[4096];
    ASSERTE(mvar);
    mvar_dump_str(mvar, buffer, 4096);
    rf_printf(x, y, "%s = %s", mvar->name, buffer);

}
#endif
