/*******************************************************************
 Copyright (C) 2007, 2008 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

#include "headers.h"

/* table showing which levels are supported by which component */
static const int capabilities[] = {
	COM_Geometry2D,	10, 		/* unverified */
	COM_Rendering,	10, 		/* unverified */
	COM_PickingSensor,	10, 	/* unverified */
	COM_DIS,	10, 		/* unverified */
	COM_EnvironmentalSensor,	10, /* unverified */
	COM_Text,	10, 		/* unverified */
	COM_NURBS,	10, 		/* unverified */
	COM_CubeMapTexturing,	10, 	/* unverified */
	COM_EventUtilities,	10, 	/* unverified */
	COM_Interpolation,	10, 	/* unverified */
	COM_Shaders,	10, 		/* unverified */
	COM_Navigation,	10, 		/* unverified */
	COM_Grouping,		3,	/* October 29, 2008 */
	COM_Texturing,	10, 		/* unverified */
	COM_Geospatial,	10, 		/* unverified */
	COM_CADGeometry,	10, 	/* unverified */
	COM_EnvironmentalEffects,	10, /* unverified */
	COM_Shape,	10, 		/* unverified */
	COM_Texturing3D,	10, 	/* unverified */
	COM_PointDeviceSensor,	10, 	/* unverified */
	COM_HAnim,	10, 		/* unverified */
	COM_RigidBodyPhysics,	10, 	/* unverified */
	COM_Core,		2,	/* October 29, 2008 */
	COM_Layout,	10, 		/* unverified */
	COM_Time,		2, 	/* October 29, 2008 */
	COM_Geometry3D,	10, 		/* unverified */
	COM_Followers,	10, 		/* unverified */
	COM_Scripting,	10, 		/* unverified */
	COM_Lighting,	10, 		/* unverified */
	COM_KeyDeviceSensor,	10, 	/* unverified */
	COM_Layering,	10, 		/* unverified */
	COM_Networking,	10, 		/* unverified */
	COM_ParticleSystems,	10, 	/* unverified */
	COM_Sound,	10, 		/* unverified */
	ID_UNDEFINED, 	ID_UNDEFINED,
};

/* profiles... */

/* ISO-IEC-FDISID_UNDEFINED9775:1.2 H3 Component support */
static const int CADInterchangeProfile[] = {
	COM_Core,		1,
	COM_Networking,		1,
	COM_Grouping,		1,
	COM_Rendering,		4,
	COM_Shape,		2,
	COM_Lighting,		1,
	COM_Texturing,		2,
	COM_Navigation,		2,
	COM_Shaders,		1,
	COM_CADGeometry,	2,
	ID_UNDEFINED, 		ID_UNDEFINED};


/* ISO-IEC-FDISID_UNDEFINED9775:1.2 A3 Component support */
static const int CoreProfile[] = {
	COM_Core,		1,
	ID_UNDEFINED, 		ID_UNDEFINED};


/* ISO-IEC-FDISID_UNDEFINED9775:1.2 F3 Component support */
static const int FullProfile[] = {
	COM_Core,			2,
	COM_Time,			2,
	COM_Networking,			3,
	COM_Grouping,			3,
	COM_Rendering,			5,
	COM_Shape,			4,
	COM_Geometry3D,			4,
	COM_Geometry2D,			2,
	COM_Text,			1,
	COM_Sound,			1,
	COM_Lighting,			3,
	COM_Texturing,			3,
	COM_Interpolation,		5,
	COM_Navigation,			3,
	COM_PointDeviceSensor,		1,
	COM_KeyDeviceSensor,		2,
	COM_EnvironmentalSensor,	3,
	COM_EnvironmentalEffects,	4,
	COM_Geospatial,			2,
	COM_HAnim,			1,
	COM_NURBS,			4,
	COM_DIS,			2,
	COM_Scripting,			1,
	COM_EventUtilities,		1,
	COM_Shaders,			1,
	COM_CADGeometry,		2,
	COM_Texturing3D,		2,
	COM_CubeMapTexturing,		3,
	COM_Layering,			1,
	COM_Layout,			2,
	COM_RigidBodyPhysics,		2,
	COM_PickingSensor,		3,
	COM_Followers,			1,
	COM_ParticleSystems,		3,
	ID_UNDEFINED, 			ID_UNDEFINED};


/* ISO-IEC-FDISID_UNDEFINED9775:1.2 E3 Component support */
static const int ImmersiveProfile[] = {
	COM_Core,			2,
	COM_Time,			1,
	COM_Networking,			3,
	COM_Grouping,			2,
	COM_Rendering,			3,
	COM_Shape,			2,
	COM_Geometry3D,			4,
	COM_Geometry2D,			1,
	COM_Text,			1,
	COM_Sound,			1,
	COM_Lighting,			2,
	COM_Texturing,			3,
	COM_Interpolation,		2,
	COM_PointDeviceSensor,		1,
	COM_KeyDeviceSensor,		2,
	COM_EnvironmentalSensor,	2,
	COM_EnvironmentalEffects,	2,
	COM_Scripting,			1,
	COM_EventUtilities,		1,
	ID_UNDEFINED, 			ID_UNDEFINED};


/* ISO-IEC-FDISID_UNDEFINED9775:1.2 C3 Component support */
static const int InteractiveProfile[] = {
	COM_Core,			1,
	COM_Time,			1,
	COM_Networking,			2,
	COM_Grouping,			2,
	COM_Rendering,			3,
	COM_Shape,			1,
	COM_Geometry3D,			3,
	COM_Lighting,			2,
	COM_Texturing,			2,
	COM_Interpolation,		2,
	COM_Navigation,			1,
	COM_PointDeviceSensor,		1,
	COM_KeyDeviceSensor,		1,
	COM_EnvironmentalSensor,	1,
	COM_EnvironmentalEffects,	1,
	COM_EventUtilities,		1,
	ID_UNDEFINED, 			ID_UNDEFINED};


/* ISO-IEC-FDISID_UNDEFINED9775:1.2 B3 Component support */
static const int InterchangeProfile[] = {
	COM_Core,			1,
	COM_Time,			1,
	COM_Networking,			1,
	COM_Grouping,			1,
	COM_Rendering,			3,
	COM_Shape,			1,
	COM_Geometry3D,			2,
	COM_Lighting,			1,
	COM_Texturing,			2,
	COM_Interpolation,		2,
	COM_Navigation,			1,
	COM_EnvironmentalEffects,	1,
	ID_UNDEFINED, 			ID_UNDEFINED};


/* ISO-IEC-FDISID_UNDEFINED9775:1.2 D3 Component support */
static const int MPEG4Profile[] = {
	COM_Core,			1,
	COM_Time,			1,
	COM_Networking,			2,
	COM_Grouping,			2,
	COM_Rendering,			1,
	COM_Shape,			1,
	COM_Geometry3D,			2,
	COM_Lighting,			2,
	COM_Texturing,			1,
	COM_Interpolation,		2,
	COM_Navigation,			1,
	COM_PointDeviceSensor,		1,
	COM_EnvironmentalSensor,	1,
	COM_Navigation,			1,
	COM_EnvironmentalEffects,	1,
	ID_UNDEFINED, 			ID_UNDEFINED};

struct proftablestruct {
	int profileName;
	const int *profileTable;
};

static struct proftablestruct profTable[] = {
	{PRO_Interchange,		InterchangeProfile},
	{PRO_CADInterchange, 		CADInterchangeProfile},
	{PRO_MPEG4,			MPEG4Profile},
	{PRO_Interactive, 		InteractiveProfile},
	{PRO_Full,			FullProfile},
	{PRO_Immersive,			ImmersiveProfile},
	{PRO_Core,			CoreProfile},
	{ID_UNDEFINED, 			(const int*) ID_UNDEFINED}
};


void handleMetaDataStringString(struct Uni_String *val1, struct Uni_String *val2) {
	#ifdef CAPABILITIESVERBOSE
	printf ("handleMetaDataStringString, :%s:, :%s:\n",val1->strptr, val2->strptr);
	#endif
}

void handleVersion (float myVersion) {
	#ifdef CAPABILITIESVERBOSE
	printf ("handleVersion, my level is %f\n",myVersion);
	#endif

	if ((myVersion < 2.999) || (myVersion > 3.201)) {
		ConsoleMessage ("expected X3D Version of between 3.0 and 3.2, got %f",myVersion);
	}
}

void handleProfile (int myProfile) {
	int *myTable = NULL;
	int i;
	/* myProfile is a valid profile number - bounds checked before entry */
	#ifdef CAPABILITIESVERBOSE
	printf ("handleProfile, my profile is %s (%d)\n",stringProfileType(myProfile), myProfile);
	#endif

	i=0;
	while ((profTable[i].profileName != ID_UNDEFINED) && (profTable[i].profileName != myProfile)) i++;

	/* we really should have found this, unless we have a new profile that is not coded properly here */
	if (profTable[i].profileName == ID_UNDEFINED) {
		ConsoleMessage ("Something wrong in handleProfile for profile %s\n",
			stringProfileType(myProfile));
	} else {
		int comp, lev;

		myTable = (int *)profTable[i].profileTable;
		/* go through the selected table, and see if each component is within range */
		comp = *myTable; myTable++; lev = *myTable; myTable++;
		while (comp != ID_UNDEFINED) {
			handleComponent(comp,lev);
			comp = *myTable; myTable++; lev = *myTable; myTable++;
		}
	}
}

void handleComponent (int myComponent, int myLevel) {
	int i;

	/* myComponent is a valid component number - bounds checked before entry */
	#ifdef CAPABILITIESVERBOSE
	printf ("handleComponent: my Component is %s, level %d\n",COMPONENTS[myComponent], myLevel);
	#endif

	i=0;
	while ((capabilities[i] != myComponent) && (capabilities[i] != ID_UNDEFINED)) {
		i+=2; 
	}

	/* did we find the component? */
	if (capabilities[i] == myComponent) {
		if (myLevel > capabilities[i+1]) {
			ConsoleMessage ("Component %s support level %d, requested %d",
				COMPONENTS[myComponent], capabilities[i+1], myLevel);
		}
	} else {
		ConsoleMessage ("did not find component %s in capabilities table!",COMPONENTS[myComponent]);
	}
}

void handleExport (char *node, char *as) {
	/* handle export statements. as will be either a string pointer, or NULL */
	
	#ifdef CAPABILITIESVERBOSE
	printf ("handleExport: node :%s: ",node);
	if (as != NULL) printf (" AS :%s: ",node);
	printf ("\n");
	#endif
}

void handleImport (char *nodeName,char *nodeImport, char *as) {
	/* handle Import statements. as will be either a string pointer, or NULL */
	
	#ifdef CAPABILITIESVERBOSE
	printf ("handleImport: inlineNodeName :%s: nodeToImport :%s:",nodeName, nodeImport);
	if (as != NULL) printf (" AS :%s: ",as);
	printf ("\n");
	#endif
}


