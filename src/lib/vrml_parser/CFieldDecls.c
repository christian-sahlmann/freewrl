/*
=INSERT_TEMPLATE_HERE=

$Id: CFieldDecls.c,v 1.3 2009/02/11 15:12:55 istakenv Exp $

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

struct FieldDecl* newFieldDecl(indexT mode, indexT type, indexT name)
{
 struct FieldDecl* ret=MALLOC(sizeof(struct FieldDecl));
 ret->mode=mode;
 ret->type=type;
 ret->name=name;

 return ret;
}
