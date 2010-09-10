/*
=INSERT_TEMPLATE_HERE=

$Id: OpenGL_Utils.h,v 1.25 2010/09/10 17:36:54 crc_canada Exp $

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


void start_textureTransform (struct X3D_Node *textureNode, int ttnum);
void end_textureTransform (void);
void markForDispose(struct X3D_Node *node, int recursive);

void
BackEndClearBuffer(int);

void
BackEndLightsOff(void);

void drawBBOX(struct X3D_Node *node);


void fw_glMatrixMode(GLint mode);
void fw_glLoadIdentity(void);
void fw_glPushMatrix(void);
void fw_glPopMatrix(void);
void fw_glTranslated(double a, double b, double c);
void fw_glTranslatef(float a, float b, float c);
void fw_glRotateRad (double a, double b, double c, double d);
void fw_glRotated (double a, double b, double c, double d);
void fw_glRotatef (float a, float b, float c, float d);
void fw_glScaled (double a, double b, double c);
void fw_glScalef (float a, float b, float c);
void fw_glGetDoublev (int ty, double *mat, char *fn, int ln);
void old_glGetDoublev (int ty, double *mat, char *fn, int ln);

/* OpenGL-ES specifics for Materials and Vertices */
void fw_iphone_enableClientState(GLenum aaa);
void fw_iphone_disableClientState(GLenum aaa);
void fw_iphone_vertexPointer(GLint aaa,GLenum bbb,GLsizei ccc,const GLvoid *ddd);
void fw_iphone_normalPointer(GLenum aaa,GLsizei bbb, const GLvoid *ccc);
void fw_iphone_texcoordPointer(GLint aaa, GLenum bbb ,GLsizei ccc,const GLvoid *ddd);
void fw_iphone_colorPointer(GLint aaa, GLenum bbb,GLsizei ccc,const GLvoid *ddd);
void sendMatriciesToShader(GLint MM,GLint PM);
void fw_gluPerspective(GLDOUBLE fovy, GLDOUBLE aspect, GLDOUBLE zNear, GLDOUBLE zFar);
void fw_gluPickMatrix(GLDOUBLE xx, GLDOUBLE yy, GLDOUBLE width, GLDOUBLE height, GLint *vp);
void fw_Frustum(double left, double right, double bottom, double top, double nearZ, double farZ);
void fw_Ortho(double left, double right, double bottom, double top, double nearZ, double farZ);
void fw_gluProject( GLdouble objX,
                         GLdouble objY,
                         GLdouble objZ,
                         const GLdouble *model,
                         const GLdouble *proj,
                         const GLint *view,
                         GLdouble* winX,
                         GLdouble* winY,
                         GLdouble* winZ );
void fw_gluUnProject( GLdouble winX,
                           GLdouble winY,
                           GLdouble winZ,
                           const GLdouble *model,
                           const GLdouble *proj,
                           const GLint *view,
                           GLdouble* objX,
                           GLdouble* objY,
                           GLdouble* objZ );

void fw_gluPickMatrix(GLdouble x, GLdouble y, GLdouble deltax, GLdouble deltay,
                  GLint viewport[4]);


#endif /* __FREEWRL_OPENGL_UTILS_H__ */
