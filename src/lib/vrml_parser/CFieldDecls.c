/*******************************************************************
  Copyright (C) 2007 Daniel Kraft,  John Stewart, CRC Canada.
  DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
  See the GNU Library General Public License (file COPYING in the distribution)
  for conditions of use and redistribution.
 *********************************************************************/



/* CFieldDecls.c - Sourcecode for CFieldDecls.h */

#include <stdlib.h>
#include <assert.h>

#include "CFieldDecls.h"

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
