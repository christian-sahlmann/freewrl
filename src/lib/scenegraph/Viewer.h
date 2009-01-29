/*
=INSERT_TEMPLATE_HERE=

$Id: Viewer.h,v 1.4 2009/01/29 17:09:18 crc_canada Exp $

Viewer ???

*/

#ifndef __FREEX3D_VIEWER_H__
#define __FREEX3D_VIEWER_H__


#define NONE 0
#define EXAMINE 1
#define WALK 2
#define EXFLY 3
#define FLY 4

#define VIEWER_STRING(type) ( \
	type == NONE ? "NONE" : ( \
	type == EXAMINE ? "EXAMINE" : ( \
	type == WALK ? "WALK" : ( \
	type == EXFLY ? "EXFLY" : ( \
	type == FLY ? "FLY" : "UNKNOWN")))))

#define PRESS "PRESS"
#define PRESS_LEN 5

#define DRAG "DRAG"
#define DRAG_LEN 4

#define RELEASE "RELEASE"
#define RELEASE_LEN 7

#define KEYS_HANDLED 12
/* my %actions = ( */
/* 	a => sub {$aadd[2] -= $_[0]}, */
/* 	z => sub {$aadd[2] += $_[0]}, */
/* 	j => sub {$aadd[0] -= $_[0]}, */
/* 	l => sub {$aadd[0] += $_[0]}, */
/* 	p => sub {$aadd[1] += $_[0]}, */
/* 	';' => sub {$aadd[1] -= $_[0]}, */

/* 	8 => sub {$radd[0] += $_[0]}, */
/* 	k => sub {$radd[0] -= $_[0]}, */
/* 	u => sub {$radd[1] -= $_[0]}, */
/* 	o => sub {$radd[1] += $_[0]}, */
/* 	7 => sub {$radd[2] -= $_[0]}, */
/* 	9 => sub {$radd[2] += $_[0]}, */
/* ); */
#define KEYMAP {{ 'a', 0 }, { 'z', 0 }, { 'j', 0 }, { 'l', 0 }, { 'p', 0 }, { ';', 0 }, { '8', 0 }, { 'k', 0 }, { 'u', 0 }, { 'o', 0 }, { '7', 0 }, { '9', 0 }}

#define COORD_SYS 3
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define STRING_SIZE 256

#define IN_FILE "/tmp/inpdev"
#define IN_FILE_BYTES 100
#define INPUT_LEN 9
#define INPUT_LEN_Z 8
#define X_OFFSET 8
#define Y_OFFSET 17
#define Z_OFFSET 0
#define QUAT_W_OFFSET 26
#define QUAT_X_OFFSET 35
#define QUAT_Y_OFFSET 44
#define QUAT_Z_OFFSET 53


/* extern struct point_XYZ ViewerPosition; */
/* extern struct orient ViewerOrientation; */


typedef struct viewer_walk {
	double SX;
	double SY;
	double XD;
	double YD;
	double ZD;
	double RD;
} X3D_Viewer_Walk;


typedef struct viewer_examine {
	struct point_XYZ Origin;
	Quaternion OQuat;
	Quaternion SQuat;
	double ODist;
	double SY;
} X3D_Viewer_Examine;

typedef struct key {
	char key;
	unsigned int hit;
} Key;


/* Modeled after Descent(tm) ;) */
typedef struct viewer_fly {
	double Velocity[COORD_SYS];
	double AVelocity[COORD_SYS];
	Key Down[KEYS_HANDLED];
	Key WasDown[KEYS_HANDLED];
	double lasttime;
} X3D_Viewer_Fly;


typedef struct viewer {
	struct point_XYZ Pos;
	struct point_XYZ AntiPos;
	struct point_XYZ currentPosInModel;
	Quaternion Quat;
	Quaternion AntiQuat;
	Quaternion bindTimeQuat;
	int headlight;
	double speed;
	double Dist;
	double eyehalf;
	double eyehalfangle;
	unsigned int buffer;
	int oktypes[6];		/* boolean for types being acceptable. */
	X3D_Viewer_Walk *walk;
	X3D_Viewer_Examine *examine;
	X3D_Viewer_Fly *fly;

	struct X3D_GeoViewpoint *GeoSpatialNode; /* NULL, unless we are a GeoViewpoint */
} X3D_Viewer;


void
viewer_init(X3D_Viewer *viewer,
			int type);

void
print_viewer();

unsigned int
get_buffer();

void
set_buffer( const unsigned int buffer);

int
get_headlight();

void
toggle_headlight();

int
use_keys(void);

void
set_eyehalf( const double eyehalf,
			const double eyehalfangle);

void
set_viewer_type(const int type);

void
resolve_pos(void);
void getViewpointExamineDistance(void);

void
xy2qua(Quaternion *ret,
	   const double x,
	   const double y);

void
viewer_togl( double fieldofview);


void handle(const int mev, const unsigned int button, const float x, const float y);

void
handle_walk( const int mev,
			const unsigned int button,
			const float x,
			const float y);

void
handle_examine(const int mev,
			   const unsigned int button,
			   const float x,
			   const float y);

void
handle_key(const char key);

void
handle_keyrelease (const char key);

void
handle_tick();

void
handle_tick_walk();

void
handle_tick_exfly();

void
handle_tick_fly();

void
set_action(char *key);

void
set_stereo_offset(unsigned int buffer,
				  const double eyehalf,
				  const double eyehalfangle,
				  double fieldofview);

void
increment_pos( struct point_XYZ *vec);

void
bind_viewpoint(struct X3D_Viewpoint *node);

void
bind_geoviewpoint(struct X3D_GeoViewpoint *node);

void viewer_calculate_speed(void);

extern X3D_Viewer Viewer; /* in VRMLC.pm */

void viewer_default(void);

extern float eyedist;
extern float screendist;

void getCurrentSpeed();

void viewer_getPosInModel(struct point_XYZ *rp);

#endif /* __FREEX3D_VIEWER_H__ */
