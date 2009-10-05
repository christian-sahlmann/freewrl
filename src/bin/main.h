/*
=INSERT_TEMPLATE_HERE=

$Id: main.h,v 1.7 2009/10/05 15:07:23 crc_canada Exp $

FreeWRL/X3D main program.
Internal header: helper macros.

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



#ifndef __FREEWRL_MAIN_H__
#define __FREEWRL_MAIN_H__

/* LOG, WARNING, ERROR macros */

#if defined(FW_DEBUG)
# define DEBUG_(_expr) _expr
#else
# define DEBUG_(...)
#endif

/* To conform C99 ISO C (do not use GCC extension) */
#define DEBUG_MSG(...) DEBUG_(fprintf(stdout, __VA_ARGS__))
#define TRACE_MSG(...) DEBUG_(fprintf(stdout, __VA_ARGS__))
#define WARN_MSG(...)  DEBUG_(fprintf(stdout, __VA_ARGS__))
#define ERROR_MSG(...) DEBUG_(fprintf(stderr, __VA_ARGS__))

#if defined(_MSC_VER)
extern int optind;
#endif


#endif /* __FREEWRL_MAIN_H__ */
