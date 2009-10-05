/*
=INSERT_TEMPLATE_HERE=

$Id: SensInterps.h,v 1.6 2009/10/05 15:07:23 crc_canada Exp $

SensInterps ???

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


#ifndef __FREEWRL_SENS_INTERPS_H__
#define __FREEWRL_SENS_INTERPS_H__


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
void do_OintNormal(void *node);

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


#endif /* __FREEWRL_SENS_INTERPS_H__ */
