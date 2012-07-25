/*
=INSERT_TEMPLATE_HERE=

$Id: OpenGL_Utils.h,v 1.52 2012/07/25 18:45:27 crc_canada Exp $

Screen snapshot.

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


#ifndef __FREEWRL_OPENGL_UTILS_H__
#define __FREEWRL_OPENGL_UTILS_H__

typedef enum vertexShaderResources {
       	vertexPrecisionDeclare,
    vertexLightDefines,
    vertexPhongDeclare,
    vertexNormalDeclare,
	vertexPositionDeclare,
	vertexSimpleColourDeclare,
    vertexOneMaterialDeclare,
    vertexBackMaterialDeclare,
    vertFrontColourDeclare,
    vertexLightingEquation,
	vertexMainStart,
	vertexPosition,
    vertexOneMaterialCalculation,
    vertexPhongCalculation,
	vertexSimpleColour,
	vertexMainEnd,
	vertexEndMarker
} vertexShaderResources_t;

typedef enum fragmenShaderResources {
	fragmentPrecisionDeclare,
    fragmentLightDefines,
    fragmentNormalColorDefs,
	fragmentSimpleColourDeclare,
    fragmentOneColourDeclare,
    fragmentBackColourDeclare,
    fragmentPhongNormPosDeclare,
    fragmentADSLLightModel,
	fragmentMainStart,
	fragmentSimpleColourAssign,
    fragmentOneColourAssign,
    fragmentADSLAssign,
	fragmentMainEnd,
	fragmentEndMarker
} fragmentShaderResources_t;


/* Ian moved this from iglobal.h so that OpenGL_Utils.c could use it
 * since OpenGL_Utils.c cannot #include <iglobal.h> due to recursion */
struct multiTexParams {
    int multitex_mode;
    int multitex_source;
    int multitex_function;
};


void do_textureTransform (struct X3D_Node *textureNode, int ttnum);
void markForDispose(struct X3D_Node *node, int recursive);

s_shader_capabilities_t *getMyShader(unsigned int);

void sendMatriciesToShader(s_shader_capabilities_t *me);
void sendMaterialsToShader(s_shader_capabilities_t *me);

void
BackEndClearBuffer(int);

void
BackEndLightsOff(void);

void drawBBOX(struct X3D_Node *node);


void fw_glMatrixMode(GLint mode);
void fw_glLoadIdentity(void);
void fw_glPushMatrix(void);
void fw_glPopMatrix(void);
void fw_glTranslated(GLDOUBLE a, GLDOUBLE b, GLDOUBLE c);
void fw_glTranslatef(float a, float b, float c);
void fw_glRotateRad (GLDOUBLE a, GLDOUBLE b, GLDOUBLE c, GLDOUBLE d);
void fw_glRotated (GLDOUBLE a, GLDOUBLE b, GLDOUBLE c, GLDOUBLE d);
void fw_glRotatef (float a, float b, float c, float d);
void fw_glScaled (GLDOUBLE a, GLDOUBLE b, GLDOUBLE c);
void fw_glScalef (float a, float b, float c);
void fw_glGetDoublev (int ty, GLDOUBLE *mat);

/* OpenGL-ES specifics for Materials and Vertices */
void fw_iphone_enableClientState(GLenum aaa);
void fw_iphone_disableClientState(GLenum aaa);
void fw_iphone_vertexPointer(GLint aaa,GLenum bbb,GLsizei ccc,const GLvoid *ddd);
void fw_iphone_normalPointer(GLenum aaa,GLsizei bbb, const GLvoid *ccc);
void fw_iphone_texcoordPointer(GLint aaa, GLenum bbb ,GLsizei ccc,const GLvoid *ddd);
void fw_iphone_colorPointer(GLint aaa, GLenum bbb,GLsizei ccc,const GLvoid *ddd);
void fw_gluPerspective(GLDOUBLE fovy, GLDOUBLE aspect, GLDOUBLE zNear, GLDOUBLE zFar);
void fw_gluPickMatrix(GLDOUBLE xx, GLDOUBLE yy, GLDOUBLE width, GLDOUBLE height, GLint *vp);
void fw_Frustum(GLDOUBLE left, GLDOUBLE right, GLDOUBLE bottom, GLDOUBLE top, GLDOUBLE nearZ, GLDOUBLE farZ);
void fw_Ortho(GLDOUBLE left, GLDOUBLE right, GLDOUBLE bottom, GLDOUBLE top, GLDOUBLE nearZ, GLDOUBLE farZ);
void fw_gluProject( GLDOUBLE objX,
                         GLDOUBLE objY,
                         GLDOUBLE objZ,
                         const GLDOUBLE *model,
                         const GLDOUBLE *proj,
                         const GLint *view,
                         GLDOUBLE* winX,
                         GLDOUBLE* winY,
                         GLDOUBLE* winZ );
void fw_gluUnProject( GLDOUBLE winX,
                           GLDOUBLE winY,
                           GLDOUBLE winZ,
                           const GLDOUBLE *model,
                           const GLDOUBLE *proj,
                           const GLint *view,
                           GLDOUBLE* objX,
                           GLDOUBLE* objY,
                           GLDOUBLE* objZ );

void fw_gluPickMatrix(GLDOUBLE x, GLDOUBLE y, GLDOUBLE deltax, GLDOUBLE deltay,
                  GLint viewport[4]);

#endif /* __FREEWRL_OPENGL_UTILS_H__ */
