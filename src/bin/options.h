/*
  $Id: options.h,v 1.5 2011/04/09 00:33:19 davejoubert Exp $

  FreeWRL command line arguments.

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



#ifndef __FREEWRL_MAIN_OPTIONS_H__
#define __FREEWRL_MAIN_OPTIONS_H__


extern int fv_parseCommandLine (int argc, char **argv);
void fv_parseEnvVars(void);

extern void fv_setGeometry_from_cmdline(const char *gstring); /* See lib/display.c : scan command line arguments (X11 convention) */


#endif /* __FREEWRL_MAIN_OPTIONS_H__ */
