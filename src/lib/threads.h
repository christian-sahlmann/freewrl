/*
  $Id: threads.h,v 1.1 2009/10/26 13:30:36 couannette Exp $

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


#ifndef __LIBFREEWRL_THREADS_H__
#define __LIBFREEWRL_THREADS_H__


int freewrlSystem(const char *string);

/**
 *   Gather here all thread function prototypes & thread variables
 *
 *   We have 4[5] threads in FreeWRL:
 *   1: main thread
 *   2: display thread
 *   [3: shape compiler ONLY on Mac and if OpenGL multi-threaded]
 *   4: input parser
 *   5: texture loader
 */

extern pthread_t mainThread;

extern pthread_t DispThrd;

#ifdef DO_MULTI_OPENGL_THREADS
extern pthread_t shapeThread;
#endif

extern pthread_t PCthread;

extern pthread_t loadThread;

/**
 *   Gather here all functions that create threads 
 */
/* DISPLAY THREAD */
void initializeDisplayThread();
void _displayThread();

#ifdef DO_MULTI_OPENGL_THREADS
/* SHAPE COMPILER THREAD */
void initializeShapeCompileThread();
void _shapeCompileThread();
#endif

/* PARSER THREAD */
void initializeInputParseThread();
void _inputParseThread ();

int isinputThreadParsing();
int isInputThreadInitialized();

/* TEXTURE THREAD */
void initializeTextureThread();
void _textureThread();
int isTextureinitialized();

int fw_thread_id();
void fw_thread_dump();

#define ENTER_THREAD(_str) trace_enter_thread(_str)
void trace_enter_thread(const char *str);

extern pthread_mutex_t mutex_resource_tree;
extern pthread_mutex_t mutex_resource_list;



#endif /* __LIBFREEWRL_THREADS_H__ */
