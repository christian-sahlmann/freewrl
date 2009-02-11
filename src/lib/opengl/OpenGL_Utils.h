/*
=INSERT_TEMPLATE_HERE=

$Id: OpenGL_Utils.h,v 1.4 2009/02/11 15:12:54 istakenv Exp $

Screen snapshot.

*/

#ifndef __FREEWRL_OPENGL_UTILS_H__
#define __FREEWRL_OPENGL_UTILS_H__


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


#endif /* __FREEWRL_OPENGL_UTILS_H__ */
