/*
=INSERT_TEMPLATE_HERE=

$Id: InputFunctions.h,v 1.8 2010/01/27 21:18:52 crc_canada Exp $

Global includes.

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


#ifndef __FREEWRL_INPUTFUNCTIONS_H__
#define __FREEWRL_INPUTFUNCTIONS_H__

/**
 * in InputFunctions.c
 */
char* makeFontDirectory();
FILE *openLocalFile (char *fn, char* access);
char * stripLocalFileName (char * origName);


#endif /* __FREEWRL_INPUTFUNCTIONS_H__ */

