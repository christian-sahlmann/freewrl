/*
  =INSERT_TEMPLATE_HERE=

  $Id: jsVRMLBrowser.h,v 1.8 2011/06/04 19:05:42 crc_canada Exp $

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

JSBool VrmlBrowserPrintln(JSContext *cx,
                        JSObject *obj,
                        uintN argc,
                        jsval *argv,
                        jsval *rval);

JSBool VrmlBrowserDeleteRoute(JSContext *cx,
                              JSObject *obj,
                              uintN argc,
                              jsval *argv,
                              jsval *rval);


#ifdef OLDCODE
OLDCODEJSBool VrmlBrowserGetMidiDeviceList(JSContext *cx,
OLDCODE                                    JSObject *obj,
OLDCODE                                    uintN argc,
OLDCODE                                    jsval *argv,
OLDCODE                                    jsval *rval);
OLDCODE
OLDCODEJSBool VrmlBrowserGetMidiDeviceInfo(JSContext *cx,
OLDCODE                                    JSObject *obj,
OLDCODE                                    uintN argc,
OLDCODE                                    jsval *argv,
OLDCODE                                    jsval *rval);
#endif // OLDCODE

#endif /* __FREEWRL_JS_VRML_BROWSER_H__ */
