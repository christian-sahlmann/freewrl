/*
=INSERT_TEMPLATE_HERE=

$Id: OpenGL_Utils.h,v 1.7 2009/07/14 15:36:01 uid31638 Exp $

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
void drawBBOX(struct X3D_Node *node);
#endif /* __FREEWRL_OPENGL_UTILS_H__ */
