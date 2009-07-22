/*
=INSERT_TEMPLATE_HERE=

$Id: Frustum.h,v 1.1 2009/07/22 19:30:03 crc_canada Exp $

Global includes.

*/

#ifndef __FREEWRL_FRUSTUM_H__
#define __FREEWRL_FRUSTUM_H__

/* for Extents and BoundingBoxen */
#define EXTENT_MAX_X _extent[0]
#define EXTENT_MIN_X _extent[1]
#define EXTENT_MAX_Y _extent[2]
#define EXTENT_MIN_Y _extent[3]
#define EXTENT_MAX_Z _extent[4]
#define EXTENT_MIN_Z _extent[5]

#define RECORD_DISTANCE if (render_geom) {record_ZBufferDistance (X3D_NODE(node)); }



void moveAndRotateThisPoint(struct point_XYZ *mypt, double x, double y, double z, double *MM);
void setExtent(float maxx, float minx, float maxy, float miny, float maxz, float minz, struct X3D_Node *me);
void printmatrix(GLdouble* mat);
void propagateExtent(struct X3D_Node *me);
void record_ZBufferDistance(struct X3D_Node *node);
void OcclusionStartofEventLoop(void);
void OcclusionCulling (void);
void zeroOcclusion(void);



#endif /* __FREEWRL_FRUSTUM_H__ */
