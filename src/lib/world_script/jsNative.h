/*
=INSERT_TEMPLATE_HERE=

$Id: jsNative.h,v 1.3 2009/02/11 15:12:55 istakenv Exp $

CProto.h - this is the object representing a PROTO definition and being
capable of instantiating it.
 
We keep a vector of pointers to all that pointers which point to "inner
memory" and need therefore be updated when copying.  Such pointers include
field-destinations and parts of ROUTEs.  Those pointers are then simply
copied, their new positions put in the new vector, and afterwards are all
pointers there updated.

*/

#ifndef __FREEWRL_JS_NATIVE_H__
#define __FREEWRL_JS_NATIVE_H__


typedef struct _BrowserNative {
	/* int magic; does this really do anything ??? */
	/* and, this really does nothing SV *sv_js; */
	int dummyEntry;
} BrowserNative;

typedef struct _SFNodeNative {
	int valueChanged;
	uintptr_t *handle;
	char *X3DString;
	int fieldsExpanded;
} SFNodeNative;

typedef struct _SFRotationNative {
	int valueChanged;
	struct SFRotation v;
} SFRotationNative;

typedef struct _SFVec2fNative {
	int valueChanged;
	struct SFVec2f v;
} SFVec2fNative;

typedef struct _SFVec3fNative {
	int valueChanged;
	struct SFColor v;
} SFVec3fNative;

typedef struct _SFVec3dNative {
	int valueChanged;
	struct SFVec3d v;
} SFVec3dNative;

typedef struct _SFImageNative {
	int valueChanged;
} SFImageNative;

typedef struct _SFColorNative {
	int valueChanged;
	struct SFColor v;
} SFColorNative;

typedef struct _SFColorRGBANative {
	int valueChanged;
	struct SFColorRGBA v;
} SFColorRGBANative;


/*
 * Adds additional (touchable) property to instance of a native
 * type.
 */
extern JSBool
addGlobalECMANativeProperty(void *cx,
							void *glob,
							char *name);

extern JSBool
addGlobalAssignProperty(void *cx,
						void *glob,
						char *name,
						char *str);

extern JSBool
addSFNodeProperty(void *cx,
				  void *glob,
				  char *nodeName,
				  char *name,
				  char *str);

extern void *
SFNodeNativeNew(void);

extern JSBool
SFNodeNativeAssign(void *top, void *fromp);

extern void *
SFRotationNativeNew(void);

extern void
SFRotationNativeAssign(void *top, void *fromp);

extern void
SFRotationNativeSet(void *p, struct Uni_String *sv);

extern void *
SFVec3fNativeNew(void);

extern void
SFVec3fNativeAssign(void *top, void *fromp);

extern void
SFVec3fNativeSet(void *p, struct Uni_String *sv);

extern void *
SFVec2fNativeNew(void);

extern void
SFVec2fNativeAssign(void *top, void *fromp);

extern void
SFVec2fNativeSet(void *p, struct Uni_String *sv);

extern void *
SFImageNativeNew(void);

extern void
SFImageNativeAssign(void *top, void *fromp);

extern void
SFImageNativeSet(void *p, struct Uni_String *sv);

extern void *
SFColorNativeNew(void);

extern void
SFColorNativeAssign(void *top, void *fromp);

extern void
SFColorNativeSet(void *p, struct Uni_String *sv);


#endif /* __FREEWRL_JS_NATIVE_H__ */
