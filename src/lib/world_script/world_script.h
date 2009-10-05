/*
=INSERT_TEMPLATE_HERE=

$Id: world_script.h,v 1.7 2009/10/05 15:07:24 crc_canada Exp $

Local include for world_script directory.

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


#ifndef __FREEWRL_WORLD_SCRIPT_LOCAL_H__
#define __FREEWRL_WORLD_SCRIPT_LOCAL_H__


void findFieldInOFFSETS(int nodeType , const int field, int *coffset, int *ctype, int *ckind);
void getJSMultiNumType (JSContext *cx, struct Multi_Vec3f *tn, int eletype);
void getMFStringtype (JSContext *cx, jsval *from, struct Multi_String *to);
void getMFNodetype (char *strp, struct Multi_Node *tn, struct X3D_Node *parent, int ar);
void SetMemory (int type, void *destptr, void *srcptr, int len);
void getEAI_ONE_MFStringtype (struct Multi_String *from, struct Multi_String *to, int len);
void getEAI_MFStringtype (struct Multi_String *from, struct Multi_String *to);
int ScanValtoBuffer(int *quant, int type, char *buf, void *memptr, int bufsz);
int findIndexInFIELDNAMES(int index, const char** arr, size_t arrCnt);


#endif /* __FREEWRL_WORLD_SCRIPT_LOCAL_H__ */
