/*
=INSERT_TEMPLATE_HERE=

$Id: CRoutes.h,v 1.10 2010/03/01 22:39:49 crc_canada Exp $

VRML-parsing routines in C.

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


#ifndef __FREEWRL_CROUTES_H__
#define __FREEWRL_CROUTES_H__

/* C routes */
typedef struct _CRnodeStruct {
        struct X3D_Node *routeToNode;
        int foffset;
} CRnodeStruct;


struct CRStruct {
        struct X3D_Node*  routeFromNode;
        int fnptr;
        int tonode_count;
        CRnodeStruct *tonodes;
        int     isActive;
        int     len;
        void    (*interpptr)(void *); /* pointer to an interpolator to run */
        int     direction_flag; /* if non-zero indicates script in/out,
                                                   proto in/out */
        int     extra;          /* used to pass a parameter (eg, 1 = addChildren..) */
	int 	intTimeStamp;	/* used for ROUTE loop breaking */
};
extern struct CRStruct *CRoutes;

/* function protos */
int get_valueChanged_flag (int, int);
void getSpecificRoute (int routeNo, struct X3D_Node **fromNode, int *fromOffset, struct X3D_Node **toNode, int *toOffset);

void mark_event (struct X3D_Node *from, int fromoffset);
void mark_event_check (struct X3D_Node *from, int fromoffset,char *fn, int line);
void resetScriptTouchedFlag(int actualscript, int fptr);

void CRoutes_js_new (int num,int scriptType);
extern int max_script_found;
extern int max_script_found_and_initialized;
extern int *scr_act;

#endif /* __FREEWRL_CROUTES_H__ */
