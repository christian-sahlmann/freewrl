/*
=INSERT_TEMPLATE_HERE=

$Id: world_script.h,v 1.5 2009/04/03 18:21:58 crc_canada Exp $

Local include for world_script directory.

*/

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
