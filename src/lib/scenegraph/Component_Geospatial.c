/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Geospatial.c,v 1.33 2010/01/12 20:04:47 sdumoulin Exp $

X3D Geospatial Component

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

#include "../world_script/fieldSet.h"
#include "../x3d_parser/Bindable.h"
#include "Collision.h"
#include "quaternion.h"
#include "Viewer.h"
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../input/EAIHelpers.h"	/* for newASCIIString() */

#include "Polyrep.h"
#include "LinearAlgebra.h"
#include "Component_Geospatial.h"
#include "Children.h"

/*
Coordinate Conversion algorithms were taken from 2 locations after
reading and comprehending the references. The code selected was
taken and modified, because the original coders "knew their stuff";
any problems with the modified code should be sent to John Stewart.

------
References:

Jean Meeus "Astronomical Algorithms", 2nd Edition, Chapter 11, page 82

"World Geodetic System"
http://en.wikipedia.org/wiki/WGS84
http://en.wikipedia.org/wiki/Geodetic_system#Conversion

"Mathworks Aerospace Blockset"
http://www.mathworks.com/access/helpdesk/help/toolbox/aeroblks/index.html?/access/helpdesk/help/toolbox/aeroblks/geocentrictogeodeticlatitude.html&http://www.google.ca/search?hl=en&q=Geodetic+to+Geocentric+conversion+equation&btnG=Google+Search&meta=

"TRANSFORMATION OF GEOCENTRIC TO GEODETIC COORDINATES WITHOUT APPROXIMATIONS"
http://www.astro.uni.torun.pl/~kb/Papers/ASS/Geod-ASS.htm

"Geodetic Coordinate Conversions"
http://www.gmat.unsw.edu.au/snap/gps/clynch_pdfs/coordcvt.pdf

"TerrestrialCoordinates.c"
http://www.lsc-group.phys.uwm.edu/lal/slug/nightly/doxygen/html/TerrestrialCoordinates_8c.html

------
Code Conversions:

Geodetic to UTM:
UTM to Geodetic:
	Geo::Coordinates::UTM - Perl extension for Latitiude Longitude conversions.
	Copyright (c) 2000,2002,2004,2007 by Graham Crookham. All rights reserved.

Geocentric to Geodetic:
Geodetic to Geocentric:
	Filename: Gdc_To_Gcc_Converter.java
	Author: Dan Toms, SRI International
	Package: GeoTransform <http://www.ai.sri.com/geotransform/>
	Acknowledgements:
	  The algorithms used in the package were created by Ralph Toms and
	  first appeared as part of the SEDRIS Coordinate Transformation API.
	  These were subsequently modified for this package. This package is
	  not part of the SEDRIS project, and the Java code written for this
	  package has not been certified or tested for correctness by NIMA.


*********************************************************************/

/* defines used to get a SFVec3d into/outof a function that expects a MFVec3d */
#define MF_SF_TEMPS	struct Multi_Vec3d mIN; struct Multi_Vec3d  mOUT; struct Multi_Vec3d gdCoords;
#define FREE_MF_SF_TEMPS FREE_IF_NZ(gdCoords.p); FREE_IF_NZ(mOUT.p);


#define INIT_MF_FROM_SF(myNode, myField) \
	mIN.n = 1; \
	mIN.p = MALLOC(sizeof (struct SFVec3d)); \
	mIN.p[0].c[0] = myNode-> myField .c[0];\
	mIN.p[0].c[1] = myNode-> myField .c[1];\
	mIN.p[0].c[2] = myNode-> myField .c[2];\
	mOUT.n=0; mOUT.p = NULL; \
	gdCoords.n=0; gdCoords.p = NULL;

#define MF_FIELD_IN_OUT &mIN, &mOUT, &gdCoords
#define COPY_MF_TO_SF(myNode, myField) \
	myNode-> myField .c[0] = mOUT.p[0].c[0]; \
	myNode-> myField .c[1] = mOUT.p[0].c[1]; \
	myNode-> myField .c[2] = mOUT.p[0].c[2]; \
	FREE_IF_NZ(mIN.p); FREE_IF_NZ(mOUT.p);


#define MOVE_TO_ORIGIN(me)	GeoMove(X3D_GEOORIGIN(me->geoOrigin), &me->__geoSystem, &mIN, &mOUT, &gdCoords);
#define COMPILE_GEOSYSTEM(me) compile_geoSystem (me->_nodeType, &me->geoSystem, &me->__geoSystem);

#define RADIANS_PER_DEGREE (double)0.0174532925199432957692
#define DEGREES_PER_RADIAN (double)57.2957795130823208768

#define ENSURE_SPACE(variableInQuestion) \
	/* enough room for output? */ \
	if (variableInQuestion ->n < inCoords->n) { \
		if (variableInQuestion ->p != NULL) { \
			FREE_IF_NZ(variableInQuestion->p); \
		} \
		variableInQuestion ->p = MALLOC(sizeof (struct SFVec3d) * inCoords->n); \
		variableInQuestion ->n = inCoords->n; \
	} 

/* for UTM, GC, GD conversions */
#define ELEVATION_OUT   outc->p[i].c[elevation]
#define ELEVATION_IN    inc->p[i].c[elevation]
#define EASTING_IN	inc->p[i].c[easting]
#define NORTHING_IN	inc->p[i].c[northing]
#define UTM_SCALE 	(double)0.9996
#define LATITUDE_OUT	outc->p[i].c[latitude]
#define LONGITUDE_OUT	outc->p[i].c[longitude]
#define LATITUDE_IN	inc->p[i].c[latitude]
#define LONGITUDE_IN	inc->p[i].c[longitude]

#define GC_X_OUT 	outc->p[i].c[0] 
#define GC_Y_OUT 	outc->p[i].c[1]
#define GC_Z_OUT 	outc->p[i].c[2]

/* for Gd_Gc conversions */
#define GEOSP_AA_A	(double)6377563.396
#define GEOSP_AA_F	(double)299.3249646
#define GEOSP_AM_A	(double)6377340.189
#define GEOSP_AM_F	(double)299.3249646
#define GEOSP_AN_A	(double)6378160
#define GEOSP_AN_F	(double)298.25
#define GEOSP_BN_A	(double)6377483.865
#define GEOSP_BN_F	(double)299.1528128
#define GEOSP_BR_A	(double)6377397.155
#define GEOSP_BR_F	(double)299.1528128
#define GEOSP_CC_A	(double)6378206.4
#define GEOSP_CC_F	(double)294.9786982
#define GEOSP_CD_A	(double)6378249.145
#define GEOSP_CD_F	(double)293.465
#define GEOSP_EA_A	(double)6377276.345
#define GEOSP_EA_F	(double)300.8017
#define GEOSP_EB_A	(double)6377298.556
#define GEOSP_EB_F	(double)300.8017
#define GEOSP_EC_A	(double)6377301.243
#define GEOSP_EC_F	(double)300.8017
#define GEOSP_ED_A	(double)6377295.664
#define GEOSP_ED_F	(double)300.8017
#define GEOSP_EE_A	(double)6377304.063
#define GEOSP_EE_F	(double)300.8017
#define GEOSP_EF_A	(double)6377309.613
#define GEOSP_EF_F	(double)300.8017
#define GEOSP_FA_A	(double)6378155
#define GEOSP_FA_F	(double)298.3
#define GEOSP_HE_A	(double)6378200
#define GEOSP_HE_F	(double)298.3
#define GEOSP_HO_A	(double)6378270
#define GEOSP_HO_F	(double)297
#define GEOSP_ID_A	(double)6378160
#define GEOSP_ID_F	(double)298.247
#define GEOSP_IN_A	(double)6378388
#define GEOSP_IN_F	(double)297
#define GEOSP_KA_A	(double)6378245
#define GEOSP_KA_F	(double)298.3
#define GEOSP_RF_A	(double)6378137
#define GEOSP_RF_F	(double)298.257222101
#define GEOSP_SA_A	(double)6378160
#define GEOSP_SA_F	(double)298.25
#define GEOSP_WD_A	(double)6378135
#define GEOSP_WD_F	(double)298.26
#define GEOSP_WE_A	(double)6378137
#define GEOSP_WE_F	(double)298.257223563

#define ELLIPSOID(typ) \
	case typ: Gd_Gc(inCoords,outCoords,typ##_A, typ##_F,geoSystem->p[3]); break;

#define UTM_ELLIPSOID(typ) \
	case typ: Utm_Gd (inCoords, gdCoords, typ##_A, typ##_F, geoSystem->p[3], geoSystem->p[2], TRUE); \
		  Gd_Gc(gdCoords,outCoords,typ##_A, typ##_F, geoSystem->p[3]); break;

#define GCC_X gcc->c[0]
#define GCC_Y gcc->c[1]
#define GCC_Z gcc->c[2]
#define GDC_LAT gdc->c[0]
#define GDC_LON gdc->c[1]
#define GDC_ELE gdc->c[2]

#define INITIALIZE_GEOSPATIAL(me) \
	initializeGeospatial((struct X3D_GeoOrigin **) &me->geoOrigin); 

#define CONVERT_BACK_TO_GD_OR_UTM(thisField) \
	/* compileGeosystem - encode the return value such that srf->p[x] is... \
                        0:      spatial reference frame (GEOSP_UTM, GEOSP_GC, GEOSP_GD); \
                        1:      spatial coordinates (defaults to GEOSP_WE) \
                        2:      UTM zone number, 1..60. INT_ID_UNDEFINED = not specified \
                        3:      UTM:    if "S" - value is FALSE, not S, value is TRUE \
                                GD:     if "latitude_first" TRUE, if "longitude_first", FALSE \
                                GC:     if "northing_first" TRUE, if "easting_first", FALSE */ \
 \
	/* do we need to change this from a GCC? */ \
	if (node->__geoSystem.n != 0) { /* do we have a GeoSystem specified?? if not, dont do this! */ \
		struct SFVec3d gdCoords; \
 \
		if (node->__geoSystem.p[0] != GEOSP_GC) { \
			/* have to convert to GD or UTM. Go to GD first */ \
 \
			if (Viewer.GeoSpatialNode != NULL) { \
        					retractOrigin(Viewer.GeoSpatialNode->geoOrigin, \
					&thisField); \
			} \
         \
 \
			/* printf ("changed retracted, %lf %lf %lf\n", thisField.c[0], thisField.c[1], thisField.c[2]); */ \
 \
			/* now, convert to a GDC */ \
			gccToGdc (&thisField, &gdCoords); \
			memcpy (&thisField, &gdCoords, sizeof (struct SFVec3d)); \
 \
			/* printf ("changed as a GDC, %lf %lf %lf\n", thisField.c[0], thisField.c[1], thisField.c[2]); */ \
		 \
			/* is this a GD? if so, go no further */ \
			if (node->__geoSystem.p[0] == GEOSP_GD) { \
				/* do we need to flip lat and lon? */ \
				if (!(node->__geoSystem.p[3])) { \
					double tmp; \
					tmp = thisField.c[0]; \
					thisField.c[0] = thisField.c[1]; \
					thisField.c[1] = tmp; \
				} \
 \
			} else { \
				/* convert this to UTM */ \
				int zone;  \
				double easting; \
				double northing; \
				 \
				/* get the zone from the geoSystem; if undefined, we will calculate */ \
				zone = node->__geoSystem.p[2]; \
				gdToUtm(thisField.c[0], \
					thisField.c[1], \
					&zone, &easting, &northing); \
 \
				thisField.c[0] = northing; \
				thisField.c[1] = easting; \
 \
			/* printf ("changed as a UTM, %lf %lf %lf\n", thisField[0], thisField[1], thisField[2]); */ \
			}  \
		} \
	}

int geoLodLevel = 0;

static int gcToGdInit = FALSE;

static void compile_geoSystem (int nodeType, struct Multi_String *args, struct Multi_Int32 *srf);
static void moveCoords(struct Multi_Int32*, struct Multi_Vec3d *, struct Multi_Vec3d *, struct Multi_Vec3d *);
static void Gd_Gc (struct Multi_Vec3d *, struct Multi_Vec3d *, double, double, int);
static void gccToGdc (struct SFVec3d *, struct SFVec3d *); 
static void calculateViewingSpeed(void);

/* for converting from GC to GD */
static double A, F, C, A2, C2, Eps2, Eps21, Eps25, C254, C2DA, CEE,
                 CE2, CEEps2, TwoCEE, tem, ARat1, ARat2, BRat1, BRat2, B1,B2,B3,B4,B5;


/* move ourselves BACK to the from the GeoOrigin */
static void retractOrigin(struct X3D_GeoOrigin *myGeoOrigin, struct SFVec3d *gcCoords) {
	if (myGeoOrigin != NULL) {
		gcCoords->c[0] += myGeoOrigin->__movedCoords.c[0];
		gcCoords->c[1] += myGeoOrigin->__movedCoords.c[1];
		gcCoords->c[2] += myGeoOrigin->__movedCoords.c[2];
	}
}


/* convert GD ellipsiod to GC coordinates */
static void Gd_Gc (struct Multi_Vec3d *inc, struct Multi_Vec3d *outc, double radius, double eccentricity, int lat_first) {
	int i;
	double A = radius;
	double A2 = radius*radius;
	double F = (double)(1/eccentricity);
	double C = A*((double)1.0 - F);
	double C2 = C*C;
	double Eps2 = F*((double)2.0 - F);
	double Eps25 = (double) 0.25 * Eps2;

	int latitude = 0;
	int longitude = 1;
	int elevation = 2;

	double source_lat;
	double source_lon;
	double slat;
	double slat2;
	double clat;
	double Rn;
	double RnPh;

	if (!lat_first) {
		printf ("Gd_Gc, NOT lat first\n");
		latitude = 1; longitude = 0;
	}

	/* enough room for output? */
	if (outc->n < inc->n) {
		FREE_IF_NZ(outc->p);
		outc->p = MALLOC(sizeof (struct SFVec3d) * inc->n);
		outc->n = inc->n;
	}
	#ifdef VERBOSE
	printf ("Gd_Gc, have n of %d\n",inc->n);
	#endif

	for (i=0; i<inc->n; i++) {
		#ifdef VERBOSE
		printf ("Gd_Gc, ining lat %lf long %lf ele %lf   ",LATITUDE_IN, LONGITUDE_IN, ELEVATION_IN);
		#endif

		source_lat = RADIANS_PER_DEGREE * LATITUDE_IN;
		source_lon = RADIANS_PER_DEGREE * LONGITUDE_IN;
	
		#ifdef VERBOSE
		printf ("Source Latitude  %lf Source Longitude %lf\n",source_lat, source_lon);
		#endif

		slat = sin(source_lat);
		slat2 = slat*slat;
		clat = cos(source_lat);
	
		#ifdef VERBOSE
		printf ("slat %lf slat2 %lf clat %lf\n",slat, slat2, clat);
		#endif


		/* square root approximation for Rn */
		Rn = A / ( (.25 - Eps25 * slat2 + .9999944354799/4) + (.25-Eps25 * slat2)/(.25 - Eps25 * slat2 + .9999944354799/4));
	
		RnPh = Rn + ELEVATION_IN;

		#ifdef VERBOSE
		printf ("Rn %lf RnPh %lf\n",Rn, RnPh);
		#endif

		GC_X_OUT = RnPh * clat * cos(source_lon);
		GC_Y_OUT = RnPh * clat * sin(source_lon);
		GC_Z_OUT = ((C2 / A2) * Rn + ELEVATION_IN) * slat;

		#ifdef VERBOSE
		printf ("Gd_Gc, outing x %lf y %lf z %lf\n", GC_X_OUT, GC_Y_OUT, GC_Z_OUT);
		#endif
	}
}

/* convert UTM to GC coordinates by converting to GD as an intermediary step */
static void Utm_Gd (struct Multi_Vec3d *inc, struct Multi_Vec3d *outc, double radius, double flatten, int hemisphere_north, int zone, int northing_first) {
	int i;
	int northing = 0;	/* for determining which input value is northing */
	int easting = 1;	/* for determining which input value is easting */
	int elevation = 2;	/* elevation is always third value, input AND output */
	int latitude = 0;	/* always return latitude as first value */
	int longitude = 1;	/* always return longtitude as second value */

	/* create the ERM constants. */
	double F = 1.0/flatten;
	double Eccentricity   = (F) * (2.0-F);

	double myEasting;
	double myphi1rad;
	double myN1;
	double myT1;
	double myC1;
	double myR1;
	double myD;
	double Latitude;
	double Longitude;
	double longitudeOrigin;
	double myeccPrimeSquared;
	double myNorthing;
	double northingDRCT1;
	double eccRoot;
	double calcConstantTerm1;
	double calcConstantTerm2;
	double calcConstantTerm3;
	double calcConstantTerm4;

	/* is the values specified with an "easting_first?" */
	if (!northing_first) { northing = 1; easting = 0; }

	#ifdef VERBOSE
	if (!northing_first) printf ("UTM to GD, not northing first, flipping norhting and easting\n");
	#endif
		
	#ifdef VERBOSE
	if (northing_first) printf ("Utm_Gd: northing first\n"); else printf ("Utm_Gd: NOT northing_first\n");
	if (!hemisphere_north) printf ("Utm_Gd: NOT hemisphere_north\n"); else printf ("Utm_Gd: hemisphere_north\n"); 
	#endif


	/* enough room for output? */
	if (outc->n < inc->n) {
		FREE_IF_NZ(outc->p);
		outc->p = MALLOC(sizeof (struct SFVec3d) * inc->n);
		outc->n = inc->n;
	}

	/* constants for all UTM vertices */
	longitudeOrigin = (zone -1) * 6 - 180 + 3;
	myeccPrimeSquared = Eccentricity/(((double) 1.0) - Eccentricity);
	eccRoot = (((double)1.0) - sqrt (((double)1.0) - Eccentricity))/
	       (((double)1.0) + sqrt (((double)1.0) - Eccentricity));

	calcConstantTerm1 = ((double)1.0) -Eccentricity/
		((double)4.0) - ((double)3.0) *Eccentricity*Eccentricity/
		((double)64.0) -((double)5.0) *Eccentricity*Eccentricity*Eccentricity/((double)256.0);

	calcConstantTerm2 = ((double)3.0) * eccRoot/((double)2.0) - ((double)27.0) *eccRoot*eccRoot*eccRoot/((double)32.0);
	calcConstantTerm3 = ((double)21.0) * eccRoot*eccRoot/ ((double)16.0) - ((double)55.0) *eccRoot*eccRoot*eccRoot*eccRoot/ ((double)32.0);
	calcConstantTerm4 = ((double)151.0) *eccRoot*eccRoot*eccRoot/ ((double)96.0);

	#ifdef VERBOSE
	printf ("zone %d\n",zone);
	printf ("longitudeOrigin %lf\n",longitudeOrigin);
	printf ("myeccPrimeSquared %lf\n",myeccPrimeSquared);
	printf ("eccRoot %lf\n",eccRoot);
	#endif

	/* go through each vertex specified */
        for(i=0;i<inc->n;i++) {
		/* get the values for THIS UTM vertex */
		ELEVATION_OUT = ELEVATION_IN;
		myEasting = EASTING_IN - 500000;
		if (hemisphere_north) myNorthing = NORTHING_IN;
		else myNorthing = NORTHING_IN - (double)10000000.0;

		#ifdef VERBOSE
		printf ("myEasting %lf\n",myEasting);
		printf ("myNorthing %lf\n",myNorthing);
		#endif


		/* scale the northing */
		myNorthing= myNorthing / UTM_SCALE;

		northingDRCT1 = myNorthing /(radius * calcConstantTerm1);

		myphi1rad = northingDRCT1 + 
			calcConstantTerm2 * sin(((double)2.0) *northingDRCT1)+
			calcConstantTerm3 * sin(((double)4.0) *northingDRCT1)+
			calcConstantTerm4 * sin(((double)6.0) *northingDRCT1);

		myN1 = radius/sqrt(((double)1.0) - Eccentricity * sin(myphi1rad) * sin (myphi1rad));
		myT1 = tan(myphi1rad) * tan(myphi1rad); 
		myC1 = Eccentricity * cos(myphi1rad) * cos (myphi1rad);
		myR1 = radius * (((double)1.0) - Eccentricity) / pow(((double)1.0) - Eccentricity * sin(myphi1rad) * sin (myphi1rad), 1.5);
		myD = myEasting/(myN1*UTM_SCALE);

		Latitude = myphi1rad-(myN1*tan(myphi1rad)/myR1)*
				(myD*myD/((double)2.0) -
			(((double)5.0) + ((double)3.0) *myT1+ ((double)10.0) *myC1-
			((double)4.0) *myC1*myC1- ((double)9.0) *myeccPrimeSquared)*
			
			myD*myD*myD*myD/((double)24.0) +(((double)61.0) +((double)90.0) *
			myT1+((double)298.0) *myC1+ ((double)45.0) *myT1*myT1-
			((double)252.0) * myeccPrimeSquared- ((double)3.0) *myC1*myC1)*myD*myD*myD*myD*myD*myD/((double)720.0));


		Longitude = (myD-(((double)1.0)+((double)2.0)*myT1+myC1)*myD*myD*myD/((double)6.0)+(((double)5.0) - ((double)2.0) *myC1+
			((double)28.0) *myT1-((double)3.0) *myC1*myC1+
			((double)8.0) *myeccPrimeSquared+((double)24.0) *myT1*myT1)*myD*myD*myD*myD*myD/120)/cos(myphi1rad);



		LATITUDE_OUT = Latitude * DEGREES_PER_RADIAN;
		LONGITUDE_OUT = longitudeOrigin + Longitude * DEGREES_PER_RADIAN;


		#ifdef VERBOSE
		/* printf ("myNorthing scaled %lf\n",myNorthing);
		printf ("northingDRCT1 %lf\n",northingDRCT1);
		printf ("myphi1rad %lf\n",myphi1rad);
		printf ("myN1 %lf\n",myN1);
		printf ("myT1 %lf\n",myT1);
		printf ("myC1 %lf\n",myC1);
		printf ("myR1 %lf\n",myR1);
		printf ("myD %lf\n",myD);
		printf ("latitude %lf\n",Latitude);
		printf ("longitude %lf\n",Longitude);
		*/
		printf ("utmtogd\tnorthing %lf easting %lf ele %lf\n\tlat %lf long %lf ele %lf\n", NORTHING_IN, EASTING_IN, ELEVATION_IN, LATITUDE_OUT, LONGITUDE_OUT, ELEVATION_IN);
		#endif
        } 
}

/* take a set of coords, and a geoSystem, and create a set of moved coords */
/* we keep around the GD coords because we need them for rotation calculations */
/* parameters: 
	geoSystem:	compiled geoSystem integer array pointer
	inCoords:	coordinate structure for input coordinates, ANY coordinate type
	outCoords:	area for GC coordinates. Will MALLOC size if required 
	gdCoords:	GD coordinates, used for rotation calculations in later stages. WILL MALLOC THIS */

static void moveCoords (struct Multi_Int32* geoSystem, struct Multi_Vec3d *inCoords, struct Multi_Vec3d *outCoords, struct Multi_Vec3d *gdCoords) {

	int i;

	/* tmpCoords used for UTM coding */
	gdCoords->n=0; gdCoords->p=NULL;

	/* make sure the output has enough space for our converted data */
	ENSURE_SPACE(outCoords)
	ENSURE_SPACE(gdCoords)

	/* GD Geosystem - copy coordinates, and convert them to GC */
	switch (geoSystem->p[0]) {
		case  GEOSP_GD:
				/* GD_Gd_Gc_convert (inCoords, outCoords); */
				switch (geoSystem->p[1]) {
					ELLIPSOID(GEOSP_AA)
					ELLIPSOID(GEOSP_AM)
					ELLIPSOID(GEOSP_AN)
					ELLIPSOID(GEOSP_BN)
					ELLIPSOID(GEOSP_BR)
					ELLIPSOID(GEOSP_CC)
					ELLIPSOID(GEOSP_CD)
					ELLIPSOID(GEOSP_EA)
					ELLIPSOID(GEOSP_EB)
					ELLIPSOID(GEOSP_EC)
					ELLIPSOID(GEOSP_ED)
					ELLIPSOID(GEOSP_EE)
					ELLIPSOID(GEOSP_EF)
					ELLIPSOID(GEOSP_FA)
					ELLIPSOID(GEOSP_HE)
					ELLIPSOID(GEOSP_HO)
					ELLIPSOID(GEOSP_ID)
					ELLIPSOID(GEOSP_IN)
					ELLIPSOID(GEOSP_KA)
					ELLIPSOID(GEOSP_RF)
					ELLIPSOID(GEOSP_SA)
					ELLIPSOID(GEOSP_WD)
					ELLIPSOID(GEOSP_WE)
					default: printf ("unknown Gd_Gc: %s\n", stringGEOSPATIALType(geoSystem->p[1]));
				}

				/* now, for the GD coord return values; is this in the correct format for calculating 
				   rotations? */
				gdCoords->n = inCoords->n;

				/* is the GD value NOT the WGS84 ellipsoid? */
				if (geoSystem->p[1] != GEOSP_WE) {
					/*no, convert BACK from the GC to GD, WGS84 level for the gd value returns */
					for (i=0; i<outCoords->n; i++) {
						gccToGdc (&outCoords->p[i], &gdCoords->p[i]);
					}
				} else {
					/* just copy the coordinates for the GD temporary return  */
					memcpy (gdCoords->p, inCoords->p, sizeof (struct SFVec3d) * inCoords->n);
				}
			break;
		case GEOSP_GC:
			/* an earth-fixed geocentric coord; no conversion required for gc value returns */
			for (i=0; i< inCoords->n; i++) {
				outCoords->p[i].c[0] = inCoords->p[i].c[0];
				outCoords->p[i].c[1] = inCoords->p[i].c[1];
				outCoords->p[i].c[2] = inCoords->p[i].c[2];

				/* convert this coord from GC to GD, WGS84 ellipsoid for gd value returns */
				gccToGdc (&inCoords->p[i], &gdCoords->p[i]);
			}

			break;
		case GEOSP_UTM:
				/* GD coords will be returned from the conversion process....*/
				/* first, convert UTM to GC, then GD, then GD to GC */
				/* see the compileGeosystem function for geoSystem fields */
				switch (geoSystem->p[1]) {
					UTM_ELLIPSOID(GEOSP_AA)
					UTM_ELLIPSOID(GEOSP_AM)
					UTM_ELLIPSOID(GEOSP_AN)
					UTM_ELLIPSOID(GEOSP_BN)
					UTM_ELLIPSOID(GEOSP_BR)
					UTM_ELLIPSOID(GEOSP_CC)
					UTM_ELLIPSOID(GEOSP_CD)
					UTM_ELLIPSOID(GEOSP_EA)
					UTM_ELLIPSOID(GEOSP_EB)
					UTM_ELLIPSOID(GEOSP_EC)
					UTM_ELLIPSOID(GEOSP_ED)
					UTM_ELLIPSOID(GEOSP_EE)
					UTM_ELLIPSOID(GEOSP_EF)
					UTM_ELLIPSOID(GEOSP_FA)
					UTM_ELLIPSOID(GEOSP_HE)
					UTM_ELLIPSOID(GEOSP_HO)
					UTM_ELLIPSOID(GEOSP_ID)
					UTM_ELLIPSOID(GEOSP_IN)
					UTM_ELLIPSOID(GEOSP_KA)
					UTM_ELLIPSOID(GEOSP_RF)
					UTM_ELLIPSOID(GEOSP_SA)
					UTM_ELLIPSOID(GEOSP_WD)
					UTM_ELLIPSOID(GEOSP_WE)
					default: printf ("unknown Gd_Gc: %s\n", stringGEOSPATIALType(geoSystem->p[1]));
				}
			break;
		default :
			printf ("incorrect geoSystem field, %s\n",stringGEOSPATIALType(geoSystem->p[0]));
			return;

	}
}


static void initializeGeospatial (struct X3D_GeoOrigin **nodeptr)  {
	MF_SF_TEMPS
	struct X3D_GeoOrigin *node = NULL;

	#ifdef VERBOSE
	printf ("\ninitializing GeoSpatial code nodeptr %u\n",*nodeptr); 
	#endif

	if (*nodeptr != NULL) {
		if (X3D_GEOORIGIN(*nodeptr)->_nodeType != NODE_GeoOrigin) {
			printf ("expected a GeoOrigin node, but got a node of type %s\n",
				stringNodeType(X3D_GEOORIGIN(*nodeptr)->_nodeType));
			*nodeptr = NULL;
			return;
		} else {
			/* printf ("um, just setting geoorign to %u\n",(*nodeptr)); */
			node = X3D_GEOORIGIN(*nodeptr);
		}

		/* printf ("initGeoSpatial ich %d ch %d\n",node->_ichange, node->_change); */

		if NODE_NEEDS_COMPILING {
			compile_geoSystem (node->_nodeType, &node->geoSystem, &node->__geoSystem);
			INIT_MF_FROM_SF(node,geoCoords)
			moveCoords(&node->__geoSystem, MF_FIELD_IN_OUT);
			COPY_MF_TO_SF(node, __movedCoords)
			#ifdef VERBOSE
			printf ("initializeGeospatial, __movedCoords %lf %lf %lf, ryup %d, geoSystem %d %d %d %d\n",
				node->__movedCoords.c[0],
				node->__movedCoords.c[1],
				node->__movedCoords.c[2],
				node->rotateYUp,
				node->__geoSystem.p[0],
				node->__geoSystem.p[1],
				node->__geoSystem.p[2],
				node->__geoSystem.p[3]);
			printf ("initializeGeospatial, done\n\n");
			#endif

			FREE_MF_SF_TEMPS
			MARK_NODE_COMPILED
		}
	}
}

/* calculate a translation that moves a Geo node to local space */
static void GeoMove(struct X3D_GeoOrigin *geoOrigin, struct Multi_Int32* geoSystem, struct Multi_Vec3d *inCoords, struct Multi_Vec3d *outCoords,
		struct Multi_Vec3d *gdCoords) {
	int i;
	struct X3D_GeoOrigin * myOrigin;

	#ifdef VERBOSE
	printf ("\nstart of GeoMove... %d coords\n",inCoords->n);
	#endif

	/* enough room for output? */
	if (inCoords->n==0) {return;}
	if (outCoords->n < inCoords->n) {
		if (outCoords->n!=0) {
			FREE_IF_NZ(outCoords->p);
		}
		outCoords->p = MALLOC(sizeof (struct SFVec3d) * inCoords->n);
		outCoords->n = inCoords->n;
	}

	/* set out values to 0.0 for now */
	for (i=0; i<outCoords->n; i++) {
		outCoords->p[i].c[0] = (double) 0.0; outCoords->p[i].c[1] = (double) 0.0; outCoords->p[i].c[2] = (double) 0.0;
	}

	#ifdef VERBOSE
	for (i=0; i<outCoords->n; i++) {
		printf ("start of GeoMove, inCoords %d: %lf %lf %lf\n",i, inCoords->p[i].c[0], inCoords->p[i].c[1], inCoords->p[i].c[2]);
	}
	#endif



	/* check the GeoOrigin attached node */
	myOrigin = NULL;
	if (geoOrigin != NULL) {
		if (X3D_GEOORIGIN(geoOrigin)->_nodeType != NODE_GeoOrigin) {
			ConsoleMessage ("GeoMove, expected a GeoOrigin, found a %s",stringNodeType(X3D_GEOORIGIN(geoOrigin)->_nodeType));
			printf ("GeoMove, expected a GeoOrigin, found a %s\n",stringNodeType(X3D_GEOORIGIN(geoOrigin)->_nodeType));
			return;
		}

		myOrigin = geoOrigin; /* local one */
	}
	/* printf ("GeoMove, using myOrigin %u, passed in geoOrigin %u with vals %lf %lf %lf\n",myOrigin, myOrigin,
		myOrigin->geoCoords.c[0], myOrigin->geoCoords.c[1], myOrigin->geoCoords.c[2] ); */ 
		
	moveCoords(geoSystem, inCoords, outCoords, gdCoords);

	for (i=0; i<outCoords->n; i++) {

	#ifdef VERBOSE
	printf ("GeoMove, before subtracting origin %lf %lf %lf\n", outCoords->p[i].c[0], outCoords->p[i].c[1], outCoords->p[i].c[2]);
	if (myOrigin != NULL) printf ("	... origin %lf %lf %lf\n",myOrigin->__movedCoords.c[0], myOrigin->__movedCoords.c[1], myOrigin->__movedCoords.c[2]);
	#endif

	if (myOrigin != NULL) {
		outCoords->p[i].c[0] -= myOrigin->__movedCoords.c[0];
		outCoords->p[i].c[1] -= myOrigin->__movedCoords.c[1];
		outCoords->p[i].c[2] -= myOrigin->__movedCoords.c[2];
	}

	#ifdef VERBOSE
	printf ("GeoMove, after subtracting origin %lf %lf %lf\n", outCoords->p[i].c[0], outCoords->p[i].c[1], outCoords->p[i].c[2]);
	#endif
	}
}


/* for converting BACK to GD from GC */
static void initializeGcToGdParams(void) {
        A = GEOSP_WE_A;
        F = GEOSP_WE_F;
            
        /*  Create the ERM constants. */
        A2     = A * A;
        F      =1/(F);
        C      =(A) * (1-F);
        C2     = C * C;
        Eps2   =(F) * (2.0-F);
        Eps21  =Eps2 - 1.0;
        Eps25  =.25 * (Eps2);
        C254   =54.0 * C2;        
        
        C2DA   = C2 / A;
        CE2    = A2 - C2;
        tem    = CE2 / C2;
        CEE    = Eps2 * Eps2;        
        TwoCEE =2.0 * CEE;
        CEEps2 =Eps2 * CE2;
         
        /* UPPER BOUNDS ON POINT */
     

        ARat1  =pow((A + 50005.0),2);
        ARat2  =(ARat1) / pow((C+50005.0),2);
    
        /* LOWER BOUNDS ON POINT */
        
        BRat1  =pow((A-10005.0),2);
        BRat2  =(BRat1) / pow((C-10005.0),2);
          
	/* use WE ellipsoid */
	B1=0.100225438677758E+01;
	B2=-0.393246903633930E-04;
	B3=0.241216653453483E+12;
	B4=0.133733602228679E+14;
	B5=0.984537701867943E+00;
	gcToGdInit = TRUE;
}


/* convert BACK to a GD coordinate, from GC coordinates using WE ellipsoid */
static void gccToGdc (struct SFVec3d *gcc, struct SFVec3d *gdc) {
        double w2,w,z2,testu,testb,top,top2,rr,q,s12,rnn,s1,zp2,wp,wp2,cf,gee,alpha,cl,arg2,p,xarg,r2,r1,ro,
               arg0,s,roe,arg,v,zo;

	#ifdef VERBOSE
	printf ("gccToGdc input %lf %lf %lf\n",GCC_X, GCC_Y, GCC_Z);
	#endif
	

	if (!gcToGdInit) initializeGcToGdParams();

        w2=GCC_X * GCC_X + GCC_Y * GCC_Y;
        w=sqrt(w2);
        z2=GCC_Z * GCC_Z;

        testu=w2 + ARat2 * z2;
        testb=w2 + BRat2 * z2;

        if ((testb > BRat1) && (testu < ARat1)) 
        {    

            /*POINT IS BETWEEN-10 KIL AND 50 KIL, SO COMPUTE TANGENT LATITUDE */
    
            top= GCC_Z * (B1 + (B2 * w2 + B3) /
                 (B4 + w2 * B5 + z2));

            top2=top*top;

            rr=top2+w2;
                  
            q=sqrt(rr);
                  
            /* ****************************************************************
                  
               COMPUTE H IN LINE SQUARE ROOT OF 1-EPS2*SIN*SIN.  USE SHORT BINOMIAL
               EXPANSION PLUS ONE ITERATION OF NEWTON'S METHOD FOR SQUARE ROOTS.
            */

            s12=top2/rr;

            rnn = A / ( (.25 - Eps25*s12 + .9999944354799/4) + (.25-Eps25*s12)/(.25 - Eps25*s12 + .9999944354799/4));
            s1=top/q;
        
            /******************************************************************/

            /* TEST FOR H NEAR POLE.  if SIN(¯)**2 <= SIN(45.)**2 THEN NOT NEAR A POLE.*/  
    
            if (s12 < .50)
                GDC_ELE = q-rnn;
            else
                GDC_ELE = GCC_Z / s1 + (Eps21 * rnn);
                GDC_LAT = atan(top / w);
                GDC_LON = atan2(GCC_Y,GCC_X);
        }
              /* POINT ABOVE 50 KILOMETERS OR BELOW -10 KILOMETERS  */
        else /* Do Exact Solution  ************ */
        { 
            wp2=GCC_X * GCC_X + GCC_Y * GCC_Y;
            zp2=GCC_Z * GCC_Z;
            wp=sqrt(wp2);
            cf=C254 * zp2;
            gee=wp2 - (Eps21 * zp2) - CEEps2;
            alpha=cf / (gee*gee);
            cl=CEE * wp2 * alpha / gee;
            arg2=cl * (cl + 2.0);
            s1=1.0 + cl + sqrt(arg2);
            s=pow(s1,(1.0/3.0));
            p=alpha / (3.0 * pow(( s + (1.0/s) + 1.0),2));
            xarg= 1.0 + (TwoCEE * p);
            q=sqrt(xarg);
            r2= -p * (2.0 * (1.0 - Eps2) * zp2 / ( q * ( 1.0 + q) ) + wp2);
            r1=(1.0 + (1.0 / q));
            r2 /=A2;

            /*    DUE TO PRECISION ERRORS THE ARGUMENT MAY BECOME NEGATIVE IF SO SET THE ARGUMENT TO ZERO.*/

            if (r1+r2 > 0.0)
                ro = A * sqrt( .50 * (r1+r2));
            else
                ro=0.0;

            ro=ro - p * Eps2 * wp / ( 1.0 + q);
            arg0 = pow(( wp - Eps2 * ro),2) + zp2;
            roe = Eps2 * ro;
            arg = pow(( wp - roe),2) + zp2;
            v=sqrt(arg - Eps2 * zp2);
            zo=C2DA * GCC_Z / v;
            GDC_ELE = sqrt(arg) * (1.0 - C2DA / v);
            top=GCC_Z+ tem*zo;
            GDC_LAT = atan( top / wp );
            GDC_LON =atan2(GCC_Y,GCC_X);
        }  /* end of Exact solution */

        GDC_LAT *= DEGREES_PER_RADIAN;
        GDC_LON *= DEGREES_PER_RADIAN;
#undef VERBOSE

}

/* convert a GDC BACK to a UTM coordinate */
static void gdToUtm(double latitude, double longitude, int *zone, double *easting, double *northing) {
#define DEG2RAD (PI/180.00)
#define GEOSP_WE_INV 0.00669438
	double lat_radian;
	double long_radian;
	double myScale;
	int longOrigin;
	double longOriginradian;
	double eccentprime;
	double NNN;
	double TTT;
	double CCC;
	double AAA;
	double MMM;

	/* calculate the zone number if it is less than zero. If greater than zero, leave alone! */
	if (*zone < 0) 
		*zone = ((longitude + 180.0)/6.0) + 1;

	lat_radian = latitude * DEG2RAD;
	long_radian = longitude * DEG2RAD;
	myScale = 0.9996;
	longOrigin = (*zone - 1)*6 - 180 + 3;
	longOriginradian = longOrigin * DEG2RAD;
	eccentprime = GEOSP_WE_INV/(1-GEOSP_WE_INV);

	/* 
	printf ("lat_radian %lf long_radian %lf myScale %lf longOrigin %d longOriginradian %lf eccentprime %lf\n",
	   lat_radian, long_radian, myScale, longOrigin, longOriginradian, eccentprime);
	*/

	NNN = GEOSP_WE_A / sqrt(1-GEOSP_WE_INV * sin(lat_radian)*sin(lat_radian));
	TTT = tan(lat_radian) * tan(lat_radian);
	CCC = eccentprime * cos(lat_radian)*cos(lat_radian);
	AAA = cos(lat_radian) * (long_radian - longOriginradian);
	MMM = GEOSP_WE_A
            * ( ( 1 - GEOSP_WE_INV/4 - 3 * GEOSP_WE_INV * GEOSP_WE_INV/64
                  - 5 * GEOSP_WE_INV * GEOSP_WE_INV * GEOSP_WE_INV/256
                ) * lat_radian
              - ( 3 * GEOSP_WE_INV/8 + 3 * GEOSP_WE_INV * GEOSP_WE_INV/32
                  + 45 * GEOSP_WE_INV * GEOSP_WE_INV * GEOSP_WE_INV/1024
                ) * sin(2 * lat_radian)
              + ( 15 * GEOSP_WE_INV * GEOSP_WE_INV/256 +
                  45 * GEOSP_WE_INV * GEOSP_WE_INV * GEOSP_WE_INV/1024
                ) * sin(4 * lat_radian)
              - ( 35 * GEOSP_WE_INV * GEOSP_WE_INV * GEOSP_WE_INV/3072
                ) * sin(6 * lat_radian)
              );

	/* printf ("N %lf T %lf C %lf A %lf M %lf\n",NNN,TTT,CCC,AAA,MMM); */

	*easting = myScale*NNN*(AAA+(1-TTT+CCC)*AAA*AAA*AAA/6
                    + (5-18*TTT+TTT*TTT+72*CCC-58*eccentprime)*AAA*AAA*AAA*AAA*AAA/120)
                    + 500000.0;

	*northing= myScale * ( MMM + NNN*tan(lat_radian) * 
		( AAA*AAA/2+(5-TTT+9*CCC+4*CCC*CCC)*AAA*AAA*AAA*AAA/24 + (61-58*TTT+TTT*TTT+600*CCC-330*eccentprime) * AAA*AAA*AAA*AAA*AAA*AAA/720));

	/*if (latitude < 0) *northing += 10000000.0;*/

	#ifdef VERBOSE
	printf ("gdToUtm: lat %lf long %lf zone %d -> easting %lf northing %lf\n",latitude, longitude, *zone,*easting, *northing);
	#endif
}

/* calculate the rotation needed to apply to this position on the GC coordinate location */
static void GeoOrient (struct Multi_Int32 *geoSystem, struct SFVec3d *gdCoords, struct SFVec4d *orient) {
	Quaternion qx;
	Quaternion qz;
	Quaternion qr;

	/* is this a straight GC geoSystem? If so, we do not do any orientation */
	if (geoSystem->n > 0) {
		if (geoSystem->p[0] == GEOSP_GC) {
			#ifdef VERBOSE
			printf ("GeoOrient - simple GC, so no orient\n");
			#endif
			orient->c[0] = 0.0; 
			orient->c[1] = 1.0; 
			orient->c[2] = 0.0; 
			orient->c[3] = 0.0; 
			return;
		}
	}

	#ifdef VERBOSE
	printf ("GeoOrient - gdCoords->c[0,1] is %f %f\n",gdCoords->c[0],gdCoords->c[1]);
	#endif

	/* initialize qx and qz */
	vrmlrot_to_quaternion (&qz,0.0, 0.0, 1.0, RADIANS_PER_DEGREE*((double)90.0 + gdCoords->c[1]));

	#ifdef VERBOSE 
	printf ("GeoOrient qz angle (deg) %lf angle (rad) %lf quat: %lf %lf %lf %lf\n",((double)90.0 + gdCoords->c[1]), 
		RADIANS_PER_DEGREE*((double)90.0 + gdCoords->c[1]),qz.x, qz.y, qz.z,qz.w);
	#endif

	vrmlrot_to_quaternion (&qx,1.0, 0.0, 0.0, RADIANS_PER_DEGREE*((double)180.0 - gdCoords->c[0]));

	#ifdef VERBOSE 
	printf ("GeoOrient qx angle (deg) %lf angle (rad) %lf quat: %lf %lf %lf %lf\n",
		((double)180.0 - gdCoords->c[0]), RADIANS_PER_DEGREE*((double)180.0 - gdCoords->c[0]), qx.x, qx.y, qx.z,qx.w);
	#endif

	quaternion_add (&qr, &qx, &qz);

	#ifdef VERBOSE
	printf ("GeoOrient qr %lf %lf %lf %lf\n",qr.x, qr.y, qr.z,qr.w);
	#endif

        quaternion_to_vrmlrot(&qr, &orient->c[0], &orient->c[1], &orient->c[2], &orient->c[3]);

	#ifdef VERBOSE
	printf ("GeoOrient rotation %lf %lf %lf %lf\n",orient->c[0], orient->c[1], orient->c[2], orient->c[3]);
	#endif
}


/* compileGeosystem - encode the return value such that srf->p[x] is...
			0:	spatial reference frame	(GEOSP_UTM, GEOSP_GC, GEOSP_GD);
			1:	spatial coordinates (defaults to GEOSP_WE)
			2:	UTM zone number, 1..60. INT_ID_UNDEFINED = not specified
			3:	UTM:	if "S" - value is FALSE, not S, value is TRUE 
				GD:	if "latitude_first" TRUE, if "longitude_first", FALSE
				GC:	if "northing_first" TRUE, if "easting_first", FALSE */

static void compile_geoSystem (int nodeType, struct Multi_String *args, struct Multi_Int32 *srf) {
	int i;
	indexT this_srf = INT_ID_UNDEFINED;
	indexT this_srf_ind = INT_ID_UNDEFINED;

	#ifdef VERBOSE
	printf ("start of compile_geoSystem\n");
	#endif

	/* malloc the area required for internal settings, if required */
	if (srf->p==NULL) {
		srf->n=4;
		srf->p=MALLOC(sizeof(int) * 4);
	}

	/* set these as defaults */
	srf->p[0] = GEOSP_GD; 
	srf->p[1] = GEOSP_WE;
	srf->p[2] = INT_ID_UNDEFINED;
	srf->p[3] = TRUE;

	/* if nothing specified, we just use these defaults */
	if (args->n==0) return;

	/* first go through, and find the Spatial Reference Frame, GD, UTM, or GC */
	for (i=0; i<args->n; i++) {
		/* printf ("geoSystem args %d %s\n",i, args->p[i]->strptr); */
		indexT tc = findFieldInGEOSPATIAL(args->p[i]->strptr);

		if ((tc == GEOSP_GD) || (tc == GEOSP_GDC)) {
			this_srf = GEOSP_GD;
			this_srf_ind = i;
		} else if ((tc == GEOSP_GC) || (tc == GEOSP_GCC)) {
			this_srf = GEOSP_GC;
			this_srf_ind = i;
		} else if (tc == GEOSP_UTM) {
			this_srf = GEOSP_UTM;
			this_srf_ind = i;
		}
	}

	/* did we find a GC, GD, or UTM? */
	if (this_srf == INT_ID_UNDEFINED) {
		ConsoleMessage ("geoSystem in node %s,  must have GC, GD or UTM",stringNodeType(nodeType));
		return;
	}

	srf->p[0] = this_srf;
	/* go through and ensure that we have the correct parameters for this spatial reference frame */
	if (this_srf == GEOSP_GC) {
		/* possible parameter: GC:	if "northing_first" TRUE, if "easting_first", FALSE */
		srf->p[1] = INT_ID_UNDEFINED;
		for (i=0; i<args->n; i++) {
			if (strcmp("northing_first",args->p[i]->strptr) == 0) { srf->p[3] = TRUE;
			} else if (strcmp("easting_first",args->p[i]->strptr) == 0) { srf->p[3] = FALSE;
			} else if (i!=this_srf_ind) ConsoleMessage ("geoSystem GC parameter %s not allowed geospatial coordinates",args->p[i]->strptr);
		}
	} else if (this_srf == GEOSP_GD) {
		srf->p[1] = GEOSP_WE;
		/* possible parameters: ellipsoid, gets put into element 1.
				if "latitude_first" TRUE, if "longitude_first", FALSE */

		/* is there an optional argument? */
		for (i=0; i<args->n; i++) {
			/* printf ("geosp_gd, ind %d i am %d string %s\n",i, this_srf_ind,args->p[i]->strptr); */
                        if (strcmp("latitude_first", args->p[i]->strptr) == 0) {
				srf->p[3] = TRUE;
                        } else if (strcmp("longitude_first", args->p[i]->strptr) == 0) {
				srf->p[3] = FALSE;
			} else {
				if (i!= this_srf_ind) {
					indexT tc = findFieldInGEOSPATIAL(args->p[i]->strptr);
					switch (tc) {
						case INT_ID_UNDEFINED:
						case GEOSP_GC:
						case GEOSP_GCC:
						case GEOSP_GD:
						case GEOSP_GDC:
						case GEOSP_UTM:
						ConsoleMessage("expected valid GC parameter in node %s",stringNodeType(nodeType));
						srf->p[1] = GEOSP_WE;
						break;

						default:
						srf->p[1] = tc;
					}
				}
			}
		}
	} else {
		/* this must be UTM */
		/* encode the return value such that srf->p[x] is...
			0:	spatial reference frame	(GEOSP_UTM, GEOSP_GC, GEOSP_GD);
			1:	spatial coordinates (defaults to GEOSP_WE)
			2:	UTM zone number, 1..60. INT_ID_UNDEFINED = not specified
			3:	UTM:	if "S" - value is FALSE, not S, value is TRUE  */
		/* first go through, and find the Spatial Reference Frame, GD, UTM, or GC */
		for (i=0; i<args->n; i++) {
			if (i != this_srf_ind) {
				if (strcmp ("S",args->p[i]->strptr) == 0) {
					srf->p[3] = FALSE;
				} else if (args->p[i]->strptr[0] == 'Z') {
					int zone = -1;
					sscanf(args->p[i]->strptr,"Z%d",&zone);
					/* printf ("zone found as %d\n",zone); */
					srf->p[2] = zone;
				} else { 
					indexT tc = findFieldInGEOSPATIAL(args->p[i]->strptr);
					switch (tc) {
						case INT_ID_UNDEFINED:
						case GEOSP_GC:
						case GEOSP_GCC:
						case GEOSP_GD:
						case GEOSP_GDC:
						case GEOSP_UTM:
							ConsoleMessage("expected valid UTM Ellipsoid parameter in node %s",stringNodeType(nodeType));
							srf->p[1] = GEOSP_WE;
						break;

					default:
						srf->p[1] = tc;
					}
				}
			}
					
		}		
	}
	#ifdef VERBOSE
	printf ("printf done compileGeoSystem\n");
	#endif

}

/************************************************************************/
void compile_GeoCoordinate (struct X3D_GeoCoordinate * node) {
	MF_SF_TEMPS
	int i;

	#ifdef VERBOSE
	printf ("compiling GeoCoordinate\n");
	#endif

	/* standard MACROS expect specific field names */
	mIN = node->point;
	mOUT.p = NULL; mOUT.n = 0;


	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	MOVE_TO_ORIGIN(node)

	/* convert the doubles down to floats, because coords are used as floats in FreeWRL. */
	FREE_IF_NZ(node->__movedCoords.p);
	node->__movedCoords.p = MALLOC (sizeof (struct SFColor)  * mOUT.n);
	for (i=0; i<mOUT.n; i++) {
		node->__movedCoords.p[i].c[0] = (float) mOUT.p[i].c[0];
		node->__movedCoords.p[i].c[1] = (float) mOUT.p[i].c[1];
		node->__movedCoords.p[i].c[2] = (float) mOUT.p[i].c[2];
		#ifdef VERBOSE
		printf ("coord %d now is %f %f %f\n", i, node->__movedCoords.p[i].c[0],node->__movedCoords.p[i].c[1],node->__movedCoords.p[i].c[2]);
		#endif
	}
	node->__movedCoords.n = mOUT.n;

	FREE_IF_NZ(gdCoords.p);
	FREE_IF_NZ(mOUT.p);
	MARK_NODE_COMPILED
	
	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoCoordinate, metadata))
}


/************************************************************************/
/* GeoElevationGrid							*/
/************************************************************************/

/* check validity of ElevationGrid fields */
int checkX3DGeoElevationGridFields (struct X3D_GeoElevationGrid *node, float **points, int *npoints) {
	MF_SF_TEMPS
	int i,j;
	int nx;
	double xSp;
	int nz;
	double zSp;
	double *height;
	int ntri;
	int nh;
	struct X3D_PolyRep *rep;
	float *newpoints;
	int nquads;
	int *cindexptr;
	float *tcoord = NULL;
	double myHeightAboveEllip = 0.0;
	int mySRF = 0;
	
	nx = node->xDimension;
	xSp = node->xSpacing;
	nz = node->zDimension;
	zSp = node->zSpacing;
	height = node->height.p;
	nh = node->height.n;

	COMPILE_GEOSYSTEM(node)
	/* various values for converting to GD/UTM, etc */
	if (node->__geoSystem.n != 0)  {
		mySRF = node->__geoSystem.p[0];
		/* NOTE - DO NOT DO THIS CALCULATION - it is added in later 
		myHeightAboveEllip = getEllipsoidRadius(node->__geoSystem.p[1]);
		*/
	}

	rep = (struct X3D_PolyRep *)node->_intern;

	/* work out how many triangles/quads we will have */
	ntri = (nx && nz ? 2 * (nx-1) * (nz-1) : 0);
	nquads = ntri/2;

	/* check validity of input fields */
	if(nh != nx * nz) {
		if (nh > nx * nz) {
			printf ("GeoElevationgrid: warning: x,y vs. height: %d * %d ne %d:\n", nx,nz,nh);
		} else {
			printf ("GeoElevationgrid: error: x,y vs. height: %d * %d ne %d:\n", nx,nz,nh);
			return FALSE;
		}
	}

	/* do we have any triangles? */
	if ((nx < 2) || (nz < 2)) {
		printf ("GeoElevationGrid: xDimension and zDimension less than 2 %d %d\n", nx,nz);
		return FALSE;
	}

	/* any texture coordinates passed in? if so, DO NOT generate any texture coords here. */
        if (!(node->texCoord)) {
		/* allocate memory for texture coords */
		FREE_IF_NZ(rep->GeneratedTexCoords);

		/* 6 vertices per quad each vertex has a 2-float tex coord mapping */
		tcoord = rep->GeneratedTexCoords = (float *)MALLOC (sizeof (float) * nquads * 12); 

		rep->tcindex=0; /* we will generate our own mapping */
	}

	/* make up points array */
	/* a point is a vertex and consists of 3 floats (x,y,z) */
	newpoints = (float *)MALLOC (sizeof (float) * nz * nx * 3);
	 
	FREE_IF_NZ(rep->actualCoord);
	rep->actualCoord = (float *)newpoints;

	/* make up coord index */
	if (node->_coordIndex.n > 0) {FREE_IF_NZ(node->_coordIndex.p);}
	node->_coordIndex.p = MALLOC (sizeof(int) * nquads * 5);
	cindexptr = node->_coordIndex.p;

	node->_coordIndex.n = nquads * 5;
	/* return the newpoints array to the caller */
	*points = newpoints;
	*npoints = node->_coordIndex.n;

	#ifdef VERBOSE
	printf ("coordindex:\n");
	#endif

	/* ElevationGrids go 1 - 2 - 3 - 4 we go 1 - 4 - 3 - 2 */
	for (j = 0; j < (nz -1); j++) {
		for (i=0; i < (nx-1) ; i++) {
			#ifdef VERBOSE
			printf ("	%d %d %d %d %d\n", j*nx+i, j*nx+i+nx, j*nx+i+nx+1, j*nx+i+1, -1);
			#endif

#ifdef WINDING_ELEVATIONGRID
			*cindexptr = j*nx+i; cindexptr++; 	/* 1 */
			*cindexptr = j*nx+i+nx; cindexptr++; 	/* 2 */
			*cindexptr = j*nx+i+nx+1; cindexptr++;  /* 3 */
			*cindexptr = j*nx+i+1; cindexptr++; 	/* 4 */
			*cindexptr = -1; cindexptr++;
#else
			*cindexptr = j*nx+i; cindexptr++; 	/* 1 */
			*cindexptr = j*nx+i+1; cindexptr++; 	/* 4 */
			*cindexptr = j*nx+i+nx+1; cindexptr++;  /* 3 */
			*cindexptr = j*nx+i+nx; cindexptr++; 	/* 2 */
			*cindexptr = -1; cindexptr++;
#endif

		}
	}

	/* tex coords These need to be streamed now; that means for each quad, each vertex needs its tex coords. */
	/* if the texCoord node exists, let render_TextureCoordinate (or whatever the node is) do our work for us */
	if (!(node->texCoord)) {
		for (j = 0; j < (nz -1); j++) {
			for (i=0; i < (nx-1) ; i++) {
				/* first triangle, 3 vertexes */
#ifdef WINDING_ELEVATIONGRID
				/* first tri */
/* 1 */				*tcoord = ((float) (i+0)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+0)/(nz-1)); tcoord ++; 
			
/* 2 */				*tcoord = ((float) (i+0)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+1)/(nz-1)); tcoord ++; 
	
/* 3 */				*tcoord = ((float) (i+1)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+1)/(nz-1)); tcoord ++; 
	
				/* second tri */
/* 1 */				*tcoord = ((float) (i+0)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+0)/(nz-1)); tcoord ++; 
	
/* 3 */				*tcoord = ((float) (i+1)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+1)/(nz-1)); tcoord ++; 
	
/* 4 */				*tcoord = ((float) (i+1)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+0)/(nz-1)); tcoord ++; 
#else
				/* first tri */
/* 1 */				*tcoord = ((float) (i+0)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+0)/(nz-1)); tcoord ++; 
			
/* 4 */				*tcoord = ((float) (i+1)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+0)/(nz-1)); tcoord ++; 

/* 3 */				*tcoord = ((float) (i+1)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+1)/(nz-1)); tcoord ++; 
	
				/* second tri */
/* 1 */				*tcoord = ((float) (i+0)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+0)/(nz-1)); tcoord ++; 

/* 3 */				*tcoord = ((float) (i+1)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+1)/(nz-1)); tcoord ++; 
	
/* 2 */				*tcoord = ((float) (i+0)/(nx-1)); tcoord++;
				*tcoord = ((float)(j+1)/(nz-1)); tcoord ++; 
			
#endif
			}
		}
	}
			
	/* Render_Polyrep will use this number of triangles */
	rep->ntri = ntri;

	/* initialize arrays used for passing values into/out of the MOVE_TO_ORIGIN(node) values */
	mIN.n = nx * nz; 
	mIN.p = (struct SFVec3d *)MALLOC (sizeof (struct SFVec3d) * mIN.n);

        mOUT.n=0; mOUT.p = NULL;
        gdCoords.n=0; gdCoords.p = NULL;

	/* make up a series of points, then go and convert them to local coords */
	for (j=0; j<nz; j++) {
		for (i=0; i < nx; i++) {
		
			#ifdef VERBOSE
		 	printf ("		%lf %lf %lf # (hei ind %d) point [%d, %d]\n",
				xSp * i,
				height[i+(j*nx)] * ((double)node->yScale),
				zSp * j,
				i+(j*nx), i,j);
			#endif
		
		
			/* Make up a new vertex. Add the geoGridOrigin to every point */

			if ((mySRF == GEOSP_GD) || (mySRF == GEOSP_UTM)) {
				/* GD - give it to em in Latitude/Longitude/Elevation order */
				/* UTM- or give it to em in Northing/Easting/Elevation order */
				/* latitude - range of -90 to +90 */
				mIN.p[i+(j*nx)].c[0] = zSp * j + node->geoGridOrigin.c[0]; 
	
				/* longitude - range -180 to +180, or 0 to 360 */
				mIN.p[i+(j*nx)].c[1] =xSp * i + node->geoGridOrigin.c[1];
	
				/* elevation, above geoid */
				mIN.p[i+(j*nx)].c[2] = (height[i+(j*nx)] *(node->yScale)) + node->geoGridOrigin.c[2]
					+ myHeightAboveEllip; 
			} else {
				/* nothing quite specified here - what do we really do??? */
				mIN.p[i+(j*nx)].c[0] = zSp * j + node->geoGridOrigin.c[0]; 
	
				mIN.p[i+(j*nx)].c[1] =xSp * i + node->geoGridOrigin.c[1];
	
				mIN.p[i+(j*nx)].c[2] = (height[i+(j*nx)] *(node->yScale)) + node->geoGridOrigin.c[2]
					+ myHeightAboveEllip; 

			}
			/* printf ("height made up of %lf, geoGridOrigin %lf, myHeightAboveEllip %lf\n",(height[i+(j*nx)] *(node->yScale)),node->geoGridOrigin.c[2], myHeightAboveEllip); */
		}
	}
	#ifdef VERBOSE
	printf ("points before moving origin:\n");
	for (j=0; j<nz; j++) {
		for (i=0; i < nx; i++) {
			printf ("	%lf %lf %lf # lat/long/height before MOVE, index %d\n",mIN.p[i+(j*nx)].c[0],
				mIN.p[i+(j*nx)].c[1],mIN.p[i+(j*nx)].c[2],i+(j*nx));

		}
	}
	#endif

	/* convert this point to a local coordinate */
        MOVE_TO_ORIGIN(node)

	/* copy the resulting array back to the ElevationGrid */

	#ifdef VERBOSE
	printf ("points:\n");
	#endif

	for (j=0; j<nz; j++) {
		for (i=0; i < nx; i++) {
		/* copy this coordinate into our ElevationGrid array */
		newpoints[0] = (float) mOUT.p[i+(j*nx)].c[0];
		newpoints[1] = (float) mOUT.p[i+(j*nx)].c[1];
		newpoints[2] = (float) mOUT.p[i+(j*nx)].c[2];

		#ifdef VERBOSE
		printf ("	%f %f %f # converted, index %d\n",newpoints[0],newpoints[1],newpoints[2],i+(j*nx));
		#endif

		newpoints += 3;
		}
	}
	FREE_MF_SF_TEMPS
	return TRUE;
}


/* a GeoElevationGrid creates a "real" elevationGrid node as a child for rendering. */
void compile_GeoElevationGrid (struct X3D_GeoElevationGrid * node) {

	#ifdef VERBOSE
	printf ("compiling GeoElevationGrid\n");
	#endif
	printf ("compiling GeoElevationGrid\n");

	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	MARK_NODE_COMPILED
	
	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoElevationGrid, metadata))

}


void render_GeoElevationGrid (struct X3D_GeoElevationGrid *node) {
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_POLY_IF_REQUIRED (NULL, node->color, node->normal, node->texCoord) 
	CULL_FACE(node->solid)
	render_polyrep(node);
}

/************************************************************************/
/* GeoLocation								*/
/************************************************************************/

void compile_GeoLocation (struct X3D_GeoLocation * node) {
	MF_SF_TEMPS

	#ifdef VERBOSE
	printf ("compiling GeoLocation\n");
	#endif

	/* work out the position */
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	INIT_MF_FROM_SF(node, geoCoords)
	MOVE_TO_ORIGIN(node)
	COPY_MF_TO_SF(node, __movedCoords)

	/* work out the local orientation */
	GeoOrient(&node->__geoSystem, &gdCoords.p[0], &node->__localOrient);

	#ifdef VERBOSE
	printf ("compile_GeoLocation, orig coords %lf %lf %lf, moved %lf %lf %lf\n", node->geoCoords.c[0], node->geoCoords.c[1], node->geoCoords.c[2], node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
	printf ("	rotation is %lf %lf %lf %lf\n",
			node->__localOrient.c[0],
			node->__localOrient.c[1],
			node->__localOrient.c[2],
			node->__localOrient.c[3]);
	#endif

	/* did the geoCoords change?? */
	MARK_SFVEC3D_INOUT_EVENT(node->geoCoords, node->__oldgeoCoords, offsetof (struct X3D_GeoLocation, geoCoords))

	/* how about the children field ?? */
	MARK_MFNODE_INOUT_EVENT(node->children, node->__oldChildren, offsetof (struct X3D_GeoLocation, children))


	MARK_NODE_COMPILED
	FREE_MF_SF_TEMPS
	
	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoLocation, metadata))

	INITIALIZE_EXTENT;

	#ifdef VERBOSE
	printf ("compiled GeoLocation\n\n");
	#endif
}

void child_GeoLocation (struct X3D_GeoLocation *node) {
	CHILDREN_COUNT
	LOCAL_LIGHT_SAVE
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

	OCCLUSIONTEST


	/* {
		int x;
		struct X3D_Node *xx;

		printf ("child_GeoLocation, this %d \n",node);
		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->children.p[x]);
			printf ("	ch %d type %s dist %f\n",node->children.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	} */

	/* Check to see if we have to check for collisions for this transform. */

	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* do we have a local for a child? */
	LOCAL_LIGHT_CHILDREN(node->children);

	/* now, just render the non-directionalLight children */

	/* printf ("GeoLocation %d, flags %d, render_sensitive %d\n",
			node,node->_renderFlags,render_sensitive); */

	#ifdef CHILDVERBOSE
		printf ("GeoLocation - doing normalChildren\n");
	#endif

	normalChildren(node->children);

	#ifdef CHILDVERBOSE
		printf ("GeoLocation - done normalChildren\n");
	#endif

	LOCAL_LIGHT_OFF
}

void changed_GeoLocation ( struct X3D_GeoLocation *node) { 
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
}

/* do transforms, calculate the distance */
void prep_GeoLocation (struct X3D_GeoLocation *node) {
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

        /* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
         * so we do nothing here in that case -ncoder */

	/* printf ("prep_GeoLocation, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	OCCLUSIONTEST

	if(!render_vp) {
		double my_rotation;

		FW_GL_PUSH_MATRIX();

		/* TRANSLATION */
		FW_GL_TRANSLATE_D(node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);

		printf ("prep_GeoLoc trans to %lf %lf %lf\n",node->__movedCoords.c[0],node->__movedCoords.c[1],node->__movedCoords.c[2]);

		my_rotation = node->__localOrient.c[3]/3.1415926536*180;
		FW_GL_ROTATE_D(my_rotation, node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);

		/*
		printf ("geoLocation trans %7.4f %7.4f %7.4f\n",node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
		printf ("geoLocation rotat %7.4f %7.4f %7.4f %7.4f\n",my_rotation, node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);
		*/

		/* did either we or the Viewpoint move since last time? */
		RECORD_DISTANCE
        }
}
void fin_GeoLocation (struct X3D_GeoLocation *node) {
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
	OCCLUSIONTEST

        if(!render_vp) {
            FW_GL_POP_MATRIX();
        } else {
		double my_rotation;

		if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
		my_rotation = -(node->__localOrient.c[3]/3.1415926536*180);
		FW_GL_ROTATE_D(my_rotation, node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);
		FW_GL_TRANSLATE_D(-node->__movedCoords.c[0], -node->__movedCoords.c[1], -node->__movedCoords.c[2]);

		}
        }
}

/************************************************************************/
/* GeoLOD								*/
/************************************************************************/

void changed_GeoLOD (struct X3D_GeoLOD *node) {
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
	INITIALIZE_EXTENT;
}

#define LOAD_CHILD(childNode,childUrl) \
		/* printf ("start of LOAD_CHILD, url has %d strings\n",node->childUrl.n); */ \
		if (node->childUrl.n > 0) { \
			/* create new inline node, link it in */ \
			if (node->childNode == NULL) { \
				node->childNode = createNewX3DNode(NODE_Inline); \
				ADD_PARENT(X3D_NODE(node->childNode), X3D_NODE(node)); \
 			}\
			/* copy over the URL from parent */ \
			X3D_INLINE(node->childNode)->url.p = MALLOC(sizeof(struct Uni_String)*node->childUrl.n); \
			for (i=0; i<node->childUrl.n; i++) { \
				/* printf ("copying over url %s\n",node->childUrl.p[i]->strptr); */ \
				X3D_INLINE(node->childNode)->url.p[i] = newASCIIString(node->childUrl.p[i]->strptr); \
			} \
			/* printf ("loading, and urlCount is %d\n",node->childUrl.n); */ \
			X3D_INLINE(node->childNode)->url.n = node->childUrl.n; \
			X3D_INLINE(node->childNode)->load = TRUE; \
		}  \


static void GeoLODchildren (struct X3D_GeoLOD *node) {
	int load = node->__inRange;
	int i;

        /* lets see if we still have to load this one... */
        if (((node->__childloadstatus)==0) && (load)) {
		#ifdef VERBOSE
		printf ("GeoLODchildren - have to LOAD_CHILD for node %u (level %d)\n",node,geoLodLevel); 
		#endif

		LOAD_CHILD(__child1Node,child1Url)
		LOAD_CHILD(__child2Node,child2Url)
		LOAD_CHILD(__child3Node,child3Url)
		LOAD_CHILD(__child4Node,child4Url)
                node->__childloadstatus = 1;
	}
}

static void GeoUnLODchildren (struct X3D_GeoLOD *node) {
	int load = node->__inRange;

        if (!(load) && ((node->__childloadstatus) != 0)) {
		#ifdef VERBOSE
                printf ("GeoLODloadChildren, removing children from node %u level %d\n",node,geoLodLevel);
		#endif

                node->__childloadstatus = 0;
        }
}


static void GeoLODrootUrl (struct X3D_GeoLOD *node) {
	int load = node->__inRange;
	int i;

        /* lets see if we still have to load this one... */
        if (((node->__rooturlloadstatus)==0) && (load)) {
		#ifdef VERBOSE
		printf ("GeoLODrootUrl - have to LOAD_CHILD for node %u\n",node); 
		#endif

		LOAD_CHILD(__rootUrl, rootUrl)
                node->__rooturlloadstatus = 1;
	}
}


static void GeoUnLODrootUrl (struct X3D_GeoLOD *node) {
	int load = node->__inRange;

        if (!(load) && ((node->__rooturlloadstatus) != 0)) {
		#ifdef VERBOSE
                printf ("GeoLODloadChildren, removing rootUrl\n");
		#endif
                node->__childloadstatus = 0;
        }
}



void compile_GeoLOD (struct X3D_GeoLOD * node) {
	MF_SF_TEMPS

	#ifdef VERBOSE
	printf ("compiling GeoLOD %u\n",node);
	#endif

	/* work out the position */
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	INIT_MF_FROM_SF(node, center)
	MOVE_TO_ORIGIN(node)
	COPY_MF_TO_SF(node, __movedCoords)

	#ifdef VERBOSE
	printf ("compile_GeoLOD %u, orig coords %lf %lf %lf, moved %lf %lf %lf\n", node, node->center.c[0], node->center.c[1], node->center.c[2], node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);

	printf ("children.n %d childurl 1: %u 2: %u 3: %u 4: %u rootUrl: %u rootNode: %d\n",
	node->children,
	node->child1Url,
	node->child2Url,
	node->child3Url,
	node->child4Url,
	node->rootUrl,
	node->rootNode.n);
	#endif

	MARK_NODE_COMPILED
	FREE_MF_SF_TEMPS
	
	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoLOD, metadata))


	#ifdef VERBOSE
	printf ("compiled GeoLOD\n\n");
	#endif
}


void child_GeoLOD (struct X3D_GeoLOD *node) {
        int i;
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

	#ifdef VERBOSE
	 printf ("child_GeoLOD %u (level %d), renderFlags %x render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	node,
	geoLodLevel, 
	node->_renderFlags,
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); 
	#endif

	/* for debugging purposes... */
	if (node->__level == -1) node->__level = geoLodLevel;
	else if (node->__level != geoLodLevel) {
		printf ("hmmm - GeoLOD %u was level %d, now %d\n",(unsigned int) node,node->__level, geoLodLevel);
	}

	#ifdef VERBOSE
	if ( node->__inRange) {
		printf ("GeoLOD %u (level %d) closer\n",node,geoLodLevel);
	} else {
		printf ("GeoLOD %u (level %d) farther away\n",node,geoLodLevel);
	}
	#endif

	/* if we are out of range, use the rootNode or rootUrl field 	*/
	/* else, use the child1Url through the child4Url fields 	*/
	if (!(node->__inRange)) {
		/* printf ("GeoLOD, node %u, doing rootNode, rootNode.n = %d\n",node,node->rootNode.n); */
		/* do we need to unload children that are no longer needed? */
		GeoUnLODchildren (node);

		if (node->rootNode.n != 0)  {
			for (i=0; i<node->rootNode.n; i++) {
				#ifdef VERBOSE
				printf ("GeoLOD %u is rendering rootNode %u",node,node->rootNode.p[i]);
				if (node->rootNode.p[i]!=NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->rootNode.p[i])->_nodeType));
				printf("\n");
				#endif

				render_node (node->rootNode.p[i]);
			}	
		} else if (node->rootUrl.n != 0) {

			/* try and load the root from the rootUrl */
			GeoLODrootUrl (node);

			/* render this rootUrl */
			if (node->__rootUrl != NULL) {
				#ifdef VERBOSE
				printf ("GeoLOD %u is rendering rootUrl %u",node,node->__rootUrl);
				if (node->__rootUrl != NULL) printf (" (%s) ", stringNodeType(X3D_NODE(node->__rootUrl)->_nodeType));
				printf ("\n");
				#endif

				render_node (node->__rootUrl);
			}	
			
			
		}
	} else {
		geoLodLevel++;

		/* go through 4 kids */
		GeoLODchildren (node);

		/* get rid of the rootUrl node, if it is loaded */
		GeoUnLODrootUrl (node);

		#ifdef VERBOSE
		printf ("rendering children at %d, they are: ",geoLodLevel);
		if (node->child1Url.n>0) printf (" :%s: ",node->child1Url.p[0]->strptr);
		if (node->child2Url.n>0) printf (" :%s: ",node->child2Url.p[0]->strptr);
		if (node->child3Url.n>0) printf (" :%s: ",node->child3Url.p[0]->strptr);
		if (node->child4Url.n>0) printf (" :%s: ",node->child4Url.p[0]->strptr);
		printf ("\n");
		#endif

		/* render these children */
		#ifdef VERBOSE
		printf ("GeoLOD %u is rendering children %u ", node, node->__child1Node);
		if (node->__child1Node != NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->__child1Node)->_nodeType));
		printf (" %u ", node->__child2Node);
		if (node->__child2Node != NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->__child2Node)->_nodeType));
		printf (" %u ", node->__child3Node);
		if (node->__child3Node != NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->__child3Node)->_nodeType));
		printf (" %u ", node->__child4Node);
		if (node->__child4Node != NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->__child4Node)->_nodeType));
		printf ("\n");
		#endif

		if (node->__child1Node != NULL) render_node (node->__child1Node);
		if (node->__child2Node != NULL) render_node (node->__child2Node);
		if (node->__child3Node != NULL) render_node (node->__child3Node);
		if (node->__child4Node != NULL) render_node (node->__child4Node);
		geoLodLevel--;

	}
}

/************************************************************************/
/* GeoMetaData								*/
/************************************************************************/

void compile_GeoMetadata (struct X3D_GeoMetadata * node) {
	#ifdef VERBOSE
	printf ("compiling GeoMetadata\n");

	#endif

	MARK_NODE_COMPILED
}

/************************************************************************/
/* GeoOrigin								*/
/************************************************************************/

void compile_GeoOrigin (struct X3D_GeoOrigin * node) {
	#ifdef VERBOSE
	printf ("compiling GeoOrigin\n");
	#endif

	printf ("compiling GeoOrigin\n");
	/* INITIALIZE_GEOSPATIAL */
	COMPILE_GEOSYSTEM(node)
	MARK_NODE_COMPILED

	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoOrigin, metadata))
	MARK_SFVEC3D_INOUT_EVENT(node->geoCoords,node->__oldgeoCoords,offsetof (struct X3D_GeoOrigin, geoCoords))
	MARK_MFSTRING_INOUT_EVENT(node->geoSystem,node->__oldMFString,offsetof (struct X3D_GeoOrigin, geoSystem))
}

/************************************************************************/
/* GeoPositionInterpolator						*/
/************************************************************************/

void compile_GeoPositionInterpolator (struct X3D_GeoPositionInterpolator * node) {
	MF_SF_TEMPS

	#ifdef VERBOSE
	printf ("compiling GeoPositionInterpolator\n");
	#endif

	/* standard MACROS expect specific field names */
	mIN = node->keyValue;
	mOUT.p = NULL; mOUT.n = 0;


	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	MOVE_TO_ORIGIN(node)

	
	/* keep the output values of this process */
	FREE_IF_NZ(node->__movedValue.p);
	node->__movedValue.p = mOUT.p;
	node->__movedValue.n = mOUT.n;

	FREE_IF_NZ(gdCoords.p);
	MARK_NODE_COMPILED
	
	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoPositionInterpolator, metadata))
}

/* PositionInterpolator, ColorInterpolator, GeoPositionInterpolator	*/
/* Called during the "events_processed" section of the event loop,	*/
/* so this is called ONLY when there is something required to do, thus	*/
/* there is no need to look at whether it is active or not		*/

/* GeoPositionInterpolator == PositionIterpolator but with geovalue_changed and coordinate conversions */
void do_GeoPositionInterpolator (void *innode) {
	struct X3D_GeoPositionInterpolator *node;
	int kin, kvin, counter, tmp;
	struct SFVec3d *kVs;
	/* struct SFColor *kVs */

	if (!innode) return;
	node = (struct X3D_GeoPositionInterpolator *) innode;

	if (NODE_NEEDS_COMPILING) compile_GeoPositionInterpolator(node);
	kvin = node->__movedValue.n;
	kVs = node->__movedValue.p;
	kin = node->key.n;
	MARK_EVENT (innode, offsetof (struct X3D_GeoPositionInterpolator, value_changed)); 
	MARK_EVENT (innode, offsetof (struct X3D_GeoPositionInterpolator, geovalue_changed)); 

	/* did the key or keyValue change? */
	if (node->__oldKeyValuePtr != node->keyValue.p) {
		MARK_EVENT (innode, offsetof (struct X3D_GeoPositionInterpolator, keyValue)); 
		node->__oldKeyValuePtr = node->keyValue.p;
	}
	if (node->__oldKeyPtr != node->key.p) {
		MARK_EVENT (innode, offsetof (struct X3D_GeoPositionInterpolator, key)); 
		node->__oldKeyPtr = node->key.p;
	}


	#ifdef SEVERBOSE
		printf("do_GeoPos: Position/Color interp, node %u kin %d kvin %d set_fraction %f\n",
			   node, kin, kvin, node->set_fraction);
	#endif

	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		node->value_changed.c[0] = 0.0;
		node->value_changed.c[1] = 0.0;
		node->value_changed.c[2] = 0.0;
		node->geovalue_changed.c[0] = 0.0;
		node->geovalue_changed.c[1] = 0.0;
		node->geovalue_changed.c[2] = 0.0;
		return;
	}

	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */

	/* set_fraction less than or greater than keys */
	if (node->set_fraction <= ((node->key).p[0])) {
		memcpy ((void *)&node->geovalue_changed, (void *)&kVs[0], sizeof (struct SFVec3d));
	} else if (node->set_fraction >= node->key.p[kin-1]) {
		memcpy ((void *)&node->geovalue_changed, (void *)&kVs[kvin-1], sizeof (struct SFVec3d));
	} else {
		/* have to go through and find the key before */
		counter = find_key(kin,((float)(node->set_fraction)),node->key.p);
		for (tmp=0; tmp<3; tmp++) {
			node->geovalue_changed.c[tmp] =
				(node->set_fraction - node->key.p[counter-1]) /
				(node->key.p[counter] - node->key.p[counter-1]) *
				(kVs[counter].c[tmp] - kVs[counter-1].c[tmp]) + kVs[counter-1].c[tmp];
		}
	}

	/* convert this back into the requested spatial format */
	CONVERT_BACK_TO_GD_OR_UTM(node->geovalue_changed)

	/* set the (float) value_changed, as well */
	for (tmp=0;tmp<3;tmp++) node->value_changed.c[tmp] = (float)node->geovalue_changed.c[tmp];

	#ifdef SEVERBOSE
	printf ("Pos/Col, new value (%f %f %f)\n",
		node->value_changed.c[0],node->value_changed.c[1],node->value_changed.c[2]);
	printf ("geovalue_changed %lf %lf %lf\n",node->geovalue_changed.c[0], node->geovalue_changed.c[1], node->geovalue_changed.c[2]);
	#endif
}

/************************************************************************/
/* GeoProximitySensor							*/
/************************************************************************/

void compile_GeoProximitySensor (struct X3D_GeoProximitySensor * node) {
	MF_SF_TEMPS

	#ifdef VERBOSE
	printf ("compiling GeoProximitySensor\n");
	#endif

	/* work out the position */
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	INIT_MF_FROM_SF(node, geoCenter)
	MOVE_TO_ORIGIN(node)
	COPY_MF_TO_SF(node, __movedCoords)

	/* work out the local orientation */
	GeoOrient(&node->__geoSystem, &gdCoords.p[0], &node->__localOrient);
	#ifdef VERBOSE
	printf ("compile_GeoProximitySensor, orig coords %lf %lf %lf, moved %lf %lf %lf\n", node->geoCenter.c[0], node->geoCenter.c[1], node->geoCenter.c[2], node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
	printf ("	rotation is %lf %lf %lf %lf\n",
			node->__localOrient.c[0],
			node->__localOrient.c[1],
			node->__localOrient.c[2],
			node->__localOrient.c[3]);
	#endif

	MARK_NODE_COMPILED
	FREE_MF_SF_TEMPS

	MARK_SFVEC3D_INOUT_EVENT(node->geoCenter, node->__oldGeoCenter,offsetof (struct X3D_GeoProximitySensor, geoCenter))
	MARK_SFVEC3F_INOUT_EVENT(node->size, node->__oldSize,offsetof (struct X3D_GeoProximitySensor, size))
	
	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoProximitySensor, metadata))


	#ifdef VERBOSE
	printf ("compiled GeoProximitySensor\n\n");
	#endif
}

	PROXIMITYSENSOR(GeoProximitySensor,__movedCoords,INITIALIZE_GEOSPATIAL(node),COMPILE_IF_REQUIRED)


/* GeoProximitySensor code for ClockTick */
void do_GeoProximitySensorTick( void *ptr) {
	struct X3D_GeoProximitySensor *node = (struct X3D_GeoProximitySensor *)ptr;

	/* if not enabled, do nothing */
	if (!node) return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_GeoProximitySensor, enabled));
	}
	if (!node->enabled) return;

	/* isOver state */
	/* did we get a signal? */
	if (node->__hit) {
		if (!node->isActive) {
			#ifdef SEVERBOSE
			printf ("PROX - initial defaults\n");
			#endif

			node->isActive = 1;
			node->enterTime = TickTime;
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, isActive));
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, enterTime));

		}

		/* now, has anything changed? */
		if (memcmp ((void *) &node->position_changed,(void *) &node->__t1,sizeof(struct SFColor))) {
			#ifdef SEVERBOSE
			printf ("PROX - position changed!!! \n");
			#endif

			memcpy ((void *) &node->position_changed,
				(void *) &node->__t1,sizeof(struct SFColor));
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, position_changed));
		
			#ifdef VERBOSE
			printf ("do_GeoProximitySensorTick, position changed; it now is %lf %lf %lf\n",node->position_changed.c[0],
				node->position_changed.c[1], node->position_changed.c[2]);
			printf ("nearPlane is %lf\n",nearPlane);

			#endif

			/* possibly we have to convert this from GCC to GDC, and maybe even then to UTM */
		
			/* prep the geoCoord changed; first, get the position. Right now, we use the
			  Viewer position, as it is more accurate (not clipped by the nearPlane) than
			  the position_changed field  */

			node->geoCoord_changed.c[0] = (double) node->position_changed.c[0];
			node->geoCoord_changed.c[1] = (double) node->position_changed.c[1];
			node->geoCoord_changed.c[2] = (double) node->position_changed.c[2];

			/* then add in the nearPlane, as the way we get the position is via a clipped frustum */
			/* if we get this via the position_changed field, we have to:
				node->geoCoord_changed.c[2] += nearPlane;
			*/
			node->geoCoord_changed.c[2] += nearPlane;
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, geoCoord_changed));

			#ifdef VERBOSE
			printf ("\ngeoCoord_changed as a GCC, %lf %lf %lf\n",
				node->geoCoord_changed.c[0],
				node->geoCoord_changed.c[1],
				node->geoCoord_changed.c[2]);
			#endif

			CONVERT_BACK_TO_GD_OR_UTM(node->geoCoord_changed)
		}
		if (memcmp ((void *) &node->orientation_changed, (void *) &node->__t2,sizeof(struct SFRotation))) {
			#ifdef SEVERBOSE
			printf  ("PROX - orientation changed!!!\n ");
			#endif

			memcpy ((void *) &node->orientation_changed,
				(void *) &node->__t2,sizeof(struct SFRotation));
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, orientation_changed));
		}
	} else {
		if (node->isActive) {
			#ifdef SEVERBOSE
			printf ("PROX - stopping\n");
			#endif

			node->isActive = 0;
			node->exitTime = TickTime;
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, isActive));

			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, exitTime));
		}
	}
	node->__hit=FALSE;
}


/************************************************************************/
/* GeoTouchSensor							*/
/************************************************************************/

void compile_GeoTouchSensor (struct X3D_GeoTouchSensor * node) {
	#ifdef VERBOSE
	printf ("compiling GeoTouchSensor\n");
	#endif

	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	MARK_NODE_COMPILED

	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoTouchSensor, metadata))

}

void do_GeoTouchSensor ( void *ptr, int ev, int but1, int over) {


	struct X3D_GeoTouchSensor *node = (struct X3D_GeoTouchSensor *)ptr;
	struct point_XYZ normalval;	/* different structures for normalization calls */

	COMPILE_IF_REQUIRED

	#ifdef SENSVERBOSE
	printf ("%lf: TS ",TickTime);
	if (ev==ButtonPress) printf ("ButtonPress ");
	else if (ev==ButtonRelease) printf ("ButtonRelease ");
	else if (ev==KeyPress) printf ("KeyPress ");
	else if (ev==KeyRelease) printf ("KeyRelease ");
	else if (ev==MotionNotify) printf ("%lf MotionNotify ");
	else printf ("ev %d ",ev);
	
	if (but1) printf ("but1 TRUE "); else printf ("but1 FALSE ");
	if (over) printf ("over TRUE "); else printf ("over FALSE ");
	printf ("\n");
	#endif

	/* if not enabled, do nothing */
	if (!node) return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_GeoTouchSensor, enabled));
	}
	if (!node->enabled) return;

	/* isOver state */
	if ((ev == overMark) && (over != node->isOver)) {
		#ifdef SENSVERBOSE
		printf ("TS %u, isOver changed %d\n",node, over);
		#endif
		node->isOver = over;
		MARK_EVENT (ptr, offsetof (struct X3D_GeoTouchSensor, isOver));
	}

	/* active */
	/* button presses */
	if (ev == ButtonPress) {
		node->isActive=1;
		MARK_EVENT (ptr, offsetof (struct X3D_GeoTouchSensor, isActive));
		#ifdef SENSVERBOSE
		printf ("touchSens %u, butPress\n",node);
		#endif

		node->touchTime = TickTime;
		MARK_EVENT(ptr, offsetof (struct X3D_GeoTouchSensor, touchTime));

	} else if (ev == ButtonRelease) {
		#ifdef SENSVERBOSE
		printf ("touchSens %u, butRelease\n",node);
		#endif
		node->isActive=0;
		MARK_EVENT (ptr, offsetof (struct X3D_GeoTouchSensor, isActive));
	}

	/* hitPoint and hitNormal */
	/* save the current hitPoint for determining if this changes between runs */
	memcpy ((void *) &node->_oldhitPoint, (void *) &ray_save_posn,sizeof(struct SFColor));

	/* did the hitPoint change between runs? */
	if ((APPROX(node->_oldhitPoint.c[0],node->hitPoint_changed.c[0])!= TRUE) ||
		(APPROX(node->_oldhitPoint.c[1],node->hitPoint_changed.c[1])!= TRUE) ||
		(APPROX(node->_oldhitPoint.c[2],node->hitPoint_changed.c[2])!= TRUE)) {

		#ifdef SENSVERBOSE
		printf ("GeoTouchSens, hitPoint changed: %f %f %f\n",node->hitPoint_changed.c[0],
			node->hitPoint_changed.c[1], node->hitPoint_changed.c[2]);
		#endif

		memcpy ((void *) &node->hitPoint_changed, (void *) &node->_oldhitPoint, sizeof(struct SFColor));
		MARK_EVENT(ptr, offsetof (struct X3D_GeoTouchSensor, hitPoint_changed));

		/* convert this back into the requested GeoSpatial format... */
			node->hitGeoCoord_changed.c[0] = (double) node->hitPoint_changed.c[0];
			node->hitGeoCoord_changed.c[1] = (double) node->hitPoint_changed.c[1];
			node->hitGeoCoord_changed.c[2] = (double) node->hitPoint_changed.c[2];

			/* then add in the nearPlane, as the way we get the position is via a clipped frustum */
			/* if we get this via the position_changed field, we have to:
				node->hitGeoCoord_changed.c[2] += nearPlane;
			*/
			node->hitGeoCoord_changed.c[2] += nearPlane;
			MARK_EVENT (ptr, offsetof(struct X3D_GeoTouchSensor, hitGeoCoord_changed));

			#ifdef SENSVERBOSE
			printf ("\nhitGeoCoord_changed as a GCC, %lf %lf %lf\n",
				node->hitGeoCoord_changed.c[0],
				node->hitGeoCoord_changed.c[1],
				node->hitGeoCoord_changed.c[2]);
			#endif

			CONVERT_BACK_TO_GD_OR_UTM(node->hitGeoCoord_changed)
	}

	/* have to normalize normal; change it from SFColor to struct point_XYZ. */
	normalval.x = hyp_save_norm.c[0];
	normalval.y = hyp_save_norm.c[1];
	normalval.z = hyp_save_norm.c[2];
	normalize_vector(&normalval);
	node->_oldhitNormal.c[0] = normalval.x;
	node->_oldhitNormal.c[1] = normalval.y;
	node->_oldhitNormal.c[2] = normalval.z;

	/* did the hitNormal change between runs? */
	MARK_SFVEC3F_INOUT_EVENT(node->hitNormal_changed,node->_oldhitNormal,offsetof (struct X3D_GeoTouchSensor, hitNormal_changed))
} 



/************************************************************************/
/* GeoViewpoint								*/
/************************************************************************/

void compile_GeoViewpoint (struct X3D_GeoViewpoint * node) {
	struct SFVec4d localOrient;
	struct SFVec4d orient;
	int i;
	Quaternion localQuat;
	Quaternion relQuat;
	Quaternion combQuat;

	MF_SF_TEMPS

	#ifdef VERBOSE
	printf ("compileViewpoint is %u, its geoOrigin is %u \n",node, node->geoOrigin);
	if (node->geoOrigin!=NULL) printf ("type %s\n",stringNodeType(X3D_GEOORIGIN(node->geoOrigin)->_nodeType));
	#endif


	/* did any of the "set_" inputOnly fields get set?  if not, just use the non-set fields */
	USE_SET_SFVEC3D_IF_CHANGED(set_position,position)
	USE_SET_SFROTATION_IF_CHANGED(set_orientation,orientation)  

	/* work out the position */
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	INIT_MF_FROM_SF(node, position)
	MOVE_TO_ORIGIN(node)
	COPY_MF_TO_SF(node, __movedPosition)


	/* work out the local orientation and copy doubles to floats */
	GeoOrient(&node->__geoSystem, &gdCoords.p[0], &localOrient);

	/* Quaternize the local Geospatial quaternion, and the specified rotation from the GeoViewpoint orientation field */
	vrmlrot_to_quaternion (&localQuat, localOrient.c[0], localOrient.c[1], localOrient.c[2], localOrient.c[3]);
	vrmlrot_to_quaternion (&relQuat, node->orientation.c[0], node->orientation.c[1], node->orientation.c[2], node->orientation.c[3]);

	/* add these together */
        quaternion_add (&combQuat, &relQuat, &localQuat);

	/* get the rotation; 2 steps to convert doubles to floats;
           should be quaternion_to_vrmlrot(&combQuat, &node->__movedOrientation.c[0]... */
        quaternion_to_vrmlrot(&combQuat, &orient.c[0], &orient.c[1], &orient.c[2], &orient.c[3]);
	for (i=0; i<4; i++) node->__movedOrientation.c[i] = (float) orient.c[i];

        #ifdef VERBOSE
	printf ("compile_GeoViewpoint, final position %lf %lf %lf\n",node->__movedPosition.c[0],
		node->__movedPosition.c[1], node->__movedPosition.c[2]);

	printf ("compile_GeoViewpoint, getLocalOrientation %lf %lf %lf %lf\n",localOrient.c[0],
		localOrient.c[1], localOrient.c[2], localOrient.c[3]);
	printf ("compile_GeoViewpoint, initial orientation: %lf %lf %lf %lf\n",node->orientation.c[0],
		node->orientation.c[1], node->orientation.c[2], node->orientation.c[3]);
	printf ("compile_GeoViewpoint, final rotation %lf %lf %lf %lf\n",node->__movedOrientation.c[0], 
		node->__movedOrientation.c[1], node->__movedOrientation.c[2], node->__movedOrientation.c[3]);
	printf ("compile_GeoViewpoint, elevation from the WGS84 ellipsoid is %lf\n",gdCoords.p[0].c[2]);
        #endif

	MARK_NODE_COMPILED
	FREE_MF_SF_TEMPS
	
	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoViewpoint, metadata))
	MARK_SFFLOAT_INOUT_EVENT(node->fieldOfView, node->__oldFieldOfView, offsetof (struct X3D_GeoViewpoint, fieldOfView))
	MARK_SFBOOL_INOUT_EVENT(node->headlight, node->__oldHeadlight, offsetof (struct X3D_GeoViewpoint, headlight))
	MARK_SFBOOL_INOUT_EVENT(node->jump, node->__oldJump, offsetof (struct X3D_GeoViewpoint, jump))
	MARK_SFSTRING_INOUT_EVENT(node->description,node->__oldSFString, offsetof(struct X3D_GeoViewpoint, description))
	MARK_MFSTRING_INOUT_EVENT(node->navType,node->__oldMFString, offsetof(struct X3D_GeoViewpoint, navType))

	#ifdef VERBOSE
	printf ("compiled GeoViewpoint\n\n");
	#endif
}

void prep_GeoViewpoint (struct X3D_GeoViewpoint *node) {
	double a1;

	if (!render_vp) return;

	INITIALIZE_GEOSPATIAL(node)

	 /* printf ("RVP, node %d ib %d sb %d gepvp\n",node,node->isBound,node->set_bind);
	 printf ("VP stack %d tos %d\n",viewpoint_tos, viewpoint_stack[viewpoint_tos]);
	 */

	/* check the set_bind eventin to see if it is TRUE or FALSE */
	/* code to perform binding is now in set_viewpoint. */

	COMPILE_IF_REQUIRED

	#ifdef VERBOSE
	printf ("prep_GeoViewpoint called\n");
	#endif

	/* perform GeoViewpoint translations */
	FW_GL_ROTATE_D(-node->__movedOrientation.c[3]/PI*180.0,node->__movedOrientation.c[0],node->__movedOrientation.c[1],
		node->__movedOrientation.c[2]); 

	FW_GL_TRANSLATE_D(-node->__movedPosition.c[0],-node->__movedPosition.c[1],-node->__movedPosition.c[2]);

	/* we have  a new currentPosInModel now... */
	/* printf ("currentPosInModel was %lf %lf %lf\n", Viewer.currentPosInModel.x, Viewer.currentPosInModel.y, Viewer.currentPosInModel.z); */

	/* the AntiPos has been applied in the trans and rots above, so we do not need to do it here */
	getCurrentPosInModel(FALSE); 


	/* now, lets work on the GeoViewpoint fieldOfView */
	glGetIntegerv(GL_VIEWPORT, viewPort);
	if(viewPort[2] > viewPort[3]) {
		a1=0;
		fieldofview = node->fieldOfView/3.1415926536*180;
	} else {
		a1 = node->fieldOfView;
		a1 = atan2(sin(a1),viewPort[2]/((float)viewPort[3]) * cos(a1));
		fieldofview = a1/3.1415926536*180;
	}

	calculateViewingSpeed();
	#ifdef VERBOSE
	printf ("prep_GeoViewpoint, fieldOfView %f \n",node->fieldOfView); 
	#endif
}

/* GeoViewpoint speeds and avatar sizes are depenent on elevation above WGS_84. These are calculated here */
/* this is called from the Viewer functions */
static void calculateViewingSpeed() {
	struct SFVec3d gcCoords;
	struct SFVec3d gdCoords;
		
	/* the current position is the GC coordinate */
	gcCoords.c[0]= Viewer.currentPosInModel.x;
	gcCoords.c[1] = Viewer.currentPosInModel.y;
	gcCoords.c[2] = Viewer.currentPosInModel.z;
		
        #ifdef VERBOSE
        printf ("calculateViewingSpeed, currentPosInModel %lf %lf %lf\n", gcCoords.c[0], gcCoords.c[1], gcCoords.c[2]);
        #endif
		
	if (Viewer.GeoSpatialNode != NULL) {
		/* do we have a valid __geoSystem?? */
        INITIALIZE_GEOSPATIAL(Viewer.GeoSpatialNode)


#define COMPILE_IF_REQUIRED { struct X3D_Virt *v; \
        if (node->_ichange != node->_change) { \
                /* printf ("COMP %d %d\n",node->_ichange, node->_change); */ \
                v = *(struct X3D_Virt **)node; \
                if (v->compile) { \
                        compileNode (v->compile, (void *)node, NULL, NULL, NULL, NULL); \
                } else {printf ("huh - have COMPIFREQD, but v->compile null for %s\n",stringNodeType(node->_nodeType));} \
                } \
                if (node->_ichange == 0) return; \
        }

/*
        COMPILE_IF_REQUIRED

*/




		if (Viewer.GeoSpatialNode->__geoSystem.n>0) {
			/* is the __geoSystem NOT gc coords? */
			/* printf ("have a GeoSpatial viewpoint, currently %d\n",Viewer.GeoSpatialNode->__geoSystem.p[0]);  */
			if (Viewer.GeoSpatialNode->__geoSystem.p[0] != GEOSP_GC) {
		
/*
		        	retractOrigin(Viewer.GeoSpatialNode->geoOrigin, &gcCoords);
*/
		
		        	#ifdef VERBOSE
				printf ("\n");
				printf ("for GeoViewpoint :%s:\n",Viewer.GeoSpatialNode->description->strptr);
		        	printf ("calculateViewingSpeed,  currentPosInModel: %lf %lf %lf\n", gcCoords.c[0], gcCoords.c[1], gcCoords.c[2]);
		        	#endif
		
		        	/* convert from local (gc) to gd coordinates, using WGS84 ellipsoid */
		        	gccToGdc (&gcCoords, &gdCoords);
		
				#ifdef VERBOSE
				printf ("speed is calculated from geodetic height %lf %lf %lf\n",gdCoords.c[0], gdCoords.c[1], gdCoords.c[2]); 
				#endif
			
				/* speed is dependent on elevation above WGS84 ellipsoid */
				Viewer.speed  = fabs(sqrt(gcCoords.c[0]*gcCoords.c[0] + gcCoords.c[1]*gcCoords.c[1] + gcCoords.c[2]*gcCoords.c[2])
					-GEOSP_WE_A);
				if (Viewer.speed < 1.0) Viewer.speed=1.0;
				#ifdef VERBOSE
				printf ("height above center %f WGS84 ellipsoid is %lf\n",Viewer.speed,GEOSP_WE_A); 
				#endif
		
/*
				Viewer.speed = fabs(Viewer.speed * Viewer.GeoSpatialNode->speedFactor);
				if (Viewer.speed < Viewer.GeoSpatialNode->speedFactor) Viewer.speed = Viewer.GeoSpatialNode->speedFactor;
*/

				/* set the navigation info - use the GeoVRML algorithms */
				set_naviWidthHeightStep(
					Viewer.speed*0.25,
					Viewer.speed*1.6,
					Viewer.speed*0.25);
			}
		}
	}
}

static void calculateExamineModeDistance(void) {
extern int doExamineModeDistanceCalculations;
/*
	printf ("bind_geoviewpoint - calculateExamineModeDistance\n");
*/
doExamineModeDistanceCalculations = TRUE;

}

void bind_geoviewpoint (struct X3D_GeoViewpoint *node) {
	Quaternion q_i;

	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

	/* set Viewer position and orientation */

	#ifdef VERBOSE
	printf ("bind_geoviewpoint, setting Viewer to %lf %lf %lf orient %f %f %f %f\n",node->__movedPosition.c[0],node->__movedPosition.c[1],
	node->__movedPosition.c[2],node->orientation.c[0],node->orientation.c[1],node->orientation.c[2],
	node->orientation.c[3]);
	printf ("	node %u fieldOfView %f\n",node,node->fieldOfView);
	#endif

	Viewer.GeoSpatialNode = node;

	Viewer.Pos.x = node->__movedPosition.c[0];
	Viewer.Pos.y = node->__movedPosition.c[1];
	Viewer.Pos.z = node->__movedPosition.c[2];
	Viewer.AntiPos.x = node->__movedPosition.c[0];
	Viewer.AntiPos.y = node->__movedPosition.c[1];
	Viewer.AntiPos.z = node->__movedPosition.c[2];

	/* printf ("bind_geoviewpoint, pos %f %f %f antipos %f %f %f\n",Viewer.Pos.x, Viewer.Pos.y, Viewer.Pos.z, Viewer.AntiPos.x, Viewer.AntiPos.y, Viewer.AntiPos.z); */

	vrmlrot_to_quaternion (&Viewer.Quat,node->__movedOrientation.c[0],
		node->__movedOrientation.c[1],node->__movedOrientation.c[2],node->__movedOrientation.c[3]);

	vrmlrot_to_quaternion (&q_i,node->__movedOrientation.c[0],
		node->__movedOrientation.c[1],node->__movedOrientation.c[2],node->__movedOrientation.c[3]);
	quaternion_inverse(&(Viewer.AntiQuat),&q_i);

	resolve_pos();

	calculateViewingSpeed();

	calculateExamineModeDistance();

}


/************************************************************************/
/* GeoTransform								*/
/************************************************************************/

void compile_GeoTransform (struct X3D_GeoTransform * node) {
	MF_SF_TEMPS

	#ifdef VERBOSE
	printf ("compiling GeoLocation\n");
	#endif

	/* work out the position */
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	INIT_MF_FROM_SF(node, geoCenter)
	MOVE_TO_ORIGIN(node)
	COPY_MF_TO_SF(node, __movedCoords)

	/* work out the local orientation */
	GeoOrient(&node->__geoSystem, &gdCoords.p[0], &node->__localOrient);

	MARK_SFVEC3D_INOUT_EVENT(node->geoCenter, node->__oldGeoCenter,offsetof (struct X3D_GeoTransform, geoCenter))
	MARK_MFNODE_INOUT_EVENT(node->children, node->__oldChildren, offsetof (struct X3D_GeoTransform, children))


	/* re-figure out which modifiers are actually in use */
	/* printf ("re-rendering for %d\n",node);*/
	node->__do_trans = verify_translate ((GLfloat *)node->translation.c);
	if (node->__do_trans) MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoTransform, translation));

	node->__do_scale = verify_scale ((GLfloat *)node->scale.c);
	if (node->__do_scale) MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoTransform, scale));

	node->__do_rotation = verify_rotate ((GLfloat *)node->rotation.c);
	if (node->__do_rotation) MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoTransform, rotation));

	node->__do_scaleO = verify_rotate ((GLfloat *)node->scaleOrientation.c);
	if (node->__do_scaleO) MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoTransform, scaleOrientation));



	#ifdef VERBOSE
	printf ("compile_GeoTransform, orig coords %lf %lf %lf, moved %lf %lf %lf\n", node->geoCoords.c[0], node->geoCoords.c[1], node->geoCoords.c[2], node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
	printf ("	rotation is %lf %lf %lf %lf\n",
			node->__localOrient.c[0],
			node->__localOrient.c[1],
			node->__localOrient.c[2],
			node->__localOrient.c[3]);
	#endif

	MARK_NODE_COMPILED
	FREE_MF_SF_TEMPS
	
	/* events */
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoTransform, metadata))


	#ifdef VERBOSE
	printf ("compiled GeoTransform\n\n");
	#endif
}


/* do transforms, calculate the distance */
void prep_GeoTransform (struct X3D_GeoTransform *node) {
	GLfloat my_rotation;
	GLfloat my_scaleO=0;

	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

        /* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
         * so we do nothing here in that case -ncoder */

	/* printf ("prep_Transform, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	OCCLUSIONTEST

	if(!render_vp) {
		FW_GL_PUSH_MATRIX();


		/* TRANSLATION */
		if (node->__do_trans)
			FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

                /* GeoTransform TRANSLATION */
                FW_GL_TRANSLATE_D(node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
                
                printf ("prep_GeoLoc trans to %lf %lf %lf\n",node->__movedCoords.c[0],node->__movedCoords.c[1],node->__movedCoords.c[2]);
                
                        
                my_rotation = node->__localOrient.c[3]/3.1415926536*180;
                FW_GL_ROTATE_D(my_rotation, node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);
                
		/* ROTATION */
		if (node->__do_rotation) {
			my_rotation = node->rotation.c[3]/3.1415926536*180;
			FW_GL_ROTATE_F(my_rotation, node->rotation.c[0],node->rotation.c[1],node->rotation.c[2]);
		}

		/* SCALEORIENTATION */
		if (node->__do_scaleO) {
			my_scaleO = node->scaleOrientation.c[3]/3.1415926536*180;
			FW_GL_ROTATE_F(my_scaleO, node->scaleOrientation.c[0],
				node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
		}

		/* SCALE */
		if (node->__do_scale)
			FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

		/* REVERSE SCALE ORIENTATION */
		if (node->__do_scaleO)
			FW_GL_ROTATE_F(-my_scaleO, node->scaleOrientation.c[0],
				node->scaleOrientation.c[1],node->scaleOrientation.c[2]);

		/* REVERSE CENTER */
                FW_GL_TRANSLATE_D(-node->__movedCoords.c[0], -node->__movedCoords.c[1], -node->__movedCoords.c[2]);

		RECORD_DISTANCE
        }
}


void fin_GeoTransform (struct X3D_GeoTransform *node) {
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
	OCCLUSIONTEST

        if(!render_vp) {
            FW_GL_POP_MATRIX();
        } else {
           /*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
            if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
                FW_GL_TRANSLATE_D(((node->__movedCoords).c[0]),((node->__movedCoords).c[1]),((node->__movedCoords).c[2])
                );
                FW_GL_ROTATE_F(((node->scaleOrientation).c[3])/3.1415926536*180,((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
                );
                FW_GL_SCALE_F(1.0/(((node->scale).c[0])),1.0/(((node->scale).c[1])),1.0/(((node->scale).c[2]))
                );
                FW_GL_ROTATE_F(-(((node->scaleOrientation).c[3])/3.1415926536*180),((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
                );
                FW_GL_ROTATE_F(-(((node->rotation).c[3]))/3.1415926536*180,((node->rotation).c[0]),((node->rotation).c[1]),((node->rotation).c[2])
                );
                FW_GL_TRANSLATE_D(-(((node->__movedCoords).c[0])),-(((node->__movedCoords).c[1])),-(((node->__movedCoords).c[2]))
                );
                FW_GL_TRANSLATE_F(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
                );
            }
        }
} 

void changed_GeoTransform (struct X3D_GeoTransform *node) { 
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
	INITIALIZE_EXTENT;
}

void child_GeoTransform (struct X3D_GeoTransform *node) {
	CHILDREN_COUNT
	LOCAL_LIGHT_SAVE
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
	OCCLUSIONTEST
	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* any children at all? */
	if (nc==0) return;

	/* {
		int x;
		struct X3D_Node *xx;

		printf ("child_Transform, this %d \n",node);
		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->children.p[x]);
			printf ("	ch %d type %s dist %f\n",node->children.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	} */

	/* Check to see if we have to check for collisions for this transform. */
#ifdef COLLISIONTRANSFORM
	if (render_collision) {
		iv.x = node->EXTENT_MAX_X/2.0;
		jv.y = node->EXTENT_MAX_Y/2.0;
		kv.z = node->EXTENT_MAX_Z/2.0;
		ov.x = -(node->EXTENT_MAX_X); 
		ov.y = -(node->EXTENT_MAX_Y); 
		ov.z = -(node->EXTENT_MAX_Z);

	       /* get the transformed position of the Box, and the scale-corrected radius. */
	       FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);

	       transform3x3(&tupv,&tupv,modelMatrix);
	       matrotate2v(upvecmat,ViewerUpvector,tupv);
	       matmultiply(modelMatrix,upvecmat,modelMatrix);
	       /* matinverse(upvecmat,upvecmat); */

	       /* values for rapid test */
	       t_orig.x = modelMatrix[12];
	       t_orig.y = modelMatrix[13];
	       t_orig.z = modelMatrix[14];
		/* printf ("TB this %d, extent %4.3f %4.3f %4.3f pos %4.3f %4.3f %4.3f\n", 
			node,node->EXTENT_MAX_X,node->EXTENT_MAX_Y,EXTENT_MAX_Z,
			t_orig.x,t_orig.y,t_orig.z); */
	       scale = pow(det3x3(modelMatrix),1./3.);
	       if(!fast_ycylinder_box_intersect(abottom,atop,awidth,t_orig,
			scale*node->EXTENT_MAX_X*2,
			scale*node->EXTENT_MAX_Y*2,
			scale*node->EXTENT_MAX_Z*2)) {
			/* printf ("TB this %d returning fast\n",node); */
			return;
		/* } else {
			printf ("TB really look at %d\n",node); */
		}
	}
#endif

	/* do we have a local light for a child? */
	LOCAL_LIGHT_CHILDREN(node->children);

	/* now, just render the non-directionalLight children */

	/* printf ("Transform %d, flags %d, render_sensitive %d\n",
			node,node->_renderFlags,render_sensitive); */

	#ifdef CHILDVERBOSE
		printf ("transform - doing normalChildren\n");
	#endif

	normalChildren(node->children);

	#ifdef CHILDVERBOSE
		printf ("transform - done normalChildren\n");
	#endif

	LOCAL_LIGHT_OFF
}
