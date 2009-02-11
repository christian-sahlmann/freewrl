/*
=INSERT_TEMPLATE_HERE=

$Id: JScript.h,v 1.2 2009/02/11 15:12:55 istakenv Exp $

???

*/

#ifndef __FREEWRL_JS_JSCRIPT_H__
#define __FREEWRL_JS_JSCRIPT_H__


void kill_javascript(void);
void cleanupDie(uintptr_t num, const char *msg);
void JSMaxAlloc(void);
void JSInit(uintptr_t num);
void SaveScriptText(uintptr_t num, char *text);
void JSInitializeScriptAndFields (uintptr_t num);
void JSCreateScriptContext(uintptr_t num);

#ifdef JAVASCRIPTVERBOSE
int ActualrunScript(uintptr_t num, char *script, jsval *rval, char *fn, int line);
#else
int ActualrunScript(uintptr_t num, char *script, jsval *rval);
#endif

int jsrrunScript(JSContext *_context, JSObject *_globalObj, char *script, jsval *rval);
void * SFNodeNativeNew(void);
int SFNodeNativeAssign(void *top, void *fromp);
void * SFColorRGBANativeNew(void);
void SFColorRGBANativeAssign(void *top, void *fromp);
void * SFColorNativeNew(void);
void SFColorNativeAssign(void *top, void *fromp);
void * SFImageNativeNew(void);
void SFImageNativeAssign(void *top, void *fromp);
void * SFRotationNativeNew(void);
void SFRotationNativeAssign(void *top, void *fromp);
void * SFVec2fNativeNew(void);
void SFVec2fNativeAssign(void *top, void *fromp);
void * SFVec3fNativeNew(void);
void SFVec3fNativeAssign(void *top, void *fromp);
void * SFVec3dNativeNew(void);
void SFVec3dNativeAssign(void *top, void *fromp);
void SaveScriptField (int num, indexT kind, indexT type, char* field, union anyVrml value);
void InitScriptField(int num, indexT kind, indexT type, char* field, union anyVrml value);
int JSaddGlobalECMANativeProperty(uintptr_t num, char *name);
int JSaddGlobalAssignProperty(uintptr_t num, char *name, char *str);


#endif /* __FREEWRL_JS_JSCRIPT_H__ */
