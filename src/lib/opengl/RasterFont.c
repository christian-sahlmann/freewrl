/*
  $Id: RasterFont.c,v 1.23 2012/07/31 20:04:51 crc_canada Exp $

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
//static struct X3D_Text myText;
//static struct X3D_FontStyle myFont;
////struct Uni_String myString;
//
//
//static bool rf_initialized = FALSE;
//
//static int xf_color = xf_white;
//static vec4f_t xf_colors[3] = { 
//    { 1.0f, 1.0f, 1.0f, 1.0f }, 
//    { 0.0f, 0.0f, 0.0f, 1.0f }, 
//    { 0.5f, 0.5f, 0.5f, 1.0f } 
//};
static vec4f_t static_xf_colors[] = { 
		{ 1.0f, 1.0f, 1.0f, 1.0f }, 
		{ 0.0f, 0.0f, 0.0f, 1.0f }, 
		{ 0.5f, 0.5f, 0.5f, 1.0f }  
	};
typedef struct pRasterFont{
	struct X3D_Text myText;
	struct X3D_FontStyle myFont;
	//struct Uni_String myString;
	bool rf_initialized;//= FALSE;
	int xf_color;// = xf_white;
	vec4f_t xf_colors[3]; /* = { 
		{ 1.0f, 1.0f, 1.0f, 1.0f }, 
		{ 0.0f, 0.0f, 0.0f, 1.0f }, 
		{ 0.5f, 0.5f, 0.5f, 1.0f }  
	};*/

}* ppRasterFont;
void *RasterFont_constructor(){
	void *v = malloc(sizeof(struct pRasterFont));
	memset(v,0,sizeof(struct pRasterFont));
	return v;
}
void RasterFont_init(struct tRasterFont *t){
	//public
	//private
	t->prv = RasterFont_constructor();
	{
		ppRasterFont p = (ppRasterFont)t->prv;
		//p->myText;
		//p->myFont;
		//p->myString;


		p->rf_initialized = FALSE;

		p->xf_color = xf_white;
		memcpy(p->xf_colors,static_xf_colors,sizeof(static_xf_colors)); 
		//{ 
		//	{ 1.0f, 1.0f, 1.0f, 1.0f }, 
		//	{ 0.0f, 0.0f, 0.0f, 1.0f }, 
		//	{ 0.5f, 0.5f, 0.5f, 1.0f } 
		//};

	}
}

void rf_print(const char *text)
{
	ppRasterFont p = (ppRasterFont)gglobal()->RasterFont.prv;
	/* has text changed? */
	p->myText.string.p[0]->touched = 0;
	verify_Uni_String (p->myText.string.p[0],(char *)text);
	if (p->myText.string.p[0]->touched > 0) {
		/* mark the FontStyle and Text node that things have changed */
		p->myText._change++;
		p->myFont._change++;
	}

	render_Text (&p->myText);
}


void rf_printf(int x, int y, const char *format, ...)
{
#if defined(IPHONE) || defined(_ANDROID ) || defined(GLES2)
//printf ("skipping the rf_printf\n");
#else
    va_list ap;
    char xfont_buffer[5000];
	ppRasterFont p = (ppRasterFont)gglobal()->RasterFont.prv;

    if (!p->rf_initialized) {
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

#ifdef HAVE_TO_REIMPLEMENT
 //JAS - this is one case where the new full-time shaders do not work.
    FW_GL_COLOR4FV(p->xf_colors[p->xf_color]);
#endif

    rf_print(xfont_buffer);
#endif
}

void rf_layer2D()
{

#ifdef GL_ES_VERSION_2_0
//printf ("skipping the push attrib\n");
#else
    FW_GL_PUSH_ATTRIB(GL_ENABLE_BIT);
    FW_GL_DISABLE(GL_LIGHTING);
#endif
    FW_GL_DISABLE(GL_DEPTH_TEST);
    FW_GL_DISABLE(GL_CULL_FACE);


    // On assume être en MODELVIEW
    FW_GL_MATRIX_MODE(GL_PROJECTION);
    FW_GL_PUSH_MATRIX();
    FW_GL_LOAD_IDENTITY();

    FW_GL_ORTHO(0.0, (GLfloat) gglobal()->display.screenWidth,  // we need a viewport variable: glc.viewport[2],
	    0.0, (GLfloat) gglobal()->display.screenHeight, // glc.viewport[3],
	    -1, 1);
    // Faire un FW_GL_POP_MATRIX après ...
    FW_GL_MATRIX_MODE(GL_MODELVIEW);
    FW_GL_PUSH_MATRIX();
    FW_GL_LOAD_IDENTITY();
    FW_GL_TRANSLATE_F(0.375f, 0.375f, 0.0f);
}

void rf_leave_layer2D()
{
#ifdef GL_ES_VERSION_2_0
//printf ("skipping the popattribhte\n");
#else
    FW_GL_POP_ATTRIB();
#endif

    FW_GL_MATRIX_MODE(GL_PROJECTION);
    FW_GL_POP_MATRIX();
    FW_GL_MATRIX_MODE(GL_MODELVIEW);
    FW_GL_POP_MATRIX();
}

int rf_xfont_init(const char *fontname)
{
	/* create a new text node, but DO NOT call one of the createNewX3DNode interface, because we only
		want a holder here, this is NOT a scenegraph node. */
	ppRasterFont p = (ppRasterFont)gglobal()->RasterFont.prv;

	/* write zeroes here - we do not want any pointers, parents, etc, etc. */
	bzero (&p->myText,sizeof (struct X3D_Text));

	p->myText._nodeType=NODE_Text;
	p->myText.fontStyle = NULL;
	p->myText.solid = TRUE;
	p->myText.__rendersub = 0;
	p->myText.origin.c[0] = 0;p->myText.origin.c[1] = 0;p->myText.origin.c[2] = 0;;

	/* give this 1 string */
 p->myText.string.p = MALLOC (struct Uni_String **, sizeof(struct Uni_String)*1);p->myText.string.p[0] = newASCIIString("Initial String for Status Line");p->myText.string.n=1; ;

	
	p->myText.textBounds.c[0] = 0;p->myText.textBounds.c[1] = 0;;
	p->myText.length.n=0; p->myText.length.p=0;
	p->myText.maxExtent = 0;
	p->myText.lineBounds.n=0; p->myText.lineBounds.p=0;
	p->myText.metadata = NULL;
	p->myText._defaultContainer = FIELDNAMES_geometry;

	/* create a new FontStyle node here and link it in */
	bzero (&p->myFont, sizeof (struct X3D_FontStyle));

	p->myFont._nodeType = NODE_FontStyle; /* needed for scenegraph structure in make_Text */
	p->myFont.language = newASCIIString("");
	p->myFont.leftToRight = TRUE;
	p->myFont.topToBottom = TRUE;
	p->myFont.style = newASCIIString("PLAIN");
	p->myFont.size = 20.0f;
	p->myFont.justify.p = MALLOC (struct Uni_String **, sizeof(struct Uni_String)*1);p->myFont.justify.p[0] = newASCIIString("BEGIN");p->myFont.justify.n=1; ;
	p->myFont.metadata = NULL;
	p->myFont.spacing = 1;
	p->myFont.horizontal = TRUE;
	/* p->myFont.family.p = MALLOC (struct Uni_String **, sizeof(struct Uni_String)*1);p->myFont.family.p[0] = newASCIIString("SERIF");p->myFont.family.n=1; */
	p->myFont.family.p = MALLOC (struct Uni_String **, sizeof(struct Uni_String)*1);p->myFont.family.p[0] = newASCIIString("TYPEWRITER");p->myFont.family.n=1; ;
	p->myFont._defaultContainer = FIELDNAMES_fontStyle;

	p->myText.fontStyle = X3D_NODE(&p->myFont);
    p->rf_initialized = TRUE;
    return TRUE;
}

void rf_xfont_set_color(e_xfont_color_t index)
{
	ppRasterFont p = (ppRasterFont)gglobal()->RasterFont.prv;

    ASSERT(index < e_xfont_color_max);
    p->xf_color = index;
}

void rf_xfont_set_usercolor(vec4f_t color)
{
	ppRasterFont p = (ppRasterFont)gglobal()->RasterFont.prv;

    p->xf_colors[xf_user][0] = color[0];
    p->xf_colors[xf_user][1] = color[1];
    p->xf_colors[xf_user][2] = color[2];
    p->xf_colors[xf_user][3] = color[3];
}
