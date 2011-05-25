/*
  $Id: fwWindowAqua.c,v 1.3 2011/05/25 19:26:34 davejoubert Exp $

  FreeWRL support library.
  Aqua specific functions.

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

#include <threads.h>

/*======== "VIRTUAL FUNCTIONS" ==============*/

int fv_open_display()
{
printf ("OSX - fv_open_display called\n");

	/* Guess: We are linked with libAquaInt....
	   display is alread initialized....
	   nothing to do except catch to relevant variables...
	*/
	return TRUE;
}

int create_main_window(int argc, char *argv[])
{

printf ("OSX - create_main_window called\n");

	/* Guess: We are linked with libAquaInt....
	   display is alread initialized....
	   nothing to do except catch to relevant variables...
	*/
	return TRUE;
}

/**
 *   fv_create_GLcontext: create the main OpenGL context.
 *                     TODO: finish implementation for Mac and Windows.
 */
bool fv_create_GLcontext()
{	

printf ("OSX - createGLContext called\n");
	fwl_thread_dump();

	/* Guess: call the aglCreateContext ... */

	return TRUE;
}

/**
 *   fv_bind_GLcontext: attache the OpenGL context to the main window.
 *                   TODO: finish implementation for Mac and Windows.
 */
bool fv_bind_GLcontext()
{

printf ("OSX - bind_GLContext called \n");
	fwl_thread_dump();

	/* Guess: call the aglMakeCurrent ... */

	return TRUE;
}
