/*
 * Copyright (C) 1998 Tuomas J. Lukka, 2002 John Stewart, Ayla Khan CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License
 * (file COPYING in the distribution) for conditions of use and
 * redistribution, EXCEPT on the files which belong under the
 * Mozilla public license.
 *
 * $Id: jsUtils.h,v 1.1 2008/11/26 11:24:15 couannette Exp $
 */

#ifndef __jsUtils_h__
#define __jsUtils_h__

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "jsapi.h" /* JS compiler */
#include "jsdbgapi.h" /* JS debugger */

#include "headers.h"

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
#define ACTUALRUNSCRIPT(a,b,c) ActualrunScript(a,b,c,__FILE__,__LINE__)
int ActualrunScript(uintptr_t num, char *script, jsval *rval, char *fn, int line);
#else
#define ACTUALRUNSCRIPT(a,b,c) ActualrunScript(a,b,c)
int ActualrunScript(uintptr_t num, char *script, jsval *rval);
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
void JSInit(uintptr_t num);

void X3D_ECMA_TO_JS(JSContext *cx, void *Data, unsigned datalen, int dataType, jsval *ret);
JSBool setSFNodeField (JSContext *context, JSObject *obj, jsval id, jsval *vp);

#endif /* __jsUtils_h__ */
