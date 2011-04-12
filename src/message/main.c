
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


/* put a string on the console. This is fork'd by FreeWRL for error messages,
 * because when running within an HTML browser, error messages to the command
 * line get lost.
 */

/* our defines for this build */
#  include <config.h>

#if !defined(TARGET_AQUA)
#include "system.h"

#define MESG "something strange happened with\nthe FreeWRL console..."
#define WINTITLE "FreeWRL X3D/VRML Console:"
#define MINTITLE "FreeWRL Console:"
#define MAXLINE 2000

/* Define an application context */
XtAppContext app_context;

/* Define the widgets */
Widget top, mainBox, messageText, dismissButton;

char inLine[MAXLINE];

/* what to do when the user hits the dismiss button */
void dismiss_proc (Widget w, XtPointer client_data, XtPointer call_data) {
	XtDestroyApplicationContext(app_context);
	exit (0);
}

int main(int argc, char **argv) {

	if (argc > 1) {
		strncpy (inLine,argv[1],strlen(argv[1]));
	} else {
		strncpy (inLine,MESG,strlen(MESG));
	}

	/* Create the application shell widget */
	top = XtVaAppInitialize(&app_context, "XViewfile", NULL, 0, &argc, argv, NULL,
		XtNtitle,WINTITLE,
		XtNiconName,MINTITLE,
		NULL);

	/* Create a box to hold everything */
	mainBox = XtVaCreateManagedWidget("mainBox", boxWidgetClass, top,
			NULL);

	/* Create a read only, scrollable text widget, to display the text */
	messageText = XtVaCreateManagedWidget("messageText", asciiTextWidgetClass,
		mainBox,
		XtNheight, 100,
		XtNwidth, 400,
		XtNtype, XawAsciiString,
		XtNscrollVertical, XawtextScrollAlways,
		XtNscrollHorizontal, XawtextScrollWhenNeeded,
		XtNstring, inLine,
		XtNlength, strlen(inLine),
		NULL);


	/* Create a file button and file menu, with open & dismiss selections */
	dismissButton = XtVaCreateManagedWidget("dismiss_button", commandWidgetClass,
	       mainBox, XtNlabel, "Dismiss", NULL);


	/* Tie in the callbacks */
	XtAddCallback(dismissButton,XtNcallback, dismiss_proc, NULL);
	XtRealizeWidget(top);
	XtAppMainLoop(app_context);
	return 0;
}
#endif
