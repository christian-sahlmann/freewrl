/*******************************************************************
 *
 * FreeWRL support library
 *
 * main.c
 *
 * $Id: main.c,v 1.11 2009/10/05 15:07:23 crc_canada Exp $
 *
 *******************************************************************/


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


/**
 * General variables
 */

/**
 * library initialization
 */
void __attribute__ ((constructor)) libFreeWRL_init(void)
{
}

/**
 * library exit routine
 */
void __attribute__ ((destructor)) libFreeWRL_fini(void)
{
}

/**
 * Explicit initialization
 */
int initFreeWRL()
{
    if (!display_initialize()) {
	ERROR_MSG("error in initialization.\n");
	return FALSE;
    }
    return TRUE;
}

/**
 * Explicit exit routine
 */
void closeFreeWRL()
{
}
