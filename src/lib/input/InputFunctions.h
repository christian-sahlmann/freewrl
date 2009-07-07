/*
=INSERT_TEMPLATE_HERE=

$Id: InputFunctions.h,v 1.2 2009/07/07 15:12:02 crc_canada Exp $

Global includes.

*/

#ifndef __FREEWRL_INPUTFUNCTIONS_H__
#define __FREEWRL_INPUTFUNCTIONS_H__

/**
 * in InputFunctions.c
 */
int dirExists(const char *dir);
char* makeFontDirectory();
char *readInputString(char *fn);
FILE *openLocalFile (char *fn, char* access);
void unlinkShadowFile(char *fn);
void addShadowFile(char *x3dname, char *myshadowname);
char *getShadowFileNamePtr (char *fn);
void kill_shadowFileTable (void);


#endif /* __FREEWRL_INPUTFUNCTIONS_H__ */

