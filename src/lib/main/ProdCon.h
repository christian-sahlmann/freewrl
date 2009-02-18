/*
=INSERT_TEMPLATE_HERE=

$Id: ProdCon.h,v 1.2 2009/02/18 16:24:04 istakenv Exp $

UI declarations.

*/

#ifndef __FREEWRL_PRODCON_MAIN_H__
#define __FREEWRL_PRODCON_MAIN_H__

/* for the definition of X3D_Node */
#include "../vrml_parser/Structs.h"


#ifdef __cplusplus
extern "C" {
#endif

int isinputThreadParsing(void);
int isInputThreadInitialized(void);
void initializeInputParseThread(void);
void registerBindable (struct X3D_Node *);

#ifdef __cplusplus
}
#endif

#endif /* __FREEWRL_PRODCON_MAIN_H__ */
