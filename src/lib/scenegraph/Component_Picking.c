/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Picking.c,v 1.2 2010/10/10 20:16:54 dug9 Exp $

X3D Picking Component

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

#ifdef DJTRACK_PICKSENSORS

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"

#include "../world_script/fieldSet.h"
#include "../x3d_parser/Bindable.h"
#include "Collision.h"
#include "quaternion.h"
#include "Viewer.h"
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../opengl/OpenGL_Utils.h"
#include "../input/EAIHelpers.h"	/* for newASCIIString() */

#include "Polyrep.h"
#include "LinearAlgebra.h"
#include "Component_Picking.h"
#include "Children.h"

/* see specifications section 38. Picking Sensor Component */


struct PickStruct {
	void *	tonode;
	void (*interpptr)(void *);
	GLDOUBLE picksensor2world[16],world2picksensor[16];
};


/* DJTRACK_PICKSENSORS */
struct PickStruct *PickSensors = NULL;
int num_PickSensors = 0;
int curr_PickSensor = 0;
int active_PickSensors = FALSE;

GLDOUBLE viewpoint2world[16];

/* The PickSensors are used via functions, ie the actual data structures should not be exposed outside this file*/

/* call save_viewpoint2world in mainloop at the scengraph root level,
   outside of any render_hier call and after initializing viewpoint for the frame */
void save_viewpoint2world()
{
	
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, viewpoint2world); 

}
char strprintbits[33];
char *printbits(unsigned short ii)
{
	unsigned short i,j;
	for(i=0;i<16;i++)
	{
		j = (1 << i) & ii;
		if(j) strprintbits[16-i-1] = '1';
		else strprintbits[16-i-1] = '0';
	}
	strprintbits[16] = 0;
	return strprintbits;
}
void pick_PointPickSensor (struct X3D_PointPickSensor *node) { 
 
	int i, index;

	if(!((node->enabled))) return; 
	//COMPILE_IF_REQUIRED 
 
	/* find which one we are in the picksensor table */
	index = -1;
	for(i=0;i<num_PickSensors;i++)
	{
		if(PickSensors[i].tonode == node)
		{
			index = i;
			break;
		}
	}
	if( index == -1 )
	{
		//should we add it here?
		//add to table
		//set VF_PickingSensor flag
	}
	if(index > -1)
	{
		/* store picksensor2world transform */
		GLDOUBLE picksensor2viewpoint[16];
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, picksensor2viewpoint); 
		
		matmultiply(PickSensors[i].picksensor2world,picksensor2viewpoint,viewpoint2world);
		//matinverse(PickSensors[i].world2picksensor,PickSensors[i].picksensor2world);

 		/* loop through target nodes and flag them as targets for the next pass*/
		for(i=0;i<node->pickTarget.n;i++)
		{
			struct X3D_Node *t = (struct X3D_Node *)node->pickTarget.p[i];
			if( t ) /* can nodes be deleted */
			{
				unsigned short isIn, pg, pa;
				pg = VF_inPickableGroup;
				//printf("flag before %s\n",printbits((unsigned short)t->_renderFlags)); //%d %o",t->_renderFlags,t->_renderFlags);
				t->_renderFlags = t->_renderFlags | pg; //& (0xFFFF^VF_inPickableGroup);
				//printf("flag  after %s\n",printbits((unsigned short)t->_renderFlags)); //%d %o VF %d %o ",t->_renderFlags,t->_renderFlags,pg,pg);
				//printf("VF_inPickab %s\n",printbits(pg));
				//pa = (0xFFFF^VF_inPickableGroup);
				//printf("ffff ^ pg   %s\n",printbits(pa));
				//pa = (((unsigned short)node->_renderFlags) & pa);
				//printf("flags&pa    %s\n",printbits(pa));
				//isIn = t->_renderFlags & pg;
				//printf(" isIn=%d\n",isIn);
			}
		}
	}

	/* we wont know if we got a hit until after we visit all the pickTargets */
	//(node->__hit) = 1; 
	//(node->isActive) = 1;
 
} 

void do_PickSensorTickDUMMY(void *ptr) {
}
void do_PickingSensorTick ( void *ptr, int nhits ) {
	struct X3D_PointPickSensor *node = (struct X3D_PointPickSensor *)ptr;
	//UNUSED(over);

	/* if not enabled, do nothing */
	if (!node) 
		return;
	//if (node->__oldEnabled != node->enabled) {
	//	node->__oldEnabled = node->enabled;
	//	MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_PointPickSensor, enabled));
	//}
	if (!node->enabled) 
		return;

	/* only do something if there were some hits */
	if (!nhits) return;

	/* set isActive true */
	node->isActive=TRUE;
	MARK_EVENT (ptr, offsetof (struct X3D_PointPickSensor, isActive));

	//if (node->autoOffset) {
	//	memcpy ((void *) &node->offset,
	//		(void *) &node->rotation_changed,
	//		sizeof (struct SFRotation));
	//}
}
int nPickedObjects;
void do_pickSensors()
{
	/* called from mainloop > render_pre(), after you have a table of picksensors and pick results. 
	Loop through them, updating each picksensor 
	*/
	int i;
	for(i=0;i<nPickedObjects;i++)
	{
		//do_PickingSensorTick ( node, hits, nhits); // void *ptr, int nhits );
	}
}
/* DJTRACK_PICKSENSORS - modelled on prep_Group above */
/* prep_PickableGroup - we need this so that distance (and, thus, distance sorting) works for PickableGroups */
void prep_PickableGroup (struct X3D_Group *node) {
	/* printf("%s:%d prep_PickableGroup\n",__FILE__,__LINE__); */
	RECORD_DISTANCE
/*
printf ("prep_PickableGroup %p (root %p), flags %x children %d ",node,rootNode,node->_renderFlags,node->children.n);
if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) printf ("VF_Viewpoint ");
if ((node->_renderFlags & VF_Geom) == VF_Geom) printf ("VF_Geom ");
if ((node->_renderFlags & VF_localLight) == VF_localLight) printf ("VF_localLight ");
if ((node->_renderFlags & VF_Sensitive) == VF_Sensitive) printf ("VF_Sensitive ");
if ((node->_renderFlags & VF_Blend) == VF_Blend) printf ("VF_Blend ");
if ((node->_renderFlags & VF_Proximity) == VF_Proximity) printf ("VF_Proximity ");
if ((node->_renderFlags & VF_Collision) == VF_Collision) printf ("VF_Collision ");
if ((node->_renderFlags & VF_globalLight) == VF_globalLight) printf ("VF_globalLight ");
if ((node->_renderFlags & VF_hasVisibleChildren) == VF_hasVisibleChildren) printf ("VF_hasVisibleChildren ");
if ((node->_renderFlags & VF_shouldSortChildren) == VF_shouldSortChildren) printf ("VF_shouldSortChildren ");
if ((node->_renderFlags & VF_inPickableGroup) == VF_inPickableGroup) printf ("VF_inPickableGroup ");
printf ("\n");
*/
}


/* DJTRACK_PICKSENSORS - modelled on child_Group above */
void child_PickableGroup (struct X3D_Group *node) {
	CHILDREN_COUNT
	LOCAL_LIGHT_SAVE
/*
printf ("chldGroup %p (root %p), flags %x children %d ",node,rootNode,node->_renderFlags,node->children.n);
if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) printf ("VF_Viewpoint ");
if ((node->_renderFlags & VF_Geom) == VF_Geom) printf ("VF_Geom ");
if ((node->_renderFlags & VF_localLight) == VF_localLight) printf ("VF_localLight ");
if ((node->_renderFlags & VF_Sensitive) == VF_Sensitive) printf ("VF_Sensitive ");
if ((node->_renderFlags & VF_Blend) == VF_Blend) printf ("VF_Blend ");
if ((node->_renderFlags & VF_Proximity) == VF_Proximity) printf ("VF_Proximity ");
if ((node->_renderFlags & VF_Collision) == VF_Collision) printf ("VF_Collision ");
if ((node->_renderFlags & VF_globalLight) == VF_globalLight) printf ("VF_globalLight ");
if ((node->_renderFlags & VF_hasVisibleChildren) == VF_hasVisibleChildren) printf ("VF_hasVisibleChildren ");
if ((node->_renderFlags & VF_shouldSortChildren) == VF_shouldSortChildren) printf ("VF_shouldSortChildren ");
if ((node->_renderFlags & VF_inPickableGroup) == VF_inPickableGroup) printf ("VF_inPickableGroup ");
printf ("\n");
*/
	RETURN_FROM_CHILD_IF_NOT_FOR_ME
	/* printf("%s:%d child_PickableGroup\n",__FILE__,__LINE__); */

	 if (1==0) {
		int x;
		struct X3D_Node *xx;

		printf ("child_PickableGroup, this %p rf %x isProto %d\n",node,node->_renderFlags, node->FreeWRL__protoDef);
		printf ("	..., render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
			render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); 

		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->_sortedChildren.p[x]);
			printf ("	ch %p type %s dist %f\n",node->_sortedChildren.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	}

	/* do we have a DirectionalLight for a child? */
	LOCAL_LIGHT_CHILDREN(node->_sortedChildren);

	/* printf ("chld_PickableGroup, for %u, protodef %d and FreeWRL_PROTOInterfaceNodes.n %d\n",
		node, node->FreeWRL__protoDef, node->FreeWRL_PROTOInterfaceNodes.n); */
	/* now, just render the non-directionalLight children */
	if ((node->FreeWRL__protoDef!=INT_ID_UNDEFINED) && render_geom) {
		(node->children).n = 1;
		normalChildren(node->children);
		(node->children).n = nc;
	} else {
		normalChildren(node->_sortedChildren);
	}

	LOCAL_LIGHT_OFF
}


void chainUpPickableTree(struct X3D_Node *shapeNode ,struct X3D_Node *chained , int status) {

	int possibleStatus ;
	/* status: -1 = indeterminate , 0 = False , 1 = True */
	/* Valid status transitions:
		-1 -> 0
		-1 -> 1

		1 -> 0
	*/

	/* Special cases:
		A branch/leaf has more than one parent:
			Consider the shape unpickable if any parent has pickable as false
		A pickable group is inside a pickable group
			Consider the shape unpickable if any pickable group in a chain has pickable as false
		Pathalogical cases: (According to JAS Jul 2010 these will never happen)
			Chain A contains a node pointing to Chain B, and B contains a node pointing to Chain A
			Similarly A --> B , B --> C , C --> A and  ... --> N ... --> N
			Similarly A --> A but closer to the leaves, ie a self-loop
	*/
	/* Invalid status transitions:
		Cannot become indeterminate
		0 -> -1
		1 -> -1 

		Take care of the non-Pathalogical special cases
		0 -> 1
	*/
	/* This will happen if you complete parent X and are going up parent Y */
	if (status == 0) return ;

	possibleStatus = status ;
	if (chained->_nodeType == NODE_PickableGroup) {
		/* OK, we have a PickableGroup and this might change the status flag */

		/* Sorry, I do not have the shorthand for this ;( */
		struct X3D_PickableGroup *pickNode ;
		pickNode = (struct X3D_PickableGroup *) chained ;
		possibleStatus = pickNode->pickable ;
		
		if (status == -1) {
			/* dump_scene (stdout, 0, chained) ; */
			status = possibleStatus ;
			if(possibleStatus == 0) {
				shapeNode->_renderFlags = shapeNode->_renderFlags & (0xFFFF^VF_inPickableGroup);
				/* No point in going further up this tree, we have gone from -1 -> 0 */
			} else {
				shapeNode->_renderFlags = shapeNode->_renderFlags | VF_inPickableGroup;
				if(0 != chained->_nparents) {
					int i;
					for(i=0;i<chained->_nparents;i++) {
						chainUpPickableTree(shapeNode, chained->_parents[i], status) ;
					}
				}
			}
		} else {
			/* The current status is 1 */
			if(possibleStatus == 0) {
				shapeNode->_renderFlags = shapeNode->_renderFlags & (0xFFFF^VF_inPickableGroup);
				/* No point in going further up this tree, we have gone from 1 -> 0 */
			} else {
				/* Still 1, so no change to _renderFlags */
				if(0 != chained->_nparents) {
					int i;
					for(i=0;i<chained->_nparents;i++) {
						chainUpPickableTree(shapeNode, chained->_parents[i], status) ;
					}
				}
			}
		}
	} else {
		if(0 != chained->_nparents) {
			int i;
			for(i=0;i<chained->_nparents;i++) {
				chainUpPickableTree(shapeNode, chained->_parents[i], status) ;
			}
		}
	}
	return ;
}






/* DJTRACK_PICKSENSORS */
void activate_picksensors() { active_PickSensors = TRUE ; }
void deactivate_picksensors() { active_PickSensors = FALSE ; }
int enabled_picksensors() 
{
  int i;
  int someEnabled = FALSE;
  for(i=0;i<num_PickSensors;i++)
	  someEnabled = someEnabled || ((struct X3D_PointPickSensor *)(PickSensors[i].tonode))->enabled;
  return someEnabled;
 }
int  active_picksensors() { return (active_PickSensors && (num_PickSensors > 0)) ; }
void rewind_picksensors() { curr_PickSensor = 0 ; }
void advance_picksensors() { curr_PickSensor++; }
int  more_picksensors() {
	if (active_PickSensors && curr_PickSensor < num_PickSensors) {
		return TRUE ;
	} else {
		return FALSE ;
	}
}

void pick_Sphere (struct X3D_Sphere *node) {

	GLDOUBLE shape2viewpoint[16],viewpoint2world[16],shape2world[16],world2shape[16],shape2picksensor[16], picksensor2shape[16];
	GLDOUBLE *picksensor2world;
	GLDOUBLE radiusSquared;
	int i,j;

	/* this sucker initialized yet? */
	if (node->__points == NULL) return;
	// check before render_hier if(!num_picksensors) return;

	/* get the transformed position of the Sphere, and the scale-corrected radius. */
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, shape2viewpoint);
	matmultiply(shape2world,shape2viewpoint,viewpoint2world);
	matinverse(world2shape,shape2world);

	for(i=0;i<num_PickSensors;i++)
	{
		struct X3D_Node *picksensor = PickSensors[i].tonode;
		picksensor2world = PickSensors[i].picksensor2world;
		matmultiply(shape2picksensor,picksensor2world,world2shape);
		switch (picksensor->_nodeType) {
			case NODE_PointPickSensor:  
			{
				struct X3D_PointPickSensor *pps = (struct X3D_PointPickSensor*)picksensor;
				struct X3D_PointSet *points = (struct X3D_PointSet *)pps->pickingGeometry;
				for(j=0;j<points->attrib.n;j++)
				{
					struct point_XYZ pickpoint;
					transform(&pickpoint,(struct point_XYZ *)&points->attrib.p[j],picksensor2shape);
					radiusSquared = vecdot(&pickpoint,&pickpoint);
					if( radiusSquared < node->radius )
					{
						printf("bingo - we have a hit ");
						/* according to specs, if we should report the intersection point then transform it into picksensor space */
						transform(&pickpoint,&pickpoint,shape2picksensor);
						printf(" at %lf %lf %lf in picksensor space\n",pickpoint.x,pickpoint.y,pickpoint.z);
					}

				}
				break;
			}
		}
	}
}



/* DJTRACK_PICKSENSORS */
void add_picksensor(struct X3D_Node * node) {
	void (*myp)(void *);
	int clocktype;
	int count;
	
	if (node == 0) {
		printf ("error in add_first; somehow the node datastructure is zero \n");
		return;
	}

	clocktype = node->_nodeType;
	/* printf ("add_picksensor for %s\n",stringNodeType(clocktype)); */

	if (NODE_PointPickSensor == clocktype) {
		printf ("add_picksensor for %s\n",stringNodeType(clocktype));
		myp =  do_PickSensorTickDUMMY;
	} else {
		/* printf ("this is not a type we need to add_first for %s\n",stringNodeType(clocktype)); */
		return;
	}
	PickSensors = (struct PickStruct *)REALLOC(PickSensors,sizeof (struct PickStruct) * (num_PickSensors+1));
	if (PickSensors == 0) {
		printf ("can not allocate memory for add_first call\n");
		num_PickSensors = 0;
	}

	/* does this event exist? */
	for (count=0; count <num_PickSensors; count ++) {
		if (PickSensors[count].tonode == node) {
			/* printf ("add_first, already have %d\n",node); */
			return;
		}	
	}

	/* now, put the function pointer and data pointer into the structure entry */
	PickSensors[num_PickSensors].interpptr = myp;
	PickSensors[num_PickSensors].tonode = node;

	num_PickSensors++;
}

/* DJTRACK_PICKSENSORS */
struct X3D_Node* get_picksensor() {
	int this_PickSensor = curr_PickSensor ;
	if ( active_PickSensors && this_PickSensor < num_PickSensors) {
		return PickSensors[this_PickSensor].tonode ;
	} else {
		return (struct X3D_Node *) NULL ;
	}
}
#else // DJTRACK_PICKSENSORS
/* PICKSENSOR stubs */
void pick_Sphere (struct X3D_Sphere *node) {}
void pick_PointPickSensor (struct X3D_PointPickSensor *node) {}
void child_PickableGroup (struct X3D_Group *node) {}
void prep_PickableGroup (struct X3D_Group *node) {}
void add_picksensor(struct X3D_Node * node) {}

#endif // DJTRACK_PICKSENSORS