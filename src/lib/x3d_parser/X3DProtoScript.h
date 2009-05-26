/*
=INSERT_TEMPLATE_HERE=

$Id: X3DProtoScript.h,v 1.2 2009/05/26 19:56:32 crc_canada Exp $

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
static int getFieldValueFromProtoInterface(struct VRMLLexer*, char *, int, char **);

#ifdef __cplusplus
}
#endif

#endif /* __FREEWRL_X3DPROTOSCRIPT_X3DPARSER_H__ */
