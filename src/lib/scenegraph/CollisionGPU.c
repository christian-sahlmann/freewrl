/*
=INSERT_TEMPLATE_HERE=

$Id: CollisionGPU.c,v 1.1 2011/08/08 04:16:05 crc_canada Exp $

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


int printed=FALSE;
int meprinted=FALSE;


/********************************************************************************/
/*										*/
/*										*/
/********************************************************************************/

struct OpenCLTransform
{
        cl_float16      matrix;
};

/********************************************************************************/
/*										*/
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

*/

const char *collideKernel_start = " \
#define PR_DOUBLESIDED 0x01 \n\
#define PR_FRONTFACING 0x02 /* overrides effect of doublesided. */ \n\
#define PR_BACKFACING 0x04 /* overrides effect of doublesided, all normals are reversed. */ \n\
\
\
\
float4 transformf (float4 vertex, float16 mat) { \
	float4 r; \
        r = (float4) (mat.s0*vertex.x +mat.s4*vertex.y +mat.s8*vertex.z +mat.sc, \
            mat.s1*vertex.x +mat.s5*vertex.y +mat.s9*vertex.z +mat.sd, \
            mat.s2*vertex.x +mat.s6*vertex.y +mat.sa*vertex.z +mat.se, \
	0.0);  \
return r; \
}  \
float4 vecdistance (float4 v1, float4 v2) { \
	float4 r; \
	r = (float4)(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z,0.0); \
	return r; \
} \
\
\
	__kernel void compute_collide ( \
	__global float *output,  	/* 0 */ \
        const unsigned int count,	/* 1 */ \
	__global float *mymat,   	/* 2 */ \
	__global float *my_vertex,	/* 3 */ \
	__global int *my_cindex 	/* 4 */ \
	) {  \
	int i = get_global_id(0); \
\
	/* matrix - floats into matrix */ \
	float16 mat = {mymat[0],mymat[1],mymat[2],mymat[3],mymat[4],mymat[5],mymat[6],mymat[7],mymat[8],mymat[9],mymat[10],mymat[11],mymat[12],mymat[13],mymat[14],mymat[15]}; \
\
\
	/* vertices for this triangle */ \
	float4 iv1; \
	float4 iv2; \
	float4 iv3; \
\
	/* transformed by matrix */ \
	float4 tv1; \
	float4 tv2; \
	float4 tv3; \
\
	/* starting index in my_vertex of this vertex */ \
	/* we work in triangles; each triangle has 3 vertices */ \
	int c1, c2, c3; \
	c1 = my_cindex[i*3+0]; c2=my_cindex[i*3+1]; c3=my_cindex[i*3+2]; \
\
	/* now, to index into the vertices, the index specifies starting vertex */ \
	/* but, as we have floats, not SFVec3fs, we multiply the index by 3 to */ \
	/* get the absolute starting index of the vertices */ \
	c1 = c1*3; c2 = c2 * 3; c3 = c3 * 3; \
	iv1 = (float4) (my_vertex[c1+0], my_vertex[c1+1], my_vertex[c1+2],0.0); \
	iv2 = (float4) (my_vertex[c2+0], my_vertex[c2+1], my_vertex[c2+2],0.0); \
	iv3 = (float4) (my_vertex[c3+0], my_vertex[c3+1], my_vertex[c3+2],0.0); \
\
	/* calculate normal for face */ \
	float4 v1 = vecdistance(iv2,iv1); \
	float4 v2 = vecdistance(iv3,iv1);  \
	/* float4 v1 = iv2-iv1; \
	float4 v2 = iv3-iv1; */ \
	float4 norm = normalize(cross(v1,v2)); \
\	
\
	/* transform each vertex */ \
	tv1 = transformf(iv1, mat); \
	tv2 = transformf(iv2, mat); \
	tv3 = transformf(iv3, mat); \
 \
	/* from polyrep_disp_rec2, see that function for full comments */ \
	bool ccw = true; /* assume for now will eventually pass in */ \
	bool frontfacing; \
\
	/* how we view it from the avatar */ \
	if (ccw) frontfacing = (dot(norm,tv1) < 0);  \
	else frontfacing = (dot(norm,tv1) >= 0); \
\
	/* now, is solid false, or ccw or ccw winded triangle? */ \
	/* if we should do this triangle, the if statement is true */ \
	int flags = 0; \
	if((frontfacing && !(flags & PR_DOUBLESIDED) ) \
		|| ( (flags & PR_DOUBLESIDED)  && !(flags & (PR_FRONTFACING | PR_BACKFACING) )  ) \
		|| (frontfacing && (flags & PR_FRONTFACING)) \
		|| (!frontfacing && (flags & PR_BACKFACING))  ) { \
\
                if(!frontfacing) { /*can only be here in DoubleSided mode*/ \
                        /*reverse polygon orientation, and do calculations*/ \
                        norm.x = norm.x * -1.0;\
                        norm.y = norm.y * -1.0;\
                        norm.z = norm.z * -1.0;\
                }\
";


/********************************************************************************/
/*										*/
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
	//double tmin[3],tmax[3],rmin[3],rmax[3],q[3];
	memcpy(tmin,&p[0],3*sizeof(double));
	memcpy(tmax,&p[3],3*sizeof(double));
	for(i=0;i<num;i++)
	{
		memcpy(q,&p[i],3*sizeof(double));
		for(j=0;j<3;j++)
		{
			tmin[j] = DOUBLE_MIN(tmin[j],q[j]);
			tmax[j] = DOUBLE_MAX(tmax[j],q[j]);
		}
	}
	for(i=0;i<3;i++)
	{
		rmin[i] = -r;
		rmax[i] = r;
	}
	if( !overlapMBBs(rmin,rmax,tmin,tmax) )
	{
		return zero;
	}
	/* end cheap MBB test */

#ifdef DEBUGFACEMASK
    if(facemask != debugsurface++)
	return zero;
#endif
    /*allocate data */
    if ((num+1)> pp->clippedPoly4Size) {
    	pp->clippedPoly4 = (struct point_XYZ*) REALLOC(pp->clippedPoly4,sizeof(struct point_XYZ) * (num + 1));
		pp->clippedPoly4Size = num+1;
    }

    for(i = 0; i < num; i++) {
		DEBUGPTSPRINT("intersect_closestpolypoints_on_surface[%d]= %d\n",i,clippedPoly4num);
		pp->clippedPoly4[clippedPoly4num++] = weighted_sum(p[i],p[(i+1)%num],closest_point_of_segment_to_origin(p[i],p[(i+1)%num]));
    }

    /*find closest point of polygon plane*/
    pp->clippedPoly4[clippedPoly4num] = closest_point_of_plane_to_origin(p[0],n);

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
		/* printf ("get_poly_min_disp_with_sphere, checking against %d %f %f %f",i,pp->clippedPoly4[i].x, 
		pp->clippedPoly4[i].y,pp->clippedPoly4[i].z); */

		double disp = vecdot(&pp->clippedPoly4[i],&pp->clippedPoly4[i]);

		/* printf (" disp %lf, get_poly_mindisp %lf\n",disp,get_poly_mindisp); */

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
const char *collideKernel_poly_disp_not_walking = " \
/* float4 dispv = get_poly_disp_2(tv1, tv2, tv3, 3, norm); */ \
";


/********************************************************************************/
/*										*/
/*										*/
/********************************************************************************/

/* finish and return results */
/* TODO - return displacement */

const char *collideKernel_fin = " \
	} \
\
	/* show us the transformed vertices */ \
	/* if (i==2) { \
		int oc = 0; \
\
		output[oc] = iv1.x; oc++;\
		output[oc] = iv1.y; oc++;\
		output[oc] = iv1.z; oc++;\
		output[oc] = tv1.x; oc++;\
		output[oc] = tv1.y; oc++;\
		output[oc] = tv1.z;  oc++;\
\
		output[oc] = iv2.x; oc++;\
		output[oc] = iv2.y; oc++;\
		output[oc] = iv2.z; oc++;\
		output[oc] = tv2.x; oc++;\
		output[oc] = tv2.y; oc++;\
		output[oc] = tv2.z; oc++; \
\
		output[oc] = iv3.x; oc++;\
		output[oc] = iv3.y; oc++;\
		output[oc] = iv3.z; oc++;\
		output[oc] = tv3.x; oc++;\
		output[oc] = tv3.y; oc++;\
		output[oc] = tv3.z;  oc++;\
\
\
		output[oc] = v2.x;  oc++;\
		output[oc] = v2.y;  oc++;\
		output[oc] = v2.z;  oc++;\
		output[oc] = v1.x;  oc++;\
		output[oc] = v1.y;  oc++;\
		output[oc] = v1.z;  oc++;\
		output[oc] = norm.x; oc++;\
		output[oc] = norm.y; oc++;\
		output[oc] = norm.z;  oc++;\
\
	} */ \
	output[i*3+0] = norm.x; output[i*3+1]=norm.y;output[i*3+2]=norm.z;  \
	/* output[i*3+0] = (float)frontfacing; output[i*3+1]=(float)frontfacing;output[i*3+2]=(float)frontfacing; */  \
  }" ;


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

void init_collide(struct X3D_PolyRep pr) {
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

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PLATFORM: printf ("clCreateContext, error CL_INVALID_PLATFORM\n"); break;
			case CL_INVALID_VALUE: printf ("clCreateContext, error CL_INVALID_VALUE\n"); break;
			case CL_INVALID_DEVICE: printf ("clCreateContext, error CL_INVALID_DEVICE\n"); break;
			case CL_DEVICE_NOT_AVAILABLE: printf ("clCreateContext, error CL_DEVICE_NOT_AVAILABLE\n"); break;
			case CL_OUT_OF_HOST_MEMORY: printf ("clCreateContext, error CL_OUT_OF_HOST_MEMORY\n"); break;
			default: printf ("unknown error in clCreateContext\n");
		}
		exit(1);
	} else {
		printf ("CL context created\n");
	}

#endif
#if defined (LINUX)
   cl_platform_id platform;
   err = clGetPlatformIDs(1, &platform, NULL);
   reportaError("error en el platform",err);
   cl_context_properties props[] =
   {
      CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
      CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
      CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
      0
   };
   
   context=clCreateContextFromType(props, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
#endif
 
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
		exit (1);
	}
	printf ("queue created\n");
 
	// create the compute program
	const char *progs[] = {collideKernel_start, collideKernel_fin, NULL, NULL};

	//program = clCreateProgramWithSource(context, 2, &collideKernel, NULL, &err);
	program = clCreateProgramWithSource(context, 2, progs, NULL, &err);
	if (!program || (err != CL_SUCCESS)) {
		switch (err) {
			case CL_INVALID_CONTEXT: printf ("clCreateContext, error CL_INVALID_CONTEXT\n"); break;
			case CL_INVALID_VALUE: printf ("clCreateContext, error CL_INVALID_VALUE\n"); break;
			case CL_OUT_OF_HOST_MEMORY: printf ("clCreateContext, error CL_OUT_OF_HOST_MEMORY\n"); break;
			default: printf ("unknown error in clCreateProgramWithSource\n");
		}
		exit(1);
	}
	printf ("program created\n");


 
	// build the compute program executable
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
        	size_t len;
        	char buffer[2048];
 
        	printf("Error: Failed to build program executable\n");           
        	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
                                          sizeof(buffer), buffer, &len);
        	printf("%s\n", buffer);
        	exit(1);
    	}
	printf ("program built\n");
 
	// create the compute kernel
	kernel = clCreateKernel(program, "compute_collide", &err);
	if (!kernel || (err != CL_SUCCESS)) {
		printf ("cl create kernel problem\n"); exit(1);
	}
	printf ("kernel built\n");
}

/********************************************************************************/
/*										*/
/*										*/
/********************************************************************************/

//struct point_XYZ run_collide_program(GLuint vertex_vbo, GLDOUBLE *modelMat,int ntri) { 
void run_collide_program(GLuint vertex_vbo, GLuint index_vbo, float *modelMat,int ntri) { 
 
	int err;
	size_t global;
	unsigned int count;
	struct OpenCLTransform transform;

	// enough space for rv?
	if (collide_rvs.n < ntri) {

		if (collide_rvs.n != 0) {
			clReleaseMemObject(output_buffer);	
		}
		output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(struct SFVec3f) *ntri,
                                                                  NULL, NULL);

		if (matrix_buffer == NULL) {
		matrix_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof (struct OpenCLTransform), NULL, NULL);
		}

		output_size = ntri;
		collide_rvs.p = REALLOC(collide_rvs.p, sizeof(struct SFVec3f) *ntri);
		collide_rvs.n = ntri;
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
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &output_buffer);
	clSetKernelArg(kernel, 1, sizeof(unsigned int), &count);
	clSetKernelArg(kernel, 2, sizeof (cl_mem), &matrix_buffer);
	clSetKernelArg(kernel, 3, sizeof (cl_mem), &vertex_buffer);
	clSetKernelArg(kernel, 4, sizeof (cl_mem), &index_buffer);

 
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
	err = clEnqueueReadBuffer (queue, output_buffer, CL_TRUE, 0, sizeof(struct SFVec3f) * ntri, collide_rvs.p, 0, NULL, NULL);
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
printf ("ntri is %d\n",ntri);
for (i=0; i<ntri; i++) printf ("i %d val %f %f %f\n",i,
	collide_rvs.p[i].c[0],
	collide_rvs.p[i].c[1],
	collide_rvs.p[i].c[2]);
printed = TRUE;
}
#endif


}

#endif //DO_COLLISION_GPU
