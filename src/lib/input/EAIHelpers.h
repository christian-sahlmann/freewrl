/*
=INSERT_TEMPLATE_HERE=

$Id: EAIHelpers.h,v 1.5 2009/02/18 13:37:50 istakenv Exp $

EAI Helpers functions

*/

#ifndef __FREEWRL_EAI_HELPERS_H__
#define __FREEWRL_EAI_HELPERS_H__


struct X3D_Node *getEAINodeFromTable(int index, int field);
int returnElementRowSize (int type);					/* from EAI_C_CommonFunctions.c */
int returnElementLength(int type);					/* from EAI_C_CommonFunctions.c */
int getEAIActualOffset(int node, int field);
uintptr_t *getEAIMemoryPointer (int node, int field);
int registerEAINodeForAccess(struct X3D_Node* myn);
void handleEAIGetValue (char command, char *bufptr, char *buf, int repno);
int EAI_GetRootNode(void);

void EAI_GetType(int cNode, char *ctmp, char *dtmp,
		 uintptr_t *cNodePtr, uintptr_t *fieldOffset,
		 uintptr_t *dataLen, uintptr_t *typeString, unsigned int *scripttype, int *accessType);

#endif /* __FREEWRL_EAI_HELPERS_H__ */

