/*
=INSERT_TEMPLATE_HERE=

$Id: fieldGet.h,v 1.5 2009/10/05 15:07:24 crc_canada Exp $

Javascript C language binding.

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



#ifndef __FREEWRL_FIELDGET_H__
#define __FREEWRL_FIELDGET_H__ 1

void getField_ToJavascript (int num, int fromoffset);
void set_one_ECMAtype (uintptr_t tonode, int toname, int dataType, void *Data, unsigned datalen);
void setScriptECMAtype (uintptr_t num);
int setMFElementtype (uintptr_t num);
int set_one_MFElementType(uintptr_t tonode, int toname, int dataType, void *Data, unsigned datalen);
void set_EAI_MFElementtype (int num, int offset, unsigned char *pptr, int len);
void set_one_MultiElementType (uintptr_t tonode, uintptr_t tnfield, void *Data, unsigned dataLen);
void setScriptMultiElementtype (uintptr_t num);
void EAI_Convert_mem_to_ASCII (int id, char *reptype, int type, char *memptr, char *buf);

#endif /* __FREEWRL_FIELDGET_H__ */
