/*
=INSERT_TEMPLATE_HERE=

$Id: EAIHelpers.h,v 1.7 2009/06/01 19:37:43 istakenv Exp $

EAI Helpers functions

*/

#ifndef __FREEWRL_EAI_HELPERS_H__
#define __FREEWRL_EAI_HELPERS_H__

#include "../vrml_parser/Structs.h"

struct X3D_Node *getEAINodeFromTable(int index, int field);
int returnElementRowSize (int type);					/* from EAI_C_CommonFunctions.c */
int returnElementLength(int type);					/* from EAI_C_CommonFunctions.c */
int getEAIActualOffset(int node, int field);
char *getEAIMemoryPointer (int node, int field);
int registerEAINodeForAccess(struct X3D_Node* myn);
void handleEAIGetValue (char command, char *bufptr, char *buf, int repno);
int EAI_GetRootNode(void);

void EAI_GetType(int cNode, char *ctmp, char *dtmp,
		 uintptr_t *cNodePtr, uintptr_t *fieldOffset,
		 uintptr_t *dataLen, uintptr_t *typeString, unsigned int *scripttype, int *accessType);

int mapToKEYWORDindex(indexT pkwIndex);

#endif /* __FREEWRL_EAI_HELPERS_H__ */

