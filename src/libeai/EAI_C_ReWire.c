
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

#ifndef REWIRE
#include "config.h"
#include "system.h"
#endif
#include "EAI_C.h"

void _handleReWireCallback (char *buf) {
	int bus, channel, controllerOrVelocity, controlType, value;
	
	if ((*buf) == 'R') buf++; if ((*buf)='W') buf++;
	if (sscanf(buf, "%d %d %d %d %d",&bus,&channel, &controllerOrVelocity, &controlType, &value) != 5) {
		printf ("handleReWireCallback - failed in sscanf\n");
	} else {
		#ifdef MIDI
		sendMIDI (bus,channel,controllerOrVelocity,controlType,value);
		#else
		printf ("handleReWireCallback - data not sent anywhere\n");
		#endif
	}
}

void sendMIDITableToFreeWRL(char *buf) {
	char *ptr;
	/* printf("in sendMIDITableToFreeWRL\n"); */
        ptr = _X3D_make1StringCommand(MIDIINFO,buf);
        ptr = _X3D_make1StringCommand(MIDIINFO,buf);
}

void sendMIDIControlToFreeWRL(long relativeSamplePos, int bus, int channel, int controller, int value) {
	/* printf("in sendMIDIControlToFreeWRL\n"); */
	char *ptr;
	char line[200];
	sprintf (line, "%ld %d %d %d %d",relativeSamplePos, bus, channel, controller, value);
	ptr = _X3D_make1StringCommand(MIDICONTROL,line);
}

