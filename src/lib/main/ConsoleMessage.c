/*
=INSERT_TEMPLATE_HERE=

$Id: ConsoleMessage.c,v 1.21 2011/05/25 19:26:34 davejoubert Exp $

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

#if defined(_ANDROID)
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "WRL-"
#endif

/* >>> statusbar hud */
void hudSetConsoleMessage(char *buffer);
/* <<< statusbar hud */

#define STRING_LENGTH 2000	/* something 'safe'	*/

/* runtime replace DEF_AQUA , TARGET_AQUA , HAVE_MOTIF , TARGET_MOTIF , MC_MSC_HAVE_VER); */
int setDefAqua = 0;
int setTargetAqua = 0;
int setHaveMotif = 0;
int setTargetMotif = 0;
int setHaveMscVer = 0;
int setTargetAndroid = 0;
void fwl_ConsoleSetup(int DefAqua , int TargetAqua , int HaveMotif , int TargetMotif , int HaveMscVer , int TargetAndroid) {
	setDefAqua = DefAqua ;
	setTargetAqua  = TargetAqua ;
	setHaveMotif  = HaveMotif ;
	setTargetMotif  = TargetMotif ;
	setHaveMscVer = HaveMscVer ;
	setTargetAndroid  = TargetAndroid ;
}
int fwl_StringConsoleMessage(char* consoleBuffer) { return ConsoleMessage(consoleBuffer); }

#ifdef AQUA
	#include <syslog.h> //TODO: configure check
	/* for sending text to the System Console */
	static int logFileOpened = FALSE;
	char ConsoleLogName[200];
#endif

#ifdef _MSC_VER
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <wtypes.h>
static HANDLE hStdErr = NULL;

void initConsoleH(DWORD pid)
{
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
}

static void initConsole(void)
{
    BOOL ac;
	//hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	if(hStdErr == NULL)
	{
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
		hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	}
}
void writeToWin32Console(char *buff)
{
    DWORD cWritten;
    if (hStdErr == NULL)
        initConsole(); 
    /* not C console - more low level windows SDK API */
    WriteConsoleA(hStdErr, buff, strlen(buff),&cWritten, NULL);
}

#endif

static char FWbuffer [STRING_LENGTH];


int consMsgCount = 0;
extern int _fw_browser_plugin;
#define MAXMESSAGES 5 
void closeConsoleMessage() {
	consMsgCount = 0;
#ifdef AQUA
	if (logFileOpened) syslog (LOG_ALERT, "FreeWRL loading a new file");
	logFileOpened = FALSE;
#endif
}

#define NEW_CONSOLEMESSAGE_VERSION 1
//#define OLD_CONSOLEMESSAGE_VERSION 1

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
int Console_writeToCRT = 1; /*regular printf*/
int Console_writeToFile = 0;
int Console_writeToHud = 0; /*something should change this to 1 if running statusbarHUD or (a gui with no console and) statusbarConsole*/
int Console_writeToLog = 0;
int Console_writePrimitive = 0;
int consolefileOpened = 0;
FILE* consolefile;
int ConsoleMessage0(const char *fmt, va_list args)  
{
#if defined(_ANDROID)
	__android_log_print(ANDROID_LOG_INFO,LOG_TAG,fmt,args);
	return 1;
#else
	int retval;
	retval = 0;
	//Console_writeToCRT = 0; //test
	//Console_writeToFile = 1; //test
	if(Console_writeToCRT) {
		retval = vfprintf(stdout,fmt,args); //printf(buffer);
#ifdef TARGET_AQUA
	/* JohnS - we need a carrage return here */
	printf ("\n");
#endif
	}

	if(Console_writeToFile)
	{
		if(!consolefileOpened)
		{
			consolefile = fopen("freewrl_console.txt","w");
			consolefileOpened = 1;
		}
		retval = vfprintf(consolefile,fmt,args); //fprintf(consolefile,buffer);
	}
	if(Console_writeToLog || Console_writeToHud || Console_writePrimitive)
	{
		char * buffer;
		int doFree = 0;

#ifdef HAVE_VSCPRINTF
		/* msvc can do this  http://msdn.microsoft.com/en-ca/library/xa1a1a6z(VS.80).aspx  */
		int len;
		len = _vscprintf( fmt, args ) +1; /* counts only */
		buffer = malloc( len * sizeof(char) );
		retval = vsprintf_s( buffer, len, fmt, args ); /*allocates the len you pass in*/
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
		retval = vsnprintf(FWbuffer,STRING_LENGTH-1,fmt,args); /*hope STRING_LENGTH is long enough, else -1 skip */
		if(retval < 0) return retval;
		buffer = FWbuffer;
		doFree = 0;
#endif
		if(Console_writeToLog)
			writeToLogFile(buffer);
		if(Console_writeToHud)
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

	if(setHaveMotif == 0) {
	/* did we have too many messages - don't want to make this into a 
	   denial of service attack! (thanks, Mufti) */
		if( (setDefAqua == 1 && RUNNINGASPLUGIN && consMsgCount > MAXMESSAGES) || (setDefAqua == 0 && consMsgCount > MAXMESSAGES) ) {
			if (consMsgCount > (MAXMESSAGES + 5)) return -1;
			consMsgCount = MAXMESSAGES + 100;
			return ConsoleMessage("Too many freewrl messages - stopping ConsoleMessage");
		} 
		consMsgCount++;
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
int Console_writeToHud;

int ConsoleMessage(const char *fmt, ...) {
	va_list ap;
	char tempbuf[STRING_LENGTH];
	char format[STRING_LENGTH];
	int count = 0;
	int i, j;
	char c;
	double d;
	unsigned u;
	char *s;
	void *v;
	char ConsoleLogName[100];

	/* try to open a file descriptor to the Console Log - on OS X 
	   this should display the text on the "Console.app" */
	#ifdef AQUA
	if (!logFileOpened) {
		logFileOpened = TRUE;
		openlog("freewrl", LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
		setlogmask(LOG_UPTO(LOG_ERR));
		syslog(LOG_ALERT,"FreeWRL opened Console Log");
	}
	#endif


	if(setHaveMotif == 1) {
	FWbuffer[0] = '\n';
	FWbuffer[1] = '\0';
	} else {
	FWbuffer[0] = '\0';
	}

	if(setHaveMotif == 0) {
	/* did we have too many messages - don't want to make this into a 
	   denial of service attack! (thanks, Mufti) */
		if( (setDefAqua == 1 && RUNNINGASPLUGIN && consMsgCount > MAXMESSAGES) || (setDefAqua == 0 && consMsgCount > MAXMESSAGES) ) {
			if (consMsgCount > (MAXMESSAGES + 5)) return -1;
			consMsgCount = MAXMESSAGES + 100;
			return ConsoleMessage("Too many freewrl messages - stopping ConsoleMessage");
		} 
		consMsgCount++;
	}
	
		va_start(ap, fmt);		 /* must be called before work	 */

		while (*fmt) {
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
					if ((strlen(s) + count) > STRING_LENGTH) {
						char tmpstr[100];
						int ltc;
						ltc = strlen(s);
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
		if ((strlen(tempbuf) + strlen(FWbuffer)) <
			(STRING_LENGTH) -10) {
			strcat (FWbuffer,tempbuf);
		}
		}
	
		va_end(ap);				/* clean up				 */
#ifndef HAVE_MOTIF
	}
#endif

#ifdef AQUA
	/* print this to stdio */
	/* printf (FWbuffer); if (FWbuffer[strlen(FWbuffer-1)] != '\n') printf ("\n"); */

        if ((strlen(FWbuffer)) < (STRING_LENGTH) -10) {
		strcat (FWbuffer,"\n");
	}

	/* print this to the console log */
	syslog (LOG_ALERT, FWbuffer);

	/* print this to the application console log if running standalone, or speak it if running as a plug in */
printf ("Console message, might be able to print\n"); if (!RUNNINGASPLUGIN) printf ("not running as plugin\n"); else printf ("runasp\n");

	if (!RUNNINGASPLUGIN) {
		if (strcmp(FWbuffer, "\n") && strcmp(FWbuffer, "\n\n")) {
			aquaSetConsoleMessage(FWbuffer);
		}
	}
#else
	/* are we running under Motif or Gtk? */
	if(setTargetMotif == 1) {
		setConsoleMessage (FWbuffer);
	} else {
		if (RUNNINGASPLUGIN) {
			freewrlSystem (FWbuffer);
		} else {
			printf (FWbuffer); if (FWbuffer[strlen(FWbuffer-1)] != '\n') printf ("\n");
		}
	}

#endif
	return count;
}
#endif /*ifdef OLD_CONSOLEMESSAGE_VERSION */

