/*
=INSERT_TEMPLATE_HERE=

$Id: jsUtils.h,v 1.13 2011/04/21 16:40:41 crc_canada Exp $

CProto.h - this is the object representing a PROTO definition and being
capable of instantiating it.
 
We keep a vector of pointers to all that pointers which point to "inner
memory" and need therefore be updated when copying.  Such pointers include
field-destinations and parts of ROUTEs.  Those pointers are then simply
copied, their new positions put in the new vector, and afterwards are all
pointers there updated.

*/

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/


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
extern void *JSSFpointer;

int jsrrunScript(JSContext *_context, JSObject *_globalObj, char *script, jsval *rval);
int JS_DefineSFNodeSpecificProperties (JSContext *context, JSObject *object, struct X3D_Node * ptr);

#ifdef JAVASCRIPTVERBOSE
# define ACTUALRUNSCRIPT(a,b,c) ActualrunScript(a,b,c,__FILE__,__LINE__)
/* now in JScript.h -- int ActualrunScript(uintptr_t num, char *script, jsval *rval, char *fn, int line); */
#else
# define ACTUALRUNSCRIPT(a,b,c) ActualrunScript(a,b,c)
/* now in JScript.h -- int ActualrunScript(uintptr_t num, char *script, jsval *rval); */
#endif

void
reportWarningsOn(void);

void
reportWarningsOff(void);

void
errorReporter(JSContext *cx,
			  const char *message,
			  JSErrorReport *report);

void X3D_ECMA_TO_JS(JSContext *cx, void *Data, int datalen, int dataType, jsval *ret);
#if JS_VERSION < 185
typedef  jsval jsid;
JSBool setSFNodeField (JSContext *context, JSObject *obj, jsid id, jsval *vp);
#else
JSBool setSFNodeField (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
#endif

const char *classToString(JSClass *myClass);
#define CHECK_CLASS(cx,obj,argv,fnString,expClass) \
/* printf ("CHECK_CLASS, obj %u, argv %u\n",obj,argv);*/ \
    	if (!JS_InstanceOf(cx, obj, &expClass, argv)) { \
		printf ("Javascript Instance problem in '%s' - expected a '%s', got a ", fnString, classToString(&expClass)); \
		printJSNodeType (cx,obj); \
		return JS_FALSE; \
	} 

#endif /* __FREEWRL_JS_UTILS_H__ */
