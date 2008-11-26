/*
 * Copyright (C) 1998 Tuomas J. Lukka, 2002 John Stewart, Ayla Khan CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License
 * (file COPYING in the distribution) for conditions of use and
 * redistribution, EXCEPT on the files which belong under the
 * Mozilla public license.
 *
 * $Id: jsVRMLBrowser.h,v 1.1 2008/11/26 11:24:15 couannette Exp $
 *
 */


#ifndef __jsVRMLBrowser_h__
#define __jsVRMLBrowser_h__
#ifndef UNUSED
#define UNUSED(v) ((void) v)
#endif

#include "jsUtils.h"
#include "jsNative.h"

extern char *BrowserName; /* defined in VRMLC.pm */
extern double BrowserFPS;				/* defined in VRMLC.pm */

#define BROWMAGIC 12345

JSBool VrmlBrowserInit(JSContext *context, JSObject *globalObj,	BrowserNative *brow );


JSBool
VrmlBrowserGetName(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);


JSBool
VrmlBrowserGetVersion(JSContext *cx,
					  JSObject *obj,
					  uintN argc,
					  jsval *argv,
					  jsval *rval);


JSBool
VrmlBrowserGetCurrentSpeed(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);


JSBool
VrmlBrowserGetCurrentFrameRate(JSContext *cx,
						   JSObject *obj,
						   uintN argc,
						   jsval *argv,
						   jsval *rval);


JSBool
VrmlBrowserGetWorldURL(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);


JSBool
VrmlBrowserReplaceWorld(JSContext *cx,
					JSObject *obj,
					uintN argc,
					jsval *argv,
					jsval *rval);


JSBool
VrmlBrowserLoadURL(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);


JSBool
VrmlBrowserSetDescription(JSContext *cx,
						  JSObject *obj,
						  uintN argc,
						  jsval *argv,
						  jsval *rval);


JSBool
VrmlBrowserCreateVrmlFromString(JSContext *cx,
						  JSObject *obj,
								uintN argc,
								jsval *argv,
								jsval *rval);


JSBool
VrmlBrowserCreateVrmlFromURL(JSContext *cx,
							 JSObject *obj,
							 uintN argc,
							 jsval *argv,
							 jsval *rval);


JSBool
VrmlBrowserAddRoute(JSContext *cx,
					JSObject *obj,
					uintN argc,
					jsval *argv,
					jsval *rval);


JSBool
VrmlBrowserPrint(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);

JSBool
VrmlBrowserDeleteRoute(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);

JSBool
VrmlBrowserGetMidiDeviceList(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);

JSBool
VrmlBrowserGetMidiDeviceInfo(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);



static JSClass Browser = {
	"Browser",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub
};



#endif /* __jsVRMLBrowser_h__ */
