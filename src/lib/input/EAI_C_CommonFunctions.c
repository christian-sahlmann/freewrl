/*
=INSERT_TEMPLATE_HERE=

$Id: EAI_C_CommonFunctions.c,v 1.33 2010/02/26 21:47:57 crc_canada Exp $

???

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



#ifndef REWIRE
#include <config.h>
#include <system.h>
#include <libFreeWRL.h>
#endif
#include <display.h>
#include <internal.h>


#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "EAIHeaders.h"
#include "EAIHelpers.h"

/* TODO: clean-up Rewire */
#ifdef REWIRE
# include "../../libeai/EAI_C.h"
# define ADD_PARENT(a,b)
#endif

/* assume eaiverbose is false, unless told otherwise */
int eaiverbose = FALSE;

#define PST_MF_STRUCT_ELEMENT(type1,type2) \
	case FIELDTYPE_MF##type1: { \
		struct Multi_##type1 *myv; \
		myv = (struct Multi_##type1 *) nst; \
		/* printf ("old val p= %u, n = %d\n",myv->p, myv->n); */\
		myv->p = myVal.mf##type2.p; \
		myv->n = myVal.mf##type2.n; \
		/* printf ("PST_MF_STRUCT_ELEMENT, now, element count %d\n",myv->n); */ \
		break; }


#define PST_SF_SIMPLE_ELEMENT(type1,type2,size3) \
	case FIELDTYPE_SF##type1: { \
		memcpy(nst, &myVal.sf##type2, size3); \
		break; }


/* create a structure to hold a string; it has a length, and a string pointer */
struct Uni_String *newASCIIString(char *str) {
	struct Uni_String *retval;
	int len;

	if (eaiverbose) {
	printf ("newASCIIString for :%s:\n",str);
	}

	/* the returning Uni_String is here. Make blank struct */
	retval = MALLOC (sizeof (struct Uni_String));
	len = (int) strlen(str);

	retval->strptr  = MALLOC (sizeof(char) * len+1);
	strncpy(retval->strptr,str,len+1);
	retval->len = len+1;
	retval->touched = 1; /* make it 1, to signal that this is a NEW string. */

	/* printf ("newASCIIString, returning UniString %x, strptr %u for string :%s:\n",retval, retval->strptr,str); */

	return retval;
}

/* do these strings differ?? If so, copy the new string over the old, and 
touch the touched flag */
void verify_Uni_String(struct  Uni_String *unis, char *str) {
	char *ns;
	char *os;
	size_t len;

	/* bounds checking */
	if (unis == NULL) {
		printf ("Warning, verify_Uni_String, comparing to NULL Uni_String, %s\n",str);
		return;
	}

	/* are they different? */
	if (strcmp(str,unis->strptr)!= 0) {
		os = unis->strptr;
		len = strlen(str);
		ns = MALLOC (len+1);
		strncpy(ns,str,len+1);
		unis->strptr = ns;
		FREE_IF_NZ (os);
		unis->touched++;
	}
}
		



/* get how many bytes in the type */
int  returnElementLength(int type) {
	  switch (type) {
		case FIELDTYPE_SFVec2d:
		case FIELDTYPE_MFVec2d:
		case FIELDTYPE_MFVec3d:
		case FIELDTYPE_SFVec3d:
		case FIELDTYPE_MFDouble:
		case FIELDTYPE_SFDouble:
		case FIELDTYPE_SFVec4d:
		case FIELDTYPE_MFVec4d:
		case FIELDTYPE_SFMatrix3d:
		case FIELDTYPE_MFMatrix3d:
		case FIELDTYPE_SFMatrix4d:
		case FIELDTYPE_MFMatrix4d:
		case FIELDTYPE_SFTime :
    		case FIELDTYPE_MFTime : return (int) sizeof(double); break;
    		case FIELDTYPE_MFInt32: return (int) sizeof(int)   ; break;
		case FIELDTYPE_FreeWRLPTR:
    		case FIELDTYPE_SFNode :
    		case FIELDTYPE_MFNode : return (int) sizeof(void *); break;
	  	default     : {}
	}
	return (int) sizeof(float) ; /* turn into byte count */
}

/* for passing into CRoutes/CRoutes_Register */
/* lengths are either positive numbers, or, if there is a complex type, a negative number. If positive,
   in routing a memcpy is performed; if negative, then some inquiry is required to get correct length
   of both source/dest during routing. */

size_t returnRoutingElementLength(int type) {
	  switch (type) {
		case FIELDTYPE_SFTime:	return sizeof(double); break;
		case FIELDTYPE_SFBool:
		case FIELDTYPE_SFInt32:	return sizeof(int); break;
		case FIELDTYPE_SFFloat:	return sizeof (float); break;
		case FIELDTYPE_SFVec2f:	return sizeof (struct SFVec2f); break;
		case FIELDTYPE_SFVec3f:
		case FIELDTYPE_SFColor: 	return sizeof (struct SFColor); break;
		case FIELDTYPE_SFVec3d: return sizeof (struct SFVec3d); break;
		case FIELDTYPE_SFColorRGBA:
		case FIELDTYPE_SFRotation:return sizeof (struct SFRotation); break;
		case FIELDTYPE_SFNode:	return ROUTING_SFNODE; break;
		case FIELDTYPE_SFMatrix3f: return sizeof (struct SFMatrix3f); break;
		case FIELDTYPE_SFMatrix3d: return sizeof (struct SFMatrix3d); break;
/* FIXME FIND DEF FOR SFVEC4F */
#ifndef REWIRE
		case FIELDTYPE_SFVec4f: return sizeof (struct SFVec4f) ; break;
#endif
		case FIELDTYPE_SFMatrix4f: return sizeof (struct SFMatrix4f); break;
		case FIELDTYPE_SFVec2d: return sizeof (struct SFVec2d); break;
		case FIELDTYPE_SFDouble: return sizeof (double); break;
		case FIELDTYPE_SFVec4d: return sizeof (struct SFVec4d); break;

		case FIELDTYPE_SFString: return ROUTING_SFSTRING; break;
		case FIELDTYPE_SFImage:	return ROUTING_SFIMAGE; break;

		case FIELDTYPE_MFNode:	return ROUTING_MFNODE; break;
		case FIELDTYPE_MFString: 	return ROUTING_MFSTRING; break;
		case FIELDTYPE_MFFloat:	return ROUTING_MFFLOAT; break;
		case FIELDTYPE_MFColorRGBA:
		case FIELDTYPE_MFRotation: return ROUTING_MFROTATION; break;
		case FIELDTYPE_MFBool:
		case FIELDTYPE_MFInt32:	return ROUTING_MFINT32; break;
		case FIELDTYPE_MFColor:	return ROUTING_MFCOLOR; break;
		case FIELDTYPE_MFVec2f:	return ROUTING_MFVEC2F; break;
		case FIELDTYPE_MFVec3f:	return ROUTING_MFVEC3F; break;
		case FIELDTYPE_MFVec3d: return ROUTING_MFVEC3D; break;
		case FIELDTYPE_MFDouble: return ROUTING_MFDOUBLE; break;
		case FIELDTYPE_MFTime: return ROUTING_MFDOUBLE; break;
		case FIELDTYPE_MFMatrix4f: return ROUTING_MFMATRIX4F; break;
		case FIELDTYPE_MFMatrix4d: return ROUTING_MFMATRIX4D; break;
		case FIELDTYPE_MFVec2d: return ROUTING_MFVEC2D; break;
		case FIELDTYPE_MFVec4f: return ROUTING_MFVEC4F; break;
		case FIELDTYPE_MFVec4d: return ROUTING_MFVEC4D; break;
		case FIELDTYPE_MFMatrix3f: return ROUTING_MFMATRIX3F; break;
		case FIELDTYPE_MFMatrix3d: return ROUTING_MFMATRIX3D; break;

                default:{
			printf ("warning - returnRoutingElementLength not a handled type, %d\n",type);
		}
	}
	return sizeof(int);
} 



/* how many numbers/etc in an array entry? eg, SFVec3f = 3 - 3 floats */
/*		"" ""			eg, MFVec3f = 3 - 3 floats, too! */
int returnElementRowSize (int type) {
	switch (type) {
		case FIELDTYPE_SFVec2f:
		case FIELDTYPE_MFVec2f:
		case FIELDTYPE_SFVec2d:
		case FIELDTYPE_MFVec2d:
			return 2;
		case FIELDTYPE_SFColor:
		case FIELDTYPE_MFColor:
		case FIELDTYPE_SFVec3f:
		case FIELDTYPE_SFVec3d:
		case FIELDTYPE_MFVec3f:
		case FIELDTYPE_MFVec3d:
		case FIELDTYPE_SFImage: /* initialization - we can have a "0,0,0" for no texture */
			return 3;
		case FIELDTYPE_SFRotation:
		case FIELDTYPE_MFRotation:
		case FIELDTYPE_SFVec4d:
		case FIELDTYPE_SFVec4f:
		case FIELDTYPE_MFVec4d:
		case FIELDTYPE_MFVec4f:
		case FIELDTYPE_SFColorRGBA:
		case FIELDTYPE_MFColorRGBA:
			return 4;
		case FIELDTYPE_MFMatrix3f:
		case FIELDTYPE_SFMatrix3f:
		case FIELDTYPE_MFMatrix3d:
		case FIELDTYPE_SFMatrix3d:
			return 9;
		case FIELDTYPE_MFMatrix4f:
		case FIELDTYPE_SFMatrix4f:
		case FIELDTYPE_MFMatrix4d:
		case FIELDTYPE_SFMatrix4d:
			return 16;
	}
	return 1;

}

static struct VRMLParser *parser = NULL;

/* from the XML parser, for instance, we can call this on close to delete memory and memory tables */
void Parser_deleteParserForScanStringValueToMem(void) {
	if (parser != NULL) {
		lexer_destroyData(parser->lexer);
		deleteParser(parser);
		parser = NULL;
	}
}


void Parser_scanStringValueToMem(struct X3D_Node *node, size_t coffset, indexT ctype, char *value, int isXML) {
	void *nst;                      /* used for pointer maths */
	union anyVrml myVal;
	char *mfstringtmp = NULL;
	int oldXMLflag;
	struct X3D_Node *np;
	
	#ifdef SETFIELDVERBOSE
	printf ("\nPST, for %s we have %s strlen %lu\n",stringFieldtypeType(ctype), value, strlen(value));
	#endif

	/* if this is the first time through, create a new parser, and tell it:
	      - that we are using X3D formatted field strings, NOT "VRML" ones;
	      - that the destination node is not important (the NULL, offset 0) */

	if (parser == NULL) parser=newParser(NULL, 0, TRUE);
	lexer_forceStringCleanup(parser->lexer);

	/* October 20, 2009; XML parsing should not go through here; XML encoded X3D should not have a "value=" field, but
	   have the SFNode or MFNode as part of the syntax, eg <field ...> <Box/> </field> */

	if (isXML) {
		/* printf ("we have XML parsing for type %s, string :%s:\n",stringFieldtypeType(ctype),value); */
		if ((ctype==FIELDTYPE_SFNode) || (ctype==FIELDTYPE_MFNode)) {
			/* printf ("returning\n"); */
			lexer_forceStringCleanup(parser->lexer);
			return;
		}

	}

	/* there is a difference sometimes, in the XML format and VRML classic format. The XML
	   parser will use xml format, scripts and EAI will use the classic format */
	oldXMLflag = parser->parsingX3DfromXML;
	parser->parsingX3DfromXML = isXML;

	/* we NEED MFStrings to have quotes on; so if this is a MFString, ensure quotes are ok */
	if (ctype == FIELDTYPE_MFString) {
		#ifdef SETFIELDVERBOSE
		printf ("parsing type %s, string :%s:\n",stringFieldtypeType(ctype),value); 
		#endif

		/* go to the first non-space character, and see if this is required;
		   sometimes people will encode mfstrings as:
			url=' "images/earth.gif" "http://ww
		   note the space in the value */
		while ((*value == ' ') && (*value != '\0')) value ++;

		/* now, does the value string need quoting? */
		if ((*value != '"') && (*value != '\'') && (*value != '[')) {
			size_t len;
			 /* printf ("have to quote this string\n"); */
			len = strlen(value);
			mfstringtmp = MALLOC (sizeof (char *) * len + 10);
			memcpy (&mfstringtmp[1],value,len);
			mfstringtmp[0] = '"';
			mfstringtmp[len+1] = '"';
			mfstringtmp[len+2] = '\0';
			/* printf ("so, mfstring is :%s:\n",mfstringtmp); */ 
			
		} else {
			mfstringtmp = STRDUP(value);
		}
		parser_fromString(parser,mfstringtmp);
		/* FREE_IF_NZ(mfstringtmp); */
        } else if (ctype == FIELDTYPE_SFNode) {
                /* Need to change index to proper node ptr */
                np = getEAINodeFromTable(atoi(value), -1);
        } else {

		mfstringtmp = STRDUP(value);
		parser_fromString(parser,mfstringtmp);
		/* FREE_IF_NZ(mfstringtmp); */
	}

	ASSERT(parser->lexer);
	FREE_IF_NZ(parser->lexer->curID);

        if (ctype == FIELDTYPE_SFNode) {
                struct X3D_Node* oldvalue;
		nst = offsetPointer_deref(void *,node,coffset);
                memcpy (&oldvalue, nst, sizeof(struct X3D_Node*));
                if (oldvalue) {
                        remove_parent(oldvalue, node);
                }
                memcpy(nst, (void*)&np, sizeof(struct X3D_Node*));
                add_parent(np, node, "sarah's add", 0);

        } else if (parseType(parser, ctype, &myVal)) {

		/* printf ("parsed successfully\n");  */

		nst = offsetPointer_deref(void *,node,coffset);


/*
MF_TYPE(MFNode, mfnode, Node)
*/
		switch (ctype) {

			PST_MF_STRUCT_ELEMENT(Vec2f,vec2f)
			PST_MF_STRUCT_ELEMENT(Vec3f,vec3f)
			PST_MF_STRUCT_ELEMENT(Vec3d,vec3d)
			PST_MF_STRUCT_ELEMENT(Vec4d,vec4d)
			PST_MF_STRUCT_ELEMENT(Vec2d,vec2d)
			PST_MF_STRUCT_ELEMENT(Color,color)
			PST_MF_STRUCT_ELEMENT(ColorRGBA,colorrgba)
			PST_MF_STRUCT_ELEMENT(Int32,int32)
			PST_MF_STRUCT_ELEMENT(Float,float)
			PST_MF_STRUCT_ELEMENT(Double,double)
			PST_MF_STRUCT_ELEMENT(Bool,bool)
			PST_MF_STRUCT_ELEMENT(Time,time)
			PST_MF_STRUCT_ELEMENT(Rotation,rotation)
			PST_MF_STRUCT_ELEMENT(Matrix3f,matrix3f)
			PST_MF_STRUCT_ELEMENT(Matrix3d,matrix3d)
			PST_MF_STRUCT_ELEMENT(Matrix4f,matrix4f)
			PST_MF_STRUCT_ELEMENT(Matrix4d,matrix4d)
			PST_MF_STRUCT_ELEMENT(String,string)

			PST_SF_SIMPLE_ELEMENT(Float,float,sizeof(float))
			PST_SF_SIMPLE_ELEMENT(Time,time,sizeof(double))
			PST_SF_SIMPLE_ELEMENT(Double,double,sizeof(double))
			PST_SF_SIMPLE_ELEMENT(Int32,int32,sizeof(int))
			PST_SF_SIMPLE_ELEMENT(Bool,bool,sizeof(int))
			PST_SF_SIMPLE_ELEMENT(Node,node,sizeof(void *))
			PST_SF_SIMPLE_ELEMENT(Vec2f,vec2f,sizeof(struct SFVec2f))
			PST_SF_SIMPLE_ELEMENT(Vec2d,vec2d,sizeof(struct SFVec2d))
			PST_SF_SIMPLE_ELEMENT(Vec3f,vec3f,sizeof(struct SFColor))
			PST_SF_SIMPLE_ELEMENT(Vec3d,vec3d,sizeof(struct SFVec3d))
			PST_SF_SIMPLE_ELEMENT(Vec4d,vec4d,sizeof(struct SFVec4d))
			PST_SF_SIMPLE_ELEMENT(Rotation,rotation,sizeof(struct SFRotation))
			PST_SF_SIMPLE_ELEMENT(Color,color,sizeof(struct SFColor))
			PST_SF_SIMPLE_ELEMENT(ColorRGBA,colorrgba,sizeof(struct SFColorRGBA))
			PST_SF_SIMPLE_ELEMENT(Matrix3f,matrix3f,sizeof(struct SFMatrix3f))
			PST_SF_SIMPLE_ELEMENT(Matrix4f,matrix4f,sizeof(struct SFMatrix4f))
			PST_SF_SIMPLE_ELEMENT(Matrix3d,matrix3d,sizeof(struct SFMatrix3d))
			PST_SF_SIMPLE_ELEMENT(Matrix4d,matrix4d,sizeof(struct SFMatrix4d))
			PST_SF_SIMPLE_ELEMENT(Image,image,sizeof(struct Multi_Int32))

			case FIELDTYPE_SFString: {
					struct Uni_String *mptr;
					mptr = * (struct Uni_String **)nst;
					FREE_IF_NZ(mptr->strptr);
					mptr->strptr = myVal.sfstring->strptr;
					mptr->len = myVal.sfstring->len;
					mptr->touched = myVal.sfstring->touched;
				break; }

			default: {
				printf ("unhandled type, in EAIParse  %s\n",stringFieldtypeType(ctype));
				lexer_forceStringCleanup(parser->lexer);
				return;
			}
		}

	} else {
		if (strlen (value) > 50) {
			value[45] = '.';
			value[46] = '.';
			value[47] = '.';
			value[48] = '\0';
		}
		ConsoleMessage ("parser problem on parsing fieldType %s, string :%s:", stringFieldtypeType(ctype),value);
	}

	/* tell the parser that we have done with the input - it will FREE the data */
	lexer_forceStringCleanup(parser->lexer);

	/* and, reset the XML flag */
	parser->parsingX3DfromXML = oldXMLflag;
}
