/*
=INSERT_TEMPLATE_HERE=

$Id: jsVRMLBrowser.c,v 1.19 2009/10/26 10:55:13 couannette Exp $

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
#include "../main/headers.h"
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

#include "CScripts.h"
#include "fieldSet.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"
#include "jsVRMLBrowser.h"


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
	{"println", VrmlBrowserPrint, 0},
	{"getMidiDeviceList", VrmlBrowserGetMidiDeviceList, 0},
	{"getMidiDeviceInfo", VrmlBrowserGetMidiDeviceInfo, 0},
	{0}
};


/* make up a new parser for parsing from createVrmlFromURL and createVrmlFromString */
struct VRMLParser* savedParser;



/* for setting field values to the output of a CreateVrml style of call */
/* it is kept at zero, unless it has been used. Then it is reset to zero */
jsval JSCreate_global_return_val = INT_TO_JSVAL(0);

/* we add/remove routes with this call */
void jsRegisterRoute(
	struct X3D_Node* from, int fromOfs,
	struct X3D_Node* to, int toOfs,
	int len, const char *adrem) {
 	char tonode_str[15];
	int ad;
#if defined(_MSC_VER)
 	sprintf_s(tonode_str, 15, "%p:%d", to, toOfs);
#else
 	snprintf(tonode_str, 15, "%p:%d", to, toOfs);
#endif

	if (strcmp("addRoute",adrem) == 0) 
		ad = 1;
	else ad = 0;

 	CRoutes_Register(ad, from, fromOfs, 1, tonode_str, len, 
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
VrmlBrowserGetName(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSString *_str;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	_str = JS_NewStringCopyZ(context,BrowserName);
	*rval = STRING_TO_JSVAL(_str);
	return JS_TRUE;
}


/* get the string stored in FWVER into a jsObject */
JSBool
VrmlBrowserGetVersion(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSString *_str;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	_str = JS_NewStringCopyZ(context, libFreeWRL_get_version());
	*rval = STRING_TO_JSVAL(_str);
	return JS_TRUE;
}


JSBool
VrmlBrowserGetCurrentSpeed(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSString *_str;
	char string[1000];

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	/* get the variable updated */
	getCurrentSpeed();
	sprintf (string,"%f",BrowserSpeed);
	_str = JS_NewStringCopyZ(context,string);
	*rval = STRING_TO_JSVAL(_str);
	return JS_TRUE;
}


JSBool
VrmlBrowserGetCurrentFrameRate(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSString *_str;
	char FPSstring[1000];

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	sprintf (FPSstring,"%6.2f",BrowserFPS);
	_str = JS_NewStringCopyZ(context,FPSstring);
	*rval = STRING_TO_JSVAL(_str);
	return JS_TRUE;
}


JSBool
VrmlBrowserGetWorldURL(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSString *_str;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	_str = JS_NewStringCopyZ(context,BrowserFullPath);
	*rval = STRING_TO_JSVAL(_str);
	return JS_TRUE;
}


JSBool
VrmlBrowserReplaceWorld(JSContext *context, JSObject *obj,
						uintN argc, jsval *argv, jsval *rval)
{
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
		_costr = JS_GetStringBytes(_str);

		/* sanitize string, for the EAI_RW call (see EAI_RW code) */
		tptr = _costr;
		while (*tptr != '\0') {
			if(*tptr == '[') *tptr = ' ';
			if(*tptr == ']') *tptr = ' ';
			if(*tptr == ',') *tptr = ' ';
			tptr++;
		}
		EAI_RW(_costr);

	} else {
		printf( "\nIncorrect argument format for replaceWorld(%s).\n", _c_args);
		return JS_FALSE;
	}
	*rval = _rval;

	return JS_TRUE;
}

JSBool
VrmlBrowserLoadURL(JSContext *context, JSObject *obj,
				   uintN argc, jsval *argv, jsval *rval)
{
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
		_costr[0] = JS_GetStringBytes(_str[0]);

		_str[1] = JS_ValueToString(context, argv[1]);
		_costr[1] = JS_GetStringBytes(_str[1]);

		/* we use the EAI code for this - so reformat this for the EAI format */
		{
			extern struct X3D_Anchor EAI_AnchorNode;  /* win32 C doesnt like new declarations in the middle of executables - start a new scope {} and put dec at top */

			/* make up the URL from what we currently know */
			createLoadUrlString(myBuf,myBufSize,_costr[0], _costr[1]);
			createLoadURL(myBuf);

			/* now tell the EventLoop that BrowserAction is requested... */
			AnchorsAnchor = &EAI_AnchorNode;
		}
		BrowserAction = TRUE;


	} else {
		printf( "\nIncorrect argument format for loadURL(%s).\n", _c_args);
		return JS_FALSE;
	}
	*rval = INT_TO_JSVAL(0);

	return JS_TRUE;
}


JSBool
VrmlBrowserSetDescription(JSContext *context, JSObject *obj,
						  uintN argc, jsval *argv, jsval *rval)
{
	char *_c, *_c_args = "SFString description", *_c_format = "s";

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &_c)) {

		/* we do not do anything with the description. If we ever wanted to, it is in _c */
		*rval = INT_TO_JSVAL(0);
	} else {
		printf( "\nIncorrect argument format for setDescription(%s).\n", _c_args);
		return JS_FALSE;
	}
	return JS_TRUE;
}


JSBool
VrmlBrowserCreateVrmlFromString(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	char *_c, *_c_args = "SFString vrmlSyntax", *_c_format = "s";

	/* for the return of the nodes */
	uintptr_t nodarr[200];
	char *xstr; 
	char *tmpstr;
	int ra;
	int count;
	int wantedsize;
	int MallocdSize;
	

	/* make this a default value */
	*rval = INT_TO_JSVAL(0);

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &_c)) {
		#ifdef JSVERBOSE
			printf("VrmlBrowserCreateVrmlFromString: obj = %u, str = \"%s\"\n",
				   obj, _c);
		#endif

		/* do the call to make the VRML code  - create a new browser just for this string */
		savedParser = globalParser; globalParser = NULL;
		ra = EAI_CreateVrml("String",_c,nodarr,200);
		globalParser = savedParser; /* restore it */


		#ifdef JSVERBOSE
		printf ("EAI_CreateVrml returns %d nodes\n",ra);
		printf ("nodes %d %d\n",nodarr[0],nodarr[1]);
		#endif

		/* and, make a string that we can use to create the javascript object */
		MallocdSize = 200;
		xstr = MALLOC (MallocdSize);
		strcpy (xstr,"new MFNode(");
		for (count=0; count<ra; count += 2) {
			tmpstr = MALLOC(strlen(_c) + 100);
			sprintf (tmpstr,"new SFNode('%s','%p')",_c, (void*) nodarr[count*2+1]);
			wantedsize = strlen(tmpstr) + strlen(xstr);
			if (wantedsize > MallocdSize) {
				MallocdSize = wantedsize +200;
				xstr = REALLOC (xstr,MallocdSize);
			}
			
			
			strncat (xstr,tmpstr,strlen(tmpstr));
			FREE_IF_NZ (tmpstr);
		}
		strcat (xstr,")");
		
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
	JSCreate_global_return_val = *rval;
	return JS_TRUE;
}

JSBool
VrmlBrowserCreateVrmlFromURL(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	JSString *_str[2];
	JSClass *_cls[2];
	SFNodeNative *oldPtr;
	char *fieldStr,
		*_costr0;
	uintptr_t nodarr[200];
	struct X3D_Node *myptr;
	int ra;
	#define myFileSizeLimit 4000
/* 	char filename[myFileSizeLimit]; */
/* 	char tfilename [myFileSizeLimit]; */
	char *tfptr; 
	char *coptr;
	char *bfp;
	int found;
	int count;
	int offset;
	int fromtype;
	int xxx;
	int myField;
	char *address;

	resource_item_t *res = NULL;

	#ifdef JSVERBOSE
	printf ("JS start of createVrmlFromURL\n");
	#endif

	/* rval is always zero, so lets just set it */
	*rval = INT_TO_JSVAL(0);

	/* first parameter - expect a MFString Object here */
	if (JSVAL_IS_OBJECT(argv[0])) {
		if ((_cls[0] = JS_GET_CLASS(context, (JSObject *)argv[0])) == NULL) {
                        printf( "JS_GetClass failed for arg 0 in VrmlBrowserLoadURL.\n");
                        return JS_FALSE;
                }
	} else {
		printf ("VrmlBrowserCreateVrmlFromURL - expect first parameter to be an object\n");
		return JS_FALSE;
	}

	/* second parameter - expect a SFNode Object here */
	if (JSVAL_IS_OBJECT(argv[1])) {
		if ((_cls[1] = JS_GET_CLASS(context, (JSObject *)argv[1])) == NULL) {
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
		fieldStr = JS_GetStringBytes(_str[1]);
		#ifdef JSVERBOSE
		printf ("field string is %s\n",fieldStr); 
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
	_costr0 = JS_GetStringBytes(_str[0]);


	#ifdef JSVERBOSE
	printf ("URL string is %s\n",_costr0);
	#endif


	/* get a pointer to the SFNode structure, in order to properly place the new string */
	if ((oldPtr = (SFNodeNative *)JS_GetPrivate(context, (JSObject *)argv[1])) == NULL) {
		printf( "JS_GetPrivate failed in VrmlBrowserLoadURL for SFNode parameter.\n");
		return JS_FALSE;
	}
	myptr = X3D_NODE(oldPtr->handle);
	if (myptr == NULL) {
		printf ("CreateVrmlFromURL, internal error - SFNodeNative memory pointer is NULL\n");
		return JS_FALSE;
	}


	#ifdef JSVERBOSE
	printf ("SFNode handle %d, old X3DString %s\n",oldPtr->handle, oldPtr->X3DString);
	printf ("myptr %d\n",myptr);
	printf ("points to a %s\n",stringNodeType(myptr->_nodeType));
	#endif


	/* bounds checks */
	if (sizeof (_costr0) > (myFileSizeLimit-200)) {
		printf ("VrmlBrowserCreateVrmlFromURL, url too long...\n"); return JS_FALSE;
	}

	/* ok - here we have:
		_costr0		: the url string array; eg: [ "vrml.wrl" ]
		opldPtr		: pointer to a SFNode, with oldPtr->handle as C memory location. 
		fielsStr	: the field to send this to, eg: addChildren
	*/

	/* find a file name that exists. If not, return JS_FALSE */
/* 	bfp = STRDUP(BrowserFullPath); */
	/* and strip off the file name, leaving any path */
/* 	removeFilenameFromPath (bfp); */

/* 	#ifdef JSVERBOSE */
/* 	printf ("have path now, of :%s:\n",bfp); */
/* 	#endif */

	res = resource_create_single(_costr0);
	res->where = myptr;
	send_resource_to_parser(res);
	resource_wait(res);
	
	if (res->status == ress_parsed) {
		/* Cool :) */
	}

#if 0
	/* go through the elements and find which (if any) url exists */	
	found = FALSE;
	coptr = _costr0;

	while (!found) {
		tfptr = tfilename;

		#ifdef JSVERBOSE
		printf ("start of loop, coptr :%s:\n",coptr);
		#endif

		if (*coptr == '[') coptr++;
		while ((*coptr != '\0') && (*coptr == ' ')) coptr++;
		if (*coptr == '\0') {
			ConsoleMessage ("javascript: could not find a valid url in %s",_costr0);
			return JS_FALSE;
		}

		if (*coptr == '"') {
			coptr++;
			/* printf ("have the initial quote string here is %s\n",coptr); */
			while (*coptr != '"') {
				*tfptr = *coptr;
				tfptr++; coptr++;
			}
			*tfptr = '\0';
			#ifdef JSVERBOSE
			printf ("found string is :%s:\n",tfilename);
			#endif
		}
        	 
		/* we work in absolute filenames... */
		makeAbsoluteFileName(filename,bfp,tfilename);
		
		if (fileExists(filename,NULL,TRUE)) {
			/* printf ("file exists, break\n"); */
			found = TRUE;
		}
#ifdef JSVERBOSE
		ERROR_MSG("nope, file %s does not exist\n", tfilename);
#endif

		/* skip along to the start of the next name */
		if (*coptr == '"') coptr++;
		if (*coptr == ',') coptr++;
		if (*coptr == ']') coptr++; /* this allows us to error out, above */
	}

	/* call the parser */
	/* "save" the old classic parser state, so that names do not cross-pollute */
	savedParser = globalParser;
	globalParser = NULL;
	ra = EAI_CreateVrml("URL",filename,nodarr,200);
	globalParser = savedParser;
#endif 

	/* get the field from the beginning of this node as an offset */
	/* try finding it, maybe with a "set_" or "changed" removed */
	myField = findRoutedFieldInFIELDNAMES(myptr,fieldStr,0);
	if (myField == -1) 
		myField = findRoutedFieldInFIELDNAMES(myptr,fieldStr,1);

	/* is this a valid X3D field? */
	if (myField == -1) {
		printf ("createVrmlFromURL - field %s is not a valid field\n",fieldStr);
		return JS_FALSE;
	}

	/* find offsets, etc */
       	findFieldInOFFSETS(myptr->_nodeType, myField, &offset, &fromtype, &xxx);
	if (offset == -1) {
		printf ("createVrmlFromURL - field %s is not a valid field of a node of type %s\n",fieldStr,stringNodeType(myptr->_nodeType));
		return JS_FALSE;
	}

	/* did we find the field? */
	address = ((char *) myptr) + offset;
	
	struct X3D_Group *subtree = (struct X3D_Group *) myptr;
	
	/* MBFILES: process the children of myptr loaded by parser */
	for (count = 1; count < subtree->children.n; count++) {
		static char dtmp[100];
		sprintf(dtmp, "%lu", subtree->children.p[count]);
		getMFNodetype(dtmp, (struct Multi_Node *) address, myptr, 1);
	}

#if 0
	/* now go through, and add the nodes to the parent node */
	for (count = 1; count < ra; count +=2) {
		char dtmp[100];
		/* ensure that this node is not NULL */
		if (nodarr[count] != 0) {
			sprintf (dtmp,"%lu", (void*) nodarr[count]);
			/* so, we send in the new node encoded as a string, 
			   the actual pointer in memory of the MFNode field,
			   the node pointer containing this field, 
			   we ALWAYS add this to the field, even if it is a "removeChildren"
			   and we let the scene graph determine whether it is an add or remove */
			getMFNodetype (dtmp,(struct Multi_Node *)address, myptr, 1);
		}
	}
#endif

	MARK_EVENT(myptr,offset);
	return JS_TRUE;
}

JSBool
VrmlBrowserAddRoute(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsval _rval = INT_TO_JSVAL(0);
	if (!doVRMLRoute(context, obj, argc, argv, "addRoute")) {
		printf( "doVRMLRoute failed in VrmlBrowserAddRoute.\n");
		return JS_FALSE;
	}
	*rval = _rval;
	return JS_TRUE;
}


JSBool
VrmlBrowserPrint(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{	int count;
	JSString *_str;
	char *_id_c;
	jsval _rval = INT_TO_JSVAL(0);

	UNUSED (context); UNUSED(obj);
	/* printf ("FreeWRL:javascript: "); */
	for (count=0; count < argc; count++) {
		if (JSVAL_IS_STRING(argv[count])) {
			_str = JSVAL_TO_STRING(argv[count]);
			_id_c = JS_GetStringBytes(_str);
			#ifdef AQUA
			ConsoleMessage(_id_c);
			consMsgCount = 0; /* reset the "Maximum" count */
			#else
				#ifdef HAVE_NOTOOLKIT 
					printf ("%s", _id_c);
				#else
					printf ("%s\n", _id_c);
					ConsoleMessage(_id_c);
					consMsgCount = 0; /* reset the "Maximum" count */
				#endif
			#endif
		} else {
	/*		printf ("unknown arg type %d\n",count); */
		}
	}
	#ifdef AQUA
	ConsoleMessage("\n");
	consMsgCount = 0; /* reset the "Maximum" count */
	#else
		#ifdef HAVE_NOTOOLKIT
			printf ("\n");
		#endif
	#endif
	*rval = _rval;
	return JS_TRUE;
}
JSBool
VrmlBrowserDeleteRoute(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsval _rval = INT_TO_JSVAL(0);
	if (!doVRMLRoute(context, obj, argc, argv, "deleteRoute")) {
		printf( "doVRMLRoute failed in VrmlBrowserDeleteRoute.\n");
		return JS_FALSE;
	}
	*rval = _rval;
	return JS_TRUE;
}

/****************************************************************************************/

/* find a name in the ReWireNameTable - this is a read-only operation; 
   int ReWireNameIndex (char *name) will find it, and add it if it is not there */

static int findEncodedName(char *target) {
	int encodedName;
	int ctr;

	encodedName = -1;
	#ifdef JSVERBOSE
	printf ("findEncodedName - looking for %s\n",target);
	#endif

	for (ctr=0; ctr<=ReWireNametableSize; ctr++) {
		if (strcmp(target,ReWireNamenames[ctr].name)==0) {
			#ifdef JSVERBOSE
			printf ("findEncodedName - FOUND IT at %d - it is %s\n",ctr,ReWireNamenames[ctr].name); 
			#endif
			encodedName = ctr;
		}
	}
	return encodedName;
}

/* return an MFString containing all of the devices CURRENTLY defined on the MIDI interface list */
JSBool
VrmlBrowserGetMidiDeviceList(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	int i;
	JSString *_str;
	JSObject *myObj;
	int currentDevice = -1;
	int deviceIndexInList = 0;

	if (argc != 0) {
		printf ("getMidiDeviceList does not take parameters\n");
		return JS_FALSE;
	}

	#ifdef JSVERBOSE
	printf ("VrmlBrowserGetMidiDeviceList - table size %d\n",ReWireDevicetableSize);
	for (i=0; i<ReWireDevicetableSize; i++) {
		printf ("entry %d is name %d :%s: ecname %d :%s:\n",i,
			ReWireDevices[i].encodedDeviceName, 
			ReWireNamenames[ReWireDevices[i].encodedDeviceName].name, 
			ReWireDevices[i].encodedControllerName,
			ReWireNamenames[ReWireDevices[i].encodedControllerName].name);
	}
	#endif

	/* construct the return object */
        if ((myObj = JS_ConstructObject(context, &MFStringClass, NULL, NULL)) == NULL) {
                printf( "JS_ConstructObject failed in VrmlBrowserGetMidiDeviceList.\n");
                return JS_FALSE;
        }

	/* go through the table, and find encoded names that are unique */
	for (i=0; i<ReWireDevicetableSize; i++) {
		/* this is a different device than before */
		if (ReWireDevices[i].encodedDeviceName != currentDevice) {
			currentDevice = ReWireDevices[i].encodedDeviceName;
			#ifdef JSVERBOSE
			printf ("getMidiDeviceList: device %d is %s\n",deviceIndexInList,ReWireNamenames[currentDevice].name);
			#endif

        		_str = JS_NewStringCopyZ(context,ReWireNamenames[currentDevice].name);
                	if (!JS_DefineElement(context, myObj, (jsint) deviceIndexInList, STRING_TO_JSVAL(_str),
				JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB7, JSPROP_ENUMERATE)) {
                	        printf( "JS_DefineElement failed for arg %d in getMidiDeviceList.\n", i);
                	        return JS_FALSE;
			}
			deviceIndexInList ++; /* next entry */
                }
        }

        *rval = OBJECT_TO_JSVAL(myObj);
        return JS_TRUE;
}

/* find a MIDI device, (parameter input is a String) and return MFString of controller names */
/* returns a MFString with 0 entries, if no controller is found */
JSBool
VrmlBrowserGetMidiDeviceInfo(JSContext *context, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSString *_str;
	JSObject *myObj;
	char *target;
	int encodedName;
	int currentController;
	int i;
	int controllerIndexInList = 0;
	int dummyController;

	#ifdef JSVERBOSE
	printf ("start of VrmlBrowserGetMidiDeviceInfo\n");
	#endif

	if (argc != 1) {
		printf ("getMidiDeviceInfo expects 1 parameter\n");
		return JS_FALSE;
	}

	/* parameter should be a string */
	if (JSVAL_IS_STRING(argv[0])) {
		target = JS_GetStringBytes( JSVAL_TO_STRING(argv[0]));
		#ifdef JSVERBOSE
		printf ("field string is %s\n",target); 
		#endif
	} else {
		printf ("getMidiDeviceInfo expects parameter to be a string\n");
		return JS_FALSE;
	}

	/* what is its index? */
	encodedName = findEncodedName(target);

	#ifdef JSVERBOSE
	printf ("VrmlBrowserGetMidiDeviceInfo - table size %d looking for encoded name %d\n",ReWireDevicetableSize,encodedName);
	for (i=0; i<ReWireDevicetableSize; i++) {
		if (encodedName == ReWireDevices[i].encodedDeviceName) 
		printf ("entry %d is name %d :%s: ecname %d :%s:\n",i,
			ReWireDevices[i].encodedDeviceName, 
			ReWireNamenames[ReWireDevices[i].encodedDeviceName].name, 
			ReWireDevices[i].encodedControllerName,
			ReWireNamenames[ReWireDevices[i].encodedControllerName].name);
	}
	#endif

	/* construct the return object */
        if ((myObj = JS_ConstructObject(context, &MFStringClass, NULL, NULL)) == NULL) {
                printf( "JS_ConstructObject failed in VrmlBrowserGetMidiDeviceList.\n");
                return JS_FALSE;
        }
        *rval = OBJECT_TO_JSVAL(myObj);

	/* one controller has been added to ensure that we can send notes to a device, without
	   ANY controllers (table entry must exist, yada, yada, yada... , so skip this one */
	dummyController = findEncodedName("use_for_buttonPresses");

	/* go through the table, and find controllers associated with this device */
	for (i=0; i<ReWireDevicetableSize; i++) {
		/* is this our device? */
		if (encodedName == ReWireDevices[i].encodedDeviceName) {
			/* it is our device, is it anything but this dummy controller? */
			if (ReWireDevices[i].encodedControllerName != dummyController) {
				/* found our device, lets add info on controllers */
				currentController = ReWireDevices[i].encodedControllerName;
				#ifdef JSVERBOSE
				printf ("getMidiDeviceList: controller %d is %s\n",controllerIndexInList,ReWireNamenames[currentController].name);
				#endif
	
	        		_str = JS_NewStringCopyZ(context,ReWireNamenames[currentController].name);
	                	if (!JS_DefineElement(context, myObj, (jsint) controllerIndexInList, STRING_TO_JSVAL(_str),
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB7, JSPROP_ENUMERATE)) {
	                	        printf( "JS_DefineElement failed for arg %d in getMidiDeviceList.\n", i);
	                	        return JS_FALSE;
				}
				controllerIndexInList ++; /* next entry */
			}
                }
        }
        *rval = OBJECT_TO_JSVAL(myObj);
	return JS_TRUE;
}

#define MIDICONNUM 1
#define MIDICONMIN 2
#define MIDICONMAX 3

/* do the guts of the getMidiControllerNumber, ControllerMax and ControllerMin */
int findMidiNumber (JSContext *cx, uintN argc, jsval *argv, int myFn) {
	char *targetDevice;
	char *targetController;
	int encDev; 
	int encCha;
	int i;

	if (argc != 2) {
		printf ("MidiControllerInfo - require 2 parameters\n");
		return -1;
	}

	/* parameters should be a string */
	if (JSVAL_IS_STRING(argv[0])) {
		targetDevice = JS_GetStringBytes( JSVAL_TO_STRING(argv[0]));
		#ifdef JSVERBOSE
		printf ("field string is %s\n",targetDevice); 
		#endif
	} else {
		printf ("getMidiDeviceInfo expects Device parameter to be a string\n");
		return -1;
	}
	if (JSVAL_IS_STRING(argv[1])) {
		targetController = JS_GetStringBytes( JSVAL_TO_STRING(argv[1]));
		#ifdef JSVERBOSE
		printf ("field string is %s\n",targetController); 
		#endif
	} else {
		printf ("getMidiDeviceInfo expects Controller parameter to be a string\n");
		return -1;
	}

	/* ok, we have 2 strings, lets change these to encoded values */
	encDev = findEncodedName(targetDevice);
	encCha = findEncodedName(targetController);

	/* find the entry */
	for (i=0; i<ReWireDevicetableSize; i++) {
		/* is this our device? */
		if (encDev == ReWireDevices[i].encodedDeviceName) {
			/* it is our device, is it anything but this dummy controller? */
			if (ReWireDevices[i].encodedControllerName == encCha) {
				/* found it! */
				#ifdef JSVERBOSE
				printf ("getMidiControllerInfo: found %s %s\n",targetDevice, targetController);
				#endif
				if (myFn == MIDICONNUM)
					return ReWireDevices[i].controller;
				else if (myFn == MIDICONMIN)
					return ReWireDevices[i].cmin;
				else if (myFn == MIDICONMAX)
					return ReWireDevices[i].cmax;
				else {
					printf ("getMidiControllerInfo, found controller, but can't figure out return\n");
					return -1;
				}			
			}
                }
        }
	return -1;
} 

/* send in 2 Strings; MIDI Device and controller, returns number on device, or -1 on error */
JSBool VrmlBrowserGetMidiControllerNumber(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	*rval = INT_TO_JSVAL(findMidiNumber (cx, argc, argv, MIDICONNUM));
	return JS_TRUE;
}

/* send in 2 Strings; MIDI Device and controller, returns minimum value, or -1 on error */
JSBool VrmlBrowserGetMidiControllerMin(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	*rval = INT_TO_JSVAL(findMidiNumber (cx, argc, argv, MIDICONMIN));
	return JS_TRUE;
}

/* send in 2 Strings; MIDI Device and controller, returns maximum value, or -1 on error */
JSBool VrmlBrowserGetMidiControllerMax(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	*rval = INT_TO_JSVAL(findMidiNumber (cx, argc, argv, MIDICONMAX));
	return JS_TRUE;
}



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
		*_c_format = "o s o s";
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
				&fromNodeObj, &fromFieldString, &toNodeObj, &toFieldString)) {
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
	} else {
		printf( "\nIncorrect argument format for %s(%s).\n",
				callingFunc, _c_args);
		return JS_FALSE;
	}

	return JS_TRUE;
}
