/*******************************************************************
 *
 * FreeWRL support library
 *
 * display_aqua.c
 *
 * $Id: display_aqua.c,v 1.4 2009/10/01 19:35:36 crc_canada Exp $
 *
 *******************************************************************/


/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/



#include "config.h"
#include "system.h"
#include "display.h"
#include "internal.h"

/* display part specific to Mac */

CGLContextObj myglobalContext;
AGLContext aqglobalContext;

GLboolean cErr;

GDHandle gGDevice;

int ccurse = ACURSE;
int ocurse = ACURSE;

/* for handling Safari window changes at the top of the display event loop */
int PaneClipnpx;
int PaneClipnpy;
WindowPtr PaneClipfwWindow;
int PaneClipct;
int PaneClipcb;
int PaneClipcr;
int PaneClipcl;
int PaneClipwidth;
int PaneClipheight;
int PaneClipChanged = FALSE;

int create_main_window_aqua()
{
    return FALSE;
}
