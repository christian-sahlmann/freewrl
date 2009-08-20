/*
=INSERT_TEMPLATE_HERE=

$Id: X3DProtoScript.h,v 1.3 2009/08/20 19:00:58 crc_canada Exp $

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
void endDumpProtoBody (const char *name);
#ifdef __cplusplus
}
#endif

#endif /* __FREEWRL_X3DPROTOSCRIPT_X3DPARSER_H__ */
