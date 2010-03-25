/*
=INSERT_TEMPLATE_HERE=

$Id: statics.c,v 1.10 2010/03/25 18:15:10 crc_canada Exp $

large constant strings; used for rendering.

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
#include "../main/headers.h"


/* BOX */

/* faces are triangles, vertices; order: front, back, top, down, right, left. */
#define FT 0, 0, 1,
#define BK 0, 0, -1,
#define TP 0, 1, 0,
#define DN 0, -1, 0,
#define RT 1, 0, 0,
#define LT -1, 0, 0,

GLfloat boxnorms[] = {   
	FT FT FT    FT FT FT
	BK BK BK    BK BK BK
	TP TP TP    TP TP TP
	DN DN DN    DN DN DN
	RT RT RT    RT RT RT
	LT LT LT    LT LT LT
	0};

#undef FT
#undef BK
#undef TP
#undef DN
#undef RT
#undef LT

/* box texture coordinates */
#define F0 1.0f, 1.0f,
#define F1 0.0f, 1.0f,
#define F2 0.0f, 0.0f,
#define F3 1.0f, 0.0f,

/* vertices:

F1-------------F0
|              |
|              |
F2-------------F3

the Rs are just behind the F's. So, we map the u,v to this...  see the compile_Box routine for vertex mapping 
	
        PTF0 PTF1 PTF2  PTF0 PTF2 PTF3 front 
        PTR2 PTR1 PTR0  PTR3 PTR2 PTR0 back 
        PTF0 PTR0 PTR1  PTF0 PTR1 PTF1 top   
        PTF3 PTF2 PTR2  PTF3 PTR2 PTR3 bottom
        PTF0 PTF3 PTR3  PTF0 PTR3 PTR0 right
        PTF1 PTR1 PTR2  PTF1 PTR2 PTF2 left
*/

GLfloat boxtex[] = {
        F0 F1 F2  F0 F2 F3
	F3 F0 F1  F2 F3 F1
	F3 F0 F1  F3 F1 F2	
	F0 F1 F2  F0 F2 F3
	F1 F2 F3  F1 F3 F0
	F0 F1 F2  F0 F2 F3
	0.0f};

#undef F0
#undef F1
#undef F2
#undef F3



/* Background and TextureBackground */
#ifdef OLDCODE
#define F0 1.0f, 1.0f,
#define F1 0.0f, 1.0f,
#define F2 0.0f, 0.0f,
#define F3 1.0f, 0.0f,

GLfloat Backtex[] = {
        F0 F1 F2  F0 F2 F3
	F3 F0 F1  F2 F3 F1
	F3 F0 F1  F3 F1 F2	
	F0 F1 F2  F0 F2 F3
	F1 F2 F3  F1 F3 F0
	F0 F1 F2  F0 F2 F3
	0.0f};
#undef F0
#undef F1
#undef F2
GLfloat oBacktex[]= {(GLfloat) 0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01,

/* junk */
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		(GLfloat)     0.99, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.99, (GLfloat)  0.01, (GLfloat) 0.01, (GLfloat)  0.99, (GLfloat) 0.01, (GLfloat) 
		0.0f};
#endif

#define PTF0 0.15f, 0.15f, -0.15f,
#define PTF1 -0.15f, 0.15f, -0.15f,
#define PTF2 -0.15f, -0.15f, -0.15f,
#define PTF3 0.15f, -0.15f, -0.15f,
#define PTR0 0.15f, 0.15f, 0.15f,
#define PTR1 -0.15f, 0.15f, 0.15f,
#define PTR2 -0.15f, -0.15f, 0.15f,
#define PTR3 0.15f, -0.15f, 0.15f,

GLfloat BackgroundVert[] = {
        PTF0 PTF1 PTF2  PTF0 PTF2 PTF3 /* front */
        PTR2 PTR1 PTR0  PTR3 PTR2 PTR0 /* back */
        PTF0 PTR0 PTR1  PTF0 PTR1 PTF1 /* top   */
        PTF3 PTF2 PTR2  PTF3 PTR2 PTR3 /* bottom*/
        PTF0 PTF3 PTR3  PTF0 PTR3 PTR0 /* right*/
        PTF1 PTR1 PTR2  PTF1 PTR2 PTF2 /* left*/
	0.0f};

#undef PTF0
#undef PTF1
#undef PTF2
#undef PTF3
#undef PTR0
#undef PTR1
#undef PTR2
#undef PTR3

/* faces are triangles, vertices; order: front, back, top, down, right, left. */
#define FT 0, 0, -1,
#define BK 0, 0, 1,
#define TP 0, -1, 0,
#define DN 0, 1, 0,
#define RT -1, 0, 0,
#define LT 1, 0, 0,

GLfloat Backnorms[] = {   
	FT FT FT    FT FT FT
	BK BK BK    BK BK BK
	TP TP TP    TP TP TP
	DN DN DN    DN DN DN
	RT RT RT    RT RT RT
	LT LT LT    LT LT LT
	0};

#undef FT
#undef BK
#undef TP
#undef DN
#undef RT
#undef LT


#ifdef OLDCODE
GLfloat Backnorms[] ={0, (GLfloat) 0, (GLfloat) -1, (GLfloat)  0, (GLfloat) 0, (GLfloat) -1, (GLfloat)  0, (GLfloat) 0, (GLfloat) -1, (GLfloat)  0, (GLfloat) 0, (GLfloat) -1, (GLfloat) 
		(GLfloat) 	0, (GLfloat) 0, (GLfloat) 1, (GLfloat)  0, (GLfloat) 0, (GLfloat) 1, (GLfloat)  0, (GLfloat) 0, (GLfloat) 1, (GLfloat)  0, (GLfloat) 0, (GLfloat) 1, (GLfloat) 
		(GLfloat) 	0, (GLfloat) 1, (GLfloat) 0, (GLfloat)  0, (GLfloat) 1, (GLfloat) 0, (GLfloat)  0, (GLfloat) 1, (GLfloat) 0, (GLfloat)  0, (GLfloat) 1, (GLfloat) 0, (GLfloat) 
		(GLfloat) 	0, (GLfloat) -1, (GLfloat) 0, (GLfloat)  0, (GLfloat) -1, (GLfloat) 0, (GLfloat)  0, (GLfloat) -1, (GLfloat) 0, (GLfloat)  0, (GLfloat) -1, (GLfloat) 0, (GLfloat) 
		(GLfloat) 	1, (GLfloat) 0, (GLfloat) 0, (GLfloat)  1, (GLfloat) 0, (GLfloat) 0, (GLfloat)  1, (GLfloat) 0, (GLfloat) 0, (GLfloat)  1, (GLfloat) 0, (GLfloat) 0, (GLfloat) 
		(GLfloat) 	-1, (GLfloat) 0, (GLfloat) 0, (GLfloat)  -1, (GLfloat) 0, (GLfloat) 0, (GLfloat)  -1, (GLfloat) 0, (GLfloat) 0, (GLfloat)  -1, (GLfloat) 0, (GLfloat) 0};

GLfloat BackgroundVert[] =  {
		(GLfloat) /* front */
		(GLfloat) 0.15, (GLfloat) 0.15, (GLfloat) -0.15, (GLfloat)  -0.15, (GLfloat) 0.15, (GLfloat) -0.15, (GLfloat)  -0.15, (GLfloat) -0.15, (GLfloat) -0.15, (GLfloat)  0.15, (GLfloat) -0.15, (GLfloat) -0.15, (GLfloat) 
		(GLfloat) /* back */
		(GLfloat) -0.15, (GLfloat)  -0.15, (GLfloat)  0.15, (GLfloat)  -0.15, (GLfloat)  0.15, (GLfloat)  0.15, (GLfloat)  0.15, (GLfloat)  0.15, (GLfloat)  0.15, (GLfloat)  0.15, (GLfloat)  -0.15, (GLfloat)  0.15, (GLfloat) 
		(GLfloat) /* top */
		(GLfloat) 0.15, (GLfloat) 0.15, (GLfloat) 0.15, (GLfloat)  -0.15, (GLfloat) 0.15, (GLfloat) 0.15, (GLfloat)  -0.15, (GLfloat) 0.15, (GLfloat) -0.15, (GLfloat)  0.15, (GLfloat) 0.15, (GLfloat) -0.15, (GLfloat) 
		(GLfloat) /* bottom */
		(GLfloat) 0.15, (GLfloat) -0.15, (GLfloat) -0.15, (GLfloat)  -0.15, (GLfloat) -0.15, (GLfloat) -0.15, (GLfloat)  -0.15, (GLfloat) -0.15, (GLfloat) 0.15, (GLfloat)  0.15, (GLfloat) -0.15, (GLfloat) 0.15, (GLfloat) 
		(GLfloat) /* right */
		(GLfloat) 0.15, (GLfloat) 0.15, (GLfloat) 0.15, (GLfloat)  0.15, (GLfloat) 0.15, (GLfloat) -0.15, (GLfloat)  0.15, (GLfloat) -0.15, (GLfloat) -0.15, (GLfloat)  0.15, (GLfloat) -0.15, (GLfloat) 0.15, (GLfloat) 
		(GLfloat) /* left */
		(GLfloat) -0.15, (GLfloat)  0.15, (GLfloat)  -0.15, (GLfloat)  -0.15, (GLfloat)  0.15, (GLfloat)  0.15, (GLfloat)  -0.15, (GLfloat)  -0.15, (GLfloat)  0.15, (GLfloat)  -0.15, (GLfloat)  -0.15, (GLfloat)  -0.15
	};
#endif


/*  CYLINDER*/

/*  simple generation; normals point outwards from each vertex.*/
/* for (i=0; i<=20; i++) {*/
/* 	a1 = PI * 2 * (i-0.5) / 20.0;*/
/* 	printf ("%4.3f, (GLfloat) 0.0, (GLfloat) %4.3f, (GLfloat)  ", (GLfloat) sin(a1), (GLfloat)   cos(a1));*/
/* 	printf ("%4.3f, (GLfloat) 0.0, (GLfloat) %4.3f, (GLfloat) \n", (GLfloat) sin(a1), (GLfloat)  cos(a1));*/
/* }*/
GLfloat cylnorms[] = { (GLfloat) -0.156, (GLfloat) 0.0, (GLfloat) 0.988, (GLfloat)  -0.156, (GLfloat) 0.0, (GLfloat) 0.988, (GLfloat) 
	0.156, (GLfloat) 0.0, (GLfloat) 0.988, (GLfloat)  0.156, (GLfloat) 0.0, (GLfloat) 0.988, (GLfloat)  0.454, (GLfloat) 0.0, (GLfloat) 0.891, (GLfloat)  0.454, (GLfloat) 0.0, (GLfloat) 0.891, (GLfloat) 
	0.707, (GLfloat) 0.0, (GLfloat) 0.707, (GLfloat)  0.707, (GLfloat) 0.0, (GLfloat) 0.707, (GLfloat)  0.891, (GLfloat) 0.0, (GLfloat) 0.454, (GLfloat)  0.891, (GLfloat) 0.0, (GLfloat) 0.454, (GLfloat) 
	0.988, (GLfloat) 0.0, (GLfloat) 0.156, (GLfloat)  0.988, (GLfloat) 0.0, (GLfloat) 0.156, (GLfloat)  0.988, (GLfloat) 0.0, (GLfloat) -0.156, (GLfloat)  0.988, (GLfloat) 0.0, (GLfloat) -0.156, (GLfloat) 
	0.891, (GLfloat) 0.0, (GLfloat) -0.454, (GLfloat)  0.891, (GLfloat) 0.0, (GLfloat) -0.454, (GLfloat)  0.707, (GLfloat) 0.0, (GLfloat) -0.707, (GLfloat)  0.707, (GLfloat) 0.0, (GLfloat) -0.707, (GLfloat) 
	0.454, (GLfloat) 0.0, (GLfloat) -0.891, (GLfloat)  0.454, (GLfloat) 0.0, (GLfloat) -0.891, (GLfloat)  0.156, (GLfloat) 0.0, (GLfloat) -0.988, (GLfloat)  0.156, (GLfloat) 0.0, (GLfloat) -0.988, (GLfloat) 
	-0.156, (GLfloat) 0.0, (GLfloat) -0.988, (GLfloat)  -0.156, (GLfloat) 0.0, (GLfloat) -0.988, (GLfloat)  -0.454, (GLfloat) 0.0, (GLfloat) -0.891, (GLfloat)  -0.454, (GLfloat) 0.0, (GLfloat) -0.891, (GLfloat) 
	-0.707, (GLfloat) 0.0, (GLfloat) -0.707, (GLfloat)  -0.707, (GLfloat) 0.0, (GLfloat) -0.707, (GLfloat)  -0.891, (GLfloat) 0.0, (GLfloat) -0.454, (GLfloat)  -0.891, (GLfloat) 0.0, (GLfloat) -0.454, (GLfloat) 
	-0.988, (GLfloat) 0.0, (GLfloat) -0.156, (GLfloat)  -0.988, (GLfloat) 0.0, (GLfloat) -0.156, (GLfloat)  -0.988, (GLfloat) 0.0, (GLfloat) 0.156, (GLfloat)  -0.988, (GLfloat) 0.0, (GLfloat) 0.156, (GLfloat) 
	-0.891, (GLfloat) 0.0, (GLfloat) 0.454, (GLfloat)  -0.891, (GLfloat) 0.0, (GLfloat) 0.454, (GLfloat)  -0.707, (GLfloat) 0.0, (GLfloat) 0.707, (GLfloat)  -0.707, (GLfloat) 0.0, (GLfloat) 0.707, (GLfloat) 
	-0.454, (GLfloat) 0.0, (GLfloat) 0.891, (GLfloat)  -0.454, (GLfloat) 0.0, (GLfloat) 0.891, (GLfloat)  -0.156, (GLfloat) 0.0, (GLfloat) 0.988, (GLfloat)  -0.156, (GLfloat) 0.0, (GLfloat) 0.988};

/*  top index into the __points generated array*/
unsigned char cyltopindx[] = {
	42, (GLfloat) 0, (GLfloat) 2, (GLfloat) 4, (GLfloat) 6, (GLfloat) 8, (GLfloat) 10, (GLfloat) 12, (GLfloat) 14, (GLfloat) 16, (GLfloat) 18, (GLfloat) 20, (GLfloat) 22, (GLfloat) 24, (GLfloat) 26, (GLfloat) 28, (GLfloat) 30, (GLfloat) 32, (GLfloat) 34, (GLfloat) 36, (GLfloat) 38, (GLfloat) 40};

/*  bottom index into the __points generated array*/
unsigned char cylbotindx[] = {43, (GLfloat) 41, (GLfloat) 39, (GLfloat) 37, (GLfloat) 35, (GLfloat) 33, (GLfloat) 31, (GLfloat) 29, (GLfloat) 27, (GLfloat) 25, (GLfloat) 23, (GLfloat) 21, (GLfloat) 
	19, (GLfloat) 17, (GLfloat) 15, (GLfloat) 13, (GLfloat) 11, (GLfloat) 9, (GLfloat) 7, (GLfloat) 5, (GLfloat) 3, (GLfloat) 1};

/*  side textures; simply 20 slices of the texture; 2 coords per slice.*/
/*  for (i=0; i<=20; i++) {*/
/* 	printf ("%4.3f, (GLfloat) 1.0, (GLfloat)  %4.3f, (GLfloat) 0.0, (GLfloat) \n", (GLfloat) (float)(i+10.0)/20.0, (GLfloat)  (float)(i+10.0)/20.0);*/
/*  }*/
GLfloat cylsidetex[] = { (GLfloat) 0.500, (GLfloat) 1.0, (GLfloat)  0.500, (GLfloat) 0.0, (GLfloat)  0.550, (GLfloat) 1.0, (GLfloat)  0.550, (GLfloat) 0.0, (GLfloat)  0.600, (GLfloat) 1.0, (GLfloat)  0.600, (GLfloat) 0.0, (GLfloat) 
	0.650, (GLfloat) 1.0, (GLfloat)  0.650, (GLfloat) 0.0, (GLfloat)  0.700, (GLfloat) 1.0, (GLfloat)  0.700, (GLfloat) 0.0, (GLfloat)  0.750, (GLfloat) 1.0, (GLfloat)  0.750, (GLfloat) 0.0, (GLfloat)  0.800, (GLfloat) 1.0, (GLfloat)  0.800, (GLfloat) 0.0, (GLfloat) 
	0.850, (GLfloat) 1.0, (GLfloat)  0.850, (GLfloat) 0.0, (GLfloat)  0.900, (GLfloat) 1.0, (GLfloat)  0.900, (GLfloat) 0.0, (GLfloat)  0.950, (GLfloat) 1.0, (GLfloat)  0.950, (GLfloat) 0.0, (GLfloat)  1.000, (GLfloat) 1.0, (GLfloat)  1.000, (GLfloat) 0.0, (GLfloat) 
	1.050, (GLfloat) 1.0, (GLfloat)  1.050, (GLfloat) 0.0, (GLfloat)  1.100, (GLfloat) 1.0, (GLfloat)  1.100, (GLfloat) 0.0, (GLfloat)  1.150, (GLfloat) 1.0, (GLfloat)  1.150, (GLfloat) 0.0, (GLfloat)  1.200, (GLfloat) 1.0, (GLfloat)  1.200, (GLfloat) 0.0, (GLfloat) 
	1.250, (GLfloat) 1.0, (GLfloat)  1.250, (GLfloat) 0.0, (GLfloat)  1.300, (GLfloat) 1.0, (GLfloat)  1.300, (GLfloat) 0.0, (GLfloat)  1.350, (GLfloat) 1.0, (GLfloat)  1.350, (GLfloat) 0.0, (GLfloat)  1.400, (GLfloat) 1.0, (GLfloat)  1.400, (GLfloat) 0.0, (GLfloat) 
	1.450, (GLfloat) 1.0, (GLfloat)  1.450, (GLfloat) 0.0, (GLfloat)  1.500, (GLfloat) 1.0, (GLfloat)  1.500, (GLfloat) 0.0};


/*  end textures; interleaved top and bottom; cyltopindx and cylbotindx are the*/
/*  indicies. Generated with:*/
/*  for (i=0; i<20; i++) {*/
/*          a1 = PI * 2 * (i) / 20.0;*/
/*          a2 = PI * 2 * (i+1) / 20.0;*/
/*          printf ("%4.3f, (GLfloat)  %4.3f, (GLfloat)  ", (GLfloat) 0.5+0.5*sin(a1), (GLfloat)   0.5+0.5*cos(a1+PI));*/
/*          printf ("%4.3f, (GLfloat)  %4.3f, (GLfloat)  ", (GLfloat) 0.5+0.5*sin(a1), (GLfloat)   0.5+0.5*cos(a1));*/
/*          printf ("\n");*/
/*  } printf ("0.5, (GLfloat) 0.5, (GLfloat) 0.5, (GLfloat) 0.5\n");*/
GLfloat cylendtex[] = {
		(GLfloat) 0.500, (GLfloat)  0.000, (GLfloat)  0.500, (GLfloat)  1.000, (GLfloat) 
		(GLfloat) 0.655, (GLfloat)  0.024, (GLfloat)  0.655, (GLfloat)  0.976, (GLfloat) 
		(GLfloat) 0.794, (GLfloat)  0.095, (GLfloat)  0.794, (GLfloat)  0.905, (GLfloat) 
		(GLfloat) 0.905, (GLfloat)  0.206, (GLfloat)  0.905, (GLfloat)  0.794, (GLfloat) 
		(GLfloat) 0.976, (GLfloat)  0.345, (GLfloat)  0.976, (GLfloat)  0.655, (GLfloat) 
		(GLfloat) 1.000, (GLfloat)  0.500, (GLfloat)  1.000, (GLfloat)  0.500, (GLfloat) 
		(GLfloat) 0.976, (GLfloat)  0.655, (GLfloat)  0.976, (GLfloat)  0.345, (GLfloat) 
		(GLfloat) 0.905, (GLfloat)  0.794, (GLfloat)  0.905, (GLfloat)  0.206, (GLfloat) 
		(GLfloat) 0.794, (GLfloat)  0.905, (GLfloat)  0.794, (GLfloat)  0.095, (GLfloat) 
		(GLfloat) 0.655, (GLfloat)  0.976, (GLfloat)  0.655, (GLfloat)  0.024, (GLfloat) 
		(GLfloat) 0.500, (GLfloat)  1.000, (GLfloat)  0.500, (GLfloat)  0.000, (GLfloat) 
		(GLfloat) 0.345, (GLfloat)  0.976, (GLfloat)  0.345, (GLfloat)  0.024, (GLfloat) 
		(GLfloat) 0.206, (GLfloat)  0.905, (GLfloat)  0.206, (GLfloat)  0.095, (GLfloat) 
		(GLfloat) 0.095, (GLfloat)  0.794, (GLfloat)  0.095, (GLfloat)  0.206, (GLfloat) 
		(GLfloat) 0.024, (GLfloat)  0.655, (GLfloat)  0.024, (GLfloat)  0.345, (GLfloat) 
		(GLfloat) 0.000, (GLfloat)  0.500, (GLfloat)  0.000, (GLfloat)  0.500, (GLfloat) 
		(GLfloat) 0.024, (GLfloat)  0.345, (GLfloat)  0.024, (GLfloat)  0.655, (GLfloat) 
		(GLfloat) 0.095, (GLfloat)  0.206, (GLfloat)  0.095, (GLfloat)  0.794, (GLfloat) 
		(GLfloat) 0.206, (GLfloat)  0.095, (GLfloat)  0.206, (GLfloat)  0.905, (GLfloat) 
		(GLfloat) 0.345, (GLfloat)  0.024, (GLfloat)  0.345, (GLfloat)  0.976, (GLfloat) 
		(GLfloat) 0.500, (GLfloat)  0.000, (GLfloat)  0.500, (GLfloat)  1.000, (GLfloat) 
		(GLfloat) 0.5, (GLfloat) 0.5, (GLfloat) 0.5, (GLfloat) 0.5
};


/*  CONE*/
/* indexes for arrays for bottom of cone */
unsigned char tribotindx[] = {21, (GLfloat) 20, (GLfloat) 19, (GLfloat) 18, (GLfloat) 17, (GLfloat) 16, (GLfloat) 15, (GLfloat) 14, (GLfloat) 13, (GLfloat) 12, (GLfloat) 11, (GLfloat) 
		(GLfloat) 	10, (GLfloat) 9, (GLfloat) 8, (GLfloat) 7, (GLfloat) 6, (GLfloat) 5, (GLfloat) 4, (GLfloat) 3, (GLfloat) 2, (GLfloat) 1, (GLfloat) 22};

/* texture mapping indexes for bottom of cone */
/* generated with:
	printf ("0.0, (GLfloat)  0.0");
	for (i=1; i<=20; i++) {
	        a1 = PI * 2 * (i) / 20.0;
	        printf ("%4.3f, (GLfloat)  %4.3f, (GLfloat)  ", (GLfloat) 0.5+0.5*sin(a1), (GLfloat)   0.5+0.5*cos(a1));
	}
	printf ("0.5, (GLfloat) 0.5, (GLfloat) 0.5, (GLfloat) 1.0\n");
*/
GLfloat tribottex[] = { 0, (GLfloat) 0, (GLfloat)  0.655, (GLfloat)  0.976, (GLfloat)  0.794, (GLfloat)  0.905, (GLfloat)  0.905, (GLfloat)  0.794, (GLfloat) 
		(GLfloat) 0.976, (GLfloat)  0.655, (GLfloat)  1.000, (GLfloat)  0.500, (GLfloat)  0.976, (GLfloat)  0.345, (GLfloat)  0.905, (GLfloat)  0.206, (GLfloat) 
		(GLfloat) 0.794, (GLfloat)  0.095, (GLfloat)  0.655, (GLfloat)  0.024, (GLfloat)  0.500, (GLfloat)  0.000, (GLfloat)  0.345, (GLfloat)  0.024, (GLfloat) 
		(GLfloat) 0.206, (GLfloat)  0.095, (GLfloat)  0.095, (GLfloat)  0.206, (GLfloat)  0.024, (GLfloat)  0.345, (GLfloat)  0.000, (GLfloat)  0.500, (GLfloat) 
		(GLfloat) 0.024, (GLfloat)  0.655, (GLfloat)  0.095, (GLfloat)  0.794, (GLfloat)  0.206, (GLfloat)  0.905, (GLfloat)  0.345, (GLfloat)  0.976, (GLfloat) 
		(GLfloat) 0.500, (GLfloat)  1.000, (GLfloat)  0.5, (GLfloat) 0.5, (GLfloat) 0.5, (GLfloat) 1.0 };

GLfloat trisidtex[] = {
		(GLfloat) 	0.575, (GLfloat) 1.0, (GLfloat)  0.55, (GLfloat) 0.0, (GLfloat)  0.60, (GLfloat) 0.0, (GLfloat)  /* 12*/
		(GLfloat) 	0.625, (GLfloat) 1.0, (GLfloat)  0.60, (GLfloat) 0.0, (GLfloat)  0.65, (GLfloat) 0.0, (GLfloat)  /* 13*/
		(GLfloat) 	0.675, (GLfloat) 1.0, (GLfloat)  0.65, (GLfloat) 0.0, (GLfloat)  0.70, (GLfloat) 0.0, (GLfloat)  /* 14*/
		(GLfloat) 	0.725, (GLfloat) 1.0, (GLfloat)  0.70, (GLfloat) 0.0, (GLfloat)  0.75, (GLfloat) 0.0, (GLfloat)  /* 15*/
		(GLfloat) 	0.775, (GLfloat) 1.0, (GLfloat)  0.75, (GLfloat) 0.0, (GLfloat)  0.80, (GLfloat) 0.0, (GLfloat)  /* 16*/
		(GLfloat) 	0.825, (GLfloat) 1.0, (GLfloat)  0.80, (GLfloat) 0.0, (GLfloat)  0.85, (GLfloat) 0.0, (GLfloat)  /* 17*/
		(GLfloat) 	0.875, (GLfloat) 1.0, (GLfloat)  0.85, (GLfloat) 0.0, (GLfloat)  0.90, (GLfloat) 0.0, (GLfloat)  /* 18*/
		(GLfloat) 	0.925, (GLfloat) 1.0, (GLfloat)  0.90, (GLfloat) 0.0, (GLfloat)  0.95, (GLfloat) 0.0, (GLfloat)  /* 19*/
		(GLfloat) 	0.975, (GLfloat) 1.0, (GLfloat)  0.95, (GLfloat) 0.0, (GLfloat)  1.0, (GLfloat) 0.0, (GLfloat)    /* 20*/
		(GLfloat) 	0.025, (GLfloat) 1.0, (GLfloat)  0.00, (GLfloat) 0.0, (GLfloat)  0.05, (GLfloat) 0.0, (GLfloat)  /* 1*/
		(GLfloat) 	0.075, (GLfloat) 1.0, (GLfloat)  0.05, (GLfloat) 0.0, (GLfloat)  0.10, (GLfloat) 0.0, (GLfloat)  /* 2*/
		(GLfloat) 	0.125, (GLfloat) 1.0, (GLfloat)  0.10, (GLfloat) 0.0, (GLfloat)  0.15, (GLfloat) 0.0, (GLfloat)  /* 3*/
		(GLfloat) 	0.175, (GLfloat) 1.0, (GLfloat)  0.15, (GLfloat) 0.0, (GLfloat)  0.20, (GLfloat) 0.0, (GLfloat)  /* 4*/
		(GLfloat) 	0.225, (GLfloat) 1.0, (GLfloat)  0.20, (GLfloat) 0.0, (GLfloat)  0.25, (GLfloat) 0.0, (GLfloat)  /* 5*/
		(GLfloat) 	0.275, (GLfloat) 1.0, (GLfloat)  0.25, (GLfloat) 0.0, (GLfloat)  0.30, (GLfloat) 0.0, (GLfloat)  /* 6*/
		(GLfloat) 	0.325, (GLfloat) 1.0, (GLfloat)  0.30, (GLfloat) 0.0, (GLfloat)  0.35, (GLfloat) 0.0, (GLfloat)  /* 7*/
		(GLfloat) 	0.375, (GLfloat) 1.0, (GLfloat)  0.35, (GLfloat) 0.0, (GLfloat)  0.40, (GLfloat) 0.0, (GLfloat)  /* 8*/
		(GLfloat) 	0.425, (GLfloat) 1.0, (GLfloat)  0.40, (GLfloat) 0.0, (GLfloat)  0.45, (GLfloat) 0.0, (GLfloat)  /* 9*/
		(GLfloat) 	0.475, (GLfloat) 1.0, (GLfloat)  0.45, (GLfloat) 0.0, (GLfloat)  0.50, (GLfloat) 0.0, (GLfloat)  /* 10*/
		(GLfloat) 	0.525, (GLfloat) 1.0, (GLfloat)  0.50, (GLfloat) 0.0, (GLfloat)  0.55, (GLfloat) 0.0 /* 11*/
		};


/*  SPHERE*/

GLfloat spheretex[] = {
		(GLfloat) 0.000, (GLfloat) 0.100, (GLfloat)  0.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 0.100, (GLfloat)  0.050, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 0.100, (GLfloat)  0.100, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 0.100, (GLfloat)  0.150, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 0.100, (GLfloat)  0.200, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.100, (GLfloat)  0.250, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 0.100, (GLfloat)  0.300, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 0.100, (GLfloat)  0.350, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 0.100, (GLfloat)  0.400, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 0.100, (GLfloat)  0.450, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 0.100, (GLfloat)  0.500, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 0.100, (GLfloat)  0.550, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 0.100, (GLfloat)  0.600, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 0.100, (GLfloat)  0.650, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 0.100, (GLfloat)  0.700, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 0.100, (GLfloat)  0.750, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 0.100, (GLfloat)  0.800, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 0.100, (GLfloat)  0.850, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 0.100, (GLfloat)  0.900, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 0.100, (GLfloat)  0.950, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.100, (GLfloat)  1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.200, (GLfloat)  0.000, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 0.200, (GLfloat)  0.050, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 0.200, (GLfloat)  0.100, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 0.200, (GLfloat)  0.150, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 0.200, (GLfloat)  0.200, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.200, (GLfloat)  0.250, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 0.200, (GLfloat)  0.300, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 0.200, (GLfloat)  0.350, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 0.200, (GLfloat)  0.400, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 0.200, (GLfloat)  0.450, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 0.200, (GLfloat)  0.500, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 0.200, (GLfloat)  0.550, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 0.200, (GLfloat)  0.600, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 0.200, (GLfloat)  0.650, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 0.200, (GLfloat)  0.700, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 0.200, (GLfloat)  0.750, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 0.200, (GLfloat)  0.800, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 0.200, (GLfloat)  0.850, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 0.200, (GLfloat)  0.900, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 0.200, (GLfloat)  0.950, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.200, (GLfloat)  1.000, (GLfloat) 0.100, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.300, (GLfloat)  0.000, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 0.300, (GLfloat)  0.050, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 0.300, (GLfloat)  0.100, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 0.300, (GLfloat)  0.150, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 0.300, (GLfloat)  0.200, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.300, (GLfloat)  0.250, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 0.300, (GLfloat)  0.300, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 0.300, (GLfloat)  0.350, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 0.300, (GLfloat)  0.400, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 0.300, (GLfloat)  0.450, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 0.300, (GLfloat)  0.500, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 0.300, (GLfloat)  0.550, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 0.300, (GLfloat)  0.600, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 0.300, (GLfloat)  0.650, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 0.300, (GLfloat)  0.700, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 0.300, (GLfloat)  0.750, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 0.300, (GLfloat)  0.800, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 0.300, (GLfloat)  0.850, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 0.300, (GLfloat)  0.900, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 0.300, (GLfloat)  0.950, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.300, (GLfloat)  1.000, (GLfloat) 0.200, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.400, (GLfloat)  0.000, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 0.400, (GLfloat)  0.050, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 0.400, (GLfloat)  0.100, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 0.400, (GLfloat)  0.150, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 0.400, (GLfloat)  0.200, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.400, (GLfloat)  0.250, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 0.400, (GLfloat)  0.300, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 0.400, (GLfloat)  0.350, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 0.400, (GLfloat)  0.400, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 0.400, (GLfloat)  0.450, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 0.400, (GLfloat)  0.500, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 0.400, (GLfloat)  0.550, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 0.400, (GLfloat)  0.600, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 0.400, (GLfloat)  0.650, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 0.400, (GLfloat)  0.700, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 0.400, (GLfloat)  0.750, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 0.400, (GLfloat)  0.800, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 0.400, (GLfloat)  0.850, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 0.400, (GLfloat)  0.900, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 0.400, (GLfloat)  0.950, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.400, (GLfloat)  1.000, (GLfloat) 0.300, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.500, (GLfloat)  0.000, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 0.500, (GLfloat)  0.050, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 0.500, (GLfloat)  0.100, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 0.500, (GLfloat)  0.150, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 0.500, (GLfloat)  0.200, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.500, (GLfloat)  0.250, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 0.500, (GLfloat)  0.300, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 0.500, (GLfloat)  0.350, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 0.500, (GLfloat)  0.400, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 0.500, (GLfloat)  0.450, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 0.500, (GLfloat)  0.500, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 0.500, (GLfloat)  0.550, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 0.500, (GLfloat)  0.600, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 0.500, (GLfloat)  0.650, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 0.500, (GLfloat)  0.700, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 0.500, (GLfloat)  0.750, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 0.500, (GLfloat)  0.800, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 0.500, (GLfloat)  0.850, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 0.500, (GLfloat)  0.900, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 0.500, (GLfloat)  0.950, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.500, (GLfloat)  1.000, (GLfloat) 0.400, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.600, (GLfloat)  0.000, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 0.600, (GLfloat)  0.050, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 0.600, (GLfloat)  0.100, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 0.600, (GLfloat)  0.150, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 0.600, (GLfloat)  0.200, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.600, (GLfloat)  0.250, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 0.600, (GLfloat)  0.300, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 0.600, (GLfloat)  0.350, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 0.600, (GLfloat)  0.400, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 0.600, (GLfloat)  0.450, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 0.600, (GLfloat)  0.500, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 0.600, (GLfloat)  0.550, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 0.600, (GLfloat)  0.600, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 0.600, (GLfloat)  0.650, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 0.600, (GLfloat)  0.700, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 0.600, (GLfloat)  0.750, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 0.600, (GLfloat)  0.800, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 0.600, (GLfloat)  0.850, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 0.600, (GLfloat)  0.900, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 0.600, (GLfloat)  0.950, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.600, (GLfloat)  1.000, (GLfloat) 0.500, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.700, (GLfloat)  0.000, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 0.700, (GLfloat)  0.050, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 0.700, (GLfloat)  0.100, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 0.700, (GLfloat)  0.150, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 0.700, (GLfloat)  0.200, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.700, (GLfloat)  0.250, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 0.700, (GLfloat)  0.300, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 0.700, (GLfloat)  0.350, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 0.700, (GLfloat)  0.400, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 0.700, (GLfloat)  0.450, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 0.700, (GLfloat)  0.500, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 0.700, (GLfloat)  0.550, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 0.700, (GLfloat)  0.600, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 0.700, (GLfloat)  0.650, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 0.700, (GLfloat)  0.700, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 0.700, (GLfloat)  0.750, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 0.700, (GLfloat)  0.800, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 0.700, (GLfloat)  0.850, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 0.700, (GLfloat)  0.900, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 0.700, (GLfloat)  0.950, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.700, (GLfloat)  1.000, (GLfloat) 0.600, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.800, (GLfloat)  0.000, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 0.800, (GLfloat)  0.050, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 0.800, (GLfloat)  0.100, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 0.800, (GLfloat)  0.150, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 0.800, (GLfloat)  0.200, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.800, (GLfloat)  0.250, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 0.800, (GLfloat)  0.300, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 0.800, (GLfloat)  0.350, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 0.800, (GLfloat)  0.400, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 0.800, (GLfloat)  0.450, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 0.800, (GLfloat)  0.500, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 0.800, (GLfloat)  0.550, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 0.800, (GLfloat)  0.600, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 0.800, (GLfloat)  0.650, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 0.800, (GLfloat)  0.700, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 0.800, (GLfloat)  0.750, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 0.800, (GLfloat)  0.800, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 0.800, (GLfloat)  0.850, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 0.800, (GLfloat)  0.900, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 0.800, (GLfloat)  0.950, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.800, (GLfloat)  1.000, (GLfloat) 0.700, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.900, (GLfloat)  0.000, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 0.900, (GLfloat)  0.050, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 0.900, (GLfloat)  0.100, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 0.900, (GLfloat)  0.150, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 0.900, (GLfloat)  0.200, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.900, (GLfloat)  0.250, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 0.900, (GLfloat)  0.300, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 0.900, (GLfloat)  0.350, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 0.900, (GLfloat)  0.400, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 0.900, (GLfloat)  0.450, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 0.900, (GLfloat)  0.500, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 0.900, (GLfloat)  0.550, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 0.900, (GLfloat)  0.600, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 0.900, (GLfloat)  0.650, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 0.900, (GLfloat)  0.700, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 0.900, (GLfloat)  0.750, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 0.900, (GLfloat)  0.800, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 0.900, (GLfloat)  0.850, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 0.900, (GLfloat)  0.900, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 0.900, (GLfloat)  0.950, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.900, (GLfloat)  1.000, (GLfloat) 0.800, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat)  0.000, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.000, (GLfloat)  0.050, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.000, (GLfloat)  0.100, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.000, (GLfloat)  0.150, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.000, (GLfloat)  0.200, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.000, (GLfloat)  0.250, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.000, (GLfloat)  0.300, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.000, (GLfloat)  0.350, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.000, (GLfloat)  0.400, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.000, (GLfloat)  0.450, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.000, (GLfloat)  0.500, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.000, (GLfloat)  0.550, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.000, (GLfloat)  0.600, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.000, (GLfloat)  0.650, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.000, (GLfloat)  0.700, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.000, (GLfloat)  0.750, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.000, (GLfloat)  0.800, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.000, (GLfloat)  0.850, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.000, (GLfloat)  0.900, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.000, (GLfloat)  0.950, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.000, (GLfloat)  1.000, (GLfloat) 0.900, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.100, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.100, (GLfloat)  0.050, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.100, (GLfloat)  0.100, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.100, (GLfloat)  0.150, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.100, (GLfloat)  0.200, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.100, (GLfloat)  0.250, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.100, (GLfloat)  0.300, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.100, (GLfloat)  0.350, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.100, (GLfloat)  0.400, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.100, (GLfloat)  0.450, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.100, (GLfloat)  0.500, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.100, (GLfloat)  0.550, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.100, (GLfloat)  0.600, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.100, (GLfloat)  0.650, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.100, (GLfloat)  0.700, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.100, (GLfloat)  0.750, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.100, (GLfloat)  0.800, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.100, (GLfloat)  0.850, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.100, (GLfloat)  0.900, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.100, (GLfloat)  0.950, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.100, (GLfloat)  1.000, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.200, (GLfloat)  0.000, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.200, (GLfloat)  0.050, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.200, (GLfloat)  0.100, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.200, (GLfloat)  0.150, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.200, (GLfloat)  0.200, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.200, (GLfloat)  0.250, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.200, (GLfloat)  0.300, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.200, (GLfloat)  0.350, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.200, (GLfloat)  0.400, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.200, (GLfloat)  0.450, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.200, (GLfloat)  0.500, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.200, (GLfloat)  0.550, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.200, (GLfloat)  0.600, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.200, (GLfloat)  0.650, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.200, (GLfloat)  0.700, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.200, (GLfloat)  0.750, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.200, (GLfloat)  0.800, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.200, (GLfloat)  0.850, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.200, (GLfloat)  0.900, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.200, (GLfloat)  0.950, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.200, (GLfloat)  1.000, (GLfloat) 1.100, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.300, (GLfloat)  0.000, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.300, (GLfloat)  0.050, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.300, (GLfloat)  0.100, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.300, (GLfloat)  0.150, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.300, (GLfloat)  0.200, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.300, (GLfloat)  0.250, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.300, (GLfloat)  0.300, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.300, (GLfloat)  0.350, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.300, (GLfloat)  0.400, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.300, (GLfloat)  0.450, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.300, (GLfloat)  0.500, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.300, (GLfloat)  0.550, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.300, (GLfloat)  0.600, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.300, (GLfloat)  0.650, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.300, (GLfloat)  0.700, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.300, (GLfloat)  0.750, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.300, (GLfloat)  0.800, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.300, (GLfloat)  0.850, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.300, (GLfloat)  0.900, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.300, (GLfloat)  0.950, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.300, (GLfloat)  1.000, (GLfloat) 1.200, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.400, (GLfloat)  0.000, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.400, (GLfloat)  0.050, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.400, (GLfloat)  0.100, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.400, (GLfloat)  0.150, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.400, (GLfloat)  0.200, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.400, (GLfloat)  0.250, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.400, (GLfloat)  0.300, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.400, (GLfloat)  0.350, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.400, (GLfloat)  0.400, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.400, (GLfloat)  0.450, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.400, (GLfloat)  0.500, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.400, (GLfloat)  0.550, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.400, (GLfloat)  0.600, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.400, (GLfloat)  0.650, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.400, (GLfloat)  0.700, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.400, (GLfloat)  0.750, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.400, (GLfloat)  0.800, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.400, (GLfloat)  0.850, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.400, (GLfloat)  0.900, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.400, (GLfloat)  0.950, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.400, (GLfloat)  1.000, (GLfloat) 1.300, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.500, (GLfloat)  0.000, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.500, (GLfloat)  0.050, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.500, (GLfloat)  0.100, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.500, (GLfloat)  0.150, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.500, (GLfloat)  0.200, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.500, (GLfloat)  0.250, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.500, (GLfloat)  0.300, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.500, (GLfloat)  0.350, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.500, (GLfloat)  0.400, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.500, (GLfloat)  0.450, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.500, (GLfloat)  0.500, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.500, (GLfloat)  0.550, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.500, (GLfloat)  0.600, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.500, (GLfloat)  0.650, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.500, (GLfloat)  0.700, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.500, (GLfloat)  0.750, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.500, (GLfloat)  0.800, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.500, (GLfloat)  0.850, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.500, (GLfloat)  0.900, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.500, (GLfloat)  0.950, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.500, (GLfloat)  1.000, (GLfloat) 1.400, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.600, (GLfloat)  0.000, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.600, (GLfloat)  0.050, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.600, (GLfloat)  0.100, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.600, (GLfloat)  0.150, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.600, (GLfloat)  0.200, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.600, (GLfloat)  0.250, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.600, (GLfloat)  0.300, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.600, (GLfloat)  0.350, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.600, (GLfloat)  0.400, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.600, (GLfloat)  0.450, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.600, (GLfloat)  0.500, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.600, (GLfloat)  0.550, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.600, (GLfloat)  0.600, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.600, (GLfloat)  0.650, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.600, (GLfloat)  0.700, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.600, (GLfloat)  0.750, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.600, (GLfloat)  0.800, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.600, (GLfloat)  0.850, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.600, (GLfloat)  0.900, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.600, (GLfloat)  0.950, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.600, (GLfloat)  1.000, (GLfloat) 1.500, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.700, (GLfloat)  0.000, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.700, (GLfloat)  0.050, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.700, (GLfloat)  0.100, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.700, (GLfloat)  0.150, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.700, (GLfloat)  0.200, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.700, (GLfloat)  0.250, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.700, (GLfloat)  0.300, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.700, (GLfloat)  0.350, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.700, (GLfloat)  0.400, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.700, (GLfloat)  0.450, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.700, (GLfloat)  0.500, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.700, (GLfloat)  0.550, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.700, (GLfloat)  0.600, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.700, (GLfloat)  0.650, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.700, (GLfloat)  0.700, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.700, (GLfloat)  0.750, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.700, (GLfloat)  0.800, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.700, (GLfloat)  0.850, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.700, (GLfloat)  0.900, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.700, (GLfloat)  0.950, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.700, (GLfloat)  1.000, (GLfloat) 1.600, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.800, (GLfloat)  0.000, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.800, (GLfloat)  0.050, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.800, (GLfloat)  0.100, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.800, (GLfloat)  0.150, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.800, (GLfloat)  0.200, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.800, (GLfloat)  0.250, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.800, (GLfloat)  0.300, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.800, (GLfloat)  0.350, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.800, (GLfloat)  0.400, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.800, (GLfloat)  0.450, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.800, (GLfloat)  0.500, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.800, (GLfloat)  0.550, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.800, (GLfloat)  0.600, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.800, (GLfloat)  0.650, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.800, (GLfloat)  0.700, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.800, (GLfloat)  0.750, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.800, (GLfloat)  0.800, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.800, (GLfloat)  0.850, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.800, (GLfloat)  0.900, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.800, (GLfloat)  0.950, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.800, (GLfloat)  1.000, (GLfloat) 1.700, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.900, (GLfloat)  0.000, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 1.900, (GLfloat)  0.050, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 1.900, (GLfloat)  0.100, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 1.900, (GLfloat)  0.150, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 1.900, (GLfloat)  0.200, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 1.900, (GLfloat)  0.250, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 1.900, (GLfloat)  0.300, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 1.900, (GLfloat)  0.350, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 1.900, (GLfloat)  0.400, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 1.900, (GLfloat)  0.450, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 1.900, (GLfloat)  0.500, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 1.900, (GLfloat)  0.550, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 1.900, (GLfloat)  0.600, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 1.900, (GLfloat)  0.650, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 1.900, (GLfloat)  0.700, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 1.900, (GLfloat)  0.750, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 1.900, (GLfloat)  0.800, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 1.900, (GLfloat)  0.850, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 1.900, (GLfloat)  0.900, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 1.900, (GLfloat)  0.950, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 1.900, (GLfloat)  1.000, (GLfloat) 1.800, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 2.000, (GLfloat)  0.000, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.050, (GLfloat) 2.000, (GLfloat)  0.050, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.100, (GLfloat) 2.000, (GLfloat)  0.100, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.150, (GLfloat) 2.000, (GLfloat)  0.150, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.200, (GLfloat) 2.000, (GLfloat)  0.200, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 2.000, (GLfloat)  0.250, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.300, (GLfloat) 2.000, (GLfloat)  0.300, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.350, (GLfloat) 2.000, (GLfloat)  0.350, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.400, (GLfloat) 2.000, (GLfloat)  0.400, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.450, (GLfloat) 2.000, (GLfloat)  0.450, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.500, (GLfloat) 2.000, (GLfloat)  0.500, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.550, (GLfloat) 2.000, (GLfloat)  0.550, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.600, (GLfloat) 2.000, (GLfloat)  0.600, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.650, (GLfloat) 2.000, (GLfloat)  0.650, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.700, (GLfloat) 2.000, (GLfloat)  0.700, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.750, (GLfloat) 2.000, (GLfloat)  0.750, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.800, (GLfloat) 2.000, (GLfloat)  0.800, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.850, (GLfloat) 2.000, (GLfloat)  0.850, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.900, (GLfloat) 2.000, (GLfloat)  0.900, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 0.950, (GLfloat) 2.000, (GLfloat)  0.950, (GLfloat) 1.900, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 2.000, (GLfloat)  1.000, (GLfloat) 1.900
};

GLfloat spherenorms[] = { 
		(GLfloat) 0.000, (GLfloat) -0.951, (GLfloat) -0.309, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.095, (GLfloat) -0.951, (GLfloat) -0.294, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) -0.951, (GLfloat) -0.250, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) -0.951, (GLfloat) -0.182, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) -0.951, (GLfloat) -0.095, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.309, (GLfloat) -0.951, (GLfloat) 0.000, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) -0.951, (GLfloat) 0.095, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) -0.951, (GLfloat) 0.182, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) -0.951, (GLfloat) 0.250, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.095, (GLfloat) -0.951, (GLfloat) 0.294, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.951, (GLfloat) 0.309, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.095, (GLfloat) -0.951, (GLfloat) 0.294, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) -0.951, (GLfloat) 0.250, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) -0.951, (GLfloat) 0.182, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) -0.951, (GLfloat) 0.095, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.309, (GLfloat) -0.951, (GLfloat) -0.000, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) -0.951, (GLfloat) -0.095, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) -0.951, (GLfloat) -0.182, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) -0.951, (GLfloat) -0.250, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.095, (GLfloat) -0.951, (GLfloat) -0.294, (GLfloat)  0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.951, (GLfloat) -0.309, (GLfloat)  -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.809, (GLfloat) -0.588, (GLfloat)  0.000, (GLfloat) -0.951, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) -0.809, (GLfloat) -0.559, (GLfloat)  -0.095, (GLfloat) -0.951, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) -0.345, (GLfloat) -0.809, (GLfloat) -0.476, (GLfloat)  -0.182, (GLfloat) -0.951, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) -0.809, (GLfloat) -0.345, (GLfloat)  -0.250, (GLfloat) -0.951, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) -0.809, (GLfloat) -0.182, (GLfloat)  -0.294, (GLfloat) -0.951, (GLfloat) -0.095, (GLfloat) 
		(GLfloat) -0.588, (GLfloat) -0.809, (GLfloat) 0.000, (GLfloat)  -0.309, (GLfloat) -0.951, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) -0.809, (GLfloat) 0.182, (GLfloat)  -0.294, (GLfloat) -0.951, (GLfloat) 0.095, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) -0.809, (GLfloat) 0.345, (GLfloat)  -0.250, (GLfloat) -0.951, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) -0.345, (GLfloat) -0.809, (GLfloat) 0.476, (GLfloat)  -0.182, (GLfloat) -0.951, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) -0.809, (GLfloat) 0.559, (GLfloat)  -0.095, (GLfloat) -0.951, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.809, (GLfloat) 0.588, (GLfloat)  0.000, (GLfloat) -0.951, (GLfloat) 0.309, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) -0.809, (GLfloat) 0.559, (GLfloat)  0.095, (GLfloat) -0.951, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) 0.345, (GLfloat) -0.809, (GLfloat) 0.476, (GLfloat)  0.182, (GLfloat) -0.951, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) -0.809, (GLfloat) 0.345, (GLfloat)  0.250, (GLfloat) -0.951, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) -0.809, (GLfloat) 0.182, (GLfloat)  0.294, (GLfloat) -0.951, (GLfloat) 0.095, (GLfloat) 
		(GLfloat) 0.588, (GLfloat) -0.809, (GLfloat) -0.000, (GLfloat)  0.309, (GLfloat) -0.951, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) -0.809, (GLfloat) -0.182, (GLfloat)  0.294, (GLfloat) -0.951, (GLfloat) -0.095, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) -0.809, (GLfloat) -0.345, (GLfloat)  0.250, (GLfloat) -0.951, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) 0.345, (GLfloat) -0.809, (GLfloat) -0.476, (GLfloat)  0.182, (GLfloat) -0.951, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) -0.809, (GLfloat) -0.559, (GLfloat)  0.095, (GLfloat) -0.951, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.809, (GLfloat) -0.588, (GLfloat)  -0.000, (GLfloat) -0.951, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.588, (GLfloat) -0.809, (GLfloat)  0.000, (GLfloat) -0.809, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) -0.588, (GLfloat) -0.769, (GLfloat)  -0.182, (GLfloat) -0.809, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) -0.588, (GLfloat) -0.655, (GLfloat)  -0.345, (GLfloat) -0.809, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) -0.655, (GLfloat) -0.588, (GLfloat) -0.476, (GLfloat)  -0.476, (GLfloat) -0.809, (GLfloat) -0.345, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) -0.588, (GLfloat) -0.250, (GLfloat)  -0.559, (GLfloat) -0.809, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) -0.809, (GLfloat) -0.588, (GLfloat) 0.000, (GLfloat)  -0.588, (GLfloat) -0.809, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) -0.588, (GLfloat) 0.250, (GLfloat)  -0.559, (GLfloat) -0.809, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) -0.655, (GLfloat) -0.588, (GLfloat) 0.476, (GLfloat)  -0.476, (GLfloat) -0.809, (GLfloat) 0.345, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) -0.588, (GLfloat) 0.655, (GLfloat)  -0.345, (GLfloat) -0.809, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) -0.588, (GLfloat) 0.769, (GLfloat)  -0.182, (GLfloat) -0.809, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.588, (GLfloat) 0.809, (GLfloat)  0.000, (GLfloat) -0.809, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) -0.588, (GLfloat) 0.769, (GLfloat)  0.182, (GLfloat) -0.809, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) -0.588, (GLfloat) 0.655, (GLfloat)  0.345, (GLfloat) -0.809, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) 0.655, (GLfloat) -0.588, (GLfloat) 0.476, (GLfloat)  0.476, (GLfloat) -0.809, (GLfloat) 0.345, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) -0.588, (GLfloat) 0.250, (GLfloat)  0.559, (GLfloat) -0.809, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) 0.809, (GLfloat) -0.588, (GLfloat) -0.000, (GLfloat)  0.588, (GLfloat) -0.809, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) -0.588, (GLfloat) -0.250, (GLfloat)  0.559, (GLfloat) -0.809, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) 0.655, (GLfloat) -0.588, (GLfloat) -0.476, (GLfloat)  0.476, (GLfloat) -0.809, (GLfloat) -0.345, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) -0.588, (GLfloat) -0.655, (GLfloat)  0.345, (GLfloat) -0.809, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) -0.588, (GLfloat) -0.769, (GLfloat)  0.182, (GLfloat) -0.809, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.588, (GLfloat) -0.809, (GLfloat)  -0.000, (GLfloat) -0.809, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.309, (GLfloat) -0.951, (GLfloat)  0.000, (GLfloat) -0.588, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) -0.309, (GLfloat) -0.905, (GLfloat)  -0.250, (GLfloat) -0.588, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) -0.309, (GLfloat) -0.769, (GLfloat)  -0.476, (GLfloat) -0.588, (GLfloat) -0.655, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) -0.309, (GLfloat) -0.559, (GLfloat)  -0.655, (GLfloat) -0.588, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) -0.905, (GLfloat) -0.309, (GLfloat) -0.294, (GLfloat)  -0.769, (GLfloat) -0.588, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) -0.951, (GLfloat) -0.309, (GLfloat) 0.000, (GLfloat)  -0.809, (GLfloat) -0.588, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.905, (GLfloat) -0.309, (GLfloat) 0.294, (GLfloat)  -0.769, (GLfloat) -0.588, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) -0.309, (GLfloat) 0.559, (GLfloat)  -0.655, (GLfloat) -0.588, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) -0.309, (GLfloat) 0.769, (GLfloat)  -0.476, (GLfloat) -0.588, (GLfloat) 0.655, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) -0.309, (GLfloat) 0.905, (GLfloat)  -0.250, (GLfloat) -0.588, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.309, (GLfloat) 0.951, (GLfloat)  0.000, (GLfloat) -0.588, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) -0.309, (GLfloat) 0.905, (GLfloat)  0.250, (GLfloat) -0.588, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) -0.309, (GLfloat) 0.769, (GLfloat)  0.476, (GLfloat) -0.588, (GLfloat) 0.655, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) -0.309, (GLfloat) 0.559, (GLfloat)  0.655, (GLfloat) -0.588, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) 0.905, (GLfloat) -0.309, (GLfloat) 0.294, (GLfloat)  0.769, (GLfloat) -0.588, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) 0.951, (GLfloat) -0.309, (GLfloat) -0.000, (GLfloat)  0.809, (GLfloat) -0.588, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.905, (GLfloat) -0.309, (GLfloat) -0.294, (GLfloat)  0.769, (GLfloat) -0.588, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) -0.309, (GLfloat) -0.559, (GLfloat)  0.655, (GLfloat) -0.588, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) -0.309, (GLfloat) -0.769, (GLfloat)  0.476, (GLfloat) -0.588, (GLfloat) -0.655, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) -0.309, (GLfloat) -0.905, (GLfloat)  0.250, (GLfloat) -0.588, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.309, (GLfloat) -0.951, (GLfloat)  -0.000, (GLfloat) -0.588, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.000, (GLfloat) -1.000, (GLfloat)  0.000, (GLfloat) -0.309, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) -0.309, (GLfloat) 0.000, (GLfloat) -0.951, (GLfloat)  -0.294, (GLfloat) -0.309, (GLfloat) -0.905, (GLfloat) 
		(GLfloat) -0.588, (GLfloat) 0.000, (GLfloat) -0.809, (GLfloat)  -0.559, (GLfloat) -0.309, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.809, (GLfloat) 0.000, (GLfloat) -0.588, (GLfloat)  -0.769, (GLfloat) -0.309, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.951, (GLfloat) 0.000, (GLfloat) -0.309, (GLfloat)  -0.905, (GLfloat) -0.309, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) -1.000, (GLfloat) 0.000, (GLfloat) 0.000, (GLfloat)  -0.951, (GLfloat) -0.309, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.951, (GLfloat) 0.000, (GLfloat) 0.309, (GLfloat)  -0.905, (GLfloat) -0.309, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) -0.809, (GLfloat) 0.000, (GLfloat) 0.588, (GLfloat)  -0.769, (GLfloat) -0.309, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) -0.588, (GLfloat) 0.000, (GLfloat) 0.809, (GLfloat)  -0.559, (GLfloat) -0.309, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) -0.309, (GLfloat) 0.000, (GLfloat) 0.951, (GLfloat)  -0.294, (GLfloat) -0.309, (GLfloat) 0.905, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.000, (GLfloat) 1.000, (GLfloat)  0.000, (GLfloat) -0.309, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) 0.309, (GLfloat) 0.000, (GLfloat) 0.951, (GLfloat)  0.294, (GLfloat) -0.309, (GLfloat) 0.905, (GLfloat) 
		(GLfloat) 0.588, (GLfloat) 0.000, (GLfloat) 0.809, (GLfloat)  0.559, (GLfloat) -0.309, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.809, (GLfloat) 0.000, (GLfloat) 0.588, (GLfloat)  0.769, (GLfloat) -0.309, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.951, (GLfloat) 0.000, (GLfloat) 0.309, (GLfloat)  0.905, (GLfloat) -0.309, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) -0.000, (GLfloat)  0.951, (GLfloat) -0.309, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.951, (GLfloat) 0.000, (GLfloat) -0.309, (GLfloat)  0.905, (GLfloat) -0.309, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) 0.809, (GLfloat) 0.000, (GLfloat) -0.588, (GLfloat)  0.769, (GLfloat) -0.309, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) 0.588, (GLfloat) 0.000, (GLfloat) -0.809, (GLfloat)  0.559, (GLfloat) -0.309, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) 0.309, (GLfloat) 0.000, (GLfloat) -0.951, (GLfloat)  0.294, (GLfloat) -0.309, (GLfloat) -0.905, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.000, (GLfloat) -1.000, (GLfloat)  -0.000, (GLfloat) -0.309, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.309, (GLfloat) -0.951, (GLfloat)  0.000, (GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) 0.309, (GLfloat) -0.905, (GLfloat)  -0.309, (GLfloat) 0.000, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) 0.309, (GLfloat) -0.769, (GLfloat)  -0.588, (GLfloat) 0.000, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) 0.309, (GLfloat) -0.559, (GLfloat)  -0.809, (GLfloat) 0.000, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) -0.905, (GLfloat) 0.309, (GLfloat) -0.294, (GLfloat)  -0.951, (GLfloat) 0.000, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) -0.951, (GLfloat) 0.309, (GLfloat) 0.000, (GLfloat)  -1.000, (GLfloat) 0.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.905, (GLfloat) 0.309, (GLfloat) 0.294, (GLfloat)  -0.951, (GLfloat) 0.000, (GLfloat) 0.309, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) 0.309, (GLfloat) 0.559, (GLfloat)  -0.809, (GLfloat) 0.000, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) 0.309, (GLfloat) 0.769, (GLfloat)  -0.588, (GLfloat) 0.000, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) 0.309, (GLfloat) 0.905, (GLfloat)  -0.309, (GLfloat) 0.000, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.309, (GLfloat) 0.951, (GLfloat)  0.000, (GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) 0.309, (GLfloat) 0.905, (GLfloat)  0.309, (GLfloat) 0.000, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) 0.309, (GLfloat) 0.769, (GLfloat)  0.588, (GLfloat) 0.000, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) 0.309, (GLfloat) 0.559, (GLfloat)  0.809, (GLfloat) 0.000, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) 0.905, (GLfloat) 0.309, (GLfloat) 0.294, (GLfloat)  0.951, (GLfloat) 0.000, (GLfloat) 0.309, (GLfloat) 
		(GLfloat) 0.951, (GLfloat) 0.309, (GLfloat) -0.000, (GLfloat)  1.000, (GLfloat) 0.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.905, (GLfloat) 0.309, (GLfloat) -0.294, (GLfloat)  0.951, (GLfloat) 0.000, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) 0.309, (GLfloat) -0.559, (GLfloat)  0.809, (GLfloat) 0.000, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) 0.309, (GLfloat) -0.769, (GLfloat)  0.588, (GLfloat) 0.000, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) 0.309, (GLfloat) -0.905, (GLfloat)  0.309, (GLfloat) 0.000, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.309, (GLfloat) -0.951, (GLfloat)  -0.000, (GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.588, (GLfloat) -0.809, (GLfloat)  0.000, (GLfloat) 0.309, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) 0.588, (GLfloat) -0.769, (GLfloat)  -0.294, (GLfloat) 0.309, (GLfloat) -0.905, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) 0.588, (GLfloat) -0.655, (GLfloat)  -0.559, (GLfloat) 0.309, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.655, (GLfloat) 0.588, (GLfloat) -0.476, (GLfloat)  -0.769, (GLfloat) 0.309, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) 0.588, (GLfloat) -0.250, (GLfloat)  -0.905, (GLfloat) 0.309, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) -0.809, (GLfloat) 0.588, (GLfloat) 0.000, (GLfloat)  -0.951, (GLfloat) 0.309, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) 0.588, (GLfloat) 0.250, (GLfloat)  -0.905, (GLfloat) 0.309, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) -0.655, (GLfloat) 0.588, (GLfloat) 0.476, (GLfloat)  -0.769, (GLfloat) 0.309, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) 0.588, (GLfloat) 0.655, (GLfloat)  -0.559, (GLfloat) 0.309, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) 0.588, (GLfloat) 0.769, (GLfloat)  -0.294, (GLfloat) 0.309, (GLfloat) 0.905, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.588, (GLfloat) 0.809, (GLfloat)  0.000, (GLfloat) 0.309, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.588, (GLfloat) 0.769, (GLfloat)  0.294, (GLfloat) 0.309, (GLfloat) 0.905, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) 0.588, (GLfloat) 0.655, (GLfloat)  0.559, (GLfloat) 0.309, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.655, (GLfloat) 0.588, (GLfloat) 0.476, (GLfloat)  0.769, (GLfloat) 0.309, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) 0.588, (GLfloat) 0.250, (GLfloat)  0.905, (GLfloat) 0.309, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) 0.809, (GLfloat) 0.588, (GLfloat) -0.000, (GLfloat)  0.951, (GLfloat) 0.309, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) 0.588, (GLfloat) -0.250, (GLfloat)  0.905, (GLfloat) 0.309, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) 0.655, (GLfloat) 0.588, (GLfloat) -0.476, (GLfloat)  0.769, (GLfloat) 0.309, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) 0.588, (GLfloat) -0.655, (GLfloat)  0.559, (GLfloat) 0.309, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.588, (GLfloat) -0.769, (GLfloat)  0.294, (GLfloat) 0.309, (GLfloat) -0.905, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.588, (GLfloat) -0.809, (GLfloat)  -0.000, (GLfloat) 0.309, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.809, (GLfloat) -0.588, (GLfloat)  0.000, (GLfloat) 0.588, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) 0.809, (GLfloat) -0.559, (GLfloat)  -0.250, (GLfloat) 0.588, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.345, (GLfloat) 0.809, (GLfloat) -0.476, (GLfloat)  -0.476, (GLfloat) 0.588, (GLfloat) -0.655, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) 0.809, (GLfloat) -0.345, (GLfloat)  -0.655, (GLfloat) 0.588, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) 0.809, (GLfloat) -0.182, (GLfloat)  -0.769, (GLfloat) 0.588, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) -0.588, (GLfloat) 0.809, (GLfloat) 0.000, (GLfloat)  -0.809, (GLfloat) 0.588, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) 0.809, (GLfloat) 0.182, (GLfloat)  -0.769, (GLfloat) 0.588, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) 0.809, (GLfloat) 0.345, (GLfloat)  -0.655, (GLfloat) 0.588, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) -0.345, (GLfloat) 0.809, (GLfloat) 0.476, (GLfloat)  -0.476, (GLfloat) 0.588, (GLfloat) 0.655, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) 0.809, (GLfloat) 0.559, (GLfloat)  -0.250, (GLfloat) 0.588, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.809, (GLfloat) 0.588, (GLfloat)  0.000, (GLfloat) 0.588, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) 0.809, (GLfloat) 0.559, (GLfloat)  0.250, (GLfloat) 0.588, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.345, (GLfloat) 0.809, (GLfloat) 0.476, (GLfloat)  0.476, (GLfloat) 0.588, (GLfloat) 0.655, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) 0.809, (GLfloat) 0.345, (GLfloat)  0.655, (GLfloat) 0.588, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) 0.809, (GLfloat) 0.182, (GLfloat)  0.769, (GLfloat) 0.588, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) 0.588, (GLfloat) 0.809, (GLfloat) -0.000, (GLfloat)  0.809, (GLfloat) 0.588, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) 0.809, (GLfloat) -0.182, (GLfloat)  0.769, (GLfloat) 0.588, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) 0.809, (GLfloat) -0.345, (GLfloat)  0.655, (GLfloat) 0.588, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) 0.345, (GLfloat) 0.809, (GLfloat) -0.476, (GLfloat)  0.476, (GLfloat) 0.588, (GLfloat) -0.655, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) 0.809, (GLfloat) -0.559, (GLfloat)  0.250, (GLfloat) 0.588, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.809, (GLfloat) -0.588, (GLfloat)  -0.000, (GLfloat) 0.588, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.951, (GLfloat) -0.309, (GLfloat)  0.000, (GLfloat) 0.809, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) -0.095, (GLfloat) 0.951, (GLfloat) -0.294, (GLfloat)  -0.182, (GLfloat) 0.809, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) 0.951, (GLfloat) -0.250, (GLfloat)  -0.345, (GLfloat) 0.809, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) 0.951, (GLfloat) -0.182, (GLfloat)  -0.476, (GLfloat) 0.809, (GLfloat) -0.345, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) 0.951, (GLfloat) -0.095, (GLfloat)  -0.559, (GLfloat) 0.809, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) -0.309, (GLfloat) 0.951, (GLfloat) 0.000, (GLfloat)  -0.588, (GLfloat) 0.809, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) 0.951, (GLfloat) 0.095, (GLfloat)  -0.559, (GLfloat) 0.809, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) 0.951, (GLfloat) 0.182, (GLfloat)  -0.476, (GLfloat) 0.809, (GLfloat) 0.345, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) 0.951, (GLfloat) 0.250, (GLfloat)  -0.345, (GLfloat) 0.809, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) -0.095, (GLfloat) 0.951, (GLfloat) 0.294, (GLfloat)  -0.182, (GLfloat) 0.809, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.951, (GLfloat) 0.309, (GLfloat)  0.000, (GLfloat) 0.809, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) 0.095, (GLfloat) 0.951, (GLfloat) 0.294, (GLfloat)  0.182, (GLfloat) 0.809, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) 0.951, (GLfloat) 0.250, (GLfloat)  0.345, (GLfloat) 0.809, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.951, (GLfloat) 0.182, (GLfloat)  0.476, (GLfloat) 0.809, (GLfloat) 0.345, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) 0.951, (GLfloat) 0.095, (GLfloat)  0.559, (GLfloat) 0.809, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) 0.309, (GLfloat) 0.951, (GLfloat) -0.000, (GLfloat)  0.588, (GLfloat) 0.809, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) 0.951, (GLfloat) -0.095, (GLfloat)  0.559, (GLfloat) 0.809, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.951, (GLfloat) -0.182, (GLfloat)  0.476, (GLfloat) 0.809, (GLfloat) -0.345, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) 0.951, (GLfloat) -0.250, (GLfloat)  0.345, (GLfloat) 0.809, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) 0.095, (GLfloat) 0.951, (GLfloat) -0.294, (GLfloat)  0.182, (GLfloat) 0.809, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.951, (GLfloat) -0.309, (GLfloat)  -0.000, (GLfloat) 0.809, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  0.000, (GLfloat) 0.951, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  -0.095, (GLfloat) 0.951, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  -0.182, (GLfloat) 0.951, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  -0.250, (GLfloat) 0.951, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  -0.294, (GLfloat) 0.951, (GLfloat) -0.095, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  -0.309, (GLfloat) 0.951, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  -0.294, (GLfloat) 0.951, (GLfloat) 0.095, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  -0.250, (GLfloat) 0.951, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  -0.182, (GLfloat) 0.951, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  -0.095, (GLfloat) 0.951, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  0.000, (GLfloat) 0.951, (GLfloat) 0.309, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  0.095, (GLfloat) 0.951, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  0.182, (GLfloat) 0.951, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  0.250, (GLfloat) 0.951, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat)  0.294, (GLfloat) 0.951, (GLfloat) 0.095, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  0.309, (GLfloat) 0.951, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  0.294, (GLfloat) 0.951, (GLfloat) -0.095, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  0.250, (GLfloat) 0.951, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  0.182, (GLfloat) 0.951, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  0.095, (GLfloat) 0.951, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat)  -0.000, (GLfloat) 0.951, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.951, (GLfloat) 0.309, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.095, (GLfloat) 0.951, (GLfloat) 0.294, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) 0.951, (GLfloat) 0.250, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.951, (GLfloat) 0.182, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) 0.951, (GLfloat) 0.095, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.309, (GLfloat) 0.951, (GLfloat) -0.000, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) 0.951, (GLfloat) -0.095, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.951, (GLfloat) -0.182, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) 0.951, (GLfloat) -0.250, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.095, (GLfloat) 0.951, (GLfloat) -0.294, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.951, (GLfloat) -0.309, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.095, (GLfloat) 0.951, (GLfloat) -0.294, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) 0.951, (GLfloat) -0.250, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) 0.951, (GLfloat) -0.182, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) 0.951, (GLfloat) -0.095, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.309, (GLfloat) 0.951, (GLfloat) 0.000, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) 0.951, (GLfloat) 0.095, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) 0.951, (GLfloat) 0.182, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) 0.951, (GLfloat) 0.250, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.095, (GLfloat) 0.951, (GLfloat) 0.294, (GLfloat)  -0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.951, (GLfloat) 0.309, (GLfloat)  0.000, (GLfloat) 1.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.809, (GLfloat) 0.588, (GLfloat)  -0.000, (GLfloat) 0.951, (GLfloat) 0.309, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) 0.809, (GLfloat) 0.559, (GLfloat)  0.095, (GLfloat) 0.951, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) 0.345, (GLfloat) 0.809, (GLfloat) 0.476, (GLfloat)  0.182, (GLfloat) 0.951, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) 0.809, (GLfloat) 0.345, (GLfloat)  0.250, (GLfloat) 0.951, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) 0.809, (GLfloat) 0.182, (GLfloat)  0.294, (GLfloat) 0.951, (GLfloat) 0.095, (GLfloat) 
		(GLfloat) 0.588, (GLfloat) 0.809, (GLfloat) -0.000, (GLfloat)  0.309, (GLfloat) 0.951, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) 0.809, (GLfloat) -0.182, (GLfloat)  0.294, (GLfloat) 0.951, (GLfloat) -0.095, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) 0.809, (GLfloat) -0.345, (GLfloat)  0.250, (GLfloat) 0.951, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) 0.345, (GLfloat) 0.809, (GLfloat) -0.476, (GLfloat)  0.182, (GLfloat) 0.951, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) 0.809, (GLfloat) -0.559, (GLfloat)  0.095, (GLfloat) 0.951, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.809, (GLfloat) -0.588, (GLfloat)  -0.000, (GLfloat) 0.951, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) 0.809, (GLfloat) -0.559, (GLfloat)  -0.095, (GLfloat) 0.951, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) -0.345, (GLfloat) 0.809, (GLfloat) -0.476, (GLfloat)  -0.182, (GLfloat) 0.951, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) 0.809, (GLfloat) -0.345, (GLfloat)  -0.250, (GLfloat) 0.951, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) 0.809, (GLfloat) -0.182, (GLfloat)  -0.294, (GLfloat) 0.951, (GLfloat) -0.095, (GLfloat) 
		(GLfloat) -0.588, (GLfloat) 0.809, (GLfloat) 0.000, (GLfloat)  -0.309, (GLfloat) 0.951, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) 0.809, (GLfloat) 0.182, (GLfloat)  -0.294, (GLfloat) 0.951, (GLfloat) 0.095, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) 0.809, (GLfloat) 0.345, (GLfloat)  -0.250, (GLfloat) 0.951, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) -0.345, (GLfloat) 0.809, (GLfloat) 0.476, (GLfloat)  -0.182, (GLfloat) 0.951, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) 0.809, (GLfloat) 0.559, (GLfloat)  -0.095, (GLfloat) 0.951, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.809, (GLfloat) 0.588, (GLfloat)  0.000, (GLfloat) 0.951, (GLfloat) 0.309, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.588, (GLfloat) 0.809, (GLfloat)  -0.000, (GLfloat) 0.809, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.588, (GLfloat) 0.769, (GLfloat)  0.182, (GLfloat) 0.809, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) 0.588, (GLfloat) 0.655, (GLfloat)  0.345, (GLfloat) 0.809, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) 0.655, (GLfloat) 0.588, (GLfloat) 0.476, (GLfloat)  0.476, (GLfloat) 0.809, (GLfloat) 0.345, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) 0.588, (GLfloat) 0.250, (GLfloat)  0.559, (GLfloat) 0.809, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) 0.809, (GLfloat) 0.588, (GLfloat) -0.000, (GLfloat)  0.588, (GLfloat) 0.809, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) 0.588, (GLfloat) -0.250, (GLfloat)  0.559, (GLfloat) 0.809, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) 0.655, (GLfloat) 0.588, (GLfloat) -0.476, (GLfloat)  0.476, (GLfloat) 0.809, (GLfloat) -0.345, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) 0.588, (GLfloat) -0.655, (GLfloat)  0.345, (GLfloat) 0.809, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) 0.588, (GLfloat) -0.769, (GLfloat)  0.182, (GLfloat) 0.809, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.588, (GLfloat) -0.809, (GLfloat)  -0.000, (GLfloat) 0.809, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) 0.588, (GLfloat) -0.769, (GLfloat)  -0.182, (GLfloat) 0.809, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) 0.588, (GLfloat) -0.655, (GLfloat)  -0.345, (GLfloat) 0.809, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) -0.655, (GLfloat) 0.588, (GLfloat) -0.476, (GLfloat)  -0.476, (GLfloat) 0.809, (GLfloat) -0.345, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) 0.588, (GLfloat) -0.250, (GLfloat)  -0.559, (GLfloat) 0.809, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) -0.809, (GLfloat) 0.588, (GLfloat) 0.000, (GLfloat)  -0.588, (GLfloat) 0.809, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) 0.588, (GLfloat) 0.250, (GLfloat)  -0.559, (GLfloat) 0.809, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) -0.655, (GLfloat) 0.588, (GLfloat) 0.476, (GLfloat)  -0.476, (GLfloat) 0.809, (GLfloat) 0.345, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) 0.588, (GLfloat) 0.655, (GLfloat)  -0.345, (GLfloat) 0.809, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) 0.588, (GLfloat) 0.769, (GLfloat)  -0.182, (GLfloat) 0.809, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.588, (GLfloat) 0.809, (GLfloat)  0.000, (GLfloat) 0.809, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.309, (GLfloat) 0.951, (GLfloat)  -0.000, (GLfloat) 0.588, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) 0.309, (GLfloat) 0.905, (GLfloat)  0.250, (GLfloat) 0.588, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) 0.309, (GLfloat) 0.769, (GLfloat)  0.476, (GLfloat) 0.588, (GLfloat) 0.655, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) 0.309, (GLfloat) 0.559, (GLfloat)  0.655, (GLfloat) 0.588, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) 0.905, (GLfloat) 0.309, (GLfloat) 0.294, (GLfloat)  0.769, (GLfloat) 0.588, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) 0.951, (GLfloat) 0.309, (GLfloat) -0.000, (GLfloat)  0.809, (GLfloat) 0.588, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.905, (GLfloat) 0.309, (GLfloat) -0.294, (GLfloat)  0.769, (GLfloat) 0.588, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) 0.309, (GLfloat) -0.559, (GLfloat)  0.655, (GLfloat) 0.588, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) 0.309, (GLfloat) -0.769, (GLfloat)  0.476, (GLfloat) 0.588, (GLfloat) -0.655, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) 0.309, (GLfloat) -0.905, (GLfloat)  0.250, (GLfloat) 0.588, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) 0.309, (GLfloat) -0.951, (GLfloat)  -0.000, (GLfloat) 0.588, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) 0.309, (GLfloat) -0.905, (GLfloat)  -0.250, (GLfloat) 0.588, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) 0.309, (GLfloat) -0.769, (GLfloat)  -0.476, (GLfloat) 0.588, (GLfloat) -0.655, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) 0.309, (GLfloat) -0.559, (GLfloat)  -0.655, (GLfloat) 0.588, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) -0.905, (GLfloat) 0.309, (GLfloat) -0.294, (GLfloat)  -0.769, (GLfloat) 0.588, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) -0.951, (GLfloat) 0.309, (GLfloat) 0.000, (GLfloat)  -0.809, (GLfloat) 0.588, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.905, (GLfloat) 0.309, (GLfloat) 0.294, (GLfloat)  -0.769, (GLfloat) 0.588, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) 0.309, (GLfloat) 0.559, (GLfloat)  -0.655, (GLfloat) 0.588, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) 0.309, (GLfloat) 0.769, (GLfloat)  -0.476, (GLfloat) 0.588, (GLfloat) 0.655, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) 0.309, (GLfloat) 0.905, (GLfloat)  -0.250, (GLfloat) 0.588, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) 0.309, (GLfloat) 0.951, (GLfloat)  0.000, (GLfloat) 0.588, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.000, (GLfloat) 1.000, (GLfloat)  -0.000, (GLfloat) 0.309, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) 0.309, (GLfloat) -0.000, (GLfloat) 0.951, (GLfloat)  0.294, (GLfloat) 0.309, (GLfloat) 0.905, (GLfloat) 
		(GLfloat) 0.588, (GLfloat) -0.000, (GLfloat) 0.809, (GLfloat)  0.559, (GLfloat) 0.309, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.809, (GLfloat) -0.000, (GLfloat) 0.588, (GLfloat)  0.769, (GLfloat) 0.309, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.951, (GLfloat) -0.000, (GLfloat) 0.309, (GLfloat)  0.905, (GLfloat) 0.309, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) 1.000, (GLfloat) -0.000, (GLfloat) -0.000, (GLfloat)  0.951, (GLfloat) 0.309, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.951, (GLfloat) -0.000, (GLfloat) -0.309, (GLfloat)  0.905, (GLfloat) 0.309, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) 0.809, (GLfloat) -0.000, (GLfloat) -0.588, (GLfloat)  0.769, (GLfloat) 0.309, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) 0.588, (GLfloat) -0.000, (GLfloat) -0.809, (GLfloat)  0.559, (GLfloat) 0.309, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) 0.309, (GLfloat) -0.000, (GLfloat) -0.951, (GLfloat)  0.294, (GLfloat) 0.309, (GLfloat) -0.905, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.000, (GLfloat) -1.000, (GLfloat)  -0.000, (GLfloat) 0.309, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) -0.309, (GLfloat) -0.000, (GLfloat) -0.951, (GLfloat)  -0.294, (GLfloat) 0.309, (GLfloat) -0.905, (GLfloat) 
		(GLfloat) -0.588, (GLfloat) -0.000, (GLfloat) -0.809, (GLfloat)  -0.559, (GLfloat) 0.309, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.809, (GLfloat) -0.000, (GLfloat) -0.588, (GLfloat)  -0.769, (GLfloat) 0.309, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.951, (GLfloat) -0.000, (GLfloat) -0.309, (GLfloat)  -0.905, (GLfloat) 0.309, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) -1.000, (GLfloat) -0.000, (GLfloat) 0.000, (GLfloat)  -0.951, (GLfloat) 0.309, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.951, (GLfloat) -0.000, (GLfloat) 0.309, (GLfloat)  -0.905, (GLfloat) 0.309, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) -0.809, (GLfloat) -0.000, (GLfloat) 0.588, (GLfloat)  -0.769, (GLfloat) 0.309, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) -0.588, (GLfloat) -0.000, (GLfloat) 0.809, (GLfloat)  -0.559, (GLfloat) 0.309, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) -0.309, (GLfloat) -0.000, (GLfloat) 0.951, (GLfloat)  -0.294, (GLfloat) 0.309, (GLfloat) 0.905, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.000, (GLfloat) 1.000, (GLfloat)  0.000, (GLfloat) 0.309, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.309, (GLfloat) 0.951, (GLfloat)  -0.000, (GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) -0.309, (GLfloat) 0.905, (GLfloat)  0.309, (GLfloat) -0.000, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) -0.309, (GLfloat) 0.769, (GLfloat)  0.588, (GLfloat) -0.000, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) -0.309, (GLfloat) 0.559, (GLfloat)  0.809, (GLfloat) -0.000, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) 0.905, (GLfloat) -0.309, (GLfloat) 0.294, (GLfloat)  0.951, (GLfloat) -0.000, (GLfloat) 0.309, (GLfloat) 
		(GLfloat) 0.951, (GLfloat) -0.309, (GLfloat) -0.000, (GLfloat)  1.000, (GLfloat) -0.000, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.905, (GLfloat) -0.309, (GLfloat) -0.294, (GLfloat)  0.951, (GLfloat) -0.000, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) -0.309, (GLfloat) -0.559, (GLfloat)  0.809, (GLfloat) -0.000, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) -0.309, (GLfloat) -0.769, (GLfloat)  0.588, (GLfloat) -0.000, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) -0.309, (GLfloat) -0.905, (GLfloat)  0.309, (GLfloat) -0.000, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.309, (GLfloat) -0.951, (GLfloat)  -0.000, (GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) -0.309, (GLfloat) -0.905, (GLfloat)  -0.309, (GLfloat) -0.000, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) -0.309, (GLfloat) -0.769, (GLfloat)  -0.588, (GLfloat) -0.000, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) -0.309, (GLfloat) -0.559, (GLfloat)  -0.809, (GLfloat) -0.000, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) -0.905, (GLfloat) -0.309, (GLfloat) -0.294, (GLfloat)  -0.951, (GLfloat) -0.000, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) -0.951, (GLfloat) -0.309, (GLfloat) 0.000, (GLfloat)  -1.000, (GLfloat) -0.000, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.905, (GLfloat) -0.309, (GLfloat) 0.294, (GLfloat)  -0.951, (GLfloat) -0.000, (GLfloat) 0.309, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) -0.309, (GLfloat) 0.559, (GLfloat)  -0.809, (GLfloat) -0.000, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) -0.309, (GLfloat) 0.769, (GLfloat)  -0.588, (GLfloat) -0.000, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) -0.309, (GLfloat) 0.905, (GLfloat)  -0.309, (GLfloat) -0.000, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.309, (GLfloat) 0.951, (GLfloat)  0.000, (GLfloat) -0.000, (GLfloat) 1.000, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.588, (GLfloat) 0.809, (GLfloat)  -0.000, (GLfloat) -0.309, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) -0.588, (GLfloat) 0.769, (GLfloat)  0.294, (GLfloat) -0.309, (GLfloat) 0.905, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) -0.588, (GLfloat) 0.655, (GLfloat)  0.559, (GLfloat) -0.309, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.655, (GLfloat) -0.588, (GLfloat) 0.476, (GLfloat)  0.769, (GLfloat) -0.309, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) -0.588, (GLfloat) 0.250, (GLfloat)  0.905, (GLfloat) -0.309, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) 0.809, (GLfloat) -0.588, (GLfloat) -0.000, (GLfloat)  0.951, (GLfloat) -0.309, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.769, (GLfloat) -0.588, (GLfloat) -0.250, (GLfloat)  0.905, (GLfloat) -0.309, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) 0.655, (GLfloat) -0.588, (GLfloat) -0.476, (GLfloat)  0.769, (GLfloat) -0.309, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) -0.588, (GLfloat) -0.655, (GLfloat)  0.559, (GLfloat) -0.309, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) -0.588, (GLfloat) -0.769, (GLfloat)  0.294, (GLfloat) -0.309, (GLfloat) -0.905, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.588, (GLfloat) -0.809, (GLfloat)  -0.000, (GLfloat) -0.309, (GLfloat) -0.951, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) -0.588, (GLfloat) -0.769, (GLfloat)  -0.294, (GLfloat) -0.309, (GLfloat) -0.905, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) -0.588, (GLfloat) -0.655, (GLfloat)  -0.559, (GLfloat) -0.309, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.655, (GLfloat) -0.588, (GLfloat) -0.476, (GLfloat)  -0.769, (GLfloat) -0.309, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) -0.588, (GLfloat) -0.250, (GLfloat)  -0.905, (GLfloat) -0.309, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) -0.809, (GLfloat) -0.588, (GLfloat) 0.000, (GLfloat)  -0.951, (GLfloat) -0.309, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.769, (GLfloat) -0.588, (GLfloat) 0.250, (GLfloat)  -0.905, (GLfloat) -0.309, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) -0.655, (GLfloat) -0.588, (GLfloat) 0.476, (GLfloat)  -0.769, (GLfloat) -0.309, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) -0.588, (GLfloat) 0.655, (GLfloat)  -0.559, (GLfloat) -0.309, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) -0.588, (GLfloat) 0.769, (GLfloat)  -0.294, (GLfloat) -0.309, (GLfloat) 0.905, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.588, (GLfloat) 0.809, (GLfloat)  0.000, (GLfloat) -0.309, (GLfloat) 0.951, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.809, (GLfloat) 0.588, (GLfloat)  -0.000, (GLfloat) -0.588, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) -0.809, (GLfloat) 0.559, (GLfloat)  0.250, (GLfloat) -0.588, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.345, (GLfloat) -0.809, (GLfloat) 0.476, (GLfloat)  0.476, (GLfloat) -0.588, (GLfloat) 0.655, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) -0.809, (GLfloat) 0.345, (GLfloat)  0.655, (GLfloat) -0.588, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) -0.809, (GLfloat) 0.182, (GLfloat)  0.769, (GLfloat) -0.588, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) 0.588, (GLfloat) -0.809, (GLfloat) -0.000, (GLfloat)  0.809, (GLfloat) -0.588, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.559, (GLfloat) -0.809, (GLfloat) -0.182, (GLfloat)  0.769, (GLfloat) -0.588, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) 0.476, (GLfloat) -0.809, (GLfloat) -0.345, (GLfloat)  0.655, (GLfloat) -0.588, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) 0.345, (GLfloat) -0.809, (GLfloat) -0.476, (GLfloat)  0.476, (GLfloat) -0.588, (GLfloat) -0.655, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) -0.809, (GLfloat) -0.559, (GLfloat)  0.250, (GLfloat) -0.588, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.809, (GLfloat) -0.588, (GLfloat)  -0.000, (GLfloat) -0.588, (GLfloat) -0.809, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) -0.809, (GLfloat) -0.559, (GLfloat)  -0.250, (GLfloat) -0.588, (GLfloat) -0.769, (GLfloat) 
		(GLfloat) -0.345, (GLfloat) -0.809, (GLfloat) -0.476, (GLfloat)  -0.476, (GLfloat) -0.588, (GLfloat) -0.655, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) -0.809, (GLfloat) -0.345, (GLfloat)  -0.655, (GLfloat) -0.588, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) -0.809, (GLfloat) -0.182, (GLfloat)  -0.769, (GLfloat) -0.588, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) -0.588, (GLfloat) -0.809, (GLfloat) 0.000, (GLfloat)  -0.809, (GLfloat) -0.588, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.559, (GLfloat) -0.809, (GLfloat) 0.182, (GLfloat)  -0.769, (GLfloat) -0.588, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) -0.476, (GLfloat) -0.809, (GLfloat) 0.345, (GLfloat)  -0.655, (GLfloat) -0.588, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) -0.345, (GLfloat) -0.809, (GLfloat) 0.476, (GLfloat)  -0.476, (GLfloat) -0.588, (GLfloat) 0.655, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) -0.809, (GLfloat) 0.559, (GLfloat)  -0.250, (GLfloat) -0.588, (GLfloat) 0.769, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.809, (GLfloat) 0.588, (GLfloat)  0.000, (GLfloat) -0.588, (GLfloat) 0.809, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.951, (GLfloat) 0.309, (GLfloat)  -0.000, (GLfloat) -0.809, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) 0.095, (GLfloat) -0.951, (GLfloat) 0.294, (GLfloat)  0.182, (GLfloat) -0.809, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) -0.951, (GLfloat) 0.250, (GLfloat)  0.345, (GLfloat) -0.809, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) -0.951, (GLfloat) 0.182, (GLfloat)  0.476, (GLfloat) -0.809, (GLfloat) 0.345, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) -0.951, (GLfloat) 0.095, (GLfloat)  0.559, (GLfloat) -0.809, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) 0.309, (GLfloat) -0.951, (GLfloat) -0.000, (GLfloat)  0.588, (GLfloat) -0.809, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) 0.294, (GLfloat) -0.951, (GLfloat) -0.095, (GLfloat)  0.559, (GLfloat) -0.809, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) 0.250, (GLfloat) -0.951, (GLfloat) -0.182, (GLfloat)  0.476, (GLfloat) -0.809, (GLfloat) -0.345, (GLfloat) 
		(GLfloat) 0.182, (GLfloat) -0.951, (GLfloat) -0.250, (GLfloat)  0.345, (GLfloat) -0.809, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) 0.095, (GLfloat) -0.951, (GLfloat) -0.294, (GLfloat)  0.182, (GLfloat) -0.809, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -0.951, (GLfloat) -0.309, (GLfloat)  -0.000, (GLfloat) -0.809, (GLfloat) -0.588, (GLfloat) 
		(GLfloat) -0.095, (GLfloat) -0.951, (GLfloat) -0.294, (GLfloat)  -0.182, (GLfloat) -0.809, (GLfloat) -0.559, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) -0.951, (GLfloat) -0.250, (GLfloat)  -0.345, (GLfloat) -0.809, (GLfloat) -0.476, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) -0.951, (GLfloat) -0.182, (GLfloat)  -0.476, (GLfloat) -0.809, (GLfloat) -0.345, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) -0.951, (GLfloat) -0.095, (GLfloat)  -0.559, (GLfloat) -0.809, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) -0.309, (GLfloat) -0.951, (GLfloat) 0.000, (GLfloat)  -0.588, (GLfloat) -0.809, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) -0.294, (GLfloat) -0.951, (GLfloat) 0.095, (GLfloat)  -0.559, (GLfloat) -0.809, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) -0.250, (GLfloat) -0.951, (GLfloat) 0.182, (GLfloat)  -0.476, (GLfloat) -0.809, (GLfloat) 0.345, (GLfloat) 
		(GLfloat) -0.182, (GLfloat) -0.951, (GLfloat) 0.250, (GLfloat)  -0.345, (GLfloat) -0.809, (GLfloat) 0.476, (GLfloat) 
		(GLfloat) -0.095, (GLfloat) -0.951, (GLfloat) 0.294, (GLfloat)  -0.182, (GLfloat) -0.809, (GLfloat) 0.559, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -0.951, (GLfloat) 0.309, (GLfloat)  0.000, (GLfloat) -0.809, (GLfloat) 0.588, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  -0.000, (GLfloat) -0.951, (GLfloat) 0.309, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  0.095, (GLfloat) -0.951, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  0.182, (GLfloat) -0.951, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  0.250, (GLfloat) -0.951, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  0.294, (GLfloat) -0.951, (GLfloat) 0.095, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  0.309, (GLfloat) -0.951, (GLfloat) -0.000, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  0.294, (GLfloat) -0.951, (GLfloat) -0.095, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  0.250, (GLfloat) -0.951, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  0.182, (GLfloat) -0.951, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  0.095, (GLfloat) -0.951, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  -0.000, (GLfloat) -0.951, (GLfloat) -0.309, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  -0.095, (GLfloat) -0.951, (GLfloat) -0.294, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  -0.182, (GLfloat) -0.951, (GLfloat) -0.250, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  -0.250, (GLfloat) -0.951, (GLfloat) -0.182, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) 0.000, (GLfloat)  -0.294, (GLfloat) -0.951, (GLfloat) -0.095, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  -0.309, (GLfloat) -0.951, (GLfloat) 0.000, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  -0.294, (GLfloat) -0.951, (GLfloat) 0.095, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  -0.250, (GLfloat) -0.951, (GLfloat) 0.182, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  -0.182, (GLfloat) -0.951, (GLfloat) 0.250, (GLfloat) 
		(GLfloat) 0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  -0.095, (GLfloat) -0.951, (GLfloat) 0.294, (GLfloat) 
		(GLfloat) -0.000, (GLfloat) -1.000, (GLfloat) -0.000, (GLfloat)  0.000, (GLfloat) -0.951, (GLfloat) 0.309
};
