/*
=INSERT_TEMPLATE_HERE=

$Id: ConsoleMessage.c,v 1.33 2012/07/29 15:47:37 crc_canada Exp $

When running in a plugin, there is no way
any longer to get the console messages to come up - eg, no
way to say "Syntax error in X3D file".

Old netscapes used to have a console.

So, now, we pop up xmessages for EACH call here, when running
as a plugin.

NOTE: Parts of this came from on line examples; apologies
for loosing the reference. Also, most if it is found in
"the C programming language" second edition.

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

#include <stdio.h>
#include <stdarg.h> //TODO: configure check

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "headers.h"
#include "../plugin/pluginUtils.h"
#include "../plugin/PluginSocket.h"

#define STRING_LENGTH 2000	/* something 'safe'	*/

/* JAS - make the Console Log write to the log app on OSX for all invocations of the library */
#ifdef AQUA
	int logFileOpened = FALSE;
	char ConsoleLogName[200];
#endif

typedef struct pConsoleMessage{
	int Console_writeToCRT;// = 1; /*regular printf*/
	int Console_writeToFile;// = 0;
	int Console_writeToLog;// = 0;
	int Console_writePrimitive; //= 0;
	int consolefileOpened;// = 0;
	FILE* consolefile;
	int setDefAqua;// = 0;
	int setTargetAqua;// = 0;
	int setHaveMotif;// = 0;
	int setTargetMotif;// = 0;
	int setHaveMscVer;// = 0;
	int setTargetAndroid;// = 0;

#if defined (_ANDROID)
#define MAX_ANDROID_CONSOLE_MESSAGE_SLOTS 20
	int androidFreeSlot;
	char * androidMessageSlot[MAX_ANDROID_CONSOLE_MESSAGE_SLOTS];
	int androidHaveUnreadMessages;
	
#endif //ANDROID

#ifdef _MSC_VER
	HANDLE hStdErr; // = NULL;
#endif
	char FWbuffer [STRING_LENGTH];


}* ppConsoleMessage;

static void *ConsoleMessage_constructor(){
	void *v = malloc(sizeof(struct pConsoleMessage));
	memset(v,0,sizeof(struct pConsoleMessage));
	return v;
}
void ConsoleMessage_init(struct tConsoleMessage *t){
	//public
	//t->Console_writeToHud; //= 0; /*something should change this to 1 if running statusbarHUD or (a gui with no console and) statusbarConsole*/
	//t->consMsgCount; // = 0;
	//private
	t->prv = ConsoleMessage_constructor();
	{
		ppConsoleMessage p = (ppConsoleMessage)t->prv;
		p->Console_writeToCRT = 1; /*regular printf*/
		p->Console_writeToFile = 0;
		//p->Console_writeToHud = 0; /*something should change this to 1 if running statusbarHUD or (a gui with no console and) statusbarConsole*/
		p->Console_writeToLog = 0;
		p->Console_writePrimitive = 0;
		p->consolefileOpened = 0;
		//FILE* consolefile;
		p->setDefAqua = 0;
		p->setTargetAqua = 0;
		p->setHaveMotif = 0;
		p->setTargetMotif = 0;
		p->setHaveMscVer = 0;
		p->setTargetAndroid = 0;

#ifdef _MSC_VER
		p->hStdErr = NULL;
#endif
		//p->FWbuffer [STRING_LENGTH]; null ok
	}
}


#if defined(_ANDROID)
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "FreeWRL-ConsoleMessage"
#endif

/* >>> statusbar hud */
void hudSetConsoleMessage(char *buffer);
/* <<< statusbar hud */


/* runtime replace DEF_AQUA , TARGET_AQUA , HAVE_MOTIF , TARGET_MOTIF , MC_MSC_HAVE_VER); */
void fwl_ConsoleSetup(int DefAqua , int TargetAqua , int HaveMotif , int TargetMotif , int HaveMscVer , int TargetAndroid) {

	ttglobal tg = gglobal();
	ppConsoleMessage p = (ppConsoleMessage)tg->ConsoleMessage.prv;
	p->setDefAqua = DefAqua ;
	p->setTargetAqua  = TargetAqua ;
	p->setHaveMotif  = HaveMotif ;
	p->setTargetMotif  = TargetMotif ;
	p->setHaveMscVer = HaveMscVer ;
	p->setTargetAndroid  = TargetAndroid ;
	//it helps to know a bit earlier -before the first statusbarHud.c draw-
	//if you have it, then early messages come out on the ! panel
#ifdef STATUSBAR_HUD
	//if(statusBarHud)
	tg->ConsoleMessage.Console_writeToHud = 1;
#endif
}
int fwl_StringConsoleMessage(char* consoleBuffer) { return ConsoleMessage(consoleBuffer); }

#ifdef AQUA
	#include <syslog.h> //TODO: configure check
	/* for sending text to the System Console */
	//static int logFileOpened = FALSE;
	//char ConsoleLogName[200];
#endif

static int Console_writePrimitive = 0; //for msc_ver, but will test in shared code
#ifdef _MSC_VER
#if _MSC_VER < 1500
#define HAVE_VSCPRINTF
#endif

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <wtypes.h>
//a console window should be a single shared process resource
static HANDLE hStdErr = NULL;
void fwl_setConsole_writePrimitive(int ibool)
{
	//this function used by the dll wrapper in win32 for 
	//some types of applications (not the console program as of Jun1/2011)
	//ppConsoleMessage p = (ppConsoleMessage)gglobal()->ConsoleMessage.prv;
	//if(ibool) p->Console_writePrimitive = 1;
	//else  p->Console_writePrimitive = 0;
	Console_writePrimitive = ibool;
}
void initConsoleH(DWORD pid)
{
	//ppConsoleMessage p = (ppConsoleMessage)gglobal()->ConsoleMessage.prv;
#if _MSC_VER >= 1500

	//p->hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	//if(!p->hStdErr)
	hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	if(!hStdErr)
	if( !AttachConsole(pid))
	{
		DWORD dw = GetLastError();
		if(dw==ERROR_ACCESS_DENIED)
			printf("attachconsole access denied\n");
		else if(dw==ERROR_INVALID_HANDLE)
			printf("attachconsole invalid handle\n");
		else if(dw==ERROR_GEN_FAILURE)
			printf("attachconsole gen failure\n");
		AllocConsole();
	}
	hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	//p->hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	//if(!p->hStdErr) p->hStdErr = -1;
#endif
}

static void initConsole(void)
{
    BOOL ac;
	//ppConsoleMessage p = (ppConsoleMessage)gglobal()->ConsoleMessage.prv;

	//hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	//if(p->hStdErr == NULL)
	if(hStdErr == NULL)
	{
#ifndef ATTACH_PARENT_PROCESS
#define ATTACH_PARENT_PROCESS ((DWORD)-1)
#endif
#if _MSC_VER >= 1500
		if( !AttachConsole(ATTACH_PARENT_PROCESS))
		{
			DWORD dw = GetLastError();
			if(dw==ERROR_ACCESS_DENIED)
				printf("attachconsole access denied\n");
			else if(dw==ERROR_INVALID_HANDLE)
				printf("attachconsole invalid handle\n");
			else if(dw==ERROR_GEN_FAILURE)
				printf("attachconsole gen failure\n");
			ac = AllocConsole();
		}
#endif
		//p->hStdErr = GetStdHandle(STD_ERROR_HANDLE);
		hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	}
}
void writeToWin32Console(char *buff)
{
	//ppConsoleMessage p = (ppConsoleMessage)gglobal()->ConsoleMessage.prv;

    DWORD cWritten;
    //if (p->hStdErr == NULL)
    if (hStdErr == NULL)
        initConsole(); 
    /* not C console - more low level windows SDK API */
    //WriteConsoleA(p->hStdErr, buff, strlen(buff),&cWritten, NULL);
    WriteConsoleA(hStdErr, buff, strlen(buff),&cWritten, NULL);
}
//stub for vc7
int DEBUG_FPRINTF(const char *fmt, ...)
{
	return 0;
}

#endif

//static char FWbuffer [STRING_LENGTH];


//int consMsgCount = 0;
extern int _fw_browser_plugin;
#define MAXMESSAGES 5 
void closeConsoleMessage() {
	gglobal()->ConsoleMessage.consMsgCount = 0;
#ifdef AQUA
	if (logFileOpened) syslog (LOG_ALERT, "FreeWRL loading a new file");
	logFileOpened = FALSE;
#endif
}

#define NEW_CONSOLEMESSAGE_VERSION 1

#ifdef NEW_CONSOLEMESSAGE_VERSION

/* try to open a file descriptor to the Console Log - on OS X 
	   this should display the text on the "Console.app" */
void openLogfile()
{
	#ifdef AQUA
	if (!logFileOpened) {
		logFileOpened = TRUE;
		openlog("freewrl", LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
		setlogmask(LOG_UPTO(LOG_ERR));
		syslog(LOG_ALERT,"FreeWRL opened Console Log");
	}
	#endif
}
void writeToLogFile(char *buffer)
{
	#ifdef AQUA
	if(!logFileOpened) openLogfile();
	/* print this to the console log */
	syslog (LOG_ALERT, buffer);
	#endif
}




int fwvsnprintf(char *buffer,int buffer_length, const char *fmt, va_list ap)
{
	int i,j,count;
	//char tempbuf[STRING_LENGTH];
	//char format[STRING_LENGTH];
	char *tempbuf;
	char *format;
	char c;
	double d;
	unsigned u;
	char *s;
	void *v;
	tempbuf = malloc(buffer_length);
	format = malloc(buffer_length);
	count = 0;
	buffer[0] = '\0';
	while (*fmt) 
	{
		tempbuf[0] = '\0';
		for (j = 0; fmt[j] && fmt[j] != '%'; j++) {
			format[j] = fmt[j];	/* not a format string	*/
		}

		if (j) {
			format[j] = '\0';
			count += sprintf(tempbuf, format);/* printf it verbatim				*/
			fmt += j;
		} else {
			for (j = 0; !isalpha(fmt[j]); j++) {	 /* find end of format specifier */
				format[j] = fmt[j];
				if (j && fmt[j] == '%')				/* special case printing '%'		*/
					break;
			}
			format[j] = fmt[j];			/* finish writing specifier		 */
			format[j + 1] = '\0';			/* don't forget NULL terminator */
			fmt += j + 1;

			switch (format[j]) {			 /* cases for all specifiers		 */
			case 'd':
			case 'i':						/* many use identical actions	 */
				i = va_arg(ap, int);		 /* process the argument	 */
				count += sprintf(tempbuf, format, i); /* and printf it		 */
				break;
			case 'o':
			case 'x':
			case 'X':
			case 'u':
				u = va_arg(ap, unsigned);
				count += sprintf(tempbuf, format, u);
				break;
			case 'c':
				c = (char) va_arg(ap, int);		/* must cast!			 */
				count += sprintf(tempbuf, format, c);
				break;
			case 's':
				s = va_arg(ap, char *);
				/* limit string to a certain length */
				if ((strlen(s) + count) > buffer_length) {
					char tmpstr[100];
					int ltc;
					ltc = (int) strlen(s);
					if (ltc>80) ltc=80;
					strncpy (tmpstr, s, ltc);
					tmpstr[ltc] = '.'; ltc++;
					tmpstr[ltc] = '.'; ltc++;
					tmpstr[ltc] = '.'; ltc++;
					tmpstr[ltc] = '\0';

					count += sprintf (tempbuf, format, tmpstr);
				} else count += sprintf(tempbuf, format, s);
				break;
			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
				d = va_arg(ap, double);
				count += sprintf(tempbuf, format, d);
				break;
			case 'p':
				v = va_arg(ap, void *);
				count += sprintf(tempbuf, format, v);
				break;
			case 'n':
				count += sprintf(tempbuf, "%d", count);
				break;
			case '%':
				count += sprintf(tempbuf, "%%");
				break;
			default:
				ERROR_MSG("ConsoleMessage: invalid format specifier: %c\n", format[j]);
			}
		}
		if( (strlen(tempbuf) + strlen(buffer)) < (buffer_length) -10) 
		{
			strcat (buffer,tempbuf);
		}
	}
	free(tempbuf);
	free(format);
	return 1;
}


/* >>> statusbar hud */
//int Console_writeToCRT = 1; /*regular printf*/
//int Console_writeToFile = 0;
//int Console_writeToHud = 0; /*something should change this to 1 if running statusbarHUD or (a gui with no console and) statusbarConsole*/
//int Console_writeToLog = 0;
//int Console_writePrimitive = 0;
//int consolefileOpened = 0;
//FILE* consolefile;


#if defined (_ANDROID)
static void android_save_log(char *thislog) {
	ttglobal tg = gglobal();
	ppConsoleMessage p = (ppConsoleMessage)tg->ConsoleMessage.prv;
	int i;

	// sanity check the string, otherwise dalvik can croak if invalid chars
	for (i=0; i<strlen(thislog); i++) {
		thislog[i] = thislog[i]&0x7f;
	}

	/* go to next slot, wrap around*/
	p->androidFreeSlot++;
	if (p->androidFreeSlot>=MAX_ANDROID_CONSOLE_MESSAGE_SLOTS) p->androidFreeSlot=0; 

	/* free our copy of this string if required; then set the pointer for this slot
	   to our free slot */
	if (p->androidMessageSlot[p->androidFreeSlot] != NULL) free (p->androidMessageSlot[p->androidFreeSlot]);
	p->androidMessageSlot[p->androidFreeSlot] = thislog;

	// indicate we have messages
	p->androidHaveUnreadMessages++;
}

// tell the UI how many unread console messages we have.
int android_get_unread_message_count() {
	ttglobal tg = gglobal();
	if (!tg) return 0;
	ppConsoleMessage p = (ppConsoleMessage)tg->ConsoleMessage.prv;
	return p->androidHaveUnreadMessages;


}

char *android_get_last_message(int whichOne) {
	ttglobal tg = gglobal();
	int whm;

	if (!tg) return "NO GGLOBAL - NO MESSAGES";
	ppConsoleMessage p = (ppConsoleMessage)tg->ConsoleMessage.prv;
	
	// reset the "number of messages" counter.
	p->androidHaveUnreadMessages = 0;

	// which message from our rotating pool do we want?
	whm = p->androidFreeSlot - whichOne;
	if (whm < 0) whm = MAX_ANDROID_CONSOLE_MESSAGE_SLOTS-1;

	if (p->androidMessageSlot[whm] == NULL) return strdup("");

	return strdup(p->androidMessageSlot[whm]);
}

#endif //ANDROID


int ConsoleMessage0(const char *fmt, va_list args)  
{
#if defined(_ANDROID)

	int retval;
	ppConsoleMessage p;
	ttglobal tg = gglobal0();
	char *buffer;
        char *cp;
        u_int nalloc;
        int r;

        r = vasprintf(&cp, fmt, args);
	__android_log_print(ANDROID_LOG_INFO,LOG_TAG,cp,NULL);

	// save log file for FreeWRL on-screen log printer
	if (tg) {
		android_save_log(cp);
	}


	return 1;
#else
	int retval;
	ppConsoleMessage p;
	ttglobal tg = gglobal0();
	if(!tg){
		/* we must be just starting up or shutting down -
		   just do primitive console write to (global resource) stdout console*/
		retval = vfprintf(stdout,fmt,args); //printf(buffer);
#ifdef TARGET_AQUA
		/* JohnS - we need a carrage return here */
		printf ("\n");
#endif
#ifdef _MSC_VER
		if(Console_writePrimitive)
		{
			char pbuffer[1024];
			vsnprintf(pbuffer,1023,fmt,args);
			writeToWin32Console(pbuffer);
		}
#endif
		return retval; 
	}
	p = (ppConsoleMessage)tg->ConsoleMessage.prv;
	retval = 0;
	//p->Console_writeToCRT = 0; //test
	//p->Console_writeToFile = 1; //test
	if(p->Console_writeToCRT) {
		retval = vfprintf(stdout,fmt,args); //printf(buffer);
#ifdef TARGET_AQUA
		/* JohnS - we need a carrage return here */
		printf ("\n");
#endif
	}

	if(p->Console_writeToFile)
	{
		if(!p->consolefileOpened)
		{
			p->consolefile = fopen("freewrl_console.txt","w");
			p->consolefileOpened = 1;
		}
		retval = vfprintf(p->consolefile,fmt,args); //fprintf(consolefile,buffer);
	}
	if(p->Console_writeToLog || tg->ConsoleMessage.Console_writeToHud || Console_writePrimitive)
	{
		char * buffer;
		int doFree = 0;

#ifdef HAVE_VSCPRINTF
		/* msvc can do this  http://msdn.microsoft.com/en-ca/library/xa1a1a6z(VS.80).aspx  */
		int len;
		len = _vscprintf( fmt, args ) +1; /* counts only */
		buffer = malloc( len * sizeof(char) );
		retval = vsprintf( buffer, fmt, args );
		//retval = vsprintf_s( buffer, len, fmt, args ); /*allocates the len you pass in*/
		doFree = 1;
#elif HAVE_VASPRINTF
		/* http://linux.die.net/man/3/vasprintf a GNU extension, not tested */
		retval = vasprintf(buffer,fmt,args); /*allocates correct length buffer and writes*/
	`	//retval = vfprintf(FILE*, fmt, args); //michel says standard lib, writes to FILE*
		doFree = 1;
#elif HAVE_FWVSNPRINTF
		/* reworked code from aqua - seems OK in msvc - if you don't have regular vsnprintf or _vscprintf we can use this */
		retval = fwvsnprintf(FWbuffer,STRING_LENGTH-1,fmt,args); /*hope STRING_LENGTH is long enough, else -1 skip */
		buffer = FWbuffer;
		doFree = 0;
#else
		/* and msvc can do this, _msc_ver currently uses. */
		retval = vsnprintf(p->FWbuffer,STRING_LENGTH-1,fmt,args); /*hope STRING_LENGTH is long enough, else -1 skip */
		if(retval < 0) return retval;
		buffer = p->FWbuffer;
		doFree = 0;
#endif
		if(p->Console_writeToLog)
			writeToLogFile(buffer);
		if(tg->ConsoleMessage.Console_writeToHud)
			hudSetConsoleMessage(buffer);
#ifdef _MSC_VER
		if(Console_writePrimitive)
			writeToWin32Console(buffer);
#endif

		if(doFree) free( buffer ); 
	}
	return retval;
#endif // _ANDROID
}
int ConsoleMessage(const char *fmt, ...)
{
	/*
		There's lots I don't understand such as aqua vs motif vs ??? and plugin vs ??? and the sound/speaker method
		Q. if we call ConsoleMessage from any thread, should there be a thread lock on something, 
		for example s_list_t *conlist (see hudConsoleMessage() in statusbarHud.c)?
	*/

	va_list args;
	va_start( args, fmt );
	return ConsoleMessage0(fmt,args);
}
int BrowserPrintConsoleMessage(const char *fmt, ...) 
{
	/* Q. am I right to assume any denial of service attacks would only come from the jsVRMLBrowser.c VRMLBrowserPrint() function?
	    If so I propose the defence should be just against that, by
		calling this function instead of ConsoleMessage from VRMLBrowserPrint.
		Dec 23 2009 status: tested ndef-motif-ndef-aqua branch calling BrowserPrintConsoleMessage from mainloop as a test but 
		not from VRMLBrowserPrint - is there  a test .wrl for it?
	*/
	/* unused int retval; */
	va_list args;
	ppConsoleMessage p;
	ttglobal tg = gglobal();
	p = (ppConsoleMessage)gglobal()->ConsoleMessage.prv;

	if(p->setHaveMotif == 0) {
	/* did we have too many messages - don't want to make this into a 
	   denial of service attack! (thanks, Mufti) */
		if( (p->setDefAqua == 1 && RUNNINGASPLUGIN && tg->ConsoleMessage.consMsgCount > MAXMESSAGES) || (p->setDefAqua == 0 && tg->ConsoleMessage.consMsgCount > MAXMESSAGES) ) {
			if (tg->ConsoleMessage.consMsgCount > (MAXMESSAGES + 5)) return -1;
			tg->ConsoleMessage.consMsgCount = MAXMESSAGES + 100;
			return ConsoleMessage("Too many freewrl messages - stopping ConsoleMessage");
		} 
		tg->ConsoleMessage.consMsgCount++;
	}
	va_start( args, fmt );
	return ConsoleMessage0(fmt,args);
}


/* <<< statusbar hud */

#endif


/**
 ** ALL the Console stuff has to be refactored.
 **
 ** I suggest a very simple solution: write to stdout/stderr.
 **
 ** On OSX the Console application should wait for something written on those file descriptors...
 ** ... and display the last string each time a new data arrives.
 **/

#ifdef OLD_CONSOLEMESSAGE_VERSION
OLDCODEint Console_writeToHud;
OLDCODE
OLDCODEint ConsoleMessage(const char *fmt, ...) {
OLDCODE	va_list ap;
OLDCODE	char tempbuf[STRING_LENGTH];
OLDCODE	char format[STRING_LENGTH];
OLDCODE	int count = 0;
OLDCODE	int i, j;
OLDCODE	char c;
OLDCODE	double d;
OLDCODE	unsigned u;
OLDCODE	char *s;
OLDCODE	void *v;
OLDCODE	char ConsoleLogName[100];
OLDCODE	ppConsoleMessage p = (ppConsoleMessage)gglobal()->ConsoleMessage.prv;
OLDCODE
OLDCODE	/* try to open a file descriptor to the Console Log - on OS X 
OLDCODE	   this should display the text on the "Console.app" */
OLDCODE	#ifdef AQUA
OLDCODE	if (!logFileOpened) {
OLDCODE		logFileOpened = TRUE;
OLDCODE		openlog("freewrl", LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
OLDCODE		setlogmask(LOG_UPTO(LOG_ERR));
OLDCODE		syslog(LOG_ALERT,"FreeWRL opened Console Log");
OLDCODE	}
OLDCODE	#endif
OLDCODE
OLDCODE
OLDCODE	if(p->setHaveMotif == 1) {
OLDCODE	p->FWbuffer[0] = '\n';
OLDCODE	p->FWbuffer[1] = '\0';
OLDCODE	} else {
OLDCODE	p->FWbuffer[0] = '\0';
OLDCODE	}
OLDCODE
OLDCODE	if(p->setHaveMotif == 0) {
OLDCODE	/* did we have too many messages - don't want to make this into a 
OLDCODE	   denial of service attack! (thanks, Mufti) */
OLDCODE		if( (p->setDefAqua == 1 && RUNNINGASPLUGIN && p->consMsgCount > MAXMESSAGES) || (p->setDefAqua == 0 && p->consMsgCount > MAXMESSAGES) ) {
OLDCODE			if (p->consMsgCount > (MAXMESSAGES + 5)) return -1;
OLDCODE			p->consMsgCount = MAXMESSAGES + 100;
OLDCODE			return ConsoleMessage("Too many freewrl messages - stopping ConsoleMessage");
OLDCODE		} 
OLDCODE		p->consMsgCount++;
OLDCODE	}
OLDCODE	
OLDCODE		va_start(ap, fmt);		 /* must be called before work	 */
OLDCODE
OLDCODE		while (*fmt) {
OLDCODE			tempbuf[0] = '\0';
OLDCODE			for (j = 0; fmt[j] && fmt[j] != '%'; j++) {
OLDCODE				format[j] = fmt[j];	/* not a format string	*/
OLDCODE			}
OLDCODE
OLDCODE			if (j) {
OLDCODE				format[j] = '\0';
OLDCODE				count += sprintf(tempbuf, format);/* printf it verbatim				*/
OLDCODE				fmt += j;
OLDCODE			} else {
OLDCODE				for (j = 0; !isalpha(fmt[j]); j++) {	 /* find end of format specifier */
OLDCODE					format[j] = fmt[j];
OLDCODE					if (j && fmt[j] == '%')				/* special case printing '%'		*/
OLDCODE						break;
OLDCODE				}
OLDCODE				format[j] = fmt[j];			/* finish writing specifier		 */
OLDCODE				format[j + 1] = '\0';			/* don't forget NULL terminator */
OLDCODE				fmt += j + 1;
OLDCODE
OLDCODE				switch (format[j]) {			 /* cases for all specifiers		 */
OLDCODE				case 'd':
OLDCODE				case 'i':						/* many use identical actions	 */
OLDCODE					i = va_arg(ap, int);		 /* process the argument	 */
OLDCODE					count += sprintf(tempbuf, format, i); /* and printf it		 */
OLDCODE					break;
OLDCODE				case 'o':
OLDCODE				case 'x':
OLDCODE				case 'X':
OLDCODE				case 'u':
OLDCODE					u = va_arg(ap, unsigned);
OLDCODE					count += sprintf(tempbuf, format, u);
OLDCODE					break;
OLDCODE				case 'c':
OLDCODE					c = (char) va_arg(ap, int);		/* must cast!			 */
OLDCODE					count += sprintf(tempbuf, format, c);
OLDCODE					break;
OLDCODE				case 's':
OLDCODE					s = va_arg(ap, char *);
OLDCODE					/* limit string to a certain length */
OLDCODE					if ((strlen(s) + count) > STRING_LENGTH) {
OLDCODE						char tmpstr[100];
OLDCODE						int ltc;
OLDCODE						ltc = strlen(s);
OLDCODE						if (ltc>80) ltc=80;
OLDCODE						strncpy (tmpstr, s, ltc);
OLDCODE						tmpstr[ltc] = '.'; ltc++;
OLDCODE						tmpstr[ltc] = '.'; ltc++;
OLDCODE						tmpstr[ltc] = '.'; ltc++;
OLDCODE						tmpstr[ltc] = '\0';
OLDCODE
OLDCODE						count += sprintf (tempbuf, format, tmpstr);
OLDCODE					} else count += sprintf(tempbuf, format, s);
OLDCODE					break;
OLDCODE				case 'f':
OLDCODE				case 'e':
OLDCODE				case 'E':
OLDCODE				case 'g':
OLDCODE				case 'G':
OLDCODE					d = va_arg(ap, double);
OLDCODE					count += sprintf(tempbuf, format, d);
OLDCODE					break;
OLDCODE				case 'p':
OLDCODE					v = va_arg(ap, void *);
OLDCODE					count += sprintf(tempbuf, format, v);
OLDCODE					break;
OLDCODE				case 'n':
OLDCODE					count += sprintf(tempbuf, "%d", count);
OLDCODE					break;
OLDCODE				case '%':
OLDCODE					count += sprintf(tempbuf, "%%");
OLDCODE					break;
OLDCODE				default:
OLDCODE					ERROR_MSG("ConsoleMessage: invalid format specifier: %c\n", format[j]);
OLDCODE				}
OLDCODE			}
OLDCODE		if ((strlen(tempbuf) + strlen(p->FWbuffer)) <
OLDCODE			(STRING_LENGTH) -10) {
OLDCODE			strcat (p->FWbuffer,tempbuf);
OLDCODE		}
OLDCODE		}
OLDCODE	
OLDCODE		va_end(ap);				/* clean up				 */
OLDCODE#ifndef HAVE_MOTIF
OLDCODE	}
OLDCODE#endif
OLDCODE
OLDCODE#ifdef AQUA
OLDCODE	/* print this to stdio */
OLDCODE	/* printf (p->FWbuffer); if (p->FWbuffer[strlen(p->FWbuffer-1)] != '\n') printf ("\n"); */
OLDCODE
OLDCODE        if ((strlen(p->FWbuffer)) < (STRING_LENGTH) -10) {
OLDCODE		strcat (p->FWbuffer,"\n");
OLDCODE	}
OLDCODE
OLDCODE	/* print this to the console log */
OLDCODE	syslog (LOG_ALERT, p->FWbuffer);
OLDCODE
OLDCODE	/* print this to the application console log if running standalone, or speak it if running as a plug in */
OLDCODEprintf ("Console message, might be able to print\n"); if (!RUNNINGASPLUGIN) printf ("not running as plugin\n"); else printf ("runasp\n");
OLDCODE
OLDCODE	if (!RUNNINGASPLUGIN) {
OLDCODE		if (strcmp(p->FWbuffer, "\n") && strcmp(p->FWbuffer, "\n\n")) {
OLDCODE			aquaSetConsoleMessage(p->FWbuffer);
OLDCODE		}
OLDCODE	}
OLDCODE#else
OLDCODE	/* are we running under Motif or Gtk? */
OLDCODE	if(p->setTargetMotif == 1) {
OLDCODE		setConsoleMessage (p->FWbuffer);
OLDCODE	} else {
OLDCODE		if (RUNNINGASPLUGIN) {
OLDCODE			freewrlSystem (p->FWbuffer);
OLDCODE		} else {
OLDCODE			printf (p->FWbuffer); if (FWbuffer[strlen(p->FWbuffer-1)] != '\n') printf ("\n");
OLDCODE		}
OLDCODE	}
OLDCODE
OLDCODE#endif
OLDCODE	return count;
OLDCODE}
#endif /*ifdef OLD_CONSOLEMESSAGE_VERSION */

