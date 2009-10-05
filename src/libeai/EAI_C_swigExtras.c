
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


#include <config.h>
#include "EAI_C.h"
//#include "EAI_swigMe.h"
#include <stdio.h>
#define SWIG
#ifdef SWIG
int isMF(int nodetype)
{
	return (nodetype %2 == 1) && nodetype >= 0 && nodetype <= 41;
}
int isSF(int nodetype)
{
	return (nodetype % 2 == 0) && nodetype >= 0 && nodetype <= 41;
}
X3DNode *X3D_newSF(int nodetype)
{
	X3DNode* retval;
	if(isSF(nodetype))
	{
		/*delegate default construction to the appropriate type */
		switch(nodetype)
		{
		case FIELDTYPE_SFColor:
			retval = X3D_newSFColor(0.0f,0.0f,0.0f); break;
		case FIELDTYPE_SFFloat:
			retval = X3D_newSFFloat(0.0f); break;
		case FIELDTYPE_SFTime:
			retval = X3D_newSFTime(0.0); break;
		case FIELDTYPE_SFInt32:
			retval = X3D_newSFInt32(0); break;
		case FIELDTYPE_SFString:
			retval = X3D_newSFString(""); break;
		case FIELDTYPE_SFNode:
			retval = X3D_newSFNode(); break; 
		case FIELDTYPE_SFRotation:
			retval = X3D_newSFRotation(0.0f,1.0f,0.0f,0.0f); break;
		case FIELDTYPE_SFVec2f:
			retval = X3D_newSFVec2f(0.0f,0.0f);  break;
		case FIELDTYPE_SFVec3f:
			retval = X3D_newSFVec3f(0.0f,0.0f,0.0f); break;
		case FIELDTYPE_SFColorRGBA:
			retval = X3D_newSFColorRGBA(.5f,.5f,.5f,1.0f); break;
		case FIELDTYPE_SFBool:
			retval = X3D_newSFBool(0); break;
		case FIELDTYPE_SFVec3d:
			retval = X3D_newSFVec3d(0.0,0.0,0.0); break;
		case FIELDTYPE_SFVec2d:
			retval = X3D_newSFVec2d(0.0,0.0); break;
		}
		return retval;
	}
	else
		printf ("New node not implemented yet for this type\n");return NULL; 
	return NULL;
}
int X3D_sizeof(int fieldtype)
{
	/* how do you dynamic_cast<> in C? to keep the recent MF code both general and non-breaking we need the size by int type
	   if the code ever changes so the MF isn't malloced in a contiguous block ie **p instead of *p then we won't need this.
	*/
	int retval;
	switch(fieldtype)
	{
		case FIELDTYPE_SFColor:
			retval = sizeof(_intX3D_SFColor); break;
		case FIELDTYPE_SFFloat:
			retval = sizeof(_intX3D_SFFloat); break;
		case FIELDTYPE_SFTime:
			retval = sizeof(_intX3D_SFTime); break;
		case FIELDTYPE_SFInt32:
			retval = sizeof(_intX3D_SFInt32); break;
		case FIELDTYPE_SFString:
			retval = sizeof(_intX3D_SFString); break;
		case FIELDTYPE_SFNode:
			retval = sizeof(_intX3D_SFNode); break; 
		case FIELDTYPE_SFRotation:
			retval = sizeof(_intX3D_SFRotation); break;
		case FIELDTYPE_SFVec2f:
			retval = sizeof(_intX3D_SFVec2f);  break;
		case FIELDTYPE_SFVec3f:
			retval = sizeof(_intX3D_SFVec3f); break;
		case FIELDTYPE_SFColorRGBA:
			retval = sizeof(_intX3D_SFColorRGBA); break;
		case FIELDTYPE_SFBool:
			retval = sizeof(_intX3D_SFBool); break;
		case FIELDTYPE_SFVec3d:
			retval = sizeof(_intX3D_SFVec3d); break;
		case FIELDTYPE_SFVec2d:
			retval = sizeof(_intX3D_SFVec2d); break;

		case FIELDTYPE_MFColor:
			retval = sizeof(_intX3D_MFColor); break;
		case FIELDTYPE_MFFloat:
			retval = sizeof(_intX3D_MFFloat); break;
		case FIELDTYPE_MFTime:
			retval = sizeof(_intX3D_MFTime); break;
		case FIELDTYPE_MFInt32:
			retval = sizeof(_intX3D_MFInt32); break;
		case FIELDTYPE_MFString:
			retval = sizeof(_intX3D_MFString); break;
		case FIELDTYPE_MFNode:
			retval = sizeof(_intX3D_MFNode); break; 
		case FIELDTYPE_MFRotation:
			retval = sizeof(_intX3D_MFRotation); break;
		case FIELDTYPE_MFVec2f:
			retval = sizeof(_intX3D_MFVec2f);  break;
		case FIELDTYPE_MFVec3f:
			retval = sizeof(_intX3D_MFVec3f); break;
		case FIELDTYPE_MFColorRGBA:
			retval = sizeof(_intX3D_MFColorRGBA); break;
		case FIELDTYPE_MFBool:
			retval = sizeof(_intX3D_MFBool); break;
		case FIELDTYPE_MFVec3d:
			retval = sizeof(_intX3D_MFVec3d); break;
		case FIELDTYPE_MFVec2d:
			retval = sizeof(_intX3D_MFVec2d); break;
		default:
			retval = 0;
	}
	return retval;
}
X3DNode *_swigNewMF(int itype, int num )
{
	int i;
	char *p, *q;
	X3DNode* retval;
	retval = malloc (sizeof (X3DNode));
	retval->type = itype; 
	retval->X3D_MFNode.n = num;
	if(num > 0)
	{
		/* X3D_freeNode assumes the MF's SFs are in one block:	free(node->X3D_MFString.p); */
		p = malloc (sizeof (X3DNode) * num);
		retval->X3D_MFNode.p = (_intX3D_SFNode*)p;
		q = (char*)X3D_newSF(itype-1); /* initialized to defaults esp. strptr=NULL or whatever */
		for (i = 0; i < num; i++) {
			memcpy(p,q,X3D_sizeof(itype-1)); /*initialize the storage area to defaults*/
			p+= X3D_sizeof(itype-1);
		}
		X3D_freeNode((X3DNode*)q);
	}else
		retval->X3D_MFNode.p = NULL;
	return retval;

}
X3DNode *X3D_swigNewMF(char *fieldtype, int num )
{
	int itype, ftype;
	itype = -1;
	ftype = findFieldInFIELDTYPES(fieldtype);
	if( isMF(ftype) ) itype = ftype;
	if( isSF(ftype) ) itype = ftype+1; /*mf type is sf+1*/
	if(itype > -1) 
	{
		return _swigNewMF(itype,num);

	}else{
		printf ("New node not implemented yet for this type\n");return NULL; 
	}
}
void _X3D_setItemSF(X3DNode* node, int item, X3DNode* value)
{
	/* set a scalar value in the SF node array  .r or .c */

	int vtype, ntype;
	double v;
	vtype = value->type;
	ntype = node->type;
	if( !isSF(vtype) )return;
	if( item < 0 || item > 3) return;
	switch(vtype)
	{
	case FIELDTYPE_SFTime:
		v = value->X3D_SFTime.value; break;
	case FIELDTYPE_SFFloat:
		v = (double)value->X3D_SFFloat.value; break;
	case FIELDTYPE_SFInt32:
		v = (double)value->X3D_SFInt32.value; break;
	case FIELDTYPE_SFBool:
		v = (double)value->X3D_SFBool.value; break;
		/* should have SFString ? SFBool (int 1/0)? */
	default:
		return;
	}
	switch(ntype)
	{
		case FIELDTYPE_SFColor:
			if(item > 2) break;
			node->X3D_SFColor.c[item] = (float)v; break;
		case FIELDTYPE_SFFloat:
			if(item > 0) break;
			node->X3D_SFFloat.value = (float)v; break;
		case FIELDTYPE_SFTime:
			if(item > 0) break;
			node->X3D_SFTime.value = (float)v; break;
		case FIELDTYPE_SFInt32:
			if(item > 0) break;
			node->X3D_SFInt32.value = (int)v; break;
		case FIELDTYPE_SFString:
			if(item > strlen(node->X3D_SFString.strptr)) break; /* int32>>char function*/
			node->X3D_SFString.strptr[item] = (char)((int)v); break;
		case FIELDTYPE_SFNode:
			if(item > 0) break;
			node->X3D_SFNode.adr = (uintptr_t*)(int)v; break; 
		case FIELDTYPE_SFRotation:
			if(item > 3) break;
			node->X3D_SFRotation.r[item] = (float)v; break;
		case FIELDTYPE_SFVec2f:
			if(item > 1) break;
			node->X3D_SFVec2f.c[item] = (float)v;  break;
		case FIELDTYPE_SFVec3f:
			if(item > 2) break;
			node->X3D_SFVec3f.c[item] = (float)v; break;
		case FIELDTYPE_SFColorRGBA:
			if(item > 3) break;
			node->X3D_SFColorRGBA.r[item] = (float)v; break;
		case FIELDTYPE_SFBool:
			if(item > 0) break;
			node->X3D_SFBool.value = ((int)v)?1:0; break;
		case FIELDTYPE_SFVec3d:
			if(item > 2) break;
			node->X3D_SFVec3d.c[item] = (double)v; break;
		case FIELDTYPE_SFVec2d:
			if(item > 1) break;
			node->X3D_SFVec2d.c[item] = (double)v; break;
		default:
			break;
	}
}
X3DNode* _X3D_getItemSF(X3DNode* node, int item)
{
	/* get a scalar value in the SF node array  .r or .c */

	int vtype,ntype;
	union{
	double d;
	float f;
	int i;
	} v;
	vtype = -1;

	ntype = node->type;
	if( item < 0 || item > 3) return NULL;
	switch(ntype)
	{
		case FIELDTYPE_SFColor:
			if(item > 2) break;
			v.f = node->X3D_SFColor.c[item]; vtype =FIELDTYPE_SFFloat; break;
		case FIELDTYPE_SFFloat:
			if(item > 0) break;
			v.f = node->X3D_SFFloat.value; vtype =FIELDTYPE_SFFloat; break;
		case FIELDTYPE_SFTime:
			if(item > 0) break;
			v.d = node->X3D_SFTime.value; vtype = FIELDTYPE_SFTime; break;
		case FIELDTYPE_SFInt32:
			if(item > 0) break;
			v.i = node->X3D_SFInt32.value; vtype = FIELDTYPE_SFInt32; break;
		case FIELDTYPE_SFString:
			if(item > strlen(node->X3D_SFString.strptr)) break; /* int32>>char function*/
			v.i = (int)(char)node->X3D_SFString.strptr[item]; vtype = FIELDTYPE_SFInt32; break;
		case FIELDTYPE_SFNode:
			if(item > 0) break;
			v.i = (int)(uintptr_t*)node->X3D_SFNode.adr; vtype = FIELDTYPE_SFInt32; break; 
		case FIELDTYPE_SFRotation:
			if(item > 3) break;
			v.f = node->X3D_SFRotation.r[item]; vtype =FIELDTYPE_SFFloat; break;
		case FIELDTYPE_SFVec2f:
			if(item > 1) break;
			v.f = node->X3D_SFVec2f.c[item]; vtype =FIELDTYPE_SFFloat; break;
		case FIELDTYPE_SFVec3f:
			if(item > 2) break;
			v.f = node->X3D_SFVec3f.c[item];vtype =FIELDTYPE_SFFloat; break;
		case FIELDTYPE_SFColorRGBA:
			if(item > 3) break;
			v.f = node->X3D_SFColorRGBA.r[item]; vtype =FIELDTYPE_SFFloat; break;
		case FIELDTYPE_SFBool:
			if(item > 0) break;
			v.i = node->X3D_SFBool.value; vtype = FIELDTYPE_SFInt32; break;
		case FIELDTYPE_SFVec3d:
			if(item > 2) break;
			v.d = node->X3D_SFVec3d.c[item]; vtype =FIELDTYPE_SFTime; break;
		case FIELDTYPE_SFVec2d:
			if(item > 1) break;
			v.d = node->X3D_SFVec2d.c[item]; vtype =FIELDTYPE_SFTime; break;
		default:
			break;
	}
	switch(vtype)
	{
	case FIELDTYPE_SFTime:
		return X3D_newSFTime(v.d); break;
	case FIELDTYPE_SFFloat:
		return X3D_newSFFloat(v.f); break;
	case FIELDTYPE_SFInt32:
		return X3D_newSFInt32(v.i); break;
	case FIELDTYPE_SFBool:
		return X3D_newSFInt32(v.i); break;
	default:
		return NULL;
	}
	return NULL;
}
X3DNode* X3D_deepcopySF(X3DNode* node)
{
	X3DNode* retval;
	/*returns deep copy of field*/
	float* f;
	double* d;
	char* s;
	int n;
	int ntype = node->type;
	switch(ntype)
	{
		case FIELDTYPE_SFColor:
			f = node->X3D_SFColor.c;
			retval = X3D_newSFColor(f[0],f[1],f[2]); break;
		case FIELDTYPE_SFFloat:
			retval = X3D_newSFFloat(node->X3D_SFFloat.value); break;
		case FIELDTYPE_SFTime:
			retval = X3D_newSFTime(node->X3D_SFTime.value); break;
		case FIELDTYPE_SFInt32:
			retval = X3D_newSFInt32(node->X3D_SFInt32.value); break;
		case FIELDTYPE_SFString:
			retval = X3D_newSFString(node->X3D_SFString.strptr); 
			s = node->X3D_SFString.strptr;
			n = min(strlen(s)+1,STRLEN);
			retval->X3D_SFString.strptr = malloc(n);
			strncpy(retval->X3D_SFString.strptr,s,n);
			retval->X3D_SFString.len = strlen(s);
			break;
		case FIELDTYPE_SFNode:
			retval = X3D_newSFNode(); break; 
		case FIELDTYPE_SFRotation:
			f = node->X3D_SFRotation.r;
			retval = X3D_newSFRotation(f[0],f[1],f[2],f[3]); break;
		case FIELDTYPE_SFVec2f:
			f = node->X3D_SFVec2f.c;
			retval = X3D_newSFVec2f(f[0],f[1]);  break;
		case FIELDTYPE_SFVec3f:
			f = node->X3D_SFVec3f.c;
			retval = X3D_newSFVec3f(f[0],f[1],f[2]);  break;
		case FIELDTYPE_SFColorRGBA:
			f = node->X3D_SFColorRGBA.r;
			retval = X3D_newSFColorRGBA(f[0],f[1],f[2],f[3]); break;
		case FIELDTYPE_SFBool:
			retval = X3D_newSFBool(node->X3D_SFBool.value); break;
		case FIELDTYPE_SFVec3d:
			retval = NULL;
			break;
		case FIELDTYPE_SFVec2d:
			d = node->X3D_SFVec2d.c;
			retval = X3D_newSFVec2d(d[0],d[1]);  break;
		default:
			break;
	}
	return retval;
}
void _X3D_setItemMF(X3DNode* node, int item, X3DNode* value)
{
	unsigned long sz;
	char * target;
	char * newstr;
	if( !isSF(value->type) )return; /* we're supposed to be setting an SF into an MF */
	if( node->type != value->type+1 )return; /* must be same type ie mfstring and sfstring, or mfvec2d and sfvec2d */
	if( item < 0 || item >= node->X3D_MFNode.n )return; /*space must have been allocated already, with swigNewMF(,#) or  _grow(,,#) */
	if( value->type == FIELDTYPE_SFString)
	{
		int len = min(value->X3D_SFString.len +1,STRLEN);
		/*free(node->X3D_MFString.p[item].strptr); /* better be reasonably initialized ie to NULL in newSF */
		FREE_IF_NZ(node->X3D_MFString.p[item].strptr);
		newstr = malloc(len*sizeof(char)); /**STRLEN);*/
		strncpy(newstr,value->X3D_SFString.strptr,len); /*policy - a string is owned by only one node (no ref counting). so deep copy so SF and MF both own their own*/
	}
	sz = X3D_sizeof(value->type);
	target = ((char *)(node->X3D_MFNode.p)) + sz*item;
	memcpy(target,value,sz);
	if( value->type == FIELDTYPE_SFString)
	{
		node->X3D_MFString.p[item].strptr = newstr;
		node->X3D_MFString.p[item].len = strnlen(newstr,STRLEN);
	}
}
X3DNode* _X3D_getItemMF(X3DNode* node, int item)
{
	X3DNode* retval;
	if( item < 0 || item >= node->X3D_MFNode.n )return NULL;
	/* MSVC increments a pointer by the size of the data structure being pointed to
		the way we do the p[] it is by specific type
	*/
	retval = X3D_deepcopySF((X3DNode*)(((char *)(node->X3D_MFNode.p)) + item*X3D_sizeof(node->X3D_MFNode.p[0].type)));
	return retval;

	/* if noncontiguous this would probably work
	X3D_freeNode(node->X3D_MFNode.p[item]);
	node->X3D_MFNode.p[item] = X3D_deepcopySF(value); 
	*/
	retval = X3D_newSF(node->type -1);
	if( retval->type == FIELDTYPE_SFString) free(retval->X3D_SFString.strptr); /*should be '\0' */
	memcpy(retval,&(node->X3D_MFNode.p[item]),X3D_sizeof(retval->type)); /*len should get copied here, strptr too though*/
	if( retval->type == FIELDTYPE_SFString) 
	{
		int len = min(node->X3D_SFString.len+1,STRLEN);
		retval->X3D_SFString.strptr = malloc(len);
		strncpy(retval->X3D_SFString.strptr,node->X3D_SFString.strptr,len);
	}

	return retval;
}

void X3D_swigSetItem(X3DNode* node, int item, X3DNode* value)
{
	if(isMF(node->type)) _X3D_setItemMF(node,item,value);
	else if( isSF(value->type) )_X3D_setItemSF(node,item,value);
}
X3DNode* X3D_swigGetItem(X3DNode* node, int item)
{
	if(isMF(node->type)) return _X3D_getItemMF(node,item);
	else if( isSF(node->type) ) return _X3D_getItemSF(node,item);
	return NULL;
}
void _grow(X3DNode *node, int num, int more)
{
	_intX3D_SFNode *tmp,*p;
	tmp = node->X3D_MFNode.p;
	p = malloc(sizeof (X3DNode) * (num+more));
	bzero(p,sizeof (X3DNode) * (num+more));
	if(num > 0)
	{
		memcpy(p,tmp,sizeof(X3DNode)*num);
		free(tmp);
	}
	node->X3D_MFNode.p = p;
	node->X3D_MFNode.n = num + more;
}
void X3D_swigAppendToMF(X3DNode* node, X3DNode* value)
{
	int num, item;
	if( node != NULL && value != NULL )
	{
		if(isMF(node->type) && isSF(value->type))
		{
			if(node->type == value->type+1)
			{
				num = node->X3D_MFNode.n;
				_grow(node,num,1);
				item = node->X3D_MFNode.n -1; 
				_X3D_setItemMF(node,item,value);
			}
		}
	}
}

int getnumtokens(char* str,char *delim)
{
	int n;
	char *tokens;
	n = 0;
	tokens = strtok(str,delim);
	while(tokens != NULL)
	{
		n++;
		tokens = strtok(NULL,delim);
	}
	return n;
}
char * _x3ditoa(int ival, char* bigbuf)
{
	char *retbuf;
	int len;
	sprintf(bigbuf,"%d ",ival); /*note blank separator */
	len = strlen(bigbuf);
	retbuf = (char*)MALLOC(len+1);
	strcpy(retbuf,bigbuf);
	return retbuf;
}
char * _x3dftoa(float fval, char* bigbuf)
{
	char *retbuf;
	int len;
	sprintf(bigbuf,"%f ",fval); /*note blank separator */
	len = strlen(bigbuf);
	retbuf = (char*)MALLOC(len+1);
	strcpy(retbuf,bigbuf);
	return retbuf;
}
char * _x3ddtoa(double dval, char* bigbuf)
{
	char *retbuf;
	int len;
	sprintf(bigbuf,"%f ",dval); /*note blank separator */
	len = strlen(bigbuf);
	retbuf = (char*)MALLOC(len+1);
	strcpy(retbuf,bigbuf);
	return retbuf;
}
char * _x3datoa(char *aval, char* bigbuf)
{
	/* hello -> "hello", strlen(aval)==0? -> "" */
	char *retbuf;
	int len;
	len = strlen(aval);
	retbuf = (char*)MALLOC(len+4);
	sprintf(retbuf,"\"%s\" ",aval);
	retbuf[len+3] = '\0';
	return retbuf;
}
char * X3D_swigStringFromField(X3DNode* field)
{
	/* 
	  issue: swig treats C arrays as opaque pointers - not very helpful. How to get a specific SF from an MF field?
	  but- swig does pass a char * as scalar string properly
	  goal: convert an MF (or SF) field to a string for easy parsing from a swigged scripting language
	  "itype n sf1 sf2 sf3" 
	  sffloat "0 1 -123.456"  - the count will always be 1
	  mffloat "1 2 -123.456 555.444"
	  sfstring "24 \"OK folks let's rock it!\""
	  mfstring "25 2 \"You know...\" \"I'm not so sure about 'rocking' it.\"'
    */
	int type, ismf, count, i, size;
	char** tokens;
	char *string;
	char buf[500];
	type = field->type;
	if(type < 0 || type > 41 ) /*yikes - is there a MAXFIELDTYPE or other validity check? */
		return NULL; /* or should it be " 'NULL' " */
	ismf = type % 2;
	count = 2;
	if( ismf )
		count = count + field->X3D_MFNode.n;
	else
		count = count + 1;
	tokens = (char** )malloc(count*sizeof(char*));
	/*buf = (char*)MALLOC(500); /*just for 1 token - sf string might be the largest*/
	switch (type) 
	{
	case FIELDTYPE_SFFloat:
		tokens[0] = _x3ditoa(FIELDTYPE_SFFloat, buf);
		tokens[1] = _x3ditoa(1,buf);
		tokens[2] = _x3dftoa(field->X3D_SFFloat.value, buf);
		break;
	case FIELDTYPE_MFFloat:
		tokens[0] = _x3ditoa(FIELDTYPE_MFFloat,buf);
		tokens[1] = _x3ditoa(field->X3D_MFNode.n,buf);
		for(i=0;i<count-2;i++)
			tokens[i+2] = _x3dftoa(field->X3D_MFFloat.p[i].value,buf);
		break;
	case FIELDTYPE_MFString:
		tokens[0] = _x3ditoa(FIELDTYPE_MFString,buf);
		tokens[1] = _x3ditoa(field->X3D_MFString.n,buf);
		for(i=0;i<count-2;i++)
			tokens[i+2] = _x3datoa(field->X3D_MFString.p[i].strptr,buf);
		break;
	}
	size = 0;
	for(i=0;i<count;i++) size = size + strlen(tokens[i]);
	/* FREE_IF_NZ(buf); */
	string = MALLOC((size+1)*sizeof(char));
	string[0] = '\0';
	for(i=0;i<count;i++)
	{
		strcat(string,tokens[i]);
		FREE_IF_NZ(tokens[i]);
	}
	FREE_IF_NZ(tokens);
	return string; /* somebody else has to free this (option: return as X3D_SFString then X3D_free */
}

X3DNode* X3D_swigFieldFromString(char* fieldtype, char* values) 
{
	/* 
	issue: swig treats C arrays as opaque pointers - not very helpful. How to create an MF from a sequence of SF fields? 
	goal: convert string into a field - sf or mf - for easy use from swigged scripting language
	/* mf = X3D_fieldFromString("MFString","111.11 222.22"); will do SF as well */
	X3DNode* retval;
	int type,count;
	char* vals; /* **carray, *c; */
	int len,i,n,start,end, inum;
	float f, *farray, **f2,**f3,**f4;
	double d,*darray, **d2,**d3,**d4;
	char *token = NULL;
	char *delim = " ,";

	if(fieldtype == NULL || values == NULL) return NULL;
	type = findFieldInFIELDTYPES(fieldtype);
	/* if( type == -1 ) return NULL; Q. is there an error return code? Where is ffift defined? */
	/* do some qc */
	len = strlen(values);
	vals = (char*) malloc(len); 
	memcpy(vals,values,len); /* getnumtokens is destructive, and a const string produces error - so use copy from original*/
	switch (type) 
	{
	case FIELDTYPE_SFFloat:
		/*
		token = strtok(values,delim); 
		if( sscanf(token,"%f",&f) == 1)
			retval = X3D_newSFFloat(f);
		break;
		*/
	case FIELDTYPE_MFFloat:
	case FIELDTYPE_SFVec2f:
	case FIELDTYPE_MFVec2f:
	case FIELDTYPE_SFVec3f:
	case FIELDTYPE_MFVec3f:
	case FIELDTYPE_SFRotation:
	case FIELDTYPE_MFRotation:
	case FIELDTYPE_SFColorRGBA:
	case FIELDTYPE_MFColorRGBA:
	case FIELDTYPE_SFColor:
	case FIELDTYPE_MFColor:

		count = getnumtokens(vals,delim);
		farray = (float*)MALLOC(count*sizeof(float));
		memcpy(vals,values,len);
		token = strtok(vals,delim);
		count  = 0;
		while (token != NULL) {
			if( sscanf(token,"%f",&f)==1 )/* = strtof(tokens);*/
			{
				farray[count] = f;
				count++;
			}
			token = strtok(NULL,delim);
		}
		f2 = (float**)MALLOC(count*sizeof(float*));
		f3 = (float**)MALLOC(count*sizeof(float*));
		f4 = (float**)MALLOC(count*sizeof(float*));
		for(i=0;i<count;i++) 
		{
			f2[i] = &farray[i*2];
			f3[i] = &farray[i*3];
			f4[i] = &farray[i*4];
		}
		switch(type)
		{
			case FIELDTYPE_SFFloat:
				retval = X3D_newSFFloat(farray[0]);
			case FIELDTYPE_MFFloat:
				retval = X3D_newMFFloat(count, farray);	break;
			case FIELDTYPE_SFVec2f:
				retval = X3D_newSFVec2f(farray[0],farray[1]);break;
			case FIELDTYPE_SFVec3f:
				retval = X3D_newSFVec3f(farray[0],farray[1],farray[2]);break;
			case FIELDTYPE_MFVec3f:
				retval = X3D_newMFVec3f(count,f3);break;
			case FIELDTYPE_SFRotation:
				retval = X3D_newSFRotation(farray[0],farray[1],farray[2],farray[3]);break;
			case FIELDTYPE_MFRotation:
				retval = X3D_newMFRotation(count,f4);break;
			case FIELDTYPE_SFColorRGBA:
				retval = X3D_newSFColorRGBA(farray[0],farray[1],farray[2],farray[3]);break;
			case FIELDTYPE_MFColorRGBA:
				retval = X3D_newMFColorRGBA(count,f4);break;
			case FIELDTYPE_MFColor:
				retval = X3D_newMFColor(count,f3);break;

		}
		FREE_IF_NZ(farray);
		FREE_IF_NZ(f2);
		FREE_IF_NZ(f3);
		FREE_IF_NZ(f4);
		break;
	case FIELDTYPE_SFBool:
	case FIELDTYPE_SFInt32:
		token = strtok(values,delim); /*string delims*/
		if( sscanf(token,"%d",&inum) == 1)
		{
			if(type == FIELDTYPE_SFInt32) retval = X3D_newSFInt32(inum);
			if(type == FIELDTYPE_SFBool) retval = X3D_newSFBool(inum);
		}
		break;
	case FIELDTYPE_SFTime:
	case FIELDTYPE_SFVec2d:
	case FIELDTYPE_SFVec3d:
		count = getnumtokens(vals,delim);
		darray = (double*)MALLOC(count*sizeof(double));
		memcpy(vals,values,len);
		token = strtok(vals,delim);
		count  = 0;
		while (token != NULL) {
			if( sscanf(token,"%lf",&d)==1 )/* = strtof(tokens);*/
			{
				darray[count] = d;
				count++;
			}
			token = strtok(NULL,delim);
		}
		d2 = (double**)MALLOC(count*sizeof(double*));
		d3 = (double**)MALLOC(count*sizeof(double*));
		d4 = (double**)MALLOC(count*sizeof(double*));
		for(i=0;i<count;i++) 
		{
			d2[i] = &darray[i*2];
			d3[i] = &darray[i*3];
			d4[i] = &darray[i*4];
		}
		switch(type)
		{
			case FIELDTYPE_SFTime:
				retval = X3D_newSFTime(darray[0]); break;
			case FIELDTYPE_SFVec2d:
				retval = X3D_newSFVec2d(darray[0],darray[1]);break;
			case FIELDTYPE_SFVec3d:
				retval = X3D_newMFVec3d(count,d3); break;
		}
		FREE_IF_NZ(darray);
		FREE_IF_NZ(d2);
		FREE_IF_NZ(d3);
		FREE_IF_NZ(d4);
		break;


	case FIELDTYPE_SFString:
		/*
		retval->X3D_SFString.type = FIELDTYPE_SFString;
		len = strlen(values);
		retval->X3D_SFString.p[count].strptr = MALLOC((len+1)*sizeof(char));
		strncpy(retval->X3D_SFString.p[count].strptr,values,len);
		retval->X3D_SFString.strptr[len] = '\0';
		retval->X3D_SFString.len = len;
		*/
		retval = X3D_newSFString(values);
		break;

	case FIELDTYPE_MFString:
		retval = malloc(sizeof(X3DNode));
		retval->X3D_MFString.type = FIELDTYPE_MFString; 
		n = 0;
		for(i=0;i<(int)strlen(vals);i++)
			if( vals[i] == '\"' )n++;
		n = n/2;
		/*
		c = (char *)MALLOC(n*STRLEN*sizeof(char));
		carray = (char **)MALLOC(n*(sizeof(char*)));
		for(i=0;i<n;i++)
			carray[i] = &c[i*STRLEN];
		*/
		retval->X3D_MFString.n = n;
		retval->X3D_MFString.p = malloc(sizeof(X3DNode)*retval->X3D_MFString.n);
		count = 0;
		for(i=0,n=0;i<(int)strlen(vals);i++)
		{
			if( vals[i] == '\"' )
			{ 
				n++;
				if(n%2) start = i;
				else
				{
					end = i;
					len = end - start; /* if "Y" start=0,end=2 need a string 2 long for Y and \0. len=end-start= 2-0=2 */
					retval->X3D_MFString.p[count].strptr = MALLOC(len*sizeof(char));
					strncpy(retval->X3D_MFString.p[count].strptr,&vals[start+1],len-1);
					/*strncpy(carray[count],&vals[start+1],len-1);
					carray[count][len-1] = '\0';*/
					retval->X3D_MFString.p[count].strptr[len-1] = '\0';
					retval->X3D_MFString.p[count].len = len-1;
					count++;
				}
			}
		}
		/*retval = X3D_newMFString(count, carray);*/
		break;
	}
	FREE_IF_NZ(vals); /*free(vals);*/
	return retval;
}
X3DNode* anyVrml2X3DNode(int type, union anyVrml* node)
{
	/*
	anyVrml - see CParseGeneral.h L.39 and VrmlTypeList.h and Structs.h L.1270
	Uni_String - see structs.h L.38
	X3DNode - see X3DNode.h
	Strategy: deepcopy from anyVrml to X3DNode (put .type on front of everything) and return
	*/
	X3DNode* retval;
	retval = NULL;
	if( type%2 == 0) /* SF */
	{
		int i;
		retval = X3D_newSF(type);
		switch(type)
		{
		case FIELDTYPE_SFInt32:
			retval->X3D_SFInt32.value = node->sfint32; break;
		case FIELDTYPE_SFBool:
			retval->X3D_SFBool.value = node->sfbool; break;
		case FIELDTYPE_SFFloat:
			retval->X3D_SFFloat.value = node->sffloat; break;
		case FIELDTYPE_SFTime:
			retval->X3D_SFTime.value = node->sftime; break;
		case FIELDTYPE_SFColorRGBA:
		case FIELDTYPE_SFRotation:
			for(i=0;i<4;i++)
				retval->X3D_SFRotation.r[i] = node->sfrotation.r[i];
			break;
		case FIELDTYPE_SFVec2f:
			for(i=0;i<2;i++)
				retval->X3D_SFVec2f.c[i] = node->sfvec2f.c[i];
			break;
		case FIELDTYPE_SFVec3f:
		case FIELDTYPE_SFColor:
			for(i=0;i<3;i++)
				retval->X3D_SFVec3f.c[i] = node->sfvec3f.c[i];
			break;
		case FIELDTYPE_SFVec3d:
			for(i=0;i<3;i++)
				retval->X3D_SFVec3d.c[i] = node->sfvec3d.c[i];
			break;
		case FIELDTYPE_SFVec2d:
			for(i=0;i<2;i++)
				retval->X3D_SFVec2d.c[i] = node->sfvec2d.c[i];
			break;
		case FIELDTYPE_SFString:
			retval->X3D_SFString.len = node->sfstring->len;
			retval->X3D_SFString.strptr = (char*)malloc(node->sfstring->len + 1);
			strncpy(retval->X3D_SFString.strptr,node->sfstring->strptr,node->sfstring->len+1);
			break;
		}
	}
	if( type%2 != 0) /* MF */
	{
		int i,j,n, sftype;

		n = node->mfnode.n;
		retval = _swigNewMF(type, n);
		sftype = type -1;
		switch(sftype)
		{
		case FIELDTYPE_SFInt32:
			for(j=0;j<n;j++)
				retval->X3D_MFInt32.p[j].value = node->mfint32.p[j]; 
			break;
		case FIELDTYPE_SFBool:
			for(j=0;j<n;j++)
				retval->X3D_MFBool.p[j].value = node->mfbool.p[j]; 
			break;
		case FIELDTYPE_SFFloat:
			for(j=0;j<n;j++)
				retval->X3D_MFFloat.p[j].value = node->mffloat.p[j]; 
			break;
		case FIELDTYPE_SFTime:
			for(j=0;j<n;j++)
				retval->X3D_MFTime.p[j].value = node->mftime.p[j]; 
			break;
		case FIELDTYPE_SFColorRGBA:
		case FIELDTYPE_SFRotation:
			for(j=0;j<n;j++)
			  for(i=0;i<4;i++)
				retval->X3D_MFRotation.p[j].r[i] = node->mfrotation.p[j].r[i];
			break;
		case FIELDTYPE_SFVec2f:
			for(j=0;j<n;j++)
 			  for(i=0;i<2;i++)
				retval->X3D_MFVec2f.p[j].c[i] = node->mfvec2f.p[j].c[i];
			break;
		case FIELDTYPE_SFVec3f:
		case FIELDTYPE_SFColor:
			for(j=0;j<n;j++)
			  for(i=0;i<3;i++)
				retval->X3D_MFVec3f.p[j].c[i] = node->mfvec3f.p[j].c[i];
			break;
		case FIELDTYPE_SFVec3d:
			for(j=0;j<n;j++)
			  for(i=0;i<3;i++)
				retval->X3D_MFVec3d.p[j].c[i] = node->mfvec3d.p[j].c[i];
			break;
		case FIELDTYPE_SFString:
			for(j=0;j<n;j++)
			{
				retval->X3D_MFString.p[j].len = node->mfstring.p[j]->len;
				retval->X3D_MFString.p[j].strptr = (char*)malloc(node->mfstring.p[j]->len + 1);
				strncpy(retval->X3D_MFString.p[j].strptr,node->mfstring.p[j]->strptr,node->mfstring.p[j]->len+1);
			}
			break;
		}
	}
	return retval;
}
union anyVrml* getListenerData(int index);
int getListenerType(int index);
X3DNode* X3D_swigCallbackDataFetch(char *ListenerTableIndex)
{
	/* relies on:
	a) what sarah said:
	"the data is sent as a binary block of data, basically as the contents of X3DNode.p or equivalent.  
	If you know what type of data you are receiving from the socket (which you should after reading 
	the first number: the advise index should tie you to a specific callback function), 
	can you not read the information from the socket directly?"
	b) doug's assumption - anyVrml:
	When I trace it looks like if you pass coffset=0 into Parser_scanStringValueToMem as we are doing in _handleFreeWRLcallback(),
	you get back something like SFVec2f L.1278 Structs.h which looks like our X3DNode except missing the .type,
	and I assume it is:
	union anyVrml    - see CParseGeneral.h L.39 and VrmlTypeList.h and Structs.h L.1270
	c) slow - the events are coming spaced out with enough time for you to fetch the data from the listenertable before it gets overwritten
	  there's no table locking. 
	  Alternative: But if this isn't a good assumption, then in the handler below you can anyVrml2X3DNode right there
	  and send the malloced address instead of the listenertable index. Then on fetch, cast the address to an X3DNode.
	*/

	int index, type;
	X3DNode* retval;
	union anyVrml* anynode;

	retval = NULL;
	if( sscanf(ListenerTableIndex,"%d",&index) < 1)
		return retval;
	//anynode = (union anyVrml*)EAI_ListenerTable[index].dataArea;
	//type = EAI_ListenerTable[index].type;
	anynode = getListenerData(index);
	type = getListenerType(index);
	retval = anyVrml2X3DNode(type, anynode);
	return retval;
}

#endif
