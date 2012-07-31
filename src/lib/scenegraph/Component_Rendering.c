/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Rendering.c,v 1.33 2012/07/31 15:19:39 crc_canada Exp $

X3D Rendering Component

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
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../opengl/OpenGL_Utils.h"
#include "Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"
#include "../scenegraph/Polyrep.h"

/* find a bounding box that fits the coord structure. save it in the common-node area for extents.*/
static void findExtentInCoord (struct X3D_Node *node, int count, struct SFVec3f* coord) {
	int i;

	INITIALIZE_EXTENT

	if (!coord) return;

	for (i=0; i<count; i++) {
		if (coord->c[0] > node->EXTENT_MAX_X) node->EXTENT_MAX_X = coord->c[0];
		if (coord->c[0] < node->EXTENT_MIN_X) node->EXTENT_MIN_X = coord->c[0];
		if (coord->c[1] > node->EXTENT_MAX_Y) node->EXTENT_MAX_Y = coord->c[1];
		if (coord->c[1] < node->EXTENT_MIN_Y) node->EXTENT_MIN_Y = coord->c[1];
		if (coord->c[2] > node->EXTENT_MAX_Z) node->EXTENT_MAX_Z = coord->c[2];
		if (coord->c[2] < node->EXTENT_MIN_Z) node->EXTENT_MIN_Z = coord->c[2];
		coord++;
	}
	/* printf ("extents %f %f, %f %f, %f %f\n",node->EXTENT_MIN_X, node->EXTENT_MAX_X,
	  node->EXTENT_MIN_Y, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z, node->EXTENT_MAX_Z); */
}

void render_IndexedTriangleFanSet (struct X3D_IndexedTriangleFanSet *node) {
                COMPILE_POLY_IF_REQUIRED(node->coord, node->color, node->normal, node->texCoord)
		CULL_FACE(node->solid)
		render_polyrep(node);
}

void render_IndexedTriangleSet (struct X3D_IndexedTriangleSet *node) {
                COMPILE_POLY_IF_REQUIRED(node->coord, node->color, node->normal, node->texCoord)
		CULL_FACE(node->solid)
		render_polyrep(node);

}

void render_IndexedTriangleStripSet (struct X3D_IndexedTriangleStripSet *node) {
                COMPILE_POLY_IF_REQUIRED( node->coord, node->color, node->normal, NULL)
		CULL_FACE(node->solid)
		render_polyrep(node);
}

void render_TriangleFanSet (struct X3D_TriangleFanSet *node) {
                COMPILE_POLY_IF_REQUIRED (node->coord, node->color, node->normal, node->texCoord)
		CULL_FACE(node->solid)
		render_polyrep(node);
}

void render_TriangleStripSet (struct X3D_TriangleStripSet *node) {
                COMPILE_POLY_IF_REQUIRED(node->coord, node->color, node->normal, node->texCoord)
		CULL_FACE(node->solid)
		render_polyrep(node);
}

void render_TriangleSet (struct X3D_TriangleSet *node) {
                COMPILE_POLY_IF_REQUIRED(node->coord, node->color, node->normal, node->texCoord)
		CULL_FACE(node->solid)
		render_polyrep(node);
}



void compile_IndexedLineSet (struct X3D_IndexedLineSet *node) {
	int i;		/* temporary */
	struct SFVec3f *points;
	struct SFVec3f *newpoints;
	struct SFVec3f *oldpoint;
	struct SFColorRGBA *newcolors;
	struct SFColorRGBA *oldcolor;
	int npoints;
	int maxCoordFound;		/* for bounds checking				*/
	struct X3D_Color *cc;
	int nSegments;			/* how many individual lines in this shape 	*/
	int curSeg;			/* for colours, !cpv, this is the color index 	*/
	int nVertices;			/* how many vertices in the streamed set	*/
	int segLength;			/* temporary					*/
	ushort *vertCountPtr;		/* temporary, for vertexCount filling		*/
	ushort **indxStartPtr;	/* temporary, for creating pointer to index arr */

	ushort * pt;
	int vtc;			/* temp counter - "vertex count"		*/
	int curcolor;			/* temp for colorIndexing.			*/
	int * colorIndInt;			/* used for streaming colors			*/
	ushort * colorIndShort;			/* used for streaming colors			*/

	/* believe it or not - material emissiveColor can affect us... */
	GLfloat defcolorRGBA[] = {1.0f, 1.0f, 1.0f,1.0f};

    /* we either use the (sizeof (int)) indices passed in from user, or calculated ushort one */
	colorIndInt = NULL;
	colorIndShort = NULL;

	MARK_NODE_COMPILED
	nSegments = 0;
	node->__segCount = 0;

	/* ok, what we do is this. Although this is Indexed, colours and vertices can have
	   different indexes; so we make them all the same. To do this, we create another
	   index (really simple one - element x contains x...) that we send to the OpenGL
	   calls. 

	   So first, we find out the maximum coordinate requested in the IndexedLineSet,
	   AND, we find out how many line segments there are.
	*/

	if (node->coord) {
		struct Multi_Vec3f *dtmp;
		dtmp = getCoordinate (node->coord, "IndexedLineSet");
		npoints = dtmp->n;
		points = dtmp->p;

		/* find the extents */
		findExtentInCoord(X3D_NODE(node), npoints, points);
	} else {
		return; /* no coordinates - nothing to do */
	}

	if (node->coordIndex.n == 0) return; /* no coord indexes - nothing to do */

	/* sanity check that we have enough coordinates */
	maxCoordFound = -1000;
	nSegments = 1;
	nVertices = 0;
	for (i=0; i<node->coordIndex.n; i++) {
		/* make sure that the coordIndex is greater than -1 */
		if (node->coordIndex.p[i] < -1) {
			ConsoleMessage ("IndexedLineSet - coordIndex less than 0 at %d\n",i);
			return;
		}

		/* count segments; dont bother if the very last number is -1 */
		if (node->coordIndex.p[i] == -1) {
			if (i!=((node->coordIndex.n)-1)) nSegments++;
		} else nVertices++;

		/* try to find the highest coordinate index for bounds checking */
		if (node->coordIndex.p[i] > maxCoordFound) maxCoordFound = node->coordIndex.p[i];
	}
	if (maxCoordFound > npoints) {
		ConsoleMessage ("IndexedLineSet - not enough coordinates - coordindex contains higher index\n");
		return;
	}

	/* so, at this step, we know how many line segments "nSegments" we require (starting at 0)
	   and, what the maximum coordinate is. So, lets create the new index... 
	   create the index for the arrays. Really simple... Used to index
	   into the coords, so, eg, __vertArr is [0,1,2], which means use
	   coordinates 0, 1, and 2 */
	FREE_IF_NZ (node->__vertArr);
	node->__vertArr = MALLOC (ushort *, sizeof(ushort)*(nVertices+1));
	pt = (ushort *)node->__vertArr;

	for (vtc = 0; vtc < nVertices; vtc++) {
		*pt=vtc; pt++; /* ie, index n contains the number n */
	}

    
	/* now, lets go through and; 1) copy old vertices into new vertex array; and 
	   2) create an array of indexes into "__vertArr" for sending to the GL call */


	FREE_IF_NZ (node->__vertIndx);
	node->__vertIndx = MALLOC (ushort **,sizeof(ushort*)*(nSegments));

	FREE_IF_NZ (node->__vertices);
	node->__vertices = MALLOC (struct SFVec3f *, sizeof(struct SFVec3f)*(nVertices+1));

	FREE_IF_NZ (node->__vertexCount);
	node->__vertexCount = MALLOC (ushort *,sizeof(ushort)*(nSegments));


	indxStartPtr = (ushort **)node->__vertIndx;
	newpoints = node->__vertices;
	vertCountPtr = (ushort *) node->__vertexCount;
    
	pt = (ushort *)node->__vertArr;

	vtc=0;
	segLength=0;
	*indxStartPtr = pt; /* first segment starts off at index zero */

	indxStartPtr++;

	for (i=0; i<node->coordIndex.n; i++) {
		/* count segments; dont bother if the very last number is -1 */
		if (node->coordIndex.p[i] == -1) {
			if (i!=((node->coordIndex.n)-1)) {
				/* new segment */
				*indxStartPtr =  pt;
				indxStartPtr++;

				/* record the old segment length */
				*vertCountPtr = segLength;
				segLength=0;
				vertCountPtr ++;
			}
		} else {
			/* new vertex */
			oldpoint = &points[node->coordIndex.p[i]];
			memcpy (newpoints, oldpoint,sizeof(struct SFColor));
			newpoints ++; 
			segLength ++;
			pt ++;
		}
	}

	/* do we have to worry about colours? */
	/* sanity check the colors, if they exist */
	if (node->color) {

        
		/* we resort the color nodes so that we have an RGBA color node per vertex */


		FREE_IF_NZ (node->__colours);
		node->__colours = MALLOC (struct SFColorRGBA *, sizeof(struct SFColorRGBA)*(nVertices+1));

		newcolors = (struct SFColorRGBA *) node->__colours;
			POSSIBLE_PROTO_EXPANSION(struct X3D_Color *, node->color,cc)
               		/* cc = (struct X3D_Color *) node->color; */
               		if ((cc->_nodeType != NODE_Color) && (cc->_nodeType != NODE_ColorRGBA)) {
               	        	ConsoleMessage ("make_IndexedLineSet, color node, expected %d got %d\n", NODE_Color, cc->_nodeType);
			return;
               		}

		/* 4 choices here - we have colorPerVertex, and, possibly, a ColorIndex */

		if (node->colorPerVertex) {
			/* assume for now that we are using the coordIndex for colour selection */
			colorIndInt = node->coordIndex.p; 
			/* so, we have a color per line segment. Lets check this stuff... */
			if ((node->colorIndex.n)>0) {
				if ((node->colorIndex.n) < (node->coordIndex.n)) {
					ConsoleMessage ("IndexedLineSet - expect more colorIndexes to match coords\n");
					return;
				}
                		colorIndInt = node->colorIndex.p; /* use ColorIndex */
			} else {
				colorIndShort = node->__vertArr;
			}
		} else {

			/* so, we have a color per line segment. Lets check this stuff... */
			if ((node->colorIndex.n)>0) {
				if ((node->colorIndex.n) < (nSegments)) {
					ConsoleMessage ("IndexedLineSet - expect more colorIndexes to match coords\n");
					return;
				}
				colorIndInt = node->colorIndex.p; /* use ColorIndex */
			} else {
                /* we are using the simple index for colour selection */
     	           colorIndShort = node->__vertArr;                 
            }
		}


		/* go and match colors with vertices */
		curSeg = 0;
		for (i=0; i<node->coordIndex.n; i++) {
			if (node->coordIndex.p[i] != -1) {
				/* have a vertex, match colour  */
				if (node->colorPerVertex) {
       			             if (colorIndInt != NULL) 
                		        curcolor = colorIndInt[i];
 	                  	     else
                    		    	curcolor = colorIndShort[i];
				} else {
                    			if (colorIndInt != NULL)
                        		curcolor = colorIndInt[curSeg];
                    			else
                        		curcolor = colorIndShort[curSeg];
				}
                //ConsoleMessage ("curSeg %d, i %d, node->coordIndex.p %d curcolor %d\n",curSeg,i,node->coordIndex.p[i], curcolor);
				if ((curcolor < 0) || (curcolor >= cc->color.n)) {
					ConsoleMessage ("IndexedLineSet, colorIndex %d (for vertex %d or segment %d) out of range (0..%d)\n",
						curcolor, i, curSeg, cc->color.n);
					return;
				}


				oldcolor = (struct SFColorRGBA *) &(cc->color.p[curcolor]);



				/* copy the correct color over for this vertex */
				if (cc->_nodeType == NODE_Color) {
					memcpy (newcolors, defcolorRGBA, sizeof (defcolorRGBA));
					memcpy (newcolors, oldcolor,sizeof(struct SFColor));
				} else {
					memcpy (newcolors, oldcolor,sizeof(struct SFColorRGBA));
				}
                //printf ("colout selected %f %f %f %f\n",newcolors->c[0],newcolors->c[1],newcolors->c[2],newcolors->c[3]);
				newcolors ++; 
			} else {
				curSeg++;
			}
		}
	}

	/* finished worrying about colours */

	/* finish this for loop off... */
	*vertCountPtr = segLength;
	node->__segCount = nSegments; /* we passed, so we can render */
}

void render_IndexedLineSet (struct X3D_IndexedLineSet *node) {
#ifdef OLDCODE
OLDCODE	DEFAULT_COLOUR_POINTER
#endif //OLDCODE

    ushort **indxStartPtr;
	ushort *count;
	int i;

#ifdef OLDCODE
OLDCODE	/* is there an emissiveColor here??? */
OLDCODE	GET_COLOUR_POINTER
#endif //OLDCODE

	LIGHTING_OFF
	DISABLE_CULL_FACE
	
	COMPILE_IF_REQUIRED

        setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, node->EXTENT_MAX_Y,
                node->EXTENT_MIN_Y, node->EXTENT_MAX_Z, node->EXTENT_MIN_Z,
                X3D_NODE(node));


	/* If we have segments... */
	if (node->__segCount > 0) {
		FW_GL_ENABLECLIENTSTATE(GL_VERTEX_ARRAY);
		FW_GL_DISABLECLIENTSTATE(GL_NORMAL_ARRAY);
		FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,node->__vertices);

		if (node->__colours) {
			FW_GL_COLOR_POINTER (4,GL_FLOAT,0,node->__colours);
			FW_GL_ENABLECLIENTSTATE(GL_COLOR_ARRAY);
#ifdef OLDCODE
OLDCODE - now with shaders, the line colour will come from the appearance field,
OLDCODE not from a specific shader variable. Yes, an "appearance" field will be present,
OLDCODE even if one does not exist in the VRML/X3D file. 
OLDCODE 		} else {
OLDCODE 			DO_COLOUR_POINTER
#endif //OLDCODE
		}

        indxStartPtr = (ushort **)node->__vertIndx;
        count  = node->__vertexCount;

		for (i=0; i<node->__segCount; i++) {
            // draw. Note the casting of the last param - it is ok, because we tell that
            // we are sending in ushorts; it gets around a compiler warning.
            
            FW_GL_DRAWELEMENTS(GL_LINE_STRIP,count[i],GL_UNSIGNED_SHORT,(int *)indxStartPtr[i]);
		}


		FW_GL_ENABLECLIENTSTATE (GL_NORMAL_ARRAY);
		if (node->__colours) {
		//	FW_GL_DISABLECLIENTSTATE(GL_COLOR_ARRAY);
		}
	}
}


void compile_PointSet (struct X3D_PointSet *node) {
	/* do nothing, except get the extents here */
	MARK_NODE_COMPILED

	if (node->coord) {
		struct Multi_Vec3f *dtmp;
		dtmp = getCoordinate (node->coord, "IndexedLineSet");

		/* find the extents */
		findExtentInCoord(X3D_NODE(node), dtmp->n, dtmp->p);
	}
}


void render_PointSet (struct X3D_PointSet *node) {
	struct SFVec3f *points=0; int npoints=0;
	struct SFColor *colors=0; int ncolors=0;
	struct X3D_Color *cc;

#ifdef OLDCODE
OLDCODE	DEFAULT_COLOUR_POINTER
#endif //OLDCODE

        COMPILE_IF_REQUIRED

        setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, node->EXTENT_MAX_Y,
                node->EXTENT_MIN_Y, node->EXTENT_MAX_Z, node->EXTENT_MIN_Z,
                X3D_NODE(node));

#ifdef OLDCODE
OLDCODE	/* is there an emissiveColor here??? */
OLDCODE	GET_COLOUR_POINTER
#endif //OLDCODE

	if (node->coord) {
		struct Multi_Vec3f *dtmp;
		dtmp = getCoordinate (node->coord, "IndexedLineSet");
		npoints = dtmp->n;
		points = dtmp->p;

		/* find the extents */
		findExtentInCoord(X3D_NODE(node), npoints, points);
	} else {
		return; /* no coordinates - nothing to do */
	}

	if (npoints <=0 ) return; /* nothing to do */
 

       	if (node->color) {
               	/* cc = (struct X3D_Color *) node->color; */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Color *, node->color,cc)
               	if ((cc->_nodeType != NODE_Color) && (cc->_nodeType != NODE_ColorRGBA)) {
               	        ConsoleMessage ("make_PointSet, expected %d got %d\n", NODE_Color, cc->_nodeType);
               	} else {
               	        ncolors = cc->color.n;
			colors = cc->color.p;
               	}
       	}

	if(ncolors && ncolors < npoints) {
		printf ("PointSet has less colors than points - removing color\n");
		ncolors = 0;
	}

	LIGHTING_OFF
	DISABLE_CULL_FACE

	#ifdef RENDERVERBOSE
	printf("PointSet: %d %d\n", npoints, ncolors);
	#endif

	if (ncolors>0) {
		FW_GL_ENABLECLIENTSTATE(GL_COLOR_ARRAY);
                cc = (struct X3D_Color *) node->color;
		/* is this a Color or ColorRGBA color node? */
               	if (cc->_nodeType == NODE_Color) {
			FW_GL_COLOR_POINTER (3,GL_FLOAT,0,(float *)colors);
		} else {
			FW_GL_COLOR_POINTER (4,GL_FLOAT,0,(float *)colors);
		}
#ifdef OLDCODE
OLDCODE - now with shaders, the line colour will come from the appearance field,
OLDCODE not from a specific shader variable. Yes, an "appearance" field will be present,
OLDCODE even if one does not exist in the VRML/X3D file. 
OLDCODE 		} else {
OLDCODE 			DO_COLOUR_POINTER
#endif //OLDCODE
	}


	/* draw the shape */
	FW_GL_DISABLECLIENTSTATE (GL_NORMAL_ARRAY);

	FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(float *)points);
	FW_GL_DRAWARRAYS(GL_POINTS,0,npoints);

	/* put things back to normal */
	FW_GL_ENABLECLIENTSTATE(GL_NORMAL_ARRAY);
	if (ncolors>0) {
		FW_GL_DISABLECLIENTSTATE(GL_COLOR_ARRAY);
	}
}

void render_LineSet (struct X3D_LineSet *node) {

#ifdef OLDCODE
OLDCODE	DEFAULT_COLOUR_POINTER
#endif //OLDCODE

	struct X3D_Color *cc;
	GLvoid **indices;
	GLsizei *count;
	int i;
	struct Multi_Vec3f* points;

#ifdef OLDCODE
OLDCODE	/* is there an emissiveColor here??? */
OLDCODE	GET_COLOUR_POINTER
#endif //OLDCODE

	LIGHTING_OFF
	DISABLE_CULL_FACE

	COMPILE_IF_REQUIRED
	
        setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, node->EXTENT_MAX_Y,
                node->EXTENT_MIN_Y, node->EXTENT_MAX_Z, node->EXTENT_MIN_Z,
                X3D_NODE(node));

	/* now, actually draw array */
	if (node->__segCount > 0) {
		FW_GL_ENABLECLIENTSTATE(GL_VERTEX_ARRAY);
		FW_GL_DISABLECLIENTSTATE(GL_NORMAL_ARRAY);

		if (node->color) {
			FW_GL_ENABLECLIENTSTATE(GL_COLOR_ARRAY);
                	cc = (struct X3D_Color *) node->color;
			/* is this a Color or ColorRGBA color node? */
                	if (cc->_nodeType == NODE_Color) {
				FW_GL_COLOR_POINTER (3,GL_FLOAT,0,(float *)cc->color.p);
			} else {
				FW_GL_COLOR_POINTER (4,GL_FLOAT,0,(float *)cc->color.p);
			}
#ifdef OLDCODE
OLDCODE - now with shaders, the line colour will come from the appearance field,
OLDCODE not from a specific shader variable. Yes, an "appearance" field will be present,
OLDCODE even if one does not exist in the VRML/X3D file. 
OLDCODE 		} else {
OLDCODE 			DO_COLOUR_POINTER
#endif //OLDCODE
		}
		points = getCoordinate(node->coord, "LineSet");

		FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(float *)points->p);

		/* aqua crashes on glMultiDrawElements and LINE_STRIPS */
		indices = node->__vertIndx;
		/* note the cast below - casting an int* to a GLsizei* seems to be ok on 32 and 64 bit systems */
		count  = (GLsizei*) node->vertexCount.p;
		for (i=0; i<node->__segCount; i++) {
			FW_GL_DRAWELEMENTS(GL_LINE_STRIP,count[i],GL_UNSIGNED_INT,indices[i]);
		}

		/* otherwise we could use 
		glMultiDrawElements ( GL_LINE_STRIP, node->vertexCount.p, GL_UNSIGNED_INT,
			node->__vertIndx, node->__segCount);  */
		
		FW_GL_ENABLECLIENTSTATE (GL_NORMAL_ARRAY);
		if (node->color) {
			FW_GL_DISABLECLIENTSTATE(GL_COLOR_ARRAY);
		}
	}
}


void compile_LineSet (struct X3D_LineSet *node) {
	int vtc;		/* which vertexCount[] we should be using for this line segment */
	int c;			/* temp variable */
	struct SFVec3f *coord=0; int ncoord;
	struct SFColor *color=0; int ncolor=0;
	int *vertexC; int nvertexc;
	int totVertexRequired;

	struct X3D_Color *cc;
	GLuint *pt;
	uintptr_t *vpt;

	MARK_NODE_COMPILED
	node->__segCount = 0; /* assume this for now */


	nvertexc = (node->vertexCount).n; vertexC = (node->vertexCount).p;
	if (nvertexc==0) return;
	totVertexRequired = 0;


	/* sanity check vertex counts */
	for  (c=0; c<nvertexc; c++) {
		totVertexRequired += vertexC[c];
		if (vertexC[c]<2) {
			ConsoleMessage ("make_LineSet, we have a vertexCount of %d, must be >=2,\n",vertexC[c]);
			return;
		}
	}

	if (node->coord) {
		struct Multi_Vec3f *dtmp;
		dtmp = getCoordinate (node->coord, "IndexedLineSet");
		ncoord = dtmp->n;
		coord = dtmp->p;

		/* find the extents */
		findExtentInCoord(X3D_NODE(node), ncoord, coord);
	} else {
		return; /* no coordinates - nothing to do */
	}

	/* check that we have enough vertexes */
	if (totVertexRequired > ncoord) {
		ConsoleMessage ("make_LineSet, not enough points for vertexCount (vertices:%d points:%d)\n",
			totVertexRequired, ncoord);
		return;
	}
 
       	if (node->color) {
               	/* cc = (struct X3D_Color *) node->color; */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Color *, node->color,cc)
               	if ((cc->_nodeType != NODE_Color) && (cc->_nodeType != NODE_ColorRGBA)) {
               	        ConsoleMessage ("make_LineSet, expected %d got %d\n", NODE_Color, cc->_nodeType);
               	} else {
               	        ncolor = cc->color.n;
		color = cc->color.p;
               	}
		/* check that we have enough verticies for the Colors */
		if (totVertexRequired > ncolor) {
			ConsoleMessage ("make_LineSet, not enough colors for vertexCount (vertices:%d colors:%d)\n",
				totVertexRequired, ncolor);
			return;
		}
       	}

	/* create the index for the arrays. Really simple... Used to index
	   into the coords, so, eg, __vertArr is [0,1,2], which means use
	   coordinates 0, 1, and 2 */
	FREE_IF_NZ (node->__vertArr);
	node->__vertArr = MALLOC (GLuint *, sizeof(GLuint)*(ncoord));
	pt = (GLuint *)node->__vertArr;
	for (vtc = 0; vtc < ncoord; vtc++) {
		*pt=vtc; pt++; /* ie, index n contains the number n */
	}

	/* create the index for each line segment. What happens here is
	   that we create an array of pointers; each pointer points into
	   the __vertArr array - this gives a starting index for each line
	   segment The LENGTH of each segment (good question) comes from the
	   vertexCount parameter of the LineSet node */
	FREE_IF_NZ (node->__vertIndx);
	node->__vertIndx = MALLOC (uintptr_t *, sizeof(uintptr_t)*(nvertexc));
	c = 0;
	pt = (GLuint *)node->__vertArr;
	vpt = (uintptr_t*) node->__vertIndx;
	for (vtc=0; vtc<nvertexc; vtc++) {
		*vpt =  (uintptr_t) pt;
		vpt++;
		pt += vertexC[vtc];
	}

	/* if we made it this far, we are ok tell the rendering engine that we are ok */
	node->__segCount = nvertexc;
}

