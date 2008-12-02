/*
=INSERT_TEMPLATE_HERE=

$Id: Component_EventUtils.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

X3D Event Utilities Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"


/******************************************************************************/
/* see the spec for a description. fields inputTrue and inputFalse are set in
   VRMLNodes.pm and are never changed, ONLY MARK_EVENT is called when
   appropriate. So, inputFalse will ALWAYS be false, BUT, the event will
   be called when set_boolean is set to false.  */

void do_BooleanFilter (void *node){
	struct X3D_BooleanFilter *px;

	if (!node) return;
	px = (struct X3D_BooleanFilter *) node;

	if (px->set_boolean == TRUE) {
		px->inputNegate = FALSE;
		MARK_EVENT (node, offsetof (struct X3D_BooleanFilter, inputTrue));
		MARK_EVENT (node, offsetof (struct X3D_BooleanFilter, inputNegate));
	} else {
		px->inputNegate = TRUE;
		MARK_EVENT (node, offsetof (struct X3D_BooleanFilter, inputFalse));
		MARK_EVENT (node, offsetof (struct X3D_BooleanFilter, inputNegate));
	}
}

/******************************************************************************/
/* see the spec for a description */

/* WHAT ARE NEXT AND PREVIOUS FIELDS FOR???? NOT MENTIONED IN SPEC (AT LEAST
REVISION FOUND WHEN IMPLEMENTING */

void do_BooleanSequencer (void *node){
	struct X3D_BooleanSequencer *px;
	int kin, kvin;
	int *kVs;
	int counter;

	if (!node) return;
	px = (struct X3D_BooleanSequencer *) node;
	kin = px->key.n;
	kvin = px->keyValue.n;
	kVs = px->keyValue.p;

	MARK_EVENT (node, offsetof (struct X3D_BooleanSequencer, value_changed));

	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		px->value_changed = 0.0;
		return;
	}
	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */

	#ifdef SEVERBOSE
		printf ("BooleanSequencer, kin %d kvin %d, vc %f\n",kin,kvin,px->value_changed);
	#endif

	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= px->key.p[0]) {
		 px->value_changed = kVs[0];
	} else if (px->set_fraction >= px->key.p[kin-1]) {
		 px->value_changed = kVs[kvin-1];
	} else {
		/* have to go through and find the key before */
		counter=find_key(kin,(float)(px->set_fraction),px->key.p);
		px->value_changed =
			(px->set_fraction - px->key.p[counter-1]) /
			(px->key.p[counter] - px->key.p[counter-1]) *
			(kVs[counter] - kVs[counter-1]) +
			kVs[counter-1];
	}
}
	
/******************************************************************************/
/* see the spec for a description */
void do_BooleanToggle (void *node){ 
	struct X3D_BooleanToggle *px;

	if (!node) return;
	px = (struct X3D_BooleanToggle *) node;

	if (px->set_boolean == TRUE) px->toggle = FALSE; 
	else px->toggle = TRUE; 

	MARK_EVENT (node, offsetof (struct X3D_BooleanToggle, toggle));
}

/******************************************************************************/
/* see the spec for a description */
void do_BooleanTrigger (void *node){
	struct X3D_BooleanTrigger *px;

	if (!node) return;
	px = (struct X3D_BooleanTrigger *) node;

	px->triggerTrue = TRUE; /* spec says that this is ALWAYS true */
	MARK_EVENT (node, offsetof (struct X3D_BooleanTrigger, triggerTrue));
}

/******************************************************************************/
/* see the spec for a description */

/* WHAT ARE NEXT AND PREVIOUS FIELDS FOR???? NOT MENTIONED IN SPEC (AT LEAST
REVISION FOUND WHEN IMPLEMENTING */

void do_IntegerSequencer (void *node){
	struct X3D_IntegerSequencer *px;
	int kin, kvin;
	int *kVs;
	int counter;

	if (!node) return;
	px = (struct X3D_IntegerSequencer *) node;
	kin = px->key.n;
	kvin = px->keyValue.n;
	kVs = px->keyValue.p;

	MARK_EVENT (node, offsetof (struct X3D_IntegerSequencer, value_changed));

	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		px->value_changed = 0.0;
		return;
	}
	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */

	#ifdef SEVERBOSE
		printf ("IntegerSequencer, kin %d kvin %d, vc %f\n",kin,kvin,px->value_changed);
	#endif

	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= px->key.p[0]) {
		 px->value_changed = kVs[0];
	} else if (px->set_fraction >= px->key.p[kin-1]) {
		 px->value_changed = kVs[kvin-1];
	} else {
		/* have to go through and find the key before */
		counter=find_key(kin,(float)(px->set_fraction),px->key.p);
		px->value_changed =
			(px->set_fraction - px->key.p[counter-1]) /
			(px->key.p[counter] - px->key.p[counter-1]) *
			(kVs[counter] - kVs[counter-1]) +
			kVs[counter-1];
	}
}

/******************************************************************************/
/* see the spec for a description */
void do_IntegerTrigger (void *node){
	struct X3D_IntegerTrigger *px;
	
	if (!node) return;

	px = (struct X3D_IntegerTrigger *) node;
	px->triggerValue = px->integerKey;
	MARK_EVENT (node, offsetof (struct X3D_IntegerTrigger,triggerValue));
}

/******************************************************************************/
/* see the spec for a description */
void do_TimeTrigger (void *node){
	struct X3D_TimeTrigger *px;

	if (!node) return;
	px = (struct X3D_TimeTrigger *) node;

	px->triggerTime = TickTime;
	MARK_EVENT (node, offsetof (struct X3D_TimeTrigger,triggerTime));
}
