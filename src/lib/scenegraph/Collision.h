/*
=INSERT_TEMPLATE_HERE=

$Id: Collision.h,v 1.13 2011/08/22 15:34:27 crc_canada Exp $

Collision ???

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


#ifndef __FREEWRL_COLLISION_H__
#define __FREEWRL_COLLISION_H__


/* Collision detection results structure*/
struct sCollisionInfo {
    struct point_XYZ Offset;
    int Count;
    double Maximum2; /*squared. so we only need to root once */
};

typedef int prflags;
#define PR_DOUBLESIDED 0x01
#define PR_FRONTFACING 0x02 /* overrides effect of doublesided. */
#define PR_BACKFACING 0x04 /* overrides effect of doublesided, all normals are reversed. */
#define PR_NOSTEPING 0x08 /* gnores stepping. used internally */


/*uncomment this to enable the scene exporting functions */
/*#define DEBUG_SCENE_EXPORT*/

double
closest_point_of_segment_to_y_axis(struct point_XYZ p1,
								   struct point_XYZ p2);
double
closest_point_of_segment_to_origin(struct point_XYZ p1,
								   struct point_XYZ p2);

struct point_XYZ
closest_point_of_plane_to_origin(struct point_XYZ b,
								 struct point_XYZ n);

int
intersect_segment_with_line_on_yplane(struct point_XYZ* pk,
									  struct point_XYZ p1,
									  struct point_XYZ p2,
									  struct point_XYZ q1,
									  struct point_XYZ q2);

int
getk_intersect_line_with_ycylinder(double* k1,
								   double* k2,
								   double r,
								   struct point_XYZ pp1,
								   struct point_XYZ n);

int
project_on_cylindersurface(struct point_XYZ* res,
						   struct point_XYZ p,
						   struct point_XYZ n,
						   double r);

int
getk_intersect_line_with_sphere(double* k1,
								double* k2,
								double r,
								struct point_XYZ pp1,
								struct point_XYZ n);

int
project_on_spheresurface(struct point_XYZ* res,
						 struct point_XYZ p,
						 struct point_XYZ n,
						 double r);

struct point_XYZ
project_on_yplane(struct point_XYZ p1,
				  struct point_XYZ n,
				  double y);

struct point_XYZ
project_on_cylindersurface_plane(struct point_XYZ p,
								 struct point_XYZ n,
								 double r);

int
perpendicular_line_passing_inside_poly(struct point_XYZ a,
									   struct point_XYZ* p,
									   int num);

int
getk_intersect_segment_with_ycylinder(double* k1,
									  double* k2,
									  double r,
									  struct point_XYZ pp1,
									  struct point_XYZ pp2);

int
helper_poly_clip_cap(struct point_XYZ* clippedpoly,
					 int clippedpolynum,
					 const struct point_XYZ* p,
					 int num,
					 double r,
					 struct point_XYZ n,
					 double y,
					 int stepping);

struct point_XYZ
polyrep_disp_rec(double y1,
				 double y2,
				 double ystep,
				 double r,
				 struct X3D_PolyRep* pr,
				 struct point_XYZ* n,
				 struct point_XYZ dispsum,
				 prflags flags);

struct point_XYZ
planar_polyrep_disp_rec(double y1,
						double y2,
						double ystep,
						double r,
						struct X3D_PolyRep* pr,
						struct point_XYZ n,
						struct point_XYZ dispsum,
						prflags flags);

int
helper_line_clip_cap(struct point_XYZ* clippedpoly,
					 int clippedpolynum,
					 struct point_XYZ p1,
					 struct point_XYZ p2,
					 double r,
					 struct point_XYZ n,
					 double y,
					 int stepping);

/*accumulator function, for displacements. */
void accumulate_disp(struct sCollisionInfo* ci, struct point_XYZ add);

/*returns (1-k)p1 + k p2 */
struct point_XYZ weighted_sum(struct point_XYZ p1, struct point_XYZ p2, double k);

/*feed a poly, and stats of a cylinder, it returns the displacement in the direction of the
  normal of the poly that is needed for them not to intersect any more.*/
/* struct point_XYZ get_poly_normal_disp(double y1, double y2, double r, struct point_XYZ* p, int num, struct point_XYZ n); */

/*feed a poly, and stats of a cylinder, it returns the vertical displacement that is needed for them not to intersect any more,
  if this displacement is less than the height of the cylinder (y2-y1).*/
struct point_XYZ get_poly_step_disp(double y1, double y2, double r, struct point_XYZ* p, int num, struct point_XYZ n);

/*feed a poly, and stats of a cylinder, it returns the displacement in the direction of the
  normal of the poly that is needed for them not to intersect any more, or vertically if contact point below ystep*/
struct point_XYZ get_poly_disp(double y1, double y2, double ystep, double r, struct point_XYZ* p, int num, struct point_XYZ n);

/*feed a poly, and radius of a sphere, it returns the displacement in the direction of the
  normal of the poly that is needed for them not to intersect any more.*/
struct point_XYZ get_poly_normal_disp_with_sphere(double r, struct point_XYZ* p, int num, struct point_XYZ n);
/*feed a poly, and radius of a sphere, it returns the minimum displacement and
  the direction that is needed for them not to intersect any more.*/
/* struct point_XYZ get_poly_min_disp_with_sphere(double r, struct point_XYZ* p, int num, struct point_XYZ n); */

/*feed a line and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal that is needed for them not to intersect any more.*/
struct point_XYZ get_line_normal_disp(double y1, double y2, double r, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ n);

/*feed a line and a normal, and stats of a cylinder, it returns the vertical displacement
  that is needed for them not to intersect any more.*/
struct point_XYZ get_line_step_disp(double y1, double y2, double r, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ n);

/*feed a line and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal, or the vertical displacement(in case of stepping) that is needed for them not to intersect any more.*/
struct point_XYZ get_line_disp(double y1, double y2, double ystep, double r, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ n);

/*feed a point and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal that is needed for them not to intersect any more.*/
struct point_XYZ get_point_normal_disp(double y1, double y2, double r, struct point_XYZ p1, struct point_XYZ n);

/*feed a point and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal, or the vertical displacement(in case of stepping) that is needed for them not to intersect any more.*/
struct point_XYZ get_point_disp(double y1, double y2, double ystep, double r, struct point_XYZ p1, struct point_XYZ n);

/*feed a box (a corner, and the three vertice sides) and the stats of a cylinder, it returns the
  displacement of the box that is needed for them not to intersect any more, with optionnal stepping displacement */
struct point_XYZ box_disp(double y1, double y2, double ystep, double r,struct point_XYZ p0, struct point_XYZ i, struct point_XYZ j, struct point_XYZ k);

/*fast test to see if a box intersects a y-cylinder.
 * gives false positives */
int fast_ycylinder_box_intersect(double y1, double y2, double r,struct point_XYZ pcenter, double xs, double ys, double zs);


/*fast test to see if the min/max of a polyrep structure (IndexedFaceSet, eg)  intersects a y-cylinder.
 * gives false positives */
int fast_ycylinder_polyrep_intersect(double y1, double y2, double r,struct point_XYZ pcenter, double scale, struct X3D_PolyRep *pr);

/*fast test to see if a cone intersects a y-cylinder. */
/*gives false positives. */
int fast_ycylinder_cone_intersect(double y1, double y2, double r,struct point_XYZ pcenter, double halfheight, double baseradius);

/* fast test to see if a sphere intersects a y-cylinder.
   specify sphere center, and a point on it's surface
  gives false positives. */
int fast_ycylinder_sphere_intersect(double y1, double y2, double r,struct point_XYZ pcenter, struct point_XYZ psurface);


/*algorithm is approximative */
/*basically, it does collision with a triangle on a plane that passes through the origin.*/
struct point_XYZ cone_disp(double y1, double y2, double ydisp, double r, struct point_XYZ base, struct point_XYZ top, double baseradius);

/*algorithm is approximative */
/*basically, it does collision with a rectangle on a plane that passes through the origin.*/
struct point_XYZ cylinder_disp(double y1, double y2, double ydisp, double r, struct point_XYZ base, struct point_XYZ top, double baseradius);

/*uses sphere displacement, and a cylinder for stepping */
//struct point_XYZ polyrep_disp(double y1, double y2, double ydisp, double r, struct X3D_PolyRep pr, GLDOUBLE* mat, prflags flags);
struct point_XYZ polyrep_disp2(struct X3D_PolyRep pr, GLDOUBLE* mat, prflags flags);

/*displacement when the polyrep structure is all in the same plane
  if normal is zero, it will be calculated form the first triangle*/
struct point_XYZ planar_polyrep_disp(double y1, double y2, double ydisp, double r, struct X3D_PolyRep pr, GLDOUBLE* mat, prflags flags, struct point_XYZ n);

struct point_XYZ elevationgrid_disp( double y1, double y2, double ydisp, double r, struct X3D_PolyRep pr,
			      int xdim, int zdim, double xs, double zs, GLDOUBLE* mat, prflags flags);

/* functions VERY usefull for debugging purposes
   Use these inside FreeWRL to export a scene to
   the debugging programs. */
#ifdef DEBUG_SCENE_EXPORT
void printpolyrep(struct X3D_PolyRep pr, int npoints);

void printmatrix(GLDOUBLE* mat);
#endif

#define VIEWER_WALK 2
//int viewer_type = VIEWER_WALK; // force to walking
struct sFallInfo
{
	double fallHeight; /*[100.0] a setting - the maximum you want to search for ground beneath before giving up and staying at your current level */
	double fallStep; /*[1.0] a setting - how much maximum on a frame to fall ie so it's not 1 frame to fall all the way, you can spread it out */
	double hfall;  /*if canFall && isFall then this is how far to fall to hit ground, in collision space dist +down */
	double hclimb; /* if isClimb then (similar to hfall) this is how far to climb to get back on top of the ground - redundant with cylinder collisions, so this is a primitive thunk */
	int isFall; /* true if there's ground underneath (within fallHeight) to fall to, and no climb is registered (isClimb is false) */
	int canFall; /* true if WALKING && COLLISION, set in render_pre() */
	int isClimb; /* true if avatar feet are below ground, in which case add hclimb in collision space */
	int hits; /* counter of vertical intersections found per frame*/
	int walking; /* true if viewer_type == VIEWER_WALK, initialize before doing collision detection */
	int smoothStep; /* [1] setting - will only fall by fallstep on a frame rather than the full hfall */
	int verticalOnly; /* [0] - setting - will over-ride cylindrical collision and do only fall/climb */
	int allowClimbing; /* [0] - setting - will allow climbing in which case cyclindrical Y collision is over-ridden */
	/* the following could be moved to sCollisionInfo */
	GLDOUBLE collision2avatar[16], avatar2collision[16]; /* fly/examine: Identity, walk: BVVA2A, A2BVVA (BVVA bound-viewpoint-vertical avatar-centric) see viewer.c  */
	int gravityVectorMethod; //[1] - setting -  0=old method 1=new method either bound viewpoint down or avatar down
	int fastTestMethod; //[2] - setting -0=old method - uses fast cylinder test 1= MBB shape space 2= MBB avatar space 3=ignor fast cylinder test and keep going 
	int checkFall;
	int checkCylinder;
	int checkPenetration;
	int walkColliderMethod; /* 0=sphere 1=normal_cylinder 2=disp_ 3=sampler */
	int canPenetrate; /* setting 1 will check for wall penetration */
	int isPenetrate; /* initialize to 0 once per frame, will return as 1 if a wall penetration was found */
	GLDOUBLE penMin[3], penMax[3]; /* MBB of scaled penetration vector (penRadius x penvec) - initialize once per frame */
	struct point_XYZ penvec; /* normalized (unit) vector from avatar(0,0,0) to last valid avatar position ie on last frame/loop */
	double penRadius; /* distance from avatar(0,0,0) to last avatar position */
	struct point_XYZ pencorrection; /* if isPenetration, this will hold the displacement vector to apply to the avatar position to un penetrate */
	double pendisp; /* set to zero once per frame, used to sort (pick) penetration intersection closest to last position */
};

int fast_ycylinder_box_intersect(double y1, double y2, double r,struct point_XYZ pcenter, double xs, double ys, double zs);
int fast_sphere_MBB_intersect_shapeSpace(double r, GLDOUBLE *collision2shape, GLDOUBLE *shapeMBBmin, GLDOUBLE *shapeMBBmax );
int fast_ycylinder_MBB_intersect_shapeSpace(double y1, double y2, double r, GLDOUBLE *collision2shape, GLDOUBLE *shapeMBBmin, GLDOUBLE *shapeMBBmax );
int fast_ycylinder_MBB_intersect_collisionSpace(double y1, double y2, double r, GLDOUBLE *shape2collision, GLDOUBLE *shapeMBBmin, GLDOUBLE *shapeMBBmax );
int fast_sphere_MBB_intersect_collisionSpace(double r, GLDOUBLE *shape2collision, GLDOUBLE *shapeMBBmin, GLDOUBLE *shapeMBBmax );
int overlapMBBs(GLDOUBLE *MBBmin1, GLDOUBLE *MBBmax1, GLDOUBLE *MBBmin2, GLDOUBLE* MBBmax2);
int fast_ycylinder_polyrep_intersect2(double y1, double y2, double AVr,struct point_XYZ pcenter, double scale, double *minVals, double *maxVals);

#ifdef DO_COLLISION_GPU
struct point_XYZ run_collide_program(GLuint vertex_vbo, GLuint index_vbo, float *modelMat,int ntri);
void init_collide(struct X3D_PolyRep pr);
#endif


#endif /* __FREEWRL_COLLISION_H__ */
