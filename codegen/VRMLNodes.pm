#
# $Id: VRMLNodes.pm,v 1.9 2009/04/21 19:19:24 crc_canada Exp $
#
# Copyright (C) 1998 Tuomas J. Lukka 1999 John Stewart CRC Canada.
# DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
# See the GNU Library General Public License (file COPYING in the distribution)
# for conditions of use and redistribution.

package VRML::NodeType;


# see note top of file - the VRML Parser REQUIRES for routing that 
# each field name exists in only one table- eg, "inputOutput", "initializeOnly",
# etc. So, in order to do this, we make sure that each field name follows this, even
# though we may, for instance, change "value" to an "inputOutput" when the spec
# says it is an "initializeOnly". This has little if any effect on parsing.

########################################################################

{
   sub new {
		my($type, $name, $fields, $X3DNodeType) = @_;
		if ($X3DNodeType eq "") {
			print "NodeType, X3DNodeType blank for $name\n";
			$X3DNodeType = "unknown";
		}
		my $this = bless {
						  Name => $name,
						  Defaults => {},
						  X3DNodeType => $X3DNodeType
						 },$type;
		my $t;
		for (keys %$fields) {
			#print "field key $_\n";
			if (ref $fields->{$_}[1] eq "ARRAY") {
				push @{$this->{Defaults}{$_}}, @{$fields->{$_}[1]};
			} else {
				$this->{Defaults}{$_} = $fields->{$_}[1];
			}
			$this->{FieldTypes}{$_} = $fields->{$_}[0];
			$t = $fields->{$_}[2];
			if (!defined $t) {
				die("Missing field or event type $type X3DNodeType $X3DNodeType for $_ in $name");
			}

			$this->{FieldKinds}{$_} = $t;
		}
		return $this;
    }
}

%VRML::Nodes = (
	###################################################################################

	#		Time Component

	###################################################################################

	TimeSensor => new VRML::NodeType("TimeSensor", {
		cycleInterval => [SFTime, 1, inputOutput],
		enabled => [SFBool, TRUE, inputOutput],
		loop => [SFBool, FALSE, inputOutput],
		startTime => [SFTime, 0, inputOutput],
		stopTime => [SFTime, 0, inputOutput],
		cycleTime => [SFTime, -1, outputOnly],
		fraction_changed => [SFFloat, 0.0, outputOnly],
		isActive => [SFBool, FALSE, outputOnly],
		time => [SFTime, -1, outputOnly],
		resumeTime => [SFTime,0,inputOutput],
		pauseTime => [SFTime,0,inputOutput],
		isPaused => [SFTime,0,outputOnly],
		metadata => [SFNode, NULL, inputOutput],

		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		# time that we were initialized at
		__inittime => [SFTime, 0, initializeOnly],
		# cycleTimer flag.
		__ctflag =>[SFTime, 10, inputOutput],
		__oldEnabled => [SFBool, TRUE, inputOutput],
	},"X3DSensorNode"),


	###################################################################################

	#		Networking Component

	###################################################################################

	Anchor => new VRML::NodeType("Anchor", {
		addChildren => [MFNode, undef, inputOnly],
		removeChildren => [MFNode, undef, inputOnly],
		children => [MFNode, [], inputOutput],
		description => [SFString, "", inputOutput],
		parameter => [MFString, [], inputOutput],
		url => [MFString, [], inputOutput],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
		__parenturl =>[SFString,"",initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DGroupingNode"),


	Inline => new VRML::NodeType("Inline", {
		url => [MFString, [], inputOutput],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
		load => [SFBool, TRUE,initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
                __children => [MFNode, [], inputOutput],
		__loadstatus =>[SFInt32,0,initializeOnly],
		__parenturl =>[SFString,"",initializeOnly],
	},"X3DNetworkSensorNode"),

	LoadSensor => new VRML::NodeType("LoadSensor", {
		enabled => [SFBool, FALSE,inputOutput],
		timeOut  => [SFTime,0,inputOutput],
		watchList => [MFNode, [], inputOutput],
		isActive  => [SFBool, TRUE,outputOnly],
		isLoaded  => [SFBool, TRUE,outputOnly],
		loadTime  => [SFTime,0,outputOnly],
		progress  => [SFFloat,0,outputOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__loading => [SFBool, TRUE,initializeOnly],		# current internal status
		__finishedloading => [SFBool, TRUE,initializeOnly],	# current internal status
		__StartLoadTime => [SFTime,0,outputOnly], # time we started loading...
		__oldEnabled => [SFBool, TRUE, inputOutput],
	},"X3DNetworkSensorNode"),

	###################################################################################

	#		Grouping Component

	###################################################################################

	Group => new VRML::NodeType("Group", {
		addChildren => [MFNode, undef, inputOnly],
		removeChildren => [MFNode, undef, inputOnly],
		children => [MFNode, [], inputOutput],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		FreeWRL__protoDef => [SFInt32, 0, initializeOnly], # tell renderer that this is a proto...
		FreeWRL_PROTOInterfaceNodes =>[MFNode, [], inputOutput],
	},"X3DGroupingNode"),

	StaticGroup => new VRML::NodeType("StaticGroup", {
		children => [MFNode, [], inputOutput],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__transparency => [SFInt32, -1, initializeOnly], # display list for transparencies
		__solid => [SFInt32, -1, initializeOnly],	 # display list for solid geoms.
	},"X3DGroupingNode"),

	Switch => new VRML::NodeType("Switch", {
		addChildren => [MFNode, undef, inputOnly],
		removeChildren => [MFNode, undef, inputOnly],
		choice => [MFNode, [], inputOutput],		# VRML nodes....
		children => [MFNode, [], inputOutput],		# X3D nodes....
		whichChoice => [SFInt32, -1, inputOutput],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__isX3D => [SFInt32, 0, initializeOnly], # 0 = VRML,  1 = X3D
	},"X3DGroupingNode"),

	Transform => new VRML::NodeType ("Transform", {
		addChildren => [MFNode, undef, inputOnly],
		removeChildren => [MFNode, undef, inputOnly],
		center => [SFVec3f, [0, 0, 0], inputOutput],
		children => [MFNode, [], inputOutput],
		rotation => [SFRotation, [0, 0, 1, 0], inputOutput],
		scale => [SFVec3f, [1, 1, 1], inputOutput],
		scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput],
		translation => [SFVec3f, [0, 0, 0], inputOutput],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro

		# fields for reducing redundant calls
		__do_center => [SFInt32, FALSE, initializeOnly],
		__do_trans => [SFInt32, FALSE, initializeOnly],
		__do_rotation => [SFInt32, FALSE, initializeOnly],
		__do_scaleO => [SFInt32, FALSE, initializeOnly],
		__do_scale => [SFInt32, FALSE, initializeOnly],
		__do_anything => [SFInt32, FALSE, initializeOnly],
	},"X3DGroupingNode"),
	

	###################################################################################

	#		Core Component

	###################################################################################


	WorldInfo => new VRML::NodeType("WorldInfo", {
		info => [MFString, [], initializeOnly],
		title => [SFString, "", initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DChildNode"),

	###################################################################################

	#		Rendering Component

	###################################################################################

	Color => new VRML::NodeType("Color", { 
		color => [MFColor, [], inputOutput], 
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DColorNode"),

	ColorRGBA => new VRML::NodeType("ColorRGBA", { 
		color => [MFColorRGBA, [], inputOutput], 
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DColorNode"),

	Coordinate => new VRML::NodeType("Coordinate", { 
		point => [MFVec3f, [], inputOutput],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DCoordinateNode"),

	IndexedLineSet => new VRML::NodeType("IndexedLineSet", {
		set_colorIndex => [MFInt32, undef, inputOnly],
		set_coordIndex => [MFInt32, undef, inputOnly],
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		fogCoord => [SFNode, NULL, inputOutput],
		colorIndex => [MFInt32, [], initializeOnly],
		colorPerVertex => [SFBool, TRUE, initializeOnly],
		coordIndex => [MFInt32, [], initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__vertArr  =>[FreeWRLPTR,0,initializeOnly],
		__vertIndx  =>[FreeWRLPTR,0,initializeOnly],
		__colours  =>[FreeWRLPTR,0,initializeOnly],
		__vertices  =>[FreeWRLPTR,0,initializeOnly],
		__vertexCount =>[FreeWRLPTR,0,initializeOnly],
		__segCount =>[SFInt32,0,initializeOnly],
	},"X3DGeometryNode"),

	IndexedTriangleFanSet => new VRML::NodeType("IndexedTriangleFanSet", {
		set_colorIndex => [MFInt32, undef, inputOnly],
		set_coordIndex => [MFInt32, undef, inputOnly],
		set_normalIndex => [MFInt32, undef, inputOnly],
		set_texCoordIndex => [MFInt32, undef, inputOnly],
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		normal => [SFNode, NULL, inputOutput],
		texCoord => [SFNode, NULL, inputOutput],
		ccw => [SFBool, TRUE, initializeOnly],
		colorIndex => [MFInt32, [], initializeOnly],
		colorPerVertex => [SFBool, TRUE, initializeOnly],
		convex => [SFBool, TRUE, initializeOnly],
		coordIndex => [MFInt32, [], initializeOnly],
		creaseAngle => [SFFloat, 0, initializeOnly],
		normalIndex => [MFInt32, [], initializeOnly],
		normalPerVertex => [SFBool, TRUE, initializeOnly],
		solid => [SFBool, TRUE, initializeOnly],
		texCoordIndex => [MFInt32, [], initializeOnly],
		index => [MFInt32, [], inputOutput],
		fanCount => [MFInt32, [], initializeOnly],
		stripCount => [MFInt32, [], initializeOnly],
		set_height => [MFFloat, undef, inputOnly],
		height => [MFFloat, [], initializeOnly],
		xDimension => [SFInt32, 0, initializeOnly],
		xSpacing => [SFFloat, 1.0, initializeOnly],
		zDimension => [SFInt32, 0, initializeOnly],
		zSpacing => [SFFloat, 1.0, initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__PolyStreamed => [SFBool, FALSE, initializeOnly],
	},"X3DGeometryNode"),

	IndexedTriangleSet => new VRML::NodeType("IndexedTriangleSet", {
		set_colorIndex => [MFInt32, undef, inputOnly],
		set_coordIndex => [MFInt32, undef, inputOnly],
		set_normalIndex => [MFInt32, undef, inputOnly],
		set_texCoordIndex => [MFInt32, undef, inputOnly],
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		normal => [SFNode, NULL, inputOutput],
		texCoord => [SFNode, NULL, inputOutput],
		ccw => [SFBool, TRUE, initializeOnly],
		colorIndex => [MFInt32, [], initializeOnly],
		colorPerVertex => [SFBool, TRUE, initializeOnly],
		convex => [SFBool, TRUE, initializeOnly],
		coordIndex => [MFInt32, [], initializeOnly],
		creaseAngle => [SFFloat, 0, initializeOnly],
		normalIndex => [MFInt32, [], initializeOnly],
		normalPerVertex => [SFBool, TRUE, initializeOnly],
		solid => [SFBool, TRUE, initializeOnly],
		texCoordIndex => [MFInt32, [], initializeOnly],
		index => [MFInt32, [], inputOutput],
		fanCount => [MFInt32, [], initializeOnly],
		stripCount => [MFInt32, [], initializeOnly],

		set_height => [MFFloat, undef, inputOnly],
		height => [MFFloat, [], initializeOnly],
		xDimension => [SFInt32, 0, initializeOnly],
		xSpacing => [SFFloat, 1.0, initializeOnly],
		zDimension => [SFInt32, 0, initializeOnly],
		zSpacing => [SFFloat, 1.0, initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__PolyStreamed => [SFBool, FALSE, initializeOnly],
	},"X3DGeometryNode"),

	IndexedTriangleStripSet => new VRML::NodeType("IndexedTriangleStripSet", {
		set_colorIndex => [MFInt32, undef, inputOnly],
		set_coordIndex => [MFInt32, undef, inputOnly],
		set_normalIndex => [MFInt32, undef, inputOnly],
		set_texCoordIndex => [MFInt32, undef, inputOnly],
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		normal => [SFNode, NULL, inputOutput],
		texCoord => [SFNode, NULL, inputOutput],
		ccw => [SFBool, TRUE, initializeOnly],
		colorIndex => [MFInt32, [], initializeOnly],
		colorPerVertex => [SFBool, TRUE, initializeOnly],
		convex => [SFBool, TRUE, initializeOnly],
		coordIndex => [MFInt32, [], initializeOnly],
		creaseAngle => [SFFloat, 0, initializeOnly],
		normalIndex => [MFInt32, [], initializeOnly],
		normalPerVertex => [SFBool, TRUE, initializeOnly],
		solid => [SFBool, TRUE, initializeOnly],
		texCoordIndex => [MFInt32, [], initializeOnly],
		index => [MFInt32, [], inputOutput],
		fanCount => [MFInt32, [], initializeOnly],
		stripCount => [MFInt32, [], initializeOnly],

		set_height => [MFFloat, undef, inputOnly],
		height => [MFFloat, [], initializeOnly],
		xDimension => [SFInt32, 0, initializeOnly],
		xSpacing => [SFFloat, 1.0, initializeOnly],
		zDimension => [SFInt32, 0, initializeOnly],
		zSpacing => [SFFloat, 1.0, initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__PolyStreamed => [SFBool, FALSE, initializeOnly],
	},"X3DGeometryNode"),

	LineSet => new VRML::NodeType("LineSet", {
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		vertexCount => [MFInt32,[],inputOutput],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__vertArr  =>[FreeWRLPTR,0,initializeOnly],
		__vertIndx  =>[FreeWRLPTR,0,initializeOnly],
		__segCount =>[SFInt32,0,initializeOnly],
	},"X3DGeometryNode"),

	Normal => new VRML::NodeType("Normal", { 
		vector => [MFVec3f, [], inputOutput] ,
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DNormalNode"),

	PointSet => new VRML::NodeType("PointSet", {
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DGeometryNode"),

	TriangleFanSet => new VRML::NodeType("TriangleFanSet", {
		set_colorIndex => [MFInt32, undef, inputOnly],
		set_coordIndex => [MFInt32, undef, inputOnly],
		set_normalIndex => [MFInt32, undef, inputOnly],
		set_texCoordIndex => [MFInt32, undef, inputOnly],
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		normal => [SFNode, NULL, inputOutput],
		texCoord => [SFNode, NULL, inputOutput],
		ccw => [SFBool, TRUE, initializeOnly],
		colorIndex => [MFInt32, [], initializeOnly],
		colorPerVertex => [SFBool, TRUE, initializeOnly],
		convex => [SFBool, TRUE, initializeOnly],
		coordIndex => [MFInt32, [], initializeOnly],
		creaseAngle => [SFFloat, 0, initializeOnly],
		normalIndex => [MFInt32, [], initializeOnly],
		normalPerVertex => [SFBool, TRUE, initializeOnly],
		solid => [SFBool, TRUE, initializeOnly],
		texCoordIndex => [MFInt32, [], initializeOnly],
		index => [MFInt32, [], inputOutput],
		fanCount => [MFInt32, [], initializeOnly],
		stripCount => [MFInt32, [], initializeOnly],

		set_height => [MFFloat, undef, inputOnly],
		height => [MFFloat, [], initializeOnly],
		xDimension => [SFInt32, 0, initializeOnly],
		xSpacing => [SFFloat, 1.0, initializeOnly],
		zDimension => [SFInt32, 0, initializeOnly],
		zSpacing => [SFFloat, 1.0, initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__PolyStreamed => [SFBool, FALSE, initializeOnly],
	},"X3DGeometryNode"),

	TriangleStripSet => new VRML::NodeType("TriangleStripSet", {
		set_colorIndex => [MFInt32, undef, inputOnly],
		set_coordIndex => [MFInt32, undef, inputOnly],
		set_normalIndex => [MFInt32, undef, inputOnly],
		set_texCoordIndex => [MFInt32, undef, inputOnly],
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		normal => [SFNode, NULL, inputOutput],
		texCoord => [SFNode, NULL, inputOutput],
		ccw => [SFBool, TRUE, initializeOnly],
		colorIndex => [MFInt32, [], initializeOnly],
		colorPerVertex => [SFBool, TRUE, initializeOnly],
		convex => [SFBool, TRUE, initializeOnly],
		coordIndex => [MFInt32, [], initializeOnly],
		creaseAngle => [SFFloat, 0, initializeOnly],
		normalIndex => [MFInt32, [], initializeOnly],
		normalPerVertex => [SFBool, TRUE, initializeOnly],
		solid => [SFBool, TRUE, initializeOnly],
		texCoordIndex => [MFInt32, [], initializeOnly],
		index => [MFInt32, [], inputOutput],
		fanCount => [MFInt32, [], initializeOnly],
		stripCount => [MFInt32, [], initializeOnly],

		set_height => [MFFloat, undef, inputOnly],
		height => [MFFloat, [], initializeOnly],
		xDimension => [SFInt32, 0, initializeOnly],
		xSpacing => [SFFloat, 1.0, initializeOnly],
		zDimension => [SFInt32, 0, initializeOnly],
		zSpacing => [SFFloat, 1.0, initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__PolyStreamed => [SFBool, FALSE, initializeOnly],
	},"X3DGeometryNode"),

	TriangleSet => new VRML::NodeType("TriangleSet", {
		set_colorIndex => [MFInt32, undef, inputOnly],
		set_coordIndex => [MFInt32, undef, inputOnly],
		set_normalIndex => [MFInt32, undef, inputOnly],
		set_texCoordIndex => [MFInt32, undef, inputOnly],
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		normal => [SFNode, NULL, inputOutput],
		texCoord => [SFNode, NULL, inputOutput],
		ccw => [SFBool, TRUE, initializeOnly],
		colorIndex => [MFInt32, [], initializeOnly],
		colorPerVertex => [SFBool, TRUE, initializeOnly],
		convex => [SFBool, TRUE, initializeOnly],
		coordIndex => [MFInt32, [], initializeOnly],
		creaseAngle => [SFFloat, 0, initializeOnly],
		normalIndex => [MFInt32, [], initializeOnly],
		normalPerVertex => [SFBool, TRUE, initializeOnly],
		solid => [SFBool, TRUE, initializeOnly],
		texCoordIndex => [MFInt32, [], initializeOnly],
		index => [MFInt32, [], inputOutput],
		fanCount => [MFInt32, [], initializeOnly],
		stripCount => [MFInt32, [], initializeOnly],

		set_height => [MFFloat, undef, inputOnly],
		height => [MFFloat, [], initializeOnly],
		xDimension => [SFInt32, 0, initializeOnly],
		xSpacing => [SFFloat, 1.0, initializeOnly],
		zDimension => [SFInt32, 0, initializeOnly],
		zSpacing => [SFFloat, 1.0, initializeOnly],
		metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__PolyStreamed => [SFBool, FALSE, initializeOnly],
	},"X3DGeometryNode"),


	###################################################################################

#			Shape Component

	###################################################################################

	Appearance => new VRML::NodeType ("Appearance", {
		lineProperties => [SFNode, NULL, inputOutput],
		fillProperties => [SFNode, NULL, inputOutput],
		material => [SFNode, NULL, inputOutput],
		shaders => [MFNode, [], inputOutput],
		texture => [SFNode, NULL, inputOutput],
		textureTransform => [SFNode, NULL, inputOutput],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DAppearanceNode"),

	FillProperties => new VRML::NodeType ("FillProperties", {
		filled => [SFBool, TRUE, inputOutput],
		hatchColor => [SFColor, [1,1,1], inputOutput],
		hatched => [SFBool, TRUE, inputOutput],
		hatchStyle => [SFInt32, 1, inputOutput],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DAppearanceChildNode"),

	LineProperties => new VRML::NodeType ("LineProperties", {
		applied => [SFBool, TRUE, inputOutput],
		linetype => [SFInt32, 1, inputOutput],
		linewidthScaleFactor => [SFFloat, 0, inputOutput],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DAppearanceChildNode"),

	Material => new VRML::NodeType ("Material", {
		ambientIntensity => [SFFloat, 0.2, inputOutput],
		diffuseColor => [SFColor, [0.8, 0.8, 0.8], inputOutput],
		emissiveColor => [SFColor, [0, 0, 0], inputOutput],
		shininess => [SFFloat, 0.2, inputOutput],
		specularColor => [SFColor, [0, 0, 0], inputOutput],
		transparency => [SFFloat, 0, inputOutput],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DMaterialNode"),

	Shape => new VRML::NodeType ("Shape", {
		appearance => [SFNode, NULL, inputOutput],
		geometry => [SFNode, NULL, inputOutput],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__visible =>[SFInt32,0,initializeOnly], # for Occlusion tests.
		__occludeCheckCount =>[SFInt32,-1,initializeOnly], # for Occlusion tests.
		__Samples =>[SFInt32,-1,initializeOnly],		# Occlude samples from last pass
	},"X3DBoundedObject"),

	TwoSidedMaterial => new VRML::NodeType ("TwoSidedMaterial", {
		ambientIntensity => [SFFloat, 0.2, inputOutput],
		diffuseColor => [SFColor, [0.8, 0.8, 0.8], inputOutput],
		emissiveColor => [SFColor, [0, 0, 0], inputOutput],
		shininess => [SFFloat, 0.2, inputOutput],
		specularColor => [SFColor, [0, 0, 0], inputOutput],
		transparency => [SFFloat, 0, inputOutput],

		backAmbientIntensity => [SFFloat, 0.2, inputOutput],
		backDiffuseColor => [SFColor, [0.8, 0.8, 0.8], inputOutput],
		backEmissiveColor => [SFColor, [0, 0, 0], inputOutput],
		backShininess => [SFFloat, 0.2, inputOutput],
		backSpecularColor => [SFColor, [0, 0, 0], inputOutput],
		backTransparency => [SFFloat, 0, inputOutput],

		separateBackColor =>[SFBool,FALSE,inputOutput],

                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DMaterialNode"),



	###################################################################################

	#		Geometry3D Component

	###################################################################################

	Box => new VRML::NodeType("Box", { 	
		size => [SFVec3f, [2, 2, 2], inputOutput], # see note top of file
		solid => [SFBool, TRUE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__points  =>[FreeWRLPTR,0,initializeOnly],
	},"X3DGeometryNode"),

	Cone => new VRML::NodeType ("Cone", {
		 bottomRadius => [SFFloat, 1.0, initializeOnly],
		 height => [SFFloat, 2.0, initializeOnly],
		 side => [SFBool, TRUE, initializeOnly],
		 bottom => [SFBool, TRUE, inputOutput], # see note top of file
		solid => [SFBool, TRUE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		 __sidepoints =>[FreeWRLPTR,0,initializeOnly],
		 __botpoints =>[FreeWRLPTR,0,initializeOnly],
		 __normals =>[FreeWRLPTR,0,initializeOnly],
	},"X3DGeometryNode"),

	Cylinder => new VRML::NodeType ("Cylinder", {
		 bottom => [SFBool, TRUE, inputOutput], # see note top of file
		 height => [SFFloat, 2.0, initializeOnly],
		 radius => [SFFloat, 1.0, inputOutput], # see note top of file
		 side => [SFBool, TRUE, initializeOnly],
		 top => [SFBool, TRUE, inputOutput], # see note top of file
		solid => [SFBool, TRUE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		 __points =>[FreeWRLPTR,0,initializeOnly],
		 __normals =>[FreeWRLPTR,0,initializeOnly],
	},"X3DGeometryNode"),

	ElevationGrid => new VRML::NodeType("ElevationGrid", {
		set_colorIndex => [MFInt32, undef, inputOnly],
		set_coordIndex => [MFInt32, undef, inputOnly],
		set_normalIndex => [MFInt32, undef, inputOnly],
		set_texCoordIndex => [MFInt32, undef, inputOnly],
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		normal => [SFNode, NULL, inputOutput],
		texCoord => [SFNode, NULL, inputOutput],
		ccw => [SFBool, TRUE, initializeOnly],
		colorIndex => [MFInt32, [], initializeOnly],
		colorPerVertex => [SFBool, TRUE, initializeOnly],
		convex => [SFBool, TRUE, initializeOnly],
		coordIndex => [MFInt32, [], initializeOnly],
		creaseAngle => [SFFloat, 0, initializeOnly],
		normalIndex => [MFInt32, [], initializeOnly],
		normalPerVertex => [SFBool, TRUE, initializeOnly],
		solid => [SFBool, TRUE, initializeOnly],
		texCoordIndex => [MFInt32, [], initializeOnly],
		index => [MFInt32, [], inputOutput],
		fanCount => [MFInt32, [], initializeOnly],
		stripCount => [MFInt32, [], initializeOnly],

		set_height => [MFFloat, undef, inputOnly],
		height => [MFFloat, [], initializeOnly],
		xDimension => [SFInt32, 0, initializeOnly],
		xSpacing => [SFFloat, 1.0, initializeOnly],
		zDimension => [SFInt32, 0, initializeOnly],
		zSpacing => [SFFloat, 1.0, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__PolyStreamed => [SFBool, FALSE, initializeOnly],
	},"X3DGeometryNode"),

	Extrusion => new VRML::NodeType("Extrusion", {
		set_crossSection => [MFVec2f, undef, inputOnly],
		set_orientation => [MFRotation, undef, inputOnly],
		set_scale => [MFVec2f, undef, inputOnly],
		set_spine => [MFVec3f, undef, inputOnly],
		beginCap => [SFBool, TRUE, initializeOnly],
		ccw => [SFBool, TRUE, initializeOnly],
		convex => [SFBool, TRUE, initializeOnly],
		creaseAngle => [SFFloat, 0, initializeOnly],
		crossSection => [MFVec2f, [[1, 1],[1, -1],[-1, -1],
						   [-1, 1],[1, 1]], initializeOnly],
		endCap => [SFBool, TRUE, initializeOnly],
		orientation => [MFRotation, [[0, 0, 1, 0]],inputOutput], # see note top of file

		scale => [MFVec2f, [[1, 1]], inputOutput], # see note top of file
		solid => [SFBool, TRUE, initializeOnly],
		spine => [MFVec3f, [[0, 0, 0],[0, 1, 0]], initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DGeometryNode"),

	IndexedFaceSet => new VRML::NodeType("IndexedFaceSet", {
		set_colorIndex => [MFInt32, undef, inputOnly],
		set_coordIndex => [MFInt32, undef, inputOnly],
		set_normalIndex => [MFInt32, undef, inputOnly],
		set_texCoordIndex => [MFInt32, undef, inputOnly],
		color => [SFNode, NULL, inputOutput],
		coord => [SFNode, NULL, inputOutput],
		normal => [SFNode, NULL, inputOutput],
		texCoord => [SFNode, NULL, inputOutput],
		ccw => [SFBool, TRUE, initializeOnly],
		colorIndex => [MFInt32, [], initializeOnly],
		colorPerVertex => [SFBool, TRUE, initializeOnly],
		convex => [SFBool, TRUE, initializeOnly],
		coordIndex => [MFInt32, [], initializeOnly],
		creaseAngle => [SFFloat, 0, initializeOnly],
		normalIndex => [MFInt32, [], initializeOnly],
		normalPerVertex => [SFBool, TRUE, initializeOnly],
		solid => [SFBool, TRUE, initializeOnly],
		texCoordIndex => [MFInt32, [], initializeOnly],
		index => [MFInt32, [], inputOutput],
		fanCount => [MFInt32, [], initializeOnly],
		stripCount => [MFInt32, [], initializeOnly],

		set_height => [MFFloat, undef, inputOnly],
		height => [MFFloat, [], initializeOnly],
		xDimension => [SFInt32, 0, initializeOnly],
		xSpacing => [SFFloat, 1.0, initializeOnly],
		zDimension => [SFInt32, 0, initializeOnly],
		zSpacing => [SFFloat, 1.0, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
                __oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__PolyStreamed => [SFBool, FALSE, initializeOnly],
	},"X3DGeometryNode"),

	Sphere => new VRML::NodeType("Sphere", { 	
		radius => [SFFloat, 1.0, inputOutput], # see note top of file
		solid => [SFBool, TRUE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__points =>[FreeWRLPTR,0,initializeOnly],
 	},"X3DGeometryNode"),


	###################################################################################

	#		Geometry 2D Component

	###################################################################################

	Arc2D => new VRML::NodeType("Arc2D", {
		endAngle => [SFFloat, 1.5707, initializeOnly],
		radius => [SFFloat, 1.0, inputOutput], # see note top of file
		startAngle => [SFFloat, 0.0, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__points  =>[FreeWRLPTR,0,initializeOnly],
		__numPoints =>[SFInt32,0,initializeOnly],
 	},"X3DGeometryNode"),

	ArcClose2D => new VRML::NodeType("ArcClose2D", {
			closureType => [SFString,"PIE",initializeOnly],
					    	endAngle => [SFFloat, 1.5707, initializeOnly],
					    	radius => [SFFloat, 1.0, inputOutput], # see note top of file
			solid => [SFBool, FALSE, initializeOnly],
					    	startAngle => [SFFloat, 0.0, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__points  =>[FreeWRLPTR,0,initializeOnly],
			__numPoints =>[SFInt32,0,initializeOnly],
 					   },"X3DGeometryNode"),


	Circle2D => new VRML::NodeType("Circle2D", {
					    	radius => [SFFloat, 1.0, inputOutput], # see note top of file
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__points  =>[FreeWRLPTR,0,initializeOnly],
			__numPoints =>[SFInt32,0,initializeOnly],
 					   },"X3DGeometryNode"),

	Disk2D => new VRML::NodeType("Disk2D", {
					    	innerRadius => [SFFloat, 0.0, initializeOnly],
					    	outerRadius => [SFFloat, 1.0, initializeOnly],
			solid => [SFBool, FALSE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__points  =>[FreeWRLPTR,0,initializeOnly],
			__texCoords  =>[FreeWRLPTR,0,initializeOnly],
			__numPoints =>[SFInt32,0,initializeOnly],
			__simpleDisk => [SFBool, TRUE,initializeOnly],
 					   },"X3DGeometryNode"),

	Polyline2D => new VRML::NodeType("Polyline2D", {
					    	lineSegments => [MFVec2f, [], initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
 					   },"X3DGeometryNode"),

	Polypoint2D => new VRML::NodeType("Polypoint2D", {
					    	point => [MFVec2f, [], inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
 					   },"X3DGeometryNode"),

	Rectangle2D => new VRML::NodeType("Rectangle2D", {
		size => [SFVec2f, [2.0, 2.0], inputOutput], # see note top of file
		solid => [SFBool, FALSE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__points  =>[FreeWRLPTR,0,initializeOnly],
			__numPoints =>[SFInt32,0,initializeOnly],
 					   },"X3DGeometryNode"),


	TriangleSet2D => new VRML::NodeType("TriangleSet2D", {
					    	vertices => [MFVec2f, [], inputOutput],
			solid => [SFBool, FALSE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__texCoords  =>[FreeWRLPTR,0,initializeOnly],
 					   },"X3DGeometryNode"),

	###################################################################################

	#		Text Component

	###################################################################################

	Text => new VRML::NodeType ("Text", {
			 string => [MFString, [], inputOutput],
			 fontStyle => [SFNode, NULL, inputOutput],
			 length => [MFFloat, [], inputOutput],
			solid => [SFBool, TRUE, initializeOnly],
			 maxExtent => [SFFloat, 0, inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			 __rendersub => [SFInt32, 0, inputOutput] # Function ptr hack
			},"X3DTextNode"),

	FontStyle => new VRML::NodeType("FontStyle", {
			family => [MFString, ["SERIF"], initializeOnly],
			horizontal => [SFBool, TRUE, initializeOnly],
			justify => [MFString, ["BEGIN"], initializeOnly],
			language => [SFString, "", initializeOnly],
			leftToRight => [SFBool, TRUE, initializeOnly],
			size => [SFFloat, 1.0, inputOutput], # see note top of file
			spacing => [SFFloat, 1.0, initializeOnly],
			style => [SFString, "PLAIN", initializeOnly],
			topToBottom => [SFBool, TRUE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					   },"X3DFontStyleNode"), 

	###################################################################################

	#		Sound Component

	###################################################################################

	AudioClip => new VRML::NodeType("AudioClip", {
			description => [SFString, "", inputOutput],
			loop =>	[SFBool, FALSE, inputOutput],
			pitch => [SFFloat, 1.0, inputOutput],
			startTime => [SFTime, 0, inputOutput],
			stopTime => [SFTime, 0, inputOutput],
			url => [MFString, [], inputOutput],
			duration_changed => [SFTime, -1, outputOnly],
			isActive => [SFBool, FALSE, outputOnly],

			pauseTime => [SFTime,0,inputOutput],
			resumeTime => [SFTime,0,inputOutput],
			elapsedTime => [SFTime,0,outputOnly],
			isPaused => [SFBool, TRUE,outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro

			# parent url, gets replaced at node build time
			__parenturl =>[SFString,"",initializeOnly],

			# internal sequence number
			__sourceNumber => [SFInt32, -1, initializeOnly],
			# local name, as received on system
			__localFileName => [FreeWRLPTR, 0,initializeOnly],
			# time that we were initialized at
			__inittime => [SFTime, 0, initializeOnly],
					   },"X3DSoundSourceNode"),

	Sound => new VRML::NodeType("Sound", {
			direction => [SFVec3f, [0, 0, 1], inputOutput],
			intensity => [SFFloat, 1.0, inputOutput],
			location => [SFVec3f, [0, 0, 0], inputOutput],
			maxBack => [SFFloat, 10.0, inputOutput],
			maxFront => [SFFloat, 10.0, inputOutput],
			minBack => [SFFloat, 1.0, inputOutput],
			minFront => [SFFloat, 1.0, inputOutput],
			priority => [SFFloat, 0, inputOutput],
			source => [SFNode, NULL, inputOutput],
			spatialize => [SFBool, FALSE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					   },"X3DSoundSourceNode"),
	
	# for testing MIDI sounds
	AudioControl => new VRML::NodeType("AudioControl", {
			direction => [SFVec3f, [0, 0, 1], inputOutput],
			intensity => [SFFloat, 1.0, inputOutput],
			location => [SFVec3f, [0, 0, 0], inputOutput],
			maxBack => [SFFloat, 10.0, inputOutput],
			maxFront => [SFFloat, 10.0, inputOutput],
			minBack => [SFFloat, 1.0, inputOutput],
			minFront => [SFFloat, 1.0, inputOutput],
			enabled => [SFBool, TRUE, inputOutput],
			source => [SFString, "", inputOutput],

			isActive => [SFBool, FALSE, outputOnly],
			# need distance, pan position as ints and floats
			volumeInt32Val => [SFInt32, 0, outputOnly],
			volumeFloatVal => [SFFloat, 0.0, outputOnly],
			panInt32Val => [SFInt32, 0, outputOnly],
			panFloatVal => [SFFloat, 0.0, outputOnly],
			deltaInt32Val => [SFInt32, 0, outputOnly],
			deltaFloatVal => [SFFloat, 0.0, outputOnly],

                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro

			# used for determing rate of change of position:
			__oldLen =>[SFTime, 0.0, initializeOnly],
			maxDelta => [SFFloat, 10.0, inputOutput],
			__oldEnabled => [SFBool, TRUE, inputOutput],
			

					   },"X3DSoundSourceNode"),

	###################################################################################

	#		Lighting Component

	###################################################################################

	DirectionalLight => new VRML::NodeType("DirectionalLight", {
			ambientIntensity => [SFFloat, 0, inputOutput],
			color => [SFColor, [1, 1, 1], inputOutput],
			direction => [SFVec3f, [0, 0, -1], inputOutput],
			intensity => [SFFloat, 1.0, inputOutput],
			on => [SFBool, TRUE, inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					   },"X3DLightNode"),

	PointLight => new VRML::NodeType("PointLight", {
			ambientIntensity => [SFFloat, 0, inputOutput],
			attenuation => [SFVec3f, [1, 0, 0], inputOutput],
			color => [SFColor, [1, 1, 1], inputOutput],
			intensity => [SFFloat, 1.0, inputOutput],
			location => [SFVec3f, [0, 0, 0], inputOutput],
			on => [SFBool, TRUE, inputOutput],
			radius => [SFFloat, 100.0, inputOutput],
			##not in the spec
			direction => [SFVec3f, [0, 0, -1.0], inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					   },"X3DLightNode"),

	SpotLight => new VRML::NodeType("SpotLight", {
			ambientIntensity => [SFFloat, 0, inputOutput],
			attenuation => [SFVec3f, [1, 0, 0], inputOutput],
			beamWidth => [SFFloat, 1.570796, inputOutput],
			color => [SFColor, [1, 1, 1], inputOutput],
			cutOffAngle => [SFFloat, 0.785398, inputOutput],
			direction => [SFVec3f, [0, 0, -1], inputOutput],
			intensity => [SFFloat, 1.0, inputOutput],
			location => [SFVec3f, [0, 0, 0], inputOutput],
			on => [SFBool, TRUE, inputOutput],
			radius => [SFFloat, 100.0, inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					   },"X3DLightNode"),

	###################################################################################

	#		Texturing Component

	###################################################################################

	ImageTexture => new VRML::NodeType("ImageTexture", {
			url => [MFString, [], inputOutput],
			repeatS => [SFBool, TRUE, initializeOnly],
			repeatT => [SFBool, TRUE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__textureTableIndex => [SFInt32, 0, initializeOnly],
			__parenturl =>[SFString,"",initializeOnly],
					   },"X3DTextureNode"),

	MovieTexture => new VRML::NodeType ("MovieTexture", {
			 loop => [SFBool, FALSE, inputOutput],
			 speed => [SFFloat, 1.0, inputOutput],
			 startTime => [SFTime, 0, inputOutput],
			 stopTime => [SFTime, 0, inputOutput],
			 url => [MFString, [""], inputOutput],
			 repeatS => [SFBool, TRUE, initializeOnly],
			 repeatT => [SFBool, TRUE, initializeOnly],
			 duration_changed => [SFTime, -1, outputOnly],
			 isActive => [SFBool, FALSE, outputOnly],
			resumeTime => [SFTime,0,inputOutput],
			pauseTime => [SFTime,0,inputOutput],
			elapsedTime => [SFTime,0,outputOnly],
			isPaused => [SFTime,0,outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__textureTableIndex => [SFInt32, 0, initializeOnly],
			 # has the URL changed???
			 __oldurl => [MFString, [""], initializeOnly],
			 # initial texture number
			 __texture0_ => [SFInt32, 0, initializeOnly],
			 # last texture number
			 __texture1_ => [SFInt32, 0, initializeOnly],
			 # which texture number is used
			 __ctex => [SFInt32, 0, initializeOnly],
			 # time that we were initialized at
			 __inittime => [SFTime, 0, initializeOnly],
			 # internal sequence number
			 __sourceNumber => [SFInt32, -1, initializeOnly],
			# parent url, gets replaced at node build time
			__parenturl =>[SFString,"",initializeOnly],
			},"X3DTextureNode"),


	MultiTexture => new VRML::NodeType("MultiTexture", {
			alpha =>[SFFloat, 1, inputOutput],
			color =>[SFColor,[1,1,1],inputOutput],
			function =>[MFString,[],inputOutput],
			mode =>[MFString,[],inputOutput],
			source =>[MFString,[],inputOutput],
			texture=>[MFNode,undef,inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__params => [FreeWRLPTR, 0, initializeOnly],
					   },"X3DTextureNode"),

	MultiTextureCoordinate => new VRML::NodeType("MultiTextureCoordinate", {
			texCoord =>[MFNode,undef,inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					   },"X3DTextureCoordinateNode"),

	MultiTextureTransform => new VRML::NodeType("MultiTextureTransform", {
			textureTransform=>[MFNode,undef,inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					   },"X3DTextureTransformNode"),

	PixelTexture => new VRML::NodeType("PixelTexture", {
			image => [SFImage, "0, 0, 0", inputOutput],
			repeatS => [SFBool, TRUE, initializeOnly],
			repeatT => [SFBool, TRUE, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__parenturl =>[SFString,"",initializeOnly],
			__textureTableIndex => [SFInt32, 0, initializeOnly],
					   },"X3DTextureNode"),

	TextureCoordinate => new VRML::NodeType("TextureCoordinate", { 
			point => [MFVec2f, [], inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__compiledpoint => [MFVec2f, [], initializeOnly],
			__lastParent => [FreeWRLPTR, 0, initializeOnly],
					 },"X3DTextureCoordinateNode"),

	TextureCoordinateGenerator => new VRML::NodeType("TextureCoordinateGenerator", { 
			parameter => [MFFloat, [], inputOutput],
			mode => [SFString,"SPHERE",inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__compiledmode => [SFInt32,0,initializeOnly],
					 },"X3DTextureCoordinateNode"),

	TextureTransform => new VRML::NodeType ("TextureTransform", {
			 center => [SFVec2f, [0, 0], inputOutput],
			 rotation => [SFFloat, 0, inputOutput],
			 scale => [SFVec2f, [1, 1], inputOutput],
			 translation => [SFVec2f, [0, 0], inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			},"X3DTextureTransformNode"),

	###################################################################################

	#		Cubemap Texturing Component

	###################################################################################


	ComposedCubeMapTexture => new VRML::NodeType("ComposedCubeMapTexture", {
		metadata => [SFNode, NULL, inputOutput],
		back =>[SFNode,NULL,inputOutput],
		bottom =>[SFNode,NULL,inputOutput],
		front =>[SFNode,NULL,inputOutput],
		left =>[SFNode,NULL,inputOutput],
		top =>[SFNode,NULL,inputOutput],
		right =>[SFNode,NULL,inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DEnvironmentTextureNode"),

	GeneratedCubeMapTexture => new VRML::NodeType("GeneratedCubeMapTexture", {
		metadata => [SFNode, NULL, inputOutput],
		update => [SFString,"NONE",inputOutput],
		size => [SFInt32,128,inputOutput], # see note top of file
		textureProperties => [SFNode, NULL, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DEnvironmentTextureNode"),

	ImageCubeMapTexture => new VRML::NodeType("ImageCubeMapTexture", {
		metadata => [SFNode, NULL, inputOutput],
		url => [MFString,[],inputOutput],
		textureProperties => [SFNode, NULL, initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		__parenturl =>[SFString,"",initializeOnly],
	},"X3DEnvironmentTextureNode"),


	###################################################################################

	#		Interpolation Component

	###################################################################################

	ColorInterpolator => new VRML::NodeType("ColorInterpolator", {
			set_fraction => [SFFloat, undef, inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFColor, [], inputOutput],
			value_changed => [SFColor, [0, 0, 0], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		   },"X3DInterpolatorNode"),

	############################################################################33
	# CoordinateInterpolators and NormalInterpolators use almost the same thing;
	# look at the _type field.

	CoordinateInterpolator => new VRML::NodeType("CoordinateInterpolator", {
			set_fraction => [SFFloat, undef, inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFVec3f, [], inputOutput],
			value_changed => [MFVec3f, [], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			_type => [SFInt32, 0, inputOutput], #1 means dont normalize
		},"X3DInterpolatorNode"),

	NormalInterpolator => new VRML::NodeType("NormalInterpolator", {
			set_fraction => [SFFloat, undef, inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFVec3f, [], inputOutput],
			value_changed => [MFVec3f, [], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			_type => [SFInt32, 1, inputOutput], #1 means normalize
		},"X3DInterpolatorNode"),

	OrientationInterpolator => new VRML::NodeType("OrientationInterpolator", {
			set_fraction => [SFFloat, undef, inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFRotation, [], inputOutput],
			value_changed => [SFRotation, [0, 0, 1, 0], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		   },"X3DInterpolatorNode"),

	PositionInterpolator => new VRML::NodeType("PositionInterpolator", {
			set_fraction => [SFFloat, undef, inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFVec3f, [], inputOutput],
			value_changed => [SFVec3f, [0, 0, 0], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		   },"X3DInterpolatorNode"),


	ScalarInterpolator => new VRML::NodeType("ScalarInterpolator", {
			set_fraction => [SFFloat, undef, inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFFloat, [], inputOutput],
			value_changed => [SFFloat, 0.0, outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		   },"X3DInterpolatorNode"),

	CoordinateInterpolator2D => new VRML::NodeType("CoordinateInterpolator2D", {
			set_fraction => [SFFloat, undef, inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFVec2f, [], inputOutput],
			value_changed => [MFVec2f, [[0,0]], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		},"X3DInterpolatorNode"),

	PositionInterpolator2D => new VRML::NodeType("PositionInterpolator2D", {
			set_fraction => [SFFloat, undef, inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFVec2f, [], inputOutput],
			value_changed => [SFVec2f, [0, 0, 0], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
		},"X3DInterpolatorNode"),


	###################################################################################

	#		Pointing Device Component

	###################################################################################

	TouchSensor => new VRML::NodeType("TouchSensor", {
			enabled => [SFBool, TRUE, inputOutput],
			hitNormal_changed => [SFVec3f, [0, 0, 0], outputOnly],
			hitPoint_changed => [SFVec3f, [0, 0, 0], outputOnly],
			hitTexCoord_changed => [SFVec2f, [0, 0], outputOnly],
			_oldhitNormal => [SFVec3f, [0, 0, 0], outputOnly], 	# send event only if changed
			_oldhitPoint => [SFVec3f, [0, 0, 0], outputOnly], 	# send event only if changed
			_oldhitTexCoord => [SFVec2f, [0, 0], outputOnly], 	# send event only if changed
			isActive => [SFBool, FALSE, outputOnly],
			isOver => [SFBool, FALSE, outputOnly],
			description => [SFString, "", inputOutput], # see note top of file
			touchTime => [SFTime, -1, outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__oldEnabled => [SFBool, TRUE, inputOutput],
					   },"X3DPointingDeviceSensorNode"),

	PlaneSensor => new VRML::NodeType("PlaneSensor", {
			autoOffset => [SFBool, TRUE, inputOutput],
			enabled => [SFBool, TRUE, inputOutput],
			maxPosition => [SFVec2f, [-1, -1], inputOutput],
			minPosition => [SFVec2f, [0, 0], inputOutput],
			offset => [SFVec3f, [0, 0, 0], inputOutput],
			isActive => [SFBool, FALSE, outputOnly],
			isOver => [SFBool, FALSE, outputOnly],
			description => [SFString, "", inputOutput], # see note top of file
			trackPoint_changed => [SFVec3f, [0, 0, 0], outputOnly],
			translation_changed => [SFVec3f, [0, 0, 0], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			_oldtrackPoint => [SFVec3f, [0, 0, 0], outputOnly],
			_oldtranslation => [SFVec3f, [0, 0, 0], outputOnly],
			# where we are at a press...
			_origPoint => [SFVec3f, [0, 0, 0], initializeOnly],
			__oldEnabled => [SFBool, TRUE, inputOutput],
					   },"X3DPointingDeviceSensorNode"),

	SphereSensor => new VRML::NodeType("SphereSensor", {
			autoOffset => [SFBool, TRUE, inputOutput],
			enabled => [SFBool, TRUE, inputOutput],
			offset => [SFRotation, [0, 1, 0, 0], inputOutput],
			isActive => [SFBool, FALSE, outputOnly],
			rotation_changed => [SFRotation, [0, 0, 1, 0], outputOnly],
			trackPoint_changed => [SFVec3f, [0, 0, 0], outputOnly],
			_oldtrackPoint => [SFVec3f, [0, 0, 0], outputOnly],
			_oldrotation => [SFRotation, [0, 0, 1, 0], outputOnly],
			isOver => [SFBool, FALSE, outputOnly],
			description => [SFString, "", inputOutput], # see note top of file
			# where we are at a press...
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			_origPoint => [SFVec3f, [0, 0, 0], initializeOnly],
			_radius => [SFFloat, 0, initializeOnly],
			__oldEnabled => [SFBool, TRUE, inputOutput],
					   },"X3DPointingDeviceSensorNode"),

	CylinderSensor => new VRML::NodeType("CylinderSensor", {
			autoOffset => [SFBool, TRUE, inputOutput],
			diskAngle => [SFFloat, 0.262, inputOutput],
			enabled => [SFBool, TRUE, inputOutput],
			maxAngle => [SFFloat, -1.0, inputOutput],
			minAngle => [SFFloat, 0, inputOutput],
			offset => [SFFloat, 0, inputOutput],
			isActive => [SFBool, FALSE, outputOnly],
			isOver => [SFBool, FALSE, outputOnly],
			description => [SFString, "", inputOutput], # see note top of file
			rotation_changed => [SFRotation, [0, 0, 1, 0], outputOnly],
			trackPoint_changed => [SFVec3f, [0, 0, 0], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			_oldtrackPoint => [SFVec3f, [0, 0, 0], outputOnly],
			_oldrotation => [SFRotation, [0, 0, 1, 0], outputOnly],
			# where we are at a press...
			_origPoint => [SFVec3f, [0, 0, 0], initializeOnly],
			_radius => [SFFloat, 0, initializeOnly],
			_dlchange => [SFInt32, 0, initializeOnly],
			__oldEnabled => [SFBool, TRUE, inputOutput],
					   },"X3DPointingDeviceSensorNode"),


	###################################################################################

	#		Key Device Component

	###################################################################################

	# KeySensor
	KeySensor => new VRML::NodeType("KeySensor", {
			enabled => [SFBool, TRUE, inputOutput],
			actionKeyPress =>[SFInt32,0,outputOnly],
			actionKeyRelease =>[SFInt32,0,outputOnly],
			altKey =>[SFBool, TRUE,outputOnly],
			controlKey =>[SFBool, TRUE,outputOnly],
			isActive =>[SFBool, TRUE,outputOnly],
			keyPress =>[SFString,"",outputOnly],
			keyRelease =>[SFString,"",outputOnly],
			shiftKey =>[SFBool, TRUE,outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__oldEnabled => [SFBool, TRUE, inputOutput],
					   },"X3DKeyDeviceSensorNode"),

	# StringSensor
	StringSensor => new VRML::NodeType("StringSensor", {
			deletionAllowed => [SFBool, TRUE, inputOutput],
			enabled => [SFBool, TRUE, inputOutput],
			enteredText => [SFString,"",outputOnly],
			finalText => [SFString,"",outputOnly],
			isActive =>[SFBool, TRUE,outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			_initialized =>[SFBool, FALSE,initializeOnly],
			__oldEnabled => [SFBool, TRUE, inputOutput],
					   },"X3DKeyDeviceSensorNode"),


	###################################################################################

	#		Environmental Sensor Component

	###################################################################################


	ProximitySensor => new VRML::NodeType("ProximitySensor", {
			center => [SFVec3f, [0, 0, 0], inputOutput],
			size => [SFVec3f, [0, 0, 0], inputOutput],
			enabled => [SFBool, TRUE, inputOutput],
			isActive => [SFBool, FALSE, outputOnly],
			position_changed => [SFVec3f, [0, 0, 0], outputOnly],
			orientation_changed => [SFRotation, [0, 0, 1, 0], outputOnly],
			enterTime => [SFTime, -1, outputOnly],
			exitTime => [SFTime, -1, outputOnly],
			centerOfRotation_changed =>[SFVec3f, [0,0,0], outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro

			# These fields are used for the info.
			__hit => [SFInt32, 0, inputOutput],
			__t1 => [SFVec3f, [10000000, 0, 0], inputOutput],
			__t2 => [SFRotation, [0, 1, 0, 0], inputOutput],
			__oldEnabled => [SFBool, TRUE, inputOutput],
					   },"X3DEnvironmentalSensorNode"),

	VisibilitySensor => new VRML::NodeType("VisibilitySensor", {
			center => [SFVec3f, [0, 0, 0], inputOutput],
			enabled => [SFBool, TRUE, inputOutput],
			size => [SFVec3f, [0, 0, 0], inputOutput],
			enterTime => [SFTime, -1, outputOnly],
			exitTime => [SFTime, -1, outputOnly],
			isActive => [SFBool, FALSE, outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			 __visible =>[SFInt32,0,initializeOnly], # for Occlusion tests.
			 __occludeCheckCount =>[SFInt32,-1,initializeOnly], # for Occlusion tests.
			__points  =>[FreeWRLPTR,0,initializeOnly],	# for Occlude Box.
			__Samples =>[SFInt32,0,initializeOnly],		# Occlude samples from last pass
			__oldEnabled => [SFBool, TRUE, inputOutput],
					   },"X3DEnvironmentalSensorNode"),



	###################################################################################

	#		Navigation Component

	###################################################################################

	LOD => new VRML::NodeType("LOD", {
			 addChildren => [MFNode, undef, inputOnly],
			 removeChildren => [MFNode, undef, inputOnly],
			level => [MFNode, [], inputOutput], 		# for VRML spec
			children => [MFNode, [], inputOutput],		# for X3D spec
			center => [SFVec3f, [0, 0, 0],  inputOutput ], # see note top of file
			range => [MFFloat, [], initializeOnly],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__isX3D => [SFInt32, 0, initializeOnly], # 0 = VRML,  1 = X3D
			_selected =>[FreeWRLPTR,0,initializeOnly],
					   },"X3DGroupingNode"),

	Billboard => new VRML::NodeType("Billboard", {
			addChildren => [MFNode, undef, inputOnly],
			removeChildren => [MFNode, undef, inputOnly],
			axisOfRotation => [SFVec3f, [0, 1, 0], inputOutput],
			children => [MFNode, [], inputOutput],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			_rotationAngle =>[SFDouble, 0, initializeOnly],
					   },"X3DGroupingNode"),

	Collision => new VRML::NodeType("Collision", {
			addChildren => [MFNode, undef, inputOnly],
			removeChildren => [MFNode, undef, inputOnly],
			children => [MFNode, [], inputOutput],
			enabled => [SFBool, TRUE, inputOutput],
			collide => [SFBool, TRUE, inputOutput],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
			proxy => [SFNode, NULL, initializeOnly],
			collideTime => [SFTime, -1, outputOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			# return info for collisions
			# bit 0 : collision or not
			# bit 1: changed from previous of not
			__hit => [SFInt32, 0, inputOutput]
					   },"X3DEnvironmentalSensorNode"),

	Viewpoint => new VRML::NodeType("Viewpoint", {
			set_bind => [SFBool, undef, inputOnly],
			fieldOfView => [SFFloat, 0.785398, inputOutput],
			jump => [SFBool, TRUE, inputOutput],
			orientation => [SFRotation, [0, 0, 1, 0], inputOutput],
			position => [SFVec3f,[0, 0, 10], inputOutput],
			description => [SFString, "", inputOutput], # see note top of file
			bindTime => [SFTime, -1, outputOnly],
			isBound => [SFBool, FALSE, outputOnly],
			centerOfRotation =>[SFVec3f, [0,0,0], inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__BGNumber => [SFInt32,-1,initializeOnly], # for ordering backgrounds for binding
					   },"X3DBindableNode"),

	NavigationInfo => new VRML::NodeType("NavigationInfo", {
			set_bind => [SFBool, undef, inputOnly],
			avatarSize => [MFFloat, [0.25, 1.6, 0.75], inputOutput],
			headlight => [SFBool, TRUE, inputOutput],
			speed => [SFFloat, 1.0, inputOutput],
			type => [MFString, ["WALK", "ANY"], inputOutput],
			visibilityLimit => [SFFloat, 0, inputOutput],
			isBound => [SFBool, FALSE, outputOnly],
			transitionType => [MFString, [],inputOutput],
			bindTime => [SFTime, -1, outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__BGNumber => [SFInt32,-1,initializeOnly], # for ordering backgrounds for binding
					   },"X3DBindableNode"),

	###################################################################################

	#		Environmental Effects Component

	###################################################################################

	Background => new VRML::NodeType("Background", {
			set_bind => [SFBool, undef, inputOnly],
			groundAngle => [MFFloat, [], inputOutput],
			groundColor => [MFColor, [], inputOutput],
			skyAngle => [MFFloat, [], inputOutput],
			skyColor => [MFColor, [[0, 0, 0]], inputOutput],
			bindTime => [SFTime,0,outputOnly],
			isBound => [SFBool, FALSE, outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__parenturl =>[SFString,"",initializeOnly],
			__points =>[FreeWRLPTR,0,initializeOnly],
			__colours =>[FreeWRLPTR,0,initializeOnly],
			__quadcount => [SFInt32,0,initializeOnly],
			__BGNumber => [SFInt32,-1,initializeOnly], # for ordering backgrounds for binding

			frontUrl => [MFString, [], inputOutput],
			backUrl => [MFString, [], inputOutput],
			topUrl => [MFString, [], inputOutput],
			bottomUrl => [MFString, [], inputOutput],
			leftUrl => [MFString, [], inputOutput],
			rightUrl => [MFString, [], inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__textureright => [SFInt32, 0, inputOutput],
			__frontTexture=>[SFNode,NULL,inputOutput],
			__backTexture=>[SFNode,NULL,inputOutput],
			__topTexture=>[SFNode,NULL,inputOutput],
			__bottomTexture=>[SFNode,NULL,inputOutput],
			__leftTexture=>[SFNode,NULL,inputOutput],
			__rightTexture=>[SFNode,NULL,inputOutput],
					   },"X3DBackgroundNode"),



	Fog => new VRML::NodeType("Fog", {
			set_bind => [SFBool, undef, inputOnly],
			color => [SFColor, [1, 1, 1], inputOutput],
			fogType => [SFString, "LINEAR", inputOutput],
			visibilityRange => [SFFloat, 0, inputOutput],
			bindTime => [SFTime, -1, outputOnly],
			isBound => [SFBool, FALSE, outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__BGNumber => [SFInt32,-1,initializeOnly], # for ordering backgrounds for binding
					   },"X3DBindableNode"),

	FogCoordinate => new VRML::NodeType("FogCoordinate", {
			depth => [MFFloat, [], inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					   },"X3DGeometricPropertyNode"),

	LocalFog => new VRML::NodeType("Fog", {
			color => [SFColor, [1, 1, 1], inputOutput],
			enabled => [SFBool, TRUE, inputOutput],
			fogType => [SFString, "LINEAR", inputOutput],
			visibilityRange => [SFFloat, 0, inputOutput],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					   },"X3DChildNode"),

	TextureBackground => new VRML::NodeType("TextureBackground", {
			set_bind => [SFBool, undef, inputOnly],
			groundAngle => [MFFloat, [], inputOutput],
			groundColor => [MFColor, [], inputOutput],
			skyAngle => [MFFloat, [], inputOutput],
			skyColor => [MFColor, [[0,0,0]], inputOutput],
			bindTime => [SFTime,0,outputOnly],
			isBound => [SFBool, FALSE, outputOnly],
                metadata => [SFNode, NULL, inputOutput],
		__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__parenturl =>[SFString,"",initializeOnly],
			__points =>[FreeWRLPTR,0,initializeOnly],
			__colours =>[FreeWRLPTR,0,initializeOnly],
			__quadcount => [SFInt32,0,initializeOnly],
			__BGNumber => [SFInt32,-1,initializeOnly], # for ordering backgrounds for binding

			frontTexture=>[SFNode,NULL,inputOutput],
			backTexture=>[SFNode,NULL,inputOutput],
			topTexture=>[SFNode,NULL,inputOutput],
			bottomTexture=>[SFNode,NULL,inputOutput],
			leftTexture=>[SFNode,NULL,inputOutput],
			rightTexture=>[SFNode,NULL,inputOutput],
			transparency=> [MFFloat,[0],inputOutput],
					   },"X3DBackgroundNode"),

	###################################################################################

	#		Geospatial Component

	###################################################################################


	GeoCoordinate => new VRML::NodeType("GeoCoordinate", {
			metadata => [SFNode, NULL, inputOutput],
			point => [MFVec3d,[],inputOutput], # see note top of file
			geoOrigin => [SFNode, NULL, initializeOnly],
			geoSystem => [MFString,["GD","WE"],initializeOnly],
			__geoSystem => [MFInt32,[],initializeOnly],
			__movedCoords => [MFVec3f, [], inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DCoordinateNode"),

	GeoElevationGrid => new VRML::NodeType("GeoElevationGrid", {
			set_height => [MFDouble, undef, inputOnly],
			color => [SFNode, NULL, inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			normal => [SFNode, NULL, inputOutput],
			texCoord => [SFNode, NULL, inputOutput],
			yScale => [SFFloat, 1.0, initializeOnly],
			ccw => [SFBool, FALSE,initializeOnly],
			colorPerVertex => [SFBool, TRUE, initializeOnly],
			creaseAngle => [SFDouble, 0, initializeOnly],
			geoGridOrigin => [SFVec3d,[0,0,0],initializeOnly],
			geoOrigin => [SFNode, NULL, initializeOnly],
			geoSystem => [MFString,["GD","WE"],initializeOnly],
			height => [MFDouble, [0,0], initializeOnly],
			normalPerVertex => [SFBool, TRUE, initializeOnly],
			solid => [SFBool, TRUE, initializeOnly],
			xDimension => [SFInt32, 0, initializeOnly],
			xSpacing => [SFDouble, 1.0, initializeOnly],
			zDimension => [SFInt32, 0, initializeOnly],
			zSpacing => [SFDouble, 1.0, initializeOnly],

			__geoSystem => [MFInt32,[],initializeOnly],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__realElevationGrid => [SFNode, NULL, initializeOnly],
		
					},"X3DGeometryNode"),

	GeoLOD => new VRML::NodeType("GeoLOD", {
			metadata => [SFNode, NULL, inputOutput],

			# the following screws up routing in the old VRML parser, because children can
			# be an "EXPOSED_FIELD" AND an "EVENT_OUT", so by changing this to an "inputOutput"
			# we can have only one field, the EXPOSED_FIELD_children
			#children => [MFNode,[],outputOnly],

			children => [MFNode, [], inputOutput],
			level_changed =>[SFInt32,0,outputOnly],

			center => [SFVec3d,[0,0,0],inputOutput], # see note top of file
			child1Url =>[MFString,[],initializeOnly],
			child2Url =>[MFString,[],initializeOnly],
			child3Url =>[MFString,[],initializeOnly],
			child4Url =>[MFString,[],initializeOnly],
			geoOrigin => [SFNode, NULL, initializeOnly],
			geoSystem => [MFString,["GD","WE"],initializeOnly],
			range => [SFFloat,10.0,initializeOnly],
			rootUrl => [MFString,[],initializeOnly],
			rootNode => [MFNode,[],initializeOnly],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],

			__geoSystem => [MFInt32,[],initializeOnly],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput],
			__inRange =>[SFBool, FALSE, inputOutput],
			__child1Node => [SFNode, NULL, inputOutput],
			__child2Node => [SFNode, NULL, inputOutput],
			__child3Node => [SFNode, NULL, inputOutput],
			__child4Node => [SFNode, NULL, inputOutput],
			__rootUrl => [SFNode, NULL, inputOutput],
			__childloadstatus => [SFInt32,0,inputOutput],
			__rooturlloadstatus => [SFInt32,0,inputOutput],

			# ProximitySensor copies.
			__inRange => [SFInt32, 0, inputOutput],
			__t1 => [SFVec3d, [10000000, 0, 0], inputOutput],

			__level => [SFInt32,-1,inputOutput], # only for debugging purposes

					},"X3DGroupingNode"),


	GeoMetadata=> new VRML::NodeType("GeoMetadata", {
			data => [MFNode,[],inputOutput],
			summary => [MFString,[],inputOutput],
			url => [MFString,[],inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DChildNode"),

	GeoPositionInterpolator=> new VRML::NodeType("GeoPositionInterpolator", {
			set_fraction => [SFFloat,undef,inputOnly],
			key => [MFFloat,[],inputOutput],
			keyValue => [MFVec3d,[],inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			geovalue_changed => [SFVec3d,[0,0,0],outputOnly],
			value_changed => [SFVec3f,[0,0,0],outputOnly],
			geoOrigin => [SFNode, NULL, initializeOnly],
			geoSystem => [MFString,["GD","WE"],initializeOnly],
			__geoSystem => [MFInt32,[],initializeOnly],
			__movedValue => [MFVec3d, [], inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__oldKeyPtr => [SFNode, NULL, outputOnly],
			__oldKeyValuePtr => [SFNode, NULL, outputOnly],
					},"X3DInterpolatorNode"),


	GeoProximitySensor => new VRML::NodeType("ProximitySensor", {
			enabled => [SFBool, TRUE, inputOutput],
			geoCenter => [SFVec3d, [0, 0, 0], inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			size => [SFVec3f, [0, 0, 0], inputOutput],
			centerOfRotation_changed =>[SFVec3f, [0,0,0], outputOnly],
			enterTime => [SFTime, -1, outputOnly],
			exitTime => [SFTime, -1, outputOnly],
			geoCoord_changed => [SFVec3d,[0,0,0],outputOnly],
			isActive => [SFBool, FALSE, outputOnly],
			orientation_changed => [SFRotation, [0, 0, 1, 0], outputOnly],
			position_changed => [SFVec3f, [0, 0, 0], outputOnly],
			geoOrigin => [SFNode, NULL, initializeOnly],
			geoSystem => [MFString,["GD","WE"],initializeOnly],


			# These fields are used for the info.
			__hit => [SFInt32, 0, inputOutput],
			__t1 => [SFVec3f, [10000000, 0, 0], inputOutput],
			__t2 => [SFRotation, [0, 1, 0, 0], inputOutput],

			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly],
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput],
			__localOrient => [SFVec4d, [0, 0, 1, 0], inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__oldEnabled => [SFBool, TRUE, inputOutput],
			__oldGeoCenter => [SFVec3d, [0, 0, 0], inputOutput],
			__oldSize => [SFVec3f, [0, 0, 0], inputOutput],
					   },"X3DEnvironmentalSensorNode"),

	GeoTouchSensor=> new VRML::NodeType("GeoTouchSensor", {
			description => [SFString, "", inputOutput], # see note top of file
			enabled => [SFBool, FALSE,inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			hitNormal_changed => [SFVec3f, [0, 0, 0], outputOnly],
			hitPoint_changed => [SFVec3f, [0, 0, 0], outputOnly],
			hitTexCoord_changed => [SFVec2f, [0, 0], outputOnly],
			hitGeoCoord_changed => [SFVec3d, [0, 0, 0] ,outputOnly],
			isActive => [SFBool, FALSE, outputOnly],
			isOver => [SFBool, FALSE, outputOnly],
			touchTime => [SFTime, -1, outputOnly],
			geoOrigin => [SFNode, NULL, initializeOnly],
			geoSystem => [MFString,["GD","WE"],initializeOnly],
			__geoSystem => [MFInt32,[],initializeOnly],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			_oldhitNormal => [SFVec3f, [0, 0, 0], outputOnly], 	# send event only if changed
			_oldhitPoint => [SFVec3f, [0, 0, 0], outputOnly], 	# send event only if changed
			_oldhitTexCoord => [SFVec2f, [0, 0], outputOnly], 	# send event only if changed
			__oldEnabled => [SFBool, TRUE, inputOutput],
					},"X3DPointingDeviceSensorNode"),


	GeoTransform => new VRML::NodeType ("GeoTransform", {
			addChildren => [MFNode, undef, inputOnly],
			removeChildren => [MFNode, undef, inputOnly],
			children => [MFNode, [], inputOutput],
			geoCenter => [SFVec3d, [0, 0, 0], inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			rotation => [SFRotation, [0, 0, 1, 0], inputOutput],
			scale => [SFVec3f, [1, 1, 1], inputOutput],
			scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput],
			translation => [SFVec3f, [0, 0, 0], inputOutput],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
			geoOrigin => [SFNode, NULL, initializeOnly],
			geoSystem => [MFString,["GD","WE"],initializeOnly],

			# fields for reducing redundant calls
			__do_center => [SFInt32, 0, initializeOnly],
			__do_trans => [SFInt32, 0, initializeOnly],
			__do_rotation => [SFInt32, 0, initializeOnly],
			__do_scaleO => [SFInt32, 0, initializeOnly],
			__do_scale => [SFInt32, 0, initializeOnly],

			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly],
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput],
			__localOrient => [SFVec4d, [0, 0, 1, 0], inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__oldGeoCenter => [SFVec3d, [0, 0, 0], inputOutput],
			__oldChildren => [MFNode, [], inputOutput],
					},"X3DGroupingNode"),

	GeoViewpoint => new VRML::NodeType("GeoViewpoint", {
			set_bind => [SFBool, undef, inputOnly],
			set_orientation => [SFRotation, [IO_FLOAT, IO_FLOAT, IO_FLOAT, IO_FLOAT], inputOnly],
			set_position => [SFVec3d, [IO_FLOAT, IO_FLOAT, IO_FLOAT], inputOnly],
			description => [SFString, "", inputOutput],
			fieldOfView => [SFFloat, 0.785398, inputOutput],
			headlight => [SFBool, TRUE, inputOutput],
			jump => [SFBool, TRUE, inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			navType => [MFString, ["EXAMINE","ANY"],inputOutput],
			bindTime => [SFTime, -1, outputOnly],
			isBound => [SFBool, FALSE, outputOnly],

			geoOrigin => [SFNode, NULL, initializeOnly],
			geoSystem => [MFString,["GD","WE"],initializeOnly],
			orientation => [SFRotation, [0, 0, 1, 0], inputOutput], # see note top of file
			position => [SFVec3d,[0, 0, 100000], inputOutput],
			speedFactor => [SFFloat,1.0,initializeOnly],
			__BGNumber => [SFInt32,-1,initializeOnly], # for ordering backgrounds for binding

			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly],
			__movedPosition => [SFVec3d, [0, 0, 0], inputOutput],
			__movedOrientation => [SFRotation, [0, 0, 1, 0], initializeOnly],

			__oldSFString => [SFString, "", inputOutput], #the description field
			__oldFieldOfView => [SFFloat, 0.785398, inputOutput],
			__oldHeadlight => [SFBool, TRUE, inputOutput],
			__oldJump => [SFBool, TRUE, inputOutput],
			__oldMFString => [MFString, [],inputOutput], # the navType
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro

					
					   },"X3DBindableNode"),

	GeoOrigin => new VRML::NodeType("GeoOrigin", {
			geoCoords => [SFVec3d, [0, 0, 0], inputOutput],
			geoSystem => [MFString,["GD","WE"],initializeOnly],
			metadata => [SFNode, NULL, inputOutput],
			rotateYUp => [SFBool, TRUE,initializeOnly],

			# these are now static in CFuncs/GeoVRML.c
			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly],
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput],
			__oldgeoCoords => [SFVec3d, [0, 0, 0], inputOutput],
			__oldMFString => [MFString, [],inputOutput], # the navType
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			
					},"X3DChildNode"),

	GeoLocation => new VRML::NodeType("GeoLocation", {
			addChildren => [MFNode, undef, inputOnly],
			removeChildren => [MFNode, undef, inputOnly],
			children => [MFNode, [], inputOutput],
			geoCoords => [SFVec3d, [0, 0, 0], inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			geoOrigin => [SFNode, NULL, initializeOnly],
			geoSystem => [MFString,["GD","WE"],initializeOnly],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],

			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly],
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput],
			__localOrient => [SFVec4d, [0, 0, 1, 0], inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__oldgeoCoords => [SFVec3d, [0, 0, 0], inputOutput],
			__oldChildren => [MFNode, [], inputOutput],
					},"X3DGroupingNode"),


	###################################################################################

	#		H-Anim Component

	###################################################################################

	HAnimDisplacer => new VRML::NodeType("HAnimDisplacer", {
			coordIndex => [MFInt32, [], inputOnly], # see note top of file
			displacements => [MFVec3f, [], inputOutput],
			name => [SFString, "", inputOutput],
			weight => [SFFloat, 0.0, inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DGeometricPropertyNode"),

	HAnimHumanoid => new VRML::NodeType("HAnimHumanoid", {
			center => [SFVec3f, [0, 0, 0], inputOutput],
			info => [MFString, [],initializeOnly], # see note top of file
			joints => [MFNode,[],inputOutput],
			name => [SFString, "", inputOutput],
			rotation => [SFRotation,[0,0,1,0], inputOutput],
			scale => [SFVec3f,[1,1,1],inputOutput],
			scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput],
			segments => [MFNode,[],inputOutput],
			sites => [MFNode,[],inputOutput],
			skeleton => [MFNode,[],inputOutput],
			skin => [MFNode,[],inputOutput],
			skinCoord => [SFNode, NULL, inputOutput],
			skinNormal => [SFNode, NULL, inputOutput],
			translation => [SFVec3f, [0, 0, 0], inputOutput],
			version => [SFString,"",inputOutput],
			viewpoints => [MFNode,[],inputOutput],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DChildNode"),

	HAnimJoint => new VRML::NodeType("HAnimJoint", {

			addChildren => [MFNode, undef, inputOnly],
			removeChildren => [MFNode, undef, inputOnly],
			children => [MFNode, [], inputOutput],
			center => [SFVec3f, [0, 0, 0], inputOutput],
			children => [MFNode, [], inputOutput],
			rotation => [SFRotation, [0, 0, 1, 0], inputOutput],
			scale => [SFVec3f, [1, 1, 1], inputOutput],
			scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput],
			translation => [SFVec3f, [0, 0, 0], inputOutput],
			displacers => [MFNode, [], inputOutput],
			limitOrientation => [SFRotation, [0, 0, 1, 0], inputOutput],
			llimit => [MFFloat,[],inputOutput],
			name => [SFString, "", inputOutput],
			skinCoordIndex => [MFInt32,[],inputOutput],
			skinCoordWeight => [MFFloat,[],inputOutput],
			stiffness => [MFFloat,[0,0,0],inputOutput],
			ulimit => [MFFloat,[],inputOutput],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro

			 # fields for reducing redundant calls
			 __do_center => [SFInt32, 0, initializeOnly],
			 __do_trans => [SFInt32, 0, initializeOnly],
			 __do_rotation => [SFInt32, 0, initializeOnly],
			 __do_scaleO => [SFInt32, 0, initializeOnly],
			 __do_scale => [SFInt32, 0, initializeOnly],
					},"X3DChildNode"),

	HAnimSegment => new VRML::NodeType("HAnimSegment", {
			addChildren => [MFNode, undef, inputOnly],
			removeChildren => [MFNode, undef, inputOnly],
			children => [MFNode, [], inputOutput],
			name => [SFString, "", inputOutput],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
			centerOfMass => [SFVec3f, [0, 0, 0], inputOutput],
			coord => [SFNode, NULL, inputOutput],
			displacers => [MFNode,[],inputOutput],
			mass => [SFFloat, 0, inputOutput],
			momentsOfInertia =>[MFFloat, [0, 0, 0, 0, 0, 0, 0, 0, 0],inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DChildNode"),



	HAnimSite => new VRML::NodeType("HAnimSite", {
			addChildren => [MFNode, undef, inputOnly],
			removeChildren => [MFNode, undef, inputOnly],
			children => [MFNode, [], inputOutput],
			name => [SFString, "", inputOutput],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
			center => [SFVec3f, [0, 0, 0], inputOutput],
			children => [MFNode, [], inputOutput],
			rotation => [SFRotation, [0, 0, 1, 0], inputOutput],
			scale => [SFVec3f, [1, 1, 1], inputOutput],
			scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput],
			translation => [SFVec3f, [0, 0, 0], inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro

			 # fields for reducing redundant calls
			 __do_center => [SFInt32, 0, initializeOnly],
			 __do_trans => [SFInt32, 0, initializeOnly],
			 __do_rotation => [SFInt32, 0, initializeOnly],
			 __do_scaleO => [SFInt32, 0, initializeOnly],
			 __do_scale => [SFInt32, 0, initializeOnly],
					},"X3DGroupingNode"),


	###################################################################################

	#		NURBS Component

	###################################################################################

	Contour2D => new VRML::NodeType("Contour2D", {
			addChildren => [MFNode, undef, inputOnly],
			removeChildren => [MFNode, undef, inputOnly],
			children => [MFNode, [], inputOutput],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DParametricGeometryNode"),


	ContourPolyLine2D =>
	new VRML::NodeType("ContourPolyline2D",
					{
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DParametricGeometryNode"
					),

	NurbsCurve =>
	new VRML::NodeType("NurbsCurve",
					{
			controlPoint =>[MFVec3f,[],inputOutput],
			weight => [MFFloat,[],inputOutput],
			tessellation => [SFInt32,0,inputOutput],
			knot => [MFFloat,[],initializeOnly],
			order => [SFInt32,3,inputOutput], # see note top of file
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DParametricGeometryNode"
					),

	NurbsCurve2D =>
	new VRML::NodeType("NurbsCurve2D",
					{
			controlPoint =>[MFVec2f,[],inputOutput],
			weight => [MFFloat,[],inputOutput],
			tessellation => [SFInt32,0,inputOutput],
			knot => [MFFloat,[],initializeOnly],
			order => [SFInt32,3,inputOutput], # see note top of file
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DParametricGeometryNode"
					),

	NurbsGroup =>
	new VRML::NodeType("NurbsGroup",
					{
			addChildren => [MFNode, undef, inputOnly],
			removeChildren => [MFNode, undef, inputOnly],
			children => [MFNode, [], inputOutput],
			tessellationScale => [SFFloat,1.0,inputOutput],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DGroupingNode"
					),

	NurbsPositionInterpolator =>
	new VRML::NodeType("NurbsPositionInterpolator",
					{

			set_fraction => [SFFloat,undef,inputOnly],
			dimension => [SFInt32,0,inputOutput],
			keyValue => [MFVec3f,[],inputOutput],
			keyWeight => [MFFloat,[],inputOutput],
			knot => [MFFloat,[],initializeOnly],# see note top of file
			order => [SFInt32,0,inputOutput],
			value_changed => [SFVec3f,[0,0,0],outputOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DInterpolatorNode"
					),

	NurbsSurface =>
	new VRML::NodeType("NurbsSurface",
					{
			controlPoint =>[MFVec3f,[],inputOutput],
			texCoord => [SFNode, NULL, inputOutput],
			uTessellation => [SFInt32,0,inputOutput],
			vTessellation => [SFInt32,0,inputOutput],
			weight => [MFFloat,[],inputOutput],
			ccw => [SFBool, FALSE,initializeOnly],

			knot => [MFFloat,[],initializeOnly],
			order => [SFInt32,3,inputOutput], # see note top of file
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DParametricGeometryNode"
					),
	NurbsTextureSurface =>
	new VRML::NodeType("NurbsTextureSurface",
					{
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DParametricGeometryNode"
					),

	NurbsTrimmedSurface =>
	new VRML::NodeType("NurbsTrimmedSurface",
					{
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
					},"X3DParametricGeometryNode"
					),


	###################################################################################

	#		Scripting Component

	###################################################################################
	Script =>
	new VRML::NodeType("Script",
					   {
			url => [MFString, [], inputOutput],
			directOutput => [SFBool, FALSE, initializeOnly],
			mustEvaluate => [SFBool, FALSE, initializeOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			 __scriptObj => [FreeWRLPTR, 0, initializeOnly],
			 _X3DScript => [SFInt32, -1, initializeOnly],
			__parenturl =>[SFString,"",initializeOnly],
					   },"X3DScriptNode"
					  ),

	###################################################################################

	#		EventUtilities Component

	###################################################################################

	BooleanFilter => 
	new VRML::NodeType("BooleanFilter", {
			set_boolean =>[SFBool,undef,inputOnly],
			inputFalse => [SFBool, FALSE, outputOnly],
			inputNegate => [SFBool, FALSE, outputOnly],
			inputTrue => [SFBool, TRUE, outputOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DChildNode"),


	BooleanSequencer => 
	new VRML::NodeType("BooleanSequencer", {
			next =>[SFBool,undef,inputOnly],
			previous =>[SFBool,undef,inputOnly],
			set_fraction =>[SFFloat,undef,inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFBool, [], inputOutput],
			value_changed => [SFBool, FALSE, outputOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DSequencerNode"),


	BooleanToggle => 
	new VRML::NodeType("BooleanToggle", {
			set_boolean =>[SFBool,undef,inputOnly],
			toggle => [SFBool, FALSE, outputOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DChildNode"),


	BooleanTrigger => 
	new VRML::NodeType("BooleanTrigger", {
			set_triggerTime => [SFTime,undef ,inputOnly],
			triggerTrue => [SFBool, FALSE, outputOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DTriggerNode"),


	IntegerSequencer => 
	new VRML::NodeType("IntegerSequencer", {
			next =>[SFBool,undef,inputOnly],
			previous =>[SFBool,undef,inputOnly],
			set_fraction =>[SFFloat,undef,inputOnly],
			key => [MFFloat, [], inputOutput],
			keyValue => [MFInt32, [], inputOutput],
			value_changed => [SFInt32, 0, outputOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DSequencerNode"),

	IntegerTrigger => 
	new VRML::NodeType("IntegerTrigger", {
			set_triggerTime => [SFTime,undef ,inputOnly],
			integerKey => [SFInt32, 0, inputOutput],
			triggerValue => [SFInt32, 0, outputOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DTriggerNode"),

	TimeTrigger => 
	new VRML::NodeType("TimeTrigger", {
			set_boolean =>[SFBool,undef,inputOnly],
			triggerTime => [SFTime, 0, outputOnly],
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DTriggerNode"),


	###################################################################################

	#		EventUtilities Component

	###################################################################################

	ComposedShader => new VRML::NodeType("ComposedShader", {
			activate =>[SFBool,undef,inputOnly],
			parts => [MFNode,[],inputOutput],
			isSelected => [SFBool, TRUE,outputOnly],
			isValid => [SFBool, TRUE,outputOnly],
			language => [SFString, "", initializeOnly],
			__shaderIDS => [MFNode, [], initializeOnly], 
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	},"X3DShaderNode"),

	FloatVertexAttribute => new VRML::NodeType("FloatVertexAttribute", {
			value => [MFFloat,[],inputOutput],
			name => [SFString,"",inputOutput], # see note top of file
			numComponents => [SFInt32, 4, initializeOnly], # 1...4 valid values
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	}, "X3DVertexAttributeNode"),

	Matrix3VertexAttribute => new VRML::NodeType("Matrix3VertexAttribute", {
			value => [MFMatrix3f,[],inputOutput],
			name => [SFString,"",inputOutput], # see note top of file
			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	}, "X3DVertexAttributeNode"),

	Matrix4VertexAttribute => new VRML::NodeType("Matrix4VertexAttribute", {
			metadata => [SFNode, NULL, inputOutput],
			value => [MFMatrix4f,[],inputOutput],
			name => [SFString,"",inputOutput], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	}, "X3DVertexAttributeNode"),

	PackagedShader => new VRML::NodeType("PackagedShader", {
			activate =>[SFBool,undef,inputOnly],
			metadata => [SFNode, NULL, inputOutput],
			url => [MFString, [], inputOutput],
			isSelected => [SFBool, TRUE,outputOnly],
			isValid => [SFBool, TRUE,outputOnly],
			language => [SFString,"",initializeOnly],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	}, "X3DProgrammableShaderObject"),

	ProgramShader => new VRML::NodeType("ProgramShader", {
			activate =>[SFBool,undef,inputOnly],
			metadata => [SFNode, NULL, inputOutput],
			programs => [MFNode, [], inputOutput],
			isSelected => [SFBool, TRUE,outputOnly],
			isValid => [SFBool, TRUE,outputOnly],
			language => [SFString,"",initializeOnly],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__shaderIDS => [MFNode, [], initializeOnly], 
	}, "X3DProgrammableShaderObject"),

	ShaderPart => new VRML::NodeType("ShaderPart", {
			metadata => [SFNode, NULL, inputOutput],
			url => [MFString, [], inputOutput],
			type => [SFString,"VERTEX",inputOutput], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__parenturl =>[SFString,"",initializeOnly],
	}, "X3DUrlObject"),

	ShaderProgram => new VRML::NodeType("ShaderProgram", {
			metadata => [SFNode, NULL, inputOutput],
			url => [MFString, [], inputOutput],
			type => [SFString,"",inputOutput], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
			__parenturl =>[SFString,"",initializeOnly],
	}, "X3DUrlObject"),


	###################################################################################

	# Metadata nodes ...

	###################################################################################


	MetadataInteger => new VRML::NodeType("MetadataInteger", {
			metadata => [SFNode, NULL, inputOutput],
			name => [SFString,"",inputOutput], # see note top of file
			reference => [SFString,"",initializeOnly],
			value => [MFInt32,[],inputOutput],  # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	}, "X3DChildNode"),
	
	MetadataDouble => new VRML::NodeType("MetadataDouble", {
			metadata => [SFNode, NULL, inputOutput],
			name => [SFString,"",inputOutput],# see note top of file:
			reference => [SFString,"",initializeOnly],
			value => [MFDouble,[],inputOutput], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	}, "X3DChildNode"),
	
	MetadataFloat => new VRML::NodeType("MetadataFloat", {
			metadata => [SFNode, NULL, inputOutput],
			name => [SFString,"",inputOutput], # see note top of file
			reference => [SFString,"",initializeOnly],
			value => [MFFloat,[],inputOutput], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	}, "X3DChildNode"),
	
	MetadataString => new VRML::NodeType("MetadataString", {
			metadata => [SFNode, NULL, inputOutput],
			name => [SFString,"",inputOutput],
			reference => [SFString,"",initializeOnly],
			value => [MFString,[],inputOutput], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	}, "X3DChildNode"),
	
	MetadataSet => new VRML::NodeType("MetadataSet", {
			metadata => [SFNode, NULL, inputOutput],
			name => [SFString,"",inputOutput], # see note top of file
			reference => [SFString,"",initializeOnly],
			value => [MFNode,[],inputOutput], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro
	}, "X3DChildNode"),


	#used mainly for PROTO invocation parameters 
	MetadataSFFloat => new VRML::NodeType("MetadataSFFloat", { 
			value => [SFFloat,0.0,inputOutput], 
			valueChanged=>[SFFloat,0.0,outputOnly], 
			setValue =>[SFFloat,0.0,inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFFloat => new VRML::NodeType("MetadataMFFloat", { 
			value => [MFFloat,[],inputOutput], 
			valueChanged=>[MFFloat,[],outputOnly], 
			setValue =>[MFFloat,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFRotation => new VRML::NodeType("MetadataSFRotation", { 
			value => [SFRotation,[0,0,0,0],inputOutput], 
			valueChanged=>[SFRotation,[0,0,0,0],outputOnly], 
			setValue =>[SFRotation,[0,0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFRotation => new VRML::NodeType("MetadataMFRotation", { 
			value => [MFRotation,[],inputOutput], 
			valueChanged=>[MFRotation,[],outputOnly], 
			setValue =>[MFRotation,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec3f => new VRML::NodeType("MetadataSFVec3f", { 
			value => [SFVec3f,[0,0,0],inputOutput], 
			valueChanged=>[SFVec3f,[0,0,0],outputOnly], 
			setValue =>[SFVec3f,[0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec3f => new VRML::NodeType("MetadataMFVec3f", { 
			value => [MFVec3f,[],inputOutput], 
			valueChanged=>[MFVec3f,[],outputOnly], 
			setValue =>[MFVec3f,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFBool => new VRML::NodeType("MetadataSFBool", { 
			value => [SFBool,FALSE,inputOutput], 
			valueChanged=>[SFBool,FALSE,outputOnly], 
			setValue =>[SFBool,FALSE,inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFBool => new VRML::NodeType("MetadataMFBool", { 
			value => [MFBool,[],inputOutput], 
			valueChanged=>[MFBool,[],outputOnly], 
			setValue =>[MFBool,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFInt32 => new VRML::NodeType("MetadataSFInt32", { 
			value => [SFInt32,0,inputOutput], 
			valueChanged=>[SFInt32,0,outputOnly], 
			setValue =>[SFInt32,0,inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFInt32 => new VRML::NodeType("MetadataMFInt32", { 
			value => [MFInt32,[],inputOutput], 
			valueChanged=>[MFInt32,[],outputOnly], 
			setValue =>[MFInt32,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFNode => new VRML::NodeType("MetadataSFNode", { 
			value => [SFNode,0,inputOutput], 
			valueChanged=>[SFNode,0,outputOnly], 
			setValue =>[SFNode,0,inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFNode => new VRML::NodeType("MetadataMFNode", { 
			value => [MFNode,[],inputOutput], 
			valueChanged=>[MFNode,[],outputOnly], 
			setValue =>[MFNode,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFColor => new VRML::NodeType("MetadataSFColor", { 
			value => [SFColor,[0,0,0],inputOutput], 
			valueChanged=>[SFColor,[0,0,0],outputOnly], 
			setValue =>[SFColor,[0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFColor => new VRML::NodeType("MetadataMFColor", { 
			value => [MFColor,[],inputOutput], 
			valueChanged=>[MFColor,[],outputOnly], 
			setValue =>[MFColor,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFColorRGBA => new VRML::NodeType("MetadataSFColorRGBA", { 
			value => [SFColorRGBA,[0,0,0,0],inputOutput], 
			valueChanged=>[SFColorRGBA,[0,0,0,0],outputOnly], 
			setValue =>[SFColorRGBA,[0,0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFColorRGBA => new VRML::NodeType("MetadataMFColorRGBA", { 
			value => [MFColorRGBA,[],inputOutput], 
			valueChanged=>[MFColorRGBA,[],outputOnly], 
			setValue =>[MFColorRGBA,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFTime => new VRML::NodeType("MetadataSFTime", { 
			value => [SFTime,0,inputOutput], 
			valueChanged=>[SFTime,0,outputOnly], 
			setValue =>[SFTime,0,inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFTime => new VRML::NodeType("MetadataMFTime", { 
			value => [MFTime,[],inputOutput], 
			valueChanged=>[MFTime,[],outputOnly], 
			setValue =>[MFTime,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFString => new VRML::NodeType("MetadataSFString", { 
			value => [SFString,"",inputOutput], 
			valueChanged=>[SFString,"",outputOnly], 
			setValue =>[SFString,"",inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFString => new VRML::NodeType("MetadataMFString", { 
			value => [MFString,[],inputOutput], 
			valueChanged=>[MFString,[],outputOnly], 
			setValue =>[MFString,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec2f => new VRML::NodeType("MetadataSFVec2f", { 
			value => [SFVec2f,[0,0],inputOutput], 
			valueChanged=>[SFVec2f,[0,0],outputOnly], 
			setValue =>[SFVec2f,[0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec2f => new VRML::NodeType("MetadataMFVec2f", { 
			value => [MFVec2f,[],inputOutput], 
			valueChanged=>[MFVec2f,[],outputOnly], 
			setValue =>[MFVec2f,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFImage => new VRML::NodeType("MetadataSFImage", { 
			value => [SFImage,[0,0,0],inputOutput], 
			valueChanged=>[SFImage,[0,0,0],outputOnly], 
			setValue =>[SFImage,[0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec3d => new VRML::NodeType("MetadataSFVec3d", { 
			value => [SFVec3d,[0,0,0],inputOutput], 
			valueChanged=>[SFVec3d,[0,0,0],outputOnly], 
			setValue =>[SFVec3d,[0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec3d => new VRML::NodeType("MetadataMFVec3d", { 
			value => [MFVec3d,[],inputOutput], 
			valueChanged=>[MFVec3d,[],outputOnly], 
			setValue =>[MFVec3d,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFDouble => new VRML::NodeType("MetadataSFDouble", { 
			value => [SFDouble,0,inputOutput], 
			valueChanged=>[SFDouble,0,outputOnly], 
			setValue =>[SFDouble,0,inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFDouble => new VRML::NodeType("MetadataMFDouble", { 
			value => [MFDouble,[],inputOutput], 
			valueChanged=>[MFDouble,[],outputOnly], 
			setValue =>[MFDouble,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFMatrix3f => new VRML::NodeType("MetadataSFMatrix3f", { 
			value => [SFMatrix3f,[0,0,0,0,0,0,0,0,0],inputOutput], 
			valueChanged=>[SFMatrix3f,[0,0,0,0,0,0,0,0,0],outputOnly], 
			setValue =>[SFMatrix3f,[0,0,0,0,0,0,0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFMatrix3f => new VRML::NodeType("MetadataMFMatrix3f", { 
			value => [MFMatrix3f,[],inputOutput], 
			valueChanged=>[MFMatrix3f,[],outputOnly], 
			setValue =>[MFMatrix3f,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFMatrix3d => new VRML::NodeType("MetadataSFMatrix3d", { 
			value => [SFMatrix3d,[0,0,0,0,0,0,0,0,0],inputOutput], 
			valueChanged=>[SFMatrix3d,[0,0,0,0,0,0,0,0,0],outputOnly], 
			setValue =>[SFMatrix3d,[0,0,0,0,0,0,0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFMatrix3d => new VRML::NodeType("MetadataMFMatrix3d", { 
			value => [MFMatrix3d,[],inputOutput], 
			valueChanged=>[MFMatrix3d,[],outputOnly], 
			setValue =>[MFMatrix3d,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFMatrix4f => new VRML::NodeType("MetadataSFMatrix4f", { 
			value => [SFMatrix4f,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],inputOutput], 
			valueChanged=>[SFMatrix4f,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],outputOnly], 
			setValue =>[SFMatrix4f,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFMatrix4f => new VRML::NodeType("MetadataMFMatrix4f", { 
			value => [MFMatrix4f,[],inputOutput], 
			valueChanged=>[MFMatrix4f,[],outputOnly], 
			setValue =>[MFMatrix4f,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFMatrix4d => new VRML::NodeType("MetadataSFMatrix4d", { 
			value => [SFMatrix4d,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],inputOutput], 
			valueChanged=>[SFMatrix4d,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],outputOnly], 
			setValue =>[SFMatrix4d,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFMatrix4d => new VRML::NodeType("MetadataMFMatrix4d", { 
			value => [MFMatrix4d,[],inputOutput], 
			valueChanged=>[MFMatrix4d,[],outputOnly], 
			setValue =>[MFMatrix4d,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec2d => new VRML::NodeType("MetadataSFVec2d", { 
			value => [SFVec2d,[0,0],inputOutput], 
			valueChanged=>[SFVec2d,[0,0],outputOnly], 
			setValue =>[SFVec2d,[0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec2d => new VRML::NodeType("MetadataMFVec2d", { 
			value => [MFVec2d,[],inputOutput], 
			valueChanged=>[MFVec2d,[],outputOnly], 
			setValue =>[MFVec2d,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec4f => new VRML::NodeType("MetadataSFVec4f", { 
			value => [SFVec4f,[0,0,0,0],inputOutput], 
			valueChanged=>[SFVec4f,[0,0,0,0],outputOnly], 
			setValue =>[SFVec4f,[0,0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec4f => new VRML::NodeType("MetadataMFVec4f", { 
			value => [MFVec4f,[],inputOutput], 
			valueChanged=>[MFVec4f,[],outputOnly], 
			setValue =>[MFVec4f,[],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec4d => new VRML::NodeType("MetadataSFVec4d", { 
			value => [SFVec4d,[0,0,0,0],inputOutput], 
			valueChanged=>[SFVec4d,[0,0,0,0],outputOnly], 
			setValue =>[SFVec4d,[0,0,0,0],inputOnly], 
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec4d => new VRML::NodeType("MetadataMFVec4d", { 
			value => [MFVec4d,[],inputOutput], 
			valueChanged=>[MFVec4d,[],outputOnly], 
			setValue =>[MFVec4d,[],inputOnly], 
	}, "X3DChildNode"),


	###################################################################################

	# testing...

	###################################################################################

	MidiControl =>
	new VRML::NodeType("MidiControl",
					{
			deviceName => [SFString,"",inputOutput],	# "Subtractor 1"
			channel => [SFInt32,-1,inputOutput],		# channel in range 0-16, on MIDI bus
			controller => [SFString,"",inputOutput],	# "Osc1 Wave"
			_deviceNameIndex => [SFInt32, -99, initializeOnly],	#  name in name table index
			_controllerIndex => [SFInt32, -99, initializeOnly],		#  name in name table index


			# encoded bus,channel,controller
			_bus => [SFInt32,-99,initializeOnly],			# internal for efficiency
			_channel => [SFInt32,-99,initializeOnly],			# internal for efficiency
			_controller => [SFInt32,-99,initializeOnly],		# internal for efficiency

			deviceMinVal => [SFInt32, 0, initializeOnly],		# what the device sets
			deviceMaxVal => [SFInt32, 0, initializeOnly],		# what the device sets

			velocity => [SFInt32, 100, inputOutput],	# velocity field for buttonPress
									# controller types.
			_vel => [SFInt32, 100, initializeOnly],			# internal copy of velocity
			_sentVel => [SFInt32, 100, initializeOnly],		# send this velocity - if <0, noteOff

			minVal => [SFInt32, 0, inputOutput],		# used to scale floats, and 
			maxVal => [SFInt32, 10000, inputOutput],	# bounds check ints. The resulting
									# value will be <= maxVal <= deviceMaxVal
									# and >=minVal >= deviceMinVal

			intValue => [SFInt32, 0, inputOutput],		# integer value for i/o
			_oldintValue => [SFInt32, 0, initializeOnly],		# old integer value for i/o
			floatValue => [SFFloat, 0, inputOutput],	# float value for i/o
			useIntValue => [SFBool, TRUE, inputOutput],	# which value to use for input

			highResolution => [SFBool, TRUE, inputOutput],	# high resolution controller
			controllerType => [SFString, "Slider", inputOutput],	# "Slider" "ButtonPress"
			_intControllerType => [SFInt32,999, initializeOnly], 	# use ReWire definitions
			controllerPresent => [SFBool, FALSE, inputOutput],	# TRUE when ReWire is working

			buttonPress => [SFBool,FALSE,inputOutput],	# is the key pressed when in "ButtonPress" mode?"
			_butPr => [SFBool,FALSE,inputOutput],		# used to determine toggle state for buttonPress

			autoButtonPress => [SFBool,TRUE,inputOutput],# send a NoteOn when the int/float 
									# value changes. if False, send only
									# when buttonPressed happens.
			pressLength => [SFFloat, 0.05, inputOutput],	# time before noteOff in AutoButtonPress mode.
			pressTime => [SFTime, 0, inputOutput],		# when the press went in

			metadata => [SFNode, NULL, inputOutput],
			__oldmetadata => [SFNode, 0, inputOutput], # see code for event macro

					}, "X3DNetworkSensorNode"
					),
); 


1;

