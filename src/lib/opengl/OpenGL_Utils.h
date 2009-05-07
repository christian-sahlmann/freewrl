/*
=INSERT_TEMPLATE_HERE=

$Id: OpenGL_Utils.h,v 1.5 2009/05/07 18:43:34 crc_canada Exp $

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

extern void glPrintError(char *);

#endif /* __FREEWRL_OPENGL_UTILS_H__ */
