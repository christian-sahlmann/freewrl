/*
=INSERT_TEMPLATE_HERE=

$Id: Material.h,v 1.1 2009/07/24 19:46:19 crc_canada Exp $

Global includes.

*/

#ifndef __FREEWRL_MATERIAL_H__
#define __FREEWRL_MATERIAL_H__


#define DEFAULT_COLOUR_POINTER  \
        GLfloat defColor[] = {1.0, 1.0, 1.0};\
        GLfloat *thisColor;

#define GET_COLOUR_POINTER \
		/* is there an emissiveColor here??? */ \
		if (lightingOn) { \
			thisColor = last_emission; \
		} else { \
			thisColor = defColor; \
		}

#define DO_COLOUR_POINTER                glColor3fv (thisColor);


extern GLfloat last_emission[4];


void do_shininess (GLenum face, float shininess);
void do_glMaterialfv (GLenum face, GLenum pname, GLfloat *param);
int verify_rotate(GLfloat *params);
int verify_translate(GLfloat *params);
int verify_scale(GLfloat *params);

#endif /* __FREEWRL_MATERIAL_H__ */
