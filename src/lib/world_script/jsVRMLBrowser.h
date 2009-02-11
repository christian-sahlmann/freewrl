/*
  =INSERT_TEMPLATE_HERE=

  $Id: jsVRMLBrowser.h,v 1.4 2009/02/11 15:12:55 istakenv Exp $

*/

#ifndef __FREEWRL_JS_VRML_BROWSER_H__
#define __FREEWRL_JS_VRML_BROWSER_H__


#ifndef UNUSED
#define UNUSED(v) ((void) v)
#endif

extern char *BrowserName; /* defined in VRMLC.pm */
extern double BrowserFPS;                               /* defined in VRMLC.pm */

#define BROWMAGIC 12345

JSBool VrmlBrowserInit(JSContext *context, JSObject *globalObj, BrowserNative *brow);


JSBool VrmlBrowserGetName(JSContext *cx,
                          JSObject *obj,
                          uintN argc,
                          jsval *argv,
                          jsval *rval);


JSBool VrmlBrowserGetVersion(JSContext *cx,
                             JSObject *obj,
                             uintN argc,
                             jsval *argv,
                             jsval *rval);


JSBool VrmlBrowserGetCurrentSpeed(JSContext *cx,
                                  JSObject *obj,
                                  uintN argc,
                                  jsval *argv,
                                  jsval *rval);


JSBool VrmlBrowserGetCurrentFrameRate(JSContext *cx,
                                      JSObject *obj,
                                      uintN argc,
                                      jsval *argv,
                                      jsval *rval);


JSBool VrmlBrowserGetWorldURL(JSContext *cx,
                              JSObject *obj,
                              uintN argc,
                              jsval *argv,
                              jsval *rval);


JSBool VrmlBrowserReplaceWorld(JSContext *cx,
                               JSObject *obj,
                               uintN argc,
                               jsval *argv,
                               jsval *rval);


JSBool VrmlBrowserLoadURL(JSContext *cx,
                          JSObject *obj,
                          uintN argc,
                          jsval *argv,
                          jsval *rval);


JSBool VrmlBrowserSetDescription(JSContext *cx,
                                 JSObject *obj,
                                 uintN argc,
                                 jsval *argv,
                                 jsval *rval);


JSBool VrmlBrowserCreateVrmlFromString(JSContext *cx,
                                       JSObject *obj,
                                       uintN argc,
                                       jsval *argv,
                                       jsval *rval);


JSBool VrmlBrowserCreateVrmlFromURL(JSContext *cx,
                                    JSObject *obj,
                                    uintN argc,
                                    jsval *argv,
                                    jsval *rval);


JSBool VrmlBrowserAddRoute(JSContext *cx,
                           JSObject *obj,
                           uintN argc,
                           jsval *argv,
                           jsval *rval);


JSBool VrmlBrowserPrint(JSContext *cx,
                        JSObject *obj,
                        uintN argc,
                        jsval *argv,
                        jsval *rval);

JSBool VrmlBrowserDeleteRoute(JSContext *cx,
                              JSObject *obj,
                              uintN argc,
                              jsval *argv,
                              jsval *rval);

JSBool VrmlBrowserGetMidiDeviceList(JSContext *cx,
                                    JSObject *obj,
                                    uintN argc,
                                    jsval *argv,
                                    jsval *rval);

JSBool VrmlBrowserGetMidiDeviceInfo(JSContext *cx,
                                    JSObject *obj,
                                    uintN argc,
                                    jsval *argv,
                                    jsval *rval);


#endif /* __FREEWRL_JS_VRML_BROWSER_H__ */
