/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Sound.c,v 1.16 2011/06/02 19:50:43 dug9 Exp $

X3D Sound Component

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

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"
#include "../opengl/OpenGL_Utils.h"

#include "LinearAlgebra.h"
#include "sounds.h"

void render_AudioControl (struct X3D_AudioControl *node) {
	GLDOUBLE mod[16];
	GLDOUBLE proj[16];
	struct point_XYZ vec, direction, location;
	double len;
	double angle;
	float midmin, midmax;

	/*  do the sound registering first, and tell us if this is an audioclip*/
	/*  or movietexture.*/


	/* if not enabled, do nothing */
	if (!node) return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_AudioControl, enabled));
	}
	if (!node->enabled) return;

	direction.x = node->direction.c[0];
	direction.y = node->direction.c[1];
	direction.z = node->direction.c[2];

	location.x = node->location.c[0];
	location.y = node->location.c[1];
	location.z = node->location.c[2];

	midmin = (node->minFront - node->minBack) / (float) 2.0;
	midmax = (node->maxFront - node->maxBack) / (float) 2.0;


	FW_GL_PUSH_MATRIX();

	/*
	first, find whether or not we are within the maximum circle.

	translate to the location, and move the centre point, depending
	on whether we have a direction and differential maxFront and MaxBack
	directions.
	*/

	FW_GL_TRANSLATE_D (location.x + midmax*direction.x,
			location.y + midmax*direction.y,
			location.z + midmax * direction.z);

	/* make the ellipse a circle by scaling...
	FW_GL_SCALE_F (direction.x*2.0 + 0.5, direction.y*2.0 + 0.5, direction.z*2.0 + 0.5);
	- scaling needs work - we need direction information, and parameter work. */

	if ((fabs(node->minFront - node->minBack) > 0.5) ||
		(fabs(node->maxFront - node->maxBack) > 0.5)) {
		if (!soundWarned) {
			printf ("FreeWRL:Sound: Warning - minBack and maxBack ignored in this version\n");
			soundWarned = TRUE;
		}
	}



	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, mod);
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, proj);
	FW_GLU_UNPROJECT(viewport[2]/2,viewport[3]/2,0.0,
		mod,proj,viewport, &vec.x,&vec.y,&vec.z);
	/* printf ("mod %lf %lf %lf proj %lf %lf %lf\n",*/
	/* mod[12],mod[13],mod[14],proj[12],proj[13],proj[14]);*/

	len = sqrt(VECSQ(vec));
	/* printf ("len %f\n",len);  */
	/* printf("Sound: len %f mB %f mF %f angles (%f %f %f)\n",len,*/
	/* 	-node->maxBack, node->maxFront,vec.x,vec.y,vec.z);*/


	/*  pan left/right. full left = 0; full right = 1.*/
	if (len < 0.001) angle = 0;
	else {
		if (APPROX (mod[12],0)) {
			/* printf ("mod12 approaches zero\n");*/
			mod[12] = 0.001;
		}
		angle = fabs(atan2(mod[14],mod[12])) - (PI/2.0);
		angle = angle/(PI/2.0);

		/*  Now, scale this angle to make it between -0.5*/
		/*  and +0.5; if we divide it by 2.0, we will get*/
		/*  this range, but if we divide it by less, then*/
		/*  the sound goes "hard over" to left or right for*/
		/*  a bit.*/
		angle = angle / 1.5;

		/*  now scale to 0 to 1*/
		angle = angle + 0.5;

		/* and, "reverse" the value, so that left is left, and right is right */
		angle = 1.0 - angle;

		/*  bounds check...*/
		if (angle > 1.0) angle = 1.0;
		if (angle < 0.0) angle = 0.0;

		#ifdef SOUNDVERBOSE
		printf ("angle: %f\n",angle); 
		#endif
	}

	/* convert to a MIDI control value */
	node->panFloatVal = (float) angle;
	node->panInt32Val = (int) (angle * 128);
	if (node->panInt32Val < 0) node->panInt32Val = 0; if (node->panInt32Val > 127) node->panInt32Val = 127;


	node->volumeFloatVal = (float) 0.0;
	/* is this within the maxFront maxBack? */

	/* this code needs rework JAS */
	if (len < node->maxFront) {
		/* did this node become active? */
		if (!node->isActive) {
			node->isActive = TRUE;
			MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_AudioControl, isActive));
			#ifdef SOUNDVERBOSE
			printf ("AudioControl node is now ACTIVE\n");
			#endif


			/* record the length for doppler shift comparisons */
			node->__oldLen = len;
		}

		/* note: using vecs, length is always positive - need to work in direction
		vector */
		if (len < 0.0) {
			if (len < node->minBack) {node->volumeFloatVal = (float) 1.0;}
			else { node->volumeFloatVal = ((float) len - node->maxBack) / (node->maxBack - node->minBack); }
		} else {
			if (len < node->minFront) {node->volumeFloatVal = (float) 1.0;}
			else { node->volumeFloatVal = (node->maxFront - (float) len) / (node->maxFront - node->minFront); }
		}

		/* work out the delta for len */
		if (APPROX(node->maxDelta, 0.0)) {
			printf ("AudioControl: maxDelta approaches zero!\n");
			node->deltaFloatVal = (float) 0.0;
		} else {
			#ifdef SOUNDVERBOSE
			printf ("maxM/S %f \n",(node->__oldLen - len)/ (TickTime()- lastTime));
			#endif

			/* calculate change as Metres/second */

			/* compute node->deltaFloatVal, and clamp to range of -1.0 to 1.0 */
			node->deltaFloatVal = (float) ((node->__oldLen - len)/(TickTime()-lastTime()))/node->maxDelta;
			if (node->deltaFloatVal < (float) -1.0) node->deltaFloatVal = (float) -1.0; if (node->deltaFloatVal > (float) 1.0) node->deltaFloatVal = (float) 1.0;
			node->__oldLen = len;
		}

		/* Now, fit in the intensity. Send along command, with
		source number, amplitude, balance, and the current Framerate */
		node->volumeFloatVal = node->volumeFloatVal*node->intensity;
		node->volumeInt32Val = (int) (node->volumeFloatVal * 128.0); 
		if (node->volumeInt32Val < 0) node->volumeInt32Val = 0; if (node->volumeInt32Val > 127) node->volumeInt32Val = 127;

		node->deltaInt32Val = (int) (node->deltaFloatVal * 64.0) + 64; 
		if (node->deltaInt32Val < 0) node->deltaInt32Val = 0; if (node->deltaInt32Val > 127) node->deltaInt32Val = 127;

		#ifdef SOUNDVERBOSE
		printf ("AudioControl: amp: %f (%d)  angle: %f (%d)  delta: %f (%d)\n",node->volumeFloatVal,node->volumeInt32Val,
			node->panFloatVal, node->panInt32Val ,node->deltaFloatVal,node->deltaInt32Val);
		#endif

		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_AudioControl, volumeInt32Val));
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_AudioControl, volumeFloatVal));
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_AudioControl, panInt32Val));
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_AudioControl, panFloatVal));
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_AudioControl, deltaInt32Val));
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_AudioControl, deltaFloatVal));

	} else {
		/* node just became inActive */
		if (node->isActive) {
			node->isActive = FALSE;
			MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_AudioControl, isActive));
			#ifdef SOUNDVERBOSE
			printf ("AudioControl node is now INACTIVE\n");
			#endif
		}
	}

	FW_GL_POP_MATRIX();
}

void render_Sound (struct X3D_Sound *node) {
#ifdef MUST_RE_IMPLEMENT_SOUND_WITH_OPENAL
	GLDOUBLE mod[16];
	GLDOUBLE proj[16];
	struct point_XYZ vec, direction, location;
	double len;
	double angle;
	float midmin, midmax;
	float amp;

	struct X3D_AudioClip *acp = NULL;
	struct X3D_MovieTexture *mcp = NULL;
	struct X3D_Node *tmpN = NULL;
	char mystring[256];

	/* why bother doing this if there is no source? */
	if (node->source == NULL) return;

	/* ok, is the source a valid node?? */

	/* might be a PROTO expansion, as in what Adam Nash does... */
	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->source,tmpN)

	/* did not find a valid source node, even after really looking at a PROTO def */
	if (tmpN == NULL) return;

	if (tmpN->_nodeType == NODE_AudioClip) 
		acp = (struct X3D_AudioClip *) tmpN;
	else if (tmpN->_nodeType == NODE_MovieTexture)
		mcp = (struct X3D_MovieTexture *) tmpN;

	else {
		ConsoleMessage ("Sound node- source type of %s invalid",stringNodeType(tmpN->_nodeType));
		node->source = NULL; /* stop messages from scrolling forever */
		return;
	}

	/* printf ("sound, node %d, acp %d source %d\n",node, acp, acp->__sourceNumber); */
	/*  MovieTextures NOT handled yet*/
	/*  first - is there a node (any node!) attached here?*/
	if (acp) {
		/*  do the sound registering first, and tell us if this is an audioclip*/
		/*  or movietexture.*/

		render_node(X3D_NODE(acp));

		/*  if the attached node is not active, just return*/
		/* printf ("in Sound, checking AudioClip isactive %d\n", acp->isActive); */
		if (acp->isActive == 0) return;

		direction.x = node->direction.c[0];
		direction.y = node->direction.c[1];
		direction.z = node->direction.c[2];

		location.x = node->location.c[0];
		location.y = node->location.c[1];
		location.z = node->location.c[2];

		midmin = (node->minFront - node->minBack) / 2.0;
		midmax = (node->maxFront - node->maxBack) / 2.0;


		FW_GL_PUSH_MATRIX();

		/*
		first, find whether or not we are within the maximum circle.

		translate to the location, and move the centre point, depending
		on whether we have a direction and differential maxFront and MaxBack
		directions.
		*/

		FW_GL_TRANSLATE_F (location.x + midmax*direction.x,
				location.y + midmax*direction.y,
				location.z + midmax * direction.z);

		/* make the ellipse a circle by scaling...
		FW_GL_SCALE_F (direction.x*2.0 + 0.5, direction.y*2.0 + 0.5, direction.z*2.0 + 0.5);
		- scaling needs work - we need direction information, and parameter work. */

		if ((fabs(node->minFront - node->minBack) > 0.5) ||
			(fabs(node->maxFront - node->maxBack) > 0.5)) {
			if (!soundWarned) {
				printf ("FreeWRL:Sound: Warning - minBack and maxBack ignored in this version\n");
				soundWarned = TRUE;
			}
		}



		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, mod);
		FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, proj);
		FW_GLU_UNPROJECT(viewport[2]/2,viewport[3]/2,0.0,
			mod,proj,viewport, &vec.x,&vec.y,&vec.z);
		/* printf ("mod %lf %lf %lf proj %lf %lf %lf\n",*/
		/* mod[12],mod[13],mod[14],proj[12],proj[13],proj[14]);*/

		len = sqrt(VECSQ(vec));
		/* printf ("len %f\n",len); */
		/* printf("Sound: len %f mB %f mF %f angles (%f %f %f)\n",len,*/
		/* 	-node->maxBack, node->maxFront,vec.x,vec.y,vec.z);*/


		/*  pan left/right. full left = 0; full right = 1.*/
		if (len < 0.001) angle = 0;
		else {
			if (APPROX (mod[12],0)) {
				/* printf ("mod12 approaches zero\n");*/
				mod[12] = 0.001;
			}
			angle = fabs(atan2(mod[14],mod[12])) - (PI/2.0);
			angle = angle/(PI/2.0);

			/*  Now, scale this angle to make it between -0.5*/
			/*  and +0.5; if we divide it by 2.0, we will get*/
			/*  this range, but if we divide it by less, then*/
			/*  the sound goes "hard over" to left or right for*/
			/*  a bit.*/
			angle = angle / 1.5;

			/*  now scale to 0 to 1*/
			angle = angle + 0.5;

			/*  bounds check...*/
			if (angle > 1.0) angle = 1.0;
			if (angle < 0.0) angle = 0.0;
			/* printf ("angle: %f\n",angle); */
		}


		amp = 0.0;
		/* is this within the maxFront maxBack? */

		/* printf ("sound %d len %f maxFront %f\n",acp->__sourceNumber, len, node->maxFront); */
		/* this code needs rework JAS */
		if (len < node->maxFront) {

			/* note: using vecs, length is always positive - need to work in direction
			vector */
			if (len < 0.0) {
				if (len < node->minBack) {amp = 1.0;}
				else {
					amp = (len - node->maxBack) / (node->maxBack - node->minBack);
				}
			} else {
				if (len < node->minFront) {amp = 1.0;}
				else {
					amp = (node->maxFront - len) / (node->maxFront - node->minFront);
				}
			}

			/* Now, fit in the intensity. Send along command, with
			source number, amplitude, balance, and the current Framerate */
			amp = amp*node->intensity;
			if (sound_from_audioclip) {
				sprintf (mystring,"AMPL %d %f %f",acp->__sourceNumber,amp,angle);
			} else {
				sprintf (mystring,"MMPL %d %f %f",mcp->__sourceNumber,amp,angle);
			}
			Sound_toserver(mystring);
		}
		FW_GL_POP_MATRIX();
	}
#endif /* MUST_RE_IMPLEMENT_SOUND_WITH_OPENAL */
}

void render_AudioClip (struct X3D_AudioClip *node) {
#ifdef MUST_RE_IMPLEMENT_SOUND_WITH_OPENAL
	/*  register an audioclip*/
	float pitch,stime, sttime;
	int loop;
	unsigned char *filename = (unsigned char *)node->__localFileName;

	/* tell Sound that this is an audioclip */
	sound_from_audioclip = TRUE;

	/* printf ("_change %d _ichange %d\n",node->_change, node->_ichange);  */

	if (!SoundEngineStarted) {
		printf ("AudioClip: initializing SoundEngine\n");
		SoundEngineStarted = TRUE;
		SoundEngineInit();
	}
#ifndef JOHNSOUND
	if (node->isActive == 0) return;  /*  not active, so just bow out*/
#endif

	if (!SoundSourceRegistered(node->__sourceNumber)) {

		/*  printf ("AudioClip: registering clip %d loop %d p %f s %f st %f url %s\n",
			node->__sourceNumber,  node->loop, node->pitch,node->startTime, node->stopTime,
			filename); */

		pitch = node->pitch;
		stime = node->startTime;
		sttime = node->stopTime;
		loop = node->loop;

		AC_LastDuration[node->__sourceNumber] =
			SoundSourceInit (node->__sourceNumber, node->loop,
			(double) pitch,(double) stime, (double) sttime, filename);
		/* printf ("globalDuration source %d %f\n",
				node->__sourceNumber,AC_LastDuration[node->__sourceNumber]);  */
	}
#endif /* MUST_RE_IMPLEMENT_SOUND_WITH_OPENAL */
}
