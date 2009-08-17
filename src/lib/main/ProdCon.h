/*
=INSERT_TEMPLATE_HERE=

$Id: ProdCon.h,v 1.3 2009/08/17 22:25:58 couannette Exp $

General functions declarations.

*/

#ifndef __FREEWRL_PRODCON_MAIN_H__
#define __FREEWRL_PRODCON_MAIN_H__


int isinputThreadParsing(void);
int isInputThreadInitialized(void);
void initializeInputParseThread(void);
void registerBindable(struct X3D_Node *);

char* do_get_url(const char *url);
bool do_file_exists(const char *filename);


#endif /* __FREEWRL_PRODCON_MAIN_H__ */
