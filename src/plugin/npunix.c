/*
 * npunix.c
 *
 * Netscape Client Plugin API
 * - Wrapper function to interface with the Netscape Navigator
 *
 * (c) Copyright 1995, Stan Melax, ALL RIGHTS RESERVED.
 * Copyright (C) 1998 Frank Holtry.  All Rights Reserved.
 * (see the accompanying COPYRIGHT files for details of the copyright
 * terms and conditions).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as Perl itself.
 *
 * dp Suresh <dp@netscape.com>
 *
 * Modified to include the ???URLNotify interfaces.  
 * Frank Holtry <fholtry@lucent.com> 11/98
 *
 *----------------------------------------------------------------------
 * PLUGIN DEVELOPERS:
 *	YOU WILL NOT NEED TO EDIT THIS FILE.
 *----------------------------------------------------------------------
 */

#define XP_UNIX 1

#include <stdio.h>
#include "npapi.h"
#if (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
#include "npupp.h" 
#else
#include "npfunctions.h"
#endif

/*
 * Define PLUGIN_TRACE to have the wrapper functions print
 * messages to stderr whenever they are called.
 */

#if defined(_DEBUG)
# include <stdio.h>
# define PLUGINDEBUGSTR(msg)	fprintf(stderr, "%s\n", msg)
#else
# define PLUGINDEBUGSTR(msg)
#endif


/***********************************************************************
 *
 * Globals
 *
 ***********************************************************************/

static NPNetscapeFuncs   gNetscapeFuncs;	/* Netscape Function table */


/***********************************************************************
 *
 * Wrapper functions : plugin calling Netscape Navigator
 *
 * These functions let the plugin developer just call the APIs
 * as documented and defined in npapi.h, without needing to know
 * about the function table and call macros in npupp.h.
 *
 ***********************************************************************/

void
NPN_Version(int* plugin_major, int* plugin_minor,
	     int* netscape_major, int* netscape_minor)
{
	*plugin_major = NP_VERSION_MAJOR;
	*plugin_minor = NP_VERSION_MINOR;

	/* Major version is in high byte */
	*netscape_major = gNetscapeFuncs.version >> 8;
	/* Minor version is in low byte */
	*netscape_minor = gNetscapeFuncs.version & 0xFF;
}

NPError
NPN_GetValue(NPP instance, NPNVariable variable, void *r_value)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_GetValueProc(gNetscapeFuncs.getvalue,
					instance, variable, r_value);
#else
	return (*gNetscapeFuncs.getvalue)(instance, variable, r_value);
#endif
}

NPError
NPN_GetURL(NPP instance, const char* url, const char* window)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_GetURLProc(gNetscapeFuncs.geturl, instance, url, window);
#else
	return (*gNetscapeFuncs.geturl)(instance, url, window);
#endif
}

NPError
NPN_PostURL(NPP instance, const char* url, const char* window,
	     uint32 len, const char* buf, NPBool file)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_PostURLProc(gNetscapeFuncs.posturl, instance,
					url, window, len, buf, file);
#else
	return (*gNetscapeFuncs.posturl)(instance, url, window, len, buf, file);
#endif
}

NPError
NPN_RequestRead(NPStream* stream, NPByteRange* rangeList)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_RequestReadProc(gNetscapeFuncs.requestread,
					stream, rangeList);
#else
	return (*gNetscapeFuncs.requestread)(stream, rangeList);
#endif
}

NPError
NPN_NewStream(NPP instance, NPMIMEType type, const char *window,
	      NPStream** stream_ptr)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_NewStreamProc(gNetscapeFuncs.newstream, instance,
					type, window, stream_ptr);
#else
	return (*gNetscapeFuncs.newstream)(instance, type, window, stream_ptr);
#endif
}

int32
NPN_Write(NPP instance, NPStream* stream, int32 len, void* buffer)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_WriteProc(gNetscapeFuncs.write, instance,
					stream, len, buffer);
#else
	return (*gNetscapeFuncs.write)(instance, stream, len, buffer);
#endif
}

NPError
NPN_DestroyStream(NPP instance, NPStream* stream, NPError reason)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_DestroyStreamProc(gNetscapeFuncs.destroystream,
						instance, stream, reason);
#else
	return (*gNetscapeFuncs.destroystream)(instance, stream, reason);
#endif
}

void
NPN_Status(NPP instance, const char* message)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	CallNPN_StatusProc(gNetscapeFuncs.status, instance, message);
#else
	(*gNetscapeFuncs.status)(instance, message);
#endif
}

const char*
NPN_UserAgent(NPP instance)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_UserAgentProc(gNetscapeFuncs.uagent, instance);
#else
	return (*gNetscapeFuncs.uagent)(instance);
#endif
}

void*
NPN_MemAlloc(uint32 size)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_MemAllocProc(gNetscapeFuncs.memalloc, size);
#else
	return (*gNetscapeFuncs.memalloc)(size);
#endif
}

void NPN_MemFree(void* ptr)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	CallNPN_MemFreeProc(gNetscapeFuncs.memfree, ptr);
#else
	(*gNetscapeFuncs.memfree)(ptr);
#endif
}

uint32 NPN_MemFlush(uint32 size)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_MemFlushProc(gNetscapeFuncs.memflush, size);
#else
	return (*gNetscapeFuncs.memflush)(size);
#endif
}

void NPN_ReloadPlugins(NPBool reloadPages)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	CallNPN_ReloadPluginsProc(gNetscapeFuncs.reloadplugins, reloadPages);
#else
	(*gNetscapeFuncs.reloadplugins)(reloadPages);
#endif
}

NPError
NPN_PostURLNotify(NPP instance, const char* url, const char* window,
	     uint32 len, const char* buf, NPBool file, void* notifyData)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_PostURLNotifyProc(gNetscapeFuncs.posturlnotify, instance,
					url, window, len, buf, file, notifyData);
#else
	return (*gNetscapeFuncs.posturlnotify)(instance, url, window, len, buf, file, notifyData);
#endif
}

NPError
NPN_GetURLNotify(NPP instance, const char* url, const char* window, void* notifyData)
{
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_GetURLNotifyProc(gNetscapeFuncs.geturlnotify, instance,
					url, window, notifyData);
#else
	return (*gNetscapeFuncs.geturlnotify)(instance, url, window, notifyData);
#endif
}


#ifdef MY_JAVA
JRIEnv* NPN_GetJavaEnv()
{
# if (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_GetJavaEnvProc(gNetscapeFuncs.getJavaEnv);
# else
	return (*gNetscapeFuncs.getJavaEnv);
# endif
}

jref NPN_GetJavaPeer(NPP instance)
{
# if (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
	return CallNPN_GetJavaPeerProc(gNetscapeFuncs.getJavaPeer,
				       instance);
# else
	return (*gNetscapeFuncs.getJavaPeer)(instance);
# endif
}
#endif


/***********************************************************************
 *
 * Wrapper functions : Netscape Navigator -> plugin
 *
 * These functions let the plugin developer just create the APIs
 * as documented and defined in npapi.h, without needing to 
 * install those functions in the function table or worry about
 * setting up globals for 68K plugins.
 *
 ***********************************************************************/

NPError
Private_New(NPMIMEType pluginType, NPP instance, uint16 mode,
		int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
	NPError ret;
	PLUGINDEBUGSTR("New");
	ret = NPP_New(pluginType, instance, mode, argc, argn, argv, saved);
	return ret;	
}

NPError
Private_Destroy(NPP instance, NPSavedData** save)
{
	PLUGINDEBUGSTR("Destroy");
	return NPP_Destroy(instance, save);
}

NPError
Private_SetWindow(NPP instance, NPWindow* window)
{
	NPError err;
	PLUGINDEBUGSTR("SetWindow");
	err = NPP_SetWindow(instance, window);
	return err;
}

NPError
Private_NewStream(NPP instance, NPMIMEType type, NPStream* stream,
			NPBool seekable, uint16* stype)
{
	NPError err;
	PLUGINDEBUGSTR("NewStream");
	err = NPP_NewStream(instance, type, stream, seekable, stype);
	return err;
}

int32
Private_WriteReady(NPP instance, NPStream* stream)
{
	unsigned int result;
	PLUGINDEBUGSTR("WriteReady");
	result = NPP_WriteReady(instance, stream);
	return result;
}

int32
Private_Write(NPP instance, NPStream* stream, int32 offset, int32 len,
		void* buffer)
{
	unsigned int result;
	PLUGINDEBUGSTR("Write");
	result = NPP_Write(instance, stream, offset, len, buffer);
	return result;
}

void
Private_StreamAsFile(NPP instance, NPStream* stream, const char* fname)
{
	PLUGINDEBUGSTR("StreamAsFile");
	NPP_StreamAsFile(instance, stream, fname);
}


NPError
Private_DestroyStream(NPP instance, NPStream* stream, NPError reason)
{
	NPError err;
	PLUGINDEBUGSTR("DestroyStream");
	err = NPP_DestroyStream(instance, stream, reason);
	return err;
}


void
Private_Print(NPP instance, NPPrint* platformPrint)
{
	PLUGINDEBUGSTR("Print");
	NPP_Print(instance, platformPrint);
}

void
Private_URLNotify(NPP instance, const char* url, NPReason reason, void *notifyData)
{
       PLUGINDEBUGSTR("URLNotify");
       NPP_URLNotify(instance, url, reason, notifyData);
}

JRIGlobalRef
Private_GetJavaClass(void)
{
/*JAS 
#ifdef MY_JAVA
    jref clazz = NPP_GetJavaClass();
#else 
    jref clazz = NULL;
#endif
    if (clazz) {
	JRIEnv* env = NPN_GetJavaEnv();
	return JRI_NewGlobalRef(env, clazz);
    }
*/
    return NULL;
}

/*********************************************************************** 
 *
 * These functions are located automagically by netscape.
 *
 ***********************************************************************/

/*
 * NP_GetMIMEDescription
 *	- Netscape needs to know about this symbol
 *	- Netscape uses the return value to identify when an object instance
 *	  of this plugin should be created.
 */
char *
NP_GetMIMEDescription(void)
{
	return NPP_GetMIMEDescription();
}

/*
 * NP_GetValue [optional]
 *	- Netscape needs to know about this symbol.
 *	- Interfaces with plugin to get values for predefined variables
 *	  that the navigator needs.
 */
NPError
NP_GetValue(void *future, NPPVariable variable, void *value)
{
	return NPP_GetValue(future, variable, value);
}


/*
 * NP_Initialize
 *	- Netscape needs to know about this symbol.
 *	- It calls this function after looking up its symbol before it
 *	  is about to create the first ever object of this kind.
 *
 * PARAMETERS
 *    nsTable	- The netscape function table. If developers just use these
 *		  wrappers, they dont need to worry about all these function
 *		  tables.
 * RETURN
 *    pluginFuncs
 *		- This functions needs to fill the plugin function table
 *		  pluginFuncs and return it. Netscape Navigator plugin
 *		  library will use this function table to call the plugin.
 *
 */
NPError
NP_Initialize(NPNetscapeFuncs* nsTable, NPPluginFuncs* pluginFuncs)
{
	NPError err = NPERR_NO_ERROR;

	PLUGINDEBUGSTR("NP_Initialize");
	
	/* validate input parameters */

	if ((nsTable == NULL) || (pluginFuncs == NULL))
		err = NPERR_INVALID_FUNCTABLE_ERROR;
	
	/*
	 * Check the major version passed in Netscape's function table.
	 * We won't load if the major version is newer than what we expect.
	 * Also check that the function tables passed in are big enough for
	 * all the functions we need (they could be bigger, if Netscape added
	 * new APIs, but that's OK with us -- we'll just ignore them).
	 *
	 */

	if (err == NPERR_NO_ERROR) {
		if ((nsTable->version >> 8) > NP_VERSION_MAJOR)
			err = NPERR_INCOMPATIBLE_VERSION_ERROR;
		if (nsTable->size < sizeof(NPNetscapeFuncs))
			err = NPERR_INVALID_FUNCTABLE_ERROR;
		if (pluginFuncs->size < sizeof(NPPluginFuncs))		
			err = NPERR_INVALID_FUNCTABLE_ERROR;
	}
		
	
	if (err == NPERR_NO_ERROR) {
		/*
		 * Copy all the fields of Netscape function table into our
		 * copy so we can call back into Netscape later.  Note that
		 * we need to copy the fields one by one, rather than assigning
		 * the whole structure, because the Netscape function table
		 * could actually be bigger than what we expect.
		 */
		gNetscapeFuncs.version       = nsTable->version;
		gNetscapeFuncs.size          = nsTable->size;
		gNetscapeFuncs.posturl       = nsTable->posturl;
		gNetscapeFuncs.geturl        = nsTable->geturl;
		gNetscapeFuncs.requestread   = nsTable->requestread;
		gNetscapeFuncs.newstream     = nsTable->newstream;
		gNetscapeFuncs.write         = nsTable->write;
		gNetscapeFuncs.destroystream = nsTable->destroystream;
		gNetscapeFuncs.status        = nsTable->status;
		gNetscapeFuncs.uagent        = nsTable->uagent;
		gNetscapeFuncs.memalloc      = nsTable->memalloc;
		gNetscapeFuncs.memfree       = nsTable->memfree;
		gNetscapeFuncs.memflush      = nsTable->memflush;
		gNetscapeFuncs.reloadplugins = nsTable->reloadplugins;
		gNetscapeFuncs.getJavaEnv    = nsTable->getJavaEnv;
		gNetscapeFuncs.getJavaPeer   = nsTable->getJavaPeer;
		gNetscapeFuncs.geturlnotify  = nsTable->geturlnotify;
		gNetscapeFuncs.posturlnotify = nsTable->posturlnotify;
		gNetscapeFuncs.getvalue      = nsTable->getvalue;

		/*
		 * Set up the plugin function table that Netscape will use to
		 * call us.  Netscape needs to know about our version and size
		 * and have a UniversalProcPointer for every function we
		 * implement.
		 */
		pluginFuncs->version    = (NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR;
		pluginFuncs->size       = sizeof(NPPluginFuncs);
#if  (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
		pluginFuncs->newp       = NewNPP_NewProc(Private_New);
		pluginFuncs->destroy    = NewNPP_DestroyProc(Private_Destroy);
		pluginFuncs->setwindow  = NewNPP_SetWindowProc(Private_SetWindow);
		pluginFuncs->newstream  = NewNPP_NewStreamProc(Private_NewStream);
		pluginFuncs->destroystream = NewNPP_DestroyStreamProc(Private_DestroyStream);
		pluginFuncs->asfile     = NewNPP_StreamAsFileProc(Private_StreamAsFile);
		pluginFuncs->writeready = NewNPP_WriteReadyProc(Private_WriteReady);
		pluginFuncs->write      = NewNPP_WriteProc(Private_Write);
		pluginFuncs->print      = NewNPP_PrintProc(Private_Print);
		pluginFuncs->urlnotify  = NewNPP_URLNotifyProc(Private_URLNotify);
#else
                pluginFuncs->newp       = (NPP_NewProcPtr)(Private_New);
                pluginFuncs->destroy    = (NPP_DestroyProcPtr)(Private_Destroy);
                pluginFuncs->setwindow  = (NPP_SetWindowProcPtr)(Private_SetWindow);
                pluginFuncs->newstream  = (NPP_NewStreamProcPtr)(Private_NewStream);
                pluginFuncs->destroystream = (NPP_DestroyStreamProcPtr)(Private_DestroyStream);
                pluginFuncs->asfile     = (NPP_StreamAsFileProcPtr)(Private_StreamAsFile);
                pluginFuncs->writeready = (NPP_WriteReadyProcPtr)(Private_WriteReady);
                pluginFuncs->write      = (NPP_WriteProcPtr)(Private_Write);
                pluginFuncs->print      = (NPP_PrintProcPtr)(Private_Print);
                pluginFuncs->urlnotify  = (NPP_URLNotifyProcPtr)(Private_URLNotify);
#endif
		pluginFuncs->event      = NULL;
		pluginFuncs->javaClass	= Private_GetJavaClass();

		err = NPP_Initialize();
	}
	
	return err;
}

/*
 * NP_Shutdown [optional]
 *	- Netscape needs to know about this symbol.
 *	- It calls this function after looking up its symbol after
 *	  the last object of this kind has been destroyed.
 *
 */
NPError
NP_Shutdown(void)
{
 	PLUGINDEBUGSTR("NP_Shutdown");
	NPP_Shutdown();
	return NPERR_NO_ERROR;
}
