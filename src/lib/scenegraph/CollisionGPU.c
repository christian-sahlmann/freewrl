/*
=INSERT_TEMPLATE_HERE=

$Id: CollisionGPU.c,v 1.19 2011/09/20 18:24:00 crc_canada Exp $

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

static const char* collide_non_walk_kernel;

#define FLOAT_TOLERANCE 0.00000001

/********************************************************************************/
/*										*/
/* 	Collide kernel, generic structures, etc					*/
/*										*/
/********************************************************************************/


#ifdef OLDCODE
OLDCODE//http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_round_up_to_power_of_two
OLDCODEstatic size_t roundUpToNextPowerOfTwo(size_t n) {
OLDCODE	if (n==0) return 1;
OLDCODE
OLDCODE	n = n - 1;
OLDCODE	n = n | (n >> 1);
OLDCODE	n = n | (n >> 2);
OLDCODE	n = n | (n >> 4);
OLDCODE	n = n | (n >> 8);
OLDCODE	n = n | (n >> 16);
OLDCODE	if (sizeof(size_t)>32)
OLDCODE		n = n | (n >> 32);
OLDCODE	n = n + 1;
OLDCODE	return n;
OLDCODE}
#endif // OLDCODE



static void printCLError(const char *where, int err) {
	switch (err) {

		case CL_INVALID_MEM_OBJECT: ConsoleMessage ("%s, error CL_INVALID_MEM_OBJECT",where); break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: ConsoleMessage ("%s, error CL_MEM_OBJECT_ALLOCATION_FAILURE",where); break;
		case CL_INVALID_PROGRAM_EXECUTABLE: ConsoleMessage ("%s, error CL_INVALID_PROGRAM_EXECUTABLE",where); break;
		case CL_INVALID_COMMAND_QUEUE: ConsoleMessage ("%s, error CL_INVALID_COMMAND_QUEUE",where); break;
		case CL_INVALID_KERNEL_ARGS: ConsoleMessage ("%s, error CL_INVALID_KERNEL_ARGS",where); break;
		case CL_INVALID_KERNEL: ConsoleMessage ("%s, error CL_INVALID_KERNEL",where); break;
		case CL_INVALID_WORK_ITEM_SIZE: ConsoleMessage ("%s, error CL_INVALID_WORK_ITEM_SIZE",where); break;
		case CL_INVALID_WORK_GROUP_SIZE: ConsoleMessage ("%s, error CL_INVALID_WORK_GROUP_SIZE",where); break;
		case CL_INVALID_WORK_DIMENSION: ConsoleMessage ("%s, error CL_INVALID_WORK_DIMENSION",where); break;
		case CL_INVALID_GLOBAL_OFFSET: ConsoleMessage ("%s, error CL_INVALID_GLOBAL_OFFSET",where); break;
		case CL_INVALID_EVENT_WAIT_LIST: ConsoleMessage ("%s, error CL_INVALID_EVENT_WAIT_LIST",where); break;
		case CL_INVALID_GL_OBJECT: ConsoleMessage ("%s, error CL_INVALID_GL_OBJECT",where); break;
		case CL_INVALID_QUEUE_PROPERTIES: ConsoleMessage ("%s, error CL_INVALID_QUEUE_PROPERTIES",where); break;
		case CL_INVALID_CONTEXT: ConsoleMessage ("%s, error CL_INVALID_CONTEXT",where); break;
		case CL_INVALID_PLATFORM: ConsoleMessage ("%s, error CL_INVALID_PLATFORM",where); break;
		case CL_INVALID_VALUE: ConsoleMessage ("%s, error CL_INVALID_VALUE",where); break;
		case CL_INVALID_DEVICE: ConsoleMessage ("%s, error CL_INVALID_DEVICE",where); break;
		case CL_DEVICE_NOT_AVAILABLE: ConsoleMessage ("%s, error CL_DEVICE_NOT_AVAILABLE",where); break;
		case CL_OUT_OF_HOST_MEMORY: ConsoleMessage ("%s, error CL_OUT_OF_HOST_MEMORY",where); break;
		case CL_OUT_OF_RESOURCES: ConsoleMessage("%s, error CL_OUT_OF_RESOURCES",where);break;
		default: ConsoleMessage ("unknown OpenCL error in %s",where);
	}
}


/********************************************************************************/
/*										*/
/*										*/
/********************************************************************************/
#ifdef _MSC_VER_NOT
cl_platform_id cpPlatform = NULL;          // OpenCL platform
cl_device_id* cdDevices = NULL;     // device list
cl_uint uiTargetDevice = 0;	        // Default Device to compute on



cl_int ciErrNum;			        // Error code var
enum LOGMODES 
{
    LOGCONSOLE = 1, // bit to signal "log to console" 
    LOGFILE    = 2, // bit to signal "log to file" 
    LOGBOTH    = 3, // convenience union of first 2 bits to signal "log to both"
    APPENDMODE = 4, // bit to set "file append" mode instead of "replace mode" on open
    MASTER     = 8, // bit to signal master .csv log output
    ERRORMSG   = 16, // bit to signal "pre-pend Error" 
    CLOSELOG   = 32  // bit to close log file, if open, after any requested file write
};
int bQATest = 0;			// false = normal GL loop, true = run No-GL test sequence
bool bGLinteropSupported = false;	// state var for GL interop supported or not
cl_uint uiNumDevsUsed = 1;          // Number of devices used in this sample 
bool bGLinterop = false;			// state var for GL interop or not

void Cleanup(int iExitCode)
{
    // Cleanup allocated objects
    //shrLog("\nStarting Cleanup...\n\n");
    if(program)clReleaseProgram(program);
    if(context)clReleaseContext(context);
    if(cdDevices)free(cdDevices);

    // Cleanup GL objects if used
    if (!bQATest)
    {
        //DeInitGL();
    }

    // finalize logs and leave
    //shrLog("%s\n\n", iExitCode == 0 ? "PASSED" : "FAILED"); 
    if ((bQATest))
    {
       // shrLogEx(LOGBOTH | CLOSELOG, 0, "oclBoxFilter.exe Exiting...\n");
    }
    else 
    {
        //shrLogEx(LOGBOTH | CLOSELOG, 0, "oclBoxFilter.exe Exiting...\nPress <Enter> to Quit\n");
        #ifdef WIN32
            getchar();
        #endif
    }
    exit (iExitCode);
}
void (*pCleanup)(int) = &Cleanup;
#define shrLog printf
cl_int oclGetPlatformID(cl_platform_id* clSelectedPlatformID)
{
    char chBuffer[1024];
    cl_uint num_platforms; 
    cl_platform_id* clPlatformIDs;
    cl_int ciErrNum;
    *clSelectedPlatformID = NULL;

    // Get OpenCL platform count
    ciErrNum = clGetPlatformIDs (0, NULL, &num_platforms);
    if (ciErrNum != CL_SUCCESS)
    {
        shrLog(" Error %i in clGetPlatformIDs Call !!!\n\n", ciErrNum);
        return -1000;
    }
    else 
    {
        if(num_platforms == 0)
        {
            shrLog("No OpenCL platform found!\n\n");
            return -2000;
        }
        else 
        {
			cl_uint i;
            // if there's a platform or more, make space for ID's
            if ((clPlatformIDs = (cl_platform_id*)malloc(num_platforms * sizeof(cl_platform_id))) == NULL)
            {
                shrLog("Failed to allocate memory for cl_platform ID's!\n\n");
                return -3000;
            }

            // get platform info for each platform and trap the NVIDIA platform if found
            ciErrNum = clGetPlatformIDs (num_platforms, clPlatformIDs, NULL);
            for(i = 0; i < num_platforms; ++i)
            {
                ciErrNum = clGetPlatformInfo (clPlatformIDs[i], CL_PLATFORM_NAME, 1024, &chBuffer, NULL);
                if(ciErrNum == CL_SUCCESS)
                {
                    if(strstr(chBuffer, "NVIDIA") != NULL)
                    {
                        *clSelectedPlatformID = clPlatformIDs[i];
                        break;
                    }
                }
            }

            // default to zeroeth platform if NVIDIA not found
            if(*clSelectedPlatformID == NULL)
            {
                shrLog("WARNING: NVIDIA OpenCL platform not found - defaulting to first platform!\n\n");
                *clSelectedPlatformID = clPlatformIDs[0];
            }

            free(clPlatformIDs);
        }
    }

    return CL_SUCCESS;
}
int extraInitFromNvidiaSamples(struct sCollisionGPU* initme)
{
    cl_uint uiNumDevices = 0;           // Number of devices available
    cl_uint uiTargetDevice = 0;	        // Default Device to compute on
    cl_uint uiNumComputeUnits;          // Number of compute units (SM's on NV GPU)

	// Get the NVIDIA platform
    ciErrNum = oclGetPlatformID(&cpPlatform);
    //oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
    shrLog("clGetPlatformID...\n"); 

    //Get all the devices
    //shrLog("Get the Device info and select Device...\n");
    ciErrNum = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 0, NULL, &uiNumDevices);
    //oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
    cdDevices = (cl_device_id *)malloc(uiNumDevices * sizeof(cl_device_id) );
    ciErrNum = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, uiNumDevices, cdDevices, NULL);
    //oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

    // Set target device and Query number of compute units on uiTargetDevice
    shrLog("  # of Devices Available = %u\n", uiNumDevices); 
    //if(shrGetCmdLineArgumentu(argc, (const char**)argv, "device", &uiTargetDevice)== shrTRUE) 
    //{
    //    uiTargetDevice = CLAMP(uiTargetDevice, 0, (uiNumDevices - 1));
    //}
    shrLog("  Using Device %u: ", uiTargetDevice); 
    //oclPrintDevName(LOGBOTH, cdDevices[uiTargetDevice]);
	initme->device_id = cdDevices[uiTargetDevice];
    ciErrNum = clGetDeviceInfo(cdDevices[uiTargetDevice], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(uiNumComputeUnits), &uiNumComputeUnits, NULL);
    //oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
    shrLog("\n  # of Compute Units = %u\n", uiNumComputeUnits); 

    // Check for GL interop capability (if using GL)
    if(!bQATest)
    {
        char extensions[1024];
        ciErrNum = clGetDeviceInfo(cdDevices[uiTargetDevice], CL_DEVICE_EXTENSIONS, 1024, extensions, 0);
        //oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
        
        #if defined (__APPLE__) || defined(MACOSX)
            bGLinteropSupported = strstr(extensions,"cl_APPLE_gl_sharing") != NULL;
        #else
            bGLinteropSupported = strstr(extensions,"cl_khr_gl_sharing") != NULL;
        #endif
    }

    //Create the context
    if(bGLinteropSupported) 
    {
        // Define OS-specific context properties and create the OpenCL context
        #if defined (__APPLE__)
            CGLContextObj kCGLContext = CGLGetCurrentContext();
            CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
            cl_context_properties props[] = 
            {
                CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 
                0 
            };
            cxGPUContext = clCreateContext(props, 0,0, NULL, NULL, &ciErrNum);
        #else
            #ifdef UNIX
                cl_context_properties props[] = 
                {
                    CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(), 
                    CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(), 
                    CL_CONTEXT_PLATFORM, (cl_context_properties)cpPlatform, 
                    0
                };
                cxGPUContext = clCreateContext(props, uiNumDevsUsed, &cdDevices[uiTargetDevice], NULL, NULL, &ciErrNum);
            #else // Win32
                cl_context_properties props[] = 
                {
                    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), 
                    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), 
                    CL_CONTEXT_PLATFORM, (cl_context_properties)cpPlatform, 
                    0
                };
                context = clCreateContext(props, uiNumDevsUsed, &cdDevices[uiTargetDevice], NULL, NULL, &ciErrNum);
				printf("ciErrNum=%d\n",ciErrNum);
            #endif
        #endif
        shrLog("clCreateContext, GL Interop supported...\n"); 
    } 
    else 
    {
        bGLinterop = false;
        context = clCreateContext(0, uiNumDevsUsed, &cdDevices[uiTargetDevice], NULL, NULL, &ciErrNum);
        shrLog("clCreateContext, GL Interop %s...\n", bQATest ? "N/A" : "not supported"); 
    }
   // oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	return ciErrNum;
}
#endif  //_MSC_VER

bool init_GPU_collide(struct sCollisionGPU* initme) {

	int err;

	// debugging information
	cl_int rv;
	size_t rvlen;
	size_t wg_size;
	size_t kernel_wg_size;

	#ifdef DEBUG
	cl_ulong longish;
	size_t xyz;
	char rvstring[1000];
	int gpu;
	#endif // DEBUG



// get the current context.
// windows - IntPtr curDC = wglGetCurrentDC();
// then in the new compute context, we pass in the context

	/* initialized yet? */
	if (initme->kernel != NULL) return false;


	// get the device id

#if defined (TARGET_AQUA)
	CGLContextObj kCGLContext = CGLGetCurrentContext();
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
	cl_context_properties properties[] = {
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 0 };

	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &initme->device_id, NULL);
	if (err != CL_SUCCESS) {
		printCLError("clGetDeviceIDs",err);
		return FALSE;
	}

	initme->context =clCreateContext(properties,0,0,clLogMessagesToStderrAPPLE,0,&err);

#endif /* AQUA */

#if defined (WIN32)

	//if(1)
	//	err = extraInitFromNvidiaSamples(initme);
	//else
	{
		cl_int ciErrNum;
		cl_platform_id cpPlatform = NULL;          // OpenCL platform
		// Get the NVIDIA platform
		//ciErrNum = oclGetPlatformID(&cpPlatform);
		{
			/* from OpenCL Programming Guide, pg 338 */
			cl_context_properties properties[] = {
				CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
				CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
				CL_CONTEXT_PLATFORM, (cl_context_properties)cpPlatform,
				0};

			initme->context = clCreateContext(properties, 1, &initme->device_id, NULL, NULL, &err);
		}
	}
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
		printCLError("clCreateContext",err);
		return FALSE;
	} else {
		printf ("CL context created\n");
	}


	// create a command queue
	initme->queue = clCreateCommandQueue(initme->context, initme->device_id, 0, &err);
	if (!initme->queue || (err != CL_SUCCESS)) {
		printCLError("clCreateCommandQueue",err);
		return FALSE;
	}
	printf ("queue created\n");
 
	{
	char *kp;

#ifdef READ_FROM_FILE
	// create the compute program
	size_t readSize;
#define RS 32768
	FILE *kf;
#ifdef _MSC_VER
	char * kernelpath = "C:/source2/freewrl/freex3d/src/lib/scenegraph/collisionKernel.txt";
#else
	char * kernelpath = "/FreeWRL/freewrl/freex3d/src/lib/scenegraph/collisionKernel.txt";
#endif
	kf = fopen (kernelpath,"r");
	if (!kf) {
		ConsoleMessage("can not find collisionKernel.txt, reverting to SW collision method");
		return FALSE;
	}


	kp = malloc(RS);
	readSize = fread(kp,1,RS,kf);
	kp[readSize] = '\0'; /* ensure null termination */
	printf ("read in %d bytes max %d\n",readSize,RS);
#else
	kp = (char *)collide_non_walk_kernel;
#endif


	// Find the work group size
	rv = clGetDeviceInfo (initme->device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &wg_size, &rvlen);

	#undef DEBUG
	#ifdef DEBUG
	// debugging information
	rv = clGetPlatformInfo(NULL,CL_PLATFORM_PROFILE,1000,rvstring,&rvlen);
	printf ("CL_PLATFORM_PROFILE :%s:\n",rvstring);
	rv = clGetPlatformInfo(NULL,CL_PLATFORM_VERSION,1000,rvstring,&rvlen);
	printf ("CL_PLATFORM_VERSION :%s:\n",rvstring);
	rv = clGetPlatformInfo(NULL,CL_PLATFORM_NAME,1000,rvstring,&rvlen);
	printf ("CL_PLATFORM_NAME :%s:\n",rvstring);
	rv = clGetPlatformInfo(NULL,CL_PLATFORM_VENDOR,1000,rvstring,&rvlen);
	printf ("CL_PLATFORM_VENDOR :%s:\n",rvstring);
	rv = clGetPlatformInfo(NULL,CL_PLATFORM_EXTENSIONS,1000,rvstring,&rvlen);
	printf ("CL_PLATFORM_EXTENSIONS :%s:\n",rvstring);
	printf ("CL_DEVICE_MAX_WORK_GROUP_SIZE %d\n",wg_size);
	rv = clGetDeviceInfo (initme->device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(size_t), &xyz, &rvlen);
	printf ("CL_DEVICE_MAX_COMPUTE_UNITS %d\n",xyz);

	rv = clGetDeviceInfo (initme->device_id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &longish, &rvlen);
	printf ("CL_DEVICE_GLOBAL_MEM_CACHE_SIZE %ld\n",longish);
	rv = clGetDeviceInfo (initme->device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &longish, &rvlen);
	printf ("CL_DEVICE_GLOBAL_MEM_SIZE %ld\n",longish);
	rv = clGetDeviceInfo (initme->device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &longish, &rvlen);
	printf ("CL_DEVICE_LOCAL_MEM_SIZE %ld\n",longish);


	#endif //DEBUG


	initme->program = clCreateProgramWithSource(initme->context, 1, (const char **) &kp, NULL, &err);
	if (!initme->program || (err != CL_SUCCESS)) {
		printCLError("clCreateProgramWithSource",err);
		return FALSE;
	}
	printf ("program created\n");

	}


 
	// build the compute program executable
	//char *opts = "-Werror -cl-single-precision-constant -cl-nv-verbose  -g -cl-opt-disable -cl-strict-aliasing";
	//char *opts = "-Werror -cl-single-precision-constant -cl-opt-disable -cl-strict-aliasing";
	//err = clBuildProgram(initme->program, 0, NULL, opts, NULL, NULL);
	err = clBuildProgram(initme->program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
        	size_t len;
        	char buffer[16384];
 
        	printf("Error: Failed to build program executable\n");           
        	clGetProgramBuildInfo(initme->program, initme->device_id, CL_PROGRAM_BUILD_LOG,
                                          sizeof(buffer), buffer, &len);
		printf ("error string len %d\n",(int)len);
        	printf("%s\n", buffer);
        	return FALSE;
    	}
	printf ("program built\n");
 
	// create the compute kernel
	initme->kernel = clCreateKernel(initme->program, "compute_collide", &err);
	if (!initme->kernel || (err != CL_SUCCESS)) {
		printf ("cl create kernel problem\n"); exit(1);
	}


	// Kernel Workgroup size
	rv = clGetKernelWorkGroupInfo (initme->kernel, initme->device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &kernel_wg_size, &rvlen);

	// try the smaller of the two
	if (kernel_wg_size < wg_size) wg_size = kernel_wg_size;
	initme->workgroup_size = wg_size;

	#ifdef DEBUG
	printf ("MAX_WORK_GROUP_SIZE %d\n",kernel_wg_size);
	printf ("We are going to set our workgroup size to %d\n",wg_size);


/*
1. Get workGroupSize from clGetDeviceInfo with CL_DEVICE_mum of two values and use that value as your optimal workGroupSize
2. Get KernelWorkGroupSize from from clGetKernelWorkGroupInfo with CL_KERNEL_WORK_GPOUP_SIZE
3. Get minimum of two values and use that value as your optimal workGroupSize
*/

	#endif // DEBUG

	printf ("kernel built\n");

	return TRUE;
}

/********************************************************************************/
/*										*/
/*										*/
/********************************************************************************/

struct point_XYZ run_non_walk_collide_program(GLuint vertex_vbo, GLuint index_vbo, float *modelMat,int ntri,
		int face_ccw, int face_flags, float avatar_radius) {
 
	int i;
	int err;
	size_t local_work_size;
	size_t global_work_size;
	unsigned int count;

	double maxdisp = 0.0;
	struct point_XYZ dispv, maxdispv = {0,0,0};

	struct sCollisionGPU* me = GPUCollisionInfo();

	// enough space for rv?
	if (me->collide_rvs.n < ntri) {

		if (me->collide_rvs.n != 0) {
			clReleaseMemObject(me->output_buffer);	
		}

		me->output_buffer = clCreateBuffer(me->context, CL_MEM_WRITE_ONLY, sizeof(struct SFColorRGBA) * ntri,
                                                                  NULL, NULL);

		if (me->matrix_buffer == NULL) {
		me->matrix_buffer = clCreateBuffer(me->context, CL_MEM_READ_ONLY, sizeof (cl_float16), NULL, NULL);
		}

		me->output_size = ntri;
		me->collide_rvs.p = REALLOC(me->collide_rvs.p, sizeof(struct SFColorRGBA) *ntri);
		me->collide_rvs.n = ntri;
	}

	// update the current matrix transform
        clEnqueueWriteBuffer(me->queue, me->matrix_buffer, CL_TRUE, 0, sizeof(cl_float16), modelMat, 0, NULL, NULL);

	// lets get the openGL vertex buffer here
	me->vertex_buffer=clCreateFromGLBuffer(me->context, CL_MEM_READ_ONLY, vertex_vbo, &err);
	if (err != CL_SUCCESS) {
		printCLError("clCreateFromGLBuffer",err);
		return maxdispv;
	}

	// and the coordinate index buffer
	me->index_buffer = clCreateFromGLBuffer(me->context, CL_MEM_READ_ONLY, index_vbo, &err);
	if (err != CL_SUCCESS) {
		printCLError("clCreateFromGLBuffer",err);
		return maxdispv;
	}

	
	// set the args values
	count = (unsigned int) ntri;

	clSetKernelArg(me->kernel, 0, sizeof(cl_mem), &me->output_buffer);
	clSetKernelArg(me->kernel, 1, sizeof(unsigned int), &count);
	clSetKernelArg(me->kernel, 2, sizeof (cl_mem), &me->matrix_buffer);
	clSetKernelArg(me->kernel, 3, sizeof (cl_mem), &me->vertex_buffer);
	clSetKernelArg(me->kernel, 4, sizeof (cl_mem), &me->index_buffer);
	clSetKernelArg(me->kernel, 5, sizeof(int), &face_ccw);
	clSetKernelArg(me->kernel, 6, sizeof(int), &face_flags);
	clSetKernelArg(me->kernel, 7, sizeof(int), &avatar_radius);
	clSetKernelArg(me->kernel, 8, sizeof(int), &ntri);
	
	// global work group size
	#define MYWG (me->workgroup_size)
	// find out how many "blocks" we can have
	if (MYWG > 0)
		global_work_size = (size_t) (ntri) / MYWG;
	else global_work_size = 0;

	// add 1 to it, because we have to round up
	global_work_size += 1;

	// now, global_work_size will be an exact multiple of local_work_size
	global_work_size *= MYWG;

	// printf ("global_work_size is %d %x right now...\n",global_work_size, global_work_size);

	local_work_size = MYWG;
	//printf ("local_work_size %d\n",local_work_size);

	// note - we let the openCL implementation work out the local work group size`
	// so just leave this as "NULL". We could specify a local work group size, but
	// there is some math (look it up again) that is something like the global group
	// size must be divisible by the local group size, and we can not ensure this
	// as we do not know how many triangles we are getting.

	// printf ("ntri %d, global_work_size %d, local_work_size %d\n",ntri,global_work_size,local_work_size);

	// If we let the system determing workgroup size, we can get non-optimal workgroup sizes.
  	//err = clEnqueueNDRangeKernel(me->queue, me->kernel, 1, NULL, &ntri, NULL, 0, NULL, NULL);

  	err = clEnqueueNDRangeKernel(me->queue, me->kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printCLError("clEnqueueNDRangeKernel",err);
		return maxdispv;
	}
	

	// wait for things to finish
	// get the data
	err = clEnqueueReadBuffer (me->queue, me->output_buffer, 
		CL_TRUE, 0, sizeof(struct SFColorRGBA) * ntri, 
		me->collide_rvs.p, 0, NULL, NULL);

	if (err != CL_SUCCESS) {
		printCLError("clEnqueueReadBuffer",err);
		return maxdispv;
	}


	for (i=0; i < ntri; i++) {
		/* XXX float to double conversion; make a vecdotf for speed */
		double disp;

		// we use the last float to indicate whether to bother here; saves us
		// doing unneeded calculations here

		if (me->collide_rvs.p[i].c[3] > 1.0) {
			// printf ("possibly triangle %d has some stuff for us\n",i);


			dispv.x = me->collide_rvs.p[i].c[0];
			dispv.y = me->collide_rvs.p[i].c[1];
			dispv.z = me->collide_rvs.p[i].c[2];
			 //printf ("GPU tri %d, disp %f %f %f\n",i,dispv.x,dispv.y,dispv.z);

                        /*keep result only if:
                          displacement is positive
                          displacement is smaller than minimum displacement up to date
                         */

			disp = vecdot (&dispv,&dispv);
			if ((disp > FLOAT_TOLERANCE) && (disp>maxdisp)) {
				maxdisp = disp;
				maxdispv = dispv;
			}
		}

	}

	
	// printf ("OpenCL - at end of opencl, maxdispv %f %f %f\n",maxdispv.x, maxdispv.y, maxdispv.z); 

	return maxdispv;
}

static const char* collide_non_walk_kernel = " \
#pragma OPENCL EXTENSION cl_khr_fp64 : enable \n\
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable \n\
#pragma OPENCL EXTENSION CL_APPLE_gl_sharing : enable \n\
#pragma OPENCL EXTENSION CL_KHR_gl_sharing : enable \n\
#pragma OPENCL EXTENSION cl_khr_select_fprounding_mode : enable \n\
 \n\
/********************************************************************************/ \n\
/*										*/ \n\
/*	Collide kernel for fly and examine modes				*/ \n\
/*										*/ \n\
/********************************************************************************/ \n\
/* Function prototypes */ \n\
float4 closest_point_on_plane(float4 point_a, float4 point_b, float4 point_c); \n\
 \n\
/* start the collide process. \n\
 \n\
1) transform the vertex. \n\
2) calculate normal \n\
3) if triangle is visible to us, get ready for collide calcs \n\
 \n\
*/ \n\
 \n\
 \n\
#define DOUGS_FLOAT_TOLERANCE 0.00000001 \n\
#define FLOAT_TOLERANCE 0.0000001 \n\
#define PR_DOUBLESIDED 0x01  \n\
#define PR_FRONTFACING 0x02 /* overrides effect of doublesided. */  \n\
#define PR_BACKFACING 0x04 /* overrides effect of doublesided, all normals are reversed. */  \n\
 \n\
/********************************************************************************/ \n\
 \n\
 \n\
#define APPROX (a, b) (fabs(a-b) < FLOAT_TOLERANCE) \n\
#define VECSCALE(v,s) (float4)(v.x*s, v.y*s, v.z*s, 0.0) \n\
#define VECLENGTH(v) (float)sqrt((float)dot((float4)v,(float4)v)) \n\
 \n\
 \n\
 \n\
/********************************************************************************/ \n\
/*										*/ \n\
/*	Three vertices; find the closest one which intersects the Z plane;	*/ \n\
/*	either we choose a Vertex, on an edge, or fabricate one in the		*/ \n\
/*	middle of the triangle somewhere.					*/ \n\
/*										*/ \n\
/*	Adapted from \"Real time Collision Detection\", Christer Ericson.		*/ \n\
/*										*/ \n\
/********************************************************************************/ \n\
 \n\
 \n\
float4 closest_point_on_plane(float4 point_a, float4 point_b, float4 point_c) { \n\
	float4 vector_ab = (point_b - point_a); // b - a \n\
	float4 vector_ac = (point_c - point_a); // c - a \n\
	float4 vector_bc = (point_c - point_b); // c - b \n\
	float4 vector_ba = (point_a - point_b); // a - b \n\
	float4 vector_ca = (point_a - point_c); // a - c \n\
	float4 vector_cb = (point_b - point_c); // b - c \n\
 \n\
 \n\
	// we have moved points, so our bounding sphere is at (0,0,0) so p = (0,0,0) \n\
	float4 vector_ap = point_a * (float4)(-1.0, -1.0, -1.0, -1.0); // p - a \n\
	float4 vector_bp = point_b * (float4)(-1.0, -1.0, -1.0, -1.0); // p - b \n\
	float4 vector_cp = point_c * (float4)(-1.0, -1.0, -1.0, -1.0); // p - c \n\
	#define vector_pa point_a    /* a - p */ \n\
	#define vector_pb point_b    /* b - p */ \n\
	#define vector_pc point_c    /* c - p */ \n\
	 \n\
	// Step 2. Compute parametric position s for projection P' of P on AB, \n\
	// P' = A + s*AB, s = snom/(snom+sdenom) \n\
 \n\
	float snom = dot(vector_ap, vector_ab); // (p - a, ab); \n\
	float sdenom = dot(vector_bp, vector_ba); // (p - b, a - b); \n\
 \n\
	// Step 3. \n\
	// Compute parametric position t for projection P' of P on AC, \n\
	// P' = A + t*AC, s = tnom/(tnom+tdenom) \n\
	float tnom = dot(vector_ap, vector_ac); // (p - a, ac); \n\
	float tdenom = dot(vector_cp, vector_ca); //  (p - c, a - c); \n\
 \n\
	// Step 4. \n\
	if (snom <= 0.0f && tnom <= 0.0f) { \n\
		return point_a; \n\
	} \n\
 \n\
	// Step 5. \n\
	// Compute parametric position u for projection P' of P on BC, \n\
	// P' = B + u*BC, u = unom/(unom+udenom) \n\
	float unom = dot(vector_bp, vector_bc); //(p - b, bc) \n\
	float udenom = dot(vector_cp, vector_cb); // (p - c, b - c); \n\
 \n\
	// Step 6. \n\
	if (sdenom <= 0.0f && unom <= 0.0f) { \n\
		return point_b; \n\
	} \n\
 \n\
	if (tdenom <= 0.0f && udenom <= 0.0f) { \n\
		return point_c; \n\
	} \n\
 \n\
 \n\
	// Step 7. \n\
	// P is outside (or on) AB if the triple scalar product [N PA PB] <= 0 \n\
	float4 n; \n\
	float4 tmp; \n\
	float vc; \n\
 \n\
	n = cross(vector_ab, vector_ac); // (b - a, c - a); \n\
	tmp = cross(vector_pa, vector_pb); // veccross (a-p, b-p); \n\
 \n\
	// vc = dot(n, veccross(a - p, b - p)); \n\
	vc = dot(n, tmp); \n\
 \n\
 \n\
	// If P outside AB and within feature region of AB, \n\
	// return projection of P onto AB \n\
	if (vc <= 0.0f && snom >= 0.0f && sdenom >= 0.0f) { \n\
		return point_a  + snom / (snom + sdenom) * vector_ab; \n\
	} \n\
 \n\
 \n\
 \n\
	// Step 8. \n\
	// P is outside (or on) BC if the triple scalar product [N PB PC] <= 0 \n\
	tmp = cross (vector_pb, vector_pc); \n\
 \n\
	float va = dot(n, tmp); // Cross(b - p, c - p)); \n\
	 \n\
	// If P outside BC and within feature region of BC, \n\
	// return projection of P onto BC \n\
	if (va <= 0.0f && unom >= 0.0f && udenom >= 0.0f) { \n\
		return point_b + unom / (unom + udenom) * vector_bc; \n\
	} \n\
 \n\
	// Step 9. \n\
	// P is outside (or on) CA if the triple scalar product [N PC PA] <= 0 \n\
	tmp = cross (vector_pc, vector_pa); \n\
 \n\
	float vb = dot(n, tmp); //  Cross(c - p, a - p)); \n\
	// If P outside CA and within feature region of CA, \n\
	// return projection of P onto CA \n\
	if (vb <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f) { \n\
		return point_a + tnom / (tnom + tdenom) * vector_ac; \n\
	} \n\
 \n\
	// 10. \n\
	// P must project inside face region. Compute Q using barycentric coordinates \n\
	float u = va / (va + vb + vc); \n\
	float v = vb / (va + vb + vc); \n\
	float w = 1.0f - u - v; // = vc / (va + vb + vc) \n\
	float4 u4 = (float4)(u); \n\
	float4 v4 = (float4)(v); \n\
	float4 w4 = (float4)(w); \n\
 \n\
	//return u * point_a + v * point_b + w * point_c; \n\
	float4 rv = mad(point_a,u4,mad(point_b,v4,point_c*w4)); \n\
	return rv; \n\
} \n\
 \n\
/********************************************************************************/ \n\
 \n\
	__kernel void compute_collide (  \n\
	__global float4 *output,  	/* 0 */  \n\
        const unsigned int count,	/* 1 */  \n\
	__global float *mymat,   	/* 2 */  \n\
	__global float *my_vertex,	/* 3 */  \n\
	__global int *my_cindex, 	/* 4 */  \n\
	const int face_ccw,		/* 5 */ \n\
	const int face_flags,		/* 6 */  \n\
	const float avatar_radius,	/* 7 */ \n\
	const int ntri			/* 8 */ \n\
	) {   \n\
  \n\
	/* which index this instantation is working on */ \n\
	int i_am_canadian = get_global_id(0);  \n\
	if (i_am_canadian >= ntri) return; /* allows for workgroup size sizes */ \n\
 \n\
	/* vertices for this triangle */  \n\
	/* transformed by matrix */  \n\
	float4 tv1;  \n\
	float4 tv2;  \n\
	float4 tv3;  \n\
 \n\
	/* starting index in my_vertex of this vertex */  \n\
	/* we work in triangles; each triangle has 3 vertices */  \n\
	#define COORD_1 (my_cindex[i_am_canadian*3+0]*3) \n\
	#define COORD_2 (my_cindex[i_am_canadian*3+1]*3) \n\
	#define COORD_3 (my_cindex[i_am_canadian*3+2]*3) \n\
 \n\
	/* do matrix transform, 4 floats wide. */ \n\
	float4 matColumn1 = (float4)(mymat[0],mymat[1],mymat[2],0.0); \n\
	float4 matColumn2 = (float4)(mymat[4],mymat[5],mymat[6],0.0); \n\
	float4 matColumn3 = (float4)(mymat[8],mymat[9],mymat[10],0.0); \n\
	float4 matColumn4 = (float4)(mymat[12],mymat[13],mymat[14],0.0); \n\
 \n\
	/* first vertex */ \n\
	float4 Vertex_X = (float4)(my_vertex[COORD_1+0]); \n\
	float4 Vertex_Y = (float4)(my_vertex[COORD_1+1]); \n\
	float4 Vertex_Z = (float4)(my_vertex[COORD_1+2]); \n\
	tv1 = mad(matColumn1,Vertex_X,mad(matColumn2,Vertex_Y,mad(matColumn3,Vertex_Z,matColumn4))); \n\
 \n\
	/* second vertex */ \n\
	Vertex_X = (float4)(my_vertex[COORD_2+0]); \n\
	Vertex_Y = (float4)(my_vertex[COORD_2+1]); \n\
	Vertex_Z = (float4)(my_vertex[COORD_2+2]); \n\
	tv2 = mad(matColumn1,Vertex_X,mad(matColumn2,Vertex_Y,mad(matColumn3,Vertex_Z,matColumn4))); \n\
 \n\
	/* third vertex */ \n\
	Vertex_X = (float4)(my_vertex[COORD_3+0]); \n\
	Vertex_Y = (float4)(my_vertex[COORD_3+1]); \n\
	Vertex_Z = (float4)(my_vertex[COORD_3+2]); \n\
	tv3 = mad(matColumn1,Vertex_X,mad(matColumn2,Vertex_Y,mad(matColumn3,Vertex_Z,matColumn4))); \n\
 \n\
 \n\
	/* calculate normal for face from transformed vertices */  \n\
	/* this replicates polynormalf for opencl */ \n\
	#define VEC_DIST_1 (tv2-tv1) \n\
	#define VEC_DIST_2 (tv3-tv1) \n\
	float4 norm = normalize(cross(VEC_DIST_1,VEC_DIST_2));  \n\
 \n\
	/* from polyrep_disp_rec2, see that function for full comments */  \n\
	bool frontfacing;  \n\
 \n\
	/* how we view it from the avatar */  \n\
	if (face_ccw) frontfacing = (dot(norm,tv1) < 0);   \n\
	else frontfacing = (dot(norm,tv1) >= 0);  \n\
 \n\
	/* now, is solid false, or ccw or ccw winded triangle? */  \n\
	/* if we should do this triangle, the if statement is true */  \n\
 \n\
	bool should_do_this_triangle =  \n\
	((frontfacing && !(face_flags & PR_DOUBLESIDED) )  \n\
		|| ( (face_flags & PR_DOUBLESIDED)  && !(face_flags & (PR_FRONTFACING | PR_BACKFACING) )  )  \n\
		|| (frontfacing && (face_flags & PR_FRONTFACING))  \n\
		|| (!frontfacing && (face_flags & PR_BACKFACING))  ); \n\
 \n\
 \n\
	if (!should_do_this_triangle) { \n\
		output[i_am_canadian] = (float4)(0.0,0.0,0.0,0.0); \n\
		return; \n\
	} \n\
 \n\
 \n\
	/* if we are down to here, we have to do this triangle */ \n\
 \n\
	if(!frontfacing) { /*can only be here in DoubleSided mode*/  \n\
		/*reverse polygon orientation, and do calculations*/  \n\
		norm = VECSCALE(norm,-1.0); \n\
	} \n\
 \n\
	/********************************************************************************/ \n\
	/*										*/ \n\
	/* Collide Kernel Step 2: do hit calculations					*/ \n\
	/* replicate Dougs get_poly_min_disp_with_sphere function 			*/  \n\
	/*										*/ \n\
	/********************************************************************************/ \n\
 \n\
	float4 closest_point = closest_point_on_plane(tv1,tv2,tv3); \n\
 \n\
	float get_poly_mindisp = dot(closest_point,closest_point); \n\
	 \n\
	if (get_poly_mindisp > (avatar_radius * avatar_radius)) { \n\
		output[i_am_canadian] = (float4)(0.0,0.0,0.0,0.0); \n\
		return; \n\
	} \n\
 \n\
	/* do we have a movement here? */ \n\
	if (VECLENGTH(closest_point) > FLOAT_TOLERANCE) { \n\
		float poly_min_rt = sqrt(get_poly_mindisp); \n\
		float sFactor = (avatar_radius -poly_min_rt) /VECLENGTH(closest_point); \n\
 \n\
		float4 result = VECSCALE(closest_point,sFactor); \n\
		/* copy over the result */ \n\
		result.w = 100.0; /* flag that this is a good one */ \n\
		output[i_am_canadian] = result; \n\
		return; \n\
	} \n\
 \n\
 \n\
	/* if we are down to here, we can just return zero */ \n\
	output[i_am_canadian] = (float4)(0.0,0.0,0.0,0.0); \n\
} \n\
";

#endif //DO_COLLISION_GPU
