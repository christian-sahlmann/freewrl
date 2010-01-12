/*
=INSERT_TEMPLATE_HERE=

$Id: Tess.h,v 1.3 2010/01/12 20:04:47 sdumoulin Exp $

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


#ifndef __FREEWRL_TESS_H__
#define __FREEWRL_TESS_H__


/* Triangulator extern defs - look in CFuncs/Tess.c */
extern struct X3D_PolyRep *global_tess_polyrep;
#ifndef IPHONE
extern GLUtriangulatorObj *global_tessobj;
#endif
extern int global_IFS_Coords[];
extern int global_IFS_Coord_count;


#endif /* __FREEWRL_TESS_H__ */
