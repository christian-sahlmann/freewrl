/*
=INSERT_TEMPLATE_HERE=

$Id: EAIHelpers.h,v 1.13 2010/02/26 16:18:08 sdumoulin Exp $

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

struct Uni_String *newASCIIString(char *str);
void verify_Uni_String(struct  Uni_String *unis, char *str);
struct X3D_Node *getEAINodeFromTable(int_t index, int_t field);
int returnElementRowSize (int type);					/* from EAI_C_CommonFunctions.c */
size_t returnElementLength(int type);					/* from EAI_C_CommonFunctions.c */
int_t getEAIActualOffset(int_t node, int_t field);
char *getEAIMemoryPointer (int_t node, int_t field);
int registerEAINodeForAccess(struct X3D_Node* myn);
void handleEAIGetValue (char command, char *bufptr, char *buf, int repno);
int EAI_GetRootNode(void);

void EAI_GetType(uintptr_t cNode, char *ctmp, char *dtmp,
		 int_t *cNodePtr, int_t *fieldOffset,
		 uintptr_t *dataLen, int_t *typeString, unsigned int *scripttype, int *accessType);

int mapToKEYWORDindex(indexT pkwIndex);

#endif /* __FREEWRL_EAI_HELPERS_H__ */

