/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Core.c,v 1.10 2011/06/02 19:50:43 dug9 Exp $

X3D Core Component

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



/*******************************************************************

	X3D Core Component

*********************************************************************/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CRoutes.h"


/************************************************************************************************/
/*												*/
/*	Metadata standard X3D nodes								*/
/*												*/
/************************************************************************************************/

void compile_MetadataInteger (struct X3D_MetadataInteger *node) {
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataInteger, metadata));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataInteger, name));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataInteger, reference));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataInteger, value));
	MARK_NODE_COMPILED;
}
void compile_MetadataDouble (struct X3D_MetadataDouble *node) {
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataDouble, metadata));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataDouble, name));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataDouble, reference));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataDouble, value));
	MARK_NODE_COMPILED;
}
void compile_MetadataFloat (struct X3D_MetadataFloat *node) {
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataFloat, metadata));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataFloat, name));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataFloat, reference));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataFloat, value));
	MARK_NODE_COMPILED;
}
void compile_MetadataSet (struct X3D_MetadataSet *node) {
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataSet, metadata));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataSet, name));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataSet, reference));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataSet, value));
	MARK_NODE_COMPILED;
}
void compile_MetadataString (struct X3D_MetadataString *node) {
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataString, metadata));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataString, name));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataString, reference));
	MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_MetadataString, value));
	MARK_NODE_COMPILED;
}


/************************************************************************************************/
/*												*/
/*	MetadataMF and MetadataSF nodes								*/
/*												*/
/************************************************************************************************/

#define META_IS_INITIALIZED (node->_ichange != 0)

/* anything changed for this PROTO interface datatype? */
#define CMD_I32(type) void compile_MetadataSF##type (struct X3D_MetadataSF##type *node) { \
	if META_IS_INITIALIZED { \
		if (node->tickTime != TickTime()) { \
		node->value = node->setValue; \
		node->valueChanged = node->setValue; \
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataSF##type, valueChanged)); \
		} \
	} else { \
		/* initialize fields */ \
		node->valueChanged = node->value; node->setValue = node->value; \
	} \
	node->tickTime = TickTime();  \
	MARK_NODE_COMPILED \
}

#define CMD_FL(type) void compile_MetadataSF##type (struct X3D_MetadataSF##type *node) { \
	if META_IS_INITIALIZED { \
		if (node->tickTime != TickTime()) { \
		node->value = node->setValue; \
		node->valueChanged = node->setValue; \
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataSF##type, valueChanged)); \
	} \
	} else { \
		/* initialize fields */ \
		node->valueChanged = node->value; node->setValue = node->value; \
	} \
	node->tickTime = TickTime();  \
	MARK_NODE_COMPILED \
}

#define CMD_MFL(type,elelength) void compile_MetadataSF##type (struct X3D_MetadataSF##type *node) { \
	int count; \
	if META_IS_INITIALIZED { \
	for (count=0; count < elelength; count++) { \
		if (!APPROX(node->value.c[count],node->setValue.c[count])) { \
			memcpy (&node->value, &node->setValue, sizeof node->value.c[0]* elelength); \
			memcpy (&node->valueChanged, &node->setValue, sizeof node->value.c[0] * elelength); \
			MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataSF##type, valueChanged)); \
			return; \
		} \
	} \
	} else { \
		/* initialize fields */ \
		memcpy (&node->setValue, &node->value, sizeof node->value.c[0]* elelength); \
		memcpy (&node->valueChanged, &node->value, sizeof node->value.c[0] * elelength); \
	} \
	MARK_NODE_COMPILED \
}



/* compare element counts, and pointer values */
/* NOTE - VALUES CAN NOT BE DESTROYED BY THE KILL PROCESSES, AS THESE ARE JUST COPIES OF POINTERS */
#define CMD_MULTI(mtype, type,elelength,dataSize) void compile_MetadataMF##type (struct X3D_MetadataMF##type *node) { \
	int count; int changed = FALSE; \
	if META_IS_INITIALIZED { \
	if (node->value.n != node->setValue.n) changed = TRUE; else { \
		/* yes, these two array must have the same index counts... */ \
		for (count=0; count<node->setValue.n; count++) { \
			int count2; for (count2=0; count2<elelength; count2++) { if (!APPROX(node->value.p[count].c[count2], node->setValue.p[count].c[count2])) changed = TRUE; break; }\
		if (changed) break; } \
	} \
	\
	if (changed) { \
                        /* printf ("MSFL, change hit, freeing pointers %x and %x\n", node->value.p, node->valueChanged.p); */ \
			FREE_IF_NZ (node->value.p); \
			FREE_IF_NZ(node->valueChanged.p); \
			node->value.p = MALLOC(mtype, dataSize * node->setValue.n * elelength); \
			node->valueChanged.p = MALLOC(mtype, dataSize * node->setValue.n * elelength); \
			memcpy(node->value.p, node->setValue.p, dataSize * node->setValue.n * elelength); \
			memcpy(node->valueChanged.p, node->setValue.p, dataSize * node->setValue.n * elelength); \
                        node->value.n = node->setValue.n; \
                        node->valueChanged.n = node->setValue.n; \
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataMF##type, valueChanged)); \
	} \
	} else { \
		/* the "value" will hold everything we need */ \
		/* initialize it, but do not bother doing any routing on it */ \
		if ((node->setValue.n != 0) || (node->setValue.p != NULL) || (node->valueChanged.n != 0) || (node->valueChanged.p != NULL)) { printf ("PROTO header - initialization set and changed, but not zero??\n");  \
                node->setValue.n = 0; FREE_IF_NZ(node->setValue.p);  \
                node->valueChanged.n = 0; FREE_IF_NZ(node->valueChanged.p); } \
		FREE_IF_NZ (node->setValue.p); \
		FREE_IF_NZ(node->valueChanged.p); \
		node->setValue.p = MALLOC(mtype, dataSize * node->value.n * elelength); \
		node->valueChanged.p = MALLOC(mtype, dataSize * node->value.n * elelength); \
		memcpy(node->setValue.p, node->value.p, dataSize * node->value.n * elelength); \
		memcpy(node->valueChanged.p, node->value.p, dataSize * node->value.n * elelength); \
                node->setValue.n = node->value.n; \
                node->valueChanged.n = node->value.n; \
                MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataMF##type, valueChanged)); \
	} \
	MARK_NODE_COMPILED \
}

#define CMD_MSFI32(mtype, type,dataSize) void compile_MetadataMF##type (struct X3D_MetadataMF##type *node) { \
        /* printf ("\nMSFI32:, node %x\n",node); \
        printf ("MSFI32:, nt %s change %d ichange %d\n",stringNodeType(node->_nodeType),node->_change, node->_ichange); */ \
        if META_IS_INITIALIZED { \
                int count; int changed = FALSE; \
                /* printf ("MSFI32:, so this is initialized; value %d setValue count%d\n",node->value.n,node->setValue.n); */ \
/* { int count; char *cptr = (char *)&(node->setValue); for (count = 0; count < 8; count ++) { printf ("%u: %x ",count, *cptr); cptr ++; } \
 printf ("\n"); \
cptr = (char *)&(node->value); for (count = 0; count < 8; count ++) { printf ("%u: %x ",count, *cptr); cptr ++; } \
 printf ("\n"); \
} */\
                if (node->value.n != node->setValue.n) changed = TRUE; \
                else { \
		    /* same count, but something caused this to be called; go through each element */ \
                    for (count=0; count<node->setValue.n; count++) { \
                          /* printf ("MSFI32, comparing ele %d %x %x\n",count, node->value.p[count], node->setValue.p[count]); */ \
                          if (node->value.p[count] != node->setValue.p[count]) {changed = TRUE; break; } \
                    } \
                } \
 \
                if (changed) { \
                        /* printf ("MSFI32, change hit, freeing pointers %x and %x\n", node->value.p, node->valueChanged.p); */ \
			FREE_IF_NZ (node->value.p); \
			FREE_IF_NZ(node->valueChanged.p); \
			node->value.p = MALLOC(mtype, dataSize * node->setValue.n); \
			node->valueChanged.p = MALLOC(mtype, dataSize * node->setValue.n); \
			memcpy(node->value.p, node->setValue.p, dataSize * node->setValue.n); \
			memcpy(node->valueChanged.p, node->setValue.p, dataSize * node->setValue.n); \
                        node->value.n = node->setValue.n; \
                        node->valueChanged.n = node->setValue.n; \
                        MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataMF##type, valueChanged)); \
                } \
        } else { \
                /* the "value" will hold everything we need */ \
                /* initialize it, but do not bother doing any routing on it */ \
		/* printf ("MSFI32: initializing... %d %p %d %p\n", node->setValue.n, node->setValue.p, node->valueChanged.n, node->valueChanged.p); */ \
		if ((node->setValue.n != 0) || (node->setValue.p != NULL) || (node->valueChanged.n != 0) || (node->valueChanged.p != NULL)) { printf ("PROTO header - initialization set and changed, but not zero??\n");  \
                node->setValue.n = 0; FREE_IF_NZ(node->setValue.p);  \
                node->valueChanged.n = 0; FREE_IF_NZ(node->valueChanged.p); } \
		/* printf ("MSFI32 - leaving the setValue and ValueChanged pointers to %x %x\n",node->setValue.p, node->valueChanged.p);*/ \
		/* printf ("MFSI32, making setValue and valueChanged equal to the value on initialization\n"); */ \
		FREE_IF_NZ (node->setValue.p); \
		FREE_IF_NZ(node->valueChanged.p); \
		node->setValue.p = MALLOC(mtype, dataSize * node->value.n); \
		node->valueChanged.p = MALLOC(mtype, dataSize * node->value.n); \
		memcpy(node->setValue.p, node->value.p, dataSize * node->value.n); \
		memcpy(node->valueChanged.p, node->value.p, dataSize * node->value.n); \
                node->setValue.n = node->value.n; \
                node->valueChanged.n = node->value.n; \
                MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataMF##type, valueChanged)); \
        } \
        MARK_NODE_COMPILED \
        /* printf ("MSFI32: DONE; value %d, value_changed.n %d\n", node->value.n,node->valueChanged.n);  */ \
} 







/* compare element counts, then individual elements, if the counts are the same */
/* NOTE - VALUES CAN NOT BE DESTROYED BY THE KILL PROCESSES, AS THESE ARE JUST COPIES OF POINTERS */
#define CMD_MSFL(mtype, type,dataSize) void compile_MetadataMF##type (struct X3D_MetadataMF##type *node) { \
	int count; int changed = FALSE; \
	if META_IS_INITIALIZED { \
	if (node->value.n != node->setValue.n) changed = TRUE; else { \
		/* yes, these two array must have the same index counts... */ \
		for (count=0; count<node->setValue.n; count++) if (!APPROX(node->value.p[count], node->setValue.p[count])) { changed = TRUE; break; }}\
	\
	if (changed) { \
                        /* printf ("MSFL, change hit, freeing pointers %x and %x\n", node->value.p, node->valueChanged.p); */ \
			FREE_IF_NZ (node->value.p); \
			FREE_IF_NZ(node->valueChanged.p); \
			node->value.p = MALLOC(mtype, dataSize * node->setValue.n); \
			node->valueChanged.p = MALLOC(mtype, dataSize * node->setValue.n); \
			memcpy(node->value.p, node->setValue.p, dataSize * node->setValue.n); \
			memcpy(node->valueChanged.p, node->setValue.p, dataSize * node->setValue.n); \
                        node->value.n = node->setValue.n; \
                        node->valueChanged.n = node->setValue.n; \
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataMF##type, valueChanged)); \
	} \
	} else { \
		/* the "value" will hold everything we need */ \
		/* initialize it, but do not bother doing any routing on it */ \
		if ((node->setValue.n != 0) || (node->setValue.p != NULL) || (node->valueChanged.n != 0) || (node->valueChanged.p != NULL)) { printf ("PROTO header - initialization set and changed, but not zero??\n");  \
                node->setValue.n = 0; FREE_IF_NZ(node->setValue.p);  \
                node->valueChanged.n = 0; FREE_IF_NZ(node->valueChanged.p); } \
	} \
	MARK_NODE_COMPILED \
}


CMD_FL(Float)
CMD_FL(Time)
CMD_FL(Double)
CMD_I32(Bool)
CMD_I32(Int32)
CMD_I32(Node)

CMD_MFL(Vec2f,2)
CMD_MFL(Vec3f,3)
CMD_MFL(Vec4f,4)
CMD_MFL(Vec2d,2)
CMD_MFL(Vec3d,3)
CMD_MFL(Vec4d,4)
CMD_MFL(Rotation,4)
CMD_MFL(Color,3)
CMD_MFL(ColorRGBA,4)
CMD_MFL(Matrix3f,9)
CMD_MFL(Matrix3d,9)
CMD_MFL(Matrix4f,16)
CMD_MFL(Matrix4d,16)

CMD_MULTI(struct SFRotation *, Rotation,4,sizeof (float))
CMD_MULTI(struct SFVec2f *, Vec2f,2,sizeof (float))
CMD_MULTI(struct SFVec3f *, Vec3f,3,sizeof (float))
CMD_MULTI(struct SFVec4f *, Vec4f,4,sizeof (float))
CMD_MULTI(struct SFVec2d *, Vec2d,2,sizeof (double))
CMD_MULTI(struct SFVec3d *,Vec3d,3,sizeof (double))
CMD_MULTI(struct SFVec4d *,Vec4d,4,sizeof (double))
CMD_MULTI(struct SFColor *, Color,3,sizeof (float))
CMD_MULTI(struct SFColorRGBA *, ColorRGBA,4,sizeof (float))
CMD_MULTI(struct SFMatrix3f *, Matrix3f,9,sizeof (float))
CMD_MULTI(struct SFMatrix4f *, Matrix4f,16,sizeof (float))
CMD_MULTI(struct SFMatrix3d *, Matrix3d,9,sizeof (double))
CMD_MULTI(struct SFMatrix4d *, Matrix4d,16,sizeof (double))

CMD_MSFI32(int *, Bool, sizeof(int))
CMD_MSFI32(int *, Int32,sizeof (int))
CMD_MSFI32(struct X3D_Node **, Node,sizeof (void *))
CMD_MSFL(double *, Time,sizeof (double))
CMD_MSFL(float *, Float,sizeof (float))
CMD_MSFL(double *, Double,sizeof (double))
CMD_MSFI32(struct Uni_String **, String,sizeof (void *))


void compile_MetadataSFImage (struct X3D_MetadataSFImage *node){ printf ("make compile_Metadata %s\n",stringNodeType(node->_nodeType));}
/*
struct Uni_String {
        int len;
        char * strptr;
        int touched;
};
*/

void compile_MetadataSFString (struct X3D_MetadataSFString *node){ 
	int count; int changed = FALSE; 

	if META_IS_INITIALIZED { 
	if (node->value->len != node->setValue->len) changed = TRUE; else { 
		for (count=0; count<node->setValue->len; count++) 
			if (node->value->strptr[count] != node->setValue->strptr[count]) changed = TRUE; }
	
	if (changed) { 
		node->value->len = node->setValue->len; node->value->strptr = node->setValue->strptr; 
		node->valueChanged->len = node->setValue->len; node->valueChanged->strptr = node->setValue->strptr; 
		node->value->touched = TRUE; node->valueChanged->touched = TRUE;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataSFString, valueChanged)); 
	} 
	} else {
		/* initialize this one */
		node->valueChanged->len = node->value->len;
		node->valueChanged->touched = node->value->touched;
		node->valueChanged->strptr = node->value->strptr;
		node->setValue->len = node->value->len;
		node->setValue->touched = node->value->touched;
		node->setValue->strptr = node->value->strptr;
	}
	MARK_NODE_COMPILED
}

