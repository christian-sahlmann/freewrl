/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Picking.h,v 1.2 2010/10/12 20:06:58 istakenv Exp $

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



#ifndef __FREEWRL_SCENEGRAPH_PICKING_H__
#define __FREEWRL_SCENEGRAPH_PICKING_H__

/* see specifications section 38. Picking Sensor Component */

/* in the meantime, provide function protos for what's in Component_Picking.c */
void other_PointPickSensor (struct X3D_PointPickSensor *node);
void other_PickableGroup (struct X3D_Group *node);
void other_Sphere (struct X3D_Sphere *node);
void child_PickableGroup (struct X3D_Group *node);
void prep_PickableGroup (struct X3D_Group *node);
void add_picksensor(struct X3D_Node * node);

#endif /* __FREEWRL_SCENEGRAPH_PICKING_H__ */
