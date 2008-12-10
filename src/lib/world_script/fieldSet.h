/*
=INSERT_TEMPLATE_HERE=

$Id: fieldSet.h,v 1.1 2008/12/10 14:31:53 couannette Exp $

???

*/

#ifndef __FREEX3D_JS_FIELD_SET_H__
#define __FREEX3D_JS_FIELD_SET_H__


#define ROUTED_FIELD_EVENT_OUT 0
#define ROUTED_FIELD_EVENT_IN  1

uintptr_t Multi_Struct_memptr (int type, void *memptr);
void setField_fromJavascript (struct X3D_Node *node, char *field, char *value);
unsigned int setField_FromEAI (char *ptr);
void setField_javascriptEventOut(struct X3D_Node *tn,unsigned int tptr,  int fieldType, unsigned len, int extraData, uintptr_t cx);
char *findFIELDNAMESfromNodeOffset(struct X3D_Node *node, int offset);
int findFieldInARR(const char* field, const char** arr, size_t cnt);

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

int findRoutedFieldInARR (struct X3D_Node * node, const char *field, int fromTo, const char** arr, size_t cnt, BOOL user);

#define DEF_FINDROUTEDFIELDFP(arr) int findRoutedFieldIn##arr(struct X3D_Node* node, const char* field, int fromTo)

DEF_FINDROUTEDFIELDFP(FIELDNAMES);
DEF_FINDROUTEDFIELDFP(EXPOSED_FIELD);
DEF_FINDROUTEDFIELDFP(EVENT_IN);
DEF_FINDROUTEDFIELDFP(EVENT_OUT);


#endif /* __FREEX3D_JS_FIELD_SET_H__ */
