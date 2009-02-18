/*
=INSERT_TEMPLATE_HERE=

$Id: jsUtils.h,v 1.5 2009/02/18 13:37:50 istakenv Exp $

CProto.h - this is the object representing a PROTO definition and being
capable of instantiating it.
 
We keep a vector of pointers to all that pointers which point to "inner
memory" and need therefore be updated when copying.  Such pointers include
field-destinations and parts of ROUTEs.  Those pointers are then simply
copied, their new positions put in the new vector, and afterwards are all
pointers there updated.

*/

#ifndef __FREEWRL_JS_UTILS_H__
#define __FREEWRL_JS_UTILS_H__


#include <system_js.h>

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#define CLEANUP_JAVASCRIPT(cx) \
	/* printf ("calling JS_GC at %s:%d cx %u thread %u\n",__FILE__,__LINE__,cx,pthread_self()); */ \
	JS_GC(cx);

#define LARGESTRING 2048
#define STRING 512
#define SMALLSTRING 128

#define FNAME_STUB "file"
#define LINENO_STUB 0

/* for keeping track of the ECMA values */
struct ECMAValueStruct {
	jsval	JS_address;
	JSContext *context;
	int	valueChanged;
	char 	*name;
};


extern struct ECMAValueStruct ECMAValues[];
extern int maxECMAVal;
int findInECMATable(JSContext *context, jsval toFind);
int findNameInECMATable(JSContext *context, char *toFind);
void resetNameInECMATable(JSContext *context, char *toFind);

/* We keep around the results of script routing, or just script running... */
extern jsval JSCreate_global_return_val;
extern jsval JSglobal_return_val;
extern uintptr_t *JSSFpointer;

int jsrrunScript(JSContext *_context, JSObject *_globalObj, char *script, jsval *rval);
int JS_DefineSFNodeSpecificProperties (JSContext *context, JSObject *object, struct X3D_Node * ptr);

#ifdef JAVASCRIPTVERBOSE
# define ACTUALRUNSCRIPT(a,b,c) ActualrunScript(a,b,c,__FILE__,__LINE__)
/* now in JScript.h -- int ActualrunScript(uintptr_t num, char *script, jsval *rval, char *fn, int line); */
#else
# define ACTUALRUNSCRIPT(a,b,c) ActualrunScript(a,b,c)
/* now in JScript.h -- int ActualrunScript(uintptr_t num, char *script, jsval *rval); */
#endif

int
JSrunScript(uintptr_t num,
			char *script,
			struct Uni_String *rstr,
			struct Uni_String *rnum);

int
JSaddGlobalAssignProperty(uintptr_t num,
						  char *name,
						  char *str);

int
JSaddSFNodeProperty(uintptr_t num,
					char *nodeName,
					char *name,
					char *str);

int
JSaddGlobalECMANativeProperty(uintptr_t num,
							  char *name);

void
reportWarningsOn(void);

void
reportWarningsOff(void);

void
errorReporter(JSContext *cx,
			  const char *message,
			  JSErrorReport *report);

int JSGetProperty(uintptr_t num, char *script, struct Uni_String *rstr);
/* moved to JScript.h -- void JSInit(uintptr_t num); */

void X3D_ECMA_TO_JS(JSContext *cx, void *Data, unsigned datalen, int dataType, jsval *ret);
JSBool setSFNodeField (JSContext *context, JSObject *obj, jsval id, jsval *vp);


#endif /* __FREEWRL_JS_UTILS_H__ */
