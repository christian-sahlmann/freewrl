/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

/*****************************************

Bindable nodes - Background, TextureBackground, Fog, NavigationInfo, Viewpoint.

******************************************/


#include <math.h>
#include <stdint.h>

#ifdef AQUA
#include <gl.h>
#include <glu.h>
#include <glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#include "headers.h"
#include "Textures.h"

/* Bind stack */
#define MAX_STACK 20

extern GLint viewPort[];

/* Bindables, Viewpoint, NavigationInfo, Background, TextureBackground and Fog */
extern void * *fognodes;
extern void * *backgroundnodes;
extern void * *navnodes;
extern void * *viewpointnodes;
extern int totfognodes, totbacknodes, totnavnodes, totviewpointnodes;
extern int currboundvpno;

extern int viewpoint_tos;
extern int background_tos;
extern int fog_tos;
extern int navi_tos;

extern uintptr_t viewpoint_stack[];
extern uintptr_t navi_stack[];

void
reset_upvector(void);

void
set_naviinfo(struct X3D_NavigationInfo *node);

void
send_bind_to(struct X3D_Node *node, int value);

void
bind_node(struct X3D_Node *node, int *tos, uintptr_t *stack);

void
render_Fog(struct X3D_Fog *node);

void
render_NavigationInfo(struct X3D_NavigationInfo *node);

void render_Background(struct X3D_Background *node);
void render_TextureBackground(struct X3D_TextureBackground *node);


