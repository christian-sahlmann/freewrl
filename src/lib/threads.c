/*
  $Id: threads.c,v 1.7 2009/10/26 10:57:07 couannette Exp $

  FreeWRL support library.
  Threads & process (fork).

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
#include <system_threads.h>
#include <internal.h>
#include <display.h>
#include <threads.h>

#include <errno.h>

/* Thread global variables */
pthread_t mainThread;

DEF_THREAD(DispThrd);

#ifdef DO_MULTI_OPENGL_THREADS
DEF_THREAD(shapeThread);
#endif

DEF_THREAD(PCthread);

DEF_THREAD(loadThread);

/* Thread synchronization global variables */

pthread_mutex_t mutex_resource_tree = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_resource_list = PTHREAD_MUTEX_INITIALIZER;


void initializeDisplayThread()
{
	int ret;

	/* Synchronize trace/error log... */
	fflush(stdout);
	fflush(stderr);
	sync();

	ASSERT(TEST_NULL_THREAD(DispThrd));
	ret = pthread_create(&DispThrd, NULL, (void *) _displayThread, NULL);
	switch (ret) {
	case 0: 
		break;
	case EAGAIN: 
		ERROR_MSG("initializeDisplayThread: not enough system resources to create a process for the new thread.");
		return;
	}

#if !defined(TARGET_AQUA) && !defined(TARGET_WIN32)
	if (global_trace_threads) {
		TRACE_MSG("initializeDisplayThread: waiting for display to become initialized...\n");
		while (IS_DISPLAY_INITIALIZED == FALSE) {
			usleep(50);
		}
	}
#endif
}

#ifdef DO_MULTI_OPENGL_THREADS
void initializeShapeCompileThread()
{
	int ret;

	/* Synchronize trace/error log... */
	fflush(stdout);
	fflush(stderr);

	ASSERT(TEST_NULL_THREAD(shapeThread));
	ret = pthread_create(&shapeThread, NULL, (void *(*)(void *))&_shapeCompileThread, NULL);
	switch (ret) {
	case 0: 
		break;
	case EAGAIN: 
		ERROR_MSG("initializeShapeCompileThread: not enough system resources to create a process for the new thread.");
		return;
	}
}
#endif

/* create consumer thread and set the "read only" flag indicating this */
void initializeInputParseThread()
{
	int ret;

	/* Synchronize trace/error log... */
	fflush(stdout);
	fflush(stderr);

	pthread_mutex_init( &mutex_resource_tree, NULL );
	pthread_mutex_init( &mutex_resource_list, NULL );

	ASSERT(TEST_NULL_THREAD(PCthread));
	ret = pthread_create(&PCthread, NULL, (void *(*)(void *))&_inputParseThread, NULL);
	switch (ret) {
	case 0: 
		break;
	case EAGAIN: 
		ERROR_MSG("initializeInputParseThread: not enough system resources to create a process for the new thread.");
		return;
	}
}

void initializeTextureThread()
{
	int ret;

	/* Synchronize trace/error log... */
	fflush(stdout);
	fflush(stderr);

	ASSERT(TEST_NULL_THREAD(loadThread));
	ret = pthread_create(&loadThread, NULL, (void *(*)(void *))&_textureThread, NULL);
	switch (ret) {
	case 0: 
		break;
	case EAGAIN: 
		ERROR_MSG("initializeTextureThread: not enough system resources to create a process for the new thread.");
		return;
	}
}

int fw_thread_id()
{
	pthread_t current_thread;

	current_thread = pthread_self();
	if (!current_thread) {
		ERROR_MSG("Critical: pthread_self returned 0\n");
		return 0;
	}

	if (current_thread == mainThread)
		return 1;

	if (current_thread == DispThrd)
		return 2;

	if (current_thread == PCthread)
		return 4;

	if (current_thread == loadThread)
		return 5;

#ifdef DO_MULTI_OPENGL_THREADS
	if (current_thread == shapeThread)
		return 4;
#endif

	return -1;
}

void fw_thread_dump()
{
	if (global_trace_threads) {
		/* Synchronize trace/error log... */
		fflush(stdout);
		fflush(stderr);
		TRACE_MSG("FreeWRL CURRENT THREAD: %d\n", fw_thread_id());
	}
}

void trace_enter_thread(const char *str)
{
	if (global_trace_threads) {
		/* Synchronize trace/error log... */
		fflush(stdout);
		fflush(stderr);
		sync();
		TRACE_MSG("*** ENTERING THREAD: %s, ID=%d self=%p\n", str, fw_thread_id(), (void*) pthread_self());
	}
}
