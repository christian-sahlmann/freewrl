/*
=INSERT_TEMPLATE_HERE=

$Id: fieldSet.h,v 1.16 2012/04/26 16:36:24 crc_canada Exp $

???

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



#ifndef __FREEWRL_JS_FIELD_SET_H__
#define __FREEWRL_JS_FIELD_SET_H__

#include <system_js.h>

#define ROUTED_FIELD_EVENT_OUT 0
#define ROUTED_FIELD_EVENT_IN  1

#define DEF_FINDFIELDFP(arr) int findFieldIn##arr(const char* field)
DEF_FINDFIELDFP(FIELDNAMES);
DEF_FINDFIELDFP(FIELD);
DEF_FINDFIELDFP(EXPOSED_FIELD);
DEF_FINDFIELDFP(EVENT_IN);
DEF_FINDFIELDFP(EVENT_OUT);
DEF_FINDFIELDFP(KEYWORDS);
DEF_FINDFIELDFP(PROTOKEYWORDS);
DEF_FINDFIELDFP(NODES);
DEF_FINDFIELDFP(PROFILES);
DEF_FINDFIELDFP(COMPONENTS);
DEF_FINDFIELDFP(FIELDTYPES);
DEF_FINDFIELDFP(X3DSPECIAL);
DEF_FINDFIELDFP(GEOSPATIAL);
DEF_FINDFIELDFP(MULTITEXTUREMODE);
DEF_FINDFIELDFP(MULTITEXTURESOURCE);
DEF_FINDFIELDFP(MULTITEXTUREFUNCTION);

int findRoutedFieldInARR (struct X3D_Node *, const char *, int, const char**, size_t, BOOL);
int findFieldInARR(const char*, const char**, size_t);
void findFieldInOFFSETS(int, const int, int *, int *, int *);
void getJSMultiNumType(JSContext *, struct Multi_Vec3f *, int);
void getMFStringtype(JSContext *, jsval *, struct Multi_String *);
int findIndexInFIELDNAMES(int, const char**, size_t);
char *findFIELDNAMESfromNodeOffset(struct X3D_Node *node, int offset);
void getMFNodetype (struct X3D_Node *strp, struct Multi_Node *ch, struct X3D_Node *par, int ar);
void setField_javascriptEventOut(struct X3D_Node *tn,unsigned int tptr,  int fieldType, unsigned len, int extraData, JSContext *scriptContext);


#define DEF_FINDROUTEDFIELDFP(arr) int findRoutedFieldIn##arr(struct X3D_Node* node, const char* field, int fromTo)
DEF_FINDROUTEDFIELDFP(FIELDNAMES);
DEF_FINDROUTEDFIELDFP(EXPOSED_FIELD);
DEF_FINDROUTEDFIELDFP(EVENT_IN);
DEF_FINDROUTEDFIELDFP(EVENT_OUT);


#endif /* __FREEWRL_JS_FIELD_SET_H__ */
