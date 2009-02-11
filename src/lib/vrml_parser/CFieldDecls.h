/*
=INSERT_TEMPLATE_HERE=

$Id: CFieldDecls.h,v 1.3 2009/02/11 15:12:55 istakenv Exp $

This is a common base class for FieldDeclarations on PROTOs and Scripts

*/

#ifndef __FREEWRL_FIELD_DECLS_H__
#define __FREEWRL_FIELD_DECLS_H__


struct FieldDecl
{
 indexT mode; /* field, exposedField, eventIn, eventOut */
 indexT type; /* field type */
 indexT name; /* field "name" (its lexer-index) */
};

/* Constructor and destructor */
/* ************************** */

struct FieldDecl* newFieldDecl(indexT, indexT, indexT);
#define deleteFieldDecl(me) \
 FREE_IF_NZ(me)

/* Copies */
#define fieldDecl_copy(me) \
 newFieldDecl((me)->mode, (me)->type, (me)->name)

/* Accessors */
/* ********* */

#define fieldDecl_getType(me) \
 ((me)->type)
#define fieldDecl_getAccessType(me) \
 ((me)->mode)
#define fieldDecl_getIndexName(me) \
 ((me)->name)
#define fieldDecl_getStringName(lex, me) \
 lexer_stringUser_fieldName(lex, fieldDecl_getIndexName(me), \
  fieldDecl_getAccessType(me))

/* Other members */
/* ************* */

/* Check if this is a given field */
#define fieldDecl_isField(me, nam, mod) \
 ((me)->name==(nam) && (me)->mode==(mod))


#endif /* __FREEWRL_FIELD_DECLS_H__ */
