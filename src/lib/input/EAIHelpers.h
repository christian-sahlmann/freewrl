/*
=INSERT_TEMPLATE_HERE=

$Id: EAIHelpers.h,v 1.17 2010/09/04 12:19:15 crc_canada Exp $

EAI Helpers functions

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


#ifndef __FREEWRL_EAI_HELPERS_H__
#define __FREEWRL_EAI_HELPERS_H__

#include "../vrml_parser/Structs.h"

extern char *outBuffer;
extern int outBufferLen;

struct Uni_String *newASCIIString(char *str);
void verify_Uni_String(struct  Uni_String *unis, char *str);
struct X3D_Node *getEAINodeFromTable(int index, int field);
int getEAINodeTypeFromTable(int node) ;
int returnElementRowSize (int type);					/* from EAI_C_CommonFunctions.c */
int returnElementLength(int type);					/* from EAI_C_CommonFunctions.c */
int getEAIActualOffset(int node, int field);
char *getEAIMemoryPointer (int node, int field);
int registerEAINodeForAccess(struct X3D_Node* myn);
void handleEAIGetValue (char command, char *bufptr, int repno);
int EAI_GetRootNode(void);

void EAI_GetType(int cNode, char *ctmp, char *dtmp,
		 int *cNodePtr, int *fieldOffset,
		 uintptr_t *dataLen, int *typeString, unsigned int *scripttype, int *accessType);

int mapToKEYWORDindex(indexT pkwIndex);
void outBufferCat (char *str);

#endif /* __FREEWRL_EAI_HELPERS_H__ */

