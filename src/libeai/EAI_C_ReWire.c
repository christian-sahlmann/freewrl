
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
#ifdef OLDCODE
OLDCODE#ifndef REWIRE
OLDCODE#include "config.h"
OLDCODE#include "system.h"
OLDCODE#endif
OLDCODE#include "EAI_C.h"
OLDCODE
OLDCODEvoid _handleReWireCallback (char *buf) {
OLDCODE	int bus, channel, controllerOrVelocity, controlType, value;
OLDCODE	
OLDCODE	if ((*buf) == 'R') buf++; if ((*buf) == 'W') buf++;
OLDCODE	if (sscanf(buf, "%d %d %d %d %d",&bus,&channel, &controllerOrVelocity, &controlType, &value) != 5) {
OLDCODE		printf ("handleReWireCallback - failed in sscanf\n");
OLDCODE	} else {
OLDCODE		#ifdef MIDI
OLDCODE		sendMIDI (bus,channel,controllerOrVelocity,controlType,value);
OLDCODE		#else
OLDCODE		printf ("handleReWireCallback - data not sent anywhere\n");
OLDCODE		#endif
OLDCODE	}
OLDCODE}
OLDCODE
OLDCODEvoid sendMIDITableToFreeWRL(char *buf) {
OLDCODE	char *ptr;
OLDCODE	/* printf("in sendMIDITableToFreeWRL\n"); */
OLDCODE        ptr = _X3D_make1StringCommand(MIDIINFO,buf);
OLDCODE        ptr = _X3D_make1StringCommand(MIDIINFO,buf);
OLDCODE}
OLDCODE
OLDCODEvoid sendMIDIControlToFreeWRL(long relativeSamplePos, int bus, int channel, int controller, int value) {
OLDCODE	/* printf("in sendMIDIControlToFreeWRL\n"); */
OLDCODE	char *ptr;
OLDCODE	char line[200];
OLDCODE	sprintf (line, "%ld %d %d %d %d",relativeSamplePos, bus, channel, controller, value);
OLDCODE	ptr = _X3D_make1StringCommand(MIDICONTROL,line);
OLDCODE}
OLDCODE
#endif // OLDCODE
