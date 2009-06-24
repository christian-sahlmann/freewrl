/*
=INSERT_TEMPLATE_HERE=

$Id: OpenGL_Utils.h,v 1.6 2009/06/24 13:03:53 crc_canada Exp $

Screen snapshot.

*/

#ifndef __FREEWRL_OPENGL_UTILS_H__
#define __FREEWRL_OPENGL_UTILS_H__


void start_textureTransform (struct X3D_Node *textureNode, int ttnum);
void end_textureTransform (void);

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
