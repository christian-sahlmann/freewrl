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
        ptr = _X3D_make1StringCommand(MIDIINFO,buf);
        ptr = _X3D_make1StringCommand(MIDIINFO,buf);
}

void sendMIDIControlToFreeWRL(long relativeSamplePos, int bus, int channel, int controller, int value) {
	char *ptr;
	char line[200];
	sprintf (line, "%ld %d %d %d %d",relativeSamplePos, bus, channel, controller, value);
	ptr = _X3D_make1StringCommand(MIDICONTROL,line);
}

