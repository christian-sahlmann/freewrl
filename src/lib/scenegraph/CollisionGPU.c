/*
=INSERT_TEMPLATE_HERE=

$Id: CollisionGPU.c,v 1.11 2011/09/08 17:31:12 crc_canada Exp $

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


//#include <OpenGL/CGLDevice.h>
//#include <OpenCL/opencl.h>
// All OpenCL headers
#if defined (__APPLE__) || defined(MACOSX) || defined(TARGET_AQUA)
	#include <OpenCL/opencl.h>
	#include <OpenGL/CGLDevice.h>
#elif defined(_MSC_VER)
    #include <windows.h>  //WGL prototyped in wingdi.h
    #include <CL/opencl.h>
	#define DEBUG
#else  //LINUX
    #include <CL/opencl.h>
#endif 


#define FLOAT_TOLERANCE 0.00000001

/********************************************************************************/
/*										*/
/* 	Collide kernel, generic structures, etc					*/
/*										*/
/********************************************************************************/

struct OpenCLTransform
{
        cl_float16      matrix;
};

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
#ifdef _MSC_VER
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
int extraInitFromNvidiaSamples()
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
	device_id = cdDevices[uiTargetDevice];
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

static int triedAlready = 0;
bool init_GPU_collide(void) {
	int err;
	int gpu;
	if(triedAlready) return false;
	triedAlready = 1;
// get the current context.
// windows - IntPtr curDC = wglGetCurrentDC();
// then in the new compute context, we pass in the context

	/* initialized yet? */
	if (kernel != NULL) return false;


	// get the device id
	err = 0;
	if(0)
	{
		gpu=1;
		err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
		printf("clGetDeviceIDs err=%d\n",err);
	}
#if defined (TARGET_AQUA)
	CGLContextObj kCGLContext = CGLGetCurrentContext();
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
	cl_context_properties properties[] = {
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 0 };

	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
	if (err != CL_SUCCESS) {
		printCLError("clGetDeviceIDs",err);
		return FALSE;
	}

	context =clCreateContext(properties,0,0,clLogMessagesToStderrAPPLE,0,&err);

#endif /* AQUA */

#if defined (WIN32)

	if(1)
		err = extraInitFromNvidiaSamples();
	else
	{
		/* from OpenCL Programming Guide, pg 338 */
		cl_context_properties properties[] = {
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)cpPlatform,
			0};

		context = clCreateContext(properties, 1, &cdDevices[uiTargetDevice], NULL, NULL, &err);
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
	queue = clCreateCommandQueue(context, device_id, 0, &err);
	if (!queue || (err != CL_SUCCESS)) {
		printCLError("clCreateCommandQueue",err);
		return FALSE;
	}
	printf ("queue created\n");
 
	// create the compute program
	{
	size_t readSize;
#define RS 32768
	char *kp;
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

	program = clCreateProgramWithSource(context, 1, &kp, NULL, &err);
	if (!program || (err != CL_SUCCESS)) {
		printCLError("clCreateProgramWithSource",err);
		return FALSE;
	}
	printf ("program created\n");

	}


 
	// build the compute program executable
	//char *opts = "-Werror -cl-single-precision-constant -cl-nv-verbose  -g -cl-opt-disable -cl-strict-aliasing";
	//err = clBuildProgram(program, 0, NULL, opts, NULL, NULL);
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
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
#ifdef _MSC_VER
        memcpy(transform.matrix.s, modelMat, sizeof(cl_float16));
#else
        memcpy(transform.matrix, modelMat, sizeof(cl_float16));
#endif
        clEnqueueWriteBuffer(queue, matrix_buffer, CL_TRUE, 0, sizeof(struct OpenCLTransform), &transform, 0, NULL, NULL);

	// lets get the openGL vertex buffer here
	vertex_buffer=clCreateFromGLBuffer(context, CL_MEM_READ_ONLY, vertex_vbo, &err);
	if (err != CL_SUCCESS) {
		printCLError("clCreateFromGLBuffer",err);
		return maxdispv;
	}

	// and the coordinate index buffer
	index_buffer = clCreateFromGLBuffer(context, CL_MEM_READ_ONLY, index_vbo, &err);
	if (err != CL_SUCCESS) {
		printCLError("clCreateFromGLBuffer",err);
		return maxdispv;
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
		printCLError("clEnqueueNDRangeKernel",err);
		return maxdispv;
	}
	

	// wait for things to finish
	clFinish(queue);

	// get the data
	//err = clEnqueueReadBuffer (queue, output_buffer, CL_TRUE, 0, sizeof(struct SFVec3f) * ntri, collide_rvs.p, 0, NULL, NULL);
	err = clEnqueueReadBuffer (queue, output_buffer, CL_TRUE, 0, sizeof(struct SFVec3f) * GET_SFVEC3F_COUNT, collide_rvs.p, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printCLError("clEnqueueNDRangeKernel",err);
		return maxdispv;
	}

#ifdef SHADERS_2011
{
int i;
#ifdef DEBUG
printf ("\n**********\nshader output: ntri is %d but doing 19\n",ntri);
for (i=0; i<GET_SFVEC3F_COUNT; i++) {
	printf ("i %d val %f %f %f\n",i,
	collide_rvs.p[i].c[0],
	collide_rvs.p[i].c[1],
	collide_rvs.p[i].c[2]);
}
printf ("**********\n\n");

#endif
}
#endif


{ int i;


	for (i=0; i < GET_SFVEC3F_COUNT; i++) {
		/* XXX float to double conversion; make a vecdotf for speed */
		double disp;
		dispv.x = collide_rvs.p[i].c[0];
		dispv.y = collide_rvs.p[i].c[1];
		dispv.z = collide_rvs.p[i].c[2];
		// printf ("GPU tri %d, disp %f %f %f\n",i,dispv.x,dispv.y,dispv.z);

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

	
	// printf ("OpenCL - at end of opencl, maxdispv %f %f %f\n",maxdispv.x, maxdispv.y, maxdispv.z); 
	
}

	return maxdispv;
}

#endif //DO_COLLISION_GPU
