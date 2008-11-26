/* $Id: Collision.h,v 1.1 2008/11/26 11:24:13 couannette Exp $
 *
 * Copyright (C) 2002 Nicolas Coderre CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License (file COPYING in the distribution)
 * for conditions of use and redistribution.
 */
#ifndef COLLISIONH
#define COLLISIONH


#include <stdio.h>
#include <math.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#if defined(__APPLE__)
#include <sys/malloc.h>
#endif

#include "headers.h"
#include "LinearAlgebra.h"
#include "Structs.h"

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
struct point_XYZ get_poly_normal_disp(double y1, double y2, double r, struct point_XYZ* p, int num, struct point_XYZ n);

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
struct point_XYZ get_poly_min_disp_with_sphere(double r, struct point_XYZ* p, int num, struct point_XYZ n);

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
struct point_XYZ polyrep_disp(double y1, double y2, double ydisp, double r, struct X3D_PolyRep pr, GLdouble* mat, prflags flags);

/*displacement when the polyrep structure is all in the same plane
  if normal is zero, it will be calculated form the first triangle*/
struct point_XYZ planar_polyrep_disp(double y1, double y2, double ydisp, double r, struct X3D_PolyRep pr, GLdouble* mat, prflags flags, struct point_XYZ n);

struct point_XYZ elevationgrid_disp( double y1, double y2, double ydisp, double r, struct X3D_PolyRep pr,
			      int xdim, int zdim, double xs, double zs, GLdouble* mat, prflags flags);

/* functions VERY usefull for debugging purposes
   Use these inside FreeWRL to export a scene to
   the debugging programs. */
#ifdef DEBUG_SCENE_EXPORT
void printpolyrep(struct X3D_PolyRep pr, int npoints);

void printmatrix(GLdouble* mat);
#endif

#endif




