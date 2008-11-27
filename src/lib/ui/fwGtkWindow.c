/*******************************************************************
 Copyright (C) 2006 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

#ifdef HAVE_GTK2

#define ABOUT_FREEWRL "FreeWRL Version %s\n\
FreeWRL is a VRML/X3D Browser for OS X and Unix\n\
\nFreeWRL is maintained by:\nJohn A. Stewart and Sarah J. Dumoulin\n\
\nContact: freewrl-09@rogers.com\n\
Telephone: +1 613-998-2079\nhttp://www.crc.ca/FreeWRL\n\
\nThanks to the Open Source community for all the help received.\n\
Communications Research Centre\n\
Ottawa, Ontario, Canada.\nhttp://www.crc.ca"



#include <headers.h>
#include <vrmlconf.h>
#include "OpenGL_Utils.h"
#include "Viewer.h"
/*
#include <unistd.h>


 #include <stdlib.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <math.h>

 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glx.h>
 #include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
*/

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gtkgl/gtkglarea.h>

GtkWidget *mainWindow;

/************************************************************************

Set window variables from FreeWRL 

************************************************************************/

void createGtkMainWindow() {
	mainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
}

/************************************************************************

Callbacks to handle button presses, etc.

************************************************************************/

/* File pulldown menu */
void createFilePulldown () {
}

/* Navigate pulldown menu */
void createNavigatePulldown() {
}

/* Preferences pulldown menu */
void createPreferencesPulldown() {
}

void createHelpPulldown() {
}

/**********************************/
void createMenuBar(void) {
}

/**********************************************************************************/
/*
	create a frame for FreeWRL, and for messages
*/
void createDrawingFrame(void) {
          const int attribs[] = {GDK_GL_RGBA,
  GDK_GL_RED_SIZE, 1,
  GDK_GL_GREEN_SIZE, 1,
  GDK_GL_BLUE_SIZE, 1,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_DEPTH_SIZE, 1,
  GDK_GL_NONE };
       

  if (!gdk_gl_query()) printf("GL not supported\n");       
  GtkWidget *glwidget = GTK_WIDGET(gtk_gl_area_new(attribs));    
}

void openGtkMainWindow (int argc, char **argv) {
}


void setConsoleMessage (char *str) {
}



void frontendUpdateButtons() {
}

void setMenuButton_collision (int val) {
}
void setMenuButton_headlight (int val) {
}
void setMenuButton_navModes (int type) {
}

void setMenuButton_texSize (int size) {
}

void setMessageBar() {
}

void  getGtkWindowedGLwin(Window *win) {
}

int isGtkDisplayInitialized (void) {
}               
  
#endif
