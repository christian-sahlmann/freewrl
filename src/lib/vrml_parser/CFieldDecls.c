/*
=INSERT_TEMPLATE_HERE=

$Id: CFieldDecls.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

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

struct FieldDecl* newFieldDecl(indexT mode, indexT type, indexT name)
{
 struct FieldDecl* ret=MALLOC(sizeof(struct FieldDecl));
 ret->mode=mode;
 ret->type=type;
 ret->name=name;

 return ret;
}
