/*
=INSERT_TEMPLATE_HERE=

$Id: OpenGL_Utils.h,v 1.3 2008/12/31 13:08:15 couannette Exp $

Screen snapshot.

*/

#ifndef __FREEX3D_OPENGL_UTILS_H__
#define __FREEX3D_OPENGL_UTILS_H__


void start_textureTransform (void *textureNode, int ttnum);
void end_textureTransform (void *textureNode, int ttnum);

void
glpOpenGLInitialize(void);


void
BackEndClearBuffer(void);

void
BackEndLightsOff(void);

void lightState (GLint light, int state);


extern void glpOpenGLInitialize(void);


#endif /* __FREEX3D_OPENGL_UTILS_H__ */
