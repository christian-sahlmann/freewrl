/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Geospatial.h,v 1.10 2010/02/17 18:03:06 crc_canada Exp $

Proximity sensor macro.

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



#ifndef __FREEWRL_SCENEGRAPH_GEOSPATIAL_H__
#define __FREEWRL_SCENEGRAPH_GEOSPATIAL_H__


/* ProximitySensor and GeoProximitySensor are same "code" at this stage of the game */
#define PROXIMITYSENSOR(type,center,initializer1,initializer2) \
void proximity_##type (struct X3D_##type *node) { \
	/* Viewer pos = t_r2 */ \
	double cx,cy,cz; \
	double len; \
	struct point_XYZ dr1r2; \
	struct point_XYZ dr2r3; \
	struct point_XYZ nor1,nor2; \
	struct point_XYZ ins; \
	static const struct point_XYZ yvec = {0,0.05,0}; \
	static const struct point_XYZ zvec = {0,0,-0.05}; \
	static const struct point_XYZ zpvec = {0,0,0.05}; \
	static const struct point_XYZ orig = {0,0,0}; \
	struct point_XYZ t_zvec, t_yvec, t_orig; \
	GLDOUBLE modelMatrix[16]; \
	GLDOUBLE projMatrix[16]; \
 \
	if(!((node->enabled))) return; \
	initializer1 \
	initializer2 \
 \
	/* printf (" vp %d geom %d light %d sens %d blend %d prox %d col %d\n",*/ \
	/* render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);*/ \
 \
	/* transforms viewers coordinate space into sensors coordinate space. \
	 * this gives the orientation of the viewer relative to the sensor. \
	 */ \
	fw_glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix); \
	fw_glGetDoublev(GL_PROJECTION_MATRIX, projMatrix); \
	FW_GLU_UNPROJECT(orig.x,orig.y,orig.z,modelMatrix,projMatrix,viewport, \
		&t_orig.x,&t_orig.y,&t_orig.z); \
	FW_GLU_UNPROJECT(zvec.x,zvec.y,zvec.z,modelMatrix,projMatrix,viewport, \
		&t_zvec.x,&t_zvec.y,&t_zvec.z); \
	FW_GLU_UNPROJECT(yvec.x,yvec.y,yvec.z,modelMatrix,projMatrix,viewport, \
		&t_yvec.x,&t_yvec.y,&t_yvec.z); \
 \
 \
	/*printf ("\n"); \
	printf ("unprojected, t_orig (0,0,0) %lf %lf %lf\n",t_orig.x, t_orig.y, t_orig.z); \
	printf ("unprojected, t_yvec (0,0.05,0) %lf %lf %lf\n",t_yvec.x, t_yvec.y, t_yvec.z); \
	printf ("unprojected, t_zvec (0,0,-0.05) %lf %lf %lf\n",t_zvec.x, t_zvec.y, t_zvec.z); \
	*/ \
	cx = t_orig.x - ((node->center ).c[0]); \
	cy = t_orig.y - ((node->center ).c[1]); \
	cz = t_orig.z - ((node->center ).c[2]); \
 \
	if(((node->size).c[0]) == 0 || ((node->size).c[1]) == 0 || ((node->size).c[2]) == 0) return; \
 \
	if(fabs(cx) > ((node->size).c[0])/2 || \
	   fabs(cy) > ((node->size).c[1])/2 || \
	   fabs(cz) > ((node->size).c[2])/2) return; \
	/* printf ("within (Geo)ProximitySensor\n"); */ \
 \
	/* Ok, we now have to compute... */ \
	(node->__hit) /*cget*/ = 1; \
 \
	/* Position */ \
	((node->__t1).c[0]) = (float)t_orig.x; \
	((node->__t1).c[1]) = (float)t_orig.y; \
	((node->__t1).c[2]) = (float)t_orig.z; \
 \
	VECDIFF(t_zvec,t_orig,dr1r2);  /* Z axis */ \
	VECDIFF(t_yvec,t_orig,dr2r3);  /* Y axis */ \
 \
	/* printf ("      dr1r2 %lf %lf %lf\n",dr1r2.x, dr1r2.y, dr1r2.z); \
	printf ("      dr2r3 %lf %lf %lf\n",dr2r3.x, dr2r3.y, dr2r3.z); \
	*/ \
 \
	len = sqrt(VECSQ(dr1r2)); VECSCALE(dr1r2,1/len); \
	len = sqrt(VECSQ(dr2r3)); VECSCALE(dr2r3,1/len); \
 \
	/* printf ("scaled dr1r2 %lf %lf %lf\n",dr1r2.x, dr1r2.y, dr1r2.z); \
	printf ("scaled dr2r3 %lf %lf %lf\n",dr2r3.x, dr2r3.y, dr2r3.z); \
	*/ \
 \
	/* \
	printf("PROX_INT: (%f %f %f) (%f %f %f) (%f %f %f)\n (%f %f %f) (%f %f %f)\n", \
		t_orig.x, t_orig.y, t_orig.z, \
		t_zvec.x, t_zvec.y, t_zvec.z, \
		t_yvec.x, t_yvec.y, t_yvec.z, \
		dr1r2.x, dr1r2.y, dr1r2.z, \
		dr2r3.x, dr2r3.y, dr2r3.z \
		); \
	*/ \
 \
	if(fabs(VECPT(dr1r2, dr2r3)) > 0.001) { \
		printf ("Sorry, can't handle unevenly scaled ProximitySensors yet :(" \
		  "dp: %f v: (%f %f %f) (%f %f %f)\n", VECPT(dr1r2, dr2r3), \
		  	dr1r2.x,dr1r2.y,dr1r2.z, \
		  	dr2r3.x,dr2r3.y,dr2r3.z \
			); \
		return; \
	} \
 \
 \
	if(APPROX(dr1r2.z,1.0)) { \
		/* rotation */ \
		((node->__t2).c[0]) = (float) 0; \
		((node->__t2).c[1]) = (float) 0; \
		((node->__t2).c[2]) = (float) 1; \
		((node->__t2).c[3]) = (float) atan2(-dr2r3.x,dr2r3.y); \
	} else if(APPROX(dr2r3.y,1.0)) { \
		/* rotation */ \
		((node->__t2).c[0]) = (float) 0; \
		((node->__t2).c[1]) = (float) 1; \
		((node->__t2).c[2]) = (float) 0; \
		((node->__t2).c[3]) = (float) atan2(dr1r2.x,dr1r2.z); \
	} else { \
		/* Get the normal vectors of the possible rotation planes */ \
		nor1 = dr1r2; \
		nor1.z -= 1.0; \
		nor2 = dr2r3; \
		nor2.y -= 1.0; \
 \
		/* Now, the intersection of the planes, obviously cp */ \
		VECCP(nor1,nor2,ins); \
 \
		len = sqrt(VECSQ(ins)); VECSCALE(ins,1/len); \
 \
		/* the angle */ \
		VECCP(dr1r2,ins, nor1); \
		VECCP(zpvec, ins, nor2); \
		len = sqrt(VECSQ(nor1)); VECSCALE(nor1,1/len); \
		len = sqrt(VECSQ(nor2)); VECSCALE(nor2,1/len); \
		VECCP(nor1,nor2,ins); \
 \
		((node->__t2).c[3]) = (float) -atan2(sqrt(VECSQ(ins)), VECPT(nor1,nor2)); \
 \
		/* rotation  - should normalize sometime... */ \
		((node->__t2).c[0]) = (float) ins.x; \
		((node->__t2).c[1]) = (float) ins.y; \
		((node->__t2).c[2]) = (float) ins.z; \
	} \
	/* \
	printf("NORS: (%f %f %f) (%f %f %f) (%f %f %f)\n", \
		nor1.x, nor1.y, nor1.z, \
		nor2.x, nor2.y, nor2.z, \
		ins.x, ins.y, ins.z \
	); \
	*/ \
} 

int checkX3DGeoElevationGridFields (struct X3D_GeoElevationGrid *node, float **points, int *npoints);

#endif /* __FREEWRL_SCENEGRAPH_GEOSPATIAL_H__ */
