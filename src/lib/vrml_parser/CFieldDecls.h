/*
=INSERT_TEMPLATE_HERE=

$Id: CFieldDecls.h,v 1.9 2009/11/06 00:09:01 crc_canada Exp $

This is a common base class for FieldDeclarations on PROTOs and Scripts

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


#ifndef __FREEWRL_FIELD_DECLS_H__
#define __FREEWRL_FIELD_DECLS_H__


struct FieldDecl
{
 indexT PKWmode; /* PKW_initializeOnly PKW_inputOutput, PKW_inputOnly, PKW_outputOnly */
 indexT fieldType; /* field type ,eg FIELDTYPE_MFInt32 */
 indexT lexerNameIndex; /* field "name" (its lexer-index) */
 indexT JSparamNameIndex; /* JSparamname - has index unique with fieldType */
 int shaderVariableID; 	/* glGetUniformLocation() cast to int */
};

/* Constructor and destructor */
/* ************************** */

struct FieldDecl* newFieldDecl(indexT, indexT, indexT, int, int);
#define deleteFieldDecl(me) \
 FREE_IF_NZ(me)

/* Copies */
#define fieldDecl_copy(me) \
 newFieldDecl((me)->PKWmode, (me)->fieldType, (me)->lexerNameIndex, \
		(me)->JSparamNameIndex, (me)->shaderVariableID)

/* Accessors */
/* ********* */

#define fieldDecl_getType(me) \
 ((me)->fieldType)
#define fieldDecl_getAccessType(me) \
 ((me)->PKWmode)
#define fieldDecl_getShaderScriptIndex(me) \
 ((me)->JSparamNameIndex)
#define fieldDecl_getIndexName(me) \
 ((me)->lexerNameIndex)
#define fieldDecl_getshaderVariableID(me) \
	(GLint) ((me)->shaderVariableID)

#define fieldDecl_setshaderVariableID(me,varid) \
	((me)->shaderVariableID) = (GLint) (varid)

#define fieldDecl_getStringName(lex, me) \
 lexer_stringUser_fieldName(lex, fieldDecl_getIndexName(me), \
  fieldDecl_getAccessType(me))

/* Other members */
/* ************* */

/* Check if this is a given field */
#define fieldDecl_isField(me, nam, mod) \
 ((me)->lexerNameIndex==(nam) && (me)->PKWmode==(mod))


#endif /* __FREEWRL_FIELD_DECLS_H__ */
