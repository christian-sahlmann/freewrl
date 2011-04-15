/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Networking.c,v 1.33 2011/04/15 15:02:13 crc_canada Exp $

X3D Networking Component

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

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"

#include "../input/EAIHeaders.h"
#include "../input/EAIHelpers.h"
#include "../opengl/Frustum.h"
#include "../opengl/Textures.h"

#include "Component_Networking.h"
#include "Children.h"

#include <libFreeWRL.h>
#include <list.h>
#include <resources.h>
#include <io_http.h>
#ifdef WANT_OSC
	#include <lo/lo.h>
	#include "ringbuf.h"
	#define USE_OSC 1
	#define TRACK_OSC_MSG 0
#else
	#define USE_OSC 0
#endif

#if USE_OSC
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/JScript.h"
#include "../world_script/CScripts.h"
#include "../world_script/fieldSet.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CProto.h"
#include "../vrml_parser/CParse.h"
#endif


/* keep track of the Midi nodes. */
static uintptr_t *MidiNodes = NULL;
static int num_MidiNodes = 0;

#define BUTTON_PRESS_STRING "use_for_buttonPresses"

struct ReWireNamenameStruct *ReWireNamenames = 0;
int ReWireNametableSize = -1;
static int MAXReWireNameNames = 0;

struct ReWireDeviceStruct *ReWireDevices = 0;
int ReWireDevicetableSize = -1;
int MAXReWireDevices = 0;

#if USE_OSC
/**************** START OF OSC node **************************/
/* DJTRACK_OSCSENSORS */

void error(int num, const char *m, const char *path);
void utilOSCcounts(char *types , int *intCount, int *fltCount, int *strCount, int *blobCount, int *midiCount, int *otherCount);

/* We actually want to keep this one inline, as it offers ease of editing with minimal infrastructure */
#include "OSCcallbacks.c"

int serverCount=0;
#define MAX_OSC_SERVERS 32
int serverPort[MAX_OSC_SERVERS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
lo_server_thread oscThread[MAX_OSC_SERVERS] ;

static uintptr_t *OSC_Nodes = NULL;
static int num_OSC_Nodes = 0;
int curr_OSC_Node = 0;
int active_OSC_Nodes = FALSE;

void utilOSCcounts(char *types , int *intCount, int *fltCount, int *strCount, int *blobCount, int *midiCount, int *otherCount) {
	*intCount	= 0;
	*fltCount	= 0;
	*strCount	= 0;
	*blobCount	= 0;
	*midiCount	= 0;
	*otherCount	= 0;

	int i,j;

	j=strlen(types) ;
	/* what amount of storage */
	for (i=0 ; i < j ; i++) {
		switch (types[i]) {
		case 'i':
			(*intCount)++;
			break;
		case 'f':
			(*fltCount)++;
			break;
		case 's':
			(*strCount)++;
			break;
		case 'b':
			(*blobCount)++;
			break;
		case 'm':
			(*midiCount)++;
			break;
		default:
			(*otherCount)++;
			break;
		}
	}
}

void activate_OSCsensors() {
	curr_OSC_Node = 0 ;
	active_OSC_Nodes = TRUE ;
	struct X3D_OSC_Sensor *realnode ;
	char buf[32];
	int i ;
	/* what amount of storage */
	int fltCount;
	int intCount;
	int strCount;
	int blobCount;
	int midiCount;
	int otherCount;

	while (active_OSC_Nodes && curr_OSC_Node < num_OSC_Nodes) {
		realnode = (struct X3D_OSC_Sensor *) OSC_Nodes[curr_OSC_Node] ;
		#if TRACK_OSC_MSG
		printf("activate_OSCsensors : %s,%d node=%p name=%s\n", __FILE__,__LINE__,realnode,realnode->description->strptr) ;
		#endif
		if (realnode->_status < 0) {
			printf("activate_OSCsensors : %s,%d Moving %s to ready.\n", __FILE__,__LINE__,realnode->description->strptr) ;
			realnode->_status = 0 ;
		} else if (realnode->_status == 0) {
			printf("activate_OSCsensors : %s,%d\n", __FILE__,__LINE__) ;
			printf("activate_OSCsensors : enabled=%d\n",	realnode->enabled) ;
			printf("activate_OSCsensors : gotEvents=%d\n",	realnode->gotEvents) ;
			printf("activate_OSCsensors : description=%s\n",realnode->description->strptr) ;
			printf("activate_OSCsensors : protocol=%s\n",	realnode->protocol->strptr) ;
			printf("activate_OSCsensors : port=%d\n",	realnode->port) ;
			printf("activate_OSCsensors : filter=%s\n",	realnode->filter->strptr) ;
			printf("activate_OSCsensors : handler=%s\n",	realnode->handler->strptr) ;
/*
11715                     if(allFields) {
11716                         spacer fprintf (fp,"\t_talkToNodes (MFNode):\n");
11717                         for (i=0; i<tmp->_talkToNodes.n; i++) { dump_scene(fp,level+1,tmp->_talkToNodes.p[i]); }
11718                     }
11719                     if(allFields) {
11720                         spacer fprintf (fp,"\t_status (SFInt32) \t%d\n",tmp->_status);
11721                     }
11722                     if(allFields) {
11723                         spacer fprintf (fp,"\t_floatInpFIFO (SFNode):\n"); dump_scene(fp,level+1,tmp->_floatInpFIFO);
11724                     }
11725                     if(allFields) {
11726                         spacer fprintf (fp,"\t_int32OutFIFO (SFNode):\n"); dump_scene(fp,level+1,tmp->_int32OutFIFO);
11727                     }
11728                         spacer fprintf (fp,"\ttalksTo (MFString): \n");
11729                         for (i=0; i<tmp->talksTo.n; i++) { spacer fprintf (fp,"                 %d: \t%s\n",i,tmp->talksTo.p[i]->strptr); }
*/
			printf("activate_OSCsensors : talksTo=[ ");
			for (i=0; i < realnode->talksTo.n; i++) {
				printf("\"%s\" ",realnode->talksTo.p[i]->strptr);
				/* This would be a good time to convert the name into an entry in _talkToNodes */
				struct X3D_Node * myNode;
				/* myNode = X3DParser_getNodeFromName(realnode->talksTo.p[i]->strptr); */
				myNode = parser_getNodeFromName(realnode->talksTo.p[i]->strptr);
				if (myNode != NULL) {
					printf("(%p) ",(void *)myNode);
				} else {
					printf("(..) ");
				}
			}
			printf("] (%d nodes) (Need to fix %s,%d)\n",realnode->talksTo.n , __FILE__,__LINE__);
			printf("activate_OSCsensors : listenfor=%s , expect %d parameters\n", realnode->listenfor->strptr , (int)strlen(realnode->listenfor->strptr)) ;
			printf("activate_OSCsensors : FIFOsize=%d\n",	realnode->FIFOsize) ;
			printf("activate_OSCsensors : _status=%d\n",	realnode->_status) ;

			if (realnode->FIFOsize > 0) {
				/* what amount of storage */
				utilOSCcounts(realnode->listenfor->strptr,&intCount,&fltCount,&strCount,&blobCount,&midiCount,&otherCount);
				intCount = realnode->FIFOsize * (intCount + midiCount); 
				fltCount = realnode->FIFOsize * fltCount;
				strCount = realnode->FIFOsize * (strCount + blobCount + otherCount); 
				printf("Allocate %d floats, %d ints for '%s'\n",fltCount,intCount,realnode->description->strptr);

				realnode->_int32InpFIFO = (void *) NewRingBuffer (intCount) ;
				realnode->_floatInpFIFO = (void *) NewRingBuffer (fltCount) ;
				realnode->_stringInpFIFO = (void *) NewRingBuffer (strCount) ;

			}

			/* start a new server on the required port */
			int foundCurrentPort = -1 ;
			for ( i=0 ; i < num_OSC_Nodes ; i++) {
				if(realnode->port == serverPort[i]) {
					foundCurrentPort=i;
					i = num_OSC_Nodes+1;
				}
			}
			if (foundCurrentPort < 0) {
				foundCurrentPort = serverCount ;
				serverPort[foundCurrentPort] = realnode->port ;
				serverCount++ ;

				sprintf (buf,"%d",realnode->port);

				if (strcmp("TCP",realnode->protocol->strptr)==0) {
					/* oscThread[foundCurrentPort] = lo_server_thread_new_with_proto(buf, LO_TCP, error); */
					oscThread[foundCurrentPort] = lo_server_thread_new(buf, error);
				} else if (strcmp("UNIX",realnode->protocol->strptr)==0) {
					/* oscThread[foundCurrentPort] = lo_server_thread_new_with_proto(buf, LO_UNIX, error); */
					oscThread[foundCurrentPort] = lo_server_thread_new(buf, error);
				} else {
					/* oscThread[foundCurrentPort] = lo_server_thread_new_with_proto(buf, LO_UDP, error); */
					oscThread[foundCurrentPort] = lo_server_thread_new(buf, error);
				}
				lo_server_thread_start(oscThread[foundCurrentPort]);
			}

			printf("%d servers; current server is running in slot %d on port %d\n",serverCount,foundCurrentPort,serverPort[foundCurrentPort]) ;

			/* add method (by looking up its name) that will in future use the required path */
			/* need to re-read the lo code to check that you can register the same callback twice (or more) with different paths) */
			/* See OSCcallbacks.c */
			int foundHandler = 0 ;
			for (i=0 ; i < OSCfuncCount ; i++) {
				printf("%d/%d : Check %s against %s\n",i,OSCfuncCount,realnode->handler->strptr,OSCfuncNames[i]);
				if (0 == strcmp(realnode->handler->strptr,OSCfuncNames[i])) {foundHandler = i;}
			}
			if (OSCcallbacks[foundHandler] != NULL) {
				printf("Going to hook '%s' to '%s' handler\n",realnode->description->strptr ,OSCfuncNames[foundHandler]) ;
				lo_server_thread_add_method(oscThread[foundCurrentPort], realnode->filter->strptr, realnode->listenfor->strptr,
					(OSCcallbacks[foundHandler]), realnode);
			}

			realnode->_status = 1 ;
			/* We only want one OSC node to become active in one slowtick */
			active_OSC_Nodes = FALSE ;
		} 
		curr_OSC_Node++;
	}
}

void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
}

void add_OSCsensor(struct X3D_Node * node) {
	uintptr_t *myptr;

	if (node == 0) {
		printf ("error in registerOSCNode; somehow the node datastructure is zero \n");
		return;
	}

	if (node->_nodeType != NODE_OSC_Sensor) return;

	OSC_Nodes = (uintptr_t *) REALLOC (OSC_Nodes,sizeof (uintptr_t *) * (num_OSC_Nodes+1));
	myptr = OSC_Nodes;

	/* now, put the node pointer into the structure entry */
	*myptr = (uintptr_t) node;

	num_OSC_Nodes++;
}
/***************** END OF OSC node ***************************/
#else
void add_OSCsensor(struct X3D_Node * node) {}
#endif
/************ START OF MIDI CONTROL **************************/

/* go through the node, and create a new int value, and a new float value, from an int value */
static void determineMIDIValFromFloat (struct X3D_MidiControl *node, int *value, float *fV) {
	int minV, maxV, possibleValueSpread;
	float tv;

	/* find the int value range possible */
	minV = 0;  maxV = 100000;
	if (minV < node->deviceMinVal) minV = node->deviceMinVal;
	if (minV < node->minVal) minV = node->minVal;
	if (maxV > node->deviceMaxVal) maxV = node->deviceMaxVal;
	if (maxV > node->maxVal) maxV = node->maxVal;
	possibleValueSpread = maxV-minV +1;

	/* bounds check the float value */
	if (*fV<(float)0.0) *fV=(float)0.0;
	if (*fV>(float)1.0) *fV=(float)1.0;
	tv = *fV * possibleValueSpread + minV;
	*value = (int) tv;

	#ifdef MIDIVERBOSE
	printf ("determinefromfloat, minV %d maxV %d, val %d fv %f tv %f\n",minV, maxV,*value, *fV,tv);
	#endif

}

/* go through the node, and create a new int value, and a new float value, from an int value */
static void determineMIDIValFromInt (struct X3D_MidiControl *node, int *value, float *fV) {
	int minV, maxV, possibleValueSpread;

	minV = 0;  maxV = 100000;
	if (minV < node->deviceMinVal) minV = node->deviceMinVal;
	if (minV < node->minVal) minV = node->minVal;
	if (maxV > node->deviceMaxVal) maxV = node->deviceMaxVal;
	if (maxV > node->maxVal) maxV = node->maxVal;
		
	possibleValueSpread = maxV-minV +1;
	if (*value < minV) *value = minV;
	if (*value > maxV) *value = maxV;
	if (possibleValueSpread != 0) {
		*fV = ((float) *value + minV) / (possibleValueSpread);
	} else { 
		*fV = (float) 0.0;
	}
	#ifdef MIDIVERBOSE
	printf ("determine, minV %d maxV %d, val %d fv %f\n",minV, maxV,*value, *fV);
	#endif
	
}


/* send this node out to the ReWire network */
static void sendNodeToReWire(struct X3D_MidiControl *node) {
	char buf[200];

	#ifdef MIDIVERBOSE
	if (node->controllerPresent) {
		printf ("sendNodeToReWire - controller present\n");
	} else {
		printf ("sendNodeToReWire - controller NOT present\n");
	}
	#endif


	if (node->controllerPresent) {
		if (node->_intControllerType == MIDI_CONTROLLER_FADER) {
			sprintf (buf,"RW %d %d %d %d %d\n",
				node->_bus, node->_channel, node->_controller, 
				node->_intControllerType, node->intValue);
				#ifdef MIDIVERBOSE
				printf ("sendNodeToReWire,  fader: bus: %d channel: %d controller: %d type: %d value: %d\n",
                                node->_bus, node->_channel, node->_controller,
                                node->_intControllerType, node->intValue);
				#endif
		} else {
			sprintf (buf,"RW %d %d %d %d %d\n",
				node->_bus, node->_channel, node->_sentVel,
				node->_intControllerType, node->intValue);
				#ifdef MIDIVERBOSE
				printf ("sendNodeToReWire,  buttonPress: bus: %d channel: %d vel: %d conttype: %d val: %d\n",
				node->_bus, node->_channel, node->_sentVel,
				node->_intControllerType, node->intValue);
				#endif
		}

		#ifdef MIDIVERBOSE
		printf ("sendNodeToReWire: sending string:%s:\n",buf);
		#endif

		EAI_send_string(buf,EAIMIDIlistenfd);
	}	
}

/* return parameters associated with this name. returns TRUE if this device has been added by
the ReWire system */

static int ReWireDeviceIndex (struct X3D_MidiControl *node, int *bus, int *internChan,  
	int *controller, int *cmin, int *cmax, int *ctptr) {
	int ctr;
	int match;
	int dev = node->_deviceNameIndex;
	int cont = node->_controllerIndex;

	#ifdef MIDIVERBOSE
	printf ("ReWireDeviceIndex, tblsz %d, looking for a device for bus %d channel %d and controller %d\n",
			ReWireDevicetableSize, *bus,node->channel,*controller);
	#endif
	
	/* is this a duplicate name and type? types have to be same,
	   name lengths have to be the same, and the strings have to be the same.
	*/
	for (ctr=0; ctr<=ReWireDevicetableSize; ctr++) {
		#ifdef MIDIVERBOSE
			printf ("ReWireDeviceIndex: comparing %d %d to %d %d\n",dev,ReWireDevices[ctr].encodedDeviceName,
			cont, ReWireDevices[ctr].encodedControllerName); 
		#endif

		/* if the MidiControl node is set for "ButtonPress" we care ONLY about whether the
		   device is present or not - ignore the encodedControllerName field */
		/* possible match? */
		if ((dev == ReWireDevices[ctr].encodedDeviceName) && (cont == ReWireDevices[ctr].encodedControllerName)) {
			#ifdef MIDIVERBOSE
			printf ("ReWireDeviceIndex, possible match; dev, cont == ReWireDevices[]encodedDevice, encodedControl\n");
			printf (" for ReWireDevices[%d]: bus %d node->channel %d comparing to our requested channel %d\n",ctr,ReWireDevices[ctr].bus,ReWireDevices[ctr].channel,node->channel);
			#endif
#undef MIDIVERBOSE

			match = FALSE;

			/* ok - so if the user asked for a specific channel, look for this, if not, make first match */
			/* NOTE that MIDI USER DEVICES use channels 1-16, but the MIDI SPEC says 0-15 */

			if ((node->channel >= 1) && (node->channel<=16)) {
				#ifdef MIDIVERBOSE
				printf ("user specified channel is %d, we map it to %d\n",node->channel, node->channel-1);
				#endif

				if (((node->channel)-1)  == ReWireDevices[ctr].channel) {
					match = TRUE;
				}
			} else {
				match = TRUE;
			}

			if (match) {

				#ifdef MIDIVERBOSE
				printf ("MATCHED!\n");
				#endif

				*bus = ReWireDevices[ctr].bus;
				*internChan = ReWireDevices[ctr].channel;
				*controller = ReWireDevices[ctr].controller;
				*cmin = ReWireDevices[ctr].cmin;
				*cmax = ReWireDevices[ctr].cmax;
				/* we know the ctype from the x3d file... 
				     *ctptr = ReWireDevices[ctr].ctype; */
	
				/* is this the first time this node is associated with this entry? */
				if (ReWireDevices[ctr].node == NULL) {
					ReWireDevices[ctr].node = node;
					#ifdef MIDIVERBOSE
					printf ("ReWireDeviceIndex, tying node %d (%s) to index %d\n",node, stringNodeType(node->_nodeType),ctr);
					#endif
				}

				return TRUE;
			} else {
				#ifdef MIDIVERBOSE
				printf ("did not match...\n");
				#endif
			}

		}
	}

	/* nope, not duplicate */
	#ifdef MIDIVERVOSE
	printf ("ReWireDeviceIndex: name not added, name not found\n");
	#endif
	return FALSE; /* name not added, name not found */ 
}

/* returns TRUE if register goes ok, FALSE if already registered */
static int ReWireDeviceRegister (int dev, int cont, int bus, int channel, int controller, int cmin, int cmax, int ctptr) {
	int ctr;
	
	/* is this a duplicate name and type? types have to be same,
	   name lengths have to be the same, and the strings have to be the same.
	*/
	for (ctr=0; ctr<=ReWireDevicetableSize; ctr++) {
		#ifdef MIDIVERBOSE
			printf ("comparing %d %d to %d %d\n",dev,ReWireDevices[ctr].encodedDeviceName,
			cont, ReWireDevices[ctr].encodedControllerName); 
		#endif

		if ((dev==ReWireDevices[ctr].encodedDeviceName) &&
			(channel == ReWireDevices[ctr].channel) &&
			(cont == ReWireDevices[ctr].encodedControllerName)) {
			#ifdef MIDIVERBOSE
				printf ("ReWireDeviceRegister, FOUND IT at %d bus %d channel %d controller %d\n",ctr,
						ReWireDevices[ctr].bus,ReWireDevices[ctr].channel,ReWireDevices[ctr].controller);
			#endif
			return FALSE; /* name found */
		}
	}

	/* nope, not duplicate */
	ReWireDevicetableSize ++;
	
	/* ok, we got a name and a type */
	if (ReWireDevicetableSize >= MAXReWireDevices) {
		/* oooh! not enough room at the table */
		MAXReWireDevices += 1024; /* arbitrary number */
		ReWireDevices = (struct ReWireDeviceStruct*)REALLOC (ReWireDevices, sizeof(*ReWireDevices) * MAXReWireDevices);
	}
	
	ReWireDevices[ReWireDevicetableSize].bus = bus;
	ReWireDevices[ReWireDevicetableSize].channel = channel;
	ReWireDevices[ReWireDevicetableSize].controller = controller;
	ReWireDevices[ReWireDevicetableSize].cmin = cmin; 
	ReWireDevices[ReWireDevicetableSize].cmax = cmax;
	ReWireDevices[ReWireDevicetableSize].ctype = ctptr; /* warning - this is just MIDI_CONTROLLER_FADER... */
	ReWireDevices[ReWireDevicetableSize].encodedDeviceName = dev;
	ReWireDevices[ReWireDevicetableSize].encodedControllerName = cont;
	ReWireDevices[ReWireDevicetableSize].node = NULL;
	#ifdef MIDIVERBOSE
		printf ("ReWireDeviceRegister, new entry at %d",ReWireDevicetableSize);
		printf ("	Device %s (%d) controller %s (%d) ", ReWireNamenames[dev].name,
					dev, ReWireNamenames[cont].name, cont);
		printf ("	bus %d channel %d controller %d cmin %d cmax %d\n",bus, channel, 
				controller, cmin, cmax);
	#endif
	return TRUE; /* name not found, but, requested */
}

void registerReWireNode(struct X3D_Node *node) {
	int count;
	uintptr_t *myptr;

	if (node == 0) {
		printf ("error in registerReWireNode; somehow the node datastructure is zero \n");
		return;
	}

	if (node->_nodeType != NODE_MidiControl) return;

	MidiNodes = (uintptr_t *) REALLOC (MidiNodes,sizeof (uintptr_t *) * (num_MidiNodes+1));
	myptr = MidiNodes;

	/* does this event exist? */
	for (count=0; count <num_MidiNodes; count ++) {
		if (*myptr == (uintptr_t) node) {
			printf ("registerReWireNode, already this node\n"); 
			return;
		}	
		myptr++;
	}


	/* now, put the function pointer and data pointer into the structure entry */
	*myptr = (uintptr_t) node;

	num_MidiNodes++;
}


/* return a node assoicated with this name. If the name exists, return the previous node. If not, return
the new node */
static int ReWireNameIndex (char *name) {
	int ctr;
	
	/* printf ("ReWireNameIndex, looking for %s\n",name); */
	/* is this a duplicate name and type? types have to be same,
	   name lengths have to be the same, and the strings have to be the same.
	*/
	for (ctr=0; ctr<=ReWireNametableSize; ctr++) {
		/* printf ("ReWireNameIndex, comparing %s to %s\n",name,ReWireNamenames[ctr].name); */
		if (strcmp(name,ReWireNamenames[ctr].name)==0) {
			/* printf ("ReWireNameIndex, FOUND IT at %d\n",ctr); */
			return ctr;
		}
	}

	/* nope, not duplicate */

	ReWireNametableSize ++;

	/* ok, we got a name and a type */
	if (ReWireNametableSize >= MAXReWireNameNames) {
		/* oooh! not enough room at the table */
		MAXReWireNameNames += 1024; /* arbitrary number */
		ReWireNamenames = (struct ReWireNamenameStruct*)REALLOC (ReWireNamenames, sizeof(*ReWireNamenames) * MAXReWireNameNames);
	}

	ReWireNamenames[ReWireNametableSize].name = STRDUP(name);
	/* printf ("ReWireNameIndex, new entry at %d\n",ReWireNametableSize); */
	return ReWireNametableSize;
}

#if defined(REWIRE_SERVER)
/* make sure the EAI port is turned on... */
static int requestToStartEAIdone = FALSE;
#endif

static void midiStartEAI() {
#if !defined(REWIRE_SERVER)
    ConsoleMessage("MIDI disabled at compile time.\n");
#else
	char myline[2000];
	if (!requestToStartEAIdone) {
		if (strlen(REWIRE_SERVER) > 1000) return; /* bounds check this compile time value */

		printf ("MidiControl - turning EAI on\n");
		sprintf (myline,"%s %u &",REWIRE_SERVER,getpid());
		create_MIDIEAI();
		if (system (myline)==127) {
			strcpy (myline,"could not start ");
			strncat (myline, REWIRE_SERVER,1000);
			ConsoleMessage (myline);
		}
	}
	requestToStartEAIdone = TRUE;
#endif
}

void prep_MidiControl (struct X3D_MidiControl *node) {
	/* get the name/device pairing */
	int tmp_bus = INT_ID_UNDEFINED;
	int tmp_controller = INT_ID_UNDEFINED;
	int tmp_internchan = INT_ID_UNDEFINED;
	int tmpdeviceMinVal = INT_ID_UNDEFINED;
	int tmpdeviceMaxVal = INT_ID_UNDEFINED;
	int tmpintControllerType = INT_ID_UNDEFINED;
	int controllerPresent = FALSE;

	/* is there anything to do? */
	#ifdef MIDIVERBOSE
printf ("prep_MidiControl, for node %d, change %d ichange %d _bus %d channel %d (_channel %d) _controller %d",node,
	node->_change, node->_ichange,
	node->_bus,node->channel,
	node->_channel, node->_controller);
printf (" _deviceNameIndex %d _controllerIndex %d\n",node->_deviceNameIndex, node->_controllerIndex);
	#endif

/*
        int _bus;
        int _butPr;
        int _channel;
        int _controller;
        int _controllerIndex;
        int _deviceNameIndex;
        int _intControllerType;
        int _oldintValue;
        int _sentVel;
        int _vel;
        int autoButtonPress;
        int buttonPress;
        int channel;
        struct Uni_String *controller;
        int controllerPresent;
        struct Uni_String *controllerType;
        int deviceMaxVal;
        int deviceMinVal;
        struct Uni_String *deviceName;
        float floatValue;
        int highResolution;
        int intValue;
        int maxVal;
        int minVal;
        float pressLength;
        double pressTime;
        int useIntValue;
        int velocity;
*/

	/* first time through, the _ichange will be 0. Lets ensure that we start events properly. */
	if (node->_ichange == 0) {
		#ifdef MIDIVERBOSE
		printf ("Midi Node being initialized\n");
		#endif
		midiStartEAI();
		node->controllerPresent = 999; /* force event for correct controllerPresent */

	}

	/* if the deviceName has changed, or has never been found... */
	if ((node->deviceName->touched > 0) || (node->_deviceNameIndex < 0)) {
		#ifdef MIDIVERBOSE
		printf ("NODE DEVICE NAME CHANGED now is %s\n",node->deviceName->strptr);
		#endif
		node->_deviceNameIndex = ReWireNameIndex(node->deviceName->strptr);
		node->deviceName->touched = 0;
		if (node->_deviceNameIndex>=0) MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, deviceName));
	}

	/* controller type... */
	if (node->controllerType->touched != 0) {
		#ifdef MIDIVERBOSE
		printf ("CONTROLLERTYPE CHANGED\n");
		#endif
		node->controllerType->touched = 0;  
		if (strcmp(node->controllerType->strptr,"Slider") == 0) {
			#ifdef MIDIVERBOSE
			printf ("midi controller is a fader\n");
			#endif

			node->_intControllerType = MIDI_CONTROLLER_FADER;
		} else if (strcmp(node->controllerType->strptr,"ButtonPress") == 0) {
			#ifdef MIDIVERBOSE
			printf ("midi controller is a keypress\n");
			#endif

			node->_intControllerType = MIDI_CONTROLLER_KEYPRESS;
			/* we have a special controller name for the "keypress" nodes; controllers are not valid here */
			node->controller = newASCIIString(BUTTON_PRESS_STRING);
		} else {
			ConsoleMessage ("MidiControl - unknown controllerType :%s:\n",node->controllerType->strptr);
		}

		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, controllerType));
	}

	/* is this a controller, or a keypress node? Keypresses go directly to the bus/channel, they do not have
	   a controller number, so we have a default controller number */
	if ((node->controller->touched > 0) || (node->_controllerIndex < 0)) {
		#ifdef MIDIVERBOSE
		printf ("NODE CONTROLLER CHANGED now is %s\n",node->controller->strptr);
		#endif
		node->_controllerIndex = ReWireNameIndex(node->controller->strptr);
		node->controller->touched = 0;
		if (node->_controllerIndex>=0) MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, controller));
	}



	/* look for the end point to be there - if a "Slider" we need the device AND controller; if
	   "ButtonPress" need just the device */
	tmpintControllerType = node->_intControllerType;
	tmp_bus = node->_bus;
	tmp_internchan = node->_channel;
	if (node->_intControllerType == MIDI_CONTROLLER_KEYPRESS) {
		/* this is a keypress to the midi bus/channel; it is not really a controller */
		node->_controller = ReWireNameIndex(BUTTON_PRESS_STRING);

		#ifdef MIDIVERBOSE
		printf ("FYI - ReWireNameIndex for keypress is %d\n",node->_controller);
		#endif
	}
	tmp_controller = node->_controller;

	controllerPresent = ReWireDeviceIndex (node,
				&tmp_bus,		/* return the bus, if it is available */
				&tmp_internchan,	/* return the channel, if it is available */
				&tmp_controller,	/* return the controller, if it is available */
				&tmpdeviceMinVal,	/* return the device min val, if available */
				&tmpdeviceMaxVal,	/* return the device max val, if available */
				&tmpintControllerType);	/* return the controller type (slider, button) if availble */

	#ifdef MIDIVERBOSE
		printf ("compile_midiControl, for %d %d controllerPresent %d, node->controllerPresent %d\n",
				ReWireNameIndex(node->deviceName->strptr),
				ReWireNameIndex(node->controller->strptr),
				controllerPresent, node->controllerPresent);
	#endif

	if (controllerPresent != node->controllerPresent) {
		#ifdef MIDIVERBOSE
		if (controllerPresent) printf ("controllerPresent changed FOUND CONTROLLER\n");
		if (!controllerPresent) printf ("controllerPresent changed LOST CONTROLLER\n");
		#endif
		node->controllerPresent = controllerPresent;
		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, controllerPresent));
	}

	if (controllerPresent) {
		if (tmp_bus != node->_bus) {
			#ifdef MIDIVERBOSE
			printf ("INTERNAL: bus changed from %d to %d\n",node->_bus, tmp_bus);
			#endif
			node->_bus = tmp_bus;
		}
		if (tmp_internchan != node->_channel) {
			#ifdef MIDIVERBOSE
			printf ("INTERNAL: channel changed from %d to %d\n",node->_channel, tmp_internchan);
			#endif
			node->_channel = tmp_internchan;
		}
		if (tmp_controller != node->_controller) {
			#ifdef MIDIVERBOSE
			printf ("INTERNAL: controller changed from %d to %d\n",node->_controller, tmp_controller);
			#endif
			node->_controller = tmp_controller;
		}
		if (tmpdeviceMinVal != node->deviceMinVal) {
			#ifdef MIDIVERBOSE
			printf ("deviceMinVal changed from %d to %d\n",node->deviceMinVal, tmpdeviceMinVal);
			#endif
			node->deviceMinVal = tmpdeviceMinVal;
			MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, deviceMinVal));
		}
		if (node->_vel != node->velocity) {
			#ifdef MIDIVERBOSE
			printf ("velocity changed from %d to %d\n",node->_vel, node->velocity);
			#endif
			if (node->velocity > 127) node->velocity = 127;
			if (node->velocity <0) node->velocity = 0;

			node->_vel = node->velocity;
			MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, velocity));
		}
		if (tmpdeviceMaxVal != node->deviceMaxVal) {
			#ifdef MIDIVERBOSE
			printf ("deviceMaxVal changed from %d to %d\n",node->deviceMaxVal, tmpdeviceMaxVal);
			#endif
			node->deviceMaxVal = tmpdeviceMaxVal;
			MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, deviceMaxVal));
		}
		if (tmpintControllerType != node->_intControllerType) {
			#ifdef MIDIVERBOSE
			printf ("intControllerType  changed from %d to %d\n",node->_intControllerType, tmpintControllerType);
			#endif

			node->_intControllerType = tmpintControllerType;
			MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, controllerType));
			switch (node->_intControllerType) {
				case MIDI_CONTROLLER_FADER:
					verify_Uni_String(node->controllerType,"Slider");
					break;
				case MIDI_CONTROLLER_KEYPRESS:
					verify_Uni_String(node->controllerType,"ButtonPress");
					break;
				case MIDI_CONTROLLER_UNKNOWN:
					verify_Uni_String(node->controllerType,"Unknown");
					break;
				default:
					ConsoleMessage ("Unknown Controller Type for MIDI node, have %d\n",
						node->_intControllerType);
			}
		}
	} else {
		#ifdef MIDIVERBOSE
		printf ("compile_MidiControl - device not present yet\n");
		#endif
	}


	MARK_NODE_COMPILED 
}

void ReWireRegisterMIDI (char *str) {
	int curBus;
	int curChannel;
	int curMin;
	int curMax;
	int curType;
	int curController;

	char *EOT;

	int encodedDeviceName = INT_ID_UNDEFINED;
	int encodedControllerName = INT_ID_UNDEFINED;



	/*
	   "Hardware Interface 1" 5 0 
	   "Melody Automation" 5 1 
	   1 "Mod Wheel" 1 127 0
	   5 "Portamento" 1 127 0
	   7 "Master Level" 1 127 0
	   9 "Oscillator B Decay" 1 127 0
	   12 "Oscillator B Sustain" 1 127 0
	   14 "Filter Env Attack" 1 127 0
	   15 "Filter Env Decay" 1 127 0
	   16 "Filter Env Sustain" 1 127 0
	   17 "Filter Env Release" 1 127 0
	   18 "Filter Env Amount" 1 127 0
	   19 "Filter Env Invert" 3 1 0
	   21 "Oscillator B Octave" 3 8 0
	   79 "Body Type" 3 4 0
	 */	

	/* set the device tables to the starting value. */
	ReWireDevicetableSize = -1;

	#ifdef MIDIVERBOSE
	printf ("ReWireRegisterMIDI - have string :%s:\n",str);
	#endif


	while (*str != '\0') {
		while (*str == '\n') str++;
		/* is this a new device? */

		if (*str == '"') {
			str++;
			EOT = strchr (str,'"');
			if (EOT != NULL) *EOT = '\0'; else {
				printf ("ReWireRegisterMidi, expected string here: %s\n",str);
			}
			#ifdef MIDIVERBOSE
			printf ("device name is :%s:\n",str);
			#endif
			encodedDeviceName = ReWireNameIndex(str);
			str = EOT+1;
			sscanf (str, "%d %d",&curBus, &curChannel);

			/* make an entry for devices that have NO controllers, maybe only buttonPresses */
			encodedControllerName = ReWireNameIndex(BUTTON_PRESS_STRING);
			curController = -1; curMin = 0; curMax = 127; curType = MIDI_CONTROLLER_KEYPRESS;

			if (!ReWireDeviceRegister(encodedDeviceName, encodedControllerName, curBus, 
				curChannel, curController, curMin, curMax, curType)) {
				#ifdef MIDIVERBOSE
				printf ("ReWireRegisterMIDI, duplicate device for %s %s\n",
					ReWireNamenames[encodedDeviceName].name, ReWireNamenames[encodedControllerName].name); 
				#endif
			}

		} else if (*str == '\t') {
			str++;
			sscanf (str, "%d", &curController);
			EOT = strchr (str,'"');
			if (EOT != NULL) str = EOT+1; else {
				printf ("ReWireRegisterMidi, expected string here: %s\n",str);
			}
			EOT = strchr (str,'"');
			if (EOT != NULL) *EOT = '\0'; else {
				printf ("ReWireRegisterMidi, expected string here: %s\n",str);
			}
			#ifdef MIDIVERBOSE
			printf ("	controllername is :%s: ",str);
			#endif
			encodedControllerName = ReWireNameIndex(str);
			str = EOT+1;
			sscanf (str, "%d %d %d",&curType, &curMax, &curMin);

			#ifdef MIDIVERBOSE
			printf ("(encodedDevice:%d encodedController:%d)  Bus:%d Channel:%d controller %d curMin %d curMax %d curType %d\n",encodedDeviceName, encodedControllerName, curBus, curChannel,
				curController, curMin, curMax, curType);
			#endif
		
			/* register the info for this controller */
			if (!ReWireDeviceRegister(encodedDeviceName, encodedControllerName, curBus, 
				curChannel, curController, curMin, curMax, curType)) {
				#ifdef MIDIVERBOSE
				printf ("ReWireRegisterMIDI, duplicate device for %s %s\n",
					ReWireNamenames[encodedDeviceName].name, ReWireNamenames[encodedControllerName].name); 
				#endif
			}

		} else {
			if (*str != ' ') printf ("ReWireRegisterMidi - garbage (%c)  at:%s\n",*str,str);
		}
		
		/* skip to the end of the line */
		while (*str >= ' ') str++;
		while (*str == '\n') str++;
	}
}

void do_MidiControl (void *this) {
	struct X3D_MidiControl* node;
	int value;
	float fV;
	int sendEvent;

	node = (struct X3D_MidiControl*) this;

	#ifdef MIDIVERBOSE
	printf ("do MidiControl for node %s\n",stringNodeType(node->_nodeType));
	#endif
	if (NODE_MidiControl == node->_nodeType) {
		#ifdef MIDIVERBOSE
		printf ("ReWire change %d %d ", node->_ichange, node->_change); 

		printf ("bus %d channel %d(%d) controller :%s: controllerType :%s: device :%s: ",
			node->_bus, node->_channel, node->channel,
			node->controller->strptr, node->controllerType->strptr, node->deviceName->strptr);
		printf (" devp %d fv %f iv %d hr %d ct %d intVal %d max %d min %d\n",
			node->controllerPresent, node->floatValue, node->intValue, node->highResolution,
			node->_intControllerType, node->intValue, node->maxVal, node->minVal);
		#endif

		value = node->intValue;
		fV = node->floatValue;
		if (node->useIntValue) {
			determineMIDIValFromInt (node, &value, &fV);
		} else {
			determineMIDIValFromFloat (node, &value, &fV); 
		}

		/* should we send this event? */
		sendEvent = FALSE;
		if (node->_intControllerType == MIDI_CONTROLLER_FADER) {
			if (value != node->_oldintValue) sendEvent = TRUE;
		} else {
			/* are we automatically generating buttonPresses? */
			if (node->autoButtonPress) {
				/* send event if value has changed */
				if (value != node->_oldintValue) {
					sendEvent = TRUE;
					node->_sentVel = node->_vel;   /* this is a Note On */
					node->pressTime = TickTime;	   /* time button pressed */
					#ifdef MIDIVERBOSE
					printf ("auto Note %d on at %lf\n",value, TickTime);
					#endif
				}

				/* have we timed out? */
				if (node->pressTime > 0.001) {
					if (TickTime > (node->pressTime + node->pressLength))  {
						#ifdef MIDIVERBOSE
						printf ("note timed out at %lf, sending note_off for pressTime %lf \n",TickTime, node->pressTime);
						#endif

						sendEvent = TRUE;
						node->_sentVel = -1;		/* this is a Note Off */
						node->pressTime = (double)0.0;
					}
				}
			} else {
				/* send event if buttonPress changes state */
				if (node->_butPr != node->buttonPress) {
					sendEvent = TRUE;
					value = node->intValue;
					if (node->buttonPress) {
						#ifdef MIDIVERBOSE
						printf ("have ButtonPress at %lf\n",TickTime);
						#endif
						node->_sentVel = node->_vel;   /* this is a Note On */
						node->pressTime = TickTime;	   /* time button pressed */
					} else {
						#ifdef MIDIVERBOSE
						printf ("have ButtonRelease at %lf\n",TickTime);
						#endif
						node->_sentVel = -1;		/* this is a Note Off */
						node->pressTime = (double)0.0;
					}

					/* now, reset the toggle... */
					node->_butPr = node->buttonPress;
				}
			}
		}
					
		if (sendEvent) {
			node->intValue = value;
			node->_oldintValue = value;
			node->floatValue = fV;
			MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, intValue));
			MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, floatValue));

			#ifdef MIDIVERBOSE
			printf ("intValue changed - now is %d sentvel %d\n",node->intValue, node->_sentVel); 
			printf ("sending %d %f ",value, node->floatValue);
			printf ("mins %d %d maxs %d %d ",node->deviceMinVal, node->minVal, node->deviceMaxVal, node->maxVal);
			printf ("float %f node->floatVal\n",node->floatValue);
			#endif

			sendNodeToReWire(node);
			update_node (X3D_NODE(node));
		}
	}	
}


/* a MIDI event is coming in from the EAI port! */
void ReWireMIDIControl (char *line) {
	long int timeDiff;
	int bus, channel, controller, value;
	int rv;
	int ctr;
	struct X3D_MidiControl *node = NULL;
	float fV;
	int sendEvent;
	int noteOn;

	sendEvent = FALSE;
	value = 0;

	#ifdef MIDIVERBOSE
	printf ("ReWireMIDIControl: read %s\n",line);
	#endif

	if ((rv=sscanf (line, "%ld %d %d %d %d",&timeDiff, &bus, &channel, &controller, &value)) != 5) {
		printf ("Error (%d)on reading MIDICONTROL, line %s\n",rv,line);
		return;
	}

	/* we only look for Controller, Note On and Note Off */
	if ( ((channel & 0xF0) != 0x80)  &&
		((channel & 0xF0) != 0x90)  &&
		((channel & 0xF0) != 0xb0)) {
		#ifdef MIDIVERBOSE
			printf ("ReWireMidiControl, got %x, unsupported. Dropping it\n",channel);
		#endif
		return;
	}

	/* is this a Slider/Fader? */
	if ((channel & 0xF0) == 0xb0) {
		/* get channel number */
		channel = channel & 0x0f;
	
		#ifdef MIDIVERBOSE
			printf ("ReWireMIDIControl - input timedif %ld bus %d channel %d controller %d value %d\n",
						timeDiff, bus, channel, controller, value);
			printf ("ReWireDevicetableSize %d\n",ReWireDevicetableSize);
		#endif
	
	
		for (ctr=0; ctr<=ReWireDevicetableSize; ctr++) {
			#ifdef REALLYMIDIVERBOSE
				printf ("	ind %d comparing bus %d:%d channel %d:%d and  controller %d:%d\n", 
					ctr, bus, ReWireDevices[ctr].bus,
					channel, ReWireDevices[ctr].channel,
					controller, ReWireDevices[ctr].controller);
			#endif
	
			if ((bus==ReWireDevices[ctr].bus) &&
				(channel == ReWireDevices[ctr].channel) &&
				(controller == ReWireDevices[ctr].controller)) {
				#ifdef MIDIVERBOSE
					printf ("ReWireMidiControl, FOUND IT at %d - bus %d channel %d controller %d\n",ctr,
						bus, channel, controller);
				#endif
	
				if (ReWireDevices[ctr].node == NULL) {
					#ifdef MIDIVERBOSE
					printf ("ReWireMidiEvent, node for %d is still NULL\n",ctr);
					#endif
				} else {
					node = ReWireDevices[ctr].node;
					#ifdef MIDIVERBOSE
					printf ("routing to device %s, controller %s ",node->deviceName->strptr, node->controller->strptr);
					printf (" nameIndex %d controllerIndex %d deviceMinVal %d deviceMaxVal %d minVal %d maxVal%d\n",
						node->_deviceNameIndex, node->_controllerIndex, node->deviceMinVal,
						node->deviceMaxVal, node->minVal, node->maxVal);
					#endif
	
					determineMIDIValFromInt (node, &value, &fV); 
					if (value != node->_oldintValue) sendEvent = TRUE;
						
	
				}
			}
		}
	} else {
		/* this is a Note on/Note off */
		noteOn = ((channel & 0xF0) == 0x90);
 
		/* get channel number */
		channel = channel & 0x0f;

		#ifdef MIDIVERBOSE
		if (noteOn) printf ("ReWireMidiControl, have a note ON  channel %d, velocity %d\n",channel,value);
		else printf ("ReWireMidiControl, have a note OFF channel %d, velocity %d\n",channel,value);
		#endif
		for (ctr=0; ctr<=ReWireDevicetableSize; ctr++) {
			#ifdef MIDIVERBOSE
				printf ("	ind %d comparing bus %d:%d channel %d:%d with ecdn %d, eccn %d, cont %d cmin %d cmax %d ctype %d\n", 
					ctr, bus, ReWireDevices[ctr].bus,
					channel, ReWireDevices[ctr].channel,
				        ReWireDevices[ctr].encodedDeviceName,          /* index into ReWireNamenames */
				        ReWireDevices[ctr].encodedControllerName,      /* index into ReWireNamenames */
				        ReWireDevices[ctr].controller,                 /* controller number */
				        ReWireDevices[ctr].cmin,                       /* minimum value for this controller */
				        ReWireDevices[ctr].cmax,                       /* maximum value for this controller */
				        ReWireDevices[ctr].ctype                      /* controller type TYPE OF FADER control - not used currently */
				);
			#endif

			/* look for the entry- we have a device with a controller of -1 for the main "device" 
			   so that we don't send events to the same node many times */

			if ((bus==ReWireDevices[ctr].bus) &&
				(channel == ReWireDevices[ctr].channel) &&
				(ReWireDevices[ctr].controller == -1)) {
				#ifdef MIDIVERBOSE
					printf ("ReWireMidiControl, FOUND IT at %d\n",ctr);
				#endif
	
				if (ReWireDevices[ctr].node == NULL) {
					#ifdef MIDIVERBOSE
					printf ("ReWireMidiEvent, node for %d is still NULL\n",ctr);
					#endif

				} else {
					node = ReWireDevices[ctr].node;

					/* ok, the last 2 bytes of this event, for faders are "controller value" but
					   for notes they are "note velocity". This is confusing... */

					/* should we send this event? */
					/* send event if buttonPress changes state */
					if (noteOn != node->buttonPress) {
						sendEvent = TRUE;
						if (noteOn) {
							#ifdef MIDIVERBOSE
							printf ("have ButtonPress at %lf\n",TickTime);
							#endif
							node->pressTime = TickTime;	   /* time button pressed */
							node->velocity = value;
						} else {
							#ifdef MIDIVERBOSE
							printf ("have ButtonRelease at %lf\n",TickTime);
							#endif
							node->pressTime = (double)0.0;
							/* DO NOT set velocity here - it will be 0 ALWAYS */
						}
						value = controller; /* second byte; see comment above */

						/* now, reset the toggle... */
						node->buttonPress = noteOn;
						node->_butPr = node->buttonPress;
	
						determineMIDIValFromInt (node, &value, &fV); 
					
					}
				}
			}
		}
	}

	/* do we send an event into the FreeWRL machine? */
	if (sendEvent) {
		node->intValue = value;
		node->_oldintValue = value;
		node->floatValue = fV;
		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, intValue));
		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, floatValue));
		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, buttonPress));
		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, velocity));

		#ifdef MIDIVERBOSE
		printf ("intValue changed - now is %d sentvel %d\n",node->intValue, node->_sentVel); 
		printf ("sending %d %f ",value, node->floatValue);
		printf ("mins %d %d maxs %d %d ",node->deviceMinVal, node->minVal, node->deviceMaxVal, node->maxVal);
		printf ("float %f node->floatVal\n",node->floatValue);
		#endif
	}
}


/************ END OF MIDI CONTROL **************************/

void render_LoadSensor (struct X3D_LoadSensor *node) {
	int count;
	int nowLoading;
	int nowFinished;
	struct X3D_ImageTexture *tnode;
#ifdef HAVE_TO_REIMPLEMENT_MOVIETEXTURES
	struct X3D_MovieTexture *mnode;
#endif /* HAVE_TO_REIMPLEMENT_MOVIETEXTURES */
	struct X3D_AudioClip *anode;
	struct X3D_Inline *inode;
	
	/* if not enabled, do nothing */
	if (!node) return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_LoadSensor, enabled));
	}
	if (!node->enabled) return;

	/* we only need to look at this during the rendering pass - once per event loop */
	if (!render_geom) return;

	/* do we need to re-generate our internal variables? */
	if NODE_NEEDS_COMPILING {
		MARK_NODE_COMPILED
		node->__loading = 0;
		node->__finishedloading = 0;
		node->progress = (float) 0.0;
		node->__StartLoadTime = 0.0;
	}

	/* do we actually have any nodes to watch? */
	if (node->watchList.n<=0) return;

	/* are all nodes loaded? */
	if (node->__finishedloading == node->watchList.n) return;

	/* our current status... */
	nowLoading = 0;
	nowFinished = 0;

	/* go through node list, and check to see what the status is */
	/* printf ("have %d nodes to watch\n",node->watchList.n); */
	for (count = 0; count < node->watchList.n; count ++) {

		tnode = (struct X3D_ImageTexture *) node->watchList.p[count];

		/* printf ("node type of node %d is %d\n",count,tnode->_nodeType); */
		switch (tnode->_nodeType) {
		case NODE_ImageTexture:
			/* printf ("opengl tex is %d\n",tnode->__texture); */
			/* is this texture thought of yet? */
			nowLoading++;
			if (isTextureLoaded(tnode->__textureTableIndex)) {
				/* is it finished loading? */
				nowFinished ++;
			}
				
			break;

		case NODE_MovieTexture:
#ifdef HAVE_TO_REIMPLEMENT_MOVIETEXTURES
			mnode = (struct X3D_MovieTexture *) tnode; /* change type to MovieTexture */
			/* printf ("opengl tex is %d\n",mnode->__texture0_); */
			/* is this texture thought of yet? */
			if (mnode->__texture0_ > 0) {
				nowLoading++;
				/* is it finished loading? */
				if (isTextureLoaded(mnode->__texture0_)) nowFinished ++;
			}
#endif /* HAVE_TO_REIMPLEMENT_MOVIETEXTURES */
				
			break;

		case NODE_Inline:
			inode = (struct X3D_Inline *) tnode; /* change type to Inline */
			/* printf ("LoadSensor, Inline %d, type %d loadstatus %d at %d\n",inode,inode->_nodeType,inode->__loadstatus, &inode->__loadstatus); */
			break;

		case NODE_Script:
			nowLoading ++; /* broken - assume that the url is ok for now */
			break;

		case NODE_AudioClip:
			anode = (struct X3D_AudioClip *) tnode; /* change type to AudioClip */
			/* AudioClip sourceNumber will be gt -1 if the clip is ok. see code for details */
			if (anode->__sourceNumber > -1) nowLoading ++;

			break;

		default :{} /* there should never be anything here, but... */
		}
	}
		

	/* ok, are we NOW finished loading? */
	if (nowFinished == node->watchList.n) {
		node->isActive = 0;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isActive));

		node->isLoaded = 1;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isLoaded));

		node->progress = (float) 1.0;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, progress));

		node->loadTime = TickTime;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, loadTime));
	}	

	/* have we NOW started loading? */
	if ((nowLoading > 0) && (node->__loading == 0)) {
		/* mark event isActive TRUE */
		node->isActive = 1;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isActive));

	
		node->__StartLoadTime = TickTime;
	}
	
	/* what is our progress? */
	if (node->isActive == 1) {
		node->progress = (float)(nowFinished)/(float)(node->watchList.n);
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, progress));
	}

	/* remember our status for next time. */
	node->__loading = nowLoading;
	node->__finishedloading = nowFinished;

	/* did we run out of time? */
	if (node->timeOut > 0.0001) {			/* we have a timeOut specified */
		if (node->__StartLoadTime > 0.001) {	/* we have a start Time recorded from the isActive = TRUE */
		
			/* ok, we should look at time outs */
			if ((TickTime - node->__StartLoadTime) > node->timeOut) {
				node->isLoaded = 0;
				MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isLoaded));

				node->isActive = 0;
				MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isActive));

				/* and, we will just assume that we have loaded everything next iteration */
				node->__finishedloading = node->watchList.n;
			}
		}
	}
}


void child_Anchor (struct X3D_Anchor *node) {
	int nc = (node->children).n;
	LOCAL_LIGHT_SAVE

	/* printf ("child_Anchor node %u, vis %d\n",node,node->_renderFlags & VF_hasVisibleChildren); */

	/* any children at all? */
	if (nc==0) return;

	/* any visible children? */
	OCCLUSIONTEST

	#ifdef CHILDVERBOSE
	printf("RENDER ANCHOR START %d (%d)\n",node, nc);
	#endif

	/* do we have a local light for a child? */
	LOCAL_LIGHT_CHILDREN(node->children);

	/* now, just render the non-directionalLight children */
	normalChildren(node->children);

	#ifdef CHILDVERBOSE
	printf("RENDER ANCHOR END %d\n",node);
	#endif

	LOCAL_LIGHT_OFF
}

/* note that we get the resources in a couple of steps; this tries to keep the scenegraph running */
void load_Inline (struct X3D_Inline *node) {
	resource_item_t *res;

	/* printf ("load_Inline %u, loadStatus %d loadResource %u\n",node, node->__loadstatus, node->__loadResource);
	printf ("	load_Inline, parentURL %s\n",node->__parenturl->strptr); */

	if (node->load) {
		/* printf ("loading Inline\n"); */

		switch (node->__loadstatus) {
			case INLINE_INITIAL_STATE: /* nothing happened yet */

			if (node->url.n == 0) {
				node->__loadstatus = INLINE_STABLE; /* a "do-nothing" approach */
			} else {
				resource_item_t *myres;
				resource_item_t *parentResource;

				myres = (resource_item_t*)(node->_parentResource);
				parentResource = (resource_item_t *)(node->_parentResource);

				res = resource_create_multi(&(node->url));
				res->media_type = resm_unknown;
				node->__loadstatus = INLINE_FETCHING_RESOURCE;
				node->__loadResource = res;
			}
			break;

			case INLINE_FETCHING_RESOURCE:
			res = node->__loadResource;
			resource_identify(node->_parentResource, res);
			
			/* printf ("load_Inline, before resource_fetch, we have type  %s  status %s\n",
				resourceTypeToString(res->type), resourceStatusToString(res->status)); */
			
			resource_fetch(res);

			/* printf ("load_Inline, after resource_fetch, we have type  %s  status %s\n",
				resourceTypeToString(res->type), resourceStatusToString(res->status)); */
			

			/* do we try the next url in the multi-url? */
			if ((res->status == ress_failed) && (res->m_request != NULL)) {
				/* printf ("load_Inline, not found, lets try this again\n");*/
				res->status = ress_invalid;
				res->type = rest_multi;

			/* did we get this one? */
			} else if (res->status == ress_downloaded) {
				res->media_type = resm_unknown;
				res->where = X3D_NODE(node);
				res->offsetFromWhere = (float) offsetof (struct X3D_Inline, __children);
				send_resource_to_parser(res);
				node->__loadstatus = INLINE_PARSING; /* a "do-nothing" approach */
			} else {
				if ((res->status == ress_failed) || (res->status == ress_invalid)) {
					printf ("resource failed to load\n");
					node->__loadstatus = INLINE_STABLE; /* a "do-nothing" approach */
				} else {
					printf ("resource Inline in invalid state\n");
					node->__loadstatus = INLINE_STABLE; /* a "do-nothing" approach */
				}
			}

			break;

			case INLINE_PARSING:
				res = node->__loadResource;
/*
				printf ("inline parsing.... %s\n",resourceStatusToString(res->status));
				printf ("res complete %d\n",res->complete);
*/
				if (res->status == ress_parsed) {
					node->__loadstatus = INLINE_STABLE; 
				} 

			break;
		}
	} else {
		printf ("unloading Inline\n");
	}
}



void child_Inline (struct X3D_Inline *node) {
	int nc = (node->__children).n;

	LOCAL_LIGHT_SAVE

	#ifdef CHILDVERBOSE
	printf("RENDER INLINE START %d (%d)\n",node, nc);
	printf ("	child_Inline, %u, loadStatus %d, nc %d\n",node,node->__loadstatus, nc);
	#endif

	#ifdef CHILDVERBOSE
		{int i;
			for (i=0; i<nc; i++) { printf ("ch %d %s ",i,stringNodeType(X3D_NODE(node->children.p[i])->_nodeType));} 
		printf ("\n");}
	#endif

	/* any children at all? */
	if (nc==0) return; 

	/* do we have a local light for a child? */
	LOCAL_LIGHT_CHILDREN(node->__children);

	/* now, just render the non-directionalLight children */
	normalChildren(node->__children);

	#ifdef CHILDVERBOSE
	printf("RENDER INLINE END %d\n",node);
	#endif

	LOCAL_LIGHT_OFF
}
