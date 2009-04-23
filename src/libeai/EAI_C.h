#ifndef __EAI_C_HEADERS__
#define __EAI_C_HEADERS__
#include <pthread.h>
#include <stdint.h>
#include <stddef.h>
#include <libFreeWRL.h>
#include <display.h>
#include <internal.h>


#include <sys/types.h>
#include "../lib/input/EAIheaders.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifndef REWIRE
#include <strings.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#ifndef REWIRE
#include <GL/gl.h>
#include <GL/glu.h>


#include "../lib/vrml_parser/Structs.h"
#include "../lib/main/headers.h"
#include "../lib/vrml_parser/CFieldDecls.h"
#include "../lib/vrml_parser/CParseGeneral.h"
#include "../lib/scenegraph/Vector.h"
#include "../lib/world_script/CScripts.h"
#include "../lib/vrml_parser/CParseParser.h"
#include "../lib/vrml_parser/CParseLexer.h"
#else
#include "GeneratedHeaders.h"
typedef size_t indexT;
#define ARR_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

extern const char *FIELDTYPES[];
extern const indexT FIELDTYPES_COUNT;
struct Uni_String {
        int len;
        char * strptr;
        int touched;
};

/*cstruct*/
struct Multi_Float { int n; float  *p; };
struct Multi_Double { int n; double* p; };
struct SFVec2d { double c[2]; };

struct SFRotation {
        float r[4]; };
struct Multi_Rotation { int n; struct SFRotation  *p; };

struct Multi_Vec3f { int n; struct SFColor  *p; };
/*cstruct*/
struct Multi_Bool { int n; int  *p; };
/*cstruct*/
struct Multi_Int32 { int n; int  *p; };

struct Multi_Node { int n; void * *p; };
struct SFColor {
        float c[3]; };
struct Multi_Color { int n; struct SFColor  *p; };
struct SFColorRGBA { float r[4]; };
struct Multi_ColorRGBA { int n; struct SFColorRGBA  *p; };
/*cstruct*/
struct Multi_Time { int n; double  *p; };
/*cstruct*/
struct Multi_String { int n; struct Uni_String * *p; };
struct SFVec2f {
        float c[2]; };
struct Multi_Vec2f { int n; struct SFVec2f  *p; };
struct Multi_Vec2d { int n; struct SFVec2d  *p; };
struct Multi_Vec3d { int n; struct SFVec3d  *p; };
struct SFVec3d { double c[3]; };
struct SFMatrix3f { float c[9]; };
struct Multi_Matrix3f { int n; struct SFMatrix3f  *p; };
struct SFVec4d { double c[4]; };
struct Multi_Vec4d { int n; struct SFVec4d  *p; };
struct SFMatrix3d { double c[9]; };
struct Multi_Matrix3d { int n; struct SFMatrix3d  *p; };
struct SFMatrix4f { float c[16]; };
struct Multi_Matrix4f { int n; struct SFMatrix4f  *p; };
struct SFMatrix4d { double c[16]; };
struct Multi_Matrix4d { int n; struct SFMatrix4d  *p; };
/* Routing - complex types */
#define ROUTING_SFNODE          -23
#define ROUTING_MFNODE          -10
#define ROUTING_SFIMAGE         -12
#define ROUTING_MFSTRING        -13
#define ROUTING_MFFLOAT        -14
#define ROUTING_MFROTATION      -15
#define ROUTING_MFINT32         -16
#define ROUTING_MFCOLOR         -17
#define ROUTING_MFVEC2F         -18
#define ROUTING_MFVEC3F         -19
#define ROUTING_MFVEC3D         -20
#define ROUTING_MFDOUBLE        -21
#define ROUTING_SFSTRING        -22
#define ROUTING_MFMATRIX4F      -30
#define ROUTING_MFMATRIX4D      -31
#define ROUTING_MFVEC2D         -32
#define ROUTING_MFVEC4F         -33
#define ROUTING_MFVEC4D         -34
#define ROUTING_MFMATRIX3F      -35
#define ROUTING_MFMATRIX3D      -36

typedef int     vrmlBoolT;
typedef struct SFColor  vrmlColorT;
typedef struct SFColorRGBA      vrmlColorRGBAT;
typedef float   vrmlFloatT;
typedef int32_t vrmlInt32T;
typedef struct Multi_Int32      vrmlImageT;
typedef struct X3D_Node*        vrmlNodeT;
typedef struct SFRotation       vrmlRotationT;
typedef struct Uni_String*      vrmlStringT;
typedef double  vrmlTimeT;
typedef double  vrmlDoubleT;
typedef struct SFVec2f  vrmlVec2fT;
typedef struct SFVec2d  vrmlVec2dT;
typedef struct SFVec4f  vrmlVec4fT;
typedef struct SFVec4d  vrmlVec4dT;
typedef struct SFColor  vrmlVec3fT;
typedef struct SFVec3d  vrmlVec3dT;
typedef struct SFMatrix3f       vrmlMatrix3fT;
typedef struct SFMatrix3d vrmlMatrix3dT;
typedef struct SFMatrix4f       vrmlMatrix4fT;
typedef struct SFMatrix4d vrmlMatrix4dT;

#endif
#include "X3DNode.h"
#endif
