/*
=INSERT_TEMPLATE_HERE=

$Id: X3DProtoScript.h,v 1.1 2009/02/18 13:37:50 istakenv Exp $

UI declarations.

*/

#ifndef __FREEWRL_X3DPROTOSCRIPT_X3DPARSER_H__
#define __FREEWRL_X3DPROTOSCRIPT_X3DPARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

void endProtoDeclare(void);
void addToProtoCode(const char *name);
void initScriptWithScript(void);
int getFieldValueFromProtoInterface(char *, int, char **);

#ifdef __cplusplus
}
#endif

#endif /* __FREEWRL_X3DPROTOSCRIPT_X3DPARSER_H__ */
