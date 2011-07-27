/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Networking.c,v 1.43 2011/07/27 23:42:31 crc_canada Exp $

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

#define BUTTON_PRESS_STRING "use_for_buttonPresses"

#ifdef OLDCODE
OLDCODE ///* keep track of the Midi nodes. */
OLDCODE //static uintptr_t *MidiNodes = NULL;
OLDCODE //static int p->num_MidiNodes = 0;
OLDCODE //
OLDCODE //struct ReWireNamenameStruct *ReWireNamenames = 0;
OLDCODE //int ReWireNametableSize = -1;
OLDCODE //static int MAXReWireNameNames = 0;
OLDCODE //
OLDCODE //struct ReWireDeviceStruct *ReWireDevices = 0;
OLDCODE //int ReWireDevicetableSize = -1;
OLDCODE //int MAXReWireDevices = 0;
OLDCODE
OLDCODEtypedef struct pComponent_Networking{
OLDCODE	/* keep track of the Midi nodes. */
OLDCODE	uintptr_t *MidiNodes;// = NULL;
OLDCODE	int num_MidiNodes;// = 0;
OLDCODE
OLDCODE	//struct ReWireNamenameStruct *ReWireNamenames;// = 0;
OLDCODE	//int ReWireNametableSize;// = -1;
OLDCODE	int MAXReWireNameNames;// = 0;
OLDCODE
OLDCODE	//struct ReWireDeviceStruct *ReWireDevices;// = 0;
OLDCODE	//int ReWireDevicetableSize;// = -1;
OLDCODE	int MAXReWireDevices;// = 0;
OLDCODE#if defined(REWIRE_SERVER)
OLDCODE	/* make sure the EAI port is turned on... */
OLDCODE	int requestToStartEAIdone;// = FALSE;
OLDCODE#endif
OLDCODE
OLDCODE}* ppComponent_Networking;
OLDCODEvoid *Component_Networking_constructor(){
OLDCODE	void *v = malloc(sizeof(struct pComponent_Networking));
OLDCODE	memset(v,0,sizeof(struct pComponent_Networking));
OLDCODE	return v;
OLDCODE}
OLDCODEvoid Component_Networking_init(struct tComponent_Networking *t){
OLDCODE	//public
OLDCODE	t->ReWireDevices = 0;
OLDCODE	t->ReWireDevicetableSize = -1;
OLDCODE	t->ReWireNamenames = 0;
OLDCODE	t->ReWireNametableSize = -1;
OLDCODE
OLDCODE	//private
OLDCODE	t->prv = Component_Networking_constructor();
OLDCODE	{
OLDCODE		ppComponent_Networking p = (ppComponent_Networking)t->prv;
OLDCODE		/* keep track of the Midi nodes. */
OLDCODE		p->MidiNodes = NULL;
OLDCODE		p->num_MidiNodes = 0;
OLDCODE
OLDCODE		p->MAXReWireNameNames = 0;
OLDCODE
OLDCODE		p->MAXReWireDevices = 0;
OLDCODE#if defined(REWIRE_SERVER)
OLDCODE/* make sure the EAI port is turned on... */
OLDCODEp->requestToStartEAIdone = FALSE;
OLDCODE#endif
OLDCODE
OLDCODE	}
OLDCODE}
OLDCODE//ppComponent_Networking p = (ppComponent_Networking)gglobal()->Component_Networking.prv;
OLDCODE
#endif // OLDCODE


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

#ifdef OLDCODE
OLDCODE/************ START OF MIDI CONTROL **************************/
OLDCODE
OLDCODE/* go through the node, and create a new int value, and a new float value, from an int value */
OLDCODEstatic void determineMIDIValFromFloat (struct X3D_MidiControl *node, int *value, float *fV) {
OLDCODE	int minV, maxV, possibleValueSpread;
OLDCODE	float tv;
OLDCODE
OLDCODE	/* find the int value range possible */
OLDCODE	minV = 0;  maxV = 100000;
OLDCODE	if (minV < node->deviceMinVal) minV = node->deviceMinVal;
OLDCODE	if (minV < node->minVal) minV = node->minVal;
OLDCODE	if (maxV > node->deviceMaxVal) maxV = node->deviceMaxVal;
OLDCODE	if (maxV > node->maxVal) maxV = node->maxVal;
OLDCODE	possibleValueSpread = maxV-minV +1;
OLDCODE
OLDCODE	/* bounds check the float value */
OLDCODE	if (*fV<(float)0.0) *fV=(float)0.0;
OLDCODE	if (*fV>(float)1.0) *fV=(float)1.0;
OLDCODE	tv = *fV * possibleValueSpread + minV;
OLDCODE	*value = (int) tv;
OLDCODE
OLDCODE	#ifdef MIDIVERBOSE
OLDCODE	printf ("determinefromfloat, minV %d maxV %d, val %d fv %f tv %f\n",minV, maxV,*value, *fV,tv);
OLDCODE	#endif
OLDCODE
OLDCODE}
OLDCODE
OLDCODE/* go through the node, and create a new int value, and a new float value, from an int value */
OLDCODEstatic void determineMIDIValFromInt (struct X3D_MidiControl *node, int *value, float *fV) {
OLDCODE	int minV, maxV, possibleValueSpread;
OLDCODE
OLDCODE	minV = 0;  maxV = 100000;
OLDCODE	if (minV < node->deviceMinVal) minV = node->deviceMinVal;
OLDCODE	if (minV < node->minVal) minV = node->minVal;
OLDCODE	if (maxV > node->deviceMaxVal) maxV = node->deviceMaxVal;
OLDCODE	if (maxV > node->maxVal) maxV = node->maxVal;
OLDCODE		
OLDCODE	possibleValueSpread = maxV-minV +1;
OLDCODE	if (*value < minV) *value = minV;
OLDCODE	if (*value > maxV) *value = maxV;
OLDCODE	if (possibleValueSpread != 0) {
OLDCODE		*fV = ((float) *value + minV) / (possibleValueSpread);
OLDCODE	} else { 
OLDCODE		*fV = (float) 0.0;
OLDCODE	}
OLDCODE	#ifdef MIDIVERBOSE
OLDCODE	printf ("determine, minV %d maxV %d, val %d fv %f\n",minV, maxV,*value, *fV);
OLDCODE	#endif
OLDCODE	
OLDCODE}
OLDCODE
OLDCODE
OLDCODE/* send this node out to the ReWire network */
OLDCODEstatic void sendNodeToReWire(struct X3D_MidiControl *node) {
OLDCODE	char buf[200];
OLDCODE	//ppEAIServ p = gglobal()->EAIServ.prv;
OLDCODE	#ifdef MIDIVERBOSE
OLDCODE	if (node->controllerPresent) {
OLDCODE		printf ("sendNodeToReWire - controller present\n");
OLDCODE	} else {
OLDCODE		printf ("sendNodeToReWire - controller NOT present\n");
OLDCODE	}
OLDCODE	#endif
OLDCODE
OLDCODE
OLDCODE	if (node->controllerPresent) {
OLDCODE		if (node->_intControllerType == MIDI_CONTROLLER_FADER) {
OLDCODE			sprintf (buf,"RW %d %d %d %d %d\n",
OLDCODE				node->_bus, node->_channel, node->_controller, 
OLDCODE				node->_intControllerType, node->intValue);
OLDCODE				#ifdef MIDIVERBOSE
OLDCODE				printf ("sendNodeToReWire,  fader: bus: %d channel: %d controller: %d type: %d value: %d\n",
OLDCODE                                node->_bus, node->_channel, node->_controller,
OLDCODE                                node->_intControllerType, node->intValue);
OLDCODE				#endif
OLDCODE		} else {
OLDCODE			sprintf (buf,"RW %d %d %d %d %d\n",
OLDCODE				node->_bus, node->_channel, node->_sentVel,
OLDCODE				node->_intControllerType, node->intValue);
OLDCODE				#ifdef MIDIVERBOSE
OLDCODE				printf ("sendNodeToReWire,  buttonPress: bus: %d channel: %d vel: %d conttype: %d val: %d\n",
OLDCODE				node->_bus, node->_channel, node->_sentVel,
OLDCODE				node->_intControllerType, node->intValue);
OLDCODE				#endif
OLDCODE		}
OLDCODE
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		printf ("sendNodeToReWire: sending string:%s:\n",buf);
OLDCODE		#endif
OLDCODE
OLDCODE		EAI_send_string(buf,gglobal()->EAIServ.EAIMIDIlistenfd);
OLDCODE	}	
OLDCODE}
OLDCODE
OLDCODE/* return parameters associated with this name. returns TRUE if this device has been added by
OLDCODEthe ReWire system */
OLDCODE
OLDCODEstatic int ReWireDeviceIndex (struct X3D_MidiControl *node, int *bus, int *internChan,  
OLDCODE	int *controller, int *cmin, int *cmax, int *ctptr) {
OLDCODE	int ctr;
OLDCODE	int match;
OLDCODE
OLDCODE	ppComponent_Networking p;
OLDCODE	int dev = node->_deviceNameIndex;
OLDCODE	int cont = node->_controllerIndex;
OLDCODE	struct ReWireNamenameStruct *ReWireNamenames;
OLDCODE	struct ReWireDeviceStruct *ReWireDevices;
OLDCODE	struct tComponent_Networking t;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	t = tg->Component_Networking;
OLDCODE	ReWireNamenames = (struct ReWireNamenameStruct *)t.ReWireNamenames;
OLDCODE	ReWireDevices = (struct ReWireDeviceStruct *)t.ReWireDevices;
OLDCODE	p = (ppComponent_Networking)tg->Component_Networking.prv;
OLDCODE
OLDCODE	#ifdef MIDIVERBOSE
OLDCODE	printf ("ReWireDeviceIndex, tblsz %d, looking for a device for bus %d channel %d and controller %d\n",
OLDCODE			t->ReWireDevicetableSize, *bus,node->channel,*controller);
OLDCODE	#endif
OLDCODE	
OLDCODE	/* is this a duplicate name and type? types have to be same,
OLDCODE	   name lengths have to be the same, and the strings have to be the same.
OLDCODE	*/
OLDCODE	for (ctr=0; ctr<=t.ReWireDevicetableSize; ctr++) {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE			printf ("ReWireDeviceIndex: comparing %d %d to %d %d\n",dev,ReWireDevices[ctr].encodedDeviceName,
OLDCODE			cont, ReWireDevices[ctr].encodedControllerName); 
OLDCODE		#endif
OLDCODE
OLDCODE		/* if the MidiControl node is set for "ButtonPress" we care ONLY about whether the
OLDCODE		   device is present or not - ignore the encodedControllerName field */
OLDCODE		/* possible match? */
OLDCODE		if ((dev == ReWireDevices[ctr].encodedDeviceName) && (cont == ReWireDevices[ctr].encodedControllerName)) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("ReWireDeviceIndex, possible match; dev, cont == ReWireDevices[]encodedDevice, encodedControl\n");
OLDCODE			printf (" for ReWireDevices[%d]: bus %d node->channel %d comparing to our requested channel %d\n",ctr,ReWireDevices[ctr].bus,ReWireDevices[ctr].channel,node->channel);
OLDCODE			#endif
OLDCODE#undef MIDIVERBOSE
OLDCODE
OLDCODE			match = FALSE;
OLDCODE
OLDCODE			/* ok - so if the user asked for a specific channel, look for this, if not, make first match */
OLDCODE			/* NOTE that MIDI USER DEVICES use channels 1-16, but the MIDI SPEC says 0-15 */
OLDCODE
OLDCODE			if ((node->channel >= 1) && (node->channel<=16)) {
OLDCODE				#ifdef MIDIVERBOSE
OLDCODE				printf ("user specified channel is %d, we map it to %d\n",node->channel, node->channel-1);
OLDCODE				#endif
OLDCODE
OLDCODE				if (((node->channel)-1)  == ReWireDevices[ctr].channel) {
OLDCODE					match = TRUE;
OLDCODE				}
OLDCODE			} else {
OLDCODE				match = TRUE;
OLDCODE			}
OLDCODE
OLDCODE			if (match) {
OLDCODE
OLDCODE				#ifdef MIDIVERBOSE
OLDCODE				printf ("MATCHED!\n");
OLDCODE				#endif
OLDCODE
OLDCODE				*bus = ReWireDevices[ctr].bus;
OLDCODE				*internChan = ReWireDevices[ctr].channel;
OLDCODE				*controller = ReWireDevices[ctr].controller;
OLDCODE				*cmin = ReWireDevices[ctr].cmin;
OLDCODE				*cmax = ReWireDevices[ctr].cmax;
OLDCODE				/* we know the ctype from the x3d file... 
OLDCODE				     *ctptr = ReWireDevices[ctr].ctype; */
OLDCODE	
OLDCODE				/* is this the first time this node is associated with this entry? */
OLDCODE				if (ReWireDevices[ctr].node == NULL) {
OLDCODE					ReWireDevices[ctr].node = node;
OLDCODE					#ifdef MIDIVERBOSE
OLDCODE					printf ("ReWireDeviceIndex, tying node %d (%s) to index %d\n",node, stringNodeType(node->_nodeType),ctr);
OLDCODE					#endif
OLDCODE				}
OLDCODE
OLDCODE				return TRUE;
OLDCODE			} else {
OLDCODE				#ifdef MIDIVERBOSE
OLDCODE				printf ("did not match...\n");
OLDCODE				#endif
OLDCODE			}
OLDCODE
OLDCODE		}
OLDCODE	}
OLDCODE
OLDCODE	/* nope, not duplicate */
OLDCODE	#ifdef MIDIVERVOSE
OLDCODE	printf ("ReWireDeviceIndex: name not added, name not found\n");
OLDCODE	#endif
OLDCODE	return FALSE; /* name not added, name not found */ 
OLDCODE}
OLDCODE
OLDCODE/* returns TRUE if register goes ok, FALSE if already registered */
OLDCODEstatic int ReWireDeviceRegister (int dev, int cont, int bus, int channel, int controller, int cmin, int cmax, int ctptr) {
OLDCODE	int ctr;
OLDCODE
OLDCODE	ppComponent_Networking p;
OLDCODE	struct ReWireNamenameStruct *ReWireNamenames;
OLDCODE	struct ReWireDeviceStruct *ReWireDevices;
OLDCODE	struct tComponent_Networking t;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	t = tg->Component_Networking;
OLDCODE	ReWireNamenames = (struct ReWireNamenameStruct *)t.ReWireNamenames;
OLDCODE	ReWireDevices = (struct ReWireDeviceStruct *)t.ReWireDevices;
OLDCODE	p = (ppComponent_Networking)tg->Component_Networking.prv;
OLDCODE	
OLDCODE	/* is this a duplicate name and type? types have to be same,
OLDCODE	   name lengths have to be the same, and the strings have to be the same.
OLDCODE	*/
OLDCODE	for (ctr=0; ctr<=t.ReWireDevicetableSize; ctr++) {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE			printf ("comparing %d %d to %d %d\n",dev,ReWireDevices[ctr].encodedDeviceName,
OLDCODE			cont, ReWireDevices[ctr].encodedControllerName); 
OLDCODE		#endif
OLDCODE
OLDCODE		if ((dev==ReWireDevices[ctr].encodedDeviceName) &&
OLDCODE			(channel == ReWireDevices[ctr].channel) &&
OLDCODE			(cont == ReWireDevices[ctr].encodedControllerName)) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE				printf ("ReWireDeviceRegister, FOUND IT at %d bus %d channel %d controller %d\n",ctr,
OLDCODE						ReWireDevices[ctr].bus,ReWireDevices[ctr].channel,ReWireDevices[ctr].controller);
OLDCODE			#endif
OLDCODE			return FALSE; /* name found */
OLDCODE		}
OLDCODE	}
OLDCODE
OLDCODE	/* nope, not duplicate */
OLDCODE	t.ReWireDevicetableSize ++;
OLDCODE	
OLDCODE	/* ok, we got a name and a type */
OLDCODE	if (t.ReWireDevicetableSize >= p->MAXReWireDevices) {
OLDCODE		/* oooh! not enough room at the table */
OLDCODE		p->MAXReWireDevices += 1024; /* arbitrary number */
OLDCODE		ReWireDevices = (struct ReWireDeviceStruct*)REALLOC (ReWireDevices, sizeof(*ReWireDevices) * p->MAXReWireDevices);
OLDCODE	}
OLDCODE	
OLDCODE	ReWireDevices[t.ReWireDevicetableSize].bus = bus;
OLDCODE	ReWireDevices[t.ReWireDevicetableSize].channel = channel;
OLDCODE	ReWireDevices[t.ReWireDevicetableSize].controller = controller;
OLDCODE	ReWireDevices[t.ReWireDevicetableSize].cmin = cmin; 
OLDCODE	ReWireDevices[t.ReWireDevicetableSize].cmax = cmax;
OLDCODE	ReWireDevices[t.ReWireDevicetableSize].ctype = ctptr; /* warning - this is just MIDI_CONTROLLER_FADER... */
OLDCODE	ReWireDevices[t.ReWireDevicetableSize].encodedDeviceName = dev;
OLDCODE	ReWireDevices[t.ReWireDevicetableSize].encodedControllerName = cont;
OLDCODE	ReWireDevices[t.ReWireDevicetableSize].node = NULL;
OLDCODE	#ifdef MIDIVERBOSE
OLDCODE		printf ("ReWireDeviceRegister, new entry at %d",t.ReWireDevicetableSize);
OLDCODE		printf ("	Device %s (%d) controller %s (%d) ", ReWireNamenames[dev].name,
OLDCODE					dev, ReWireNamenames[cont].name, cont);
OLDCODE		printf ("	bus %d channel %d controller %d cmin %d cmax %d\n",bus, channel, 
OLDCODE				controller, cmin, cmax);
OLDCODE	#endif
OLDCODE	return TRUE; /* name not found, but, requested */
OLDCODE}
OLDCODE
OLDCODEvoid registerReWireNode(struct X3D_Node *node) {
OLDCODE	int count;
OLDCODE	uintptr_t *myptr;
OLDCODE	ppComponent_Networking p = (ppComponent_Networking)gglobal()->Component_Networking.prv;
OLDCODE
OLDCODE	if (node == 0) {
OLDCODE		printf ("error in registerReWireNode; somehow the node datastructure is zero \n");
OLDCODE		return;
OLDCODE	}
OLDCODE
OLDCODE	if (node->_nodeType != NODE_MidiControl) return;
OLDCODE
OLDCODE	p->MidiNodes = (uintptr_t *) REALLOC (p->MidiNodes,sizeof (uintptr_t *) * (p->num_MidiNodes+1));
OLDCODE	myptr = p->MidiNodes;
OLDCODE
OLDCODE	/* does this event exist? */
OLDCODE	for (count=0; count <p->num_MidiNodes; count ++) {
OLDCODE		if (*myptr == (uintptr_t) node) {
OLDCODE			printf ("registerReWireNode, already this node\n"); 
OLDCODE			return;
OLDCODE		}	
OLDCODE		myptr++;
OLDCODE	}
OLDCODE
OLDCODE
OLDCODE	/* now, put the function pointer and data pointer into the structure entry */
OLDCODE	*myptr = (uintptr_t) node;
OLDCODE
OLDCODE	p->num_MidiNodes++;
OLDCODE}
OLDCODE
OLDCODE
OLDCODE/* return a node assoicated with this name. If the name exists, return the previous node. If not, return
OLDCODEthe new node */
OLDCODEstatic int ReWireNameIndex (char *name) {
OLDCODE	int ctr;
OLDCODE	
OLDCODE	ppComponent_Networking p;
OLDCODE	struct ReWireNamenameStruct *ReWireNamenames;
OLDCODE	struct ReWireDeviceStruct *ReWireDevices;
OLDCODE	struct tComponent_Networking t;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	t = tg->Component_Networking;
OLDCODE	ReWireNamenames = (struct ReWireNamenameStruct *)t.ReWireNamenames;
OLDCODE	ReWireDevices = (struct ReWireDeviceStruct *)t.ReWireDevices;
OLDCODE	p = (ppComponent_Networking)tg->Component_Networking.prv;
OLDCODE	
OLDCODE	/* printf ("ReWireNameIndex, looking for %s\n",name); */
OLDCODE	/* is this a duplicate name and type? types have to be same,
OLDCODE	   name lengths have to be the same, and the strings have to be the same.
OLDCODE	*/
OLDCODE	for (ctr=0; ctr<=t.ReWireNametableSize; ctr++) {
OLDCODE		/* printf ("ReWireNameIndex, comparing %s to %s\n",name,p->ReWireNamenames[ctr].name); */
OLDCODE		if (strcmp(name,ReWireNamenames[ctr].name)==0) {
OLDCODE			/* printf ("ReWireNameIndex, FOUND IT at %d\n",ctr); */
OLDCODE			return ctr;
OLDCODE		}
OLDCODE	}
OLDCODE
OLDCODE	/* nope, not duplicate */
OLDCODE
OLDCODE	t.ReWireNametableSize ++;
OLDCODE
OLDCODE	/* ok, we got a name and a type */
OLDCODE	if (t.ReWireNametableSize >= p->MAXReWireNameNames) {
OLDCODE		/* oooh! not enough room at the table */
OLDCODE		p->MAXReWireNameNames += 1024; /* arbitrary number */
OLDCODE		ReWireNamenames = (struct ReWireNamenameStruct*)REALLOC (ReWireNamenames, sizeof(*ReWireNamenames) * p->MAXReWireNameNames);
OLDCODE	}
OLDCODE
OLDCODE	ReWireNamenames[t.ReWireNametableSize].name = STRDUP(name);
OLDCODE	/* printf ("ReWireNameIndex, new entry at %d\n",t.ReWireNametableSize); */
OLDCODE	return t.ReWireNametableSize;
OLDCODE}
OLDCODE
OLDCODE//#if defined(REWIRE_SERVER)
OLDCODE///* make sure the EAI port is turned on... */
OLDCODE//static int requestToStartEAIdone = FALSE;
OLDCODE//#endif
OLDCODE
OLDCODEstatic void midiStartEAI() {
OLDCODE#if !defined(REWIRE_SERVER)
OLDCODE    ConsoleMessage("MIDI disabled at compile time.\n");
OLDCODE#else
OLDCODE	char myline[2000];
OLDCODE	ppComponent_Networking p = (ppComponent_Networking)gglobal()->Component_Networking.prv;
OLDCODE	if (!p->requestToStartEAIdone) {
OLDCODE		if (strlen(REWIRE_SERVER) > 1000) return; /* bounds check this compile time value */
OLDCODE
OLDCODE		printf ("MidiControl - turning EAI on\n");
OLDCODE		sprintf (myline,"%s %u &",REWIRE_SERVER,getpid());
OLDCODE		create_MIDIEAI();
OLDCODE		if (system (myline)==127) {
OLDCODE			strcpy (myline,"could not start ");
OLDCODE			strncat (myline, REWIRE_SERVER,1000);
OLDCODE			ConsoleMessage (myline);
OLDCODE		}
OLDCODE	}
OLDCODE	p->requestToStartEAIdone = TRUE;
OLDCODE#endif
OLDCODE}
OLDCODE
OLDCODEvoid prep_MidiControl (struct X3D_MidiControl *node) {
OLDCODE	/* get the name/device pairing */
OLDCODE	int tmp_bus = INT_ID_UNDEFINED;
OLDCODE	int tmp_controller = INT_ID_UNDEFINED;
OLDCODE	int tmp_internchan = INT_ID_UNDEFINED;
OLDCODE	int tmpdeviceMinVal = INT_ID_UNDEFINED;
OLDCODE	int tmpdeviceMaxVal = INT_ID_UNDEFINED;
OLDCODE	int tmpintControllerType = INT_ID_UNDEFINED;
OLDCODE	int controllerPresent = FALSE;
OLDCODE
OLDCODE	/* is there anything to do? */
OLDCODE	#ifdef MIDIVERBOSE
OLDCODEprintf ("prep_MidiControl, for node %d, change %d ichange %d _bus %d channel %d (_channel %d) _controller %d",node,
OLDCODE	node->_change, node->_ichange,
OLDCODE	node->_bus,node->channel,
OLDCODE	node->_channel, node->_controller);
OLDCODEprintf (" _deviceNameIndex %d _controllerIndex %d\n",node->_deviceNameIndex, node->_controllerIndex);
OLDCODE	#endif
OLDCODE
OLDCODE/*
OLDCODE        int _bus;
OLDCODE        int _butPr;
OLDCODE        int _channel;
OLDCODE        int _controller;
OLDCODE        int _controllerIndex;
OLDCODE        int _deviceNameIndex;
OLDCODE        int _intControllerType;
OLDCODE        int _oldintValue;
OLDCODE        int _sentVel;
OLDCODE        int _vel;
OLDCODE        int autoButtonPress;
OLDCODE        int buttonPress;
OLDCODE        int channel;
OLDCODE        struct Uni_String *controller;
OLDCODE        int controllerPresent;
OLDCODE        struct Uni_String *controllerType;
OLDCODE        int deviceMaxVal;
OLDCODE        int deviceMinVal;
OLDCODE        struct Uni_String *deviceName;
OLDCODE        float floatValue;
OLDCODE        int highResolution;
OLDCODE        int intValue;
OLDCODE        int maxVal;
OLDCODE        int minVal;
OLDCODE        float pressLength;
OLDCODE        double pressTime;
OLDCODE        int useIntValue;
OLDCODE        int velocity;
OLDCODE*/
OLDCODE
OLDCODE	/* first time through, the _ichange will be 0. Lets ensure that we start events properly. */
OLDCODE	if (node->_ichange == 0) {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		printf ("Midi Node being initialized\n");
OLDCODE		#endif
OLDCODE		midiStartEAI();
OLDCODE		node->controllerPresent = 999; /* force event for correct controllerPresent */
OLDCODE
OLDCODE	}
OLDCODE
OLDCODE	/* if the deviceName has changed, or has never been found... */
OLDCODE	if ((node->deviceName->touched > 0) || (node->_deviceNameIndex < 0)) {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		printf ("NODE DEVICE NAME CHANGED now is %s\n",node->deviceName->strptr);
OLDCODE		#endif
OLDCODE		node->_deviceNameIndex = ReWireNameIndex(node->deviceName->strptr);
OLDCODE		node->deviceName->touched = 0;
OLDCODE		if (node->_deviceNameIndex>=0) MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, deviceName));
OLDCODE	}
OLDCODE
OLDCODE	/* controller type... */
OLDCODE	if (node->controllerType->touched != 0) {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		printf ("CONTROLLERTYPE CHANGED\n");
OLDCODE		#endif
OLDCODE		node->controllerType->touched = 0;  
OLDCODE		if (strcmp(node->controllerType->strptr,"Slider") == 0) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("midi controller is a fader\n");
OLDCODE			#endif
OLDCODE
OLDCODE			node->_intControllerType = MIDI_CONTROLLER_FADER;
OLDCODE		} else if (strcmp(node->controllerType->strptr,"ButtonPress") == 0) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("midi controller is a keypress\n");
OLDCODE			#endif
OLDCODE
OLDCODE			node->_intControllerType = MIDI_CONTROLLER_KEYPRESS;
OLDCODE			/* we have a special controller name for the "keypress" nodes; controllers are not valid here */
OLDCODE			node->controller = newASCIIString(BUTTON_PRESS_STRING);
OLDCODE		} else {
OLDCODE			ConsoleMessage ("MidiControl - unknown controllerType :%s:\n",node->controllerType->strptr);
OLDCODE		}
OLDCODE
OLDCODE		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, controllerType));
OLDCODE	}
OLDCODE
OLDCODE	/* is this a controller, or a keypress node? Keypresses go directly to the bus/channel, they do not have
OLDCODE	   a controller number, so we have a default controller number */
OLDCODE	if ((node->controller->touched > 0) || (node->_controllerIndex < 0)) {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		printf ("NODE CONTROLLER CHANGED now is %s\n",node->controller->strptr);
OLDCODE		#endif
OLDCODE		node->_controllerIndex = ReWireNameIndex(node->controller->strptr);
OLDCODE		node->controller->touched = 0;
OLDCODE		if (node->_controllerIndex>=0) MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, controller));
OLDCODE	}
OLDCODE
OLDCODE
OLDCODE
OLDCODE	/* look for the end point to be there - if a "Slider" we need the device AND controller; if
OLDCODE	   "ButtonPress" need just the device */
OLDCODE	tmpintControllerType = node->_intControllerType;
OLDCODE	tmp_bus = node->_bus;
OLDCODE	tmp_internchan = node->_channel;
OLDCODE	if (node->_intControllerType == MIDI_CONTROLLER_KEYPRESS) {
OLDCODE		/* this is a keypress to the midi bus/channel; it is not really a controller */
OLDCODE		node->_controller = ReWireNameIndex(BUTTON_PRESS_STRING);
OLDCODE
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		printf ("FYI - ReWireNameIndex for keypress is %d\n",node->_controller);
OLDCODE		#endif
OLDCODE	}
OLDCODE	tmp_controller = node->_controller;
OLDCODE
OLDCODE	controllerPresent = ReWireDeviceIndex (node,
OLDCODE				&tmp_bus,		/* return the bus, if it is available */
OLDCODE				&tmp_internchan,	/* return the channel, if it is available */
OLDCODE				&tmp_controller,	/* return the controller, if it is available */
OLDCODE				&tmpdeviceMinVal,	/* return the device min val, if available */
OLDCODE				&tmpdeviceMaxVal,	/* return the device max val, if available */
OLDCODE				&tmpintControllerType);	/* return the controller type (slider, button) if availble */
OLDCODE
OLDCODE	#ifdef MIDIVERBOSE
OLDCODE		printf ("compile_midiControl, for %d %d controllerPresent %d, node->controllerPresent %d\n",
OLDCODE				ReWireNameIndex(node->deviceName->strptr),
OLDCODE				ReWireNameIndex(node->controller->strptr),
OLDCODE				controllerPresent, node->controllerPresent);
OLDCODE	#endif
OLDCODE
OLDCODE	if (controllerPresent != node->controllerPresent) {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		if (controllerPresent) printf ("controllerPresent changed FOUND CONTROLLER\n");
OLDCODE		if (!controllerPresent) printf ("controllerPresent changed LOST CONTROLLER\n");
OLDCODE		#endif
OLDCODE		node->controllerPresent = controllerPresent;
OLDCODE		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, controllerPresent));
OLDCODE	}
OLDCODE
OLDCODE	if (controllerPresent) {
OLDCODE		if (tmp_bus != node->_bus) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("INTERNAL: bus changed from %d to %d\n",node->_bus, tmp_bus);
OLDCODE			#endif
OLDCODE			node->_bus = tmp_bus;
OLDCODE		}
OLDCODE		if (tmp_internchan != node->_channel) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("INTERNAL: channel changed from %d to %d\n",node->_channel, tmp_internchan);
OLDCODE			#endif
OLDCODE			node->_channel = tmp_internchan;
OLDCODE		}
OLDCODE		if (tmp_controller != node->_controller) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("INTERNAL: controller changed from %d to %d\n",node->_controller, tmp_controller);
OLDCODE			#endif
OLDCODE			node->_controller = tmp_controller;
OLDCODE		}
OLDCODE		if (tmpdeviceMinVal != node->deviceMinVal) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("deviceMinVal changed from %d to %d\n",node->deviceMinVal, tmpdeviceMinVal);
OLDCODE			#endif
OLDCODE			node->deviceMinVal = tmpdeviceMinVal;
OLDCODE			MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, deviceMinVal));
OLDCODE		}
OLDCODE		if (node->_vel != node->velocity) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("velocity changed from %d to %d\n",node->_vel, node->velocity);
OLDCODE			#endif
OLDCODE			if (node->velocity > 127) node->velocity = 127;
OLDCODE			if (node->velocity <0) node->velocity = 0;
OLDCODE
OLDCODE			node->_vel = node->velocity;
OLDCODE			MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, velocity));
OLDCODE		}
OLDCODE		if (tmpdeviceMaxVal != node->deviceMaxVal) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("deviceMaxVal changed from %d to %d\n",node->deviceMaxVal, tmpdeviceMaxVal);
OLDCODE			#endif
OLDCODE			node->deviceMaxVal = tmpdeviceMaxVal;
OLDCODE			MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, deviceMaxVal));
OLDCODE		}
OLDCODE		if (tmpintControllerType != node->_intControllerType) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("intControllerType  changed from %d to %d\n",node->_intControllerType, tmpintControllerType);
OLDCODE			#endif
OLDCODE
OLDCODE			node->_intControllerType = tmpintControllerType;
OLDCODE			MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MidiControl, controllerType));
OLDCODE			switch (node->_intControllerType) {
OLDCODE				case MIDI_CONTROLLER_FADER:
OLDCODE					verify_Uni_String(node->controllerType,"Slider");
OLDCODE					break;
OLDCODE				case MIDI_CONTROLLER_KEYPRESS:
OLDCODE					verify_Uni_String(node->controllerType,"ButtonPress");
OLDCODE					break;
OLDCODE				case MIDI_CONTROLLER_UNKNOWN:
OLDCODE					verify_Uni_String(node->controllerType,"Unknown");
OLDCODE					break;
OLDCODE				default:
OLDCODE					ConsoleMessage ("Unknown Controller Type for MIDI node, have %d\n",
OLDCODE						node->_intControllerType);
OLDCODE			}
OLDCODE		}
OLDCODE	} else {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		printf ("compile_MidiControl - device not present yet\n");
OLDCODE		#endif
OLDCODE	}
OLDCODE
OLDCODE
OLDCODE	MARK_NODE_COMPILED 
OLDCODE}
OLDCODE
OLDCODEvoid ReWireRegisterMIDI (char *str) {
OLDCODE	int curBus;
OLDCODE	int curChannel;
OLDCODE	int curMin;
OLDCODE	int curMax;
OLDCODE	int curType;
OLDCODE	int curController;
OLDCODE
OLDCODE	char *EOT;
OLDCODE
OLDCODE	int encodedDeviceName = INT_ID_UNDEFINED;
OLDCODE	int encodedControllerName = INT_ID_UNDEFINED;
OLDCODE	
OLDCODE	ppComponent_Networking p;
OLDCODE	struct ReWireNamenameStruct *ReWireNamenames;
OLDCODE	struct ReWireDeviceStruct *ReWireDevices;
OLDCODE	struct tComponent_Networking t;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	t = tg->Component_Networking;
OLDCODE	ReWireNamenames = (struct ReWireNamenameStruct *)t.ReWireNamenames;
OLDCODE	ReWireDevices = (struct ReWireDeviceStruct *)t.ReWireDevices;
OLDCODE	p = (ppComponent_Networking)tg->Component_Networking.prv;
OLDCODE
OLDCODE
OLDCODE	/*
OLDCODE	   "Hardware Interface 1" 5 0 
OLDCODE	   "Melody Automation" 5 1 
OLDCODE	   1 "Mod Wheel" 1 127 0
OLDCODE	   5 "Portamento" 1 127 0
OLDCODE	   7 "Master Level" 1 127 0
OLDCODE	   9 "Oscillator B Decay" 1 127 0
OLDCODE	   12 "Oscillator B Sustain" 1 127 0
OLDCODE	   14 "Filter Env Attack" 1 127 0
OLDCODE	   15 "Filter Env Decay" 1 127 0
OLDCODE	   16 "Filter Env Sustain" 1 127 0
OLDCODE	   17 "Filter Env Release" 1 127 0
OLDCODE	   18 "Filter Env Amount" 1 127 0
OLDCODE	   19 "Filter Env Invert" 3 1 0
OLDCODE	   21 "Oscillator B Octave" 3 8 0
OLDCODE	   79 "Body Type" 3 4 0
OLDCODE	 */	
OLDCODE
OLDCODE	/* set the device tables to the starting value. */
OLDCODE	t.ReWireDevicetableSize = -1;
OLDCODE
OLDCODE	#ifdef MIDIVERBOSE
OLDCODE	printf ("ReWireRegisterMIDI - have string :%s:\n",str);
OLDCODE	#endif
OLDCODE
OLDCODE
OLDCODE	while (*str != '\0') {
OLDCODE		while (*str == '\n') str++;
OLDCODE		/* is this a new device? */
OLDCODE
OLDCODE		if (*str == '"') {
OLDCODE			str++;
OLDCODE			EOT = strchr (str,'"');
OLDCODE			if (EOT != NULL) *EOT = '\0'; else {
OLDCODE				printf ("ReWireRegisterMidi, expected string here: %s\n",str);
OLDCODE			}
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("device name is :%s:\n",str);
OLDCODE			#endif
OLDCODE			encodedDeviceName = ReWireNameIndex(str);
OLDCODE			str = EOT+1;
OLDCODE			sscanf (str, "%d %d",&curBus, &curChannel);
OLDCODE
OLDCODE			/* make an entry for devices that have NO controllers, maybe only buttonPresses */
OLDCODE			encodedControllerName = ReWireNameIndex(BUTTON_PRESS_STRING);
OLDCODE			curController = -1; curMin = 0; curMax = 127; curType = MIDI_CONTROLLER_KEYPRESS;
OLDCODE
OLDCODE			if (!ReWireDeviceRegister(encodedDeviceName, encodedControllerName, curBus, 
OLDCODE				curChannel, curController, curMin, curMax, curType)) {
OLDCODE				#ifdef MIDIVERBOSE
OLDCODE				printf ("ReWireRegisterMIDI, duplicate device for %s %s\n",
OLDCODE					p->ReWireNamenames[encodedDeviceName].name, p->ReWireNamenames[encodedControllerName].name); 
OLDCODE				#endif
OLDCODE			}
OLDCODE
OLDCODE		} else if (*str == '\t') {
OLDCODE			str++;
OLDCODE			sscanf (str, "%d", &curController);
OLDCODE			EOT = strchr (str,'"');
OLDCODE			if (EOT != NULL) str = EOT+1; else {
OLDCODE				printf ("ReWireRegisterMidi, expected string here: %s\n",str);
OLDCODE			}
OLDCODE			EOT = strchr (str,'"');
OLDCODE			if (EOT != NULL) *EOT = '\0'; else {
OLDCODE				printf ("ReWireRegisterMidi, expected string here: %s\n",str);
OLDCODE			}
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("	controllername is :%s: ",str);
OLDCODE			#endif
OLDCODE			encodedControllerName = ReWireNameIndex(str);
OLDCODE			str = EOT+1;
OLDCODE			sscanf (str, "%d %d %d",&curType, &curMax, &curMin);
OLDCODE
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("(encodedDevice:%d encodedController:%d)  Bus:%d Channel:%d controller %d curMin %d curMax %d curType %d\n",encodedDeviceName, encodedControllerName, curBus, curChannel,
OLDCODE				curController, curMin, curMax, curType);
OLDCODE			#endif
OLDCODE		
OLDCODE			/* register the info for this controller */
OLDCODE			if (!ReWireDeviceRegister(encodedDeviceName, encodedControllerName, curBus, 
OLDCODE				curChannel, curController, curMin, curMax, curType)) {
OLDCODE				#ifdef MIDIVERBOSE
OLDCODE				printf ("ReWireRegisterMIDI, duplicate device for %s %s\n",
OLDCODE					p->ReWireNamenames[encodedDeviceName].name, p->ReWireNamenames[encodedControllerName].name); 
OLDCODE				#endif
OLDCODE			}
OLDCODE
OLDCODE		} else {
OLDCODE			if (*str != ' ') printf ("ReWireRegisterMidi - garbage (%c)  at:%s\n",*str,str);
OLDCODE		}
OLDCODE		
OLDCODE		/* skip to the end of the line */
OLDCODE		while (*str >= ' ') str++;
OLDCODE		while (*str == '\n') str++;
OLDCODE	}
OLDCODE}
OLDCODE
OLDCODEvoid do_MidiControl (void *this) {
OLDCODE	struct X3D_MidiControl* node;
OLDCODE	int value;
OLDCODE	float fV;
OLDCODE	int sendEvent;
OLDCODE
OLDCODE	node = (struct X3D_MidiControl*) this;
OLDCODE
OLDCODE	#ifdef MIDIVERBOSE
OLDCODE	printf ("do MidiControl for node %s\n",stringNodeType(node->_nodeType));
OLDCODE	#endif
OLDCODE	if (NODE_MidiControl == node->_nodeType) {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		printf ("ReWire change %d %d ", node->_ichange, node->_change); 
OLDCODE
OLDCODE		printf ("bus %d channel %d(%d) controller :%s: controllerType :%s: device :%s: ",
OLDCODE			node->_bus, node->_channel, node->channel,
OLDCODE			node->controller->strptr, node->controllerType->strptr, node->deviceName->strptr);
OLDCODE		printf (" devp %d fv %f iv %d hr %d ct %d intVal %d max %d min %d\n",
OLDCODE			node->controllerPresent, node->floatValue, node->intValue, node->highResolution,
OLDCODE			node->_intControllerType, node->intValue, node->maxVal, node->minVal);
OLDCODE		#endif
OLDCODE
OLDCODE		value = node->intValue;
OLDCODE		fV = node->floatValue;
OLDCODE		if (node->useIntValue) {
OLDCODE			determineMIDIValFromInt (node, &value, &fV);
OLDCODE		} else {
OLDCODE			determineMIDIValFromFloat (node, &value, &fV); 
OLDCODE		}
OLDCODE
OLDCODE		/* should we send this event? */
OLDCODE		sendEvent = FALSE;
OLDCODE		if (node->_intControllerType == MIDI_CONTROLLER_FADER) {
OLDCODE			if (value != node->_oldintValue) sendEvent = TRUE;
OLDCODE		} else {
OLDCODE			/* are we automatically generating buttonPresses? */
OLDCODE			if (node->autoButtonPress) {
OLDCODE				/* send event if value has changed */
OLDCODE				if (value != node->_oldintValue) {
OLDCODE					sendEvent = TRUE;
OLDCODE					node->_sentVel = node->_vel;   /* this is a Note On */
OLDCODE					node->pressTime = TickTime();	   /* time button pressed */
OLDCODE					#ifdef MIDIVERBOSE
OLDCODE					printf ("auto Note %d on at %lf\n",value, TickTime());
OLDCODE					#endif
OLDCODE				}
OLDCODE
OLDCODE				/* have we timed out? */
OLDCODE				if (node->pressTime > 0.001) {
OLDCODE					if (TickTime() > (node->pressTime + node->pressLength))  {
OLDCODE						#ifdef MIDIVERBOSE
OLDCODE						printf ("note timed out at %lf, sending note_off for pressTime %lf \n",TickTime(), node->pressTime);
OLDCODE						#endif
OLDCODE
OLDCODE						sendEvent = TRUE;
OLDCODE						node->_sentVel = -1;		/* this is a Note Off */
OLDCODE						node->pressTime = (double)0.0;
OLDCODE					}
OLDCODE				}
OLDCODE			} else {
OLDCODE				/* send event if buttonPress changes state */
OLDCODE				if (node->_butPr != node->buttonPress) {
OLDCODE					sendEvent = TRUE;
OLDCODE					value = node->intValue;
OLDCODE					if (node->buttonPress) {
OLDCODE						#ifdef MIDIVERBOSE
OLDCODE						printf ("have ButtonPress at %lf\n",TickTime());
OLDCODE						#endif
OLDCODE						node->_sentVel = node->_vel;   /* this is a Note On */
OLDCODE						node->pressTime = TickTime();	   /* time button pressed */
OLDCODE					} else {
OLDCODE						#ifdef MIDIVERBOSE
OLDCODE						printf ("have ButtonRelease at %lf\n",TickTime());
OLDCODE						#endif
OLDCODE						node->_sentVel = -1;		/* this is a Note Off */
OLDCODE						node->pressTime = (double)0.0;
OLDCODE					}
OLDCODE
OLDCODE					/* now, reset the toggle... */
OLDCODE					node->_butPr = node->buttonPress;
OLDCODE				}
OLDCODE			}
OLDCODE		}
OLDCODE					
OLDCODE		if (sendEvent) {
OLDCODE			node->intValue = value;
OLDCODE			node->_oldintValue = value;
OLDCODE			node->floatValue = fV;
OLDCODE			MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, intValue));
OLDCODE			MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, floatValue));
OLDCODE
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE			printf ("intValue changed - now is %d sentvel %d\n",node->intValue, node->_sentVel); 
OLDCODE			printf ("sending %d %f ",value, node->floatValue);
OLDCODE			printf ("mins %d %d maxs %d %d ",node->deviceMinVal, node->minVal, node->deviceMaxVal, node->maxVal);
OLDCODE			printf ("float %f node->floatVal\n",node->floatValue);
OLDCODE			#endif
OLDCODE
OLDCODE			sendNodeToReWire(node);
OLDCODE			update_node (X3D_NODE(node));
OLDCODE		}
OLDCODE	}	
OLDCODE}
OLDCODE
OLDCODE
OLDCODE/* a MIDI event is coming in from the EAI port! */
OLDCODEvoid ReWireMIDIControl (char *line) {
OLDCODE	long int timeDiff;
OLDCODE	int bus, channel, controller, value;
OLDCODE	int rv;
OLDCODE	int ctr;
OLDCODE	struct X3D_MidiControl *node = NULL;
OLDCODE	float fV;
OLDCODE	int sendEvent;
OLDCODE	int noteOn;
OLDCODE
OLDCODE	ppComponent_Networking p;
OLDCODE	struct ReWireNamenameStruct *ReWireNamenames;
OLDCODE	struct ReWireDeviceStruct *ReWireDevices;
OLDCODE	struct tComponent_Networking t;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	t = tg->Component_Networking;
OLDCODE	ReWireNamenames = (struct ReWireNamenameStruct *)t.ReWireNamenames;
OLDCODE	ReWireDevices = (struct ReWireDeviceStruct *)t.ReWireDevices;
OLDCODE	p = (ppComponent_Networking)tg->Component_Networking.prv;
OLDCODE
OLDCODE	sendEvent = FALSE;
OLDCODE	value = 0;
OLDCODE
OLDCODE	#ifdef MIDIVERBOSE
OLDCODE	printf ("ReWireMIDIControl: read %s\n",line);
OLDCODE	#endif
OLDCODE
OLDCODE	if ((rv=sscanf (line, "%ld %d %d %d %d",&timeDiff, &bus, &channel, &controller, &value)) != 5) {
OLDCODE		printf ("Error (%d)on reading MIDICONTROL, line %s\n",rv,line);
OLDCODE		return;
OLDCODE	}
OLDCODE
OLDCODE	/* we only look for Controller, Note On and Note Off */
OLDCODE	if ( ((channel & 0xF0) != 0x80)  &&
OLDCODE		((channel & 0xF0) != 0x90)  &&
OLDCODE		((channel & 0xF0) != 0xb0)) {
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE			printf ("ReWireMidiControl, got %x, unsupported. Dropping it\n",channel);
OLDCODE		#endif
OLDCODE		return;
OLDCODE	}
OLDCODE
OLDCODE	/* is this a Slider/Fader? */
OLDCODE	if ((channel & 0xF0) == 0xb0) {
OLDCODE		/* get channel number */
OLDCODE		channel = channel & 0x0f;
OLDCODE	
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE			printf ("ReWireMIDIControl - input timedif %ld bus %d channel %d controller %d value %d\n",
OLDCODE						timeDiff, bus, channel, controller, value);
OLDCODE			printf ("ReWireDevicetableSize %d\n",t.ReWireDevicetableSize);
OLDCODE		#endif
OLDCODE	
OLDCODE	
OLDCODE		for (ctr=0; ctr<=t.ReWireDevicetableSize; ctr++) {
OLDCODE			#ifdef REALLYMIDIVERBOSE
OLDCODE				printf ("	ind %d comparing bus %d:%d channel %d:%d and  controller %d:%d\n", 
OLDCODE					ctr, bus, ReWireDevices[ctr].bus,
OLDCODE					channel, ReWireDevices[ctr].channel,
OLDCODE					controller, ReWireDevices[ctr].controller);
OLDCODE			#endif
OLDCODE	
OLDCODE			if ((bus==ReWireDevices[ctr].bus) &&
OLDCODE				(channel == ReWireDevices[ctr].channel) &&
OLDCODE				(controller == ReWireDevices[ctr].controller)) {
OLDCODE				#ifdef MIDIVERBOSE
OLDCODE					printf ("ReWireMidiControl, FOUND IT at %d - bus %d channel %d controller %d\n",ctr,
OLDCODE						bus, channel, controller);
OLDCODE				#endif
OLDCODE	
OLDCODE				if (ReWireDevices[ctr].node == NULL) {
OLDCODE					#ifdef MIDIVERBOSE
OLDCODE					printf ("ReWireMidiEvent, node for %d is still NULL\n",ctr);
OLDCODE					#endif
OLDCODE				} else {
OLDCODE					node = ReWireDevices[ctr].node;
OLDCODE					#ifdef MIDIVERBOSE
OLDCODE					printf ("routing to device %s, controller %s ",node->deviceName->strptr, node->controller->strptr);
OLDCODE					printf (" nameIndex %d controllerIndex %d deviceMinVal %d deviceMaxVal %d minVal %d maxVal%d\n",
OLDCODE						node->_deviceNameIndex, node->_controllerIndex, node->deviceMinVal,
OLDCODE						node->deviceMaxVal, node->minVal, node->maxVal);
OLDCODE					#endif
OLDCODE	
OLDCODE					determineMIDIValFromInt (node, &value, &fV); 
OLDCODE					if (value != node->_oldintValue) sendEvent = TRUE;
OLDCODE						
OLDCODE	
OLDCODE				}
OLDCODE			}
OLDCODE		}
OLDCODE	} else {
OLDCODE		/* this is a Note on/Note off */
OLDCODE		noteOn = ((channel & 0xF0) == 0x90);
OLDCODE 
OLDCODE		/* get channel number */
OLDCODE		channel = channel & 0x0f;
OLDCODE
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		if (noteOn) printf ("ReWireMidiControl, have a note ON  channel %d, velocity %d\n",channel,value);
OLDCODE		else printf ("ReWireMidiControl, have a note OFF channel %d, velocity %d\n",channel,value);
OLDCODE		#endif
OLDCODE		for (ctr=0; ctr<=t.ReWireDevicetableSize; ctr++) {
OLDCODE			#ifdef MIDIVERBOSE
OLDCODE				printf ("	ind %d comparing bus %d:%d channel %d:%d with ecdn %d, eccn %d, cont %d cmin %d cmax %d ctype %d\n", 
OLDCODE					ctr, bus, ReWireDevices[ctr].bus,
OLDCODE					channel, ReWireDevices[ctr].channel,
OLDCODE				        ReWireDevices[ctr].encodedDeviceName,          /* index into ReWireNamenames */
OLDCODE				        ReWireDevices[ctr].encodedControllerName,      /* index into ReWireNamenames */
OLDCODE				        ReWireDevices[ctr].controller,                 /* controller number */
OLDCODE				        ReWireDevices[ctr].cmin,                       /* minimum value for this controller */
OLDCODE				        ReWireDevices[ctr].cmax,                       /* maximum value for this controller */
OLDCODE				        ReWireDevices[ctr].ctype                      /* controller type TYPE OF FADER control - not used currently */
OLDCODE				);
OLDCODE			#endif
OLDCODE
OLDCODE			/* look for the entry- we have a device with a controller of -1 for the main "device" 
OLDCODE			   so that we don't send events to the same node many times */
OLDCODE
OLDCODE			if ((bus==ReWireDevices[ctr].bus) &&
OLDCODE				(channel == ReWireDevices[ctr].channel) &&
OLDCODE				(ReWireDevices[ctr].controller == -1)) {
OLDCODE				#ifdef MIDIVERBOSE
OLDCODE					printf ("ReWireMidiControl, FOUND IT at %d\n",ctr);
OLDCODE				#endif
OLDCODE	
OLDCODE				if (ReWireDevices[ctr].node == NULL) {
OLDCODE					#ifdef MIDIVERBOSE
OLDCODE					printf ("ReWireMidiEvent, node for %d is still NULL\n",ctr);
OLDCODE					#endif
OLDCODE
OLDCODE				} else {
OLDCODE					node = ReWireDevices[ctr].node;
OLDCODE
OLDCODE					/* ok, the last 2 bytes of this event, for faders are "controller value" but
OLDCODE					   for notes they are "note velocity". This is confusing... */
OLDCODE
OLDCODE					/* should we send this event? */
OLDCODE					/* send event if buttonPress changes state */
OLDCODE					if (noteOn != node->buttonPress) {
OLDCODE						sendEvent = TRUE;
OLDCODE						if (noteOn) {
OLDCODE							#ifdef MIDIVERBOSE
OLDCODE							printf ("have ButtonPress at %lf\n",TickTime());
OLDCODE							#endif
OLDCODE							node->pressTime = TickTime();	   /* time button pressed */
OLDCODE							node->velocity = value;
OLDCODE						} else {
OLDCODE							#ifdef MIDIVERBOSE
OLDCODE							printf ("have ButtonRelease at %lf\n",TickTime());
OLDCODE							#endif
OLDCODE							node->pressTime = (double)0.0;
OLDCODE							/* DO NOT set velocity here - it will be 0 ALWAYS */
OLDCODE						}
OLDCODE						value = controller; /* second byte; see comment above */
OLDCODE
OLDCODE						/* now, reset the toggle... */
OLDCODE						node->buttonPress = noteOn;
OLDCODE						node->_butPr = node->buttonPress;
OLDCODE	
OLDCODE						determineMIDIValFromInt (node, &value, &fV); 
OLDCODE					
OLDCODE					}
OLDCODE				}
OLDCODE			}
OLDCODE		}
OLDCODE	}
OLDCODE
OLDCODE	/* do we send an event into the FreeWRL machine? */
OLDCODE	if (sendEvent) {
OLDCODE		node->intValue = value;
OLDCODE		node->_oldintValue = value;
OLDCODE		node->floatValue = fV;
OLDCODE		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, intValue));
OLDCODE		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, floatValue));
OLDCODE		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, buttonPress));
OLDCODE		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_MidiControl, velocity));
OLDCODE
OLDCODE		#ifdef MIDIVERBOSE
OLDCODE		printf ("intValue changed - now is %d sentvel %d\n",node->intValue, node->_sentVel); 
OLDCODE		printf ("sending %d %f ",value, node->floatValue);
OLDCODE		printf ("mins %d %d maxs %d %d ",node->deviceMinVal, node->minVal, node->deviceMaxVal, node->maxVal);
OLDCODE		printf ("float %f node->floatVal\n",node->floatValue);
OLDCODE		#endif
OLDCODE	}
OLDCODE}
OLDCODE
OLDCODE
OLDCODE/************ END OF MIDI CONTROL **************************/
#endif // OLDCODE

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
	if (!renderstate()->render_geom) return;

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
			if (fwl_isTextureLoaded(tnode->__textureTableIndex)) {
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
				if (fwl_isTextureLoaded(mnode->__texture0_)) nowFinished ++;
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

		node->loadTime = TickTime();
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, loadTime));
	}	

	/* have we NOW started loading? */
	if ((nowLoading > 0) && (node->__loading == 0)) {
		/* mark event isActive TRUE */
		node->isActive = 1;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isActive));

	
		node->__StartLoadTime = TickTime();
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
			if ((TickTime() - node->__StartLoadTime) > node->timeOut) {
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


#ifdef FRONTEND_GETS_FILES

/* do we want the GUI to load Inlines? Probabally if it loads the other files.... */

/* XXXX - because of the async nature of this, only the first file will be tried; if it fails, nothing
   else will be tried. We should store the resource locally, and, when scanning inline during rendering, check
   the status... */


static void loadInline(struct X3D_Inline *me)  
{
	resource_item_t *res = NULL;
	struct Multi_String *url = NULL;
	resource_item_t *parentPath = NULL;

	url = &(me->url);
	parentPath = me->_parentResource;

/*
	printf ("loadInline, we have status of ");
	switch (me->__loadstatus) {
		case INLINE_INITIAL_STATE: printf ("INLINE_INITIAL_STATE\n"); break;
		case INLINE_FETCHING_RESOURCE: printf ("INLINE_FETCHING_RESOURCE\n"); break;
		case INLINE_PARSING: printf ("INLINE_PARSING\n"); break;
		case INLINE_STABLE: printf ("INLINE_STABLE\n"); break;
	};
*/

	// get the resource pointer from the Inline node.
	res = me->__loadResource;

	switch (me->__loadstatus) {

		case INLINE_INITIAL_STATE: {
			// starting up here. Is the URL empty?
			if (me->url.n == 0) {
				me->__loadstatus = INLINE_STABLE; 
			} else {
				// create the resource;
				res = resource_create_multi(url);

				// save it...;
				me->__loadResource = res;

				// Setup parent
				resource_identify(parentPath, res);
				res->media_type = resm_image; /* quick hack */

				// go to next one
				me->__loadstatus = INLINE_FETCHING_RESOURCE;
			}
			break;
		}

		case INLINE_FETCHING_RESOURCE: {
			bool rv;

			/* printf ("load_Inline, before resource_fetch, we have type  %s  status %s\n",
				resourceTypeToString(res->type), resourceStatusToString(res->status)); */
			
			rv = resource_fetch(res);

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
				res->where = X3D_NODE(me);
				res->offsetFromWhere = (float) offsetof (struct X3D_Inline, __children);
				send_resource_to_parser_async(res);
				me->__loadstatus = INLINE_PARSING; /* a "do-nothing" approach */
			} else {
				if ((res->status == ress_failed) || (res->status == ress_invalid)) {
					printf ("resource failed to load\n");
					me->__loadstatus = INLINE_STABLE; /* a "do-nothing" approach */
				} else {
					printf ("resource Inline in invalid state\n");
					me->__loadstatus = INLINE_STABLE; /* a "do-nothing" approach */
				}
			}

			break;
		}

		case INLINE_PARSING: {
			/* printf ("load_Inline, after resource_fetch, we have type  %s  status %s\n",
				resourceTypeToString(res->type), resourceStatusToString(res->status)); */

			// did this one fail?
			if (res->status == ress_not_loaded) {
				printf ("hmmm - problem here...\n");
				res->status = ress_invalid;
                                res->type = rest_multi;

				// go and try the next URL on the url array
				resource_identify(parentPath, res);

				// go back and try another URL
				me->__loadstatus = INLINE_FETCHING_RESOURCE;

			}


			// are we good to go??
			else if (res->status == ress_parsed) {
				me->__loadstatus = INLINE_STABLE;
			}

			break;
		}
	}
}
#endif

/* note that we get the resources in a couple of steps; this tries to keep the scenegraph running */
void load_Inline (struct X3D_Inline *node) {
	// printf ("load_Inline %u, loadStatus %d loadResource %u\n",node, node->__loadstatus, node->__loadResource);

	if (node->load) {
		/* printf ("loading Inline\n");  */

#ifdef FRONTEND_GETS_FILES
	if (node->__loadstatus != INLINE_STABLE) {
		loadInline(node);
	}

#else
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

#endif /* FRONTEND_GETS_FILES */
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
