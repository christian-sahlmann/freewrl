/*
=INSERT_TEMPLATE_HERE=

$Id: Bindable.c,v 1.59 2011/06/01 15:02:21 crc_canada Exp $

Bindable nodes - Background, TextureBackground, Fog, NavigationInfo, Viewpoint, GeoViewpoint.

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



#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CRoutes.h"
#include "../opengl/OpenGL_Utils.h"
#include "Bindable.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../scenegraph/Component_Geospatial.h"
#include "../scenegraph/RenderFuncs.h"
#include "../scenegraph/Component_ProgrammableShaders.h"

/* for Background spheres */
struct MyVertex
 {
   struct SFVec3f vert;    //Vertex
   struct SFColorRGBA col;     //Colour
 };


/* Viewport data */
GLint viewPort[10];

int background_tos = -1;
int fog_tos = -1;
int navi_tos = -1;
int viewpoint_tos = -1;
uintptr_t background_stack[MAX_STACK];
uintptr_t fog_stack[MAX_STACK];
uintptr_t viewpoint_stack[MAX_STACK];
uintptr_t navi_stack[MAX_STACK];

#ifndef GL_ES_VERSION_2_0
/* Background - fog nodes do not affect the background node rendering. */
static int fog_enabled = FALSE;
#endif


static void saveBGVert (float *colptr, float *pt, int *vertexno, float *col, double dist, double x, double y, double z) ;

/* common entry routine for setting avatar size */
void set_naviWidthHeightStep(double wid, double hei, double step) {

	naviinfo.width = wid;
	naviinfo.height = hei;
	naviinfo.step = step;

	/* printf ("set_naviWdithHeightStep - width %lf height %lf step %lf speed %lf\n",wid,hei,step,Viewer.speed); */

}

/* called when binding NavigationInfo nodes */
void set_naviinfo(struct X3D_NavigationInfo *node) {
	struct Uni_String **svptr;
	int i;
	char *typeptr;

        Viewer.speed = (double) node->speed;
	if (node->avatarSize.n<2) {
		printf ("set_naviinfo, avatarSize smaller than expected\n");
	} else {
		set_naviWidthHeightStep ((double)(node->avatarSize.p[0]),
			(double)(node->avatarSize.p[1]),
			(double)((node->avatarSize.p[2]))); //dug9 Jan 6, 2010 - this is too crazy for the new gravity. * node->speed) * 2));
	}

	/* keep track of valid Navigation types. */
	svptr = node->type.p;

	/* assume "NONE" is set */
	for (i=0; i<7; i++) Viewer.oktypes[i] = FALSE;


	/* now, find the ones that are ok */
	for (i = 0; i < node->type.n; i++) {
		/*  get the string pointer */
		typeptr = svptr[i]->strptr;

		if (strcmp(typeptr,"WALK") == 0) {
			Viewer.oktypes[VIEWER_WALK] = TRUE;
			if (i==0) fwl_set_viewer_type(VIEWER_WALK);
		}
		if (strcmp(typeptr,"FLY") == 0) {
			Viewer.oktypes[VIEWER_FLY] = TRUE;
			if (i==0) fwl_set_viewer_type(VIEWER_FLY);
		}
		if (strcmp(typeptr,"EXAMINE") == 0) {
			Viewer.oktypes[VIEWER_EXAMINE] = TRUE;
			if (i==0) fwl_set_viewer_type(VIEWER_EXAMINE);
		}
		if (strcmp(typeptr,"NONE") == 0) {
			Viewer.oktypes[VIEWER_NONE] = TRUE;
			if (i==0) fwl_set_viewer_type(VIEWER_NONE);
		}
		if (strcmp(typeptr,"EXFLY") == 0) {
			Viewer.oktypes[VIEWER_EXFLY] = TRUE;
			if (i==0) fwl_set_viewer_type(VIEWER_EXFLY);
		}
		if (strcmp(typeptr,"YAWPITCHZOOM") == 0) {
			Viewer.oktypes[VIEWER_YAWPITCHZOOM] = TRUE;
			if (i==0) fwl_set_viewer_type(VIEWER_YAWPITCHZOOM);
		}
		if (strcmp(typeptr,"ANY") == 0) {
			Viewer.oktypes[VIEWER_EXAMINE] = TRUE;
			Viewer.oktypes[VIEWER_WALK] = TRUE;
			Viewer.oktypes[VIEWER_EXFLY] = TRUE;
			Viewer.oktypes[VIEWER_FLY] = TRUE;
			if (i==0) fwl_set_viewer_type (VIEWER_WALK); /*  just choose one */
		}
	}
        Viewer.headlight = node->headlight;
	/* tell the menu buttons of the state of this headlight */
	setMenuButton_headlight(node->headlight);

	/* transition effects */
	Viewer.transitionTime = node->transitionTime;
	/* bounds checking */
	if (Viewer.transitionTime < 0.0) Viewer.transitionTime = 0.0;

	Viewer.transitionType = VIEWER_TRANSITION_LINEAR; /* assume LINEAR */
	if (node->transitionType.n > 0) {
		if (strcmp("LINEAR", node->transitionType.p[0]->strptr) == 0) Viewer.transitionType = VIEWER_TRANSITION_LINEAR;
		else if (strcmp("TELEPORT", node->transitionType.p[0]->strptr) == 0) Viewer.transitionType = VIEWER_TRANSITION_TELEPORT;
		else if (strcmp("ANIMATE", node->transitionType.p[0]->strptr) == 0) Viewer.transitionType = VIEWER_TRANSITION_ANIMATE;
		else {
			ConsoleMessage ("Unknown NavigationInfo transitionType :%s:",node->transitionType.p[0]->strptr);
		}
	}

}




/* send a set_bind event from an event to this Bindable node */
void send_bind_to(struct X3D_Node *node, int value) {
	//printf ("\n%lf: send_bind_to, nodetype %s node %u value %d\n",TickTime,stringNodeType(node->_nodeType),node,value);  

	switch (node->_nodeType) {

	case NODE_Background:  {
		struct X3D_Background *bg = (struct X3D_Background *) node;
		bg->set_bind = value;
		bind_node (node, &background_tos,&background_stack[0]);
		break;
		}

	case NODE_TextureBackground: {
		struct X3D_TextureBackground *tbg = (struct X3D_TextureBackground *) node;
		tbg->set_bind = value;
		bind_node (node, &background_tos,&background_stack[0]);
		break;
		}

	case NODE_OrthoViewpoint: {
		struct X3D_OrthoViewpoint *ovp = (struct X3D_OrthoViewpoint *) node;
		ovp->set_bind = value;
		setMenuStatus(ovp->description->strptr);
		bind_node (node, &viewpoint_tos,&viewpoint_stack[0]);
		if (value==1) {
			bind_OrthoViewpoint (ovp);
		}
		break;
		}

	case NODE_Viewpoint:  {
		struct X3D_Viewpoint* vp = (struct X3D_Viewpoint *) node;
		vp->set_bind = value;
		setMenuStatus (vp->description->strptr);
		bind_node (node, &viewpoint_tos,&viewpoint_stack[0]);
		if (value==1) {
			bind_Viewpoint (vp);
		}
		break;
		}

	case NODE_GeoViewpoint:  {
		struct X3D_GeoViewpoint *gvp = (struct X3D_GeoViewpoint *) node;
		gvp->set_bind = value;
		setMenuStatus (gvp->description->strptr);
		bind_node (node, &viewpoint_tos,&viewpoint_stack[0]);
		if (value==1) {
			bind_GeoViewpoint (gvp);
		}
		break;
		}


	case NODE_Fog:  {
		struct X3D_Fog *fg = (struct X3D_Fog *) node;
		fg->set_bind = value;
		bind_node (node, &fog_tos,&fog_stack[0]);
		break;
		}

	case NODE_NavigationInfo:  {
		struct X3D_NavigationInfo *nv = (struct X3D_NavigationInfo *) node;
		nv->set_bind = value;
		bind_node (node, &navi_tos,&navi_stack[0]);
		if (value==1) set_naviinfo(nv);
		break;
		}

	default:
		ConsoleMessage("send_bind_to, cant send a set_bind to %s !!\n",stringNodeType(node->_nodeType));
	}
}




/* Do binding for node and stack - works for all bindable nodes */

/* return the setBind offset of this node */
static size_t setBindofst(void *node) {
	struct X3D_Background *tn;
	tn = (struct X3D_Background *) node;
	switch (tn->_nodeType) {
		case NODE_Background: return offsetof(struct X3D_Background, set_bind);
		case NODE_TextureBackground: return offsetof(struct X3D_TextureBackground, set_bind);
		case NODE_Viewpoint: return offsetof(struct X3D_Viewpoint, set_bind);
		case NODE_OrthoViewpoint: return offsetof(struct X3D_OrthoViewpoint, set_bind);
		case NODE_GeoViewpoint: return offsetof(struct X3D_GeoViewpoint, set_bind);
		case NODE_Fog: return offsetof(struct X3D_Fog, set_bind);
		case NODE_NavigationInfo: return offsetof(struct X3D_NavigationInfo, set_bind);
		default: {printf ("setBindoffst - huh? node type %d\n",tn->_nodeType); }
	}
	return 0;
}

/* return the isBound offset of this node */
static size_t bindTimeoffst (struct X3D_Node  *node) {
	X3D_NODE_CHECK(node);

	switch (node->_nodeType) {
		case NODE_Background: return offsetof(struct X3D_Background, bindTime);
		case NODE_TextureBackground: return offsetof(struct X3D_TextureBackground, bindTime);
		case NODE_Viewpoint: return offsetof(struct X3D_Viewpoint, bindTime);
		case NODE_OrthoViewpoint: return offsetof(struct X3D_Viewpoint, bindTime);
		case NODE_GeoViewpoint: return offsetof(struct X3D_GeoViewpoint, bindTime);
		case NODE_Fog: return offsetof(struct X3D_Fog, bindTime);
		case NODE_NavigationInfo: return offsetof(struct X3D_NavigationInfo, bindTime);
		default: {printf ("bindTimeoffst  - huh? node type %s\n",stringNodeType(node->_nodeType)); }
	}
	return 0;
}

/* return the isBound offset of this node */
static size_t isboundofst(void *node) {
	struct X3D_Background *tn;

	/* initialization */
	tn = (struct X3D_Background *) node;

	X3D_NODE_CHECK(node);

	switch (tn->_nodeType) {
		case NODE_Background: return offsetof(struct X3D_Background, isBound);
		case NODE_TextureBackground: return offsetof(struct X3D_TextureBackground, isBound);
		case NODE_Viewpoint: return offsetof(struct X3D_Viewpoint, isBound);
		case NODE_OrthoViewpoint: return offsetof(struct X3D_Viewpoint, isBound);
		case NODE_GeoViewpoint: return offsetof(struct X3D_GeoViewpoint, isBound);
		case NODE_Fog: return offsetof(struct X3D_Fog, isBound);
		case NODE_NavigationInfo: return offsetof(struct X3D_NavigationInfo, isBound);
		default: {printf ("isBoundoffst - huh? node type %s\n",stringNodeType(tn->_nodeType)); }
	}
	return 0;
}

void bind_node (struct X3D_Node *node, int *tos, uintptr_t *stack) {

	uintptr_t *oldstacktop;
	uintptr_t *newstacktop;
	uintptr_t unbindNode;
	char *nst;			/* used for pointer maths */
	unsigned int *setBindptr;	/* this nodes setBind */
	unsigned int *isBoundptr;	/* this nodes isBound */
	unsigned int *oldboundptr;	/* previous nodes isBound */
	int i, ioldposition, do_unbind;
	struct X3D_Background *bgnode;
	size_t offst;

	X3D_NODE_CHECK(node);

	bgnode=(struct X3D_Background*) node;
	/* lets see what kind of node this is... */

#ifdef BINDVERBOSE
	printf ("\nbind_node, we have %d (%s) tos %d \n",bgnode->_nodeType,stringNodeType(bgnode->_nodeType),*tos); 
	#endif

	/* setup some variables. Use char * as a pointer as it is ok between 32
	   and 64 bit systems for a pointer arithmetic. */
	nst = (char *)node;
	nst += setBindofst(node);
	setBindptr = (unsigned int *)nst;

	nst = (char *)node;
	nst += isboundofst(node);
	isBoundptr = (unsigned int *) nst;

        if (*isBoundptr && (*setBindptr != 0) ){ 
			#ifdef BINDVERBOSE
			/*printf("%d already bound\n",(uintptr_t)node);*/
			#endif
			*setBindptr = 100; 
			return; } /* It has to be at the top of the stack so return */

	if (*tos >=0) {oldstacktop = stack + *tos;}
	else oldstacktop = stack;

	#ifdef BINDVERBOSE
	printf ("bind_node, node %d, set_bind %d tos %d\n",node,*setBindptr,*tos); 
	printf ("stack %x, oldstacktop %x sizeof usint %x\n",stack, oldstacktop,
			sizeof(unsigned int));
	#endif
	
	

	/* we either have a setBind of 1, which is a push, or 0, which
	   is a pop. the value of 100 (arbitrary) indicates that this
	   is not a new push or pop */

	if (*setBindptr == 1) {
		/* PUSH THIS TO THE TOP OF THE STACK */

		/* are we off the top of the stack? */
		/*if (*tos >= (MAX_STACK-2)) return;  
		   dug9: too restrictive. It means you can't have > 20 
		   different viewpoints/navinfos in your scene.
		   Better: scroll the stack down. See below.
		 */

		/* isBound mimics setBind */
		*isBoundptr = 1;

		/* unset the set_bind flag  - setBind can be 0 or 1; lets make it garbage */
		*setBindptr = 100;

		MARK_EVENT (node, (unsigned int) isboundofst(node));

		/* set up the "bindTime" field */
		offst = bindTimeoffst(node);
		if (offst != 0) {
			double *dp;
			/* add them as bytes, not pointers. */
			dp = offsetPointer_deref(double*, node, offst);
			*dp = TickTime;
			MARK_EVENT (node, offst);
		} 


		/* set up pointers, increment stack */
		/* is this node somewhere in the stack already? */
		ioldposition = -1;
		do_unbind = 1;
		unbindNode = *oldstacktop;
		for(i=0;i<=(*tos);i++)
		{
			#ifdef BINDVERBOSE
			printf("%d %d %d\n",stack[i],(uintptr_t)node,i);
			#endif
			if(stack[i] == (uintptr_t) node) ioldposition = i;
		}
		if(ioldposition > -1)
		{
			#ifdef BINDVERBOSE
			printf("Yes - it is already in the stack at position %d\n",ioldposition);
			#endif
			if(ioldposition == *tos)
				do_unbind = 0; /* already top of stack */
			else
			{
				/* bubble up */
				for(i=ioldposition;i<(*tos);i++)
					stack[i] = stack[i+1];
			}
		}
		else
		{
			if (*tos >= (MAX_STACK-2))
			{
				/* scroll stack down (oldest bound get scrolled off) */
				for(i=0;i<(*tos);i++)
					stack[i] = stack[i+1];
			}
			else
			{
				*tos = *tos+1;
				#ifdef BINDVERBOSE
				printf ("just incremented tos, ptr %x val %d\n",tos,*tos);
				#endif
			}
		}

		newstacktop = stack + *tos;
		#ifdef BINDVERBOSE
		printf ("so, newstacktop is %x\n",newstacktop);
		#endif


		/* save pointer to new top of stack */
		*newstacktop = (uintptr_t) node;
		update_node(X3D_NODE(newstacktop));

		/* was there another DIFFERENT node at the top of the stack?
		   have to check for a different one, as if we are binding to the current
		   Viewpoint, then we do NOT want to unbind it, if we do then the current
		   top of stack Viewpoint is unbound! */

		#ifdef BINDVERBOSE
		//printf ("before if... *tos %d *oldstacktop %d *newstacktop %d\n",*tos, *oldstacktop, *newstacktop);
		printf ("before if... *tos %d *oldstacktop %d *newstacktop %d\n",*tos, unbindNode, *newstacktop);
		#endif

		if ((*tos >= 1) && do_unbind ) {
			/* yep... unbind it, and send an event in case anyone cares */
			//oldboundptr = (unsigned int *) (*oldstacktop  + (uintptr_t)isboundofst((void *)*oldstacktop));
			oldboundptr = (unsigned int *) (unbindNode  + (uintptr_t)isboundofst((void *)unbindNode));
			*oldboundptr = 0;

			#ifdef BINDVERBOSE
			printf ("....bind_node, in set_bind true, unbinding node %d\n",unbindNode);
			#endif

			//MARK_EVENT (X3D_NODE(*oldstacktop), (unsigned int) isboundofst((void *)*oldstacktop));
			MARK_EVENT (X3D_NODE(unbindNode), (unsigned int) isboundofst((void *)unbindNode));

			/* tell the possible parents of this change */
			//update_node(X3D_NODE(*oldstacktop));
			update_node(X3D_NODE(unbindNode));
		}
	} else {
		/* POP FROM TOP OF STACK  - if we ARE the top of stack */

		/* isBound mimics setBind */
		*isBoundptr = 0;

		/* unset the set_bind flag  - setBind can be 0 or 1; lets make it garbage */
		*setBindptr = 100;

		/* anything on stack? */
		if (*tos <= -1) return;   /* too many pops */

		MARK_EVENT (node, (unsigned int) isboundofst(node));

		#ifdef BINDVERBOSE
		printf ("old TOS is %d (%s) , we are %d (%s)\n",*oldstacktop,
		((struct X3D_Viewpoint *)(*oldstacktop))->description->strptr, node, ((struct X3D_Viewpoint *)node)->description->strptr);
		#endif


		/* are we the top of the stack? */
		if ((uintptr_t) node != *oldstacktop) {
			return;
		}

		#ifdef BINDVERBOSE
		printf ("ok, we were TOS, setting %d (%s) to 0\n",node, ((struct X3D_Viewpoint *)node)->description->strptr);
		#endif


		*tos = *tos - 1;

		if (*tos >= 0) {
			/* stack is not empty */
			newstacktop = stack + *tos;
			#ifdef BINDVERBOSE
			printf ("   .... and we had a stack value; binding node %d\n",*newstacktop);
			#endif

			/* set the popped value of isBound to true */
			isBoundptr = (unsigned int *) (*newstacktop + (uintptr_t)isboundofst((void *)*newstacktop));

			*isBoundptr = 1;

			/* tell the possible parents of this change */
			nst = (void *) (*newstacktop + (int) 0);
			update_node(X3D_NODE(nst));

			MARK_EVENT (X3D_NODE(nst), (unsigned int) isboundofst(nst));
		}
	}
}

void render_Fog (struct X3D_Fog *node) {
	#ifndef GL_ES_VERSION_2_0 /* this should be handled in material shader */
	GLDOUBLE mod[16];
	GLDOUBLE proj[16];
	GLDOUBLE x,y,z;
	GLDOUBLE x1,y1,z1;
	GLDOUBLE sx, sy, sz;
	GLfloat fog_colour [4];
	char *fogptr;
	int foglen;
	GLDOUBLE unit[16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};


	/* printf ("render_Fog, node %d isBound %d color %f %f %f set_bind %d\n",
	node, node->isBound, node->color.c[0],node->color.c[1],node->color.c[2],node->set_bind); */

	/* check the set_bind eventin to see if it is TRUE or FALSE */
	if (node->set_bind < 100) {

		bind_node (X3D_NODE(node), &fog_tos,&fog_stack[0]);

		/* if we do not have any more nodes on top of stack, disable fog */
		FW_GL_DISABLE (GL_FOG);
		fog_enabled = FALSE;		


	}

	if(!node->isBound) return;
	if (node->visibilityRange <= 0.00001) return;

	fog_colour[0] = node->color.c[0];
	fog_colour[1] = node->color.c[1];
	fog_colour[2] = node->color.c[2];
	fog_colour[3] = (float) 1.0;

	fogptr = node->fogType->strptr;
	foglen = node->fogType->len;
	FW_GL_PUSH_MATRIX();
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, mod);
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, proj);
	/* Get origin */
	FW_GLU_UNPROJECT(0.0f,0.0f,0.0f,mod,proj,viewport,&x,&y,&z);
	FW_GL_TRANSLATE_D(x,y,z);

	FW_GLU_UNPROJECT(0.0f,0.0f,0.0f,mod,unit,viewport,&x,&y,&z);
	/* Get scale */
	FW_GLU_PROJECT(x+1,y,z,mod,unit,viewport,&x1,&y1,&z1);
	sx = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	FW_GLU_PROJECT(x,y+1,z,mod,unit,viewport,&x1,&y1,&z1);
	sy = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	FW_GLU_PROJECT(x,y,z+1,mod,unit,viewport,&x1,&y1,&z1);
	sz = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	/* Undo the translation and scale effects */
	FW_GL_SCALE_D(sx,sy,sz);


	/* now do the foggy stuff */
	FW_GL_FOGFV(GL_FOG_COLOR,fog_colour);

	/* make the fog look like the examples in the VRML Source Book */
	if (strcmp("LINEAR",fogptr)) {
		/* Exponential */
		FW_GL_FOGF(GL_FOG_DENSITY, (float) (4.0)/ (node->visibilityRange));
		FW_GL_FOGF(GL_FOG_END, (float) (node->visibilityRange));
		FW_GL_FOGI(GL_FOG_MODE, GL_EXP);
	} else {
		/* Linear */
		FW_GL_FOGF(GL_FOG_START, (float) 1.0);
		FW_GL_FOGF(GL_FOG_END, (float) (node->visibilityRange));
		FW_GL_FOGI(GL_FOG_MODE, GL_LINEAR);
	}
	FW_GL_ENABLE (GL_FOG);
	fog_enabled = TRUE;

	FW_GL_POP_MATRIX();
	#endif /* GL_ES_VERSION_2_0 this should be handled in material shader */
}


/******************************************************************************
 *
 * Background, TextureBackground stuff 
 *
 ******************************************************************************/

/* save a Background vertex into the __points and __colours arrays */
static void saveBGVert (float *colptr, float *pt,
		int *vertexno, float *col, double dist,
		double x, double y, double z) {
		/* save the colour */
		float ccc[3];
		memcpy(ccc,col,sizeof(float)*3);
		if(usingAnaglyph2())
		{
			fwAnaglyphRemapf(&ccc[0], &ccc[1],&ccc[2], ccc[0],ccc[1], ccc[2]);
			//float gray = .299*ccc[0] + .587*ccc[1] + .114*ccc[2];
			//ccc[0] = ccc[1] = ccc[2] = gray;
		}
		memcpy (&colptr[*vertexno*3], ccc, sizeof(float)*3);

		/* and, save the vertex info */
		pt[*vertexno*3+0] = (float)(x*dist);
		pt[*vertexno*3+1] = (float)(y*dist);
		pt[*vertexno*3+2] = (float)(z*dist);

		(*vertexno)++;
}

/* the background centre follows our position, so, move it! */
static void moveBackgroundCentre () {
	GLDOUBLE mod[16];
	GLDOUBLE proj[16];
	GLDOUBLE unit[16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	GLDOUBLE x,y,z;
	GLDOUBLE x1,y1,z1;
	GLDOUBLE sx, sy, sz;

	/* glPushAttrib(GL_LIGHTING_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);  */
	FW_GL_PUSH_MATRIX();
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, mod);
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, proj);
	/* Get origin */
	FW_GLU_UNPROJECT(0.0f,0.0f,0.0f,mod,proj,viewport,&x,&y,&z);
	FW_GL_TRANSLATE_D(x,y,z);

	LIGHTING_OFF

	FW_GLU_UNPROJECT(0.0f,0.0f,0.0f,mod,unit,viewport,&x,&y,&z);
	/* Get scale */
	FW_GLU_PROJECT(x+1,y,z,mod,unit,viewport,&x1,&y1,&z1);
	sx = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	FW_GLU_PROJECT(x,y+1,z,mod,unit,viewport,&x1,&y1,&z1);
	sy = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	FW_GLU_PROJECT(x,y,z+1,mod,unit,viewport,&x1,&y1,&z1);
	sz = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );

	/* Undo the translation and scale effects */
	FW_GL_SCALE_D(sx,sy,sz);
}

static void recalculateBackgroundVectors(struct X3D_Background *node) {
	struct SFColor *c1,*c2;
	int hdiv;			/* number of horizontal strips allowed */
	int h,v;
	double va1, va2, ha1, ha2;	/* JS - vert and horiz angles 	*/
	int estq;
	int actq;

	/* filled in if this is a TextureBackground node */
	struct X3D_TextureBackground *tbnode;

	/* generic structures between nodes used for taking individual pointers from node defns */
	struct SFColor *skyCol; int skyColCt;
	struct SFColor *gndCol; int gndColCt;
	float  *skyAng; int skyAngCt;
	float  *gndAng; int gndAngCt;
	float *newPoints; float *newColors;
	double outsideRadius, insideRadius;

	/* initialization */
	tbnode = NULL;
	hdiv = 20;

	/* We draw spheres, one for the sky, one for the ground - outsideRadius and insideRadius */
	outsideRadius =  DEFAULT_FARPLANE* 0.750;
	insideRadius = DEFAULT_FARPLANE * 0.50;

	/* lets try these values - we will scale when we draw this */
	outsideRadius = 1.0;
	insideRadius = 0.5;

	/* handle Background and TextureBackgrounds here */
	if (node->_nodeType == NODE_Background) {
		skyCol = node->skyColor.p;
		gndCol = node ->groundColor.p;
		skyColCt = node->skyColor.n;
		gndColCt = node->groundColor.n;
		skyAng = node->skyAngle.p;
		gndAng = node ->groundAngle.p;
		skyAngCt = node->skyAngle.n;
		gndAngCt = node->groundAngle.n;
	} else {
		tbnode = (struct X3D_TextureBackground *) node;
		skyCol = tbnode->skyColor.p;
		gndCol = tbnode ->groundColor.p;
		skyColCt = tbnode->skyColor.n;
		gndColCt = tbnode->groundColor.n;
		skyAng = tbnode->skyAngle.p;
		gndAng = tbnode ->groundAngle.p;
		skyAngCt = tbnode->skyAngle.n;
		gndAngCt = tbnode->groundAngle.n;
	}

	/* do we have NO background triangles? (ie, maybe all textures??) */
	if ((skyColCt == 0) & (gndColCt == 0)) {
        	if (node->_nodeType == NODE_Background) {
			MARK_NODE_COMPILED
                	/* do we have an old background to destroy? */
                	FREE_IF_NZ (node->__points.p);
                	FREE_IF_NZ (node->__colours.p);
                	node->__quadcount = 0;
        	} else {
                	tbnode->_ichange = tbnode->_change; /* mimic MARK_NODE_COMPILED */

                	/* do we have an old background to destroy? */
                	FREE_IF_NZ (tbnode->__points.p);
                	FREE_IF_NZ (tbnode->__colours.p);
                	tbnode->__quadcount = 0;
        	}
		return;
	}


	/* calculate how many quads are required */
	estq=0; actq=0;
	if(skyColCt == 1) {
		estq += 40;
	} else {
		estq += (skyColCt-1) * 20 + 20;
		/* attempt to find exact estimate, fails if no skyAngle, so
		 simply changed above line to add 20 automatically.
		if ((skyColCt >2) &&
			(skyAngCt > skyColCt-2)) {
			if (skyAng[skyColCt-2] < (PI-0.01))
				estq += 20;
		}
		*/
	}

	if(gndColCt == 1) estq += 40;
	else if (gndColCt>0) estq += (gndColCt-1) * 20;

	/* now, MALLOC space for new arrays  - 3 points per vertex, 6 per quad. */
	newPoints = MALLOC (GLfloat *, sizeof (GLfloat) * estq * 3 * 6);
	newColors = MALLOC (GLfloat *, sizeof (GLfloat) * estq * 3 * 6);


	if(skyColCt == 1) {
		c1 = &skyCol[0];
		va1 = 0;
		va2 = PI/2;

		for(v=0; v<2; v++) {
			for(h=0; h<hdiv; h++) {
				ha1 = h * PI*2 / hdiv;
				ha2 = (h+1) * PI*2 / hdiv;
				/* 0 */ saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius, sin(va2)*cos(ha1), cos(va2), sin(va2)*sin(ha1));
				/* 1 */ saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius, sin(va2)*cos(ha2), cos(va2), sin(va2)*sin(ha2));
				/* 2 */ saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius, sin(va1)*cos(ha2), cos(va1), sin(va1)*sin(ha2));
				/* 0 */ saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius, sin(va2)*cos(ha1), cos(va2), sin(va2)*sin(ha1));
				/* 2 */ saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius, sin(va1)*cos(ha2), cos(va1), sin(va1)*sin(ha2));
				/* 3 */ saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius, sin(va1)*cos(ha1), cos(va1), sin(va1) * sin(ha1));
			}
			va1 = va2;
			va2 = PI;
		}
	} else {
		va1 = 0;
		/* this gets around a compiler warning - we really DO want last values of this from following
		   for loop */
		c1 = &skyCol[0];
		if (skyAngCt>0) {
			va2= skyAng[0];
		} else {
			va2 = PI/2;
		}
		c2=c1;


		for(v=0; v<(skyColCt-1); v++) {
			c1 = &skyCol[v];
			c2 = &skyCol[v+1];
			if (skyAngCt>0) { va2 = skyAng[v];}
			else { va2 = PI/2; }

			for(h=0; h<hdiv; h++) {
				ha1 = h * PI*2 / hdiv;
				ha2 = (h+1) * PI*2 / hdiv;
				/* 0 */ saveBGVert(newColors,newPoints, &actq,&c2->c[0],outsideRadius, sin(va2)*cos(ha1), cos(va2), sin(va2) * sin(ha1));
				/* 1 */ saveBGVert(newColors,newPoints, &actq,&c2->c[0],outsideRadius, sin(va2)*cos(ha2), cos(va2), sin(va2) * sin(ha2));
				/* 2 */ saveBGVert(newColors,newPoints, &actq,&c1->c[0],outsideRadius, sin(va1)*cos(ha2), cos(va1), sin(va1) * sin(ha2));
				/* 0 */ saveBGVert(newColors,newPoints, &actq,&c2->c[0],outsideRadius, sin(va2)*cos(ha1), cos(va2), sin(va2) * sin(ha1));
				/* 2 */ saveBGVert(newColors,newPoints, &actq,&c1->c[0],outsideRadius, sin(va1)*cos(ha2), cos(va1), sin(va1) * sin(ha2));
				/* 3 */ saveBGVert(newColors,newPoints, &actq,&c1->c[0],outsideRadius, sin(va1) * cos(ha1), cos(va1), sin(va1) * sin(ha1));
			}
			va1 = va2;
		}

		/* now, the spec states: "If the last skyAngle is less than pi, then the
		  colour band between the last skyAngle and the nadir is clamped to the last skyColor." */
		if (va2 < (PI-0.01)) {
			for(h=0; h<hdiv; h++) {
				ha1 = h * PI*2 / hdiv;
				ha2 = (h+1) * PI*2 / hdiv;
				/* 0 */ saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius, sin(PI) * cos(ha1), cos(PI), sin(PI) * sin(ha1));
				/* 1 */ saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius, sin(PI) * cos(ha2), cos(PI), sin(PI) * sin(ha2));
				/* 2 */ saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius, sin(va2) * cos(ha2), cos(va2), sin(va2) * sin(ha2));
				/* 0 */ saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius, sin(PI) * cos(ha1), cos(PI), sin(PI) * sin(ha1));
				/* 2 */ saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius, sin(va2) * cos(ha2), cos(va2), sin(va2) * sin(ha2));
				/* 3 */ saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius, sin(va2) * cos(ha1), cos(va2), sin(va2) * sin(ha1));
			}
		}
	}

	/* Do the ground, if there is anything  to do. */

	if (gndColCt>0) {
		if(gndColCt == 1) {
			c1 = &gndCol[0];
			for(h=0; h<hdiv; h++) {
				ha1 = h * PI*2 / hdiv;
				ha2 = (h+1) * PI*2 / hdiv;

				/* 0 */ saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius, sin(PI) * cos(ha1), cos(PI), sin(PI) * sin(ha1));
				/* 1 */ saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius, sin(PI) * cos(ha2), cos(PI), sin(PI) * sin(ha2));
				/* 2 */ saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius, sin(PI/2) * cos(ha2), cos(PI/2), sin(PI/2) * sin(ha2));
				/* 0 */ saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius, sin(PI) * cos(ha1), cos(PI), sin(PI) * sin(ha1));
				/* 2 */ saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius, sin(PI/2) * cos(ha2), cos(PI/2), sin(PI/2) * sin(ha2));
				/* 3 */ saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius, sin(PI/2) * cos(ha1), cos(PI/2), sin(PI/2) * sin(ha1));
			}
		} else {
			va1 = PI;
			for(v=0; v<gndColCt-1; v++) {
				c1 = &gndCol[v];
				c2 = &gndCol[v+1];
				if (v>=gndAngCt) va2 = PI; /* bounds check */
				else va2 = PI - gndAng[v];

				for(h=0; h<hdiv; h++) {
					ha1 = h * PI*2 / hdiv;
					ha2 = (h+1) * PI*2 / hdiv;

					/* 0 */ saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius, sin(va1)*cos(ha1), cos(va1), sin(va1)*sin(ha1));
					/* 1 */ saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius, sin(va1)*cos(ha2), cos(va1), sin(va1)*sin(ha2));
					/* 2 */ saveBGVert(newColors,newPoints,&actq,&c2->c[0],insideRadius, sin(va2)*cos(ha2), cos(va2), sin(va2)*sin(ha2));
					/* 0 */ saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius, sin(va1)*cos(ha1), cos(va1), sin(va1)*sin(ha1));
					/* 2 */ saveBGVert(newColors,newPoints,&actq,&c2->c[0],insideRadius, sin(va2)*cos(ha2), cos(va2), sin(va2)*sin(ha2));
					/* 3 */ saveBGVert(newColors,newPoints,&actq,&c2->c[0],insideRadius, sin(va2) * cos(ha1), cos(va2), sin(va2)*sin(ha1));
				}
				va1 = va2;
			}
		}
	}

	/* We have guessed at the quad count; lets make sure
	 * we record what we have. */
	if (actq > (estq*6)) {
		printf ("Background quadcount error, %d > %d\n",
				actq,estq);
		actq = 0;
	}

	/* save changes */
	/* if we are doing shaders, we write the vertex and color info to a VBO, else we keep pointers in the node */
	if (node->_nodeType == NODE_Background) {

		MARK_NODE_COMPILED

		/* do we have an old background to destroy? */
		FREE_IF_NZ (node->__points.p);
		FREE_IF_NZ (node->__colours.p);
		node->__quadcount = actq;
		#ifndef SHADERS_2011
		/* record this info if NOT doing shaders */
		node->__points.p = (struct SFVec3f *)newPoints;
		node->__colours.p = (struct SFColor *)newColors;
		#endif /* SHADERS_2011 */
	} else {
		tbnode->_ichange = tbnode->_change; /* mimic MARK_NODE_COMPILED */
		/* do we have an old background to destroy? */
		FREE_IF_NZ (tbnode->__points.p);
		FREE_IF_NZ (tbnode->__colours.p);
		tbnode->__quadcount = actq;
		#ifndef SHADERS_2011
		/* record this info if NOT doing shaders */
		tbnode->__points.p = (struct SFVec3f *)newPoints;
		tbnode->__colours.p = (struct SFColor *)newColors;
		#endif /* SHADERS_2011 */
	}


	#ifdef SHADERS_2011
	{
		struct MyVertex *combinedBuffer = MALLOC(struct MyVertex *, sizeof (struct MyVertex) * actq * 2);
		int i;
		float *npp = newPoints;
		float *ncp = newColors;


		if (node->_nodeType == NODE_Background) {
			if (node->__VBO == 0) glGenBuffers(1,(unsigned int*) &node->__VBO);
		} else {
			if (tbnode->__VBO == 0) glGenBuffers(1,(unsigned int*) &tbnode->__VBO);
		}

		/* stream both the vertex and colours together (could have done this above, but
		   maybe can redo this if we go 100% material shaders */

		/* NOTE - we use SFColorRGBA - and set the Alpha to 1 so that we can use the
		   shader with other nodes with Color fields */

		for (i=0; i<actq; i++) {
			combinedBuffer[i].vert.c[0] = *npp; npp++;
			combinedBuffer[i].vert.c[1] = *npp; npp++;
			combinedBuffer[i].vert.c[2] = *npp; npp++;
			combinedBuffer[i].col.c[0] = *ncp; ncp++;
			combinedBuffer[i].col.c[1] = *ncp; ncp++;
			combinedBuffer[i].col.c[2] = *ncp; ncp++;
			combinedBuffer[i].col.c[3] = 1.0f;
		}
		FREE_IF_NZ(newPoints);
		FREE_IF_NZ(newColors);

		/* send this data along ... */
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,node->__VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof (struct MyVertex)*actq, combinedBuffer, GL_STATIC_DRAW);
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,0);

		/* and, we can free it */
		FREE_IF_NZ(combinedBuffer);
	}
	#endif /* SHADERS_2011 */
}

void render_Background (struct X3D_Background *node) {
	/* if we are rendering blended nodes, don't bother with this one */
	if (render_blend) return;

	/* printf ("RBG, num %d node %d ib %d sb %d gepvp\n",node->__BGNumber, node,node->isBound,node->set_bind);    */
	/* check the set_bind eventin to see if it is TRUE or FALSE */
	if (node->set_bind < 100) {
		bind_node (X3D_NODE(node), &background_tos,&background_stack[0]);
	}

	/* don't even bother going further if this node is not bound on the top */
	if(!node->isBound) return;

	#ifndef GL_ES_VERSION_2_0
	/* is fog enabled? if so, disable it right now */
	if (fog_enabled ==TRUE) FW_GL_DISABLE (GL_FOG);
	#endif

	/* Cannot start_list() because of moving center, so we do our own list later */
	moveBackgroundCentre();

	if (NODE_NEEDS_COMPILING) {
		recalculateBackgroundVectors(node);
	}

	/* we have a sphere (maybe one and a half, as the sky and ground are different) so scale it up so that
	   all geometry fits within the spheres */
	FW_GL_SCALE_D (Viewer.backgroundPlane, Viewer.backgroundPlane, Viewer.backgroundPlane);

	#ifdef SHADERS_2011
		enableGlobalShader(backgroundSphereShader);

		FW_GL_ENABLECLIENTSTATE(GL_COLOR_ARRAY);
		FW_GL_ENABLECLIENTSTATE(GL_VERTEX_ARRAY);
		FW_GL_DISABLECLIENTSTATE(GL_NORMAL_ARRAY);

		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, node->__VBO);
		FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);

		#define BUFFER_OFFSET(i) ((char *)NULL + (i))
		FW_GL_VERTEX_POINTER(3, GL_FLOAT, (GLfloat) sizeof(struct MyVertex), (GLfloat *)BUFFER_OFFSET(0));   //The starting point of the VBO, for the vertices
		FW_GL_COLOR_POINTER(4, GL_FLOAT, (GLfloat) sizeof(struct MyVertex), (GLfloat *)BUFFER_OFFSET(sizeof(struct SFVec3f)));   //The starting point of Colours, 12 bytes away

		FW_GL_DRAWARRAYS (GL_TRIANGLES, 0, node->__quadcount);

		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);
		FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);
		TURN_GLOBAL_SHADER_OFF;
	#else

		/* now, display the lists */
		FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(GLfloat *)node->__points.p);
		FW_GL_COLOR_POINTER(3, GL_FLOAT, 0, (GLfloat *)node->__colours.p);
		FW_GL_ENABLECLIENTSTATE(GL_COLOR_ARRAY);
		FW_GL_ENABLECLIENTSTATE(GL_VERTEX_ARRAY);
		FW_GL_DISABLECLIENTSTATE(GL_NORMAL_ARRAY);

		FW_GL_DRAWARRAYS (GL_TRIANGLES, 0, node->__quadcount);

		FW_GL_DISABLECLIENTSTATE(GL_COLOR_ARRAY);
		FW_GL_ENABLECLIENTSTATE(GL_NORMAL_ARRAY);
	#endif

	/* now, for the textures, if they exist */
	if (((node->backUrl).n>0) ||
			((node->frontUrl).n>0) ||
			((node->leftUrl).n>0) ||
			((node->rightUrl).n>0) ||
			((node->topUrl).n>0) ||
			((node->bottomUrl).n>0)) {

        	FW_GL_ENABLE(GL_TEXTURE_2D);
        	FW_GL_COLOR3D(1.0,1.0,1.0);
        	FW_GL_ENABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);
        	FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,BackgroundVert);
        	FW_GL_NORMAL_POINTER (GL_FLOAT,0,Backnorms);
        	FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,boxtex);

		#ifdef SHADERS_2011
		enableGlobalShader(backgroundTextureBoxShader);
		#endif

		loadBackgroundTextures(node);

        	FW_GL_DISABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);

		#ifdef SHADERS_2011
		TURN_GLOBAL_SHADER_OFF;
		#endif
	}
	FW_GL_POP_MATRIX();

	#ifndef GL_ES_VERSION_2_0
	/* is fog enabled? if so, disable it right now */
	if (fog_enabled ==TRUE) FW_GL_ENABLE (GL_FOG);
	#endif

}


void render_TextureBackground (struct X3D_TextureBackground *node) {
	/* if we are rendering blended nodes, don't bother with this one */
	if (render_blend) return;


	/* printf ("RTBG, node %d ib %d sb %d gepvp\n",node,node->isBound,node->set_bind);  */
	/* check the set_bind eventin to see if it is TRUE or FALSE */
	if (node->set_bind < 100) {
		bind_node (X3D_NODE(node), &background_tos,&background_stack[0]);
	}

	/* don't even bother going further if this node is not bound on the top */
	if(!node->isBound) return;

	/* is fog enabled? if so, disable it right now */
	#ifndef GL_ES_VERSION_2_0
	if (fog_enabled ==TRUE) FW_GL_DISABLE (GL_FOG);
	#endif

	/* Cannot start_list() because of moving center, so we do our own list later */
	moveBackgroundCentre();

	if  NODE_NEEDS_COMPILING
		/* recalculateBackgroundVectors will determine exact node type */
		recalculateBackgroundVectors((struct X3D_Background *)node);	

	/* we have a sphere (maybe one and a half, as the sky and ground are different) so scale it up so that
	   all geometry fits within the spheres */
	FW_GL_SCALE_D (Viewer.backgroundPlane, Viewer.backgroundPlane, Viewer.backgroundPlane);


	#ifdef SHADERS_2011
		enableGlobalShader(backgroundSphereShader);

		FW_GL_ENABLECLIENTSTATE(GL_COLOR_ARRAY);
		FW_GL_ENABLECLIENTSTATE(GL_VERTEX_ARRAY);
		FW_GL_DISABLECLIENTSTATE(GL_NORMAL_ARRAY);

		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, node->__VBO);
		FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);

		#define BUFFER_OFFSET(i) ((char *)NULL + (i))
		FW_GL_VERTEX_POINTER(3, GL_FLOAT, (GLfloat) sizeof(struct MyVertex), (GLfloat *)BUFFER_OFFSET(0));   //The starting point of the VBO, for the vertices
		FW_GL_COLOR_POINTER(4, GL_FLOAT, (GLfloat) sizeof(struct MyVertex), (GLfloat *)BUFFER_OFFSET(sizeof(struct SFVec3f)));   //The starting point of Colours, 12 bytes away

		FW_GL_DRAWARRAYS (GL_TRIANGLES, 0, node->__quadcount);

		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);
		FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);
		TURN_GLOBAL_SHADER_OFF;
	#else

		/* now, display the lists */
		FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(GLfloat *)node->__points.p);
		FW_GL_COLOR_POINTER(3, GL_FLOAT, 0, (GLfloat *)node->__colours.p);
		FW_GL_ENABLECLIENTSTATE(GL_COLOR_ARRAY);
		FW_GL_ENABLECLIENTSTATE(GL_VERTEX_ARRAY);
		FW_GL_DISABLECLIENTSTATE(GL_NORMAL_ARRAY);

		FW_GL_DRAWARRAYS (GL_TRIANGLES, 0, node->__quadcount);

		FW_GL_DISABLECLIENTSTATE(GL_COLOR_ARRAY);
		FW_GL_ENABLECLIENTSTATE(GL_NORMAL_ARRAY);
	#endif
	/* now, for the textures, if they exist */
	if ((node->backTexture !=0) ||
			(node->frontTexture !=0) ||
			(node->leftTexture !=0) ||
			(node->rightTexture !=0) ||
			(node->topTexture !=0) ||
			(node->bottomTexture !=0)) {

		#ifdef SHADERS_2011
		enableGlobalShader(backgroundTextureBoxShader);
		#endif


		loadTextureBackgroundTextures(node);
        	FW_GL_DISABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);

		#ifdef SHADERS_2011
		TURN_GLOBAL_SHADER_OFF;
		#endif
	}

	/* pushes are done in moveBackgroundCentre */
	FW_GL_POP_MATRIX();

	#ifndef GL_ES_VERSION_2_0
	/* is fog enabled? if so, disable it right now */
	if (fog_enabled ==TRUE) FW_GL_ENABLE (GL_FOG);
	#endif
}
