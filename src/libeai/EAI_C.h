#ifndef __EAI_C_HEADERS__
#define __EAI_C_HEADERS__
#include <sys/types.h>
#include <stdint.h>
#include <EAIheaders.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#include <stdint.h>
#include <math.h>
#include <stddef.h>

#include <X3DNode.h>

#include <GeneratedHeaders.h>

/* copied from ../CFuncs/ */
typedef size_t indexT;
#define ARR_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
/* Table of built-in fieldIds */
extern const char *FIELDTYPES[];
extern const indexT FIELDTYPES_COUNT;


/* definitions to help scanning values in from a string */
#define SCANTONUMBER(value) while ((*value==' ') || (*value==',')) value++;
#define SCANTOSTRING(value) while ((*value==' ') || (*value==',')) value++;
#define SCANPASTFLOATNUMBER(value) while (isdigit(*value) \
		|| (*value == '.') || \
		(*value == 'E') || (*value == 'e') || (*value == '-')) value++;
#define SCANPASTINTNUMBER(value) if (isdigit(*value) || (*value == '-')) value++; \
		while (isdigit(*value) || \
		(*value == 'x') || (*value == 'X') ||\
		((*value >='a') && (*value <='f')) || \
		((*value >='A') && (*value <='F')) || \
		(*value == '-')) value++;

/*cstruct*/
struct Multi_Float { int n; float  *p; };
struct SFRotation { float r[4]; };
struct Multi_Rotation { int n; struct SFRotation  *p; };

struct Multi_Vec3f { int n; struct SFColor  *p; };
/*cstruct*/
struct Multi_Bool { int n; int  *p; };
/*cstruct*/
struct Multi_Int32 { int n; int  *p; };

struct Multi_Node { int n; void * *p; };
struct SFColor { float c[3]; };
struct Multi_Color { int n; struct SFColor  *p; };
struct SFColorRGBA { float r[4]; };
struct Multi_ColorRGBA { int n; struct SFColorRGBA  *p; };
/*cstruct*/
struct Multi_Time { int n; double  *p; };
/*cstruct*/
struct Multi_String { int n; struct Uni_String * *p; };
struct SFVec2f { float c[2]; };
struct Multi_Vec2f { int n; struct SFVec2f  *p; };
/*cstruct*/
/*cstruct*/
struct SFVec3d { double c[3]; };
struct Multi_Vec3d { int n; struct SFVec3d  *p; };
/*cstruct*/
struct Multi_Double { int n; double  *p; };
struct SFMatrix3f { float c[9]; };
struct Multi_Matrix3f { int n; struct SFMatrix3f  *p; };
struct SFMatrix3d { double c[9]; };
struct Multi_Matrix3d { int n; struct SFMatrix3d  *p; };
struct SFMatrix4f { float c[16]; };
struct Multi_Matrix4f { int n; struct SFMatrix4f  *p; };
struct SFMatrix4d { double c[16]; };
struct Multi_Matrix4d { int n; struct SFMatrix4d  *p; };
struct SFVec2d { double c[2]; };
struct Multi_Vec2d { int n; struct SFVec2d  *p; };
struct SFVec4f { float c[4]; };
struct Multi_Vec4f { int n; struct SFVec4f  *p; };
struct SFVec4d { double c[4]; };
struct Multi_Vec4d { int n; struct SFVec4d  *p; };

/*cstruct*/
/*cstruct*/

struct Uni_String {
        int len;
        char * strptr;
	int touched;
};


#define FREE_IF_NZ(a) if(a) {free(a); a = 0;}
#endif
