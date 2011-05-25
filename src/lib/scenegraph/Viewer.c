/*
=INSERT_TEMPLATE_HERE=

$Id: Viewer.c,v 1.66 2011/05/25 19:26:34 davejoubert Exp $

CProto ???

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
#include "../opengl/OpenGL_Utils.h"
#include "../main/headers.h"

#include "LinearAlgebra.h"
#include "quaternion.h"
#include "Viewer.h"


int doExamineModeDistanceCalculations = FALSE;
static int examineCounter = 5;

static int viewer_type = VIEWER_NONE;
int viewer_initialized = FALSE;
static X3D_Viewer_Walk viewer_walk = { 0, 0, 0, 0, 0, 0 };
static X3D_Viewer_Examine viewer_examine = { {0, 0, 0}, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0 };
static X3D_Viewer_Fly viewer_fly = { { 0, 0, 0 }, { 0, 0, 0 }, KEYMAP, KEYMAP, -1 };
//static X3D_Viewer_YawPitchZoom viewer_ypz = { { 0, 0, 0 },  {0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, 0.0f, 0.0f };
static X3D_Viewer_YawPitchZoom viewer_ypz = { {0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, 0.0f, 0.0f };

static int translate[COORD_SYS] = { 0, 0, 0 }, rotate[COORD_SYS] = { 0, 0, 0 };

static FILE *exfly_in_file;

struct point_XYZ VPvelocity;

double nearPlane=DEFAULT_NEARPLANE;                     /* near Clip plane - MAKE SURE that statusbar is not in front of this!! */
double farPlane=DEFAULT_FARPLANE;                       /* a good default value */
double backgroundPlane = DEFAULT_BACKGROUNDPLANE;	/* where Background and TextureBackground nodes go */
GLDOUBLE fieldofview=45.0;
GLDOUBLE fovZoom = 1.0;
double calculatedNearPlane = 0.0;
double calculatedFarPlane = 0.0;


void print_viewer(void);
unsigned int get_buffer(void);
int fwl_get_headlight(void);
void fwl_toggle_headlight(void);
static void handle_tick_walk(void);
static void handle_tick_fly(void);
static void handle_tick_exfly(void);

/* used for EAI calls to get the current speed. Not used for general calcs */
/* we DO NOT return as a float, as some gccs have trouble with this causing segfaults */
void getCurrentSpeed() {
	BrowserSpeed = BrowserFPS * (fabs(VPvelocity.x) + fabs(VPvelocity.y) + fabs(VPvelocity.z));
}

void viewer_default() {
	Quaternion q_i;

	fieldofview = 45.0;
	fovZoom = 1.0;

	VPvelocity.x = 0.0; VPvelocity.y = 0.0; VPvelocity.z = 0.0; 
	Viewer.Pos.x = 0; Viewer.Pos.y = 0; Viewer.Pos.z = 10;
	Viewer.currentPosInModel.x = 0; Viewer.currentPosInModel.y = 0; Viewer.currentPosInModel.z = 10;
	Viewer.AntiPos.x = 0; Viewer.AntiPos.y = 0; Viewer.AntiPos.z = 0;

	vrmlrot_to_quaternion (&Viewer.Quat,1.0,0.0,0.0,0.0);
	vrmlrot_to_quaternion (&Viewer.bindTimeQuat,1.0,0.0,0.0,0.0);
	vrmlrot_to_quaternion (&Viewer.prepVPQuat,0.0,1.0,0.0,3.14);
	vrmlrot_to_quaternion (&q_i,1.0,0.0,0.0,0.0);
	quaternion_inverse(&(Viewer.AntiQuat),&q_i);

	Viewer.headlight = TRUE;
	/* tell the menu buttons of the state of this headlight */
	setMenuButton_headlight(Viewer.headlight);
	Viewer.speed = 1.0;
	Viewer.Dist = 10.0;
	Viewer.walk = &viewer_walk;
	Viewer.examine = &viewer_examine;
	Viewer.fly = &viewer_fly;
	Viewer.ypz = &viewer_ypz;

	set_viewer_type(VIEWER_EXAMINE);

	//set_eyehalf( Viewer.eyedist/2.0,
	//	atan2(Viewer.eyedist/2.0,Viewer.screendist)*360.0/(2.0*3.1415926));

	/* assume we are not bound to a GeoViewpoint */
	Viewer.GeoSpatialNode = NULL;

//#ifndef AQUA
//	if (Viewer.shutterGlasses)
//	    setStereoBufferStyle(0);/* setXEventStereo();*/
//#endif
}

void viewer_init (X3D_Viewer *viewer, int type) {
	Quaternion q_i;

	/* what type are we? used for handle events below */
	viewer_type = type;

	/* if we are brand new, set up our defaults */
	if (!viewer_initialized) {
		viewer_initialized = TRUE;

		viewer->Pos.x = 0; viewer->Pos.y = 0; viewer->Pos.z = 10;
		viewer->currentPosInModel.x = 0; viewer->currentPosInModel.y = 0; viewer->currentPosInModel.z = 10;
		viewer->AntiPos.x = 0; viewer->AntiPos.y = 0; viewer->AntiPos.z = 0;


		vrmlrot_to_quaternion (&Viewer.Quat,1.0,0.0,0.0,0.0);
		vrmlrot_to_quaternion (&Viewer.bindTimeQuat,1.0,0.0,0.0,0.0);
		vrmlrot_to_quaternion (&Viewer.prepVPQuat,1.0,0.0,0.0,0.0);
		vrmlrot_to_quaternion (&q_i,1.0,0.0,0.0,0.0);
		quaternion_inverse(&(Viewer.AntiQuat),&q_i);

		viewer->headlight = TRUE;
		/* tell the menu buttons of the state of this headlight */
		setMenuButton_headlight(viewer->headlight);
		viewer->speed = 1.0;
		viewer->Dist = 10.0;
		viewer->walk = &viewer_walk;
		viewer->examine = &viewer_examine;
		viewer->fly = &viewer_fly;
		viewer->ypz = &viewer_ypz;

		/* SLERP code for moving between viewpoints */
		viewer->SLERPing = FALSE;
		viewer->startSLERPtime = 0.0;
		viewer->transitionType = 1; /* assume LINEAR */
		viewer->transitionTime = 1.0; /* assume 1 second */

		/* Orthographic projections */
		viewer->ortho = FALSE;
	}

	resolve_pos();
}


void
print_viewer()
{
	struct orient_XYZA ori;
	quaternion_to_vrmlrot(&(Viewer.Quat), &(ori.x),&(ori.y),&(ori.z), &(ori.a));
	printf("Viewer {\n\tPosition [ %.4g, %.4g, %.4g ]\n\tQuaternion [ %.4g, %.4g, %.4g, %.4g ]\n\tOrientation [ %.4g, %.4g, %.4g, %.4g ]\n}\n", (Viewer.Pos).x, (Viewer.Pos).y, (Viewer.Pos).z, (Viewer.Quat).w, (Viewer.Quat).x, (Viewer.Quat).y, (Viewer.Quat).z, ori.x, ori.y, ori.z, ori.a);

}

unsigned int
get_buffer()
{
	return(Viewer.buffer);
}

/*
void
set_buffer(const unsigned int buffer,int iside)
{
	Viewer.buffer = buffer;
	Viewer.iside = iside;
}
*/
int fwl_get_headlight() { 
	return(Viewer.headlight);
}

void fwl_toggle_headlight() {
	if (Viewer.headlight == TRUE) {
		Viewer.headlight = FALSE;
	} else {
		Viewer.headlight = TRUE;
	}
	/* tell the menu buttons of the state of this headlight */
	setMenuButton_headlight(Viewer.headlight);

}
void setNoCollision() {
	fwl_setp_collision(0);
	setMenuButton_collision(fwl_getp_collision());
}

int get_collision() { 
	return fwl_getp_collision();
}
void toggle_collision() {
	fwl_setp_collision(!fwl_getp_collision()); 
	setMenuButton_collision(fwl_getp_collision());
}


void set_eyehalf(const double eyehalf, const double eyehalfangle) {
	Viewer.eyehalf = eyehalf;
	Viewer.eyehalfangle = eyehalfangle;
	Viewer.isStereo = 1;
}

void fwl_set_viewer_type(const int type) {
	set_viewer_type(type);
}
void set_viewer_type(const int type) {

	/* set velocity array to zero again - used only for EAI */
	VPvelocity.x=0.0; VPvelocity.y=0.0; VPvelocity.z=0.0;

	/* can the currently bound viewer type handle this */
	/* if there is no bound viewer, just ignore (happens on initialization) */
	if (navi_tos != -1)
		if (Viewer.oktypes[type]==FALSE) {
			setMenuButton_navModes(viewer_type);
			return;
		}

	viewer_init(&Viewer,type);

	/* tell the window menu what we are */
	setMenuButton_navModes(viewer_type);

	switch(type) {
	case VIEWER_NONE:
	case VIEWER_EXAMINE:
	case VIEWER_WALK:
	case VIEWER_EXFLY:
	case VIEWER_YAWPITCHZOOM:
	case VIEWER_FLY:
		viewer_type = type;
		break;
	default:
		fprintf(stderr, "Viewer type %d is not supported. See Viewer.h.\n", type);
		viewer_type = VIEWER_NONE;
		break;
	}
}


int use_keys() {
	if (viewer_type == VIEWER_FLY) {
		return TRUE;
	}
	return FALSE;
}


void resolve_pos() {
	/* my($this) = @_; */
	struct point_XYZ rot, z_axis = { 0, 0, 1 };
	Quaternion q_inv;
	double dist = 0;
	X3D_Viewer_Examine *examine = Viewer.examine;


	if (viewer_type == VIEWER_EXAMINE) {
		/* my $z = $this->{Quat}->invert->rotate([0,0,1]); */
		quaternion_inverse(&q_inv, &(Viewer.Quat));
		quaternion_rotation(&rot, &q_inv, &z_axis);

		/* my $d = 0; for(0..2) {$d += $this->{Pos}[$_] * $z->[$_]} */
		dist = VECPT(Viewer.Pos, rot);

		/* $this->{Origin} = [ map {$this->{Pos}[$_] - $d * $z->[$_]} 0..2 ]; */
/*
printf ("RP, before orig calc %4.3f %4.3f %4.3f\n",examine->Origin.x, examine->Origin.y,examine->Origin.z);
*/
		(examine->Origin).x = (Viewer.Pos).x - Viewer.Dist * rot.x;
		(examine->Origin).y = (Viewer.Pos).y - Viewer.Dist * rot.y;
		(examine->Origin).z = (Viewer.Pos).z - Viewer.Dist * rot.z;
/*
printf ("RP, aft orig calc %4.3f %4.3f %4.3f\n",examine->Origin.x, examine->Origin.y,examine->Origin.z);
*/
	}
}
double vecangle2(struct point_XYZ* V1, struct point_XYZ* V2, struct point_XYZ* rotaxis) {
	/* similar full circle angle computation as:
	double matrotate2v() 
	*/

	double cosine, sine, ulen, vlen, scale, dot, angle;
	struct point_XYZ cross;
	/* use dot product to get cosine:  cosTheta = (U dot V)/(||u||||v||) */
	dot = vecdot(V1,V2);
	ulen = sqrt(vecdot(V1,V1));
	vlen = sqrt(vecdot(V2,V2));
	scale = ulen*vlen;
	if( APPROX(scale, 0.0) )
	{
		rotaxis->y = rotaxis->z = 0.0;
		rotaxis->x = 1.0; //arbitrary axis
		return 0.0;
	}
	cosine = dot/scale;
	/* use cross product to get sine: ||u X v|| = ||u||||v||sin(theta) or sinTheta = ||uXv||/(||u||||v||)*/
	veccross(&cross,*V1,*V2);
	sine = sqrt(vecdot(&cross,&cross))/scale;
	/* get full circle unambiguous angle using both cosine and sine */
	angle = atan2(sine,cosine);
	vecnormal(rotaxis,&cross);
	return angle;
}
void avatar2BoundViewpointVerticalAvatar(GLDOUBLE *matA2BVVA, GLDOUBLE *matBVVA2A)
{
	/* goal: make 2 transform matrices to go back and forth from Avatar A to 
	   Bound-Viewpoint-Vertical aligned Avatar-centric (no translations or scales - just 2 tilts) coordinates
    */
	struct point_XYZ tilted;
	struct point_XYZ downvec = {0.0,-1.0,0.0};

	//downvec is in bound viewpoint space
	quaternion_rotation(&tilted, &Viewer.Quat, &downvec);
	//tilted is in avatar space.
	matrotate2v(matA2BVVA,downvec,tilted); 
	matrotate2v(matBVVA2A,tilted,downvec); 
	//matinverse(matBVVA2A,matA2BVVA);
	return;
}

void viewer_level_to_bound() 
{
/* 
Goal: Gravity as per specs 
Gravity:
	From specs > abstract > architecture > 23.3.4 NavigationInfo:
	"The speed, avatarSize and visibilityLimit values are all scaled by the transformation being applied 
	to the currently bound Viewpoint node. 
	If there is no currently bound Viewpoint node, the values are interpreted in the world coordinate system. "

	"For purposes of terrain following, the browser maintains a notion of the down direction (down vector), since gravity 
	is applied in the direction of the down vector. This down vector shall be along the negative Y-axis in the 
	local coordinate system of the currently bound Viewpoint node (i.e., the accumulation of the Viewpoint node's 
	ancestors' transformations, not including the Viewpoint node's orientation field)."

	From specs > abstract > architecture > 23.3.5 Viewpoint
	"When a Viewpoint node is at the top of the stack, the user's view is 
	conceptually re-parented as a child of the Viewpoint node."
	
	"Navigation types (see 23.3.4 NavigationInfo) that require a definition of a down vector (e.g., terrain following) 
	shall use the negative Y-axis of the coordinate system of the currently bound Viewpoint node. 
	Likewise, navigation types that require a definition of an up vector shall use the positive Y-axis of the 
	coordinate system of the currently bound Viewpoint node. The orientation field of the Viewpoint node does 
	not affect the definition of the down or up vectors. This allows the author to separate the viewing direction 
	from the gravity direction."

	Implication: if your entire scene is tilted (ie. Z up), along with your viewpoint, you shouldn't notice. 
	Even when terrain following, stepping, colliding.

Transforms:
World > [TransformStack] > Bound-Viewpoint > [Quat + Pos] > viewer/avatar > [AntiQuat + AntiPos?] > Bound-Viewpoint > Inverse[TransformStack] > World   
Viewer.Quat, Viewer.Pos - local pose of avatar wrt its currently bound viewpoint parent. 
	Includes/contains the viewpoint node's position and orientation field info.

ViewerUpVector: - looks like a global tilt of the avatar - I don't use it here or in collision
ViewerUpVector computation - see RenderFuncs.c L595   
*/

	/*
	first attempts at leveling avatar to bound viewpoint:
	1. Transform a bound-viewpoint-coordinates down vector {0,-1,0} to avatar coords using Quat
	2. compute tilts of that down vector in avatar space
	3. apply inverse tilts to end of transform chain ie Quat = Quat*inverse(tilts)
	*/
	struct point_XYZ rotaxis, tilted;
	Quaternion q, Quat; //, AntiQuat;
	double angle;
	struct point_XYZ downvec = {0.0,-1.0,0.0};

	Quat = Viewer.Quat;
	//AntiQuat = Viewer.AntiQuat;
	quaternion_rotation(&tilted, &Quat, &downvec);
	//tilted is in avatar space.
	angle = vecangle2(&downvec,&tilted,&rotaxis);
	if( APPROX(angle,0.0) ) return; //we're level already
	vrmlrot_to_quaternion(&q, rotaxis.x, rotaxis.y, rotaxis.z, -angle );
	quaternion_normalize(&q);
	quaternion_multiply(&(Viewer.Quat), &q, &Quat);

	/* make sure Viewer.Dist is configured properly for Examine mode */
	CALCULATE_EXAMINE_DISTANCE
}

void viewer_togl(double fieldofview) 
{
	/* goal: take the curring Viewer pose (.Pos, .Quat) and set openGL transforms
	   to prepare for a separate call to move the viewpoint - 
	   (currently done in Mainloop.c setup_viewpoint())
    */
	if (Viewer.isStereo) /* buffer != GL_BACK)  */
		set_stereo_offset0(); /*Viewer.iside, Viewer.eyehalf, Viewer.eyehalfangle);*/

	if (Viewer.SLERPing) {
		double tickFrac;
		Quaternion slerpedDiff;
		struct point_XYZ pos, antipos;

/*
printf ("SLERPing...\n");
printf ("\t	startSlerpPos %lf %lf %lf\n",Viewer.startSLERPPos.x,Viewer.startSLERPPos.y,Viewer.startSLERPPos.z);
printf ("\t	Pos           %lf %lf %lf\n",Viewer.Pos.x,Viewer.Pos.y,Viewer.Pos.z);
printf ("\t	startSlerpAntiPos %lf %lf %lf\n",Viewer.startSLERPAntiPos.x,Viewer.startSLERPAntiPos.y,Viewer.startSLERPAntiPos.z);
printf ("\t	AntiPos           %lf %lf %lf\n",Viewer.AntiPos.x,Viewer.AntiPos.y,Viewer.AntiPos.z);
*/

		/* printf ("slerping in togl, type %s\n", VIEWER_STRING(viewer_type)); */
		tickFrac = (TickTime - Viewer.startSLERPtime)/Viewer.transitionTime;
		//tickFrac = tickFrac/4.0;
		//printf ("tick frac %lf\n",tickFrac); 

		pos.x = Viewer.Pos.x * tickFrac + (Viewer.startSLERPPos.x * (1.0 - tickFrac));
		pos.y = Viewer.Pos.y * tickFrac + (Viewer.startSLERPPos.y * (1.0 - tickFrac));
		pos.z = Viewer.Pos.z * tickFrac + (Viewer.startSLERPPos.z * (1.0 - tickFrac));
		antipos.x = Viewer.AntiPos.x * tickFrac + (Viewer.startSLERPAntiPos.x * (1.0 - tickFrac));
		antipos.y = Viewer.AntiPos.y * tickFrac + (Viewer.startSLERPAntiPos.y * (1.0 - tickFrac));
		antipos.z = Viewer.AntiPos.z * tickFrac + (Viewer.startSLERPAntiPos.z * (1.0 - tickFrac));

		quaternion_slerp (&slerpedDiff,&Viewer.startSLERPQuat,&Viewer.Quat,tickFrac);

		quaternion_togl(&slerpedDiff);
		FW_GL_TRANSLATE_D(-pos.x, -pos.y, -pos.z);
		FW_GL_TRANSLATE_D(antipos.x, antipos.y, antipos.z);
		quaternion_slerp (&slerpedDiff,&Viewer.startSLERPAntiQuat,&Viewer.AntiQuat,tickFrac);
		quaternion_togl(&slerpedDiff);


		if (tickFrac >= 1.0) Viewer.SLERPing = FALSE;
	} else {
		quaternion_togl(&Viewer.Quat);
		FW_GL_TRANSLATE_D(-(Viewer.Pos).x, -(Viewer.Pos).y, -(Viewer.Pos).z);
		FW_GL_TRANSLATE_D((Viewer.AntiPos).x, (Viewer.AntiPos).y, (Viewer.AntiPos).z);
		quaternion_togl(&Viewer.AntiQuat);

	}

	getCurrentPosInModel(TRUE);
}

/* go through the modelMatrix and see where we are. Notes:
	- this should ideally be done in prep_Viewpoint, but if there is NO viewpoint... at least
	  here, it gets called. (that is why the antipos is added in here)

	- for X3D Viewpoints, this one adds in the AntiPos; for GeoViewpoints, we do a get after
	  doing Geo transform and rotation that are integral with the GeoViewpoint node.
*/

void getCurrentPosInModel (int addInAntiPos) {
	struct point_XYZ rp;
	struct point_XYZ tmppt;

	GLDOUBLE modelMatrix[16];
	GLDOUBLE inverseMatrix[16];

	/* "Matrix Quaternion FAQ: 8.050
	Given the current ModelView matrix, how can I determine the object-space location of the camera?

   	The "camera" or viewpoint is at (0., 0., 0.) in eye space. When you
   	turn this into a vector [0 0 0 1] and multiply it by the inverse of
   	the ModelView matrix, the resulting vector is the object-space
   	location of the camera.

   	OpenGL doesn't let you inquire (through a glGet* routine) the
   	inverse of the ModelView matrix. You'll need to compute the inverse
   	with your own code." */


       FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);

/* printf ("togl, before inverse, %lf %lf %lf\n",modelMatrix[12],modelMatrix[13],modelMatrix[14]);
       printf ("Viewer end _togl modelview Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                modelMatrix[0],  modelMatrix[4],  modelMatrix[ 8],  modelMatrix[12],
                modelMatrix[1],  modelMatrix[5],  modelMatrix[ 9],  modelMatrix[13],
                modelMatrix[2],  modelMatrix[6],  modelMatrix[10],  modelMatrix[14],
                modelMatrix[3],  modelMatrix[7],  modelMatrix[11],  modelMatrix[15]);
*/


	matinverse(inverseMatrix,modelMatrix);

/*
printf ("togl, after inverse, %lf %lf %lf\n",inverseMatrix[12],inverseMatrix[13],inverseMatrix[14]);
       printf ("inverted modelview Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                inverseMatrix[0],  inverseMatrix[4],  inverseMatrix[ 8],  inverseMatrix[12],
                inverseMatrix[1],  inverseMatrix[5],  inverseMatrix[ 9],  inverseMatrix[13],
                inverseMatrix[2],  inverseMatrix[6],  inverseMatrix[10],  inverseMatrix[14],
                inverseMatrix[3],  inverseMatrix[7],  inverseMatrix[11],  inverseMatrix[15]);
*/


	tmppt.x = inverseMatrix[12];
	tmppt.y = inverseMatrix[13];
	tmppt.z = inverseMatrix[14];


	/* printf ("going to do rotation on %f %f %f\n",tmppt.x, tmppt.y, tmppt.z); */
	quaternion_rotation(&rp, &Viewer.bindTimeQuat, &tmppt);
	/* printf ("new inverseMatrix  after rotation %4.2f %4.2f %4.2f\n",rp.x, rp.y, rp.z); */

	if (addInAntiPos) {
		Viewer.currentPosInModel.x = Viewer.AntiPos.x + rp.x;
		Viewer.currentPosInModel.y = Viewer.AntiPos.y + rp.y;
		Viewer.currentPosInModel.z = Viewer.AntiPos.z + rp.z;
	} else {
		Viewer.currentPosInModel.x = rp.x;
		Viewer.currentPosInModel.y = rp.y;
		Viewer.currentPosInModel.z = rp.z;
	}

	
/* 	printf ("getCurrentPosInModel, so, our place in object-land is %4.2f %4.2f %4.2f\n",
		Viewer.currentPosInModel.x, Viewer.currentPosInModel.y, Viewer.currentPosInModel.z);
*/
}

double quadratic(double x,double a,double b,double c)
{
	/* y = a*x*x + b*x + c; */
	return x*x*a + x*b + c;
}
double xsign_quadratic(double x,double a,double b,double c)
{
	/* y = sign(x)*(a*abs(x)*abs(x) + b*abs(x) + c); */
	double xSign;
	//xSign = _copysign(1.0,x); _MSC_VER
	if(x < 0.0) xSign = -1.0; else xSign = 1.0;
	x = fabs(x);
	return xSign*quadratic(x,a,b,c);
}
static void handle_walk(const int mev, const unsigned int button, const float x, const float y) {
/*
 * walk.xd,zd are in a plane parallel to the scene/global horizon.
 * walk.yd is vertical in the global/scene
 * walk.rd is an angle in the global/scene horizontal plane (around vertical axis)
*/
	X3D_Viewer_Walk *walk = Viewer.walk;
	double frameRateAdjustment = 1.0;
	if( BrowserFPS > 0)
		frameRateAdjustment = 20.0 / BrowserFPS; /* lets say 20FPS is our speed benchmark for developing tuning parameters */
	else
		frameRateAdjustment = 1.0;
	

	if (mev == ButtonPress ) {
		walk->SY = y;
		walk->SX = x;
	} else if (mev == MotionNotify) {
		if (button == 1) {
			/* July31,2010 new quadratic speed: allows you slow speed with small mouse motions, or 
			   fast speeds with large mouse motions. The .05, 5.0 etc are tuning parameters - I tinkered / experimented
			   using the townsite scene http://dug9.users.sourceforge.net/townsite.zip
			   which has the default navigationInfo speed (1.0) and is to geographic scale in meters.
			   If the tuning params don't work for you please fix/iterate/re-tune/change back/put a switch
			   I find them amply speedy, maybe yaw a bit too fast 
			   dug9: button 1 ZD: .05 5.0 0.0  RD: .1 .5 0.0
				     button 3 XD: 5.0 10.0 0.0 YD: 5.0 10.0 0.0
			*/
			walk->ZD = xsign_quadratic(y - walk->SY,.05,5.0,0.0)*Viewer.speed * frameRateAdjustment;
			walk->RD = xsign_quadratic(x - walk->SX,0.1,0.5,0.0)*frameRateAdjustment;
			//walk->ZD = (y - walk->SY) * Viewer.speed;
			//walk->RD = (x - walk->SX) * 0.1;
		} else if (button == 3) {
			walk->XD =  xsign_quadratic(x - walk->SX,5.0,10.0,0.0)*Viewer.speed * frameRateAdjustment;
			walk->YD = -xsign_quadratic(y - walk->SY,5.0,10.0,0.0)*Viewer.speed * frameRateAdjustment;
			//walk->XD = (x - walk->SX) * Viewer.speed;
			//walk->YD = -(y - walk->SY) * Viewer.speed;
		}
	} else if (mev == ButtonRelease) {
		if (button == 1) {
			walk->ZD = 0;
			walk->RD = 0;
		} else if (button == 3) {
			walk->XD = 0;
			walk->YD = 0;
		}
	}
}

static double
  norm(const Quaternion *quat)
  {
        return(sqrt(
                                quat->w * quat->w +
                                quat->x * quat->x +
                                quat->y * quat->y +
                                quat->z * quat->z
                                ));
  }


void handle_examine(const int mev, const unsigned int button, float x, float y) {
	Quaternion q, q_i, arc;
	struct point_XYZ p = { 0, 0, 0};
	X3D_Viewer_Examine *examine = Viewer.examine;
	double squat_norm;

	p.z=Viewer.Dist;

	if (mev == ButtonPress) {
		if (button == 1) {
/*
			printf ("\n");
			printf ("bp, before SQ %4.3f %4.3f %4.3f %4.3f\n",examine->SQuat.x, examine->SQuat.y, examine->SQuat.z, examine->SQuat.w);
			printf ("bp, before OQ %4.3f %4.3f %4.3f %4.3f\n",examine->OQuat.x, examine->OQuat.y, examine->OQuat.z, examine->OQuat.w);
			printf ("bp, before Q %4.3f %4.3f %4.3f %4.3f\n",Viewer.Quat.x, Viewer.Quat.y, Viewer.Quat.z, Viewer.Quat.w);
			printf ("bp, before, pos %4.3f %4.3f %4.3f\n",Viewer.Pos.x, Viewer.Pos.y, Viewer.Pos.z);
			printf ("bp, before, aps %4.3f %4.3f %4.3f\n",Viewer.AntiPos.x, Viewer.AntiPos.y, Viewer.AntiPos.z);
*/
			xy2qua(&(examine->SQuat), x, y);
			quaternion_set(&(examine->OQuat), &(Viewer.Quat));
/*
			printf ("bp, after SQ %4.3f %4.3f %4.3f %4.3f\n",examine->SQuat.x, examine->SQuat.y, examine->SQuat.z, examine->SQuat.w);
			printf ("bp, after OQ %4.3f %4.3f %4.3f %4.3f\n",examine->OQuat.x, examine->OQuat.y, examine->OQuat.z, examine->OQuat.w);
			printf ("bp, after Q %4.3f %4.3f %4.3f %4.3f\n",Viewer.Quat.x, Viewer.Quat.y, Viewer.Quat.z, Viewer.Quat.w);
			printf ("bp, after, pos %4.3f %4.3f %4.3f\n",Viewer.Pos.x, Viewer.Pos.y, Viewer.Pos.z);
			printf ("bp, after, aps %4.3f %4.3f %4.3f\n",Viewer.AntiPos.x, Viewer.AntiPos.y, Viewer.AntiPos.z);
*/

		} else if (button == 3) {
			examine->SY = y;
			examine->ODist = Viewer.Dist;
		}
	} else if (mev == MotionNotify) {
		if (button == 1) {
			squat_norm = norm(&(examine->SQuat));
			/* we have missed the press */
			if (APPROX(squat_norm, 0)) {
				fprintf(stderr, "Viewer handle_examine: mouse event DRAG - missed press\n");
				/* 			$this->{SQuat} = $this->xy2qua($mx,$my); */
				xy2qua(&(examine->SQuat), x, y);
				/* 			$this->{OQuat} = $this->{Quat}; */
				quaternion_set(&(examine->OQuat), &(Viewer.Quat));
			} else {
				/* my $q = $this->xy2qua($mx,$my); */
				xy2qua(&q, x, y);
				/* my $arc = $q->multiply($this->{SQuat}->invert()); */
				quaternion_inverse(&q_i, &(examine->SQuat));
				quaternion_multiply(&arc, &q, &q_i);


				/* $this->{Quat} = $arc->multiply($this->{OQuat}); */
				quaternion_multiply(&(Viewer.Quat), &arc, &(examine->OQuat));
			}
		} else if (button == 3) {
			Viewer.Dist = examine->ODist * exp(examine->SY - y);

		}
 	}

	quaternion_inverse(&q_i, &(Viewer.Quat));
	quaternion_rotation(&(Viewer.Pos), &q_i, &p);
/*
	printf ("bp, after quat rotation, pos %4.3f %4.3f %4.3f\n",Viewer.Pos.x, Viewer.Pos.y, Viewer.Pos.z);
*/

	Viewer.Pos.x += (examine->Origin).x;
	Viewer.Pos.y += (examine->Origin).y;
	Viewer.Pos.z += (examine->Origin).z;
/*
printf ("examine->origin %4.3f %4.3f %4.3f\n",examine->Origin.x, examine->Origin.y, examine->Origin.z);
*/
}

void handle_yawpitchzoom(const int mev, const unsigned int button, float x, float y) {
	/* handle_examine almost works except we don't want roll-tilt, and we want to zoom */
	Quaternion qyaw, qpitch;
	X3D_Viewer_YawPitchZoom *ypz = Viewer.ypz;
	double dyaw,dpitch;
	/* unused double dzoom; */

	if (mev == ButtonPress) {
		if (button == 1) {
			ypz->ypz0[0] = ypz->ypz[0];
			ypz->ypz0[1] = ypz->ypz[1];
			ypz->x = x;
			ypz->y = y;
		} else if (button == 3) {
			ypz->x = x;
		}
	} else if (mev == MotionNotify) {
		if (button == 1) {
			dyaw   = (ypz->x - x) * fieldofview*PI/180.0*fovZoom * screenRatio; 
			dpitch = (ypz->y - y) * fieldofview*PI/180.0*fovZoom;
			ypz->ypz[0] = ypz->ypz0[0] + dyaw;
			ypz->ypz[1] = ypz->ypz0[1] + dpitch;
			vrmlrot_to_quaternion(&qyaw, 0.0, 1.0, 0.0, ypz->ypz[0]);
			vrmlrot_to_quaternion(&qpitch,1.0,0.0,0.0,ypz->ypz[1]);
			quaternion_multiply(&(Viewer.Quat), &qpitch, &qyaw);
		} else if (button == 3) {
			double d, fac;
			d = (x - ypz->x)*.25;
			if(d > 0.0)
				fac = ((d *  2.0) + (1.0 - d) * 1.0);
			else
			{
				d = fabs(d);
				fac = ((d * .5) + (1.0 - d) * 1.0);
			}
			fovZoom = fovZoom * fac;
			fovZoom = DOUBLE_MIN(2.0,DOUBLE_MAX(.125,fovZoom));  
		}
 	}
}

/************************************************************************************/

int getViewerType()
{
	return viewer_type;
}
void handle(const int mev, const unsigned int button, const float x, const float y)
{
	/* printf("Viewer handle: viewer_type %s, mouse event %d, button %u, x %f, y %f\n", 
	   VIEWER_STRING(viewer_type), mev, button, x, y); */

	if (button == 2) {
		return;
	}

	switch(viewer_type) {
	case VIEWER_NONE:
		break;
	case VIEWER_EXAMINE:
		handle_examine(mev, button, ((float) x), ((float) y));
		break;
	case VIEWER_WALK:
		handle_walk(mev, button, ((float) x), ((float) y));
		break;
	case VIEWER_EXFLY:
		break;
	case VIEWER_FLY:
		break;
	case VIEWER_YAWPITCHZOOM:
		handle_yawpitchzoom(mev,button,((float) x),((float)y));
	default:
		break;
	}
}


void
handle_key(const char key)
{
	X3D_Viewer_Fly *fly = Viewer.fly;
	char _key;
	int i;

	if (viewer_type == VIEWER_FLY) {
		/* $key = lc $key; */
		_key = (char) tolower((int) key);

		for (i = 0; i < KEYS_HANDLED; i++) {
			if ((fly->Down[i]).key  == _key) {
				/* $this->{Down}{$key} = 1; */
				(fly->Down[i]).hit = 1;
			}
		}
	}
}


void
handle_keyrelease(const char key)
{
	/* my($this,$time,$key) = @_; */
	X3D_Viewer_Fly *fly = Viewer.fly;
	char _key;
	int i;

	if (viewer_type == VIEWER_FLY) {
		/* $key = lc $key; */
		_key = (char) tolower((int) key);

		for (i = 0; i < KEYS_HANDLED; i++) {
			if ((fly->Down[i]).key  == _key) {
				/* $this->{WasDown}{$key} += $this->{Down}{$key}; */
				(fly->WasDown[i]).hit += (fly->Down[i]).hit;
				/* delete $this->{Down}{$key}; */
				(fly->Down[i]).hit = 0;
			}
		}
	}
}

/* wall penetration detection variables
   lastP - last avatar position, relative to current avatar position at 0,0,0 in avatar space
		- is a sum of walk_tick and collision displacement increment_pos()
   lastQ - quaternion increment from walk_tick which applies to previous lastP:
         if current frame number is i, and lastP is from i-1, then lastQ applies to i-1 lastP
*/
struct point_XYZ viewer_lastP;
void viewer_lastP_clear()
{
	viewer_lastP.x = viewer_lastP.y = viewer_lastP.z = 0.0;
}
void viewer_lastQ_set(Quaternion *lastQ)
{
	quaternion_rotation(&viewer_lastP,lastQ,&viewer_lastP); 
}
void viewer_lastP_add(struct point_XYZ *vec) 
{
	if(get_collision()) /* fw_params.collision use if(1) to test with toggling_collision */
	{
		VECADD(viewer_lastP,*vec);
	}
	else
		viewer_lastP_clear();
}

struct point_XYZ viewer_get_lastP()
{ 
	/* returns a vector from avatar to the last avatar location ie on the last loop, in avatar space */
	struct point_XYZ nv = viewer_lastP;
	vecscale(&nv,&nv,-1.0); 
	return nv; 
}

/*
 * handle_tick_walk: called once per frame.
 *
 * Sets viewer to next expected position.
 * This should be called before position sensor calculations
 * (and event triggering) take place.
 * Position dictated by this routine is NOT final, and is likely to
 * change if the viewer is left in a state of collision. (ncoder)
 * walk.xd,zd are in a plane parallel to the scene/global horizon.
 * walk.yd is vertical in the global/scene
 * walk.rd is an angle in the global/scene horizontal plane (around vertical axis)
 */

static void
handle_tick_walk()
{
	X3D_Viewer_Walk *walk = Viewer.walk;
	Quaternion q, nq;
	struct point_XYZ p;

	p.x = 0.15 * walk->XD;
	p.y = 0.15 * walk->YD;
	p.z = 0.15 * walk->ZD;
	q.w = (Viewer.Quat).w;
	q.x = (Viewer.Quat).x;
	q.y = (Viewer.Quat).y;
	q.z = (Viewer.Quat).z;
	nq.w = 1 - 0.2 * walk->RD;
	nq.x = 0.0l;
	nq.y = 0.2 * walk->RD;
	nq.z = 0.0;

	increment_pos(&p);

	quaternion_normalize(&nq);
	viewer_lastQ_set(&nq);
	quaternion_multiply(&(Viewer.Quat), &nq, &q);

	/* make sure Viewer.Dist is configured properly for Examine mode */
	CALCULATE_EXAMINE_DISTANCE
}


/* formerly package VRML::Viewer::ExFly
 * entered via the "f" key.
 *
 * External input for x,y,z and quat. Reads in file
 * /tmp/inpdev (macro IN_FILE), which is a single line file that is
 * updated by some external program.
 *
 * eg:
 *    9.67    -1.89    -1.00  0.99923 -0.00219  0.01459  0.03640
 *
 * Do nothing for the mouse.
 */

/* my $in_file = "/tmp/inpdev"; */
/* #JAS my $in_file_date = stat($in_file)->mtime; */
/* my $string = ""; */
/* my $inc = 0; */
/* my $inf = 0; */
#ifdef _MSC_VER
static int exflyMethod = 1;  /* could be a user settable option, which kind of exfly to do */
#else
static int exflyMethod = 0;
#endif
static void
handle_tick_exfly()
{
	size_t len = 0;
	char string[STRING_SIZE];
	float px,py,pz,q1,q2,q3,q4;
	size_t rv; /* unused, but here for compile warnings */

	memset(string, 0, STRING_SIZE * sizeof(char));

	/*
	 * my $chk_file_date = stat($in_file)->mtime;
	 * following uncommented as time on file only change
	 * once per second - should change this...
     *
	 * $in_file_date = $chk_file_date;
	 */

/* 	sysopen ($inf, $in_file, O_RDONLY) or  */
/* 		die "Error reading external sensor input file $in_file\n"; */
/* 	$inc = sysread ($inf, $string, 100); */
/* 	close $inf; */
	if ((exfly_in_file = fopen(IN_FILE, "r")) == NULL) {
		fprintf(stderr,
				"Viewer handle_tick_exfly: could not open %s for read, returning to EXAMINE mode.\nSee the FreeWRL man page for further details on the usage of Fly - External Sensor input mode.\n",
				IN_FILE);

		/* allow the user to continue in default Viewer mode */
		viewer_type = VIEWER_EXAMINE;
		setMenuButton_navModes(viewer_type);
		return;
	}
	rv = fread(string, sizeof(char), IN_FILE_BYTES, exfly_in_file);
	if (ferror(exfly_in_file)) {
		fprintf(stderr,
				"Viewer handle_tick_exfly: error reading from file %s.",
				IN_FILE);
		fclose(exfly_in_file);
		return;
	}
	fclose(exfly_in_file);

/* 	if (length($string)>0) */
	if ((len = strlen(string)) > 0) {
		if(exflyMethod == 0)
		{
			len = sscanf (string, "%f %f %f %f %f %f %f",&px,&py,&pz,
				&q1,&q2,&q3,&q4);

			/* read error? */
			if (len != 7) return;

			(Viewer.Pos).x = px;
			(Viewer.Pos).y = py;
			(Viewer.Pos).z = pz;

			(Viewer.Quat).w = q1;
			(Viewer.Quat).x = q2;
			(Viewer.Quat).y = q3;
			(Viewer.Quat).z = q4;
		}else if(exflyMethod == 1){
			static int lastbut = 0;
			int mev, but;
			len = sscanf (string, "%d %f %f ",&but,&px,&py);
			if (len != 3) return;
			mev = ButtonRelease;
			if(but) mev = MotionNotify;
			if(but != lastbut)
			{
				mev = (but==1 || but==4)? ButtonPress : ButtonRelease;
			}
			// change raw wii values from ( -1 to 1 ) to (0 - 1.0)
			//px = (px + 1.0)*.5;  //done in wiimote code
			//py = 1.0 - (py + 1.0)*.5;  //done in wiimote code
			handle_walk(mev,but,px,py);
			handle_tick_walk();
			lastbut = but;
		}
	}
}


void
set_action(char *key)
{
	switch(*key) {
	case 'a':
		translate[Z_AXIS] -= 1;
		break;
	case 'z':
		translate[Z_AXIS] += 1;
		break;
	case 'j':
		translate[X_AXIS] -= 1;
		break;
	case 'l':
		translate[X_AXIS] += 1;
		break;
	case 'p':
		translate[Y_AXIS] -= 1;
		break;
	case ';':
		translate[Y_AXIS] += 1;
		break;
	case '8':
		rotate[X_AXIS] += 1;
		break;
	case 'k':
		rotate[X_AXIS] -= 1;
		break;
	case 'u':
		rotate[Y_AXIS] -= 1;
		break;
	case 'o':
		rotate[Y_AXIS] += 1;
		break;
	case '7':
		rotate[Z_AXIS] -= 1;
		break;
	case '9':
		rotate[Z_AXIS] += 1;
		break;
	default:
		break;
	}
}

static void
handle_tick_fly()
{
	X3D_Viewer_Fly *fly = Viewer.fly;
	Key ps[KEYS_HANDLED] = KEYMAP;
	Quaternion q_v, nq = { 1, 0, 0, 0 };
	struct point_XYZ v;
	double changed = 0, time_diff = -1;
	int i;

	if (fly->lasttime < 0) {
		fly->lasttime = TickTime;
		return;
	} else {
		time_diff = TickTime - fly->lasttime;
		fly->lasttime = TickTime;
		if (APPROX(time_diff, 0)) {
			return;
		}
	}

	/* first, get all the keypresses since the last time */
	for (i = 0; i < KEYS_HANDLED; i++) {
		(ps[i]).hit += (fly->Down[i]).hit;
	}

	for (i = 0; i < KEYS_HANDLED; i++) {
		(ps[i]).hit += (fly->WasDown[i]).hit;
		(fly->WasDown[i]).hit = 0;
	} 

	memset(translate, 0, sizeof(int) * COORD_SYS);
	memset(rotate, 0, sizeof(int) * COORD_SYS);

	for (i = 0; i < KEYS_HANDLED; i++) {
		if ((ps[i]).hit) {
			set_action(&(ps[i]).key);
		}
	}

	/* has anything changed? if so, then re-render */

	/* linear movement */
	for (i = 0; i < COORD_SYS; i++) {
		fly->Velocity[i] *= pow(0.06, time_diff);

		fly->Velocity[i] += time_diff * translate[i] * 14.5 * Viewer.speed;
		changed += fly->Velocity[i];
		/* printf ("vel %d %f\n",i,fly->Velocity[i]); */
	}

	/* if we do NOT have a GeoViewpoint node, constrain all 3 axis */
	if (Viewer.GeoSpatialNode == NULL) 
		for (i = 0; i < COORD_SYS; i++) {
			if (fabs(fly->Velocity[i]) >9.0) fly->Velocity[i] /= (fabs(fly->Velocity[i]) /9.0);
		}

	/* angular movement */
	for (i = 0; i < COORD_SYS; i++) {
		fly->AVelocity[i] *= pow(0.04, time_diff);
		fly->AVelocity[i] += time_diff * rotate[i] * 0.025;

		if (fabs(fly->AVelocity[i]) > 0.8) {
			fly->AVelocity[i] /= (fabs(fly->AVelocity[i]) / 0.8);
		}
		changed += fly->AVelocity[i];
		/* printf ("avel %d %f\n",i,fly->AVelocity[i]); */
	}

	/* have we done anything here? */
	if (APPROX(changed,0.0)) return;

	v.x = fly->Velocity[0] * time_diff;
	v.y = fly->Velocity[1] * time_diff;
	v.z = fly->Velocity[2] * time_diff;
	increment_pos(&v);


	nq.x = fly->AVelocity[0];
	nq.y = fly->AVelocity[1];
	nq.z = fly->AVelocity[2];
	quaternion_normalize(&nq);

	quaternion_set(&q_v, &(Viewer.Quat));
	quaternion_multiply(&(Viewer.Quat), &nq, &q_v);

	/* make sure Viewer.Dist is configured properly for Examine mode */
	CALCULATE_EXAMINE_DISTANCE

}

void
handle_tick()
{
	switch(viewer_type) {
	case VIEWER_NONE:
		break;
	case VIEWER_EXAMINE:
		break;
	case VIEWER_WALK:
		handle_tick_walk();
		break;
	case VIEWER_EXFLY:
		handle_tick_exfly();
		break;
	case VIEWER_FLY:
		handle_tick_fly();
		break;
	case VIEWER_YAWPITCHZOOM:
		break;
	default:
		break;
	}

	if (doExamineModeDistanceCalculations) {
/*
		printf ("handle_tick - doing calculations\n");
*/
		CALCULATE_EXAMINE_DISTANCE
		resolve_pos();
		examineCounter --;

		if (examineCounter < 0) {
		doExamineModeDistanceCalculations = FALSE;
		examineCounter = 5;
		}
	}
}



/*
 * Semantics: given a viewpoint and orientation,
 * we take the center to revolve around to be the closest point to origin
 * on the z axis.
 * Changed Feb27 2003 JAS - by fixing $d to 10.0, we make the rotation
 * point to be 10 metres in front of the user.
 */

/* ArcCone from TriD */
void
xy2qua(Quaternion *ret, const double x, const double y)
{
	double _x = x - 0.5, _y = y - 0.5, _z, dist;
	_x *= 2;
	_y *= -2;

	dist = sqrt((_x * _x) + (_y * _y));

	if (dist > 1.0) {
		_x /= dist;
		_y /= dist;
		dist = 1.0;
	}
	_z = 1 - dist;

	ret->w = 0;
	ret->x = _x;
	ret->y = _y;
	ret->z = _z;
	quaternion_normalize(ret);
}



int initAnaglyphShaders()
{
	//p.642 red book
	GLint compiled, linked;
	GLuint shader, program;
	int retval,i;

	//one shader and program for each of Red, Green, Blue, Amber(Red+Green), Cyan(Green+Blue). Magenta(Red+Blue).
	const GLchar* shaderSrc_R[] = {
		"void main()"
		"{"
		"     float gray = dot(gl_Color.rgb, vec3(0.299, 0.587, 0.114));"
		"     gl_FragColor = vec4(gray, 0.0,0.0, gl_Color.a);"
		"}"
	};
	const GLchar* shaderSrc_G[] = {
		"void main()"
		"{"
		"     float gray = dot(gl_Color.rgb, vec3(0.299, 0.587, 0.114));"
		"     gl_FragColor = vec4(0.0,gray,0.0, gl_Color.a);"
		"}"
	};
	const GLchar* shaderSrc_B[] = {
		"void main()"
		"{"
		"     float gray = dot(gl_Color.rgb, vec3(0.299, 0.587, 0.114));"
		"     gl_FragColor = vec4(0.0,0.0,gray, gl_Color.a);"
		"}"
	};
	const GLchar* shaderSrc_A[] = {
		"void main()"
		"{"
		"     float gray = dot(gl_Color.rgb, vec3(0.299, 0.587, 0.114));"
		"     gl_FragColor = vec4(gray,gray,0.0, gl_Color.a);"
		"}"
	};
	const GLchar* shaderSrc_C[] = {
		"void main()"
		"{"
		"     float gray = dot(gl_Color.rgb, vec3(0.299, 0.587, 0.114));"
		"     gl_FragColor = vec4(0.0,gray,gray, gl_Color.a);"
		"}"
	};
	const GLchar* shaderSrc_M[] = {
		"void main()"
		"{"
		"     float gray = dot(gl_Color.rgb, vec3(0.299, 0.587, 0.114));"
		"     gl_FragColor = vec4(gray,0.0,gray, gl_Color.a);"
		"}"
	};

	const GLchar **src[6];
	if(!rdr_caps.have_GL_VERSION_2_0) return 0;
	src[0] = shaderSrc_R;
	src[1] = shaderSrc_G;
	src[2] = shaderSrc_B;
	src[3] = shaderSrc_A;
	src[4] = shaderSrc_C;
	src[5] = shaderSrc_M;
	retval = 1;
	for(i=0;i<6;i++)
	{
		shader = CREATE_SHADER(GL_FRAGMENT_SHADER);
		SHADER_SOURCE(shader,1,src[i],NULL);
		COMPILE_SHADER(shader);
		GET_SHADER_INFO(shader,GL_COMPILE_STATUS,&compiled);
		if(!compiled){
			GLint length;
			GLchar* log;
			GET_SHADER_INFO(shader,GL_INFO_LOG_LENGTH,&length);
			log = (GLchar*)malloc(length);
			glGetShaderInfoLog(shader,length,&length,log);
			fprintf(stderr,"compile log - '%s\n",log);
			retval = 0;
			break;
		}
		program = CREATE_PROGRAM;
		ATTACH_SHADER(program,shader);
		LINK_SHADER(program);

		glGetProgramiv(program,GL_LINK_STATUS,&linked);

		if(linked){
			Viewer.shaders[i] = shader;
			Viewer.programs[i] = program;
			retval = retval && 1;
		}else{
			GLint length;
			GLchar* log;
			glGetProgramiv(program,GL_INFO_LOG_LENGTH,&length);
			log = (GLchar*)malloc(length);
			glGetProgramInfoLog(program,length,&length,log);
			fprintf(stderr,"link log = '%s'\n",log);
			retval = 0;
			break;
		}
	}
	return retval;
}


int StereoInitializedOnce = 0;
void fwl_init_StereoDefaults()
{
	/* must call this before getting values from command line in options.c */
	Viewer.shutterGlasses = 0;
	Viewer.anaglyph = 0;
	Viewer.anaglyphMethod = 2; /* 1= use shaders 2= draw gray .It's hardwired here, no way to set from command line or HUD*/
	Viewer.sidebyside = 0;
	Viewer.isStereo = 0;
	if(!StereoInitializedOnce)
	{
		Viewer.eyedist = 0.06;
		Viewer.screendist = 0.8;
		Viewer.stereoParameter = 0.4;
		Viewer.dominantEye = 1; /*0=Left 1=Right used for picking*/
		Viewer.haveAnaglyphShader = 0; /* call after gl initialized initAnaglyphShaders(); */
		Viewer.iprog[0] = 0; /* left red */
		Viewer.iprog[1] = 1; /* right green */
		Viewer.haveQuadbuffer = 0;
		StereoInitializedOnce = 1;
	}
}


void deleteAnaglyphShaders()
{
	int i;
	if(!rdr_caps.have_GL_VERSION_2_0) return;
	for(i=0;i<6;i++)
	{
		DELETE_SHADER(Viewer.shaders[i]);
		DELETE_PROGRAM(Viewer.programs[i]);
	}
}
static GLboolean acMask[2][3]; //anaglyphChannelMask
void setmask(GLboolean *mask,int r, int g, int b)
{
	mask[0] = (GLboolean)r;
	mask[1] = (GLboolean)g;
	mask[2] = (GLboolean)b;
}
void Viewer_anaglyph_setSide(int iside)
{
	if( Viewer.anaglyphMethod == 2 )
	{
		/* draw in gray */
		/* and use channel masks */
		GLboolean t = 1;
		glColorMask(acMask[iside][0],acMask[iside][1],acMask[iside][2],t);
	}
	else if(Viewer.anaglyphMethod == 1)
	{
		/* use shaders. textures (images) don't render */
		USE_SHADER(Viewer.programs[Viewer.iprog[iside]]);
	}
}

static char * RGBACM = "RGBACM";
static int indexRGBACM(int a)
{
	return (int) (strchr(RGBACM,a)-RGBACM);
}
void setAnaglyphSideColor(char val, int iside)
{
	Viewer.iprog[iside] = indexRGBACM(val);
	if(Viewer.iprog[iside] == -1 )
	{
		printf ("warning, command line anaglyph parameter incorrect - was %c need something like RG\n",val);
		Viewer.iprog[iside] = iside;
	}
	/* used for anaglyphMethod==2 */
	switch (Viewer.iprog[iside]) {
		case 0: //'R':
		   setmask(acMask[iside],1,0,0);
		   break;
		case 1: //'G':
		   setmask(acMask[iside],0,1,0);
			break;
		case 2: //'B':
		   setmask(acMask[iside],0,0,1);
		  break;
		case 3: //'A':
		   setmask(acMask[iside],1,1,0);
		  break;
		case 4: //'C':
		   setmask(acMask[iside],0,1,1);
		  break;
		case 5://'M':
		   setmask(acMask[iside],1,0,1);
		  break;
	}
}
void fwl_set_AnaglyphParameter(const char *optArg) {
/*
  NOTE: "const char" means that you wont modify it in the function :)
 */
	const char* glasses;
	glasses = optArg;
	if(strlen(glasses)!=2)
	{
	  printf ("warning, command line anaglyph parameter incorrect - was %s need something like RC\n",optArg);
	  glasses ="RC";
	}
	setAnaglyphSideColor(glasses[0],0);
	setAnaglyphSideColor(glasses[1],1);
	//Viewer.iprog[0] = indexRGBACM(glasses[0]);
	//Viewer.iprog[1] = indexRGBACM(glasses[1]);
	//if(Viewer.iprog[0] == -1 || Viewer.iprog[1] == -1)
	//{
	//	printf ("warning, command line anaglyph parameter incorrect - was %s need something like RG\n",optArg);
	//	Viewer.iprog[0] = 0;
	//	Viewer.iprog[1] = 1;
	//}
	Viewer.anaglyph = 1; /*0=none 1=active */
	Viewer.shutterGlasses = 0;
	Viewer.sidebyside = 0;
	//Viewer.haveAnaglyphShader = 1; do not set until openGL initialized
	Viewer.isStereo = 1;
	setStereoBufferStyle(1);
}
int usingAnaglyph2()
{
	return (Viewer.anaglyph && (Viewer.anaglyphMethod == 2)) ? 2 : 0;
}
/* shutter glasses, stereo view  from Mufti@rus */
/* handle setting shutter from parameters */
void fwl_init_Shutter (void)
{
	/* if you put --shutter on the command line, you'll come in here twice: 
	  first: from options.c but haveQuadbuffer will == 0 because we haven't init gl yet, so don't know
	  second: post_gl_init - we'll know haveQuadbuffer which might = 1 (if not it goes into flutter mode)
    */

	shutterGlasses = 2;
	Viewer.shutterGlasses = 2;
	setStereoBufferStyle(1); 
	if(Viewer.haveQuadbuffer)
	{
		shutterGlasses = 1; /* platform specific pixelformat/window initialization code should hint PRF_STEREO */
		Viewer.shutterGlasses = 1;
		setStereoBufferStyle(0); 
	}
	Viewer.isStereo = 1;

}

void fwl_init_SideBySide()
{
	setStereoBufferStyle(1); 
	Viewer.isStereo = 1;
	Viewer.sidebyside = 1;
}
void setAnaglyph()
{
	/* called from post_gl_init and hud/options (option.c calls fwl_set_AnaglyphParameter above) */
	if(Viewer.anaglyphMethod == 1)
	{
		if(Viewer.haveAnaglyphShader)
		{
			Viewer.anaglyph = 1; 
			Viewer.isStereo = 1;
			setStereoBufferStyle(1);
		}
	}
	else if(Viewer.anaglyphMethod == 2)
	{
			Viewer.anaglyph = 1; 
			Viewer.isStereo = 1;
			setStereoBufferStyle(1);
	}
}
void setMono()
{
	Viewer.isStereo = 0;
	if(usingAnaglyph2())
		glColorMask(1,1,1,1);
	Viewer.anaglyph = 0;
	Viewer.sidebyside = 0;
	Viewer.shutterGlasses = 0;
	shutterGlasses = 0;
}

void setStereo(int type)
{
	/* type: 0 off  1 shutterglasses 2 sidebyside 3 analgyph */
	/* can only be called after opengl is initialized */
	//initStereoDefaults(); 
	setMono();
	switch(type)
	{
	case 0: {/*setMono()*/;break;}
	case 1: {fwl_init_Shutter(); break;}
	case 2: {fwl_init_SideBySide(); break;}
	case 3: {setAnaglyph(); break;}
	default: break;
	}
}
void toggleOrSetStereo(int type)
{
	/* if user clicks the active stereovision type on a HUD, then it should turn it off - back to mono
	if it's not active, then it should be set active*/
	int curtype, shut;
	shut = Viewer.shutterGlasses ? 1 : 0;
	curtype = Viewer.isStereo*( (shut)*1 + Viewer.sidebyside*2 + Viewer.anaglyph*3);
	if(type != curtype) 
		setStereo(type);
	else
		setMono();
}
void updateEyehalf()
{
	if( Viewer.screendist != 0.0)
		set_eyehalf( Viewer.eyedist/2.0,atan2(Viewer.eyedist/2.0,Viewer.screendist)*360.0/(2.0*3.1415926));
}

void viewer_postGLinit_init(void)
{
	GLboolean quadbuffer;
	int type;
	FW_GL_GETBOOLEANV(GL_STEREO,&quadbuffer);
	Viewer.haveQuadbuffer = (quadbuffer == GL_TRUE);
	Viewer.haveAnaglyphShader = initAnaglyphShaders();
	updateEyehalf();
	type = 0;
	if( Viewer.shutterGlasses ) type = 1;
	if( Viewer.sidebyside ) type = 2;
	if( Viewer.anaglyph ==1 ) type = 3;
	
	if(Viewer.anaglyph ==1) 
	{
		if(Viewer.anaglyphMethod == 1)
			if( !Viewer.haveAnaglyphShader ) 
			{
				ConsoleMessage("anaglyph shaders did not initialize - do you have opengl 2.0+ drivers?\n");
			}
	}
	if(Viewer.shutterGlasses)
	{
		// does this opengl driver/hardware support GL_STEREO? p.469, p.729 RedBook and
		//   WhiteDune > swt.c L1306
		if (!Viewer.haveQuadbuffer ) {
			ConsoleMessage("Unable to get quadbuffer stereo visual, switching to flutter mode\n");
		}
	}
	setStereo(type);
}

void fwl_set_StereoParameter (const char *optArg) {

	int i;
	//if(Viewer.isStereo == 0)
	//	initStereoDefaults();

	i = sscanf(optArg,"%lf",&Viewer.stereoParameter);
	if (i==0) printf ("warning, command line stereo parameter incorrect - was %s\n",optArg);
	else updateEyehalf();
}

void fwl_set_EyeDist (const char *optArg) {
	int i;
	//if(Viewer.isStereo == 0)
	//	initStereoDefaults();

	i= sscanf(optArg,"%lf",&Viewer.eyedist);
	if (i==0) printf ("warning, command line eyedist parameter incorrect - was %s\n",optArg);
	else updateEyehalf();
}

void fwl_set_ScreenDist (const char *optArg) {
	int i;
	//if(Viewer.isStereo == 0)
	//	initStereoDefaults();

	i= sscanf(optArg,"%lf",&Viewer.screendist);
	if (i==0) printf ("warning, command line screendist parameter incorrect - was %s\n",optArg);
	else updateEyehalf();
}
/* end of Shutter glasses, stereo mode configure */

void set_stereo_offset0() /*int iside, double eyehalf, double eyehalfangle)*/
{
      double x = 0.0, angle = 0.0;

      if (Viewer.iside == 0) {
		      /* left */
              x = Viewer.eyehalf;
              angle = Viewer.eyehalfangle * Viewer.stereoParameter; /*stereoparamter: 0-1 1=toe in to cross-over at Screendist 0=look at infinity, eyes parallel*/
      } else if (Viewer.iside == 1) {
		      /* right */
              x = -Viewer.eyehalf;
              angle = -Viewer.eyehalfangle * Viewer.stereoParameter;
      }
      FW_GL_TRANSLATE_D(x, 0.0, 0.0);
      FW_GL_ROTATE_D(angle, 0.0, 1.0, 0.0);
}

/* used to move, in WALK, FLY modes. */
void increment_pos(struct point_XYZ *vec) {
	struct point_XYZ nv;
	Quaternion q_i;

	viewer_lastP_add(vec);

	/* bound-viewpoint-space > Viewer.Pos,Viewer.Quat > avatar-space */
	quaternion_inverse(&q_i, &(Viewer.Quat));
	quaternion_rotation(&nv, &q_i, vec);

	/* save velocity calculations for this mode; used for EAI calls only */
	VPvelocity.x = nv.x; VPvelocity.y = nv.y; VPvelocity.z = nv.z;
	/* and, act on this change of location. */
	Viewer.Pos.x += nv.x;  /* Viewer.Pos must be in bound-viewpoint space */
	Viewer.Pos.y += nv.y; 
	Viewer.Pos.z += nv.z;
	

	/* printf ("increment_pos; oldpos %4.2f %4.2f %4.2f, anti %4.2f %4.2f %4.2f nv %4.2f %4.2f %4.2f \n",
		Viewer.Pos.x, Viewer.Pos.y, Viewer.Pos.z, 
		Viewer.AntiPos.x, Viewer.AntiPos.y, Viewer.AntiPos.z, 
		nv.x, nv.y, nv.z); */
	
}

/* We have a OrthoViewpoint node being bound. (not a GeoViewpoint node) */
void bind_OrthoViewpoint (struct X3D_OrthoViewpoint *vp) {
	Quaternion q_i;
	float xd, yd,zd;


	/* did bind_node tell us we could bind this guy? */
	if (!(vp->isBound)) return;

	/* SLERPing */
	/* record position BEFORE calculating new Viewpoint position */
	INITIATE_SLERP

	/* calculate distance between the node position and defined centerOfRotation */
	INITIATE_POSITION

	/* assume Perspective, unless Otrho set */
	Viewer.ortho=TRUE;
	if (vp->fieldOfView.n == 4) {
			/* Ortho mapping - glOrtho order left/right/bottom/top
			   assume X3D says left bottom right top */
		Viewer.orthoField[0] = (double) vp->fieldOfView.p[0];
		Viewer.orthoField[1] = (double) vp->fieldOfView.p[2];
		Viewer.orthoField[2] = (double) vp->fieldOfView.p[1];
		Viewer.orthoField[3] = (double) vp->fieldOfView.p[3];
	} else {
		ERROR_MSG("OrthoViewpoint - fieldOfView must have 4 parameters");
		Viewer.orthoField[0] = 0.0;
		Viewer.orthoField[1] = 0.0;
		Viewer.orthoField[2] = 0.0;
		Viewer.orthoField[3] = 0.0;
	}

	/* printf ("orthoviewpoint binding distance %f\n",Viewer.Dist);  */

	/* since this is not a bind to a GeoViewpoint node... */
	Viewer.GeoSpatialNode = NULL;

	/* set the examine mode rotation origin */
	INITIATE_ROTATION_ORIGIN

	/* printf ("BVP, origin %4.3f %4.3f %4.3f\n",Viewer.examine->Origin.x, Viewer.examine->Origin.y, Viewer.examine->Origin.z); */

	/* set Viewer position and orientation */

	/*
	printf ("bind_OrthoViewpoint, setting Viewer to %f %f %f orient %f %f %f %f\n",vp->position.c[0],vp->position.c[1],
	vp->position.c[2],vp->orientation.c[0],vp->orientation.c[1],vp->orientation.c[2], vp->orientation.c[3]);
	printf ("	node %d fieldOfView %f\n",vp,vp->fieldOfView); 
	printf ("	center of rotation %f %f %f\n",vp->centerOfRotation.c[0], vp->centerOfRotation.c[1],vp->centerOfRotation.c[2]);
	*/
	
	/* 
	
	From specs > abstract > architecture > 23.3.5 Viewpoint
	"When a Viewpoint node is at the top of the stack, the user's view is 
	conceptually re-parented as a child of the Viewpoint node. All subsequent changes to the Viewpoint node's
	coordinate system change the user's view (e.g., changes to any ancestor transformation nodes or to 
	the Viewpoint node's position or orientation fields)."
	
	"Navigation types (see 23.3.4 NavigationInfo) that require a definition of a down vector (e.g., terrain following) 
	shall use the negative Y-axis of the coordinate system of the currently bound Viewpoint node. 
	Likewise, navigation types that require a definition of an up vector shall use the positive Y-axis of the 
	coordinate system of the currently bound Viewpoint node. The orientation field of the Viewpoint node does 
	not affect the definition of the down or up vectors. This allows the author to separate the viewing direction 
	from the gravity direction."

	concept of transformations Jan3,2010:
world coords > [Transform stack] > bound Viewpoint > [Viewer.Pos,.Quat] > avatar 
"            < inverse[Transformstack] < "         < [AntiPos,AntiQuat] < avatar 
	gravity (according to specs): Y-down in the (bound Viewpoint local coords). 
	The viewpoint node orientation field doesn't count toward specs-gravity.
	If you want global-gravity, then put your viewpoint node at the scene level, or compute a 
	per-frame gravity for spherical worlds - see mainloop.c render_collisions.

	Implication: the user button LEVEL should level out/cancel/zero the bound-viewpoint's orientation field value.

	*/

	INITIATE_POSITION_ANTIPOSITION
	/* printf ("bind_OrthoViewpoint, pos %f %f %f antipos %f %f %f\n",Viewer.Pos.x, Viewer.Pos.y, Viewer.Pos.z, Viewer.AntiPos.x, Viewer.AntiPos.y, Viewer.AntiPos.z);
	*/

	viewer_lastP_clear();
	resolve_pos();
}

/* We have a Viewpoint node being bound. (not a GeoViewpoint node) */
void bind_Viewpoint (struct X3D_Viewpoint *vp) {
	Quaternion q_i;
	float xd, yd,zd;

	/* did bind_node tell us we could bind this guy? */
	if (!(vp->isBound)) return;

	/* SLERPing */
	/* record position BEFORE calculating new Viewpoint position */
	INITIATE_SLERP

	/* calculate distance between the node position and defined centerOfRotation */
	INITIATE_POSITION

	/* assume Perspective, unless Otrho set */
	Viewer.ortho=FALSE;

	/* printf ("viewpoint binding distance %f\n",Viewer.Dist);  */

	/* since this is not a bind to a GeoViewpoint node... */
	Viewer.GeoSpatialNode = NULL;

	/* set the examine mode rotation origin */
	INITIATE_ROTATION_ORIGIN

	/* set Viewer position and orientation */
	/* 
	
	From specs > abstract > architecture > 23.3.5 Viewpoint
	"When a Viewpoint node is at the top of the stack, the user's view is 
	conceptually re-parented as a child of the Viewpoint node. All subsequent changes to the Viewpoint node's
	coordinate system change the user's view (e.g., changes to any ancestor transformation nodes or to 
	the Viewpoint node's position or orientation fields)."
	
	"Navigation types (see 23.3.4 NavigationInfo) that require a definition of a down vector (e.g., terrain following) 
	shall use the negative Y-axis of the coordinate system of the currently bound Viewpoint node. 
	Likewise, navigation types that require a definition of an up vector shall use the positive Y-axis of the 
	coordinate system of the currently bound Viewpoint node. The orientation field of the Viewpoint node does 
	not affect the definition of the down or up vectors. This allows the author to separate the viewing direction 
	from the gravity direction."

	concept of transformations Jan3,2010:
world coords > [Transform stack] > bound Viewpoint > [Viewer.Pos,.Quat] > avatar 
"            < inverse[Transformstack] < "         < [AntiPos,AntiQuat] < avatar 
	gravity (according to specs): Y-down in the (bound Viewpoint local coords). 
	The viewpoint node orientation field doesn't count toward specs-gravity.
	If you want global-gravity, then put your viewpoint node at the scene level, or compute a 
	per-frame gravity for spherical worlds - see mainloop.c render_collisions.

	Implication: the user button LEVEL should level out/cancel/zero the bound-viewpoint's orientation field value.

	*/

	INITIATE_POSITION_ANTIPOSITION

	viewer_lastP_clear();
	resolve_pos();
}

