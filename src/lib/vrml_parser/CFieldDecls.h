/*
=INSERT_TEMPLATE_HERE=

$Id: CFieldDecls.h,v 1.4 2009/05/15 19:42:22 crc_canada Exp $

This is a common base class for FieldDeclarations on PROTOs and Scripts

*/

#ifndef __FREEWRL_FIELD_DECLS_H__
#define __FREEWRL_FIELD_DECLS_H__


struct FieldDecl
{
 indexT mode; /* PKW_initializeOnly PKW_inputOutput, PKW_inputOnly, PKW_outputOnly */
 indexT type; /* field type ,eg FIELDTYPE_MFInt32 */
 indexT name; /* field "name" (its lexer-index) */
 int shaderVariableID; 	/* glGetUniformLocation() cast to int */
 int shaderVariableIsUniform; /* TRUE: this is a Uniform var, or else it is a varying.. */
};

/* Constructor and destructor */
/* ************************** */

struct FieldDecl* newFieldDecl(indexT, indexT, indexT, int, int);
#define deleteFieldDecl(me) \
 FREE_IF_NZ(me)

/* Copies */
#define fieldDecl_copy(me) \
 newFieldDecl((me)->mode, (me)->type, (me)->name, (me)->shaderVariableID, (me)->shaderVariableIsUniform)

/* Accessors */
/* ********* */

#define fieldDecl_getType(me) \
 ((me)->type)
#define fieldDecl_getAccessType(me) \
 ((me)->mode)
#define fieldDecl_getIndexName(me) \
 ((me)->name)

#define fieldDecl_getshaderVariableID(me) \
	(GLint) ((me)->shaderVariableID)

#define fieldDecl_setshaderVariableID(me,varid) \
	((me)->shaderVariableID) = (GLint) (varid)

#define fieldDecl_isshaderVariableUniform(me) \
	((me)->shaderVariableIsUniform)

#define fieldDecl_setshaderVariableUniform(me,val) \
	((me)->shaderVariableIsUniform) = val;



#define fieldDecl_getStringName(lex, me) \
 lexer_stringUser_fieldName(lex, fieldDecl_getIndexName(me), \
  fieldDecl_getAccessType(me))

/* Other members */
/* ************* */

/* Check if this is a given field */
#define fieldDecl_isField(me, nam, mod) \
 ((me)->name==(nam) && (me)->mode==(mod))


#endif /* __FREEWRL_FIELD_DECLS_H__ */
