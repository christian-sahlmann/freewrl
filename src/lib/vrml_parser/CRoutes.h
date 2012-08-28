/*
=INSERT_TEMPLATE_HERE=

$Id: CRoutes.h,v 1.13 2012/08/28 15:33:52 crc_canada Exp $

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



#define REINITIALIZE_SORTED_NODES_FIELD(aaa,bbb) \
       /* has this changed size? */ \
        if (aaa.n != bbb.n) { \
                FREE_IF_NZ(bbb.p); \
                bbb.p = MALLOC (void *, sizeof (struct X3DNode *) * aaa.n); \
        } \
 \
        /* copy the nodes over; we will sort the sorted list */ \
        memcpy (bbb.p,aaa.p,sizeof (struct X3DNode *) * aaa.n); \
        bbb.n = aaa.n; 


struct CRStruct *getCRoutes();

/* function protos */
int get_valueChanged_flag (int, int);
void getSpecificRoute (int routeNo, struct X3D_Node **fromNode, int *fromOffset, struct X3D_Node **toNode, int *toOffset);

void mark_event (struct X3D_Node *from, int fromoffset);
void mark_event_check (struct X3D_Node *from, int fromoffset,char *fn, int line);
void resetScriptTouchedFlag(int actualscript, int fptr);
void Multimemcpy (struct X3D_Node *toNode, struct X3D_Node *fromNode, void *tn, void *fn, size_t multitype);

void CRoutes_js_new (int num,int scriptType);
//extern int max_script_found;
//extern int max_script_found_and_initialized;
//extern int *scr_act;

#endif /* __FREEWRL_CROUTES_H__ */
