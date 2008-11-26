/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

#ifndef __SENSINTERPS_H__
#define __SENSINTERPS_H__


#include "headers.h"

#include <math.h>

#ifdef AQUA
#include <gl.h>
#include <glu.h>
#include <glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#include "LinearAlgebra.h"
#include "quaternion.h"
#include "sounds.h"


#define ASLEN 500


double return_Duration(int indx);

void
do_active_inactive(int *act,
				   double *inittime,
				   double *startt,
				   double *stopt,
				   int loop,
				   double myDuration,
				   double speed);

int
find_key(int kin, float frac, float *keys);

void
do_OintScalar(void *node);

void
do_OintCoord(void *node);

void do_OintCoord2D(void *node);
void do_OintPos2D(void *node);
void do_PositionInterpolator(void *node);
void do_ColorInterpolator(void *node);
void do_GeoPositionInterpolator(void *node);

void
do_Oint4(void *node);

void do_CollisionTick(void *ptr);
void do_AudioTick(void *ptr);
void do_TimeSensorTick(void *ptr);
void do_ProximitySensorTick(void *ptr);
void do_GeoProximitySensorTick(void *ptr);
void do_MovieTextureTick(void *ptr);
void do_VisibilitySensorTick(void *ptr);

void do_Anchor( void *ptr, int typ, int but1, int over);
void do_TouchSensor( void *ptr, int typ, int but1, int over);
void do_GeoTouchSensor(void *ptr, int typ, int but1, int over);
void do_PlaneSensor(void *ptr, int typ, int but1, int over);
void do_CylinderSensor(void *ptr, int typ, int but1, int over);
void do_SphereSensor(void *ptr, int typ, int but1, int over);


#endif /* __SENSINTERPS_H__ */
