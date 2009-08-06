/*
=INSERT_TEMPLATE_HERE=

$Id: RenderFuncs.h,v 1.2 2009/08/06 20:10:11 crc_canada Exp $

Proximity sensor macro.

*/

#ifndef __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__
#define __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__

/* for X3D_Node */
#include "../vrml_parser/Structs.h"

/* function protos */
int nextlight(void);
void render_node(struct X3D_Node *node);
void initializeShapeCompileThread(void);

#endif /* __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__ */
