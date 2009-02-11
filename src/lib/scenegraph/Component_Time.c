/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Time.c,v 1.3 2009/02/11 15:12:55 istakenv Exp $

X3D Time Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"


/* void do_TimeSensorTick (struct X3D_TimeSensor *node) {*/
void do_TimeSensorTick ( void *ptr) {
	struct X3D_TimeSensor *node = (struct X3D_TimeSensor *)ptr;
	double myDuration;
	int oldstatus;
	double myTime;
	double frac;

	/* are we not enabled */
	if (!node) return;

	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_TimeSensor, enabled));
	}
	if (!node->enabled) {
		if (node->isActive) {
			node->isActive=0;
			MARK_EVENT (ptr, offsetof(struct X3D_TimeSensor, isActive));
		}
		return;
	}

	/* can we possibly have started yet? */
	if(TickTime < node->startTime) {
		return;
	}

	oldstatus = node->isActive;
	myDuration = node->cycleInterval;

	/* call common time sensor routine */
	/*
		printf ("cycleInterval %f \n",node->cycleInterval);

		uncomment the following to ensure that the gcc bug
		in calling doubles/floats in here is not causing us
		problems again...


		printf ("calling ");
		printf ("act %d ",node->isActive);
		printf ("initt %lf ",node->__inittime);
		printf ("startt %lf ",node->startTime);
		printf ("stopt %lf ",node->stopTime);
		printf ("loop %d ",node->loop);
		printf ("myDuration %f ",(float) myDuration);
		printf ("speed %f\n",(float) 1.0);
	*/

	do_active_inactive (
		&node->isActive, &node->__inittime, &node->startTime,
		&node->stopTime,node->loop,myDuration, 1.0);

	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_TimeSensor, metadata))

	/* now process if we have changed states */
	if (oldstatus != node->isActive) {
		if (node->isActive == 1) {
			/* force code below to generate event */
			node->__ctflag = 10.0;
		}

		/* push @e, [$t, "isActive", node->{isActive}]; */
		MARK_EVENT (ptr, offsetof(struct X3D_TimeSensor, isActive));
	}


	if(node->isActive == 1) {
		/* set time field */
		node->time = TickTime;
		MARK_EVENT (ptr, offsetof(struct X3D_TimeSensor, time));

		/* calculate what fraction we should be */
 		myTime = (TickTime - node->startTime) / myDuration;

		if (node->loop) {
			frac = myTime - (int) myTime;
		} else {
			frac = (myTime > 1 ? 1 : myTime);
		}

		#ifdef SEVERBOSE
		printf ("TimeSensor myTime %f frac %f dur %f\n", myTime,frac,myDuration);
		#endif

		/* cycleTime events once at start, and once every loop. */
		if (frac < node->__ctflag) {
			/* push @e, [$t, cycleTime, $TickTime]; */
			node->cycleTime = TickTime;
			MARK_EVENT (ptr, offsetof(struct X3D_TimeSensor, cycleTime));
		}
		node->__ctflag = frac;

		node->fraction_changed = frac;
		MARK_EVENT (ptr, offsetof(struct X3D_TimeSensor, fraction_changed));

	}
}

