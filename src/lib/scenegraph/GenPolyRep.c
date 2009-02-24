/*
=INSERT_TEMPLATE_HERE=

$Id: GenPolyRep.c,v 1.5 2009/02/24 20:57:50 crc_canada Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"

#include "LinearAlgebra.h"
#include "Polyrep.h"

/*****************************************
 *
 * Complex Geometry; ElevationGrid, Extrusion, IndexedFaceSet, Text.
 *
 * This code generates a Polyrep structure, that is further
 * streamlined then streamed to OpenGL.
 *
 * Polyreps are streamed in Polyrep.c
 *
 *******************************************/

#define X3DCOMPOSED_STRING(f) ( \
        f == NODE_IndexedFaceSet          ? "IndexedFaceSet" : ( \
        f == NODE_ElevationGrid           ? "ElevationGrid" : ( \
        f == NODE_IndexedTriangleFanSet   ? "IndexedTriangleFanSet" : ( \
        f == NODE_IndexedTriangleSet      ? "IndexedTriangleSet" : ( \
        f == NODE_IndexedTriangleStripSet ? "IndexedTriangleStripSet" : ( \
        f == NODE_TriangleFanSet          ? "TriangleFanSet" : ( \
        f == NODE_TriangleStripSet        ? "TriangleStripSet" : ( \
        f == NODE_TriangleSet             ? "TriangleSet" : "unknown X3DComposedGeometry Node"))))))))


extern void Elev_Tri (int vertex_ind,int this_face,int A,int D,int E,int NONORMALS,struct X3D_PolyRep *this_Elev,struct point_XYZ *facenormals,int *pointfaces,int ccw);
extern int count_IFS_faces(int cin, struct X3D_IndexedFaceSet *this_IFS);
extern void verify_global_IFS_Coords(int max);
extern void IFS_check_normal(struct point_XYZ *facenormals,int this_face,struct SFColor *points,int base,struct X3D_IndexedFaceSet *this_IFS,int ccw);
extern void Extru_check_normal(struct point_XYZ *facenormals,int this_face,int dire,struct X3D_PolyRep *rep_,int ccw);





/* calculate how many triangles are required for IndexedTriangleFanSet and 
	IndexedTriangleStripSets */
int returnIndexedFanStripIndexSize (struct Multi_Int32 index ) {
	int IndexSize;
	int xx, yy,zz;
	IndexSize = 0;
	xx = 0;
	yy = 0;
	zz = 0;

	for (xx=0; xx<index.n; xx++) {
		/* printf ("looking at index %d, is %d of %d\n",xx,index.p[xx],index.n); */
		if ((index.p[xx] <=-1) || (xx == (index.n-1))) {
			/* printf ("found an end of run at %d\n",xx); */

			/* are we on the last index, and it is not a -1? */
			if ((index.p[xx] > -1) && (xx == (index.n-1))) {
				zz++; /* include this index */
			}

			IndexSize += (zz-2) *4;
			/* bounds checking... */
			if (zz < 3) {
				printf ("IndexedTriangle[Fan|Strip]Set, index %d is less than 3\n");
				return 0;
			}
			zz = 0;
		} else {
			zz++;
		}
	}

	/* printf ("ITFS, IndexSize %d\n",IndexSize); */
	return IndexSize;
}


/* check validity of fields */
int checkX3DIndexedFaceSetFields (struct X3D_IndexedFaceSet *this_) {
	/* does this have any coordinates? */
	if (this_->coord == 0) {
		#ifdef VERBOSE
		printf ("checkX3DIFS - have an IFS (%d) with no coords...\n",this_);
		#endif
		return FALSE;
	}
	if (this_->coordIndex.n == 0) {
		#ifdef VERBOSE
		printf ("checkX3DIFS - have an IFS (%d) with no coordIndex, pointer is %d offset is %d\n",this_,
			this_->coordIndex.p,offsetof (struct X3D_IndexedFaceSet, coordIndex));
		#endif
		return FALSE;
	}
	return TRUE;
}

/* check validity of ElevationGrid fields */
int checkX3DElevationGridFields (struct X3D_ElevationGrid *this_, float **points, int *npoints) {
	int i,j;
	int nx = (this_->xDimension);
	float xSp = (this_->xSpacing);
	int nz = (this_->zDimension);
	float zSp = (this_->zSpacing);
	float *height = ((this_->height).p);
	int ntri = (nx && nz ? 2 * (nx-1) * (nz-1) : 0);
	int nh = ((this_->height).n);
	struct X3D_PolyRep *rep = (struct X3D_PolyRep *)this_->_intern;

	float *newpoints;
	float newPoint[3];
	int nquads = ntri/2;
	int *cindexptr;

	float *tcoord;
	
	/* check validity of input fields */
	if(nh != nx * nz) {
		if (nh > nx * nz) {
			printf ("Elevationgrid: warning: x,y vs. height: %d * %d ne %d:\n", nx,nz,nh);
		} else {
			printf ("Elevationgrid: error: x,y vs. height: %d * %d ne %d:\n", nx,nz,nh);
			return FALSE;
		}
	}

	/* do we have any triangles? */
	if ((nx < 2) || (nz < 2)) {
		printf ("ElevationGrid: xDimension and zDimension less than 2 %d %d\n", nx,nz);
		return FALSE;
	}

	/* any texture coordinates passed in? if so, DO NOT generate any texture coords here. */
        if (!(this_->texCoord)) {
		/* allocate memory for texture coords */
		FREE_IF_NZ(rep->GeneratedTexCoords);

		/* 6 vertices per quad each vertex has a 2-float tex coord mapping */
		tcoord = rep->GeneratedTexCoords = (float *)MALLOC (sizeof (float) * nquads * 12); 

		rep->tcindex=0; /* we will generate our own mapping */
	}

	/* make up points array */
	/* a point is a vertex and consists of 3 floats (x,y,z) */
	newpoints = (float *)MALLOC (sizeof (float) * nz * nx * 3);
	 
	FREE_IF_NZ(rep->actualCoord);
	rep->actualCoord = (float *)newpoints;

	/* make up coord index */
	if (this_->coordIndex.n > 0) FREE_IF_NZ(this_->coordIndex.p);
	this_->coordIndex.p = MALLOC (sizeof(int) * nquads * 5);
	cindexptr = this_->coordIndex.p;

	this_->coordIndex.n = nquads * 5;
	/* return the newpoints array to the caller */
	*points = newpoints;
	*npoints = this_->coordIndex.n;

	for (j = 0; j < (nz -1); j++) {
		for (i=0; i < (nx-1) ; i++) {
			/*
			 printf ("coord maker, j %d i %d\n",j,i);
			printf ("coords for this quad: %d %d %d %d %d\n",
				j*nx+i, j*nx+i+nx, j*nx+i+nx+1, j*nx+i+1, -1);
			*/
			
			*cindexptr = j*nx+i; cindexptr++;
			*cindexptr = j*nx+i+nx; cindexptr++;
			*cindexptr = j*nx+i+nx+1; cindexptr++;
			*cindexptr = j*nx+i+1; cindexptr++;
			*cindexptr = -1; cindexptr++;

		}
	}

	/* tex coords These need to be streamed now; that means for each quad, each vertex needs its tex coords. */
	/* if the texCoord node exists, let render_TextureCoordinate (or whatever the node is) do our work for us */
	if (!(this_->texCoord)) {
		for (j = 0; j < (nz -1); j++) {
			for (i=0; i < (nx-1) ; i++) {
				/* first triangle, 3 vertexes */
				/* first tri */
				*tcoord = ((float) (i+0)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+0)/(nz-1)); tcoord ++; 
			
				*tcoord = ((float) (i+0)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+1)/(nz-1)); tcoord ++; 
	
				*tcoord = ((float) (i+1)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+1)/(nz-1)); tcoord ++; 
	
				/* second tri */
				*tcoord = ((float) (i+0)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+0)/(nz-1)); tcoord ++; 
	
				*tcoord = ((float) (i+1)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+1)/(nz-1)); tcoord ++; 
	
				*tcoord = ((float) (i+1)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+0)/(nz-1)); tcoord ++; 
			}
		}
	}
			
	/* Render_Polyrep will use this number of triangles */
	rep->ntri = ntri;

	for (j=0; j<nz; j++) {
		for (i=0; i < nx; i++) {
		
		/*
		 printf ("point [%d,%d] is %f %f %f (hei ind %d)\n",
			i,j,
			xSp * i,
			height[i+(j*nx)],
			zSp * j,
			i+(j*nx));
		*/
		
		
		newPoint[0] = xSp * i; newPoint[1] = height[i+(j*nx)]; newPoint[2]=zSp*j;
		memcpy(newpoints,newPoint,sizeof(float)*3);
		newpoints += 3;
		}
	}

	return TRUE;
}

int checkX3DComposedGeomFields (struct X3D_IndexedFaceSet *this_) {
	struct SFColor *points;
	int npoints = 0;
	int retval = TRUE;
	int IndexSize = 0;
	int xx,yy,zz; /* temporary variables */
	int fanVertex;
	int *newIndex;
	int windingOrder; /*TriangleStripSet ordering */


	/* printf ("checkX3DComposedGeomFields for node (%d) %s\n",
			this_->_nodeType, X3DCOMPOSED_STRING(this_->_nodeType)); 
	*/
	

	/* creaseAngle - set if normalPerVertex TRUE */
	if (this_->normalPerVertex) {
		switch (this_->_nodeType) {
			/* IFS has creaseAngle; TriangleSet specs no smoothing */
			case NODE_TriangleSet:
				break;

			/* set the creaseAngle to set smoothing for the rest */
			default:
				/* printf ("ITS - setting creaseAngle to 3.141*2\n"); */
				this_->creaseAngle = PI*2;
		}
	}

	/* colorPerVertex always TRUE for these Geom types */
	this_->colorPerVertex = TRUE;

	/* verify fields for each Node type, according to the spec. Fields that SHOULD
	   not appear in a node are identified in Parser.pm using the hashes in VRMLNodes.pm
	   so we do not have to worry about them here. */
	switch (this_->_nodeType) {
		case NODE_IndexedTriangleFanSet   :
			/* printf ("start of ITFS\n"); */
			IndexSize = returnIndexedFanStripIndexSize(this_->index);
			if (IndexSize == 0) {
				/* printf ("IndexSize for ITFS %d\n",IndexSize); */
				break;
			}

			newIndex = MALLOC (sizeof(int) * IndexSize);
			/* now calculate the indexes */

			xx=0;
			yy=0;
			while (xx < (this_->index.n-1)) {
				fanVertex = xx;
				/* scan forward to find end of fan */
				while ((xx<this_->index.n) && (this_->index.p[xx] > -1)) xx++;
				/* printf ("fan runs between %d and %d\n", fanVertex,xx);  */

				/* bounds checking... */
				if (xx >= IndexSize) {
					printf ("ITFS - index size error... IndexSize < index value \n");
					xx = IndexSize;
				}

				for (zz=fanVertex+1; zz<(xx-1); zz++) {
					/* printf ("newIndexSize %d, fv %d, zz %d\n",IndexSize, fanVertex, zz); */
					newIndex[yy] = this_->index.p[fanVertex]; yy++;
					newIndex[yy] = this_->index.p[zz]; yy++; 
					newIndex[yy] = this_->index.p[zz+1]; yy++;
					newIndex[yy] = -1; yy++;
				}
				
				/* is this the end of the fan? */
				if (xx < (this_->index.n-1)) {
					xx++; /* skip past the -1 */
					fanVertex = xx;
					/* printf ("end of fan, but not end of structure - fanVertex %d, xx %d yy %d\n",fanVertex,xx,yy); */
				}
			}
					
			/* xx=0; while (xx < IndexSize) { printf ("index %d val %d\n",xx,newIndex[xx]); xx++; } */

			/* now, make the new index active */
			/* FREE_IF_NZ (this_->coordIndex.p); should free if MALLOC'd already */
			this_->coordIndex.p = newIndex;
			this_->coordIndex.n = IndexSize;
                	break;

		case NODE_IndexedTriangleStripSet:
			/* printf ("start of ITSS\n"); */
			IndexSize = returnIndexedFanStripIndexSize(this_->index);
			if (IndexSize == 0) {
				/* printf ("IndexSize for ITFS %d\n",IndexSize); */
				break;
			}

			newIndex = MALLOC (sizeof(int) * IndexSize);

			/* now calculate the indexes */
			xx=0;
			yy=0; zz = 0;
			while (xx < (this_->index.n-1)) {
				fanVertex = xx;
				/* scan forward to find end of fan */
				while ((xx<this_->index.n) && (this_->index.p[xx] > -1)) xx++;
				/* printf ("strip runs between %d and %d\n", fanVertex,xx);  */

				/* bounds checking... */
				if (xx >= IndexSize) {
					printf ("ITFS - index size error... IndexSize < index value \n");
					xx = IndexSize;
				}


				windingOrder=0;
				for (zz=fanVertex; zz<(xx-2); zz++) {
					if (windingOrder==0) {
						newIndex[yy] = this_->index.p[zz]; yy++;
						newIndex[yy] = this_->index.p[zz+1]; yy++; 
						newIndex[yy] = this_->index.p[zz+2]; yy++;
						windingOrder ++;
					} else {
						newIndex[yy] = this_->index.p[zz]; yy++;
						newIndex[yy] = this_->index.p[zz+2]; yy++; 
						newIndex[yy] = this_->index.p[zz+1]; yy++;
						windingOrder =0;
					}
					newIndex[yy] = -1; yy++;
				}
				
				/* is this the end of the fan? */
				if (xx < (this_->index.n-1)) {
					xx++; /* skip past the -1 */
					fanVertex = xx;
					/* printf ("end of fan, but not end of structure - fanVertex %d, xx %d yy %d\n",fanVertex,xx,yy); */
				}
				zz += 2;
			}
					
			/* xx=0; while (xx < IndexSize) { printf ("index %d val %d\n",xx,newIndex[xx]); xx++; }  */

			/* now, make the new index active */
			/* FREE_IF_NZ (this_->coordIndex.p); should free if MALLOC'd already */
			this_->coordIndex.p = newIndex;
			this_->coordIndex.n = IndexSize;
                	break;

		case NODE_IndexedTriangleSet :
			IndexSize = ((this_->index.n) * 4) / 3;
			if (IndexSize <= 0) {
				break;
			}

			newIndex = MALLOC (sizeof(int) * IndexSize);
			zz = 0; yy=0;
			/* printf ("index: "); */
			for (xx = 0; xx < this_->index.n; xx++) {
				newIndex[zz] = this_->index.p[xx];
				/* printf (" %d ",newIndex[zz]);  */
				zz++;
				yy++;
				if (yy == 3) {
					/* end of one triangle, put a -1 in there */
					newIndex[zz] = -1;
					/* printf (" -1 "); */
					zz++;
					yy = 0;
				}
			/* printf ("\n"); */
			}

			/* now, make the new index active */
			/* FREE_IF_NZ (this_->coordIndex.p); should free if MALLOC'd already */
			this_->coordIndex.p = newIndex;
			this_->coordIndex.n = IndexSize;
                	break;

		case NODE_TriangleSet :

		        if(this_->coord) {
				struct Multi_Vec3f *dtmp;
				dtmp = getCoordinate (this_->coord, "TriangleSet");
				npoints = dtmp->n;
				points = dtmp->p;
		        }

			/* verify whether we have an incorrect number of coords or not */
			if (((npoints/3)*3) != npoints) {
				printf ("Warning, in TriangleSet, Coordinates not a multiple of 3\n");
				npoints = ((npoints/3)*3);
			}

			/* printf ("npoints %d\n",npoints); */


			/* calculate index size; every "face" ends in -1 */
			IndexSize = (npoints * 4) / 3;
			/* printf ("IndexSize is %d\n",IndexSize); */
			this_->coordIndex.p = MALLOC (sizeof(int) * IndexSize);
			this_->coordIndex.n = IndexSize;

			IndexSize = 0; /* for assigning the indexes */
			
			/* now calculate the indexes */
			yy=0; zz=0;
			for (xx=0; xx<npoints; xx+=3) {
				/* printf ("index %d tris %d %d %d -1\n", xx/3, xx, xx+1, xx+2);  */
				this_->coordIndex.p[IndexSize++] = xx;
				this_->coordIndex.p[IndexSize++] = xx+1;
				this_->coordIndex.p[IndexSize++] = xx+2;
				this_->coordIndex.p[IndexSize++] = -1;
			}
                	break;

		case NODE_TriangleStripSet :
			 /* printf ("TSS, stripCount %d\n",(this_->stripCount).n);  */
			if ((this_->stripCount).n < 1) {
				freewrlDie ("TriangleStripSet, need at least one stripCount element");
			}

			/* calculate the size of the Index array */
			for (xx=0; xx<(this_->stripCount).n; xx++) {
				 /* printf ("stripCount %d is %d  \n",xx,(this_->stripCount).p[xx]); */
				IndexSize += ((this_->stripCount).p[xx]-2) * 4;
				/* bounds checking... */
				if ((this_->stripCount).p[xx] < 3) {
					printf ("TriangleStripSet, index %d is less than 3\n");
				}
			}

			/* printf ("IndexSize is %d\n",IndexSize); */
			this_->coordIndex.p = MALLOC (sizeof(int) * IndexSize);
			this_->coordIndex.n = IndexSize;
			IndexSize = 0; /* for assigning the indexes */
			
			/* now calculate the indexes */
			yy=0; zz=0;
			for (xx=0; xx<(this_->stripCount).n; xx++) {
				windingOrder=0;
				/* printf ("stripCount %d is %d  \n",xx,(this_->stripCount).p[xx]);  */
				for (yy=0; yy< ((this_->stripCount).p[xx]-2); yy++) {
					if (windingOrder==0) {
						/* printf ("fcwo0 %d tris %d %d %d -1\n", xx, zz, zz+1, zz+2); */
						this_->coordIndex.p[IndexSize++] = zz;
						this_->coordIndex.p[IndexSize++] = zz+1;
						this_->coordIndex.p[IndexSize++] = zz+2;
						windingOrder++;
					} else {
						/* printf ("fcwo1 %d tris %d %d %d -1\n", xx, zz+1, zz, zz+2); */
						this_->coordIndex.p[IndexSize++] = zz+1;
						this_->coordIndex.p[IndexSize++] = zz;
						this_->coordIndex.p[IndexSize++] = zz+2;
						windingOrder=0;
					}
					this_->coordIndex.p[IndexSize++] = -1;
					zz++;
				}
				zz += 2;
			}
					
                	break;

		case NODE_TriangleFanSet :
			/* printf ("TFS, fanCount %d\n",(this_->fanCount).n);  */
			if ((this_->fanCount).n < 1) {
				freewrlDie ("TriangleFanSet, need at least one fanCount element");
			}

			/* calculate the size of the Index array */
			for (xx=0; xx<(this_->fanCount).n; xx++) {
				/* printf ("fanCount %d is %d  \n",xx,(this_->fanCount).p[xx]); */
				IndexSize += ((this_->fanCount).p[xx]-2) * 4;
				/* bounds checking... */
				if ((this_->fanCount).p[xx] < 3) {
					printf ("TriangleFanSet, fanCount index %d is less than 3\n", (this_->fanCount).p[xx]);
				}
			}

			/* printf ("IndexSize is %d\n",IndexSize); */
			this_->coordIndex.p = MALLOC (sizeof(int) * IndexSize);
			this_->coordIndex.n = IndexSize;
			IndexSize = 0; /* for assigning the indexes */

			/* now calculate the indexes */
			yy=0; zz=0;
			for (xx=0; xx<(this_->fanCount).n; xx++) {
				/* printf ("fanCount %d is %d  \n",xx,(this_->fanCount).p[xx]); */
				fanVertex = zz;
				zz ++;
				for (yy=0; yy< ((this_->fanCount).p[xx]-2); yy++) {
					/* printf ("fc %d tris %d %d %d -1\n",
						xx, fanVertex, zz, zz+1); */
					this_->coordIndex.p[IndexSize++] = fanVertex;
					this_->coordIndex.p[IndexSize++] = zz;
					this_->coordIndex.p[IndexSize++] = zz+1;
					this_->coordIndex.p[IndexSize++] = -1;
					zz++;
				}
				zz++;
			}
                	break;
        	default:
			printf ("invalid node passed to CheckX3DComposedGeometryFields!\n");
			return FALSE;
                	break;
        }

	/* printf ("check returning TRUE for X3DGeom fields\n"); */
	return retval;
}


void make_indexedfaceset(struct X3D_IndexedFaceSet *this_) {
	int cin;
	int cpv;
	int npv;
	int tcin;
	int colin;
	int norin;
	float creaseAngle;
	int ccw;

	int ntri = 0;
	int nvert = 0;
	int npoints = 0;
	int nnormals=0;
	int ncolors=0;
	int texCoordNodeType = 0;
	int vert_ind = 0;
	int calc_normind = 0;

	struct SFColor *c1;
	struct SFColor *points;
	struct X3D_PolyRep *rep_ = (struct X3D_PolyRep *)this_->_intern;
	struct SFColor *normals;

	int *cindex;		/* Coordinate Index	*/
	int *colindex;		/* Color Index		*/
	int *tcindex=0;		/* Tex Coord Index	*/
	int *norindex;		/* Normals Index	*/

	int faces=0;
	struct point_XYZ *facenormals; /*  normals for each face*/
	int	*faceok;	/*  is this face ok? (ie, not degenerate triangles, etc)*/
	int	*pointfaces;

	GLdouble tess_v[3];             /*param.to gluTessVertex()*/
	int *tess_vs;              /* pointer to space needed */


	int i;				/* general purpose counters */
	int this_face, this_coord, this_normal, this_normalindex;

	struct X3D_Color *cc;
	struct X3D_Normal *nc;
	struct X3D_TextureCoordinate *tc;

	#ifdef VERBOSE
	printf ("start of make_indexedfaceset for node %u, cin %d\n",this_, this_->coordIndex.n);
	#endif

	if (this_->_nodeType == NODE_IndexedFaceSet) {
		if (!checkX3DIndexedFaceSetFields(this_)) {
	        	rep_->ntri = 0;
	        	return;
		}
		

	} else if (this_->_nodeType == NODE_ElevationGrid) {
		/* this might be an ElevationGrid based on a GeoElevationGrid */
		if ((this_->_nparents>=1) && (X3D_NODE(this_->_parents[0])->_nodeType == NODE_GeoElevationGrid)) {
			if (!checkX3DGeoElevationGridFields(X3D_ELEVATIONGRID(this_),
				(float **)&points, &npoints)) {
		        	rep_->ntri = 0;
		        	return;
			}
		} else {
			if (!checkX3DElevationGridFields(X3D_ELEVATIONGRID(this_),
				(float **)&points, &npoints)) {
		        	rep_->ntri = 0;
		        	return;
			}
		}

	/* if this is a X3DComposedGeom - check fields */
	} else {
		if (!checkX3DComposedGeomFields(this_)) {
	        	rep_->ntri = 0;
	        	return;
		}
	}

	/* lets get the structure parameters, after munging by checkX3DComposedGeomFields... */
	cin= ((this_->coordIndex).n);
	cpv = ((this_->colorPerVertex));
	npv = ((this_->normalPerVertex));
	tcin = ((this_->texCoordIndex).n);
	colin = ((this_->colorIndex).n);
	norin = ((this_->normalIndex).n);
	creaseAngle = (this_->creaseAngle);
	ccw = ((this_->ccw));
	#ifdef VERBOSE
	printf ("NOW, the IFS has a cin of %d ca %f\n",cin,creaseAngle);
	#endif

	/* record ccw flag */
	rep_->ccw = ccw;

	/* check to see if there are params to make at least one triangle */
	if (cin<2) {
		#ifdef VERBOSE
		printf ("Null IFS found, returing ntri0\n");
		#endif
	        rep_->ntri = 0;
	        return;
	}

	/* if the last coordIndex == -1, ignore it */
	if(((this_->coordIndex).p[cin-1]) == -1) { cin--; }


	/* texture coords IndexedFaceSet coords colors and normals */
	if(this_->coord) {
		struct Multi_Vec3f *dtmp;
		dtmp = getCoordinate (this_->coord, "make FacedSet");
		npoints = dtmp->n;
		points = dtmp->p;
	}


	/* just check this parameter here for correctness and, whether to generate other nodes. We
	   will check it better in stream_polyrep. */
	if (this_->color) {
		cc = (struct X3D_Color *) this_->color;
		if ((cc->_nodeType != NODE_Color) && (cc->_nodeType != NODE_ColorRGBA)) {
			printf ("make_IFS, expected %d got %d\n", NODE_Color, cc->_nodeType);
		} else {
			ncolors = cc->color.n;
		}
	}
	
	if(this_->normal) {
		nc = (struct X3D_Normal *) this_->normal;
		if (nc->_nodeType != NODE_Normal) {
			printf ("make_IFS, normal expected %d, got %d\n",NODE_Normal, nc->_nodeType);
		} else {
			normals = nc->vector.p;
			nnormals = nc->vector.n;
		}
	}


	/* just check this parameter here for correctness and, whether to generate other nodes. We
	   will check it better in stream_polyrep. */
	if (this_->texCoord) {
		tc = (struct X3D_TextureCoordinate *) this_->texCoord;
		rep_->tcoordtype=tc->_nodeType;
		texCoordNodeType = tc->_nodeType;
	} else {
		rep_->tcoordtype=0;
	}

	if (!smooth_normals){
		creaseAngle = 0.0;  /* trick following code into doing things quick */
	}

	/* count the faces in this polyrep and allocate memory. */
	faces = count_IFS_faces (cin,this_);
	#ifdef VERBOSE
	printf ("faces %d, cin %d npoints %d\n",faces,cin,npoints);
	#endif

	if (faces == 0) {
		rep_->ntri = 0;
		return;
	}

	/* are there any coordinates? */
	if (npoints <= 0) {
		rep_->ntri = 0;
		return;
	}

	facenormals = (struct point_XYZ*)MALLOC(sizeof(*facenormals)*faces);
	faceok = (int*)MALLOC(sizeof(int)*faces);
	pointfaces = (int*)MALLOC(sizeof(*pointfaces)*npoints*POINT_FACES); /* save max x points */

	/* generate the face-normals table, so for each face, we know the normal
	   and for each point, we know the faces that it is in */
	if (!IFS_face_normals (facenormals,faceok,pointfaces,faces,npoints,cin,points,this_,ccw)) {
		rep_->ntri=0;
		FREE_IF_NZ (facenormals);
		FREE_IF_NZ (faceok);
		FREE_IF_NZ (pointfaces);

		return;
	}

	/* wander through to see how much memory needs allocating for triangles */
	for(i=0; i<cin; i++) {
		if(((this_->coordIndex).p[i]) == -1) {
			ntri += nvert-2;
			nvert = 0;
		} else {
			nvert ++;
		}
	}
	if(nvert>2) {ntri += nvert-2;}


	#ifdef VERBOSE
	printf ("vert %d ntri %d\n",nvert,ntri);
	#endif

	/* Tesselation MAY use more triangles; lets estimate how many more */
	if(!(this_->convex)) { ntri =ntri*2; }

	/* fudge factor - leave space for 1 more triangle just in case we have errors on input */
	ntri++;

	cindex = rep_->cindex = (int*)MALLOC(sizeof(*(rep_->cindex))*3*(ntri));
	colindex = rep_->colindex = (int*)MALLOC(sizeof(*(rep_->colindex))*3*(ntri));
	norindex = rep_->norindex = (int*)MALLOC(sizeof(*(rep_->norindex))*3*ntri);
	
	/* zero the indexes */
	bzero (colindex,sizeof(*(rep_->colindex))*3*(ntri));
	bzero (norindex,sizeof(*(rep_->colindex))*3*(ntri));

	/* if we calculate normals, we use a normal per point, NOT per triangle */
	if (!nnormals) {  		/* 3 vertexes per triangle, and 3 points per tri */
		rep_->normal = (float*)MALLOC(sizeof(*(rep_->normal))*3*3*ntri);
	} else { 			/* dont do much, but get past check below */
		rep_->normal = (float*)MALLOC(1);
	}


	tcindex = rep_->tcindex = (int*)MALLOC(sizeof(*(rep_->tcindex))*3*(ntri));

	/* Concave faces - use the OpenGL Triangulator to give us the triangles */
	tess_vs=(int*)MALLOC(sizeof(*(tess_vs))*(ntri)*3);

	this_coord = 0;
	this_normal = 0;
	this_normalindex = 0;
	i = 0;

	for (this_face=0; this_face<faces; this_face++) {
		int relative_coord;		/* temp, used if not tesselating	*/
		int initind, lastind;  		/* coord indexes 			*/

		global_IFS_Coord_count = 0;
		relative_coord = 0;

		if (!faceok[this_face]) {
			#ifdef VERBOSE
			printf ("in generate of faces, face %d is invalid, skipping...\n",this_face);
			#endif

			/* skip past the seperator, except if we are t the end */

			/*  skip to either end or the next -1*/
			while ((this_coord < cin) && (((this_->coordIndex).p[this_coord]) != -1)) this_coord++;

			/*  skip past the -1*/
			if ((this_coord < (cin-1)) && (((this_->coordIndex).p[this_coord]) == -1)) this_coord++;
		} else {

			#ifdef VERBOSE
			printf ("working on face %d coord %d total coords %d coordIndex %d\n",
				this_face,this_coord,cin,((this_->coordIndex).p[ this_coord]));
			#endif

			/* create the global_IFS_coords array, at least this time 	*/
			/*								*/
			/* What we do is to create a series of triangle vertex 		*/
			/* relative to the current coord index, then use that		*/
			/* to generate the actual coords further down. This helps	*/
			/* to map normals, textures, etc when tesselated and the	*/
			/*  *perVertex modes are set.					*/

			/* If we have concave, tesselate! */
			if (!((this_->convex))) {
				gluBeginPolygon(global_tessobj);
			} else {
				initind = relative_coord++;
				lastind = relative_coord++;
			}

			i = ((this_->coordIndex).p[ relative_coord + this_coord]);

			while (i != -1) {
				if (!((this_->convex))) {
					/*  printf ("\nwhile, i is %d this_coord %d rel coord %d\n",i,this_coord,relative_coord);*/
					c1 = &(points[i]);
					tess_v[0] = c1->c[0];
					tess_v[1] = c1->c[1];
					tess_v[2] = c1->c[2];
					tess_vs[relative_coord] = relative_coord;
					gluTessVertex(global_tessobj,tess_v,&tess_vs[relative_coord++]);
				} else {
					/* take coordinates and make triangles out of them */
					global_IFS_Coords[global_IFS_Coord_count++] = initind;
					global_IFS_Coords[global_IFS_Coord_count++] = lastind;
					global_IFS_Coords[global_IFS_Coord_count++] = relative_coord;
					/* printf ("triangle %d %d %d\n",initind,lastind,relative_coord);*/
					lastind = relative_coord++;
				}

				if (relative_coord + this_coord == cin) {
					i = -1;
				} else {
					i = ((this_->coordIndex).p[ relative_coord + this_coord]);
				}
			}

			if (!((this_->convex))) {
				gluEndPolygon(global_tessobj);

				/* Tesselated faces may have a different normal than calculated previously */
				/* bounds check, once again */

				verify_global_IFS_Coords(cin);

				IFS_check_normal (facenormals,this_face,points, this_coord, this_,ccw);
			}


			/* now store this information for the whole of the polyrep */
			for (i=0; i<global_IFS_Coord_count; i++) {
				/* Triangle Coordinate */
				cindex [vert_ind] = ((this_->coordIndex).p[this_coord+global_IFS_Coords[i]]);

				/* printf ("vertex  %d  gic %d cindex %d\n",vert_ind,global_IFS_Coords[i],cindex[vert_ind]);*/

				/* Vertex Normal */
				if(nnormals) {
					if (norin) {
						/* we have a NormalIndex */
						if (npv) {
							norindex[vert_ind] = ((this_->normalIndex).p[this_coord+global_IFS_Coords[i]]);
							/*  printf ("norm1, index %d\n",norindex[vert_ind]);*/
						} else {
							norindex[vert_ind] = ((this_->normalIndex).p[this_face]);
							/*  printf ("norm2, index %d\n",norindex[vert_ind]);*/
						}
					} else {
						/* no normalIndex  - use the coordIndex */
						if (npv) {
							norindex[vert_ind] = ((this_->coordIndex).p[this_coord+global_IFS_Coords[i]]);
							/* printf ("norm3, index %d\n",norindex[vert_ind]);*/
						} else {
							norindex[vert_ind] = this_face;
							/* printf ("norm4, index %d\n",norindex[vert_ind]);*/
						}
					}

				} else {
					if (fabs(creaseAngle) > 0.00001) {
						/* normalize each vertex */
						normalize_ifs_face (&rep_->normal[calc_normind*3],
							facenormals, pointfaces, cindex[vert_ind],
							this_face, creaseAngle);
						rep_->norindex[vert_ind] = calc_normind++;
					} else {
						/* use the calculated normals */
						rep_->normal[vert_ind*3+0]=facenormals[this_face].x;
						rep_->normal[vert_ind*3+1]=facenormals[this_face].y;
						rep_->normal[vert_ind*3+2]=facenormals[this_face].z;
						rep_->norindex[vert_ind] = vert_ind;
						/* printf ("using calculated normals %f %f %f for face %d, vert_ind %d\n",*/
							/* rep_->normal[vert_ind*3+0],rep_->normal[vert_ind*3+1],*/
							/* rep_->normal[vert_ind*3+2],this_face,rep_->norindex[vert_ind]);*/
					}
				}

				/* Vertex Colours */
				if(ncolors) {
					if (colin) {
						int tmpI;
						/* we have a colorIndex */
						if (cpv) tmpI = this_coord+global_IFS_Coords[i];
						else tmpI = this_face;
						
						if (tmpI >= this_->colorIndex.n) {
							printf ("faceSet, colorIndex problem, %d >= %d\n", tmpI, this_->colorIndex.n);
							colindex[vert_ind] = 0;
						} else {
							colindex[vert_ind] = this_->colorIndex.p[tmpI];
						}
						/* printf ("col2, index %d\n",colindex[vert_ind]); */
						
					} else {
						/* no colorIndex  - use the coordIndex */
						if (cpv) {
							colindex[vert_ind] = ((this_->coordIndex).p[this_coord+global_IFS_Coords[i]]);
							  /* printf ("col3, index %d\n",colindex[vert_ind]); */
						} else {
							colindex[vert_ind] = this_face;
							  /* printf ("col4, index %d\n",colindex[vert_ind]); */
						}
					}
				}


				/* Texture Coordinates */
				if (tcin) {
					/* bounds checking if we run out of texCoords, just fill in with 0 */
					if ((this_coord+global_IFS_Coords[i]) < this_->texCoordIndex.n) {
						tcindex[vert_ind] = ((this_->texCoordIndex).p[this_coord+global_IFS_Coords[i]]);
					} else {
						tcindex[vert_ind] = 0;
					}
					/* printf ("ntexCoords,tcin,  index %d\n",tcindex[vert_ind]); */
				} else {
					/* no texCoordIndex, use the Coord Index */
					tcindex[vert_ind] = ((this_->coordIndex).p[this_coord+global_IFS_Coords[i]]);
					/* printf ("ntexcoords, notcin, vertex %d point %d\n",vert_ind,tcindex[vert_ind]); */
				}

				/* increment index, but check for baaad errors.	 */
				if (vert_ind < (ntri*3-1)) vert_ind++;
			}

			/* for the next face, we work from a new base */
			this_coord += relative_coord;

			/* skip past the seperator, except if we are t the end */
			if (this_coord < cin)
				if (((this_->coordIndex).p[this_coord]) == -1) {this_coord++;}
		}
	}

	/* we have an accurate triangle count now... */
	rep_->ntri = vert_ind/3;
	#ifdef VERBOSE
	printf ("make_indededfaceset, end, ntri %d\n",rep_->ntri);
	#endif

	FREE_IF_NZ (tess_vs);
	FREE_IF_NZ (facenormals);
	FREE_IF_NZ (faceok);
	FREE_IF_NZ (pointfaces);
}
/********************************************************************************************/
/* get a valid alpha angle from that that is passed in */
/* asin of 1.0000 seems to fail sometimes, so */
double getAlpha(float ang) {
	if (ang >= 0.99999) return asin(0.9999);
	else if (ang <= -0.99999) return asin(-0.9999);
	return asin((double)ang);
}


double getGamma(double alpha, double minor) {
	double gamma;

	if(APPROX(cos(alpha),0))
		return (double) 0;
	else {
		gamma=acos(minor / cos(alpha));
		if(fabs(sin(gamma)-(-minor/cos(alpha)))>fabs(sin(gamma))) gamma=-gamma;
	}
	return gamma;
}

void compute_spy_spz(struct point_XYZ *spy, struct point_XYZ *spz, struct SFColor *spine, int nspi) {
	int majorX = FALSE;
	int majorY = FALSE;
	int majorZ = FALSE;
	int minorX = FALSE;
	int minorY = FALSE;
	int minorZ = FALSE;
	double alpha,gamma;	/* angles for the rotation	*/
	int spi;
	float spylen;
	struct point_XYZ spp1;


	/* need to find the rotation from SCP[spi].y to (0 1 0)*/
	/* and rotate (0 0 1) and (0 1 0) to be the new y and z	*/
	/* values for all SCPs					*/
	/* I will choose rotation about the x and z axis	*/

	/* search a non trivial vector along the spine */
	for(spi=1;spi<nspi;spi++) {
		VEC_FROM_CDIFF(spine[spi],spine[0],spp1);
		if(!APPROX(VECSQ(spp1),0))
 			break;
 	}

	/* normalize the non trivial vector */
	spylen=1/sqrt(VECSQ(spp1)); VECSCALE(spp1,spylen);
	#ifdef VERBOSE
		printf("Reference vector along spine=[%f,%f,%f]\n", spp1.x,spp1.y,spp1.z);
	#endif


	/* find the major and minor axes */
	if ((fabs(spp1.x) >= fabs(spp1.y)) && (fabs(spp1.x) >= fabs(spp1.z))) majorX = TRUE;
	else if ((fabs(spp1.y) >= fabs(spp1.x)) && (fabs(spp1.y) >= fabs(spp1.z))) majorY = TRUE;
	else majorZ = TRUE;
	if ((fabs(spp1.x) <= fabs(spp1.y)) && (fabs(spp1.x) <= fabs(spp1.z))) minorX = TRUE;
	else if ((fabs(spp1.y) <= fabs(spp1.x)) && (fabs(spp1.y) <= fabs(spp1.z))) minorY = TRUE;
	else minorZ = TRUE;

	#ifdef VERBOSE
	printf ("major axis %d %d %d\n",majorX, majorY, majorZ);
	printf ("minor axis %d %d %d\n",minorX, minorY, minorZ);
	#endif

	if(majorX) {
		/* get the angle for the x axis rotation	*/
		/* asin of 1.0000 seems to fail sometimes, so */

		alpha = getAlpha(spp1.x);
		gamma = getGamma(alpha,minorY?spp1.y:spp1.z);

		#ifdef VERBOSE
			printf("majorX: alpha=%f gamma=%f\n",alpha,gamma);
		#endif


		/* XXX: should we use the minor axis to determine the order of minor calculations???? */
		spy->y=-(cos(alpha)*(-sin(gamma)));
		spy->z=cos(alpha)*cos(gamma);
		spy->x=sin(alpha);
		spz->y=-(sin(alpha)*sin(gamma));
		spz->z=(-sin(alpha))*cos(gamma);
		spz->x=cos(alpha);
	} else if(majorZ) {
		/* get the angle for the z axis rotation	*/

		alpha = getAlpha(spp1.z);
		gamma = getGamma(alpha,minorX?spp1.x:spp1.y);

		#ifdef VERBOSE
			printf("majorZ: alpha=%f gamma=%f\n",alpha,gamma);
		#endif
		/* XXX: should we use the minor axis to determine the order of minor calculations???? */
		spy->y=-(cos(alpha)*(-sin(gamma)));
		spy->x=cos(alpha)*cos(gamma);
		spy->z=sin(alpha);
		spz->y=-(sin(alpha)*sin(gamma));
		spz->x=(-sin(alpha))*cos(gamma);
		spz->z=cos(alpha);
	} else {
		/* get the angle for the y axis rotation	*/

		alpha = getAlpha(spp1.y);
		gamma = getGamma(alpha,minorX?spp1.x:spp1.z);

		#ifdef VERBOSE
			printf("majorY: lpha=%f gamma=%f\n",alpha,gamma);
		#endif
		/* XXX: should we use the minor axis to determine the order of minor calculations???? */
		spy->x=-(cos(alpha)*(-sin(gamma)));
		spy->z=cos(alpha)*cos(gamma);
		spy->y=sin(alpha);
		spz->x=-(sin(alpha)*sin(gamma));
		spz->z=(-sin(alpha))*cos(gamma);
		spz->y=cos(alpha);
	}
} 





/***************************************************************
   stream the extrusion texture coords. We do this now because 
   stream_polyrep does not go through the tcindexes - the old
   "render every triangle" method did. So, we gain in rendering
   speed for a little bit of post-processing here. 
 ***************************************************************/
void stream_extrusion_texture_coords (struct X3D_PolyRep *rep_, 
			float *tcoord, 
			int *tcindex) {

	int count; 
	int ind;
	float* nc;

	/* printf ("stream_extrusion_texture_coords, have %d triangles \n",rep_->ntri); */

	/* 2 floats per vertex, each triangle has 3 vertexes... */
	rep_->GeneratedTexCoords = (float*)MALLOC (sizeof(float) * 2 * 3 * rep_->ntri);

	nc = rep_->GeneratedTexCoords;

	/* go through - note now that the "span" is 2 floats per vertex, while the old
	   method (used when the extrusion code was written) was to use 3 floats, but
	   ignoring one of them. Thus the "ind*3" stuff below. Yes, we could go through
	   and re-write the generator, but, who cares - the tcoord param is freed after
	   the return of this, so the "waste" is only temporary.
	*/
	for (count = 0; count < rep_->ntri*3; count++) {
		ind = tcindex[count];
		/* printf ("working through vertex %d - tcindex %d vertex %f %f \n",count,ind,
			tcoord[ind*3], tcoord[ind*3+2]);  */
		*nc = tcoord[ind*3]; nc++; *nc = tcoord[ind*3+2]; nc++;
	}
}


void make_Extrusion(struct X3D_Extrusion *node) {

	/*****begin of Member Extrusion	*/
	/* This code originates from the file VRMLExtrusion.pm */

	int tcoordsize;
	int tcindexsize;

	int beginCap = node->beginCap;			/* beginCap flag */
	int endCap = node->endCap;			/* endCap flag */

	int nspi = node->spine.n;			/* number of spine points	*/
	int nsec = node->crossSection.n;		/* no. of points in the 2D curve
							   but note that this is verified
							   and coincident points thrown out */

	int nori = node->orientation.n;			/* no. of given orientators
							   which rotate the calculated SCPs =
							   spine-aligned cross-section planes*/
	int nsca = node->scale.n;			/* no. of scale parameters	*/

	struct SFColor *spine =node->spine.p;		/* vector of spine vertices	*/
	struct SFVec2f *curve =node->crossSection.p;	/* vector of 2D curve points	*/
	struct SFRotation *orientation=node->orientation.p;/*vector of SCP rotations*/

	struct X3D_PolyRep *rep_=(struct X3D_PolyRep *)node->_intern;/*internal rep, we want to fill*/

	/* the next variables will point at members of *rep		*/
	int   *cindex;				/* field containing indices into
						   the coord vector. Three together
						   indicate which points form a
						   triangle			*/
	float *coord;				/* contains vertices building the
						   triangles as x y z values	*/

	float *tcoord;				/* contains vertices building the
						   textures as x y z values	*/

	int	*tcindex;			/* field containing texture indices
						   for the vertex. 		*/

	int   *norindex; 			/* indices into *normal		*/
	float *normal; 				/* (filled in a different function)*/


	int ntri = 0;			 	/* no. of triangles to be used
						   to represent all, but the caps */
	int nctri=0;				/* no. of triangles for both caps*/
	int max_ncoord_add=0;			/* max no. of add coords	*/
	int ncoord_add=0;			/* no. off added coords		*/
	int ncoord=0;				/* no. of used coords		*/

	int ncolinear_at_begin=0;		/* no. of triangles which need
						to be skipped, because curve-points
						are in one line at start of curve*/
	int ncolinear_at_end=0;			/* no. of triangles which need
						to be skipped, because curve-points
						are in one line at end of curve*/

	int spi,sec,triind,pos_of_last_zvalue;	/* help variables 		*/
	int next_spi, prev_spi;
	int t;					/* another loop var		*/


	int circular = FALSE;			/* is spine  closed?		*/
	int tubular=FALSE;				/* is the 2D curve closed?	*/
	int spine_is_one_vertex;		/* only one real spine vertix	*/

	float spxlen,spylen,spzlen;		/* help vars for scaling	*/

						/* def:struct representing SCPs	*/
	struct SCP { 				/* spine-aligned cross-section plane*/
		struct point_XYZ y;			/* y axis of SCP		*/
		struct point_XYZ z;			/* z axis of SCP		*/
		int prev,next;			/* index in SCP[]
						prev/next different vertix for
						calculation of this SCP		*/
		   };

	struct SCP *SCP;			/* dyn. vector rep. the SCPs	*/

	struct point_XYZ spm1,spp1,spy,spz,spx;	/* help vertix vars	*/

	int	tci_ct;				/* Tex Gen index counter	*/

	/* variables for calculating smooth normals */
	int 	HAVETOSMOOTH;
	struct 	point_XYZ *facenormals = 0;
	int	*pointfaces = 0;
	int	*defaultface = 0;
	int	this_face = 0;			/* always counts up		*/
	int	tmp;
	float creaseAngle = node->creaseAngle;
	int	ccw = node->ccw;
	int	end_of_sides;			/* for triangle normal generation,
						   keep track of where the sides end
						   and caps begin		*/

	/* variables for begin/endcap S,T mapping for textures			*/
	float *beginVals;
	float *endVals;
	struct SFVec2f *crossSection;

	#ifdef VERBOSE
		printf ("VRMLExtrusion.pm start\n");
	#endif

	/***********************************************************************
	 *
	 * Copy and verify cross section - remove coincident points (yes, virginia,
	 * one of the NIST tests has this - the pie-shaped convex one
	 *
	 ************************************************************************/

	/* is there anything to this Extrusion??? */
	if (nsec < 1) {
		rep_->ntri=0;
		return;
	} else {
		int tmp1, temp_indx;
		int increment, currentlocn;

		crossSection     = (struct SFVec2f*)MALLOC(sizeof(crossSection)*nsec*2);


		currentlocn = 0;
		for (tmp1=0; tmp1<nsec; tmp1++) {
			/* save this crossSection */
			crossSection[currentlocn].c[0] = curve[tmp1].c[0];
			crossSection[currentlocn].c[1] = curve[tmp1].c[1];

			/* assume that it is not duplicated */
			increment = 1;

			for (temp_indx=0; temp_indx<currentlocn; temp_indx++) {
				if ((APPROX(crossSection[currentlocn].c[0],crossSection[temp_indx].c[0])) &&
				    (APPROX(crossSection[currentlocn].c[1],crossSection[temp_indx].c[1]))) {
					/* maybe we have a closed curve, so points SHOULD be the same */
					if ((temp_indx != 0) && (tmp1 != (nsec-1))) {
						/* printf ("... breaking; increment = 0\n");*/
						increment = 0;
						break;
					} else {
						/* printf ("... we are tubular\n");*/
						tubular = TRUE;
					}
				}
			}
			/* increment the crossSection index, unless it was duplicated */
			currentlocn += increment;
		}

		#ifdef VERBOSE
			printf ("we had nsec %d coords, but now we have %d\n",nsec,currentlocn);
		#endif

		nsec = currentlocn;
	}


	/* now that we have removed possible coincident vertices, we can calc ntris */
	ntri = 2 * (nspi-1) * (nsec-1);

	#ifdef VERBOSE
		printf ("so, we have ntri %d nspi %d nsec %d\n",ntri,nspi,nsec);
	#endif

	/* check if the spline is closed					*/

	circular = APPROX(spine[0].c[0], spine[nspi-1].c[0]) &&
	   		APPROX(spine[0].c[1], spine[nspi-1].c[1]) &&
	  		APPROX(spine[0].c[2], spine[nspi-1].c[2]);

	#ifdef VERBOSE
		printf ("tubular %d circular %d\n",tubular, circular);
	#endif


	/************************************************************************
	 * calc number of triangles per cap, if caps are enabled and possible
	 */

	/* if we are both circular and tubular, we ignore any caps */
	if (circular && tubular) {
		beginCap = FALSE; 
		endCap = FALSE;
		#if VERBOSE
		printf ("Extrusion, turning off caps \n");
		#endif
	}

	if(beginCap||endCap) {
		if(tubular?nsec<4:nsec<3) {
			freewrlDie("Only two real vertices in crossSection. Caps not possible!");
		}

		if(tubular)	nctri=nsec-2;
		else		nctri=nsec-1;

		#ifdef VERBOSE
			printf ("nsec = %d, ntri = %d nctri = %d\n",nsec, ntri,nctri);
		#endif


			/* check if there are colinear points at the beginning of the curve*/
		sec=0;
		while(sec+2<=nsec-1 &&
			/* to find out if two vectors a and b are colinear,
			   try a.x*b.y=a.y*b.x					*/

			APPROX(0,    (crossSection[sec+1].c[0]-crossSection[0].c[0])
				    *(crossSection[sec+2].c[1]-crossSection[0].c[1])
				  -  (crossSection[sec+1].c[1]-crossSection[0].c[1])
				    *(crossSection[sec+2].c[0]-crossSection[0].c[0]))
		     ) ncolinear_at_begin++, sec++;

		/* check if there are colinear points at the end of the curve
			in line with the very first point, because we want to
			draw the triangle to there.				*/
		sec=tubular?(nsec-2):(nsec-1);
		while(sec-2>=0 &&
			APPROX(0,    (crossSection[sec  ].c[0]-crossSection[0].c[0])
				    *(crossSection[sec-1].c[1]-crossSection[0].c[1])
				  -  (crossSection[sec  ].c[1]-crossSection[0].c[1])
				    *(crossSection[sec-1].c[0]-crossSection[0].c[0]))
		     ) ncolinear_at_end++,sec--;

		nctri-= ncolinear_at_begin+ncolinear_at_end;

		if(nctri<1) {
			/* no triangle left :(	*/
			freewrlDie("All in crossSection points colinear. Caps not possible!");
	 	}

		/* so we have calculated nctri for one cap, but we might have two*/
		nctri= ((beginCap)?nctri:0) + ((endCap)?nctri:0) ;
	}

	/* if we have non-convex polygons, we might need a few triangles more	*/
	/* 	The unused memory will be freed with realloc later		*/
	if(!node->convex) {

		max_ncoord_add=(nspi-1)*(nsec-1) /* because of intersections	*/
				+nctri;		/* because of cap tesselation	*/
		nctri*=2;	/* we might need more trigs for the caps	*/
	}

	/************************************************************************
	 * prepare for filling *rep
	 */

	rep_->ccw = 1;

	rep_->ntri = ntri + nctri;	/* Thats the no. of triangles representing
					the whole Extrusion Shape.		*/

	/* get some memory							*/
	cindex  = rep_->cindex   = (int *)MALLOC(sizeof(*(rep_->cindex))*3*(rep_->ntri));
	coord   = rep_->actualCoord    = (float *)MALLOC(sizeof(*(rep_->actualCoord))*(nspi*nsec+max_ncoord_add)*3);
	normal  = rep_->normal   = (float *)MALLOC(sizeof(*(rep_->normal))*3*(rep_->ntri)*3);
	norindex= rep_->norindex = (int *)MALLOC(sizeof(*(rep_->norindex))*3*(rep_->ntri));

	/* face normals - one face per quad (ie, 2 triangles) 			*/
	/* have to make sure that if nctri is odd, that we increment by one	*/


	facenormals = (struct point_XYZ *)MALLOC(sizeof(*facenormals)*(rep_->ntri+1)/2);

	/* for each triangle vertex, tell me which face(s) it is in		*/
	pointfaces = (int *)MALLOC(sizeof(*pointfaces)*POINT_FACES*3*rep_->ntri);

	/* for each triangle, it has a defaultface...				*/
	defaultface = (int *)MALLOC(sizeof(*defaultface)*rep_->ntri);


	/*memory for the SCPs. Only needed in this function. Freed later	*/
	SCP     = (struct SCP *)MALLOC(sizeof(struct SCP)*nspi);

		/* so, we now have to worry about textures. */
		/* XXX note - this over-estimates; realloc to be exact */

		tcoordsize = (nctri + (ntri*2))*3;

		#ifdef VERBOSE
			printf ("tcoordsize is %d\n",tcoordsize);
		# endif

		FREE_IF_NZ (rep_->GeneratedTexCoords);
		FREE_IF_NZ (rep_->tcindex);

		tcoord = (float *)MALLOC(sizeof(*(rep_->GeneratedTexCoords))*tcoordsize);

		tcindexsize = rep_->ntri*3;
		#ifdef VERBOSE
			printf ("tcindexsize %d\n",tcindexsize);
		#endif

		tcindex = (int *)MALLOC(sizeof(*(rep_->tcindex))*tcindexsize);

		/* keep around cross section info for tex coord mapping */
		beginVals = (float *)MALLOC(sizeof(float) * 2 * (nsec+1)*100);
		endVals = (float *)MALLOC(sizeof(float) * 2 * (nsec+1)*100);

		memset((void *)tcindex,0,tcindexsize*sizeof(*(rep_->tcindex)));
		/* printf ("zeroing tcindex\n");*/
		/* { int i; for (i=0; i<tcindexsize; i++) { tcindex[i]=0; } }*/

	/* Normal Generation Code */
	HAVETOSMOOTH = smooth_normals && (fabs(creaseAngle)>0.0001);
	for (tmp = 0; tmp < 3*rep_->ntri; tmp++) {
		pointfaces[tmp*POINT_FACES]=0;
	}


	/************************************************************************
	 * calculate all SCPs
	 */

	spine_is_one_vertex=0;

	/* fill the prev and next values in the SCP structs first
	 *
	 *	this is so complicated, because spine vertices can be the same
	 *	They should have exactly the same SCP, therefore only one of
	 *	an group of sucessive equal spine vertices (now called SESVs)
	 *	must be used for calculation.
	 *	For calculation the previous and next different spine vertex
	 *	must be known. We save that info in the prev and next fields of
	 *	the SCP struct.
	 *	Note: We have start and end SESVs which will be treated differently
	 *	depending on whether the spine is closed or not
	 *
	 */

	for(spi=0; spi<nspi;spi++){
		for(next_spi=spi+1;next_spi<nspi;next_spi++) {
			VEC_FROM_CDIFF(spine[spi],spine[next_spi],spp1);
			if(!APPROX(VECSQ(spp1),0))
				break;
		}
		if(next_spi<nspi) SCP[next_spi].prev=next_spi-1;

		#ifdef VERBOSE
			printf("spi=%d next_spi=%d\n",spi,next_spi); /**/
		#endif

		prev_spi=spi-1;
		SCP[spi].next=next_spi;
		SCP[spi].prev=prev_spi;

		while(next_spi>spi+1) { /* fill gaps */
			spi++;
			SCP[spi].next=next_spi;
			SCP[spi].prev=prev_spi;
		}
	}
	/* now:	start-SEVS .prev fields contain -1				*/
	/* 	and end-SEVS .next fields contain nspi				*/


	/* calculate the SCPs now...						*/

	#ifdef VERBOSE
		printf (" SCP[0].next = %d, nspi = %d\n",SCP[0].next,nspi);
	#endif



	if(SCP[0].next==nspi) {
		spine_is_one_vertex=1;
		#ifdef VERBOSE
			printf("All spine vertices are the same!\n");
		#endif

		/* initialize all y and z values with zero, they will		*/
		/* be treated as colinear case later then			*/
		SCP[0].z.x=0; SCP[0].z.y=0; SCP[0].z.z=0;
		SCP[0].y=SCP[0].z;
		for(spi=1;spi<nspi;spi++) {
			SCP[spi].y=SCP[0].y;
			SCP[spi].z=SCP[0].z;
		}
	}else{
		#ifdef VERBOSE
			for(spi=0;spi<nspi;spi++) {
				printf("SCP[%d].next=%d, SCP[%d].prev=%d\n",
					spi,SCP[spi].next,spi,SCP[spi].prev);
			}
		#endif

		/* find spine vertix different to the first spine vertix	*/
		spi=0;
		while(SCP[spi].prev==-1) spi++;

		/* find last spine vertix different to the last 		*/
		t=nspi-1;
		while(SCP[t].next==nspi) t--;

		#ifdef VERBOSE
			printf ("now, spi = %d, t = %d\n",spi,t);
		#endif

		/* for all but the first + last really different spine vertix	*/
		/* add case for then there are only 2 spines, and spi is already */
		/* spi is already greater than t... JAS				*/

		if (spi > t) {
			/* calc y 	*/
			VEC_FROM_CDIFF(spine[1],spine[0],SCP[0].y);
			/* calc z	*/
			VEC_FROM_CDIFF(spine[1],spine[0],spp1);
			VEC_FROM_CDIFF(spine[1],spine[0],spm1);
	 		VECCP(spp1,spm1,SCP[1].z);
			#ifdef VERBOSE
			printf ("just calculated z for spi 0\n");
			printf("SCP[0].y=[%f,%f,%f], SCP[1].z=[%f,%f,%f]\n",
				SCP[0].y.x,SCP[0].y.y,SCP[0].y.z,
				SCP[1].z.x,SCP[1].z.y,SCP[1].z.z);
			#endif
		}

		else {
			for(; spi<=t; spi++) {
				/* calc y 	*/
				VEC_FROM_CDIFF(spine[SCP[spi].next],spine[SCP[spi].prev],SCP[spi].y);
				/* calc z	*/
				VEC_FROM_CDIFF(spine[SCP[spi].next],spine[spi],spp1);
				VEC_FROM_CDIFF(spine[SCP[spi].prev],spine[spi],spm1);
	 			VECCP(spp1,spm1,SCP[spi].z);
				#ifdef VERBOSE
					printf ("just calculated z for spi %d\n",spi);
				#endif
	 		}
		}

	 	if(circular) {
			#ifdef VERBOSE
				printf ("we are circular\n");
			#endif
	 		/* calc y for first SCP				*/
			VEC_FROM_CDIFF(spine[SCP[0].next],spine[SCP[nspi-1].prev],SCP[0].y);
	 		/* the last is the same as the first */
	 		SCP[nspi-1].y=SCP[0].y;

			/* calc z */
			VEC_FROM_CDIFF(spine[SCP[0].next],spine[0],spp1);
			VEC_FROM_CDIFF(spine[SCP[nspi-1].prev],spine[0],spm1);
			VECCP(spp1,spm1,SCP[0].z);
			/* the last is the same as the first */
			SCP[nspi-1].z=SCP[0].z;

	 	} else {
			#ifdef VERBOSE
				printf ("we are not circular\n");
			#endif

	 		/* calc y for first SCP				*/
			VEC_FROM_CDIFF(spine[SCP[0].next],spine[0],SCP[0].y);

	 		/* calc y for the last SCP			*/
			/* in the case of 2, nspi-1 = 1, ...prev = 0	*/
			VEC_FROM_CDIFF(spine[nspi-1],spine[SCP[nspi-1].prev],SCP[nspi-1].y);

			/* z for the start SESVs is the same as for the next SCP */
			SCP[0].z=SCP[SCP[0].next].z;
	 		/* z for the last SCP is the same as for the one before the last*/
			SCP[nspi-1].z=SCP[SCP[nspi-1].prev].z;

			#ifdef VERBOSE
			printf("SCP[0].y=[%f,%f,%f], SCP[0].z=[%f,%f,%f]\n",
				SCP[0].y.x,SCP[0].y.y,SCP[0].y.z,
				SCP[0].z.x,SCP[0].z.y,SCP[0].z.z);
			printf("SCP[1].y=[%f,%f,%f], SCP[1].z=[%f,%f,%f]\n",
				SCP[1].y.x,SCP[1].y.y,SCP[1].y.z,
				SCP[1].z.x,SCP[1].z.y,SCP[1].z.z);
			#endif
		} /* else */

		/* fill the other start SESVs SCPs*/
		spi=1;
		while(SCP[spi].prev==-1) {
			SCP[spi].y=SCP[0].y;
			SCP[spi].z=SCP[0].z;
			spi++;
		}
		/* fill the other end SESVs SCPs*/
		t=nspi-2;
		while(SCP[t].next==nspi) {
			SCP[t].y=SCP[nspi-1].y;
			SCP[t].z=SCP[nspi-1].z;
			t--;
		}

	} /* else */


	/* We have to deal with colinear cases, what means z=0			*/
	pos_of_last_zvalue=-1;		/* where a zvalue is found */
	for(spi=0;spi<nspi;spi++) {
		if(pos_of_last_zvalue>=0) { /* already found one?		*/
			if(APPROX(VECSQ(SCP[spi].z),0))
				SCP[spi].z= SCP[pos_of_last_zvalue].z;

			pos_of_last_zvalue=spi;
		} else
			if(!APPROX(VECSQ(SCP[spi].z),0)) {
				/* we got the first, fill the previous		*/
				#ifdef VERBOSE
					printf("Found z-Value!\n");
				#endif

				for(t=spi-1; t>-1; t--)
					SCP[t].z=SCP[spi].z;
	 			pos_of_last_zvalue=spi;
			}
	}

	#ifdef VERBOSE
		printf("pos_of_last_zvalue=%d\n",pos_of_last_zvalue);
	#endif


	/* z axis flipping, if VECPT(SCP[i].z,SCP[i-1].z)<0 			*/
	/* we can do it here, because it is not needed in the all-colinear case	*/
	for(spi=(circular?2:1);spi<nspi;spi++) {
		if(VECPT(SCP[spi].z,SCP[spi-1].z)<0) {
			VECSCALE(SCP[spi].z,-1);
			#ifdef VERBOSE
			    printf("Extrusion.GenPloyRep: Flipped axis spi=%d\n",spi);
			#endif
		}
	} /* for */

	/* One case is missing: whole spine is colinear				*/
	if(pos_of_last_zvalue==-1) {

		#ifdef VERBOSE
			printf("Extrusion.GenPloyRep:Whole spine is colinear!\n");
		#endif

		/* this is the default, if we don`t need to rotate		*/
		spy.x=0; spy.y=1; spy.z=0;
		spz.x=0; spz.y=0; spz.z=1;

		if(!spine_is_one_vertex) {
			compute_spy_spz(&spy,&spz,spine,nspi);
		}

		#ifdef VERBOSE
		printf ("so, spy [%f %f %f], spz [%f %f %f]\n", spy.x, spy.y,spy.z, spz.x, spz.y, spz.z);
		#endif

		/* apply new y and z values to all SCPs	*/
		for(spi=0;spi<nspi;spi++) {
			SCP[spi].y=spy;
			SCP[spi].z=spz;
		}

	} /* if all colinear */

	#ifdef VERBOSE
		for(spi=0;spi<nspi;spi++) {
			printf("SCP[%d].y=[%f,%f,%f], SCP[%d].z=[%f,%f,%f]\n",
				spi,SCP[spi].y.x,SCP[spi].y.y,SCP[spi].y.z,
				spi,SCP[spi].z.x,SCP[spi].z.y,SCP[spi].z.z);
		}
	#endif


	/************************************************************************
	 * calculate the coords
	 */

	/* test for number of scale and orientation parameters			*/
	if(nsca>1 && nsca <nspi)
		printf("Extrusion.GenPolyRep: Warning!\n"
		"\tNumber of scaling parameters do not match the number of spines!\n"
		"\tWill revert to using only the first scale value.\n");

	if(nori>1 && nori <nspi)
		printf("Extrusion.GenPolyRep: Warning!\n"
		"\tNumber of orientation parameters "
			"do not match the number of spines!\n"
		"\tWill revert to using only the first orientation value.\n");


	for(spi = 0; spi<nspi; spi++) {
		double m[3][3];		/* space for the rotation matrix	*/
		spy=SCP[spi].y; 
		spz=SCP[spi].z;
		VECCP(spy,spz,spx);
		spylen = 1/sqrt(VECSQ(spy)); VECSCALE(spy, spylen);
		spzlen = 1/sqrt(VECSQ(spz)); VECSCALE(spz, spzlen);
		spxlen = 1/sqrt(VECSQ(spx)); VECSCALE(spx, spxlen);

		/* rotate spx spy and spz			*/
		if(nori) {
			int ori = (nori==nspi ? spi : 0);

			if(IS_ROTATION_VEC_NOT_NORMAL(orientation[ori]))
				printf("Extrusion.GenPolyRep: Warning!\n"
				  "\tRotationvector #%d not normal!\n"
				  "\tWon`t correct it, because it is bad VRML`97.\n",
				  ori+1);

			MATRIX_FROM_ROTATION(orientation[ori],m);
			VECMM(m,spx);
			VECMM(m,spy);
			VECMM(m,spz);
		}

		for(sec = 0; sec<nsec; sec++) {
			struct point_XYZ point;
			float ptx = crossSection[sec].c[0];
			float ptz = crossSection[sec].c[1];
			if(nsca) {
				int sca = (nsca==nspi ? spi : 0);
				ptx *= node->scale.p[sca].c[0];
				ptz *= node->scale.p[sca].c[1];
	 		}
			point.x = ptx;
			point.y = 0;
			point.z = ptz;

			/* printf ("working on sec %d of %d, spine %d of %d\n", sec, nsec, spi, nspi);*/


		  /* texture mapping for caps - keep vals around */
		  	if (spi == 0) { /* begin cap vertices */
				/* printf ("begin cap vertecies index %d %d \n", sec*2+0, sec*2+1); */

				beginVals[sec*2+0] = ptx;
				beginVals[sec*2+1] = ptz;
		   	} else if (spi == (nspi-1)) {  /* end cap vertices */
				/* printf ("end cap vertecies index %d %d size %d\n", sec*2+0, sec*2+1, 2 * (nsec+1));*/
				endVals[(sec*2)+0]=ptx;
				endVals[(sec*2)+1]=ptz;
		   	}

		   /* printf ("coord index %x sec %d spi %d nsec %d\n",*/
		   /* 		&coord[(sec+spi*nsec)*3+0], sec, spi,nsec);*/

		   coord[(sec+spi*nsec)*3+0] =
		    spx.x * point.x + spy.x * point.y + spz.x * point.z
		    + node->spine.p[spi].c[0];
		   coord[(sec+spi*nsec)*3+1] =
		    spx.y * point.x + spy.y * point.y + spz.y * point.z
		    + node->spine.p[spi].c[1];
		   coord[(sec+spi*nsec)*3+2] =
		    spx.z * point.x + spy.z * point.y + spz.z * point.z
		    + node->spine.p[spi].c[2];

		} /* for(sec */
	} /* for(spi */
	ncoord=nsec*nspi;


	/* freeing SCP coordinates. not needed anymore.				*/
	FREE_IF_NZ (SCP);

	/************************************************************************
	 * setting the values of *cindex to the right coords
	 */

	triind = 0;
	{
	int x,z;
	int A,B,C,D; /* should referr to the four vertices of the polygon
			(hopefully) counted counter-clockwise, like

			 D----C
			 |    |
			 |    |
			 |    |
			 A----B

			*/
	int Atex, Btex, Ctex, Dtex, Etex, Ftex;	/* Tex Coord points */

	struct point_XYZ ac,bd,	/* help vectors	*/
		ab,cd;		/* help vectors	for testing intersection */
	int E,F;		/* third point to be used for the triangles*/
	double u,r,		/* help variables for testing intersection */
		denominator,	/* ... */
		numerator;	/* ... */

	#ifdef VERBOSE
		printf("Coords: \n");

		for(x=0; x<nsec; x++) {
		 for(z=0; z<nspi; z++) {
		 	int xxx = 3*(x+z*nsec);
		 	printf("coord: %d [%f %f %f] ",(x+z*nsec),
				coord[xxx], coord[xxx+1], coord[xxx+2]);

		 }
		printf("\n");
		}
		printf("\n");
	#endif


	/* Now, lay out the spines/sections, and generate triangles */

	for(x=0; x<nsec-1; x++) {
	  for(z=0; z<nspi-1; z++) {
	  A=x+z*nsec;
	  B=(x+1)+z*nsec;
	  C=(x+1)+(z+1)*nsec;
	  D= x+(z+1)*nsec;

	  /* texture mapping coords */
	  Atex = A; Btex = B; Ctex = C; Dtex = D;

	  /* if we are circular, check to see if this is the first tri, or the last */
	  /* the vertexes are identical, but for smooth normal calcs, make the    */
	  /* indexes the same, too                                                */
	  /* note, we dont touch tex coords here.				  */
	  /*  printf ("x %d z %d nsec %d nspi %d\n",x,z,nsec,nspi);*/

	  if (tubular) {
		/* printf ("tubular, x %d nsec %d this_face %d\n",x,nsec,this_face);*/
		if (x==(nsec-2)) {
			B -=(x+1);
			C -=(x+1);
		}
	  }

	  if (circular) {
		if (z==(nspi-2)) {
			/* last row in column, assume z=nspi-2, subtract this off */
			C -= (z+1)*nsec;
			D -= (z+1)*nsec;
		}
	  }

	  /* calculate the distance A-C and see, if it is smaller as B-D	*/
	  VEC_FROM_COORDDIFF(coord,C,coord,A,ac);
	  VEC_FROM_COORDDIFF(coord,D,coord,B,bd);

	  if(sqrt(VECSQ(ac))>sqrt(VECSQ(bd))) {
	  	E=B; F=D; Etex=Btex; Ftex=Dtex;
	  } else {
	  	E=C; F=A; Etex=Ctex; Ftex=Atex;
	  }

	  /* if concave polygons are expected, we also expect intersecting ones
	  	so we are testing, whether A-B and D-C intersect	*/
	  if(!node->convex) {
	    	VEC_FROM_COORDDIFF(coord,B,coord,A,ab);
	  	VEC_FROM_COORDDIFF(coord,D,coord,C,cd);
		/* ca=-ac */
		#ifdef VERBOSE
			printf("ab=[%f,%f,%f],cd=[%f,%f,%f]\n",
				ab.x,ab.y,ab.z,cd.x,cd.y,cd.z);
			printf("Orig: %d %d  [%f %f %f] [%f %f %f] (%d, %d, %d) \n",
					D, C,
					coord[D*3], coord[D*3+1], coord[D*3+2],
					coord[C*3], coord[C*3+1], coord[C*3+2],
					ncoord, nsec, nspi
			);
		#endif

		denominator= ab.y*cd.x-ab.x*cd.y;
		numerator  = (-ac.x)*cd.y-(-ac.y)*cd.x;

		r=u=-1;
		if(!APPROX(denominator,0)) {
			u=numerator/denominator;
			r=((-ac.x)*ab.y-(-ac.y)*ab.x)/denominator;
		} else {
			/* lines still may be coincident*/
			if(APPROX(numerator,0)) {
				/* we have to calculate u and r using the z coord*/
				denominator=ab.z*cd.x-ab.x*cd.z;
				numerator  = (-ac.x)*cd.z-(-ac.z)*cd.x;
				if(!APPROX(denominator,0)) {
				u=numerator/denominator;
				r=((-ac.x)*ab.y-(-ac.y)*ab.x)/denominator;
				}
			}
		} /* else */
		#ifdef VERBOSE
			printf("u=%f, r=%f\n",u,r);
		#endif

		if(u>=0 && u<=1 && r>=0 && r<=1
			&& APPROX((-ac.x)+u*ab.x,r*cd.x)
			&& APPROX((-ac.y)+u*ab.y,r*cd.y)
			&& APPROX((-ac.z)+u*ab.z,r*cd.z)) {

			#ifdef VERBOSE
				printf("Intersection found at P=[%f,%f,%f]!\n",
				coord[A*3]+u*ab.x,
				coord[A*3+1]+u*ab.y,
				coord[A*3+2]+u*ab.y
				);
			#endif

			coord[(ncoord)*3  ]=coord[A*3  ]+u*ab.x;
			coord[(ncoord)*3+1]=coord[A*3+1]+u*ab.y;
			coord[(ncoord)*3+2]=coord[A*3+2]+u*ab.z;
			E=ncoord;
			F=ncoord;
			ncoord_add++;
			ncoord++;
		}

	  }

	   /* printf ("tcindex %d\n",tcindex);*/
	   /* printf ("Triangle1 %d %d %d\n",D,A,E);*/
	  /* first triangle  calculate pointfaces, etc, for this face */
	  Elev_Tri(triind*3, this_face, D,A,E, TRUE , rep_, facenormals, pointfaces,ccw);

		tcindex[triind*3] = Dtex;
		tcindex[triind*3+2] = Etex;
		tcindex[triind*3+1] = Atex;

	  defaultface[triind] = this_face;
	  triind++;

	   /* printf ("Triangle2 %d %d %d\n",B,C,F);*/
	  /* second triangle - pointfaces, etc,for this face  */
	  Elev_Tri(triind*3, this_face, B, C, F, TRUE, rep_, facenormals, pointfaces,ccw);

		tcindex[triind*3] = Btex;
		tcindex[triind*3+1] = Ctex;
		tcindex[triind*3+2] = Ftex;

	  if ((triind*3+2) >= tcindexsize)
		printf ("INTERNAL ERROR: Extrusion  - tcindex size too small!\n");
	  defaultface[triind] = this_face;
	  triind ++;
	  this_face ++;

	 }
	}

	/* do normal calculations for the sides, here */
	for (tmp=0; tmp<(triind*3); tmp++) {
		if (HAVETOSMOOTH) {
			normalize_ifs_face (&rep_->normal[tmp*3],
				facenormals, pointfaces, cindex[tmp],
				defaultface[tmp/3], creaseAngle);
		} else {
			rep_->normal[tmp*3+0] = facenormals[defaultface[tmp/3]].x;
			rep_->normal[tmp*3+1] = facenormals[defaultface[tmp/3]].y;
			rep_->normal[tmp*3+2] = facenormals[defaultface[tmp/3]].z;
		}
		rep_->norindex[tmp] = tmp;
	}
	/* keep track of where the sides end, triangle count-wise, for Normal mapping */
	end_of_sides = triind*3;

	/* tcindexes are TOTALLY different from sides  - set this in case we are
	   doing textures in the end caps */
	tci_ct = nspi*nsec;

	if(node->convex) {
		int endpoint;

		int triind_start; 	/* textures need 2 passes */

		/* if not tubular, we need one more triangle */
		if (tubular) endpoint = nsec-3-ncolinear_at_end;
		else endpoint = nsec-2-ncolinear_at_end;


		/* printf ("beginCap, starting at triind %d\n",triind);*/

		/* this is the simple case with convex polygons	*/
		if(beginCap) {
			triind_start = triind;

			for(x=0+ncolinear_at_begin; x<endpoint; x++) {
	  			Elev_Tri(triind*3, this_face, 0, x+2, x+1, TRUE , rep_, facenormals, pointfaces,ccw);
	  			defaultface[triind] = this_face;
				Extru_tex(triind*3, tci_ct, 0 , +x+2, x+1, tcindex ,ccw,tcindexsize);
				triind ++;
			}

			Extru_ST_map(triind_start,0+ncolinear_at_begin,endpoint,
				beginVals,nsec,tcindex, cindex, tcoord, tcoordsize);
			tci_ct+=endpoint-(0+ncolinear_at_begin);
			triind_start+=endpoint-(0+ncolinear_at_begin);
			this_face++;
		} /* if beginCap */

		if(endCap) {
			triind_start = triind;

			for(x=0+ncolinear_at_begin; x<endpoint; x++) {
	  			Elev_Tri(triind*3, this_face, 0  +(nspi-1)*nsec,
					x+1+(nspi-1)*nsec,x+2+(nspi-1)*nsec,
					TRUE , rep_, facenormals, pointfaces,ccw);
	  			defaultface[triind] = this_face;
				Extru_tex(triind*3, tci_ct, 0+(nspi-1)*nsec,
					x+1+(nspi-1)*nsec,
					x+2+(nspi-1)*nsec, 
					tcindex,ccw,tcindexsize);
				triind ++;
			}
			this_face++;
			Extru_ST_map(triind_start,0+ncolinear_at_begin,endpoint,
					endVals, nsec, tcindex, cindex, tcoord, tcoordsize);
		} /* if endCap */
	 	/* for (tmp=0;tmp<tcindexsize; tmp++) printf ("index1D %d tcindex %d\n",tmp,tcindex[tmp]);*/

	} else
	    if(beginCap || endCap) {
		/* polygons might be concave-> do tessellation			*/
		/* XXX - no textures yet - Linux Tesselators give me enough headaches;
		   lets wait until they are all ok before trying texture mapping */

		/* give us some memory - this array will contain tessd triangle counts */
		int *tess_vs;
		struct SFColor *c1;
		GLdouble tess_v[3];
		int endpoint;

		tess_vs=(int *)MALLOC(sizeof(*(tess_vs)) * (nsec - 3 - ncolinear_at_end) * 3);

		/* if not tubular, we need one more triangle */
		if (tubular) endpoint = nsec-1-ncolinear_at_end;
		else endpoint = nsec-ncolinear_at_end;


		if (beginCap) {
			global_IFS_Coord_count = 0;
			gluBeginPolygon(global_tessobj);

			for(x=0+ncolinear_at_begin; x<endpoint; x++) {
				/* printf ("starting tv for x %d of %d\n",x,endpoint);*/
	                	c1 = (struct SFColor *) &rep_->actualCoord[3*x];
				/* printf ("and, coords for this one are: %f %f %f\n",*/
				/* 		c1->c[0], c1->c[1],c1->c[2]);*/

				tess_v[0] = c1->c[0]; tess_v[1] = c1->c[1]; tess_v[2] = c1->c[2];
				tess_vs[x] = x;
				gluTessVertex(global_tessobj,tess_v,&tess_vs[x]);
			}
			gluEndPolygon(global_tessobj);
			verify_global_IFS_Coords(ntri*3);

			for (x=0; x<global_IFS_Coord_count; x+=3) {
				/* printf ("now, in 2nd for loop, x %d glob %d\n",x,*/
				/* 		global_IFS_Coord_count);*/
	  			Elev_Tri(triind*3, this_face, global_IFS_Coords[x],
					global_IFS_Coords[x+2], global_IFS_Coords[x+1],
					TRUE , rep_, facenormals, pointfaces,ccw);
	  			defaultface[triind] = this_face;
				triind ++;
			}
			/* Tesselated faces may have a different normal than calculated previously */
			Extru_check_normal (facenormals,this_face,-1,rep_,ccw);

			this_face++;
		}

		if (endCap) {
			global_IFS_Coord_count = 0;
			gluBeginPolygon(global_tessobj);

			for(x=0+ncolinear_at_begin; x<endpoint; x++) {
	                	c1 = (struct SFColor *) &rep_->actualCoord[3*(x+(nspi-1)*nsec)];
				tess_v[0] = c1->c[0]; tess_v[1] = c1->c[1]; tess_v[2] = c1->c[2];
				tess_vs[x] = x+(nspi-1)*nsec;
				gluTessVertex(global_tessobj,tess_v,&tess_vs[x]);
			}
			gluEndPolygon(global_tessobj);
			verify_global_IFS_Coords(ntri*3);

			for (x=0; x<global_IFS_Coord_count; x+=3) {
	  			Elev_Tri(triind*3, this_face, global_IFS_Coords[x],
					global_IFS_Coords[x+1], global_IFS_Coords[x+2],
					TRUE , rep_, facenormals, pointfaces,ccw);
	  			defaultface[triind] = this_face;
				triind ++;
			}
			/* Tesselated faces may have a different normal than calculated previously */
			Extru_check_normal (facenormals,this_face,1,rep_,ccw);

			this_face++;
		}

		/* get rid of MALLOCd memory  for tess */
		FREE_IF_NZ (tess_vs);

	    } /* elseif */
	} /* end of block */

	/* if we have tesselated, we MAY have fewer triangles than estimated, so... */
	rep_->ntri=triind;

	/* for (tmp=0;tmp<tcindexsize; tmp++) printf ("index2 %d tcindex %d\n",tmp,tcindex[tmp]);*/
	/* do normal calculations for the caps here note - no smoothing */
	for (tmp=end_of_sides; tmp<(triind*3); tmp++) {
		rep_->normal[tmp*3+0] = facenormals[defaultface[tmp/3]].x;
		rep_->normal[tmp*3+1] = facenormals[defaultface[tmp/3]].y;
		rep_->normal[tmp*3+2] = facenormals[defaultface[tmp/3]].z;
		rep_->norindex[tmp] = tmp;
	}

	/* do texture mapping calculations for sides */
		/* range check - this should NEVER happen... */
		if (tcoordsize <= ((nsec-1)+(nspi-1)*(nsec-1)*3+2)) {
			printf ("INTERNAL ERROR: Extrusion side tcoord calcs nspi %d nsec %d tcoordsize %d\n",
				nspi,nsec,tcoordsize);
		}
		for(sec=0; sec<nsec; sec++) {
			for(spi=0; spi<nspi; spi++) {
				/* printf ("tcoord idx %d %d %d tcoordsize %d ",*/
				/* (sec+spi*nsec)*3,(sec+spi*nsec)*3+1,(sec+spi*nsec)*3+2,tcoordsize);*/
				/* printf ("side texts sec %d spi %d\n",sec,spi);*/
				tcoord[(sec+spi*nsec)*3+0] = (float) sec/(nsec-1);
				tcoord[(sec+spi*nsec)*3+1] = 0;
				tcoord[(sec+spi*nsec)*3+2] = (float) spi/(nspi-1);
				/* printf (" %f %f\n",tcoord[(sec+spi*nsec)*3+0],tcoord[(sec+spi*nsec)*3+2]);*/
			}
		}

	#ifdef VERBOSE
		printf ("done, lets free\n");
	#endif

	/* we no longer need to keep normal-generating memory around */
	FREE_IF_NZ (defaultface);
	FREE_IF_NZ (pointfaces);
	FREE_IF_NZ (facenormals);
	FREE_IF_NZ (crossSection);

	FREE_IF_NZ (beginVals);
	FREE_IF_NZ (endVals);


	/* stream the texture coords so that they are linear as tcindex is not used in stream_polyrep */
	stream_extrusion_texture_coords (rep_, tcoord, tcindex);

	/* now that the tex coords are streamed, remove the temoporary arrays */
	FREE_IF_NZ (tcoord);
	FREE_IF_NZ (tcindex);


	#ifdef VERBOSE
		printf("Extrusion.GenPloyRep: triind=%d  ntri=%d nctri=%d "
		"ncolinear_at_begin=%d ncolinear_at_end=%d\n",
		triind,ntri,nctri,ncolinear_at_begin,ncolinear_at_end);

		printf ("end VRMLExtrusion.pm\n");
	#endif

	/*****end of Member Extrusion	*/
}
