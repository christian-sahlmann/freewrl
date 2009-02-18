/*
=INSERT_TEMPLATE_HERE=

$Id: ProdCon.h,v 1.1 2009/02/18 13:37:50 istakenv Exp $

UI declarations.

*/

#ifndef __FREEWRL_PRODCON_MAIN_H__
#define __FREEWRL_PRODCON_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

int isinputThreadParsing(void);
int isInputThreadInitialized(void);
void initializeInputParseThread(void);

#ifdef __cplusplus
}
#endif

#endif /* __FREEWRL_PRODCON_MAIN_H__ */
