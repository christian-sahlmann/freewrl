/*
=INSERT_TEMPLATE_HERE=

$Id: Bindable.c,v 1.10 2009/04/08 19:40:55 crc_canada Exp $

Bindable nodes - Background, TextureBackground, Fog, NavigationInfo, Viewpoint, GeoViewpoint.

*/

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
#include "../world_script/CScripts.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"

#include "Bindable.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../scenegraph/Component_Geospatial.h"


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
int forceBackgroundRecompile = FALSE;

/* Background - fog nodes do not affect the background node rendering. */
int fog_enabled = FALSE;


void saveBGVert (float *colptr, float *pt,
		int *vertexno, float *col, double dist,
		double x, double y, double z) ;

/* this is called after a Viewpoint or GeoViewpoint bind */
void reset_upvector() {
    ViewerUpvector.x = 0;
    ViewerUpvector.y = 0;
    ViewerUpvector.z = 0;
}

/* called when binding NavigationInfo nodes */
void set_naviinfo(struct X3D_NavigationInfo *node) {
	struct Uni_String **svptr;
	int i;
	char *typeptr;

	if (node->avatarSize.n<2) {
		printf ("set_naviinfo, avatarSize smaller than expected\n");
	} else {
        	naviinfo.width = node->avatarSize.p[0];
        	naviinfo.height = node->avatarSize.p[1];
        	naviinfo.step = (node->avatarSize.p[2] * node->speed) * 2;
	}
        Viewer.speed = (double) node->speed;


	/* keep track of valid Navigation types. */
	svptr = node->type.p;

	/* assume "NONE" is set */
	for (i=0; i<6; i++) Viewer.oktypes[i] = FALSE;


	/* now, find the ones that are ok */
	for (i = 0; i < node->type.n; i++) {
		/*  get the string pointer */
		typeptr = svptr[i]->strptr;

		if (strcmp(typeptr,"WALK") == 0) {
			Viewer.oktypes[WALK] = TRUE;
			if (i==0) set_viewer_type(WALK);
		}
		if (strcmp(typeptr,"FLY") == 0) {
			Viewer.oktypes[FLY] = TRUE;
			if (i==0) set_viewer_type(FLY);
		}
		if (strcmp(typeptr,"EXAMINE") == 0) {
			Viewer.oktypes[EXAMINE] = TRUE;
			if (i==0) set_viewer_type(EXAMINE);
		}
		if (strcmp(typeptr,"NONE") == 0) {
			Viewer.oktypes[NONE] = TRUE;
			if (i==0) set_viewer_type(NONE);
		}
		if (strcmp(typeptr,"EXFLY") == 0) {
			Viewer.oktypes[EXFLY] = TRUE;
			if (i==0) set_viewer_type(EXFLY);
		}
		if (strcmp(typeptr,"ANY") == 0) {
			Viewer.oktypes[EXAMINE] = TRUE;
			Viewer.oktypes[WALK] = TRUE;
			Viewer.oktypes[EXFLY] = TRUE;
			Viewer.oktypes[FLY] = TRUE;
			if (i==0) set_viewer_type (WALK); /*  just choose one */
		}
	}
        Viewer.headlight = node->headlight;
	/* tell the menu buttons of the state of this headlight */
	setMenuButton_headlight(node->headlight);
}




/* send a set_bind event from an event to this Bindable node */
void send_bind_to(struct X3D_Node *node, int value) {
	struct X3D_Background *bg;
	struct X3D_TextureBackground *tbg;
	struct X3D_Fog *fg;
	struct X3D_NavigationInfo *nv;
	struct X3D_Viewpoint *vp;
	struct X3D_GeoViewpoint *gvp;
	char * nameptr;

	/* printf ("\n%lf: send_bind_to, nodetype %s node %u value %d\n",TickTime,stringNodeType(node->_nodeType),node,value);  */

	switch (node->_nodeType) {

	case NODE_Background: 
		/* this is node a Background node*/
		bg = (struct X3D_Background *) node;
		bg->set_bind = value;
		bind_node (node, &background_tos,&background_stack[0]);
		break;

	case NODE_TextureBackground:
		/* this is a TextureBackground node */
		tbg = (struct X3D_TextureBackground *) node;
		tbg->set_bind = value;
		bind_node (node, &background_tos,&background_stack[0]);
		break;

	case NODE_Viewpoint: 
		vp = (struct X3D_Viewpoint *) node;
		vp->set_bind = value;
		nameptr = vp->description->strptr;
		setMenuStatus (nameptr);

		bind_node (node, &viewpoint_tos,&viewpoint_stack[0]);

		/* up_vector is reset after a bind */
		if (value==1) {
			reset_upvector();
			bind_viewpoint (vp);
		}
		break;

	case NODE_GeoViewpoint: 
		gvp = (struct X3D_GeoViewpoint *) node;
		gvp->set_bind = value;
		nameptr = gvp->description->strptr;
		setMenuStatus (nameptr);

		bind_node (node, &viewpoint_tos,&viewpoint_stack[0]);

		/* up_vector is reset after a bind */
		if (value==1) {
			reset_upvector();
			bind_geoviewpoint (gvp);
		}
		break;


	case NODE_Fog: 
		fg = (struct X3D_Fog *) node;
		fg->set_bind = value;
		bind_node (node, &fog_tos,&fog_stack[0]);
		break;

	case NODE_NavigationInfo: 
		nv = (struct X3D_NavigationInfo *) node;
		nv->set_bind = value;
		bind_node (node, &navi_tos,&navi_stack[0]);
		if (value==1) set_naviinfo(nv);

		break;

	default:
		ConsoleMessage("send_bind_to, cant send a set_bind to %s !!\n",stringNodeType(node->_nodeType));
	}
}




/* Do binding for node and stack - works for all bindable nodes */

/* return the setBind offset of this node */
unsigned int setBindofst(void *node) {
	struct X3D_Background *tn;
	tn = (struct X3D_Background *) node;
	switch (tn->_nodeType) {
		case NODE_Background: return offsetof(struct X3D_Background, set_bind);
		case NODE_TextureBackground: return offsetof(struct X3D_TextureBackground, set_bind);
		case NODE_Viewpoint: return offsetof(struct X3D_Viewpoint, set_bind);
		case NODE_GeoViewpoint: return offsetof(struct X3D_GeoViewpoint, set_bind);
		case NODE_Fog: return offsetof(struct X3D_Fog, set_bind);
		case NODE_NavigationInfo: return offsetof(struct X3D_NavigationInfo, set_bind);
		default: {printf ("setBindoffst - huh? node type %d\n",tn->_nodeType); }
	}
	return 0;
}

/* return the isBound offset of this node */
int bindTimeoffst (struct X3D_Node  *node) {
	X3D_NODE_CHECK(node);

	switch (node->_nodeType) {
		case NODE_Background: return offsetof(struct X3D_Background, bindTime);
		case NODE_TextureBackground: return offsetof(struct X3D_TextureBackground, bindTime);
		case NODE_Viewpoint: return offsetof(struct X3D_Viewpoint, bindTime);
		case NODE_GeoViewpoint: return offsetof(struct X3D_GeoViewpoint, bindTime);
		case NODE_Fog: return offsetof(struct X3D_Fog, bindTime);
		case NODE_NavigationInfo: return offsetof(struct X3D_NavigationInfo, bindTime);
		default: {printf ("bindTimeoffst  - huh? node type %s\n",stringNodeType(node->_nodeType)); }
	}
	return 0;
}

/* return the isBound offset of this node */
int isboundofst(void *node) {
	struct X3D_Background *tn;
	tn = (struct X3D_Background *) node;

	X3D_NODE_CHECK(node);

	switch (tn->_nodeType) {
		case NODE_Background: return offsetof(struct X3D_Background, isBound);
		case NODE_TextureBackground: return offsetof(struct X3D_TextureBackground, isBound);
		case NODE_Viewpoint: return offsetof(struct X3D_Viewpoint, isBound);
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
	char *nst;			/* used for pointer maths */
	unsigned int *setBindptr;	/* this nodes setBind */
	unsigned int *isBoundptr;	/* this nodes isBound */
	unsigned int *oldboundptr;	/* previous nodes isBound */

	struct X3D_Background *bgnode;
	unsigned int offst;

	X3D_NODE_CHECK(node);

	bgnode=(struct X3D_Background*) node;
	/* lets see what kind of node this is... */
	
	#ifdef BINDVERBOSE
	printf ("bind_node, we have %d (%s) tos %d \n",bgnode->_nodeType,stringNodeType(bgnode->_nodeType),*tos); 
	#endif

	
	/* setup some variables. Use char * as a pointer as it is ok between 32
	   and 64 bit systems for a pointer arithmetic. */
	nst = (char *)node;
	nst += setBindofst(node);
	setBindptr = (unsigned int *)nst;

	nst = (char *)node;
	nst += isboundofst(node);
	isBoundptr = (unsigned int *) nst;

        if (*isBoundptr && (*setBindptr != 0) ){ *setBindptr = 100; return; } /* It has to be at the top of the stack so return */

	if (*tos >=0) {oldstacktop = stack + *tos;}
	else oldstacktop = stack;

	
	#ifdef BINDVERBOSE
	printf ("\nbind_node, node %d, set_bind %d tos %d\n",node,*setBindptr,*tos); 
	printf ("stack %x, oldstacktop %x sizeof usint %x\n",stack, oldstacktop,
			sizeof(unsigned int));
	#endif
	
	

	/* we either have a setBind of 1, which is a push, or 0, which
	   is a pop. the value of 100 (arbitrary) indicates that this
	   is not a new push or pop */

	if (*setBindptr == 1) {
		/* PUSH THIS TO THE TOP OF THE STACK */

		/* are we off the top of the stack? */
		if (*tos >= (MAX_STACK-2)) return;

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
			dp = (double *) (((unsigned char*)node)+offst);
			*dp = TickTime;
			MARK_EVENT (node, offst);
		} 


		/* set up pointers, increment stack */
		*tos = *tos+1;
		#ifdef BINDVERBOSE
		printf ("just incremented tos, ptr %d val %d\n",tos,*tos);
		#endif

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
		printf ("before if... *tos %d *oldstacktop %d *newstacktop %d\n",*tos, *oldstacktop, *newstacktop);
		#endif

		if ((*tos >= 1) && (*oldstacktop!=*newstacktop)) {
			/* yep... unbind it, and send an event in case anyone cares */
			oldboundptr = (unsigned int *) (*oldstacktop  + (uintptr_t)isboundofst((void *)*oldstacktop));
			*oldboundptr = 0;

			#ifdef BINDVERBOSE
			printf ("....bind_node, in set_bind true, unbinding node %d\n",*oldstacktop);
			#endif

			MARK_EVENT (X3D_NODE(*oldstacktop), (unsigned int) isboundofst((void *)*oldstacktop));

			/* tell the possible parents of this change */
			update_node(X3D_NODE(*oldstacktop));
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


		if ((uintptr_t) node != *oldstacktop) return;

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
	GLdouble mod[16];
	GLdouble proj[16];
	GLdouble unit[16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	GLdouble x,y,z;
	GLdouble x1,y1,z1;
	GLdouble sx, sy, sz;
	GLfloat fog_colour [4];
	char *fogptr;
	int foglen;


	/* printf ("render_Fog, node %d isBound %d color %f %f %f set_bind %d\n",
	node, node->isBound, node->color.c[0],node->color.c[1],node->color.c[2],node->set_bind); */

	/* check the set_bind eventin to see if it is TRUE or FALSE */
	if (node->set_bind < 100) {

		bind_node (X3D_NODE(node), &fog_tos,&fog_stack[0]);

		/* if we do not have any more nodes on top of stack, disable fog */
		glDisable (GL_FOG);
		fog_enabled = FALSE;		


	}

	if(!node->isBound) return;
	if (node->visibilityRange <= 0.00001) return;

	fog_colour[0] = node->color.c[0];
	fog_colour[1] = node->color.c[1];
	fog_colour[2] = node->color.c[2];
	fog_colour[3] = 1.0;

	fogptr = node->fogType->strptr;
	foglen = node->fogType->len;
	FW_GL_PUSH_MATRIX();
	fwGetDoublev(GL_MODELVIEW_MATRIX, mod);
	fwGetDoublev(GL_PROJECTION_MATRIX, proj);
	/* Get origin */
	gluUnProject(0.0f,0.0f,0.0f,mod,proj,viewport,&x,&y,&z);
	FW_GL_TRANSLATE_D(x,y,z);

	gluUnProject(0.0f,0.0f,0.0f,mod,unit,viewport,&x,&y,&z);
	/* Get scale */
	gluProject(x+1,y,z,mod,unit,viewport,&x1,&y1,&z1);
	sx = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	gluProject(x,y+1,z,mod,unit,viewport,&x1,&y1,&z1);
	sy = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	gluProject(x,y,z+1,mod,unit,viewport,&x1,&y1,&z1);
	sz = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	/* Undo the translation and scale effects */
	FW_GL_SCALE_D(sx,sy,sz);


	/* now do the foggy stuff */
	glFogfv(GL_FOG_COLOR,fog_colour);

	/* make the fog look like the examples in the VRML Source Book */
	if (strcmp("LINEAR",fogptr)) {
		/* Exponential */
		glFogf(GL_FOG_DENSITY, (float) (4.0)/ (node->visibilityRange));
		glFogf(GL_FOG_END, (float) (node->visibilityRange));
		glFogi(GL_FOG_MODE, GL_EXP);
	} else {
		/* Linear */
		glFogf(GL_FOG_START, 1.0);
		glFogf(GL_FOG_END, (float) (node->visibilityRange));
		glFogi(GL_FOG_MODE, GL_LINEAR);
	}
	glEnable (GL_FOG);
	fog_enabled = TRUE;

	FW_GL_POP_MATRIX();
}


/******************************************************************************
 *
 * Background, TextureBackground stuff 
 *
 ******************************************************************************/

/* save a Background vertex into the __points and __colours arrays */
void saveBGVert (float *colptr, float *pt,
		int *vertexno, float *col, double dist,
		double x, double y, double z) {
		/* save the colour */
		memcpy (&colptr[*vertexno*3], col, sizeof(float)*3);

		/* and, save the vertex info */
		pt[*vertexno*3+0] = (float)x*dist;
		pt[*vertexno*3+1] = (float)y*dist;
		pt[*vertexno*3+2] = (float)z*dist;

		(*vertexno)++;
}

/* the background centre follows our position, so, move it! */
void moveBackgroundCentre () {
	GLdouble mod[16];
	GLdouble proj[16];
	GLdouble unit[16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	GLdouble x,y,z;
	GLdouble x1,y1,z1;
	GLdouble sx, sy, sz;

	/* glPushAttrib(GL_LIGHTING_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);  */
	glShadeModel(GL_SMOOTH);
	FW_GL_PUSH_MATRIX();
	fwGetDoublev(GL_MODELVIEW_MATRIX, mod);
	fwGetDoublev(GL_PROJECTION_MATRIX, proj);
	/* Get origin */
	gluUnProject(0.0f,0.0f,0.0f,mod,proj,viewport,&x,&y,&z);
	FW_GL_TRANSLATE_D(x,y,z);

	LIGHTING_OFF

	gluUnProject(0.0f,0.0f,0.0f,mod,unit,viewport,&x,&y,&z);
	/* Get scale */
	gluProject(x+1,y,z,mod,unit,viewport,&x1,&y1,&z1);
	sx = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	gluProject(x,y+1,z,mod,unit,viewport,&x1,&y1,&z1);
	sy = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );
	gluProject(x,y,z+1,mod,unit,viewport,&x1,&y1,&z1);
	sz = 1/sqrt( x1*x1 + y1*y1 + z1*z1*4 );

	/* Undo the translation and scale effects */
	FW_GL_SCALE_D(sx,sy,sz);
}

void recalculateBackgroundVectors(struct X3D_Background *node) {
	struct SFColor *c1,*c2;
	int hdiv = 20;			/* number of horizontal strips allowed */
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

	/* We draw spheres, one for the sky, one for the ground - outsideRadius and insideRadius */
	double outsideRadius =  DEFAULT_FARPLANE* 0.750;
	double insideRadius = DEFAULT_FARPLANE * 0.50;

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
                	FREE_IF_NZ (node->__points);
                	FREE_IF_NZ (node->__colours);
                	node->__quadcount = 0;
        	} else {
                	tbnode->_ichange = tbnode->_change; /* mimic MARK_NODE_COMPILED */

                	/* do we have an old background to destroy? */
                	FREE_IF_NZ (tbnode->__points);
                	FREE_IF_NZ (tbnode->__colours);
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

	/* now, MALLOC space for new arrays  - 3 points per vertex, 4 per quad. */
	newPoints = MALLOC (sizeof (GLfloat) * estq * 3 * 4);
	newColors = MALLOC (sizeof (GLfloat) * estq * 3 * 4);

	if(skyColCt == 1) {
		c1 = &skyCol[0];
		va1 = 0;
		va2 = PI/2;

		for(v=0; v<2; v++) {
			for(h=0; h<hdiv; h++) {
				ha1 = h * PI*2 / hdiv;
				ha2 = (h+1) * PI*2 / hdiv;
				saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius,
				  sin(va2)*cos(ha1), cos(va2), sin(va2)*sin(ha1));
				saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius,
				  sin(va2)*cos(ha2), cos(va2), sin(va2)*sin(ha2));
				saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius,
				  sin(va1)*cos(ha2), cos(va1), sin(va1)*sin(ha2));
				saveBGVert (newColors, newPoints, &actq,&c1->c[0],outsideRadius,
				  sin(va1)*cos(ha1), cos(va1), sin(va1) * sin(ha1));
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
				saveBGVert(newColors,newPoints, &actq,&c2->c[0],outsideRadius,
				  sin(va2)*cos(ha1), cos(va2), sin(va2) * sin(ha1));
				saveBGVert(newColors,newPoints, &actq,&c2->c[0],outsideRadius,
				  sin(va2)*cos(ha2), cos(va2), sin(va2) * sin(ha2));
				saveBGVert(newColors,newPoints, &actq,&c1->c[0],outsideRadius,
				  sin(va1)*cos(ha2), cos(va1), sin(va1) * sin(ha2));
				saveBGVert(newColors,newPoints, &actq,&c1->c[0],outsideRadius,
				  sin(va1) * cos(ha1), cos(va1), sin(va1) * sin(ha1));
			}
			va1 = va2;
		}

		/* now, the spec states: "If the last skyAngle is less than pi, then the
		  colour band between the last skyAngle and the nadir is clamped to the last skyColor." */
		if (va2 < (PI-0.01)) {
			for(h=0; h<hdiv; h++) {
				ha1 = h * PI*2 / hdiv;
				ha2 = (h+1) * PI*2 / hdiv;
				saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius,
				  sin(PI) * cos(ha1), cos(PI), sin(PI) * sin(ha1));
				saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius,
				  sin(PI) * cos(ha2), cos(PI), sin(PI) * sin(ha2));
				saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius,
				  sin(va2) * cos(ha2), cos(va2), sin(va2) * sin(ha2));
				saveBGVert(newColors,newPoints,&actq,&c2->c[0],outsideRadius,
				  sin(va2) * cos(ha1), cos(va2), sin(va2) * sin(ha1));
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

				saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius,
				  sin(PI) * cos(ha1), cos(PI), sin(PI) * sin(ha1));
				saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius,
				  sin(PI) * cos(ha2), cos(PI), sin(PI) * sin(ha2));
				saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius,
				  sin(PI/2) * cos(ha2), cos(PI/2), sin(PI/2) * sin(ha2));
				saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius,
				  sin(PI/2) * cos(ha1), cos(PI/2), sin(PI/2) * sin(ha1));
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

					saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius,
					  sin(va1)*cos(ha1), cos(va1), sin(va1)*sin(ha1));
					saveBGVert(newColors,newPoints,&actq,&c1->c[0],insideRadius,
					  sin(va1)*cos(ha2), cos(va1), sin(va1)*sin(ha2));
					saveBGVert(newColors,newPoints,&actq,&c2->c[0],insideRadius,
					  sin(va2)*cos(ha2), cos(va2), sin(va2)*sin(ha2));
					saveBGVert(newColors,newPoints,&actq,&c2->c[0],insideRadius,
					  sin(va2) * cos(ha1), cos(va2), sin(va2)*sin(ha1));
				}
				va1 = va2;
			}
		}
	}

	/* We have guessed at the quad count; lets make sure
	 * we record what we have. */
	if (actq > (estq*4)) {
		printf ("Background quadcount error, %d > %d\n",
				actq,estq);
		actq = 0;
	}

	/* save changes */
	if (node->_nodeType == NODE_Background) {
		MARK_NODE_COMPILED

		/* do we have an old background to destroy? */
		FREE_IF_NZ (node->__points);
		FREE_IF_NZ (node->__colours);
		node->__points = newPoints;
		node->__colours = newColors;
		node->__quadcount = actq;

	} else {
		tbnode->_ichange = tbnode->_change; /* mimic MARK_NODE_COMPILED */
		/* do we have an old background to destroy? */
		FREE_IF_NZ (tbnode->__points);
		FREE_IF_NZ (tbnode->__colours);
		tbnode->__points = newPoints;
		tbnode->__colours = newColors;
		tbnode->__quadcount = actq;
	}
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

	/* is fog enabled? if so, disable it right now */
	if (fog_enabled ==TRUE) glDisable (GL_FOG);

	/* Cannot start_list() because of moving center, so we do our own list later */
	moveBackgroundCentre();

	if (forceBackgroundRecompile || NODE_NEEDS_COMPILING) {
		recalculateBackgroundVectors(node);
		forceBackgroundRecompile = FALSE;
	}

	/* we have a sphere (maybe one and a half, as the sky and ground are different) so scale it up so that
	   all geometry fits within the spheres */
	FW_GL_SCALE_D (backgroundPlane, backgroundPlane, backgroundPlane);

	/* now, display the lists */
	glVertexPointer (3,GL_FLOAT,0,(GLfloat *)node->__points);
	glColorPointer(3, GL_FLOAT, 0, (GLfloat *)node->__colours);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	FW_GL_DRAWARRAYS (GL_QUADS, 0, node->__quadcount);

	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);


	/* now, for the textures, if they exist */
	if (((node->backUrl).n>0) ||
			((node->frontUrl).n>0) ||
			((node->leftUrl).n>0) ||
			((node->rightUrl).n>0) ||
			((node->topUrl).n>0) ||
			((node->bottomUrl).n>0)) {

        	glEnable(GL_TEXTURE_2D);
        	glColor3d(1.0,1.0,1.0);
        	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        	glVertexPointer (3,GL_FLOAT,0,BackgroundVert);
        	glNormalPointer (GL_FLOAT,0,Backnorms);
        	glTexCoordPointer (2,GL_FLOAT,0,Backtex);

		loadBackgroundTextures(node);

        	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	}
	FW_GL_POP_MATRIX();

	/* is fog enabled? if so, disable it right now */
	if (fog_enabled ==TRUE) glEnable (GL_FOG);

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
	if (fog_enabled ==TRUE) glDisable (GL_FOG);

	/* Cannot start_list() because of moving center, so we do our own list later */
	moveBackgroundCentre();

	if  NODE_NEEDS_COMPILING
		/* recalculateBackgroundVectors will determine exact node type */
		recalculateBackgroundVectors((struct X3D_Background *)node);	

	/* we have a sphere (maybe one and a half, as the sky and ground are different) so scale it up so that
	   all geometry fits within the spheres */
	FW_GL_SCALE_D (backgroundPlane, backgroundPlane, backgroundPlane);


	/* now, display the lists */
	glVertexPointer (3,GL_FLOAT,0,(GLfloat *)node->__points);
	glColorPointer(3, GL_FLOAT, 0, (GLfloat *)node->__colours);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	FW_GL_DRAWARRAYS (GL_QUADS, 0, node->__quadcount);

	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	/* now, for the textures, if they exist */
	if ((node->backTexture !=0) ||
			(node->frontTexture !=0) ||
			(node->leftTexture !=0) ||
			(node->rightTexture !=0) ||
			(node->topTexture !=0) ||
			(node->bottomTexture !=0)) {

		loadTextureBackgroundTextures(node);
        	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	}

	/* pushes are done in moveBackgroundCentre */
	FW_GL_POP_MATRIX();

	/* is fog enabled? if so, disable it right now */
	if (fog_enabled ==TRUE) glEnable (GL_FOG);

}
