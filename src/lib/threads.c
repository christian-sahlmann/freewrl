/*
  $Id: threads.c,v 1.23 2011/05/25 19:26:34 davejoubert Exp $

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

#ifdef FREEWRL_THREAD_COLORIZED

/* Notes on thread ids and colors

   We have 5 threads max: main, display, parse, texture and shape.
   Each has its thread variable (pthread_t), it's number (1 through 5).

   Each thead has a "name" now (FREEWRL_THREAD_*).

   Now we associate a color for each thread in thread_colors[].
   The color is an ANSI color code for console output.

   In internal.h/c the code for console output is modified to use
   the thread color ([F]PRINTF gets the thread id with fw_thread_id()
   and gets the thread color from the array thread_colors through the 
   fw_thread_color() function).

*/

static int threads_colors[FREEWRL_MAX_THREADS] = {
	32, /* main thread is green */
	36, /* display thread is cyan */
	35, /* parser thread is purple */
	33, /* texture thread is brown */
	34, /* shape thread is blue */
	/* red color is reserved for important threading functions */
};
#define FREEWRL_DEFAULT_COLOR 37 /* white */

#endif /* FREEWRL_THREAD_COLORIZED */

/* Thread global variables */
pthread_t mainThread; /* main (default) thread */

DEF_THREAD(DispThrd); /* display thread */

DEF_THREAD(PCthread); /* parser thread */

DEF_THREAD(loadThread); /* texture thread */

/* Thread synchronization global variables */

/* Synchronize / exclusion root_res and below */
pthread_mutex_t mutex_resource_tree = PTHREAD_MUTEX_INITIALIZER;

/* Synchronize / exclusion : resource queue for parser */
pthread_mutex_t mutex_resource_list = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t resource_list_condition = PTHREAD_COND_INITIALIZER;

/* Synchronize / exclusion (main<=>texture) */
pthread_mutex_t mutex_texture_list = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t texture_list_condition = PTHREAD_COND_INITIALIZER;


#ifdef _MSC_VER
void sync(){}
#endif

void fwl_initializeDisplayThread()
{
	int ret;

	/* Synchronize trace/error log... */
	fflush(stdout);
	fflush(stderr);
	sync();
	ASSERT(TEST_NULL_THREAD(DispThrd));


	/* Initialize all mutex/condition variables ... */
	pthread_mutex_init( &mutex_resource_tree, NULL );
	pthread_mutex_init( &mutex_resource_list, NULL );
	pthread_mutex_init( &mutex_texture_list, NULL );
	pthread_cond_init( &resource_list_condition, NULL );
	pthread_cond_init( &texture_list_condition, NULL );


/* IPHONE handles the display itself */
#if !defined (FRONTEND_HANDLES_DISPLAY_THREAD)
	ret = pthread_create(&DispThrd, NULL, (void *) _displayThread, NULL);
	switch (ret) {
	case 0: 
		break;
	case EAGAIN: 
		ERROR_MSG("initializeDisplayThread: not enough system resources to create a process for the new thread.");
		return;
	}
#endif /* FRONTEND_HANDLES_DISPLAY_THREAD */


#if !defined(TARGET_AQUA) && !defined(_MSC_VER) //TARGET_WIN32)
	if (global_trace_threads) {
		TRACE_MSG("initializeDisplayThread: waiting for display to become initialized...\n");
		while (IS_DISPLAY_INITIALIZED == FALSE) {
			usleep(50);
		}
	}
#endif
}

/* create consumer thread and set the "read only" flag indicating this */
void fwl_initializeInputParseThread()
{
	int ret;

	/* Synchronize trace/error log... */
	fflush(stdout);
	fflush(stderr);

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

void fwl_initializeTextureThread()
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

#ifdef _MSC_VER 
	if (!current_thread.p) {
#else
	if (!current_thread) {
#endif
		ERROR_MSG("Critical: pthread_self returned 0\n");
		return 0;
	}

	if (pthread_equal(current_thread, mainThread))
		return FREEWRL_THREAD_MAIN;

	if (pthread_equal(current_thread, DispThrd))
		return FREEWRL_THREAD_DISPLAY;

	if (pthread_equal(current_thread, PCthread))
		return FREEWRL_THREAD_PARSER;

	if (pthread_equal(current_thread, loadThread))
		return FREEWRL_THREAD_TEXTURE;

/*#endif*/
	return -1;
}

#ifdef FREEWRL_THREAD_COLORIZED

int fw_thread_color(int thread_id)
{
	/* id will range from 1 to 5 */
	if ((thread_id > 0) && (thread_id <= FREEWRL_MAX_THREADS)) {
		return threads_colors[ thread_id - 1 ];
	}
	return FREEWRL_DEFAULT_COLOR;
}

#endif /* FREEWRL_THREAD_COLORIZED */

void fwl_thread_dump()
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
#ifdef _MSC_VER
		TRACE_MSG("*** ENTERING THREAD: %s, ID=%d self=%p\n", str, fw_thread_id(), (void*) pthread_self().p);
#else
		TRACE_MSG("*** ENTERING THREAD: %s, ID=%d self=%p\n", str, fw_thread_id(), (void*) pthread_self());
#endif
	}
}
