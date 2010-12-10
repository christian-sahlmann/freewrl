/*
=INSERT_TEMPLATE_HERE=

$Id: ringbuf.c,v 1.1 2010/12/10 17:17:20 davejoubert Exp $

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
/* #include <stdlib.h> */
#include "../scenegraph/ringbuf.h"

RingBuffer * NewRingBuffer (int elCount) {

	RingBuffer * buffer ;

	buffer = malloc(sizeof(RingBuffer)) ;

	rbItem * data = malloc(sizeof(rbItem)*(FORCE_GUARD_ELEMENT+elCount));

	buffer -> head = 0 ;
	buffer -> tail = 0 ;
	buffer -> noOfElements = FORCE_GUARD_ELEMENT+elCount ;
	buffer -> data = data ;

	#ifdef TRACK_RINGBUFFER_MSG
	printf("NewRingBuffer at %p , data at %p , %d elements\n",buffer, buffer->data , buffer->noOfElements);
	#endif

	return buffer ;
}

int RingBuffer_qLen(RingBuffer * buffer) {

	if(buffer->data == NULL) return 0;

	if (buffer->tail >= buffer->head) {
		return buffer->tail - buffer->head;
	} else {
		return (buffer->tail + buffer->noOfElements) - buffer->head;
	}
}

int RingBuffer_freeLen(RingBuffer * buffer) {

	if(buffer->data == NULL) return 0;
	int used = RingBuffer_qLen(buffer) ;
	return (buffer->noOfElements-FORCE_GUARD_ELEMENT) - used ;
}

int RingBuffer_testEmpty(RingBuffer * buffer) {
	if(buffer->data == NULL) return 1;
	return (buffer->tail == buffer->head)? 1:0;
}

int RingBuffer_testFull(RingBuffer * buffer) {
	if(buffer->data == NULL) return 1;
	int qlen = RingBuffer_qLen(buffer) ;
	return (qlen < (buffer->noOfElements-FORCE_GUARD_ELEMENT))? 0:1;
}

int RingBuffer_pushInt(RingBuffer * buffer, int newInt) {

	if(buffer->data == NULL) return -1;

	if(!RingBuffer_testFull(buffer)) {
		rbItem * data = buffer->data ;
		(data+buffer->tail)->i = newInt;
		buffer->tail++;
		buffer->tail = buffer->tail % buffer->noOfElements;
		return 0;
	}
	return -1;
}

int RingBuffer_pushFloat(RingBuffer * buffer, float newFloat) {

	if(buffer->data == NULL) return -1;

	if(!RingBuffer_testFull(buffer)) {
		rbItem * data = buffer->data ;
		(data+buffer->tail)->f = newFloat;
		buffer->tail++;
		buffer->tail = buffer->tail % buffer->noOfElements;
		return 0;
	}
	return -1;
}

int RingBuffer_pushPointer(RingBuffer * buffer, void *newPointer) {

	if(buffer->data == NULL) return -1;

	if(!RingBuffer_testFull(buffer)) {
		rbItem * data = buffer->data ;
		(data+buffer->tail)->p = newPointer;
		buffer->tail++;
		buffer->tail = buffer->tail % buffer->noOfElements;
		return 0;
	}
	return -1;
}

rbItem * RingBuffer_pullUnion(RingBuffer * buffer) {

	if(buffer->data == NULL) return NULL;

	if(!RingBuffer_testEmpty(buffer)) {
		rbItem * xyz ;
		rbItem * data = buffer->data ;
		xyz=(data+buffer->head);
		buffer->head++;
		buffer->head = buffer->head % buffer->noOfElements;

		return xyz;
	} else {
		return NULL ;
	}
}

rbItem * RingBuffer_peekUnion(RingBuffer * buffer) {

	if(buffer->data == NULL) return NULL;

	if(!RingBuffer_testEmpty(buffer)) {
		rbItem * xyz ;
		rbItem * data = buffer->data ;
		xyz=(data+buffer->head);
		return xyz;
	} else {
		return NULL ;
	}
}

void RingBuffer_makeEmpty(RingBuffer * buffer) {
	buffer->tail = 0 ;
	buffer->head = 0 ;
}

void RingBuffer_freeDataArea(RingBuffer * buffer) {
	if(buffer->data == NULL) return ;
	free(buffer->data) ;
	buffer->data = NULL;
}

