/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Core.h,v 1.4 2009/10/05 15:07:23 crc_canada Exp $

X3D Core Component

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


#ifndef __FREEWRL_SCENEGRAPH_CORE_H__
#define __FREEWRL_SCENEGRAPH_CORE_H__


void compile_MetadataSFBool (struct X3D_MetadataSFBool *node);
void compile_MetadataSFFloat (struct X3D_MetadataSFFloat *node);
void compile_MetadataMFFloat (struct X3D_MetadataMFFloat *node);
void compile_MetadataSFRotation (struct X3D_MetadataSFRotation *node);
void compile_MetadataMFRotation (struct X3D_MetadataMFRotation *node);
void compile_MetadataSFVec3f (struct X3D_MetadataSFVec3f *node);
void compile_MetadataMFVec3f (struct X3D_MetadataMFVec3f *node);
void compile_MetadataMFBool (struct X3D_MetadataMFBool *node);
void compile_MetadataSFInt32 (struct X3D_MetadataSFInt32 *node);
void compile_MetadataMFInt32 (struct X3D_MetadataMFInt32 *node);
void compile_MetadataSFNode (struct X3D_MetadataSFNode *node);
void compile_MetadataMFNode (struct X3D_MetadataMFNode *node);
void compile_MetadataSFColor (struct X3D_MetadataSFColor *node);
void compile_MetadataMFColor (struct X3D_MetadataMFColor *node);
void compile_MetadataSFColorRGBA (struct X3D_MetadataSFColorRGBA *node);
void compile_MetadataMFColorRGBA (struct X3D_MetadataMFColorRGBA *node);
void compile_MetadataSFTime (struct X3D_MetadataSFTime *node);
void compile_MetadataMFTime (struct X3D_MetadataMFTime *node);
void compile_MetadataSFString (struct X3D_MetadataSFString *node);
void compile_MetadataMFString (struct X3D_MetadataMFString *node);
void compile_MetadataSFVec2f (struct X3D_MetadataSFVec2f *node);
void compile_MetadataMFVec2f (struct X3D_MetadataMFVec2f *node);
void compile_MetadataSFImage (struct X3D_MetadataSFImage *node);
void compile_MetadataSFVec3d (struct X3D_MetadataSFVec3d *node);
void compile_MetadataMFVec3d (struct X3D_MetadataMFVec3d *node);
void compile_MetadataSFDouble (struct X3D_MetadataSFDouble *node);
void compile_MetadataMFDouble (struct X3D_MetadataMFDouble *node);
void compile_MetadataSFMatrix3f (struct X3D_MetadataSFMatrix3f *node);
void compile_MetadataMFMatrix3f (struct X3D_MetadataMFMatrix3f *node);
void compile_MetadataSFMatrix3d (struct X3D_MetadataSFMatrix3d *node);
void compile_MetadataMFMatrix3d (struct X3D_MetadataMFMatrix3d *node);
void compile_MetadataSFMatrix4f (struct X3D_MetadataSFMatrix4f *node);
void compile_MetadataMFMatrix4f (struct X3D_MetadataMFMatrix4f *node);
void compile_MetadataSFMatrix4d (struct X3D_MetadataSFMatrix4d *node);
void compile_MetadataMFMatrix4d (struct X3D_MetadataMFMatrix4d *node);
void compile_MetadataSFVec2d (struct X3D_MetadataSFVec2d *node);
void compile_MetadataMFVec2d (struct X3D_MetadataMFVec2d *node);
void compile_MetadataSFVec4f (struct X3D_MetadataSFVec4f *node);
void compile_MetadataMFVec4f (struct X3D_MetadataMFVec4f *node);
void compile_MetadataSFVec4d (struct X3D_MetadataSFVec4d *node);
void compile_MetadataMFVec4d (struct X3D_MetadataMFVec4d *node);

#endif  /* __FREEWRL_SCENEGRAPH_CORE_H__ */
