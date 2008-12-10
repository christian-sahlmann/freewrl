/*
=INSERT_TEMPLATE_HERE=

$Id: ConsoleMessage.c,v 1.4 2008/12/10 18:46:54 crc_canada Exp $

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

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <syslog.h> //TODO: configure check
#include <stdarg.h> //TODO: configure check

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h"
#include "headers.h"
#include "../plugin/pluginUtils.h"
#include "../plugin/PluginSocket.h"


#define STRING_LENGTH 2000	/* something 'safe'	*/
#define MAXMESSAGES 5 

/* for sending text to the System Console */
static int logFileOpened = FALSE;

static char FWbuffer [STRING_LENGTH];
int consMsgCount = 0;
extern int _fw_browser_plugin;


void closeConsoleMessage() {
	consMsgCount = 0;
	if (logFileOpened) syslog (LOG_ALERT, "FreeWRL loading a new file");
	logFileOpened = FALSE;
}

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


	#ifdef HAVE_MOTIF
	FWbuffer[0] = '\n';
	FWbuffer[1] = '\0';
	#else
	FWbuffer[0] = '\0';
	#endif

	#ifndef HAVE_MOTIF
	/* did we have too many messages - don't want to make this into a 
	   denial of service attack! (thanks, Mufti) */
	#ifdef AQUA
		if (RUNNINGASPLUGIN && consMsgCount > MAXMESSAGES) {
	#else 
		if (consMsgCount > MAXMESSAGES) {
	#endif
		if (consMsgCount > (MAXMESSAGES + 5)) return;
		strcpy(FWbuffer, "Too many freewrl messages - stopping ConsoleMessage");
		consMsgCount = MAXMESSAGES + 100;
	} else {
		consMsgCount++;

	#endif
	
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
					printf("Invalid format specifier in ConsoleMessage().\n");
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
	if (!RUNNINGASPLUGIN) {
		if (strcmp(FWbuffer, "\n") && strcmp(FWbuffer, "\n\n")) {
			aquaSetConsoleMessage(FWbuffer);
		}
	} else {
                char systemBuffer[STRING_LENGTH + 10];
		int i;

		/* remove any newlines; this may have been giving us problems */
		for (i=0; i<strlen(FWbuffer); i++) {
			if (FWbuffer[i] == '\n') { FWbuffer[i] = ' '; }
		}

		/* and call freewrlSystem to speak to the user */
                sprintf(systemBuffer, "%s \"%s\"", FREEX3D_MESSAGE_WRAPPER, FWbuffer);
                freewrlSystem(systemBuffer); 
	}
#else
	/* are we running under Motif or Gtk? */
	#ifndef HAVE_NOTOOLKIT
		setConsoleMessage (FWbuffer);
	#else
		if (RUNNINGASPLUGIN) {
			freewrlSystem (FWbuffer);
		} else {
			printf (FWbuffer); if (FWbuffer[strlen(FWbuffer-1)] != '\n') printf ("\n");
		}
	#endif
#endif
	return count;
}


