/*
  $Id: statusbarEx.c,v 1.1 2009/12/06 22:18:48 dug9 Exp $

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
#include "main/headers.h"
#include "vrml_parser/Structs.h"
#include "scenegraph/Viewer.h"



/* shortcuts */
/*
EXAMINE Mode
Mouse Button 1 controls rotation; left/right mouse movement controls rotation around Y axis; up/down movement controls movement around X axis. 

Mouse Button 3 zooms model in/out. On Apple computers with one button mice, press and hold the "control" key, and use your mouse. 
WALK Mode
Mouse Button 1 Controls movement; moving the mouse up/down makes you walk forward/backward; moving the mouse left/right turns you left/right in the model. 

Mouse Button 3 moves you up/down (changes your height above the ground). On Apple computers with one button mice, press and hold the "control" key, and use your mouse. 
FLY Mode
This is all keyboard based. It gives infinite control over your movement in a 3D world; it does take some time to get used to, but once mastered, it is a pleasure to use. 
There are two keymappings: one from the game Descent, using keys on the keyboard, and (currently on Linux only), a mapping using the keypad.

All motion is controlled by 12 keys, two for each of your 6 degrees of freedom (3 translations, 3 rotations).

The keymap in this mode is:

'8'/'k' (or, 'Keypad Down'/Keypad Up' on linux systems) rotation down/up 

'u'/'o' (or, 'Keypad Left'/Keypad Right' on linux systems) rotation left/right 

'7'/'9' (or, 'Keypad 7'/Keypad 9' on linux systems) rotation about the Z axis 

'a'/'z' (or, 'Keypad 0'/Keypad %' on linux systems) translation forwards/backwards 

'j'/'l' (or, 'Keypad arrow left'/Keypad arrow right' on linux systems) translation left/right 

'p'/';' (or, 'Keypad arrow up'/Keypad arrow down' on linux systems) translation up/down 
*/
/*
char *shortcuts[][2] =
{
	{"d","Switch to the Fly (Keyboard input) navigation mode"},
	{"f","Switch to the Fly (External Sensor input) navigation mode"},
	{"e","Switch to the Examine navigation mode"}, 
	{"w","Switch to the Walk navigation mode"}, 
	{"v","Go to the next viewpoint in the scene"}, 
	{"b","Go to the previous viewpoint in the scene"}, 
	{"/","Print out the position and orientation of the current viewpoint in the current Viewpoint node's local coordinate system."}, 
	{"h","(or, NumLock on linux keypad) Toggle the headlight on or off."}, 
	{"c","Toggle collision detection on or off."}, 
	{"q","Quit the browser"}
};
*/
char *shortcutsRaw[][3] =
{
	{"e"," ","Examine"}, 
	{"w"," ","Walk"}, 
	{"f"," ","Fly External"},
	{"d"," ","Fly Keyboard"},
	{"v"," ","next viewpoint"}, 
	{"b"," ","previous viewpoint"}, 
	{"/"," ","Print viewpoint pose"}, 
	{"h"," ","headlight  or linux numlock"}, 
	{"c"," ","collision detection"}, 
	{"q"," ","Quit"}
};
static char*** shortcuts = NULL;
static int nshort = 0;
void initShortcuts()
{
	/* I copy it from static data into heap so I can change it, change flags etc */
	int i,j,n;
	char* p;
	n = sizeof(shortcutsRaw)/sizeof(char*[3]);
	shortcuts = (char ***)malloc(n * sizeof(char**));
	for(i=0;i<n;i++)
	{
	  shortcuts[i] = (char **)malloc(sizeof(char*)*3);
	  for(j=0;j<3;j++)
	  {
		shortcuts[i][j] = (char*) malloc(strlen(shortcutsRaw[i][j])+1);
		memcpy(shortcuts[i][j],shortcutsRaw[i][j],strlen(shortcutsRaw[i][j])+1);
	  }
	}
	nshort = n;
}
void setMenuButton_collision(int val)
{
	if(shortcuts)
		if(val)
			shortcuts[8][1] = "X";
		else
			shortcuts[8][1] = " ";
}
void setMenuButton_texSize(int size){}
void setMenuButton_headlight(int val)
{
	if(shortcuts)
		if(val)
			shortcuts[7][1] = "X";
		else
			shortcuts[7][1] = " ";

}
void setMenuButton_navModes(int type)
{
	int i;
	if(shortcuts)
		if(type > 0 && type < 5)
		{
			for(i=0;i<4;i++)
			{
				shortcuts[i][1] = " ";
			}
			shortcuts[type-1][1] = "X";
		}
	//printf("new nav mode=%d\n",type);
}


/* raster font drawing taken from redbook p.311 / fonts.c in www.opengl-redbook.com/code */
/* 
by using Calculator, plugging in the hex, converting to bin, and clicking pixels in paint, 
I have determined each character has 13 pixel rows, starting at the bottom, 2 (empty) rows below the 
capital letter baseline, and 8 pixels wide, which does not including inter-character spacing.
*/
GLubyte space[] = 
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
GLubyte fill[] = 
{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
GLubyte slash[] = 
{0x00, 0x00, 0xc0, 0xe0, 0x70, 0x38, 0x1C, 0x0e, 0x07, 0x03, 0x01, 0x00, 0x00};

GLubyte letters[][13] = {
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18}, 
{0x00, 0x00, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe}, 
{0x00, 0x00, 0x7e, 0xe7, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e}, 
{0x00, 0x00, 0xfc, 0xce, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xce, 0xfc}, 
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff}, 
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xff}, 
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xcf, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e}, 
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e}, 
{0x00, 0x00, 0x7c, 0xee, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, 
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc3}, 
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0}, 
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xff, 0xe7, 0xc3}, 
{0x00, 0x00, 0xc7, 0xc7, 0xcf, 0xcf, 0xdf, 0xdb, 0xfb, 0xf3, 0xf3, 0xe3, 0xe3}, 
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e}, 
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe}, 
{0x00, 0x00, 0x3f, 0x6e, 0xdf, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c}, 
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe}, 
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0xe0, 0xc0, 0xc0, 0xe7, 0x7e}, 
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff}, 
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
{0x00, 0x00, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3}, 
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3}, 
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x7e, 0x0c, 0x06, 0x03, 0x03, 0xff}
};

GLuint fontOffset;

void makeRasterFont(void)
{
   GLuint i, j;
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   fontOffset = glGenLists (128);
   for (i = 0,j = 'a'; i < 26; i++,j++) {
      glNewList(fontOffset + j, GL_COMPILE);
      glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, letters[i]);
      glEndList();
   }
   for (i = 0,j = 'A'; i < 26; i++,j++) {
      glNewList(fontOffset + j, GL_COMPILE);
      glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, letters[i]);
      glEndList();
   }
   glNewList(fontOffset + ' ', GL_COMPILE);
   glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, space);
   glEndList();
   glNewList(fontOffset + '/', GL_COMPILE);
   glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, slash);
   glEndList();
}

static int fontInitialized = 0;
void initFont(void)
{
   glShadeModel (GL_FLAT);
   makeRasterFont();
   fontInitialized = 1;
}

void printString(char *s)
{
   glPushAttrib (GL_LIST_BIT);
   glListBase(fontOffset);
   glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *) s);
   glPopAttrib ();
}

static int sb_hasString = FALSE;
static struct Uni_String *myline;

static char buffer[200];
void render_init(void);

#define STATUS_LEN 2000

/* make sure that on a re-load that we re-init */
void kill_status (void) {
	/* hopefully, by this time, rendering has been stopped */
	sb_hasString = FALSE;
}


/* trigger a update */
void update_status(char* msg) {

	if (msg==NULL) sb_hasString = FALSE;
	else {
		sb_hasString = TRUE;
		strcpy (buffer,msg);
	}
}
void displayShortcuts()
{
	int i,n;
	char* p;
	if(!shortcuts) initShortcuts();
	n = nshort; //sizeof(shortcuts)/sizeof(char*[3]);
	for(i=0;i<n;i++)
	{
		//draw shortcut key
		glWindowPos2i(5,screenHeight -15*(i+2));
		p = shortcuts[i][0];
		printString(p); 
		//draw on/off flag
		glWindowPos2i(22,screenHeight -15*(i+2));
		p = shortcuts[i][1];
		printString(p); 
		//draw text description
		glWindowPos2i(5+35,screenHeight -15*(i+2));
		p = shortcuts[i][2];
		printString(p); 
	}
}

extern int currentX, currentY;
static int followCursor = 0;
static int showShortcuts = 1;
void drawStatusBar() 
{
	char *p; 
	int xvp;
	float c[4];
	int ic[4];
	if (!sb_hasString && !showShortcuts) {
		return;
	}
	if(!fontInitialized) initFont();
	xvp = 0;
	if( Viewer.iside == 1) xvp = screenWidth/2.0;

	// you must call drawStatusBar() from render() just before swapbuffers 
	glDepthMask(FALSE);
	glDisable(GL_DEPTH_TEST);
	//glWindowPos seems to set the bitmap color correctly in windows
	if(sb_hasString)
	{
		glColor3f(1.0f,1.0f,1.0f);
		glWindowPos2i(xvp+5,screenHeight -15);
		p = buffer;
		printString(p); 
	}
	//glFlush();

	if(showShortcuts)
	{
		//dim scene
		float rgba[4];
		rgba[0] = 0.0f; rgba[1] = 0.0f; rgba[2] = 0.0f, rgba[3] = .35f;
		glWindowPos2i(0,0); 
		glPixelZoom((float)screenWidth,(float)screenHeight);
		glDrawPixels(1,1,GL_RGBA,GL_FLOAT,rgba);

		//draw highlight under mouse
		if(0)
		{
		glColor4f(.5f,.5f,.5f,.75f);
		glWindowPos2i(currentX,screenHeight - currentY); 
		glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, fill);
		}
		rgba[0] = .5f; rgba[1] = .5f; rgba[2] = .5f, rgba[3] = .75f;
		//glWindowPos2i(currentX,screenHeight - currentY);
		glWindowPos2i(0,screenHeight - (currentY/15 +1)*15);
		glPixelZoom(screenWidth,13.0f);
		glDrawPixels(1,1,GL_RGBA,GL_FLOAT,rgba);

		//restore
		glPixelZoom(1.0f,1.0f);

		//draw text
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		displayShortcuts();
	}

	glDepthMask(TRUE);
	glEnable(GL_DEPTH_TEST);
	glFlush();

}
