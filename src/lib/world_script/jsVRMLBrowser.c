/*
=INSERT_TEMPLATE_HERE=

$Id: jsVRMLBrowser.c,v 1.56 2012/08/28 15:33:53 crc_canada Exp $

Javascript C language binding.

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


#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>
#include <list.h>
#include <resources.h>

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../opengl/OpenGL_Utils.h"
#include "../main/headers.h"
#include "../scenegraph/RenderFuncs.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CProto.h"
#include "../vrml_parser/CParse.h"
#include "../main/Snapshot.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../x3d_parser/Bindable.h"
#include "../input/EAIHeaders.h"	/* for implicit declarations */


#include "JScript.h"
#include "CScripts.h"
#include "fieldSet.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"
#include "jsVRMLBrowser.h"

#ifdef HAVE_JAVASCRIPT

//Q. is this a true sharable static?
static JSClass Browser = {
    "Browser",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
#if JS_VERSION < 185
    JS_PropertyStub,
#else
    JS_StrictPropertyStub,
#endif
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub
};

static JSBool doVRMLRoute(JSContext *context, JSObject *obj, uintN argc, jsval *argv, const char *browserFunc); 

static JSFunctionSpec (BrowserFunctions)[] = {
	{"getName", VrmlBrowserGetName, 0},
	{"getVersion", VrmlBrowserGetVersion, 0},
	{"getCurrentSpeed", VrmlBrowserGetCurrentSpeed, 0},
	{"getCurrentFrameRate", VrmlBrowserGetCurrentFrameRate, 0},
	{"getWorldURL", VrmlBrowserGetWorldURL, 0},
	{"replaceWorld", VrmlBrowserReplaceWorld, 0},
	{"loadURL", VrmlBrowserLoadURL, 0},
	{"setDescription", VrmlBrowserSetDescription, 0},
	{"createVrmlFromString", VrmlBrowserCreateVrmlFromString, 0},
	{"createVrmlFromURL", VrmlBrowserCreateVrmlFromURL, 0},
	{"createX3DFromString", VrmlBrowserCreateVrmlFromString, 0},
	{"createX3DFromURL", VrmlBrowserCreateVrmlFromURL, 0},
	{"addRoute", VrmlBrowserAddRoute, 0},
	{"deleteRoute", VrmlBrowserDeleteRoute, 0},
	{"print", VrmlBrowserPrint, 0},
	{"println", VrmlBrowserPrintln, 0},
	{0}
};


///* for setting field values to the output of a CreateVrml style of call */
///* it is kept at zero, unless it has been used. Then it is reset to zero */
//jsval JSCreate_global_return_val;
typedef struct pjsVRMLBrowser{
	int ijunk;
}* ppjsVRMLBrowser;
void *jsVRMLBrowser_constructor(){
	void *v = malloc(sizeof(struct pjsVRMLBrowser));
	memset(v,0,sizeof(struct pjsVRMLBrowser));
	return v;
}
void jsVRMLBrowser_init(struct tjsVRMLBrowser *t){
	//public
	//private
	t->prv = jsVRMLBrowser_constructor();
	{
		ppjsVRMLBrowser p = (ppjsVRMLBrowser)t->prv;
	}
}
//	ppjsVRMLBrowser p = (ppjsVRMLBrowser)gglobal()->jsVRMLBrowser.prv;
/* we add/remove routes with this call */
void jsRegisterRoute(
	struct X3D_Node* from, int fromOfs,
	struct X3D_Node* to, int toOfs,
	int len, const char *adrem) {
	int ad;

	if (strcmp("addRoute",adrem) == 0) 
		ad = 1;
	else ad = 0;

 	CRoutes_Register(ad, from, fromOfs, to, toOfs , len, 
 		 returnInterpolatorPointer(stringNodeType(to->_nodeType)), 0, 0);
}
 

/* used in loadURL*/
void conCat (char *out, char *in) {

	while (strlen (in) > 0) {
		strcat (out," :loadURLStringBreak:");
		while (*out != '\0') out++;

		if (*in == '[') in++;
		while ((*in != '\0') && (*in == ' ')) in++;
		if (*in == '"') {
			in++;
			/* printf ("have the initial quote string here is %s\n",in); */
			while (*in != '"') { *out = *in; out++; in++; }
			*out = '\0';
			/* printf ("found string is :%s:\n",tfilename); */
		}

		/* skip along to the start of the next name */
		if (*in == '"') in++;
		if (*in == ',') in++;
		if (*in == ']') in++; /* this allows us to leave */
	}
}



void createLoadUrlString(char *out, int outLen, char *url, char *param) {
	int commacount1;
	int commacount2;
	char *tptr;
	char *orig;

	/* mimic the EAI loadURL, java code is:
        // send along sizes of the Strings
        SysString = "" + url.length + " " + parameter.length;
                
        for (count=0; count<url.length; count++) {
                SysString = SysString + " :loadURLStringBreak:" + url[count];
        }       

        for (count=0; count<parameter.length; count++) {
                SysString = SysString + " :loadURLStringBreak:" + parameter[count];
        }
	*/

	/* keep an original copy of the pointer */
	orig = out;
	
	/* find out how many elements there are */

	commacount1 = 0; commacount2 = 0;
	tptr = url; while (*tptr != '\0') { if (*tptr == '"') commacount1 ++; tptr++; }
	tptr = param; while (*tptr != '\0') { if (*tptr == '"') commacount2 ++; tptr++; }
	commacount1 = commacount1 / 2;
	commacount2 = commacount2 / 2;

	if ((	strlen(url) +
		strlen(param) +
		(commacount1 * strlen (" :loadURLStringBreak:")) +
		(commacount2 * strlen (" :loadURLStringBreak:"))) > (outLen - 20)) {
		printf ("createLoadUrlString, string too long\n");
		return;
	}

	sprintf (out,"%d %d",commacount1,commacount2);
	
	/* go to the end of this string */
	while (*out != '\0') out++;

	/* go through the elements and find which (if any) url exists */	
	conCat (out,url);
	while (*out != '\0') out++;
	conCat (out,param);
}

JSBool
VrmlBrowserInit(JSContext *context, JSObject *globalObj, BrowserNative *brow)
{
	JSObject *obj;
	ttglobal tg = gglobal();
	tg->jsVRMLBrowser.JSCreate_global_return_val = INT_TO_JSVAL(0);

	#ifdef JSVERBOSE
		printf("VrmlBrowserInit\n");
	#endif

	obj = JS_DefineObject(context, globalObj, "Browser", &Browser, NULL, 
			JSPROP_ENUMERATE | JSPROP_PERMANENT);
	if (!JS_DefineFunctions(context, obj, BrowserFunctions)) {
		printf( "JS_DefineFunctions failed in VrmlBrowserInit.\n");
		return JS_FALSE;
	}

	if (!JS_SetPrivate(context, obj, brow)) {
		printf( "JS_SetPrivate failed in VrmlBrowserInit.\n");
		return JS_FALSE;
	}
	return JS_TRUE;
}


JSBool
#if JS_VERSION < 185
VrmlBrowserGetName(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
VrmlBrowserGetName(JSContext *context, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(context,vp);
	jsval *argv = JS_ARGV(context,vp);
#endif
	JSString *_str;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	_str = JS_NewStringCopyZ(context,BrowserName);
#if JS_VERSION < 185
	*rval = STRING_TO_JSVAL(_str);
#else
	JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
#endif
	return JS_TRUE;
}


/* get the string stored in FWVER into a jsObject */
JSBool
#if JS_VERSION < 185
VrmlBrowserGetVersion(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
VrmlBrowserGetVersion(JSContext *context, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(context,vp);
	jsval *argv = JS_ARGV(context,vp);
#endif
	JSString *_str;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	_str = JS_NewStringCopyZ(context, libFreeWRL_get_version());
#if JS_VERSION < 185
	*rval = STRING_TO_JSVAL(_str);
#else
	JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
#endif
	return JS_TRUE;
}


JSBool
#if JS_VERSION < 185
VrmlBrowserGetCurrentSpeed(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
VrmlBrowserGetCurrentSpeed(JSContext *context, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(context,vp);
	jsval *argv = JS_ARGV(context,vp);
#endif
	JSString *_str;
	char string[1000];

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	/* get the variable updated */
	getCurrentSpeed();
	sprintf (string,"%f",gglobal()->Mainloop.BrowserSpeed);
	_str = JS_NewStringCopyZ(context,string);
#if JS_VERSION < 185
        *rval = STRING_TO_JSVAL(_str);
#else
        JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
#endif
	return JS_TRUE;
}


JSBool
#if JS_VERSION < 185
VrmlBrowserGetCurrentFrameRate(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
VrmlBrowserGetCurrentFrameRate(JSContext *context, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(context,vp);
	jsval *argv = JS_ARGV(context,vp);
#endif
	JSString *_str;
	char FPSstring[1000];

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	sprintf (FPSstring,"%6.2f",gglobal()->Mainloop.BrowserFPS);
	_str = JS_NewStringCopyZ(context,FPSstring);
#if JS_VERSION < 185
        *rval = STRING_TO_JSVAL(_str);
#else
        JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
#endif
	return JS_TRUE;
}


JSBool
#if JS_VERSION < 185
VrmlBrowserGetWorldURL(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
VrmlBrowserGetWorldURL(JSContext *context, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(context,vp);
        jsval *argv = JS_ARGV(context,vp);
#endif
	JSString *_str;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	_str = JS_NewStringCopyZ(context,BrowserFullPath);
#if JS_VERSION < 185
        *rval = STRING_TO_JSVAL(_str);
#else
        JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
#endif
	return JS_TRUE;
}


JSBool
#if JS_VERSION < 185
VrmlBrowserReplaceWorld(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
VrmlBrowserReplaceWorld(JSContext *context, uintN argc, jsval *vp) {
        jsval *argv = JS_ARGV(context,vp);
#endif
	JSObject *_obj;
	JSString *_str;
	JSClass *_cls;
	jsval _rval = INT_TO_JSVAL(0);
	char *_c_args = "MFNode nodes",
		*_costr,
		*_c_format = "o";
	char *tptr;

	if (JS_ConvertArguments(context, argc, argv, _c_format, &_obj)) {
		if ((_cls = JS_GET_CLASS(context, _obj)) == NULL) {
			printf("JS_GetClass failed in VrmlBrowserReplaceWorld.\n");
			return JS_FALSE;
		}

		if (memcmp("MFNode", _cls->name, strlen(_cls->name)) != 0) {
			printf( "\nIncorrect argument in VrmlBrowserReplaceWorld.\n");
			return JS_FALSE;
		}
		_str = JS_ValueToString(context, argv[0]);
#if JS_VERSION < 185
		_costr = JS_GetStringBytes(_str);
#else
		_costr = JS_EncodeString(context,_str);
#endif
		/* sanitize string, for the EAI_RW call (see EAI_RW code) */
		tptr = _costr;
		while (*tptr != '\0') {
			if(*tptr == '[') *tptr = ' ';
			if(*tptr == ']') *tptr = ' ';
			if(*tptr == ',') *tptr = ' ';
			tptr++;
		}
		EAI_RW(_costr);
#if JS_VERSION >= 185
		JS_free(context,_costr);
#endif
	} else {
		printf( "\nIncorrect argument format for replaceWorld(%s).\n", _c_args);
		return JS_FALSE;
	}
#if JS_VERSION < 185
	*rval = _rval;
#else
	JS_SET_RVAL(context,vp,_rval);
#endif

	return JS_TRUE;
}
struct X3D_Anchor* get_EAIEventsIn_AnchorNode();
JSBool
#if JS_VERSION < 185
VrmlBrowserLoadURL(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
VrmlBrowserLoadURL(JSContext *context, uintN argc, jsval *vp) {
        jsval *argv = JS_ARGV(context,vp);
#endif
	JSObject *_obj[2];
	JSString *_str[2];
	JSClass *_cls[2];
	char *_c_args = "MFString url, MFString parameter",
		*_costr[2],
		*_c_format = "o o";
	#define myBufSize 2000
	char myBuf[myBufSize];

	if (JS_ConvertArguments(context, argc, argv, _c_format, &(_obj[0]), &(_obj[1]))) {
		if ((_cls[0] = JS_GET_CLASS(context, _obj[0])) == NULL) {
			printf( "JS_GetClass failed for arg 0 in VrmlBrowserLoadURL.\n");
			return JS_FALSE;
		}
		if ((_cls[1] = JS_GET_CLASS(context, _obj[1])) == NULL) {
			printf( "JS_GetClass failed for arg 1 in VrmlBrowserLoadURL.\n");
			return JS_FALSE;
		}
		if (memcmp("MFString", (_cls[0])->name, strlen((_cls[0])->name)) != 0 &&
			memcmp("MFString", (_cls[1])->name, strlen((_cls[1])->name)) != 0) {
			printf( "\nIncorrect arguments in VrmlBrowserLoadURL.\n");
			return JS_FALSE;
		}
		_str[0] = JS_ValueToString(context, argv[0]);
#if JS_VERSION < 185
		_costr[0] = JS_GetStringBytes(_str[0]);
#else
		_costr[0] = JS_EncodeString(context,_str[0]);
#endif

		_str[1] = JS_ValueToString(context, argv[1]);
#if JS_VERSION < 185
		_costr[1] = JS_GetStringBytes(_str[1]);
#else
		_costr[1] = JS_EncodeString(context,_str[1]);
#endif

		/* we use the EAI code for this - so reformat this for the EAI format */
		{
			//extern struct X3D_Anchor EAI_AnchorNode;  /* win32 C doesnt like new declarations in the middle of executables - start a new scope {} and put dec at top */

			/* make up the URL from what we currently know */
			createLoadUrlString(myBuf,myBufSize,_costr[0], _costr[1]);
			createLoadURL(myBuf);

			/* now tell the fwl_RenderSceneUpdateScene that BrowserAction is requested... */
			setAnchorsAnchor( get_EAIEventsIn_AnchorNode()); //&gglobal()->EAIEventsIn.EAI_AnchorNode;
		}
		gglobal()->RenderFuncs.BrowserAction = TRUE;

#if JS_VERSION >= 185
		JS_free(context,_costr[0]);
		JS_free(context,_costr[1]);
#endif
	} else {
		printf( "\nIncorrect argument format for loadURL(%s).\n", _c_args);
		return JS_FALSE;
	}
#if JS_VERSION < 185
	*rval = INT_TO_JSVAL(0);
#else
	JS_SET_RVAL(context,vp,JSVAL_ZERO);
#endif

	return JS_TRUE;
}


JSBool
#if JS_VERSION < 185
VrmlBrowserSetDescription(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	char *_c_format = "s";
#else
VrmlBrowserSetDescription(JSContext *context, uintN argc, jsval *vp) {
        jsval *argv = JS_ARGV(context,vp);
	JSString *js_c;
	char *_c_format = "S";
#endif
	char *_c, *_c_args = "SFString description";

	if (argc == 1 &&
#if JS_VERSION < 185
		JS_ConvertArguments(context, argc, argv, _c_format, &_c)) {
#else
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
			/* _c = JS_EncodeString(context,js_c);
			...why encode the string when we just have to JS_free it later? */
#endif

		/* we do not do anything with the description. If we ever wanted to, it is in _c */
#if JS_VERSION < 185
		*rval = INT_TO_JSVAL(0);
#else
		JS_SET_RVAL(context,vp,JSVAL_ZERO);
#endif
	} else {
		printf( "\nIncorrect argument format for setDescription(%s).\n", _c_args);
		return JS_FALSE;
	}
	return JS_TRUE;
}


JSBool
#if JS_VERSION < 185
VrmlBrowserCreateVrmlFromString(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	char *_c_format = "s";
#else
VrmlBrowserCreateVrmlFromString(JSContext *context, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(context,vp);
        jsval *argv = JS_ARGV(context,vp);
	jsval _my_rval;
	jsval *rval = &_my_rval;
	char *_c_format = "S";
	JSString *js_c;
#endif
	char *_c, *_c_args = "SFString vrmlSyntax";

	/* for the return of the nodes */
	struct X3D_Group *retGroup;
	char *xstr; 
	char *tmpstr;
	char *separator;
	int ra;
	int count;
	int wantedsize;
	int MallocdSize;
	ttglobal tg = gglobal();
	struct VRMLParser *globalParser = (struct VRMLParser *)tg->CParse.globalParser;
	

	/* make this a default value */
	*rval = INT_TO_JSVAL(0);

	if (argc == 1 &&
#if JS_VERSION < 185
		JS_ConvertArguments(context, argc, argv, _c_format, &_c)) {
#else
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
			_c = JS_EncodeString(context,js_c);
#endif
		#ifdef JSVERBOSE
			printf("VrmlBrowserCreateVrmlFromString: obj = %u, str = \"%s\"\n",
				   obj, _c);
		#endif

		/* do the call to make the VRML code  - create a new browser just for this string */
		gglobal()->ProdCon.savedParser = (void *)globalParser; globalParser = NULL;
		retGroup = createNewX3DNode(NODE_Group);
		ra = EAI_CreateVrml("String",_c,retGroup);
		globalParser = (struct VRMLParser*)gglobal()->ProdCon.savedParser; /* restore it */


		/* and, make a string that we can use to create the javascript object */
		MallocdSize = 200;
		xstr = MALLOC (char *, MallocdSize);
		strcpy (xstr,"new MFNode(");
		separator = " ";
		for (count=0; count<retGroup->children.n; count ++) {
			tmpstr = MALLOC(char *, strlen(_c) + 100);
			sprintf (tmpstr,"%s new SFNode('%s','%p')",separator, _c, (void*) retGroup->children.p[count]);
			wantedsize = (int) (strlen(tmpstr) + strlen(xstr));
			if (wantedsize > MallocdSize) {
				MallocdSize = wantedsize +200;
				xstr = REALLOC (xstr,MallocdSize);
			}
			
			
			strncat (xstr,tmpstr,strlen(tmpstr));
			FREE_IF_NZ (tmpstr);
			separator = ", ";
		}
		strcat (xstr,")");
		markForDispose(X3D_NODE(retGroup),FALSE);

#if JS_VERSION >= 185
		JS_free(context,_c);
#endif
		
		#ifdef JSVERBOSE
		printf ("running runscript on :%s:\n",xstr);
		#endif

		/* create this value NOTE: rval is set here. */
		jsrrunScript(context, obj, xstr, rval);
		FREE_IF_NZ (xstr);

	} else {
		printf("\nIncorrect argument format for createVrmlFromString(%s).\n", _c_args);
		return JS_FALSE;
	}

	/* save this value, in case we need it */
#if JS_VERSION < 185
	tg->jsVRMLBrowser.JSCreate_global_return_val = *rval;
#else
	JS_SET_RVAL(context,vp,*rval);
#endif
	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
VrmlBrowserCreateVrmlFromURL(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
VrmlBrowserCreateVrmlFromURL(JSContext *context, uintN argc, jsval *vp) {
        jsval *argv = JS_ARGV(context,vp);
	jsval _my_rval;
	jsval *rval = &_my_rval;
#endif
	JSString *_str[2];
	JSClass *_cls[2];
	SFNodeNative *oldPtr;
	char *fieldStr,
		*_costr0;
	struct X3D_Node *myptr;
	#define myFileSizeLimit 4000

/* DJ Tue May  4 21:25:15 BST 2010 Old stuff, no longer applicable
	int count;
	int offset;
	int fromtype;
	int xxx;
	int myField;
	char *address;
	struct X3D_Group *subtree;
*/
	resource_item_t *res = NULL;
	int fieldInt;
	int offs;
	int type;
	int accessType;
	struct Multi_String url;


	#ifdef JSVERBOSE
	printf ("JS start of createVrmlFromURL\n");
	#endif

	/* rval is always zero, so lets just set it */
#if JS_VERSION < 185
	*rval = INT_TO_JSVAL(0);
#else
	*rval = JSVAL_ZERO;
#endif

	/* first parameter - expect a MFString Object here */
	if (JSVAL_IS_OBJECT(argv[0])) {
		if ((_cls[0] = JS_GET_CLASS(context, JSVAL_TO_OBJECT(argv[0]))) == NULL) {
                        printf( "JS_GetClass failed for arg 0 in VrmlBrowserLoadURL.\n");
                        return JS_FALSE;
                }
	} else {
		printf ("VrmlBrowserCreateVrmlFromURL - expect first parameter to be an object\n");
		return JS_FALSE;
	}

	/* second parameter - expect a SFNode Object here */
	if (JSVAL_IS_OBJECT(argv[1])) {
		if ((_cls[1] = JS_GET_CLASS(context, JSVAL_TO_OBJECT(argv[1]))) == NULL) {
                        printf( "JS_GetClass failed for arg 1 in VrmlBrowserLoadURL.\n");
                        return JS_FALSE;
                }
	} else {
		printf ("VrmlBrowserCreateVrmlFromURL - expect first parameter to be an object\n");
		return JS_FALSE;
	}

	#ifdef JSVERBOSE
	printf ("JS createVrml - step 2\n");
	printf ("JS create - we should havve a MFString and SFNode, have :%s: :%s:\n",(_cls[0])->name, (_cls[1])->name);
	#endif

	/* make sure these 2 objects are really MFString and SFNode */
	if (memcmp("MFString", (_cls[0])->name, strlen((_cls[0])->name)) != 0 &&
		memcmp("SFNode", (_cls[1])->name, strlen((_cls[1])->name)) != 0) {
		printf( "Incorrect arguments in VrmlBrowserLoadURL.\n");
		return JS_FALSE;
	}

	/* third parameter should be a string */
	if (JSVAL_IS_STRING(argv[2])) {
		_str[1] = JSVAL_TO_STRING(argv[2]);
#if JS_VERSION < 185
		fieldStr = JS_GetStringBytes(_str[1]);
#else
		fieldStr = JS_EncodeString(context,_str[1]);
#endif
		#ifdef JSVERBOSE
		printf ("field string is :%s:\n",fieldStr); 
		#endif
	 } else {
		printf ("Expected a string in createVrmlFromURL\n");
		return JS_FALSE;
	}

	#ifdef JSVERBOSE
	printf ("passed object type tests\n");
	#endif

	/* get the URL listing as a string */
	_str[0] = JS_ValueToString(context, argv[0]);
#if JS_VERSION < 185
	_costr0 = JS_GetStringBytes(_str[0]);
#else
	_costr0 = JS_EncodeString(context,_str[0]);
#endif


	#ifdef JSVERBOSE
	printf ("URL string is %s\n",_costr0);
	#endif


	/* get a pointer to the SFNode structure, in order to properly place the new string */
	if ((oldPtr = (SFNodeNative *)JS_GetPrivate(context, JSVAL_TO_OBJECT(argv[1]))) == NULL) {
		printf( "JS_GetPrivate failed in VrmlBrowserLoadURL for SFNode parameter.\n");
#if JS_VERSION >= 185
		JS_free(context,_costr0);
		JS_free(context,fieldStr);
#endif
		return JS_FALSE;
	}
	myptr = X3D_NODE(oldPtr->handle);
	if (myptr == NULL) {
		printf ("CreateVrmlFromURL, internal error - SFNodeNative memory pointer is NULL\n");
#if JS_VERSION >= 185
		JS_free(context,_costr0);
		JS_free(context,fieldStr);
#endif
		return JS_FALSE;
	}


	#ifdef JSVERBOSE
	printf ("SFNode handle %d, old X3DString %s\n",oldPtr->handle, oldPtr->X3DString);
	printf ("myptr %d\n",myptr);
	printf ("points to a %s\n",stringNodeType(myptr->_nodeType));
	#endif


	/* bounds checks */
	if (sizeof (_costr0) > (myFileSizeLimit-200)) {
		printf ("VrmlBrowserCreateVrmlFromURL, url too long...\n");
#if JS_VERSION >= 185
		JS_free(context,_costr0);
		JS_free(context,fieldStr);
#endif
		return JS_FALSE;
	}

	/* ok - here we have:
		_costr0		: the url string array; eg: [ "vrml.wrl" ]
		opldPtr		: pointer to a SFNode, with oldPtr->handle as C memory location. 
		fielsStr	: the field to send this to, eg: addChildren
	*/
	
	url.n = 0;
	url.p = NULL;
		
	/* parse the string, put it into the "url" struct defined here */
	Parser_scanStringValueToMem(X3D_NODE(&url),0,FIELDTYPE_MFString, _costr0, FALSE);

	/* find a file name that exists. If not, return JS_FALSE */
	res = resource_create_multi(&url);
	res->where = myptr;


	/* lets see if this node has a routed field  fromTo  = 0 = from node, anything else = to node */
	fieldInt = findRoutedFieldInFIELDNAMES (myptr, fieldStr, TRUE);

	if (fieldInt >=0) { 
		findFieldInOFFSETS(myptr->_nodeType, fieldInt, &offs, &type, &accessType);
	} else {
		ConsoleMessage ("Can not find field :%s: in nodeType :%s:",fieldStr,stringNodeType(myptr->_nodeType));
#if JS_VERSION >= 185
		JS_free(context,_costr0);
		JS_free(context,fieldStr);
#endif
		return JS_FALSE;
	}

	/* printf ("type of field %s, accessType %s\n",stringFieldtypeType(type),stringKeywordType(accessType)); */
	res->offsetFromWhere = offs;

	send_resource_to_parser(res,__FILE__,__LINE__);
	resource_wait(res);
	
	if (res->status == ress_parsed) {
		/* Cool :) */
	}

	MARK_EVENT(myptr,offs);
#if JS_VERSION >= 185
	JS_SET_RVAL(context,vp,*rval);
	JS_free(context,fieldStr);
	JS_free(context,_costr0);
#endif
	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
VrmlBrowserAddRoute(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	jsval _rval = INT_TO_JSVAL(0);
#else
VrmlBrowserAddRoute(JSContext *context, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(context,vp);
        jsval *argv = JS_ARGV(context,vp);
#endif
	if (!doVRMLRoute(context, obj, argc, argv, "addRoute")) {
		printf( "doVRMLRoute failed in VrmlBrowserAddRoute.\n");
		return JS_FALSE;
	}
#if JS_VERSION < 185
	*rval = _rval;
#else
	JS_SET_RVAL(context,vp,JSVAL_ZERO);
#endif
	return JS_TRUE;
}
//#define OLD_CONSOLEMESSAGE_VERSION 1
#ifdef OLD_CONSOLEMESSAGE_VERSION
#define BrowserPrintConsoleMessage ConsoleMessage
#endif
JSBool
#if JS_VERSION < 185
VrmlBrowserPrint(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	jsval _rval = INT_TO_JSVAL(0);
#else
VrmlBrowserPrint(JSContext *context, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(context,vp);
        jsval *argv = JS_ARGV(context,vp);
#endif
	int count;
	JSString *_str;
	char *_id_c;

	UNUSED (context); UNUSED(obj);
	/* printf ("FreeWRL:javascript: "); */
	for (count=0; count < argc; count++) {
		if (JSVAL_IS_STRING(argv[count])) {
			_str = JSVAL_TO_STRING(argv[count]);
#if JS_VERSION < 185
			_id_c = JS_GetStringBytes(_str);
#else
			_id_c = JS_EncodeString(context,_str);
#endif
			#if defined(AQUA) || defined(_MSC_VER)
			BrowserPrintConsoleMessage(_id_c); /* statusbar hud */
			gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
			#else
				#ifdef HAVE_NOTOOLKIT 
					printf ("%s", _id_c);
				#else
					printf ("%s\n", _id_c);
					BrowserPrintConsoleMessage(_id_c); /* statusbar hud */
					gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
				#endif
			#endif
#if JS_VERSION >= 185
			JS_free(context,_id_c);
#endif
		} else {
	/*		printf ("unknown arg type %d\n",count); */
		}
	}
	/* the \n should be done with println below, or in javascript print("\n"); */
	#if defined(AQUA) 
	BrowserPrintConsoleMessage("\n"); /* statusbar hud */
	gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
	#elif !defined(_MSC_VER)
		#ifdef HAVE_NOTOOLKIT
			printf ("\n");
		#endif
	#endif
#if JS_VERSION < 185
	*rval = _rval;
#else
	JS_SET_RVAL(context,vp,JSVAL_ZERO);
#endif
	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
VrmlBrowserPrintln(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {	
    VrmlBrowserPrint(context,obj,argc,argv,rval);
#else
VrmlBrowserPrintln(JSContext *context, uintN argc, jsval *vp) {
	/* note, vp holds rval, since it is set in here we should be good */
	VrmlBrowserPrint(context,argc,vp); 
#endif
	#if defined(AQUA) || defined(_MSC_VER)
		BrowserPrintConsoleMessage("\n"); /* statusbar hud */
		gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
	#else
		#ifdef HAVE_NOTOOLKIT
			printf ("\n");
		#endif
	#endif
	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
VrmlBrowserDeleteRoute(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	jsval _rval = INT_TO_JSVAL(0);
#else
VrmlBrowserDeleteRoute(JSContext *context, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(context,vp);
        jsval *argv = JS_ARGV(context,vp);
#endif
	if (!doVRMLRoute(context, obj, argc, argv, "deleteRoute")) {
		printf( "doVRMLRoute failed in VrmlBrowserDeleteRoute.\n");
		return JS_FALSE;
	}
#if JS_VERSION < 185
	*rval = _rval;
#else
	JS_SET_RVAL(context,vp,JSVAL_ZERO);
#endif
	return JS_TRUE;
}

/****************************************************************************************/


/****************************************************************************************************/

/* internal to add/remove a ROUTE */
static JSBool doVRMLRoute(JSContext *context, JSObject *obj, uintN argc, jsval *argv, const char *callingFunc) {
	JSObject *fromNodeObj, *toNodeObj;
	SFNodeNative *fromNative, *toNative;
	JSClass *_cls[2];
	char 
		*fromFieldString, *toFieldString,
		*_c_args =
		"SFNode fromNode, SFString fromEventOut, SFNode toNode, SFString toEventIn",
#if JS_VERSION < 185
		*_c_format = "o s o s";
#else
		*_c_format = "oSoS";
	JSString *fromFieldStringJS, *toFieldStringJS;
#endif
	struct X3D_Node *fromNode;
	struct X3D_Node *toNode;
	int fromOfs, toOfs, len;
	int fromtype, totype;
	int xxx;
	int myField;

	/* first, are there 4 arguments? */
	if (argc != 4) {
		printf ("Problem with script - add/delete route command needs 4 parameters\n");
		return JS_FALSE;
	}

	/* get the arguments, and ensure that they are obj, string, obj, string */
	if (JS_ConvertArguments(context, argc, argv, _c_format,
#if JS_VERSION < 185
				&fromNodeObj, &fromFieldString, &toNodeObj, &toFieldString)) {
#else
				&fromNodeObj, &fromFieldStringJS, &toNodeObj, &toFieldStringJS)) {
		fromFieldString = JS_EncodeString(context,fromFieldStringJS);
		toFieldString = JS_EncodeString(context,toFieldStringJS);
#endif
		if ((_cls[0] = JS_GET_CLASS(context, fromNodeObj)) == NULL) {
			printf("JS_GetClass failed for arg 0 in doVRMLRoute called from %s.\n",
					callingFunc);
			return JS_FALSE;
		}
		if ((_cls[1] = JS_GET_CLASS(context, toNodeObj)) == NULL) {
			printf("JS_GetClass failed for arg 2 in doVRMLRoute called from %s.\n",
					callingFunc);
			return JS_FALSE;
		}

		/* make sure these are both SFNodes */
		if (memcmp("SFNode", (_cls[0])->name, strlen((_cls[0])->name)) != 0 &&
			memcmp("SFNode", (_cls[1])->name, strlen((_cls[1])->name)) != 0) {
			printf("\nArguments 0 and 2 must be SFNode in doVRMLRoute called from %s(%s): %s\n",
					callingFunc, _c_args, callingFunc);
			return JS_FALSE;
		}

		/* get the "private" data for these nodes. It will consist of a SFNodeNative structure */
		if ((fromNative = (SFNodeNative *)JS_GetPrivate(context, fromNodeObj)) == NULL) {
			printf ("problem getting native props\n");
			return JS_FALSE;
		}
		if ((toNative = (SFNodeNative *)JS_GetPrivate(context, toNodeObj)) == NULL) {
			printf ("problem getting native props\n");
			return JS_FALSE;
		}
		/* get the "handle" for the actual memory pointer */
		fromNode = X3D_NODE(fromNative->handle);
		toNode = X3D_NODE(toNative->handle);

		#ifdef JSVERBOSE
		printf ("routing from a node of type %s to a node of type %s\n",
			stringNodeType(fromNode->_nodeType), 
			stringNodeType(toNode->_nodeType));
		#endif	

		/* From field */
		/* try finding it, maybe with a "set_" or "changed" removed */
		myField = findRoutedFieldInFIELDNAMES(fromNode,fromFieldString,0);
		if (myField == -1) 
			myField = findRoutedFieldInFIELDNAMES(fromNode,fromFieldString,1);

		/* find offsets, etc */
       		findFieldInOFFSETS(fromNode->_nodeType, myField, &fromOfs, &fromtype, &xxx);

		/* To field */
		/* try finding it, maybe with a "set_" or "changed" removed */
		myField = findRoutedFieldInFIELDNAMES(toNode,toFieldString,0);
		if (myField == -1) 
			myField = findRoutedFieldInFIELDNAMES(toNode,toFieldString,1);

		/* find offsets, etc */
       		findFieldInOFFSETS(toNode->_nodeType, myField, &toOfs, &totype, &xxx);

		/* do we have a mismatch here? */
		if (fromtype != totype) {
			printf ("Javascript routing problem - can not route from %s to %s\n",
				stringNodeType(fromNode->_nodeType), 
				stringNodeType(toNode->_nodeType));
			return JS_FALSE;
		}

		len = returnRoutingElementLength(totype);

		jsRegisterRoute(fromNode, fromOfs, toNode, toOfs, len,callingFunc);

#if JS_VERSION >= 185
		JS_free(context,fromFieldString);
		JS_free(context,toFieldString);
#endif
	} else {
		printf( "\nIncorrect argument format for %s(%s).\n",
				callingFunc, _c_args);
		return JS_FALSE;
	}

	return JS_TRUE;
}
#endif /* HAVE_JAVASCRIPT */
