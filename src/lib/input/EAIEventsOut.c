/*
=INSERT_TEMPLATE_HERE=

$Id: EAIEventsOut.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

Small routines to help with interfacing EAI to Daniel Kraft's parser.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"

#include "../input/EAIheaders.h"


/*****************************************************************
*
*	handle_Listener is called when a requested value changes.
*
*	What happens is that the "normal" freewrl routing code finds
*	an EventOut changed, copies the data to the buffer EAIListenerData,
*	and copies an extra bit to the global CRoutesExtra.
*
*	(see the CRoutes_Register call above for this routing setup)
*
*	This routine decodes the data type and acts on it. The data type
*	has (currently) an id number, that the client uses, and the data
*	type.
*
********************************************************************/

void handle_Listener () {
	int id, tp;
	char buf[EAIREADSIZE];
	struct Multi_Node *mfptr;	/* used for freeing memory*/

	/* get the type and the id.*/
	tp = CRoutesExtra&0xff;
	id = (CRoutesExtra & 0xffffff00) >>8;

	if (eaiverbose) {
		printf ("Handle listener, id %x type %x extradata %x\n",id,tp,CRoutesExtra);
	}	

	/* convert the data to string form, for sending to the EAI java client */
	EAI_Convert_mem_to_ASCII (id,"EV", tp, EAIListenerData, buf);

	/* if this is a MF type, there most likely will be MALLOC'd memory to free... */
	switch (tp) {
		case FIELDTYPE_MFColor:
		case FIELDTYPE_MFColorRGBA:
		case FIELDTYPE_MFFloat:
		case FIELDTYPE_MFTime:
		case FIELDTYPE_MFInt32:
		case FIELDTYPE_MFString:
		case FIELDTYPE_MFNode:
		case FIELDTYPE_MFRotation:
		case FIELDTYPE_MFVec2f:
		case FIELDTYPE_MFVec3f: {
			mfptr = (struct Multi_Node *) EAIListenerData;
			FREE_IF_NZ ((*mfptr).p);
		}
		default: {}
	}




	/* zero the memory for the next time - MultiMemcpy needs this to be zero, otherwise
	   it might think that the "oldlen" will be non-zero */
	bzero(&EAIListenerData, sizeof(EAIListenerData));

	/* append the EV_EOT marker to the end of the string */
	strcat (buf,"\nEV_EOT");

	if (eaiverbose) {
		printf ("Handle Listener, returning %s\n",buf);
	}	

	/* send the EV reply */
	EAI_send_string(buf,EAIlistenfd);
}


