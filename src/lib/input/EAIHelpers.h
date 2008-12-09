/*
=INSERT_TEMPLATE_HERE=

$Id: EAIHelpers.h,v 1.1 2008/12/09 12:12:41 couannette Exp $

EAI Helpers functions

*/

#ifndef __FREEX3D_EAI_HELPERS_H__
#define __FREEX3D_EAI_HELPERS_H__


struct X3D_Node *getEAINodeFromTable(int index);
/* 
old:

void EAI_GetType (uintptr_t cNode,  char *ctmp, char *dtmp, uintptr_t *cNodePtr, uintptr_t *fieldOffset,
                        uintptr_t *dataLen, uintptr_t *typeString,  unsigned int *scripttype, int *accessType);

new:
*/
void EAI_GetType(int cNode, char *ctmp, char *dtmp,
		 uintptr_t *cNodePtr, uintptr_t *fieldOffset,
		 uintptr_t *dataLen, uintptr_t *typeString, unsigned int *scripttype, int *accessType);


#endif /* __FREEX3D_EAI_HELPERS_H__ */

