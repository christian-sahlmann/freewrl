# Copyright (C) 1998 Tuomas J. Lukka 1999 John Stewart CRC Canada
# Portions Copyright (C) 1998 Bernhard Reiter
# DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
# See the GNU Library General Public License (file COPYING in the distribution)
# for conditions of use and redistribution.
#
# $Id: VRMLRend.pm,v 1.12 2009/06/17 18:50:42 crc_canada Exp $
#
# Name:        VRMLRend.c
# Description:
#              Fills Hash Variables with "C" Code. They are used by VRMLC.pm
#              to write the C functions-source to render different nodes.
#
#              Certain Abbreviation are used, some are substituted in the
#              writing process in get_rendfunc() [VRMLC.pm].
#              Others are "C-#defines".
#              e.g. for #define glTexCoord2f(a,b) glTexCoord2f(a,b) see gen() [VRMLC.pm]
#
# $Log: VRMLRend.pm,v $
# Revision 1.12  2009/06/17 18:50:42  crc_canada
# More VRML1 code entered.
#
# Revision 1.11  2009/06/17 15:05:24  crc_canada
# VRML1 nodes added to build process.
#
# Revision 1.10  2009/06/05 20:29:32  crc_canada
# verifying fields of nodes against spec.
#
# Revision 1.9  2009/05/22 16:18:40  crc_canada
# more XML formatted code script/shader programming.
#
# Revision 1.8  2009/05/21 20:30:08  crc_canada
# XML parser - scripts and shaders now using common routing and maintenance routines.
#
# Revision 1.7  2009/05/12 19:53:14  crc_canada
# Confirm current support levels, and verify that Components and Profiles are checked properly.
#
# Revision 1.6  2009/05/11 21:11:58  crc_canada
# local/global lighting rules applied to SpotLight, DirectionalLight and PointLight.
#
# Revision 1.5  2009/04/02 18:48:28  crc_canada
# PROTO Routing for MFNodes.
#
# Revision 1.4  2009/03/10 21:00:34  crc_canada
# checking in some ongoing PROTO support work in the Classic parser.
#
# Revision 1.3  2009/03/09 21:32:30  crc_canada
# initial handling of new PROTO parameter methodology
#
# Revision 1.2  2009/03/06 18:50:31  istakenv
# fixed metadata variable names
#
# Revision 1.1  2009/03/05 21:33:39  istakenv
# Added code-generator perl scripts to new freewrl tree.  Initial commit, still need to patch them to make them work.
#
# Revision 1.232  2009/03/03 16:59:14  crc_canada
# Metadata fields now verified to be in every X3D node.
#
# Revision 1.231  2009/01/29 16:01:21  crc_canada
# more node definitions.
#
# Revision 1.230  2008/10/29 21:07:05  crc_canada
# Work on fillProperties and TwoSidedMaterial.
#
# Revision 1.229  2008/10/29 18:32:07  crc_canada
# Add code to confirm Profiles and Components.
#
# Revision 1.228  2008/10/23 19:18:53  crc_canada
# CubeMap texturing - start.
#
# Revision 1.227  2008/10/02 15:38:42  crc_canada
# Shader support started; Geospatial eventOut verification.
#
# Revision 1.226  2008/09/24 19:23:01  crc_canada
# GeoTouchSensor work.
#
# Revision 1.225  2008/09/23 16:45:02  crc_canada
# initial GeoTransform code added.
#
# Revision 1.224  2008/09/22 16:06:48  crc_canada
# all fieldtypes now defined in freewrl code; some not parsed yet, though, as there are no supported
# nodes that use them.
#
# Revision 1.223  2008/09/05 17:46:49  crc_canada
# reduce warnings counts when compiled with warnings=all
#
# Revision 1.222  2008/08/18 14:45:38  crc_canada
# Billboard node Scene Graph changes.
#
# Revision 1.221  2008/08/14 05:02:32  crc_canada
# Bindable threading issues, continued; EXAMINE mode default rotation distance, continued; LOD improvements.
#
# Revision 1.220  2008/08/04 19:14:36  crc_canada
# August 4 GeoLOD  changes
#
# Revision 1.219  2008/07/30 18:08:34  crc_canada
# GeoLOD, July 30 changes.
#
# Revision 1.218  2008/06/17 19:00:27  crc_canada
# Geospatial work - June 17 2008
#
# Revision 1.217  2008/06/13 13:50:49  crc_canada
# Geospatial, SF/MFVec3d support.
#
# Revision 1.216  2008/05/07 15:22:41  crc_canada
# input function modified to better handle files without clear end of line on last line.
#
# Revision 1.215  2008/03/31 20:10:17  crc_canada
# Review texture transparency, use node table to update scenegraph to allow for
# node updating.
#
# Revision 1.214  2007/12/13 20:12:52  crc_canada
# KeySensor and StringSensor
#
# Revision 1.213  2007/12/12 23:24:58  crc_canada
# X3DParser work
#
# Revision 1.212  2007/12/10 19:13:53  crc_canada
# Add parsing for x3dv COMPONENT, EXPORT, IMPORT, META, PROFILE
#
# Revision 1.211  2007/12/08 13:38:17  crc_canada
# first changes for x3dv handling of META, COMPONENT, etc. taga.
#
# Revision 1.210  2007/12/06 21:50:57  crc_canada
# Javascript X3D initializers
#
# Revision 1.209  2007/11/06 20:25:28  crc_canada
# Lighting revisited - pointlights and spotlights should all now work ok
#
# Revision 1.208  2007/08/23 14:01:22  crc_canada
# Initial AudioControl work
#
# Revision 1.207  2007/03/12 20:54:00  crc_canada
# MidiKey started.
#
# Revision 1.206  2007/02/28 20:34:50  crc_canada
# More MIDI work - channelPresent works!
#
# Revision 1.205  2007/01/17 21:29:28  crc_canada
# more X3D XML parsing work.
#
# Revision 1.204  2007/01/12 17:55:27  crc_canada
# more 1.18.11 changes
#
# Revision 1.203  2007/01/11 21:09:21  crc_canada
# new files for X3D parsing
#
# Revision 1.201  2006/11/14 20:16:39  crc_canada
# ReWire work.
#
# Revision 1.200  2006/07/10 14:24:11  crc_canada
# add keywords for PROTO interface fields.
#
# Revision 1.199  2006/05/31 14:52:28  crc_canada
# more changes to code for SAI.
#
# Revision 1.198  2006/05/23 16:15:10  crc_canada
# remove print statements, add more defines for a VRML C parser.
#
# Revision 1.197  2006/05/15 14:05:59  crc_canada
# Various fixes; CVS was down for a week. Multithreading for shape compile
# is the main one.
#
# Revision 1.196  2006/03/01 15:16:57  crc_canada
# Changed include file methodology and some Frustum work.
#
# Revision 1.195  2006/02/28 16:19:42  crc_canada
# BoundingBox
#
# Revision 1.194  2006/01/06 22:05:15  crc_canada
# VisibilitySensor
#
# Revision 1.193  2006/01/03 23:01:22  crc_canada
# EXTERNPROTO and Group sorting bugs fixed.
#
# Revision 1.192  2005/12/21 18:16:40  crc_canada
# Rework Generation code.
#
# Revision 1.191  2005/12/19 21:25:08  crc_canada
# HAnim start
#
# Revision 1.190  2005/12/16 18:07:11  crc_canada
# rearrange perl generation
#
# Revision 1.189  2005/12/16 13:49:23  crc_canada
# updating generation functions.
#
# Revision 1.188  2005/12/15 20:42:01  crc_canada
# CoordinateInterpolator2D PositionInterpolator2D
#
# Revision 1.187  2005/12/15 19:57:58  crc_canada
# Geometry2D nodes complete.
#
# Revision 1.186  2005/12/14 13:51:32  crc_canada
# More Geometry2D nodes.
#
# Revision 1.185  2005/12/13 17:00:29  crc_canada
# Arc2D work.
#.....

# used for the X3D Parser only. Return type of node.
%defaultContainerType = (
	ContourPolyLine2D 	=>geometry,
	NurbsTrimmedSurface	=>geometry,
	MidiControl		=>children,
	MidiKey			=>children,


	Arc2D			=>geometry,
	ArcClose2D		=>geometry,
	Circle2D		=>geometry,
	Disk2D			=>geometry,
	Polyline2D		=>geometry,
	Polypoint2D		=>geometry,
	Rectangle2D		=>geometry,
	TriangleSet2D		=>geometry,
	
	Anchor 			=>children,
	Appearance 		=>appearance,
	AudioClip 		=>source,
	AudioControl		=>children,
	Background 		=>children,
	Billboard 		=>children,
	Box 			=>geometry,
	ClipPlane 		=>children,
	Collision 		=>children,
	Color 			=>color,
	ColorInterpolator 	=>children,
	ColorRGBA 		=>color,
	Cone 			=>geometry,
	Contour2D 		=>geometry,
	Coordinate 		=>coord,
	FogCoordinate 		=>coord,
	CoordinateDeformer 	=>children,
	CoordinateInterpolator 	=>children,
	CoordinateInterpolator2D 	=>children,
	Cylinder 		=>geometry,
	CylinderSensor 		=>children,
	DirectionalLight 	=>children,
	ElevationGrid 		=>geometry,
	Extrusion 		=>geometry,
	FillProperties		=>fillProperties,
	Fog 			=>children,
	LocalFog 		=>children,
	FontStyle 		=>fontStyle,
	GeoCoordinate 		=>coord,
	GeoElevationGrid 	=>geometry,
	GeoLocation 		=>children,
	GeoLOD 			=>children,
	GeoMetadata		=>children,
	GeoOrigin 		=>geoOrigin,
	GeoPositionInterpolator	=>children,
	GeoProximitySensor 	=>children,
	GeoTouchSensor		=>children,
	GeoTransform		=>children,
	GeoViewpoint 		=>children,
	Group 			=>children,
	HAnimDisplacer		=>children,
	HAnimHumanoid		=>children,
	HAnimJoint		=>joints,
	HAnimSegment		=>segments,
	HAnimSite		=>sites,
	ImageTexture 		=>texture,
	ImageCubeMapTexture 	=>texture,
	GeneratedCubeMapTexture	=>texture, 
	ComposedCubeMapTexture	=>texture,
	IndexedFaceSet 		=>geometry,
	IndexedLineSet 		=>geometry,
	IndexedTriangleFanSet 	=>geometry,
	IndexedTriangleSet 	=>geometry,
	IndexedTriangleStripSet	=>geometry,
	Inline 			=>children,
	KeySensor		=>children,
	LineSet 		=>geometry,
	LineProperties		=>lineProperties,
	LoadSensor		=>children,
	LOD 			=>children,
	Material 		=>material,
	TwoSidedMaterial	=>material,
	MultiTexture		=>texture,
	MultiTextureCoordinate  =>texCoord,
	MultiTextureTransform	=>textureTransform,
	MovieTexture 		=>texture,
	NavigationInfo 		=>children,
	Normal 			=>normal,
	NormalInterpolator 	=>children,
	NurbsCurve2D 		=>geometry,
	NurbsCurve 		=>geometry,
	NurbsGroup 		=>children,
	NurbsPositionInterpolator=>children,
	NurbsSurface 		=>children,
	NurbsTextureSurface 	=>children,
	OrientationInterpolator	=>children,
	PixelTexture 		=>texture,
	PlaneSensor 		=>children,
	PointLight 		=>children,
	PointSet 		=>geometry,
	PositionInterpolator 	=>children,
	PositionInterpolator2D 	=>children,
	ProximitySensor 	=>children,
	ScalarInterpolator 	=>children,
	Scene 			=>children,
	Script 			=>children,
	Shape 			=>children,
	Sound 			=>children,
	Sphere 			=>geometry,
	SphereSensor 		=>children,
	SpotLight 		=>children,
	StaticGroup		=>children,
	StringSensor		=>children,
	Switch 			=>children,
	Text 			=>geometry,
	TextureBackground 	=>children,
	TextureCoordinate 	=>texCoord,
	TextureCoordinateGenerator  =>texCoord,
	TextureTransform 	=>textureTransform,
	TextureProperties	=>children,
	TimeSensor 		=>children,
	TouchSensor 		=>children,
	Transform 		=>children,
	TriangleFanSet 		=>geometry,
	TriangleSet 		=>geometry,
	TriangleStripSet 	=>geometry,
	TrimmedSurface 		=>children,
	Viewpoint 		=>children,
	VisibilitySensor 	=>children,
	WorldInfo 		=>children,

	BooleanFilter		=>children,
	BooleanSequencer	=>children,
	BooleanToggle		=>children,
	BooleanTrigger		=>children,
	IntegerSequencer	=>children,
	IntegerTrigger		=>children,
	TimeTrigger		=>children,

	ComposedShader		=>shaders,
	FloatVertexAttribute	=>children,
	Matrix3VertexAttribute	=>children,
	Matrix4VertexAttribute	=>children,
	PackagedShader		=>material,
	ProgramShader		=>programs,
	ShaderPart		=>parts,
	ShaderProgram		=>material,

	MetadataSet		=>metadata,
	MetadataInteger		=>metadata,
	MetadataDouble		=>metadata,
	MetadataFloat		=>metadata,
	MetadataString		=>metadata,

	
	MetadataSFFloat		=>metadata,
	MetadataMFFloat		=>metadata,
	MetadataSFRotation	=>metadata,
	MetadataMFRotation	=>metadata,
	MetadataSFVec3f		=>metadata,
	MetadataMFVec3f		=>metadata,
	MetadataSFBool		=>metadata,
	MetadataMFBool		=>metadata,
	MetadataSFInt32		=>metadata,
	MetadataMFInt32		=>metadata,
	MetadataSFNode		=>metadata,
	MetadataMFNode		=>metadata,
	MetadataSFColor		=>metadata,
	MetadataMFColor		=>metadata,
	MetadataSFColorRGBA	=>metadata,
	MetadataMFColorRGBA	=>metadata,
	MetadataSFTime		=>metadata,
	MetadataMFTime		=>metadata,
	MetadataSFString	=>metadata,
	MetadataMFString	=>metadata,
	MetadataSFVec2f		=>metadata,
	MetadataMFVec2f		=>metadata,
	MetadataSFImage		=>metadata,
	MetadataFreeWRLPTR	=>metadata,
	MetadataSFVec3d		=>metadata,
	MetadataMFVec3d		=>metadata,
	MetadataSFDouble	=>metadata,
	MetadataMFDouble	=>metadata,
	MetadataSFMatrix3f	=>metadata,
	MetadataMFMatrix3f	=>metadata,
	MetadataSFMatrix3d	=>metadata,
	MetadataMFMatrix3d	=>metadata,
	MetadataSFMatrix4f	=>metadata,
	MetadataMFMatrix4f	=>metadata,
	MetadataSFMatrix4d	=>metadata,
	MetadataMFMatrix4d	=>metadata,
	MetadataSFVec2d		=>metadata,
	MetadataMFVec2d		=>metadata,
	MetadataSFVec4f		=>metadata,
	MetadataMFVec4f		=>metadata,
	MetadataSFVec4d		=>metadata,
	MetadataMFVec4d		=>metadata,


	VRML1_AsciiText		=>children,
	VRML1_Cone		=>children,
	VRML1_Coordinate3	=>children,
	VRML1_Cube		=>children,
	VRML1_Cylinder		=>children,
	VRML1_DirectionalLight	=>children,
	VRML1_FontStyle		=>children,
	VRML1_IndexedFaceSet	=>children,
	VRML1_IndexedLineSet	=>children,
	VRML1_Info		=>children,
	VRML1_LOD		=>children,
	VRML1_Material		=>children,
	VRML1_MaterialBinding	=>children,
	VRML1_MatrixTransform	=>children,
	VRML1_Normal		=>children,
	VRML1_NormalBinding	=>children,
	VRML1_OrthographicCamera=>children,
	VRML1_PerspectiveCamera	=>children,
	VRML1_PointLight	=>children,
	VRML1_PointSet		=>children,
	VRML1_Rotation		=>children,
	VRML1_Scale		=>children,
	VRML1_Separator		=>children,
	VRML1_ShapeHints	=>children,
	VRML1_Sphere		=>children,
	VRML1_SpotLight		=>children,
	VRML1_Switch		=>children,
	VRML1_Texture2		=>children,
	VRML1_Texture2Transform	=>children,
	VRML1_TextureCoordinate2=>children,
	VRML1_Transform		=>children,
	VRML1_Translation	=>children,
	VRML1_WWWAnchor		=>children,
	VRML1_WWWInline		=>children,

);

#######################################################################
#######################################################################
#######################################################################
#
# Rend --
#  actually render the node
#
#

# Rend = real rendering - rend_geom is true; this is for things that
#	actually affect triangles/lines on the screen.
#
# All of these will have a render_xxx name associated with them.

%RendC = map {($_=>1)} qw/
	NavigationInfo
	Fog
	Background
	TextureBackground
	Box 
	Cylinder 
	Cone 
	Sphere 
	IndexedFaceSet 
	Extrusion 
	ElevationGrid 
	Arc2D 
	ArcClose2D 
	Circle2D 
	Disk2D 
	Polyline2D 
	Polypoint2D 
	Rectangle2D 
	TriangleSet2D 
	IndexedTriangleFanSet 
	IndexedTriangleSet 
	IndexedTriangleStripSet 
	TriangleFanSet 
	TriangleStripSet 
	TriangleSet 
	LineSet 
	IndexedLineSet 
	PointSet 
	GeoElevationGrid 
	LoadSensor 
	TextureCoordinateGenerator 
	TextureCoordinate 
	Text 
	LineProperties 
	FillProperties 
	Material 
	TwoSidedMaterial 
	ProgramShader
	PackagedShader
	ComposedShader
	PixelTexture 
	ImageTexture 
	MultiTexture 
	MovieTexture 
	ComposedCubeMapTexture
	GeneratedCubeMapTexture
	ImageCubeMapTexture
	Sound 
	AudioControl
	AudioClip 
	DirectionalLight 
	SpotLight
	PointLight
	HAnimHumanoid
	HAnimJoint

	VRML1_AsciiText
	VRML1_Cone
	VRML1_Cube
	VRML1_Cylinder
	VRML1_IndexedFaceSet
	VRML1_IndexedLineSet
	VRML1_PointSet
	VRML1_Sphere
	VRML1_Scale
	VRML1_Translation
	VRML1_Transform
	VRML1_Material
	VRML1_Rotation
/;

#######################################################################
#######################################################################
#######################################################################

#
# GenPolyRep
#  code for generating internal polygonal representations
#  of some nodes (ElevationGrid, Text, Extrusion and IndexedFaceSet)
#
# 

%GenPolyRepC = map {($_=>1)} qw/
	ElevationGrid
	Extrusion
	IndexedFaceSet
	IndexedTriangleFanSet
	IndexedTriangleSet
	IndexedTriangleStripSet
	TriangleFanSet
	TriangleStripSet
	TriangleSet
	Text
	GeoElevationGrid
/;

#######################################################################
#######################################################################
#######################################################################
#
# Prep --
#  Prepare for rendering a node - e.g. for transforms, do the transform
#  but not the children.

%PrepC = map {($_=>1)} qw/
	HAnimJoint
	HAnimSite
	Viewpoint
	Transform
	Billboard
	Group
	MidiControl
	MidiKey
	PointLight
	SpotLight
	DirectionalLight
	GeoLocation
	GeoViewpoint
	GeoTransform
	VRML1_Separator
/;

#######################################################################
#######################################################################
#######################################################################
#
# Fin --
#  Finish the rendering i.e. restore matrices and whatever to the
#  original state.
#
#

%FinC = map {($_=>1)} qw/
	GeoLocation
	Transform
	Billboard
	HAnimSite
	HAnimJoint
	GeoTransform
	VRML1_Separator
/;


#######################################################################
#######################################################################
#######################################################################
#
# Child --
#  Render the actual children of the node.
#
#

# Render children (real child nodes, not e.g. appearance/geometry)
%ChildC = map {($_=>1)} qw/
	HAnimHumanoid
	HAnimJoint
	HAnimSegment
	HAnimSite
	Group
	StaticGroup
	Billboard
	Transform
	Anchor
	GeoLocation
	GeoTransform
	Inline
	Switch
	GeoLOD
	LOD
	Collision
	Appearance
	Shape
	VisibilitySensor
	VRML1_Separator
/;


#######################################################################
#######################################################################
#######################################################################
#
# Compile --
#
%CompileC = map {($_=>1)} qw/
	IndexedLineSet
	LineSet
	Arc2D
	ArcClose2D
	Circle2D
	Disk2D
	TriangleSet2D
	Rectangle2D
	Box
	Cone
	Cylinder
	Sphere
	GeoLocation
	GeoCoordinate
	GeoElevationGrid
	GeoLocation
	GeoLOD
	GeoMetadata
	GeoOrigin
	GeoPositionInterpolator
	GeoTouchSensor
	GeoViewpoint	
	GeoProximitySensor
	GeoTransform
	ComposedShader
	ProgramShader
	PackagedShader
	MetadataMFFloat
	MetadataMFRotation
	MetadataMFVec3f
	MetadataMFBool
	MetadataMFInt32
	MetadataMFNode
	MetadataMFColor
	MetadataMFColorRGBA
	MetadataMFTime
	MetadataMFString
	MetadataMFVec2f
	MetadataMFVec3d
	MetadataMFDouble
	MetadataMFMatrix3f
	MetadataMFMatrix3d
	MetadataMFMatrix4f
	MetadataMFMatrix4d
	MetadataMFVec2d
	MetadataMFVec4f
	MetadataMFVec4d
	MetadataSFFloat
	MetadataSFRotation
	MetadataSFVec3f
	MetadataSFBool
	MetadataSFInt32
	MetadataSFNode
	MetadataSFColor
	MetadataSFColorRGBA
	MetadataSFTime
	MetadataSFString
	MetadataSFVec2f
	MetadataSFImage
	MetadataSFVec3d
	MetadataSFDouble
	MetadataSFMatrix3f
	MetadataSFMatrix3d
	MetadataSFMatrix4f
	MetadataSFMatrix4d
	MetadataSFVec2d
	MetadataSFVec4f
	MetadataSFVec4d
/;


#######################################################################
#
# ChangedC - when the fields change, the following code is run before
# rendering for caching the data.
#
#

%ChangedC = map {($_=>1)} qw/
	GeoLOD
	LOD
	Group
        Inline
	Transform
	StaticGroup
	Billboard
	Anchor
	Collision
	GeoTransform
	GeoLocation
	HAnimSite
	Switch
/;

#######################################################################
#
# ProximityC = following code is run to let proximity sensors send their
# events. This is done in the rendering pass, because the position of
# of the object relative to the viewer is available via the
# modelview transformation matrix.
#

%ProximityC = map {($_=>1)} qw/
	ProximitySensor
	GeoProximitySensor
	GeoLOD
	LOD
	Billboard
/;


#######################################################################
#
# CollisionC = following code is run to do collision detection
#
# In collision nodes:
#    if enabled:
#       if no proxy:
#           passes rendering to its children
#       else (proxy)
#           passes rendering to its proxy
#    else
#       does nothing.
#
# In normal nodes:
#    uses gl modelview matrix to determine distance from viewer and
# angle from viewer. ...
#
#
#	       /* the shape of the avatar is a cylinder */
#	       /*                                           */
#	       /*           |                               */
#	       /*           |                               */
#	       /*           |--|                            */
#	       /*           | width                         */
#	       /*        ---|---       -                    */
#	       /*        |     |       |                    */
#	       /*    ----|() ()| - --- | ---- y=0           */
#	       /*        |  \  | |     |                    */
#	       /*     -  | \ / | |head | height             */
#	       /*    step|     | |     |                    */
#	       /*     -  |--|--| -     -                    */
#	       /*           |                               */
#	       /*           |                               */
#	       /*           x,z=0                           */


%CollisionC = map {($_=>1)} qw/
	Disk2D
	Rectangle2D
	TriangleSet2D
	Sphere
	Box
	Cone
	Cylinder
	ElevationGrid
	IndexedFaceSet
	IndexedTriangleFanSet
	IndexedTriangleSet
	IndexedTriangleStripSet
	TriangleFanSet
	TriangleStripSet
	TriangleSet
	Extrusion
	Text
	GeoElevationGrid
/;

#######################################################################
#######################################################################
#######################################################################
#
# RendRay --
#  code for checking whether a ray (defined by mouse pointer)
#  intersects with the geometry of the primitive.
#
#

# Y axis rotation around an unit vector:
# alpha = angle between Y and vec, theta = rotation angle
#  1. in X plane ->
#   Y = Y - sin(alpha) * (1-cos(theta))
#   X = sin(alpha) * sin(theta)
#
#
# How to find out the orientation from two vectors (we are allowed
# to assume no negative scales)
#  1. Y -> Y' -> around any vector on the plane midway between the
#                two vectors
#     Z -> Z' -> around any vector ""
#
# -> intersection.
#
# The plane is the midway normal between the two vectors
# (if the two vectors are the same, it is the vector).


# found in the C code:
# Distance to zero as function of ratio is
# sqrt(
#	((1-r)t_r1.x + r t_r2.x)**2 +
#	((1-r)t_r1.y + r t_r2.y)**2 +
#	((1-r)t_r1.z + r t_r2.z)**2
# ) == radius
# Therefore,
# radius ** 2 == ... ** 2
# and
# radius ** 2 =
# 	(1-r)**2 * (t_r1.x**2 + t_r1.y**2 + t_r1.z**2) +
#       2*(r*(1-r)) * (t_r1.x*t_r2.x + t_r1.y*t_r2.y + t_r1.z*t_r2.z) +
#       r**2 (t_r2.x**2 ...)
# Let's name tr1sq, tr2sq, tr1tr2 and then we have
# radius ** 2 =  (1-r)**2 * tr1sq + 2 * r * (1-r) tr1tr2 + r**2 tr2sq
# = (tr1sq - 2*tr1tr2 + tr2sq) r**2 + 2 * r * (tr1tr2 - tr1sq) + tr1sq
#
# I.e.
#
# (tr1sq - 2*tr1tr2 + tr2sq) r**2 + 2 * r * (tr1tr2 - tr1sq) +
#	(tr1sq - radius**2) == 0
#
# I.e. second degree eq. a r**2 + b r + c == 0 where
#  a = tr1sq - 2*tr1tr2 + tr2sq
#  b = 2*(tr1tr2 - tr1sq)
#  c = (tr1sq-radius**2)
#
#
# Cylinder: first test the caps, then against infinite cylinder.

# For cone, this is most difficult. We have
# sqrt(
#	((1-r)t_r1.x + r t_r2.x)**2 +
#	((1-r)t_r1.z + r t_r2.z)**2
# ) == radius*( -( (1-r)t_r1.y + r t_r2.y )/(2*h)+0.5)
# == radius * ( -( r*(t_r2.y - t_r1.y) + t_r1.y )/(2*h)+0.5)
# == radius * ( -r*(t_r2.y-t_r1.y)/(2*h) + 0.5 - t_r1.y/(2*h))

#
# Other side: r*r*(

%RendRayC = map {($_=>1)} qw/
	Box
	Sphere
	Cylinder
	Cone
	GeoElevationGrid
	ElevationGrid
	Text
	Extrusion
	IndexedFaceSet
	IndexedTriangleSet
	IndexedTriangleFanSet
	IndexedTriangleStripSet
	TriangleSet
	TriangleFanSet
	TriangleStripSet
/;

#######################################################################
#######################################################################
#######################################################################
#
# VRML1_ Keywords
%VRML1_C = map {($_=>1)} qw/
	VRML1_AsciiText
	VRML1_Cone
	VRML1_Cube
	VRML1_Cylinder
	VRML1_IndexedFaceSet
	VRML1_IndexedLineSet
	VRML1_PointSet
	VRML1_Sphere
	VRML1_Coordinate3
	VRML1_FontStyle
	VRML1_Info
	VRML1_Material
	VRML1_MaterialBinding
	VRML1_Normal
	VRML1_NormalBinding
	VRML1_Texture2
	VRML1_Texture2Transform
	VRML1_TextureCoordinate2
	VRML1_ShapeHints
	VRML1_MatrixTransform
	VRML1_Rotation
	VRML1_Scale
	VRML1_Transform
	VRML1_Translation
	VRML1_Separator
	VRML1_Switch
	VRML1_WWWAnchor
	VRML1_LOD
	VRML1_OrthographicCamera
	VRML1_PerspectiveCamera
	VRML1_DirectionalLight
	VRML1_PointLight
	VRML1_SpotLight
	VRML1_WWWInline
/;

#######################################################################
#######################################################################
#######################################################################
#
# Keywords
# a listing of keywords for use in the C VRML parser.
#
# 

%KeywordC = map {($_=>1)} qw/
	COMPONENT
	DEF
	EXPORT
	EXTERNPROTO
	FALSE
	IMPORT
	IS
	META
	NULL
	PROFILE
	PROTO
	ROUTE
	TO
	TRUE
	USE
	inputOnly
	outputOnly
	inputOutput
	initializeOnly
	exposedField
	field
	eventIn
	eventOut
/;


#######################################################################
#
# Components 
# a listing of Components for use in the C VRML parser.
#
# 

%ComponentC = map {($_=>1)} qw/
	CADGeometry
	Core
	CubeMapTexturing
	DIS
	EnvironmentalEffects
	EnvironmentalSensor
	EventUtilities
	Followers
	Geometry2D
	Geometry3D
	Geospatial
	Grouping
	H-Anim
	Interpolation
	KeyDeviceSensor
	Layering
	Layout
	Lighting
	Navigation
	Networking
	NURBS
	ParticleSystems
	PickingSensor
	PointDeviceSensor
	Shaders
	Rendering
	RigidBodyPhysics
	Scripting
	Shape
	Sound
	Text
	Texturing
	Texturing3D
	Time
/;


#######################################################################
#
# Profiles 
# a listing of Profiles for use in the C VRML parser.
#
# 

%ProfileC = map {($_=>1)} qw/
	CADInterchange
	Core
	Full
	Immersive
	Interactive
	Interchange
	MPEG-4
/;

#######################################################################
#######################################################################
#######################################################################

#
# GEOSpatialKeywords
# a listing of Geospatial Elipsoid keywords.
#
# 

%GEOSpatialKeywordC = map {($_=>1)} qw/
	AA
	AM
	AN
	BN
	BR
	CC
	CD
	EA
	EB
	EC
	ED
	EE
	EF
	FA
	GC
	GCC
	GCC
	GD
	GDC
	GDC
	HE
	HO
	ID
	IN
	KA
	RF
	SA
	UTM
	WD
	WE
	WGS84
	coordinateSystem
	copyright
	dataFormat
	dataUrl
	date
	description
	ellipsoid
	extent
	horizontalDatum
	metadataFormat
	originator
	resolution
	title
	verticalDatum
/;

#######################################################################
#######################################################################
#######################################################################

#
# PROTOKeywords
# a listing of PROTO define keywords for use in the C VRML parser.
#
# 

%PROTOKeywordC = map {($_=>1)} qw/
	exposedField
	field
	eventIn
	eventOut
	inputOnly
	outputOnly
	inputOutput
	initializeOnly
/;

#######################################################################
#######################################################################
#######################################################################

#
# X3DSPECIAL Keywords
# a listing of control keywords for use in the XML parser.
#
# 

%X3DSpecialC = map {($_=>1)} qw/
	Scene
	Header
	head
	meta
	ProtoDeclare
	ProtoInterface
	ProtoInstance
	ProtoBody
	ROUTE
	IS
	connect
	X3D
	field
	fieldValue
	component
	import
	export
/;
1;
