/*
=INSERT_TEMPLATE_HERE=

$Id: fieldSet.h,v 1.8 2009/04/03 18:21:58 crc_canada Exp $

???

*/

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

int findRoutedFieldInARR (struct X3D_Node *, const char *, int, const char**, size_t, BOOL);
int findFieldInARR(const char*, const char**, size_t);
void findFieldInOFFSETS(int, const int, int *, int *, int *);
void getJSMultiNumType(JSContext *, struct Multi_Vec3f *, int);
void getMFStringtype(JSContext *, jsval *, struct Multi_String *);
int findIndexInFIELDNAMES(int, const char**, size_t);
char *findFIELDNAMESfromNodeOffset(struct X3D_Node *node, int offset);

#define DEF_FINDROUTEDFIELDFP(arr) int findRoutedFieldIn##arr(struct X3D_Node* node, const char* field, int fromTo)
DEF_FINDROUTEDFIELDFP(FIELDNAMES);
DEF_FINDROUTEDFIELDFP(EXPOSED_FIELD);
DEF_FINDROUTEDFIELDFP(EVENT_IN);
DEF_FINDROUTEDFIELDFP(EVENT_OUT);


#endif /* __FREEWRL_JS_FIELD_SET_H__ */
