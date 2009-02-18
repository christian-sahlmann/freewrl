/*
=INSERT_TEMPLATE_HERE=

$Id: fieldGet.h,v 1.3 2009/02/18 13:37:50 istakenv Exp $

Javascript C language binding.

*/

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
