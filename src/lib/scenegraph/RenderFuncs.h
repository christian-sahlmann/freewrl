/*
=INSERT_TEMPLATE_HERE=

$Id: RenderFuncs.h,v 1.1 2009/02/18 16:24:04 istakenv Exp $

Proximity sensor macro.

*/

#ifndef __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__
#define __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__

/* for X3D_Node */
#include "../vrml_parser/Structs.h"

/* function protos */
int nextlight(void);
void render_node(struct X3D_Node *node);

#endif /* __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__ */
