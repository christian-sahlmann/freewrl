/*
=INSERT_TEMPLATE_HERE=

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



/**********************************************************************************************/
/*                                                                                            */
/* This file is part of the FreeWRL/FreeX3D Distribution, from http://freewrl.sourceforge.net */
/*                                                                                            */
/**********************************************************************************************/

#define MAXEAIHOSTNAME	255		/* length of hostname on command line */
#define EAIREADSIZE	8192		/* maximum we are allowed to read in from socket */
#define EAIBASESOCKET   9877		/* socket number to start at */
#define MIDIPORTOFFSET	5		/* offset for midi EAI port to start at */

#define MAX_SERVICE_CHANNELS 8
/*
 * But please notice that as Jun 2012, only 2 are actually in use
 * CHANNEL_EAI and CHANNEL_MIDI and have proper full definitions.
 */
#define SOCKET_SERVICE_DEFAULT_VERBOSE 0
