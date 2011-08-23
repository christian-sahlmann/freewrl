/*
=INSERT_TEMPLATE_HERE=

$Id: CollisionGPU.c,v 1.5 2011/08/23 15:24:40 crc_canada Exp $

Render the children of nodes.

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

#include "Viewer.h"
#include "RenderFuncs.h"
#include "../vrml_parser/Structs.h"

#include "../main/headers.h"

#include "LinearAlgebra.h"
#include "Collision.h"

#ifdef DO_COLLISION_GPU


#include <OpenGL/CGLDevice.h>
#include <OpenCL/opencl.h>

#define DOUGS_FLOAT_TOLERANCE 0.00000001
#define FLOAT_TOLERANCE 0.000001


int printed=FALSE;
int meprinted=FALSE;

/********************************************************************************/
/*										*/
/* 	Collide kernel, generic structures, etc					*/
/*										*/
/********************************************************************************/

struct OpenCLTransform
{
        cl_float16      matrix;
};

/********************************************************************************/
/*										*/
/*	Collide kernel. Step 1 - transform vertices and calculate normal	*/
/*										*/
/********************************************************************************/

/* start the collide process.

1) transform the vertex.
2) calculate normal
3) if triangle is visible to us, get ready for collide calcs

TODO: 	ccw flag should be passed in.
TODO:	flags for ccw tri, cw tri or two facing tris should be passed in
TODO:	pre-compile kernels.
TODO:	make kernel invocation dependent (using dougs' global structures)
TODO:	get and drop OpenGL vertices every call, in case they change?

*/

/********************************************************************************/
/*										*/
/* Collide Kernel Step 2: do hit calculations					*/
/*										*/
/********************************************************************************/

/* here we do the equivalent of get_poly_disp_2 for each mode */
/* struct point_XYZ get_poly_disp_2(struct point_XYZ* p, int num, struct point_XYZ n) { */
/* but, for non-walking, we have: 
                // fly, examine 
                result = get_poly_min_disp_with_sphere(awidth, p, num, n);
        }
        pp->get_poly_mindisp = vecdot(&result,&result);
        return result;

and,

struct point_XYZ get_poly_min_disp_with_sphere(double r, struct point_XYZ* p, int num, struct point_XYZ n)

where 	r == avatar radius,
	p == our 3 vectors,
	num== 3?? (3 vectors)
	n = normal
*/

/*
TODO: 	code for non-walk mode;
TODO:	code for walk-mode;
*/

#ifdef DougsCode

struct point_XYZ get_poly_min_disp_with_sphere(double r, struct point_XYZ* p, int num, struct point_XYZ n) {
    int i,j;
    /* double polydisp; */
    struct point_XYZ result;
	double tmin[3],tmax[3],rmin[3],rmax[3],q[3];
    double get_poly_mindisp;
    int clippedPoly4num = 0;
	ppcollision pp = (ppcollision)gglobal()->collision.prv;
    get_poly_mindisp = 1E90;

	/* cheap MBB test */
...
	/* end cheap MBB test */

#ifdef DEBUGFACEMASK
    if(facemask != debugsurface++)
	return zero;
#endif
    /*keep if inside*/
    if(perpendicular_line_passing_inside_poly(pp->clippedPoly4[clippedPoly4num],p, num)) {
		DEBUGPTSPRINT("perpendicular_line_passing_inside_poly[%d]= %d\n",0,clippedPoly4num);
		clippedPoly4num++;
	}


#ifdef DEBUGPTS
    for(i=0; i < clippedPoly4num; i++) {
	debugpts.push_back(clippedPoly4[i]);
    }
#endif

    /*here we find mimimum displacement possible */

    /*calculate the closest point to origin */
    for(i = 0; i < clippedPoly4num; i++) 
	{
		printf ("\t\tget_poly_min_disp_with_sphere, checking against %d %f %f %f",i,pp->clippedPoly4[i].x, 
		pp->clippedPoly4[i].y,pp->clippedPoly4[i].z);

		double disp = vecdot(&pp->clippedPoly4[i],&pp->clippedPoly4[i]);

		printf ("\t\tdisp %lf, get_poly_mindisp %lf\n",disp,get_poly_mindisp); 

		if(disp < get_poly_mindisp) 
		{
			get_poly_mindisp = disp;
			result = pp->clippedPoly4[i];
		}
    }
    if(get_poly_mindisp <= r*r) 
	{
		/*  scale result to length of missing distance. */
		double rl;
		rl = veclength(result);
		/* printf ("get_poly_min_disp_with_sphere, comparing %f and %f veclen %lf result %f %f %f\n",get_poly_mindisp, r*r, rl, result.x,result.y,result.z); */
		/* if(rl != 0.) */
		if(! APPROX(rl, 0)) 
		{
			/* printf ("approx rl, 0... scaling by %lf, %lf - %lf / %lf\n",(r-sqrt(get_poly_mindisp)) / rl,
				r, sqrt(get_poly_mindisp), rl); */
			vecscale(&result,&result,(r-sqrt(get_poly_mindisp)) / rl);
		} 
		else
		{
			result = zero;
		}
    }
    else
	{
		result = zero;
	}
    return result;
}

#endif //DougsCode


/********************************************************************************/
/*										*/
/*	Collide kernel step 3 - return vector for this triangle			*/
/*										*/
/********************************************************************************/

/* finish and return results */
/* TODO - return displacement */

/********************************************************************************/
/*										*/
/*										*/
/********************************************************************************/

cl_program program = NULL;
cl_kernel kernel = NULL;
cl_context context = NULL;
cl_command_queue queue = NULL;
cl_device_id device_id;
int output_size;
cl_mem output_buffer = NULL;
cl_mem matrix_buffer = NULL;
cl_mem vertex_buffer = NULL;
cl_mem index_buffer = NULL;


struct Multi_Vec3f collide_rvs = {0,NULL};
struct Multi_Int32 cindicies = {0,NULL};


/********************************************************************************/
/*										*/
/*										*/
/********************************************************************************/

bool init_GPU_collide(void) {
	int err;

// get the current context.
// windows - IntPtr curDC = wglGetCurrentDC();
// then in the new compute context, we pass in the context

	/* initialized yet? */
	if (kernel != NULL) return;


	// get the device id
	int gpu=1;
	err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);

#if defined (TARGET_AQUA)
	CGLContextObj kCGLContext = CGLGetCurrentContext();
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
	cl_context_properties properties[] = {
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 0 };

	context =clCreateContext(properties,0,0,clLogMessagesToStderrAPPLE,0,&err);

#endif /* AQUA */

#if defined (WIN32)
	/* from OpenCL Programming Guide, pg 338 */
	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_KHC_HDR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)cpPlatform,
		0};

	context = clCreateContext(properties, 1, &cdDevices[uiDeviceUsed], NULL, NULL, &err);
#endif /* WIN32 */



#if defined (LINUX)
	cl_platform_id platform;

	printf ("linux, have to check clGetPlatformIDs for error \n");

	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
		0 };
   
	err = clGetPlatformIDs(1, &platform, NULL);
	context=clCreateContextFromType(properties, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
#endif

 
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PLATFORM: printf ("clCreateContext, error CL_INVALID_PLATFORM\n"); break;
			case CL_INVALID_VALUE: printf ("clCreateContext, error CL_INVALID_VALUE\n"); break;
			case CL_INVALID_DEVICE: printf ("clCreateContext, error CL_INVALID_DEVICE\n"); break;
			case CL_DEVICE_NOT_AVAILABLE: printf ("clCreateContext, error CL_DEVICE_NOT_AVAILABLE\n"); break;
			case CL_OUT_OF_HOST_MEMORY: printf ("clCreateContext, error CL_OUT_OF_HOST_MEMORY\n"); break;
			default: printf ("unknown error in clCreateContext\n");
		}
		return FALSE;
	} else {
		printf ("CL context created\n");
	}

	// create a command queue
	queue = clCreateCommandQueue(context, device_id, 0, &err);
	if (!queue || (err != CL_SUCCESS)) {
		switch (err) {
			case CL_INVALID_CONTEXT: printf ("clCreateContext, error CL_INVALID_CONTEXT\n"); break;
			case CL_INVALID_DEVICE: printf ("clCreateContext, error CL_INVALID_DEVICE\n"); break;
			case CL_INVALID_VALUE: printf ("clCreateContext, error CL_INVALID_VALUE\n"); break;
			case CL_INVALID_QUEUE_PROPERTIES: printf ("clCreateContext, error CL_INVALID_QUEUE_PROPERTIES\n"); break;
			case CL_OUT_OF_HOST_MEMORY: printf ("clCreateContext, error CL_OUT_OF_HOST_MEMORY\n"); break;
			default: printf ("unknown error in clCreateCommandQueue\n");
		}
		return FALSE;
	}
	printf ("queue created\n");
 
	// create the compute program
	{
	size_t readSize;
#define RS 32768
	char *kp;
	FILE *kf;
	kf = fopen ("/FreeWRL/freewrl/freex3d/src/lib/scenegraph/collisionKernel.txt","r");
	printf ("fopen for collisionKernel returns %p\n",kf);

	kp = malloc(RS);
	readSize = fread(kp,1,RS,kf);
	kp[readSize] = '\0'; /* ensure null termination */
	printf ("read in %d bytes max %d\n",readSize,RS);

	program = clCreateProgramWithSource(context, 1, &kp, NULL, &err);
	if (!program || (err != CL_SUCCESS)) {
		switch (err) {
			case CL_INVALID_CONTEXT: printf ("clCreateContext, error CL_INVALID_CONTEXT\n"); break;
			case CL_INVALID_VALUE: printf ("clCreateContext, error CL_INVALID_VALUE\n"); break;
			case CL_OUT_OF_HOST_MEMORY: printf ("clCreateContext, error CL_OUT_OF_HOST_MEMORY\n"); break;
			default: printf ("unknown error in clCreateProgramWithSource\n");
		}
		return FALSE;
	}
	printf ("program created\n");

	}


 
	// build the compute program executable
char *opts = "-Werror -cl-single-precision-constant -cl-nv-verbose  -g -cl-opt-disable -cl-strict-aliasing";
	err = clBuildProgram(program, 0, NULL, opts, NULL, NULL);
	//err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
        	size_t len;
        	char buffer[16384];
 
        	printf("Error: Failed to build program executable\n");           
        	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
                                          sizeof(buffer), buffer, &len);
printf ("error string len %d\n",len);
        	printf("%s\n", buffer);
        	return FALSE;
    	}
	printf ("program built\n");
 
	// create the compute kernel
	kernel = clCreateKernel(program, "compute_collide", &err);
	if (!kernel || (err != CL_SUCCESS)) {
		printf ("cl create kernel problem\n"); exit(1);
	}
	printf ("kernel built\n");
	return TRUE;
}

/********************************************************************************/
/*										*/
/*										*/
/********************************************************************************/

#define GET_SFVEC3F_COUNT ntri

//struct point_XYZ run_collide_program(GLuint vertex_vbo, GLDOUBLE *modelMat,int ntri) { 
struct point_XYZ run_collide_program(GLuint vertex_vbo, GLuint index_vbo, float *modelMat,int ntri) { 
 
	int err;
	size_t global;
	unsigned int count;

	bool face_ccw;	
	int face_flags; /* ccw, double sided, etc */
	struct OpenCLTransform transform;

	double maxdisp = 0.0;
	struct point_XYZ dispv, maxdispv = {0,0,0};

	// enough space for rv?
	if (collide_rvs.n < ntri) {

		if (collide_rvs.n != 0) {
			clReleaseMemObject(output_buffer);	
		}

		output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(struct SFVec3f) * GET_SFVEC3F_COUNT,
                                                                  NULL, NULL);

		if (matrix_buffer == NULL) {
		matrix_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof (struct OpenCLTransform), NULL, NULL);
		}

		output_size = GET_SFVEC3F_COUNT;
		collide_rvs.p = REALLOC(collide_rvs.p, sizeof(struct SFVec3f) *GET_SFVEC3F_COUNT);
		collide_rvs.n = GET_SFVEC3F_COUNT;
	}

	// update the current matrix transform
        memcpy(transform.matrix, modelMat, sizeof(cl_float16));

        clEnqueueWriteBuffer(queue, matrix_buffer, CL_TRUE, 0, sizeof(struct OpenCLTransform), &transform, 0, NULL, NULL);

	// lets get the openGL vertex buffer here
	vertex_buffer=clCreateFromGLBuffer(context, CL_MEM_READ_ONLY, vertex_vbo, &err);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT: printf ("clEnqueueNDRangeKernel, CL_INVALID_CONTEXT\n"); break;
			case CL_INVALID_VALUE: printf ("clEnqueueNDRangeKernel, CL_INVALID_VALUE\n"); break;
			case CL_INVALID_GL_OBJECT: printf ("clEnqueueNDRangeKernel, CL_INVALID_GL_OBJECT\n"); break;
			case CL_OUT_OF_RESOURCES: printf ("clEnqueueNDRangeKernel, CL_OUT_OF_RESOURCES\n"); break;
			case CL_OUT_OF_HOST_MEMORY: printf ("clEnqueueNDRangeKernel, CL_OUT_OF_HOST_MEMORY\n"); break;
			default: printf ("clCreateFromGLBuffer, failure\n");
		}
	}

	// and the coordinate index buffer
	index_buffer = clCreateFromGLBuffer(context, CL_MEM_READ_ONLY, index_vbo, &err);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT: printf ("clEnqueueNDRangeKernel, CL_INVALID_CONTEXT\n"); break;
			case CL_INVALID_VALUE: printf ("clEnqueueNDRangeKernel, CL_INVALID_VALUE\n"); break;
			case CL_INVALID_GL_OBJECT: printf ("clEnqueueNDRangeKernel, CL_INVALID_GL_OBJECT\n"); break;
			case CL_OUT_OF_RESOURCES: printf ("clEnqueueNDRangeKernel, CL_OUT_OF_RESOURCES\n"); break;
			case CL_OUT_OF_HOST_MEMORY: printf ("clEnqueueNDRangeKernel, CL_OUT_OF_HOST_MEMORY\n"); break;
			default: printf ("clCreateFromGLBuffer, failure\n");
		}
	}

	
	// set the args values
	count = (unsigned int) ntri;
	face_ccw = TRUE;
#define PR_DOUBLESIDED 0x01
#define PR_FRONTFACING 0x02 /* overrides effect of doublesided. */
#define PR_BACKFACING 0x04 /* overrides effect of doublesided, all normals are reversed. */

	face_flags = PR_FRONTFACING; /* ccw, double sided, etc */

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &output_buffer);
	clSetKernelArg(kernel, 1, sizeof(unsigned int), &count);
	clSetKernelArg(kernel, 2, sizeof (cl_mem), &matrix_buffer);
	clSetKernelArg(kernel, 3, sizeof (cl_mem), &vertex_buffer);
	clSetKernelArg(kernel, 4, sizeof (cl_mem), &index_buffer);
	clSetKernelArg(kernel, 5, sizeof(int), &face_ccw);
	clSetKernelArg(kernel, 6, sizeof(int), &face_flags);
	
	// global work group size
	global = (size_t) ntri;

	// note - we let the openCL implementation work out the local work group size`
	// so just leave this as "NULL". We could specify a local work group size, but
	// there is some math (look it up again) that is something like the global group
	// size must be divisible by the local group size, and we can not ensure this
	// as we do not know how many triangles we are getting.


  	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM_EXECUTABLE: printf ("clEnqueueNDRangeKernel, CL_INVALID_PROGRAM_EXECUTABLE\n"); break;
			case CL_INVALID_COMMAND_QUEUE: printf ("clEnqueueNDRangeKernel, CL_INVALID_COMMAND_QUEUE\n"); break;
			case CL_INVALID_KERNEL: printf ("clEnqueueNDRangeKernel, CL_INVALID_KERNEL\n"); break;
			case CL_INVALID_CONTEXT: printf ("clEnqueueNDRangeKernel, CL_INVALID_CONTEXT\n"); break;
			case CL_INVALID_KERNEL_ARGS: printf ("clEnqueueNDRangeKernel, CL_INVALID_KERNEL_ARGS\n"); break;
			case CL_INVALID_WORK_DIMENSION: printf ("clEnqueueNDRangeKernel, CL_INVALID_WORK_DIMENSION\n"); break;
			case CL_INVALID_WORK_GROUP_SIZE: printf ("clEnqueueNDRangeKernel, CL_INVALID_WORK_WORK_GROUP_SIZE\n"); break;
			case CL_INVALID_WORK_ITEM_SIZE: printf ("clEnqueueNDRangeKernel, CL_INVALID_WORK_WORK_ITEM_SIZE\n"); break;
			case CL_INVALID_GLOBAL_OFFSET: printf ("clEnqueueNDRangeKernel, CL_INVALID_GLOBAL_OFFSET\n"); break;
			case CL_OUT_OF_RESOURCES: printf ("clEnqueueNDRangeKernel, CL_OUT_OF_RESOURCES\n"); break;

			case CL_MEM_OBJECT_ALLOCATION_FAILURE: printf ("clEnqueueNDRangeKernel, CL_MEM_OBJECT_ALLOCATION_FAILURE\n"); break;
			case CL_INVALID_EVENT_WAIT_LIST: printf ("clEnqueueNDRangeKernel, CL_INVALID_EVENT_WAIT_LIST\n"); break;
			case CL_OUT_OF_HOST_MEMORY: printf ("clEnqueueNDRangeKernel, CL_OUT_OF_HOST_MEMORY\n"); break;

			default: printf ("enqueueNDRange, failure\n");


		}
		exit(1);
	}
	

	// wait for things to finish
	clFinish(queue);

	// get the data
	//err = clEnqueueReadBuffer (queue, output_buffer, CL_TRUE, 0, sizeof(struct SFVec3f) * ntri, collide_rvs.p, 0, NULL, NULL);
	err = clEnqueueReadBuffer (queue, output_buffer, CL_TRUE, 0, sizeof(struct SFVec3f) * GET_SFVEC3F_COUNT, collide_rvs.p, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE: printf ("clGetKernelWorkGroupInfo, CL_INVALID_COMMAND_QUEUE\n"); break;
			case CL_INVALID_CONTEXT: printf ("clGetKernelWorkGroupInfo, CL_INVALID_CONTEXT\n"); break;
			case CL_INVALID_MEM_OBJECT: printf ("clGetKernelWorkGroupInfo, CL_INVALID_MEM_OBJECT\n"); break;
			case CL_INVALID_VALUE: printf ("clGetKernelWorkGroupInfo, CL_INVALID_VALUE\n"); break;
			case CL_INVALID_EVENT_WAIT_LIST: printf ("clGetKernelWorkGroupInfo, CL_INVALID_EVENT_WAIT_LIST\n"); break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE: printf ("clGetKernelWorkGroupInfo, CL_MEM_OBJECT_ALLOCATION_FAILURE\n"); break;
			case CL_OUT_OF_HOST_MEMORY: printf ("clGetKernelWorkGroupInfo, CL_OUT_OF_HOST_MEMORY\n"); break;
			default: printf ("glGetKernetWorkGroupInfo, failure\n");


		}
		exit(1);
	}

#ifdef SHADERS_2011
if (!printed) {
int i;
printf ("\n**********\nshader output: ntri is %d but doing 19\n",ntri);
for (i=0; i<GET_SFVEC3F_COUNT; i++) {
#ifdef DEBUG
	switch (i) {
		case 0: printf ("cp1\t"); break;
		case 1: printf ("cp2\t"); break;
		case 2: printf ("cp3\t"); break;
		case 3: printf ("cp4\t"); break;
		case 4: printf ("i\t"); break;
		case 5: printf ("j\t"); break;
		case 6: printf ("epsil\t"); break;
		case 7: printf ("cpts\t"); break;
		case 8: printf ("norm\t"); break;
		case 9: printf ("unused\t"); break;
		case 10: printf ("unused\t"); break;
		case 11: printf ("cppto\t"); break;
		case 12: printf ("tv1\t"); break;
		case 13: printf ("tv2\t"); break;
		case 14: printf ("tv3\t"); break;
		case 15: printf ("iv1\t"); break;
		case 16: printf ("iv2\t"); break;
		case 17: printf ("iv3\t"); break;
		case 18: printf ("n\t"); break;

	}
#endif
	printf ("i %d val %f %f %f\n",i,
	collide_rvs.p[i].c[0],
	collide_rvs.p[i].c[1],
	collide_rvs.p[i].c[2]);
}
//printed = TRUE;
printf ("**********\n\n");

}
#endif


{ int i;


	for (i=0; i < GET_SFVEC3F_COUNT; i++) {
		/* XXX float to double conversion; make a vecdotf for speed */
		dispv.x = collide_rvs.p[i].c[0];
		dispv.y = collide_rvs.p[i].c[1];
		dispv.z = collide_rvs.p[i].c[2];
		double disp;

                        /*keep result only if:
                          displacement is positive
                          displacement is smaller than minimum displacement up to date
                         */

		disp = vecdot (&dispv,&dispv);
		if ((disp > FLOAT_TOLERANCE) && (disp>maxdisp)) {
			maxdisp = disp;
			maxdispv = dispv;
			printf ("OpenCL - polyrep_disp_rec, maxdisp now %f, dispv %f %f %f\n",maxdisp,dispv.x, dispv.y, dispv.z);
		}

	} 

	printf ("OpenCL - at end of opencl, maxmaxdispv %f %f %f\n",maxdispv.x, maxdispv.y, maxdispv.z);
}

	return maxdispv;
}

#endif //DO_COLLISION_GPU
