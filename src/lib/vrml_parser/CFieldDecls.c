/*
=INSERT_TEMPLATE_HERE=

$Id: CFieldDecls.c,v 1.4 2009/05/15 19:42:22 crc_canada Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"


/* ************************************************************************** */
/* ********************************** FieldDecl ***************************** */
/* ************************************************************************** */

/* Constructor and destructor */
/* ************************** */

struct FieldDecl* newFieldDecl(indexT mode, indexT type, indexT name, int shv, int shvuni)
{
 struct FieldDecl* ret=MALLOC(sizeof(struct FieldDecl));
 ret->mode=mode;
 ret->type=type;
 ret->name=name;
 ret->shaderVariableID=shv;
 ret->shaderVariableIsUniform=shvuni;

 return ret;
}
