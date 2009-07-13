/*
=INSERT_TEMPLATE_HERE=

$Id: Bindable.h,v 1.6 2009/07/13 18:49:50 crc_canada Exp $

Bindable nodes - Background, TextureBackground, Fog, NavigationInfo, Viewpoint.

*/

#ifndef __FREEWRL_BINDABLE_H__
#define __FREEWRL_BINDABLE_H__


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
extern uintptr_t background_stack[];
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

void set_naviWidthHeightStep(double wid, double hei, double step) ;

#endif /* __FREEWRL_BINDABLE_H__ */
