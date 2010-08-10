#
# $Id: VRMLNodes.pm,v 1.50 2010/08/10 21:15:59 crc_canada Exp $
#
# Copyright (C) 1998 Tuomas J. Lukka 1999 John Stewart CRC Canada.
# DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
# See the GNU Library General Public License (file COPYING in the distribution)
# for conditions of use and redistribution.

package VRML::NodeType;


# see note top of file - the VRML Parser REQUIRES for routing that 
# each field name exists in only one table- eg, "inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)", "initializeOnly, (SPEC_VRML | SPEC_X3D30)",
# etc. So, in order to do this, we make sure that each field name follows this, even
# though we may, for instance, change "value" to an "inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)" when the spec
# says it is an "initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)". This has little if any effect on parsing.

# SPEC_VRML tag verified against http://web3d.org/x3d/specifications/vrml/ISO-IEC-14772-VRML97/part1/nodesRef.html

########################################################################

{
   sub new {
		my($type, $name, $fields, $X3DNodeType) = @_;
		if ($X3DNodeType eq "") {
			print "NodeType, X3DNodeType blank for $name\n";
			$X3DNodeType = "unknown";
		}
		# DEBUG: print "Node: $X3DNodeType\n";
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

			$t = $fields->{$_}[3];
			if (!defined $t) {
				die("Missing field or event type $type X3DNodeType $X3DNodeType for $_ in $name");
			}
			$this->{SpecLevel}{$_} = $t;

		}
		return $this;
    }
}

%VRML::Nodes = (




	###################################################################################

	# chapter 7: 		Core Component

	###################################################################################


	WorldInfo => new VRML::NodeType("WorldInfo", {
		info => [MFString, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		title => [SFString, "", initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DChildNode"),

	MetadataInteger => new VRML::NodeType("MetadataInteger", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString,"",inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			reference => [SFString,"",initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value => [MFInt32,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],  # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	}, "X3DChildNode"),
	
	MetadataDouble => new VRML::NodeType("MetadataDouble", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString,"",inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],# see note top of file:
			reference => [SFString,"",initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value => [MFDouble,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	}, "X3DChildNode"),
	
	MetadataFloat => new VRML::NodeType("MetadataFloat", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString,"",inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			reference => [SFString,"",initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value => [MFFloat,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	}, "X3DChildNode"),
	
	MetadataString => new VRML::NodeType("MetadataString", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString,"",inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			reference => [SFString,"",initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value => [MFString,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	}, "X3DChildNode"),
	
	MetadataSet => new VRML::NodeType("MetadataSet", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString,"",inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			reference => [SFString,"",initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value => [MFNode,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	}, "X3DChildNode"),

	###################################################################################

	# Chapter 8:		Time Component

	###################################################################################

	TimeSensor => new VRML::NodeType("TimeSensor", {
		cycleInterval => [SFTime, 1, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		enabled => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		loop => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		pauseTime => [SFTime,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		resumeTime => [SFTime,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		startTime => [SFTime, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		stopTime => [SFTime, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		cycleTime => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		elapsedTime => [SFTime,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fraction_changed => [SFFloat, 0.0, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isPaused => [SFTime,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		time => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		# time that we were initialized at
		__inittime => [SFTime, 0, initializeOnly, 0],
		# cycleTimer flag.
		__ctflag =>[SFTime, 10, inputOutput, 0],
		__oldEnabled => [SFBool, TRUE, inputOutput, 0],
	},"X3DSensorNode"),

	###################################################################################

	# Chapter 9:		Networking Component

	###################################################################################

	Anchor => new VRML::NodeType("Anchor", {
		addChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		removeChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		children => [MFNode, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		description => [SFString, "", inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		parameter => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		url => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_sortedChildren => [MFNode, [], inputOutput, 0],
	},"X3DGroupingNode"),


	Inline => new VRML::NodeType("Inline", {
		load => [SFBool, TRUE,initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		url => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
                __children => [MFNode, [], inputOutput, 0],
		__loadstatus =>[SFInt32,0,initializeOnly, 0],
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
		_sortedChildren => [MFNode, [], inputOutput, 0],
		 __loadResource => [FreeWRLPTR, 0, initializeOnly, 0],
	},"X3DNetworkSensorNode"),

	LoadSensor => new VRML::NodeType("LoadSensor", {
		enabled => [SFBool, FALSE,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		timeOut  => [SFTime,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		watchList => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isActive  => [SFBool, TRUE,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isLoaded  => [SFBool, TRUE,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		loadTime  => [SFTime,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		progress  => [SFFloat,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__loading => [SFBool, TRUE,initializeOnly, 0],		# current internal status
		__finishedloading => [SFBool, TRUE,initializeOnly, 0],	# current internal status
		__StartLoadTime => [SFTime,0,outputOnly, 0], # time we started loading...
		__oldEnabled => [SFBool, TRUE, inputOutput, 0],
	},"X3DNetworkSensorNode"),


	###################################################################################

	# Chapter 10:		Grouping Component

	###################################################################################

	Group => new VRML::NodeType("Group", {
		addChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		removeChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		children => [MFNode, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		FreeWRL__protoDef => [SFInt32, INT_ID_UNDEFINED, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # tell renderer that this is a proto...
		FreeWRL_PROTOInterfaceNodes =>[MFNode, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		_sortedChildren => [MFNode, [], inputOutput, 0],
	},"X3DGroupingNode"),

	StaticGroup => new VRML::NodeType("StaticGroup", {
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__transparency => [SFInt32, -1, initializeOnly, 0], # display list for transparencies
		__solid => [SFInt32, -1, initializeOnly, 0],	 # display list for solid geoms.
		_sortedChildren => [MFNode, [], inputOutput, 0],
	},"X3DGroupingNode"),

	Switch => new VRML::NodeType("Switch", {
		addChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		removeChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		choice => [MFNode, [], inputOutput, "(SPEC_VRML)"],		# VRML nodes....
		children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],		# X3D nodes....
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		whichChoice => [SFInt32, -1, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__isX3D => [SFBool, "(inputFileVersion[0]==3)" , initializeOnly, 0], # TRUE for X3D V3.x files
	},"X3DGroupingNode"),

	Transform => new VRML::NodeType ("Transform", {
		addChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		removeChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		center => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		children => [MFNode, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		rotation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		scale => [SFVec3f, [1, 1, 1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		translation => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		# fields for reducing redundant calls
		__do_center => [SFInt32, FALSE, initializeOnly, 0],
		__do_trans => [SFInt32, FALSE, initializeOnly, 0],
		__do_rotation => [SFInt32, FALSE, initializeOnly, 0],
		__do_scaleO => [SFInt32, FALSE, initializeOnly, 0],
		__do_scale => [SFInt32, FALSE, initializeOnly, 0],
		__do_anything => [SFInt32, FALSE, initializeOnly, 0],
		_sortedChildren => [MFNode, [], inputOutput, 0],
	},"X3DGroupingNode"),
	

	###################################################################################

	# Chapter 11:		Rendering Component

	###################################################################################

	ClipPlane => new VRML::NodeType("ClipPlane", { 
		enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		plane => [SFVec4f, [0, 1, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DChildNode"),

	Color => new VRML::NodeType("Color", { 
		color => [MFColor, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DColorNode"),

	ColorRGBA => new VRML::NodeType("ColorRGBA", { 
		color => [MFColorRGBA, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DColorNode"),

	Coordinate => new VRML::NodeType("Coordinate", { 
		point => [MFVec3f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DCoordinateNode"),

	IndexedLineSet => new VRML::NodeType("IndexedLineSet", {
		set_colorIndex => [MFInt32, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_coordIndex => [MFInt32, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorIndex => [MFInt32, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coordIndex => [MFInt32, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__vertArr  =>[FreeWRLPTR,0,initializeOnly, 0],
		__vertIndx  =>[FreeWRLPTR,0,initializeOnly, 0],
		__colours  =>[FreeWRLPTR,0,initializeOnly, 0],
		__vertices  =>[FreeWRLPTR,0,initializeOnly, 0],
		__vertexCount =>[FreeWRLPTR,0,initializeOnly, 0],
		__segCount =>[SFInt32,0,initializeOnly, 0],
	},"X3DGeometryNode"),

	IndexedTriangleFanSet => new VRML::NodeType("IndexedTriangleFanSet", {
		set_index => [MFInt32, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normal => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		index => [MFInt32, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		_coordIndex => [MFInt32, [], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DGeometryNode"),

	IndexedTriangleSet => new VRML::NodeType("IndexedTriangleSet", {
		set_index => [MFInt32, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normal => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		index => [MFInt32, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		_coordIndex => [MFInt32, [], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DGeometryNode"),

	IndexedTriangleStripSet => new VRML::NodeType("IndexedTriangleStripSet", {
		set_index => [MFInt32, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normal => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		index => [MFInt32, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		_coordIndex => [MFInt32, [], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DGeometryNode"),

	LineSet => new VRML::NodeType("LineSet", {
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		vertexCount => [MFInt32,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__vertArr  =>[FreeWRLPTR,0,initializeOnly, 0],
		__vertIndx  =>[FreeWRLPTR,0,initializeOnly, 0],
		__segCount =>[SFInt32,0,initializeOnly, 0],
	},"X3DGeometryNode"),

	Normal => new VRML::NodeType("Normal", { 
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		vector => [MFVec3f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"] ,
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DNormalNode"),

	PointSet => new VRML::NodeType("PointSet", {
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DGeometryNode"),

	TriangleFanSet => new VRML::NodeType("TriangleFanSet", {
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fanCount => [MFInt32, [3], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normal => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		_coordIndex => [MFInt32, [], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DGeometryNode"),

	TriangleStripSet => new VRML::NodeType("TriangleStripSet", {
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normal => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		stripCount => [MFInt32, [], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		_coordIndex => [MFInt32, [], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro

	},"X3DGeometryNode"),

	TriangleSet => new VRML::NodeType("TriangleSet", {
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normal => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		_coordIndex => [MFInt32, [], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DGeometryNode"),


	###################################################################################

#	Chapter 12:		Shape Component

	###################################################################################

	Appearance => new VRML::NodeType ("Appearance", {
		fillProperties => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		lineProperties => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		material => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		shaders => [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texture => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		textureTransform => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DAppearanceNode"),

	FillProperties => new VRML::NodeType ("FillProperties", {
		filled => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		hatchColor => [SFColor, [1,1,1], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		hatched => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		hatchStyle => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DAppearanceChildNode"),

	LineProperties => new VRML::NodeType ("LineProperties", {
		applied => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		linetype => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		linewidthScaleFactor => [SFFloat, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DAppearanceChildNode"),

	Material => new VRML::NodeType ("Material", {
		ambientIntensity => [SFFloat, 0.2, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		diffuseColor => [SFColor, [0.8, 0.8, 0.8], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		emissiveColor => [SFColor, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		shininess => [SFFloat, 0.2, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		specularColor => [SFColor, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transparency => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_dcol => [SFVec4f, [0,0,0,0], inputOutput, 0],
		_scol => [SFVec4f, [0,0,0,0], inputOutput, 0],
		_ecol => [SFVec4f, [0,0,0,0], inputOutput, 0],
		_amb => [SFVec4f, [0,0,0,0], inputOutput, 0],
		_shin => [SFFloat, 0.2, inputOutput, 0],
	},"X3DMaterialNode"),

	Shape => new VRML::NodeType ("Shape", {
		appearance => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		geometry => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__visible =>[SFInt32,0,initializeOnly, 0], # for Occlusion tests.
		__occludeCheckCount =>[SFInt32,-1,initializeOnly, 0], # for Occlusion tests.
		__Samples =>[SFInt32,-1,initializeOnly, 0],		# Occlude samples from last pass
	},"X3DBoundedObject"),

	TwoSidedMaterial => new VRML::NodeType ("TwoSidedMaterial", {
		ambientIntensity => [SFFloat, 0.2, inputOutput, "(SPEC_X3D33)"],
		backAmbientIntensity => [SFFloat, 0.2, inputOutput, "(SPEC_X3D33)"],
		backDiffuseColor => [SFColor, [0.8, 0.8, 0.8], inputOutput, "(SPEC_X3D33)"],
		backEmissiveColor => [SFColor, [0, 0, 0], inputOutput, "(SPEC_X3D33)"],
		backShininess => [SFFloat, 0.2, inputOutput, "(SPEC_X3D33)"],
		backSpecularColor => [SFColor, [0, 0, 0], inputOutput, "(SPEC_X3D33)"],
		backTransparency => [SFFloat, 0, inputOutput, "(SPEC_X3D33)"],
		diffuseColor => [SFColor, [0.8, 0.8, 0.8], inputOutput, "(SPEC_X3D33)"],
		emissiveColor => [SFColor, [0, 0, 0], inputOutput, "(SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D33)"],
		shininess => [SFFloat, 0.2, inputOutput, "(SPEC_X3D33)"],
		separateBackColor =>[SFBool,FALSE,inputOutput, "(SPEC_X3D33)"],
		specularColor => [SFColor, [0, 0, 0], inputOutput, "(SPEC_X3D33)"],
		transparency => [SFFloat, 0, inputOutput, "(SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DMaterialNode"),



	###################################################################################

	# Chapter 13:		Geometry3D Component

	###################################################################################

	Box => new VRML::NodeType("Box", { 	
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		size => [SFVec3f, [2, 2, 2], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__points  =>[FreeWRLPTR,0,initializeOnly, 0],
	},"X3DGeometryNode"),

	Cone => new VRML::NodeType ("Cone", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bottom => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		bottomRadius => [SFFloat, 1.0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		height => [SFFloat, 2.0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		side => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		 __sidepoints =>[FreeWRLPTR,0,initializeOnly, 0],
		 __botpoints =>[FreeWRLPTR,0,initializeOnly, 0],
		 __normals =>[FreeWRLPTR,0,initializeOnly, 0],
		__coneVBO =>[SFInt32,0,initializeOnly,0],
		__coneTriangles =>[SFInt32,0,initializeOnly,0],
	},"X3DGeometryNode"),

	Cylinder => new VRML::NodeType ("Cylinder", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bottom => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		height => [SFFloat, 2.0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radius => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		side => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		top => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		 __points =>[FreeWRLPTR,0,initializeOnly, 0],
		 __normals =>[FreeWRLPTR,0,initializeOnly, 0],
		__cylinderVBO =>[SFInt32,0,initializeOnly,0],
		__cylinderTriangles =>[SFInt32,0,initializeOnly,0],
	},"X3DGeometryNode"),

	ElevationGrid => new VRML::NodeType("ElevationGrid", {
		set_height => [MFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normal => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		creaseAngle => [SFFloat, 0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		height => [MFFloat, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		xDimension => [SFInt32, 0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		xSpacing => [SFFloat, 1.0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		zDimension => [SFInt32, 0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		zSpacing => [SFFloat, 1.0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_coordIndex => [MFInt32, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	},"X3DGeometryNode"),

	Extrusion => new VRML::NodeType("Extrusion", {
		set_crossSection => [MFVec2f, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_orientation => [MFRotation, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_scale => [MFVec2f, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_spine => [MFVec3f, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		beginCap => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		convex => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		creaseAngle => [SFFloat, 0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		crossSection => [MFVec2f, [[1, 1],[1, -1],[-1, -1],
						   [-1, 1],[1, 1]], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		endCap => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		orientation => [MFRotation, [[0, 0, 1, 0]],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		scale => [MFVec2f, [[1, 1]], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		spine => [MFVec3f, [[0, 0, 0],[0, 1, 0]], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DGeometryNode"),

	IndexedFaceSet => new VRML::NodeType("IndexedFaceSet", {
		set_colorIndex => [MFInt32, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_coordIndex => [MFInt32, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_normalIndex => [MFInt32, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_texCoordIndex => [MFInt32, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		attrib	=> [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coord => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fogCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normal => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorIndex => [MFInt32, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		convex => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		coordIndex => [MFInt32, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		creaseAngle => [SFFloat, 0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalIndex => [MFInt32, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoordIndex => [MFInt32, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DGeometryNode"),

	Sphere => new VRML::NodeType("Sphere", { 	
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radius => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__points =>[FreeWRLPTR,0,initializeOnly, 0],
		_sideVBO =>[SFInt32, 0, initializeOnly, 0], 
 	},"X3DGeometryNode"),


	###################################################################################

	#	Chapter 14:	Geometry 2D Component

	###################################################################################

	Arc2D => new VRML::NodeType("Arc2D", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		endAngle => [SFFloat, 1.5707, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radius => [SFFloat, 1.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		startAngle => [SFFloat, 0.0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__points  =>[FreeWRLPTR,0,initializeOnly, 0],
		__numPoints =>[SFInt32,0,initializeOnly, 0],
 	},"X3DGeometryNode"),

	ArcClose2D => new VRML::NodeType("ArcClose2D", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		closureType => [SFString,"PIE",initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	    	endAngle => [SFFloat, 1.5707, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	    	radius => [SFFloat, 1.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		solid => [SFBool, FALSE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	    	startAngle => [SFFloat, 0.0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__points  =>[FreeWRLPTR,0,initializeOnly, 0],
		__numPoints =>[SFInt32,0,initializeOnly, 0],
 	},"X3DGeometryNode"),


	Circle2D => new VRML::NodeType("Circle2D", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	    	radius => [SFFloat, 1.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__points  =>[FreeWRLPTR,0,initializeOnly, 0],
		__numPoints =>[SFInt32,0,initializeOnly, 0],
 	},"X3DGeometryNode"),

	Disk2D => new VRML::NodeType("Disk2D", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		innerRadius => [SFFloat, 0.0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		outerRadius => [SFFloat, 1.0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, FALSE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__points  =>[FreeWRLPTR,0,initializeOnly, 0],
		__texCoords  =>[FreeWRLPTR,0,initializeOnly, 0],
		__numPoints =>[SFInt32,0,initializeOnly, 0],
		__simpleDisk => [SFBool, TRUE,initializeOnly, 0],
	},"X3DGeometryNode"),

	Polyline2D => new VRML::NodeType("Polyline2D", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		lineSegments => [MFVec2f, [], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
 	},"X3DGeometryNode"),

	Polypoint2D => new VRML::NodeType("Polypoint2D", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	    	point => [MFVec2f, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
 	},"X3DGeometryNode"),

	Rectangle2D => new VRML::NodeType("Rectangle2D", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		size => [SFVec2f, [2.0, 2.0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		solid => [SFBool, FALSE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__points  =>[FreeWRLPTR,0,initializeOnly, 0],
		__numPoints =>[SFInt32,0,initializeOnly, 0],
 	},"X3DGeometryNode"),


	TriangleSet2D => new VRML::NodeType("TriangleSet2D", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	    	vertices => [MFVec2f, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, FALSE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__texCoords  =>[FreeWRLPTR,0,initializeOnly, 0],
 	},"X3DGeometryNode"),

	###################################################################################

	#	Chapter 15:		Text Component

	###################################################################################

	Text => new VRML::NodeType ("Text", {
		fontStyle => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		length => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		maxExtent => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		string => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		lineBounds => [MFVec2f,[],outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		origin => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		textBounds => [SFVec2f, [0, 0], outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__rendersub => [SFInt32, 0, inputOutput, 0] # Function ptr hack
	},"X3DTextNode"),

	FontStyle => new VRML::NodeType("FontStyle", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		family => [MFString, ["SERIF"], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		horizontal => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		justify => [MFString, ["BEGIN"], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		language => [SFString, "", initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		leftToRight => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		size => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		spacing => [SFFloat, 1.0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		style => [SFString, "PLAIN", initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		topToBottom => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DFontStyleNode"), 

	###################################################################################

	#	Chapter 16:		Sound Component

	###################################################################################

	AudioClip => new VRML::NodeType("AudioClip", {
		description => [SFString, "", inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		loop =>	[SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		pauseTime => [SFTime,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		pitch => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		resumeTime => [SFTime,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		startTime => [SFTime, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		stopTime => [SFTime, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		url => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		duration_changed => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		elapsedTime => [SFTime,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isPaused => [SFBool, TRUE,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
		# internal sequence number
		__sourceNumber => [SFInt32, -1, initializeOnly, 0],
		# local name, as received on system
		__localFileName => [FreeWRLPTR, 0,initializeOnly, 0],
		# time that we were initialized at
		__inittime => [SFTime, 0, initializeOnly, 0],
	},"X3DSoundSourceNode"),

	Sound => new VRML::NodeType("Sound", {
		direction => [SFVec3f, [0, 0, 1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		intensity => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		location => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		maxBack => [SFFloat, 10.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		maxFront => [SFFloat, 10.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		minBack => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		minFront => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		priority => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		source => [SFNode, NULL, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		spatialize => [SFBool, FALSE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DSoundSourceNode"),
	
	# for testing MIDI sounds
	AudioControl => new VRML::NodeType("AudioControl", {
		direction => [SFVec3f, [0, 0, 1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		intensity => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		location => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		maxBack => [SFFloat, 10.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		maxFront => [SFFloat, 10.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		minBack => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		minFront => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		enabled => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		source => [SFString, "", inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		# need distance, pan position as ints and floats
		volumeInt32Val => [SFInt32, 0, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		volumeFloatVal => [SFFloat, 0.0, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		panInt32Val => [SFInt32, 0, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		panFloatVal => [SFFloat, 0.0, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		deltaInt32Val => [SFInt32, 0, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		deltaFloatVal => [SFFloat, 0.0, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro

		# used for determing rate of change of position:
		__oldLen =>[SFTime, 0.0, initializeOnly, 0],
		maxDelta => [SFFloat, 10.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldEnabled => [SFBool, TRUE, inputOutput, 0],
	},"X3DSoundSourceNode"),

	###################################################################################

	# Chapter 17:		Lighting Component

	###################################################################################

	DirectionalLight => new VRML::NodeType("DirectionalLight", {
		ambientIntensity => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFColor, [1, 1, 1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		direction => [SFVec3f, [0, 0, -1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		global => [SFBool, FALSE, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		intensity => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		on => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DLightNode"),

	PointLight => new VRML::NodeType("PointLight", {
		ambientIntensity => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		attenuation => [SFVec3f, [1, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFColor, [1, 1, 1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		global => [SFBool, TRUE, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		intensity => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		location => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		on => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radius => [SFFloat, 100.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		##not in the spec
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DLightNode"),

	SpotLight => new VRML::NodeType("SpotLight", {
		ambientIntensity => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		attenuation => [SFVec3f, [1, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		beamWidth => [SFFloat, 1.570796, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFColor, [1, 1, 1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		cutOffAngle => [SFFloat, 0.785398, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		direction => [SFVec3f, [0, 0, -1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		global => [SFBool, TRUE, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		intensity => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		location => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		on => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radius => [SFFloat, 100.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DLightNode"),

	###################################################################################

	#	Chapter18:	Texturing Component

	###################################################################################

	ImageTexture => new VRML::NodeType("ImageTexture", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		url => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		repeatS => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		repeatT => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		textureProperties => [SFNode, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__textureTableIndex => [SFInt32, 0, initializeOnly, 0],
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
	},"X3DTextureNode"),

	MovieTexture => new VRML::NodeType ("MovieTexture", {
		description => [SFString, "", inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		loop => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		resumeTime => [SFTime,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		pauseTime => [SFTime,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		speed => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		startTime => [SFTime, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		stopTime => [SFTime, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		url => [MFString, [""], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		duration_changed => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		elapsedTime => [SFTime,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isPaused => [SFTime,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		repeatS => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		repeatT => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		textureProperties => [SFNode, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__textureTableIndex => [SFInt32, 0, initializeOnly, 0],
		 # which texture number is used
		 #__ctex => [SFInt32, 0, initializeOnly, 0],
		 # time that we were initialized at
		 #__inittime => [SFTime, 0, initializeOnly, 0],
		 # internal sequence number
		 #__sourceNumber => [SFInt32, -1, initializeOnly, 0],
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
	},"X3DTextureNode"),


	MultiTexture => new VRML::NodeType("MultiTexture", {
		alpha =>[SFFloat, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color =>[SFColor,[1,1,1],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		function =>[MFString,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		mode =>[MFString,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		source =>[MFString,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texture=>[MFNode,undef,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__params => [FreeWRLPTR, 0, initializeOnly, 0],
	},"X3DTextureNode"),

	MultiTextureCoordinate => new VRML::NodeType("MultiTextureCoordinate", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord =>[MFNode,undef,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DTextureCoordinateNode"),

	MultiTextureTransform => new VRML::NodeType("MultiTextureTransform", {
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		textureTransform=>[MFNode,undef,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DTextureTransformNode"),

	PixelTexture => new VRML::NodeType("PixelTexture", {
		image => [SFImage, "0, 0, 0", inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		repeatS => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		repeatT => [SFBool, TRUE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		textureProperties => [SFNode, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
		__textureTableIndex => [SFInt32, 0, initializeOnly, 0],
	},"X3DTextureNode"),

	TextureCoordinate => new VRML::NodeType("TextureCoordinate", { 
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		point => [MFVec2f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__compiledpoint => [MFVec2f, [], initializeOnly, 0],
		__lastParent => [FreeWRLPTR, 0, initializeOnly, 0],
		__VBO =>[SFInt32,0,initializeOnly,0],
	},"X3DTextureCoordinateNode"),

	TextureCoordinateGenerator => new VRML::NodeType("TextureCoordinateGenerator", { 
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		mode => [SFString,"SPHERE",inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		parameter => [MFFloat, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__compiledmode => [SFInt32,0,initializeOnly, 0],
	},"X3DTextureCoordinateNode"),

	TextureProperties => new VRML::NodeType("TextureProperties", {
		anisotropicDegree => [SFFloat, 1.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		borderColor=>[SFColorRGBA,[0,0,0,0],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
		borderWidth => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		boundaryModeS => [SFString, "REPEAT", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		boundaryModeT => [SFString, "REPEAT", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		boundaryModeR => [SFString, "REPEAT", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		magnificationFilter => [SFString, "FASTEST", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		minificationFilter => [SFString, "FASTEST", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		textureCompression => [SFString, "FASTEST", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texturePriority => [SFFloat, 0.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		generateMipMaps => [SFBool, FALSE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	}, "X3DSFNode"),

	TextureTransform => new VRML::NodeType ("TextureTransform", {
		center => [SFVec2f, [0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		rotation => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		scale => [SFVec2f, [1, 1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		translation => [SFVec2f, [0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DTextureTransformNode"),


	###################################################################################

	#	Chapter 19:		Interpolation Component

	###################################################################################

	ColorInterpolator => new VRML::NodeType("ColorInterpolator", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFColor, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [SFColor, [0, 0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	CoordinateInterpolator => new VRML::NodeType("CoordinateInterpolator", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFVec3f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [MFVec3f, [], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	CoordinateInterpolator2D => new VRML::NodeType("CoordinateInterpolator2D", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFVec2f, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [MFVec2f, [[0,0]], outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	EaseInEaseOut => new VRML::NodeType("EaseInEaseOut", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		easeInEaseOut => [MFVec2f, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		modifiedFraction_changed => [SFFloat, 0.0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),



	NormalInterpolator => new VRML::NodeType("NormalInterpolator", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFVec3f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [MFVec3f, [], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	OrientationInterpolator => new VRML::NodeType("OrientationInterpolator", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFRotation, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [SFRotation, [0, 0, 1, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	PositionInterpolator => new VRML::NodeType("PositionInterpolator", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFVec3f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	PositionInterpolator2D => new VRML::NodeType("PositionInterpolator2D", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFVec2f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [SFVec2f, [0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	ScalarInterpolator => new VRML::NodeType("ScalarInterpolator", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [SFFloat, 0.0, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	SplinePositionInterpolator => new VRML::NodeType("SplinePositionInterpolator", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		closed => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFVec3f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyVelocity => [MFVec3f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalizeVelocity => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [SFVec3f, [0,0,0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	SplinePositionInterpolator2D => new VRML::NodeType("SplinePositionInterpolator2D", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		closed => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFVec2f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyVelocity => [MFVec2f, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalizeVelocity => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [SFVec2f, [0,0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	SplineScalarInterpolator => new VRML::NodeType("SplineScalarInterpolator", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		closed => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyVelocity => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalizeVelocity => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [SFFloat, 0.0, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	SquadOrientationInterpolator => new VRML::NodeType("SquadOrientationInterpolator", {
		set_fraction => [SFFloat, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		key => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		keyValue => [MFRotation, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalizeVelocity => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		value_changed => [SFRotation, [0,0,1,0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DInterpolatorNode"),

	###################################################################################

	#		Cubemap Texturing Component

	###################################################################################


	ComposedCubeMapTexture => new VRML::NodeType("ComposedCubeMapTexture", {
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		back =>[SFNode,NULL,inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bottom =>[SFNode,NULL,inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		front =>[SFNode,NULL,inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		left =>[SFNode,NULL,inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		top =>[SFNode,NULL,inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		right =>[SFNode,NULL,inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DEnvironmentTextureNode"),

	GeneratedCubeMapTexture => new VRML::NodeType("GeneratedCubeMapTexture", {
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		update => [SFString,"NONE",inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		size => [SFInt32,128,inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		textureProperties => [SFNode, NULL, initializeOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__textureTableIndex => [SFInt32, 0, initializeOnly, 0],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DEnvironmentTextureNode"),

	ImageCubeMapTexture => new VRML::NodeType("ImageCubeMapTexture", {
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		url => [MFString,[],inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		textureProperties => [SFNode, NULL, initializeOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__textureTableIndex => [SFInt32, 0, initializeOnly, 0],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
		__subTextures => [MFNode,[],initializeOnly,0],
		__regenSubTextures => [SFBool,FALSE,initializeOnly,0],
	},"X3DEnvironmentTextureNode"),




	###################################################################################

	#		Pointing Device Component

	###################################################################################

	TouchSensor => new VRML::NodeType("TouchSensor", {
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			hitNormal_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			hitPoint_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			hitTexCoord_changed => [SFVec2f, [0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			_oldhitNormal => [SFVec3f, [0, 0, 0], outputOnly, 0], 	# send event only if changed
			_oldhitPoint => [SFVec3f, [0, 0, 0], outputOnly, 0], 	# send event only if changed
			_oldhitTexCoord => [SFVec2f, [0, 0], outputOnly, 0], 	# send event only if changed
			isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isOver => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			description => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			touchTime => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
					   },"X3DPointingDeviceSensorNode"),

	PlaneSensor => new VRML::NodeType("PlaneSensor", {
			autoOffset => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			maxPosition => [SFVec2f, [-1, -1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			minPosition => [SFVec2f, [0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			offset => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isOver => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			description => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			trackPoint_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			translation_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			_oldtrackPoint => [SFVec3f, [0, 0, 0], outputOnly, 0],
			_oldtranslation => [SFVec3f, [0, 0, 0], outputOnly, 0],
			# where we are at a press...
			_origPoint => [SFVec3f, [0, 0, 0], initializeOnly, 0],
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
					   },"X3DPointingDeviceSensorNode"),

	SphereSensor => new VRML::NodeType("SphereSensor", {
			autoOffset => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			offset => [SFRotation, [0, 1, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			rotation_changed => [SFRotation, [0, 0, 1, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			trackPoint_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			_oldtrackPoint => [SFVec3f, [0, 0, 0], outputOnly, 0],
			_oldrotation => [SFRotation, [0, 0, 1, 0], outputOnly, 0],
			isOver => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			description => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			# where we are at a press...
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			_origPoint => [SFVec3f, [0, 0, 0], initializeOnly, 0],
			_origNormalizedPoint => [SFVec3f, [0, 0, 0], initializeOnly, 0],
			_radius => [SFFloat, 0, initializeOnly, 0],
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
					   },"X3DPointingDeviceSensorNode"),

	CylinderSensor => new VRML::NodeType("CylinderSensor", {
			autoOffset => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			diskAngle => [SFFloat, 0.262, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			maxAngle => [SFFloat, -1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			minAngle => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			offset => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isOver => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			description => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			rotation_changed => [SFRotation, [0, 0, 1, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			trackPoint_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			_oldtrackPoint => [SFVec3f, [0, 0, 0], outputOnly, 0],
			_oldrotation => [SFRotation, [0, 0, 1, 0], outputOnly, 0],
			# where we are at a press...
			_origPoint => [SFVec3f, [0, 0, 0], initializeOnly, 0],
			_radius => [SFFloat, 0, initializeOnly, 0],
			_dlchange => [SFInt32, 0, initializeOnly, 0],
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
					   },"X3DPointingDeviceSensorNode"),


	###################################################################################

	#		Key Device Component

	###################################################################################

	# KeySensor
	KeySensor => new VRML::NodeType("KeySensor", {
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			actionKeyPress =>[SFInt32,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			actionKeyRelease =>[SFInt32,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			altKey =>[SFBool, TRUE,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			controlKey =>[SFBool, TRUE,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isActive =>[SFBool, TRUE,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			keyPress =>[SFString,"",outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			keyRelease =>[SFString,"",outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			shiftKey =>[SFBool, TRUE,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
					   },"X3DKeyDeviceSensorNode"),

	# StringSensor
	StringSensor => new VRML::NodeType("StringSensor", {
			deletionAllowed => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enteredText => [SFString,"",outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			finalText => [SFString,"",outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isActive =>[SFBool, TRUE,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			_initialized =>[SFBool, FALSE,initializeOnly, 0],
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
					   },"X3DKeyDeviceSensorNode"),


	###################################################################################

	#		Environmental Sensor Component

	###################################################################################


	ProximitySensor => new VRML::NodeType("ProximitySensor", {
			center => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			size => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			position_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			orientation_changed => [SFRotation, [0, 0, 1, 0], outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enterTime => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			exitTime => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			centerOfRotation_changed =>[SFVec3f, [0,0,0], outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro

			# These fields are used for the info.
			__hit => [SFInt32, 0, inputOutput, 0],
			__t1 => [SFVec3f, [10000000, 0, 0], inputOutput, 0],
			__t2 => [SFRotation, [0, 1, 0, 0], inputOutput, 0],
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
					   },"X3DEnvironmentalSensorNode"),

	VisibilitySensor => new VRML::NodeType("VisibilitySensor", {
			center => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			size => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enterTime => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			exitTime => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isActive => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			 __visible =>[SFInt32,0,initializeOnly, 0], # for Occlusion tests.
			 __occludeCheckCount =>[SFInt32,-1,initializeOnly, 0], # for Occlusion tests.
			__points  =>[FreeWRLPTR,0,initializeOnly, 0],	# for Occlude Box.
			__Samples =>[SFInt32,0,initializeOnly, 0],		# Occlude samples from last pass
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
					   },"X3DEnvironmentalSensorNode"),



	###################################################################################

	#		Navigation Component

	###################################################################################

	LOD => new VRML::NodeType("LOD", {
		addChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		removeChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		level => [MFNode, [], inputOutput, "(SPEC_VRML)"], 		# for VRML spec
		children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],		# for X3D spec
		center => [SFVec3f, [0, 0, 0],  inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		range => [MFFloat, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		levelChanged => [SFInt32, 0, outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		forceTransitions => [SFBool, FALSE, initializeOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__isX3D => [SFBool, "(inputFileVersion[0]==3)" , initializeOnly, 0], # TRUE for X3D V3.x files
		_selected =>[FreeWRLPTR,0,initializeOnly, 0],
	},"X3DGroupingNode"),

	Billboard => new VRML::NodeType("Billboard", {
			addChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			removeChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			axisOfRotation => [SFVec3f, [0, 1, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			_rotationAngle =>[SFDouble, 0, initializeOnly, 0],
		_sortedChildren => [MFNode, [], inputOutput, 0],
					   },"X3DGroupingNode"),

	Collision => new VRML::NodeType("Collision", {
			addChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			removeChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			collide => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			proxy => [SFNode, NULL, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			collideTime => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_sortedChildren => [MFNode, [], inputOutput, 0],
			# return info for collisions
			# bit 0 : collision or not
			# bit 1: changed from previous of not
			__hit => [SFInt32, 0, inputOutput, 0]
					   },"X3DEnvironmentalSensorNode"),

	Viewpoint => new VRML::NodeType("Viewpoint", {
		set_bind => [SFBool, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		centerOfRotation =>[SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		description => [SFString, "", inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		fieldOfView => [SFFloat, 0.785398, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		jump => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		orientation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		position => [SFVec3f,[0, 0, 10], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		retainUserOffsets => [SFBool, FALSE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bindTime => [SFTime, -1, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isBound => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__BGNumber => [SFInt32,-1,initializeOnly, 0], # for ordering backgrounds for binding
	},"X3DBindableNode"),

	OrthoViewpoint => new VRML::NodeType("OrthoViewpoint", {
		set_bind => [SFBool, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		centerOfRotation =>[SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		description => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		fieldOfView => [MFFloat, [-1.0, -1.0, 1.0, 1.0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		jump => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		orientation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		position => [SFVec3f,[0, 0, 10], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		retainUserOffsets => [SFBool, FALSE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bindTime => [SFTime, -1, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isBound => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__BGNumber => [SFInt32,-1,initializeOnly, 0], # for ordering backgrounds for binding
	},"X3DBindableNode"),



	NavigationInfo => new VRML::NodeType("NavigationInfo", {
		set_bind => [SFBool, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		avatarSize => [MFFloat, [0.25, 1.6, 0.75], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		headlight => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		speed => [SFFloat, 1.0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		type => [MFString, ["EXAMINE", "ANY"], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		visibilityLimit => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isBound => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transitionType => [MFString, [],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bindTime => [SFTime, -1, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transitionTime => [SFTime, 1.0, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transitionComplete => [SFBool, FALSE, outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__BGNumber => [SFInt32,-1,initializeOnly, 0], # for ordering backgrounds for binding
	},"X3DBindableNode"),

	ViewpointGroup => new VRML::NodeType("ViewpointGroup", {
		center => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		description => [SFString, "", inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
		displayed => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		retainUserOffsets => [SFBool, FALSE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		size => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__proxNode=> [SFNode, NULL, inputOutput, "0"],
	},"X3DGroupingNode"),



	###################################################################################

	#		Environmental Effects Component

	###################################################################################

	Background => new VRML::NodeType("Background", {
		set_bind => [SFBool, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		groundAngle => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		groundColor => [MFColor, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		skyAngle => [MFFloat, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		skyColor => [MFColor, [[0, 0, 0]], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bindTime => [SFTime,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isBound => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
		__points =>[FreeWRLPTR,0,initializeOnly, 0],
		__colours =>[FreeWRLPTR,0,initializeOnly, 0],
		__quadcount => [SFInt32,0,initializeOnly, 0],
		__BGNumber => [SFInt32,-1,initializeOnly, 0], # for ordering backgrounds for binding

		transparency => [SFFloat, 0.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		frontUrl => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		backUrl => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		topUrl => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bottomUrl => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		leftUrl => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		rightUrl => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		__textureright => [SFInt32, 0, inputOutput, 0],
		__frontTexture=>[SFNode,NULL,inputOutput, 0],
		__backTexture=>[SFNode,NULL,inputOutput, 0],
		__topTexture=>[SFNode,NULL,inputOutput, 0],
		__bottomTexture=>[SFNode,NULL,inputOutput, 0],
		__leftTexture=>[SFNode,NULL,inputOutput, 0],
		__rightTexture=>[SFNode,NULL,inputOutput, 0],
	},"X3DBackgroundNode"),



	Fog => new VRML::NodeType("Fog", {
			set_bind => [SFBool, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			color => [SFColor, [1, 1, 1], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			fogType => [SFString, "LINEAR", inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			visibilityRange => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bindTime => [SFTime, -1, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isBound => [SFBool, FALSE, outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			__BGNumber => [SFInt32,-1,initializeOnly, 0], # for ordering backgrounds for binding
					   },"X3DBindableNode"),

	FogCoordinate => new VRML::NodeType("FogCoordinate", {
		depth => [MFFloat, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					   },"X3DGeometricPropertyNode"),

	LocalFog => new VRML::NodeType("Fog", {
			color => [SFColor, [1, 1, 1], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			fogType => [SFString, "LINEAR", inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			visibilityRange => [SFFloat, 0, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					   },"X3DChildNode"),

	TextureBackground => new VRML::NodeType("TextureBackground", {
		set_bind => [SFBool, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		groundAngle => [MFFloat, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		groundColor => [MFColor, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		skyAngle => [MFFloat, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		skyColor => [MFColor, [[0,0,0]], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bindTime => [SFTime,0,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isBound => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
		__points =>[FreeWRLPTR,0,initializeOnly, 0],
		__colours =>[FreeWRLPTR,0,initializeOnly, 0],
		__quadcount => [SFInt32,0,initializeOnly, 0],
		__BGNumber => [SFInt32,-1,initializeOnly, 0], # for ordering backgrounds for binding

		frontTexture=>[SFNode,NULL,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		backTexture=>[SFNode,NULL,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		topTexture=>[SFNode,NULL,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bottomTexture=>[SFNode,NULL,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		leftTexture=>[SFNode,NULL,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		rightTexture=>[SFNode,NULL,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transparency=> [MFFloat,[0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	},"X3DBackgroundNode"),

	###################################################################################

	#		Geospatial Component

	###################################################################################


	GeoCoordinate => new VRML::NodeType("GeoCoordinate", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			point => [MFVec3d,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			geoOrigin => [SFNode, NULL, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__geoSystem => [MFInt32,[],initializeOnly, 0],
			__movedCoords => [MFVec3f, [], inputOutput, 0],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DCoordinateNode"),

	GeoElevationGrid => new VRML::NodeType("GeoElevationGrid", {
		set_height => [MFDouble, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		color => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normal => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		texCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		yScale => [SFFloat, 1.0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		ccw => [SFBool, FALSE,initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		colorPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		creaseAngle => [SFDouble, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		geoGridOrigin => [SFVec3d,[0,0,0],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		geoOrigin => [SFNode, NULL, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		height => [MFDouble, [0,0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		normalPerVertex => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		solid => [SFBool, TRUE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		xDimension => [SFInt32, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		xSpacing => [SFDouble, 1.0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		zDimension => [SFInt32, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		zSpacing => [SFDouble, 1.0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		_coordIndex => [MFInt32, [], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__geoSystem => [MFInt32,[],initializeOnly, 0],
		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DGeometryNode"),

	GeoLOD => new VRML::NodeType("GeoLOD", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

			# the following screws up routing in the old VRML parser, because children can
			# be an "EXPOSED_FIELD" AND an "EVENT_OUT", so by changing this to an "inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"
			# we can have only one field, the EXPOSED_FIELD_children
			#children => [MFNode,[],outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

			children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			level_changed =>[SFInt32,0,outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

			center => [SFVec3d,[0,0,0],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			child1Url =>[MFString,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			child2Url =>[MFString,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			child3Url =>[MFString,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			child4Url =>[MFString,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoOrigin => [SFNode, NULL, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			range => [SFFloat,10.0,initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			rootUrl => [MFString,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			rootNode => [MFNode,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

			__geoSystem => [MFInt32,[],initializeOnly, 0],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__inRange =>[SFBool, FALSE, inputOutput, 0],
			__child1Node => [SFNode, NULL, inputOutput, 0],
			__child2Node => [SFNode, NULL, inputOutput, 0],
			__child3Node => [SFNode, NULL, inputOutput, 0],
			__child4Node => [SFNode, NULL, inputOutput, 0],
			__rootUrl => [SFNode, NULL, inputOutput, 0],
			__childloadstatus => [SFInt32,0,inputOutput, 0],
			__rooturlloadstatus => [SFInt32,0,inputOutput, 0],

			# ProximitySensor copies.
			#__t1 => [SFVec3d, [10000000, 0, 0], inputOutput, 0],
			__level => [SFInt32,-1,inputOutput, 0], # only for debugging purposes
					},"X3DGroupingNode"),


	GeoMetadata=> new VRML::NodeType("GeoMetadata", {
			data => [MFNode,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			summary => [MFString,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			url => [MFString,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DChildNode"),

	GeoPositionInterpolator=> new VRML::NodeType("GeoPositionInterpolator", {
			set_fraction => [SFFloat,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			key => [MFFloat,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			keyValue => [MFVec3d,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geovalue_changed => [SFVec3d,[0,0,0],outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value_changed => [SFVec3f,[0,0,0],outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoOrigin => [SFNode, NULL, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__geoSystem => [MFInt32,[],initializeOnly, 0],
			__movedValue => [MFVec3d, [], inputOutput, 0],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			__oldKeyPtr => [SFNode, NULL, outputOnly, 0],
			__oldKeyValuePtr => [SFNode, NULL, outputOnly, 0],
					},"X3DInterpolatorNode"),


	GeoProximitySensor => new VRML::NodeType("ProximitySensor", {
			enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D33)"],
			geoCenter => [SFVec3d, [0, 0, 0], inputOutput, "(SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D33)"],
			size => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D33)"],
			centerOfRotation_changed =>[SFVec3f, [0,0,0], outputOnly, "(SPEC_X3D33)"],
			enterTime => [SFTime, -1, outputOnly, "(SPEC_X3D33)"],
			exitTime => [SFTime, -1, outputOnly, "(SPEC_X3D33)"],
			geoCoord_changed => [SFVec3d,[0,0,0],outputOnly, "(SPEC_X3D33)"],
			isActive => [SFBool, FALSE, outputOnly, "(SPEC_X3D33)"],
			orientation_changed => [SFRotation, [0, 0, 1, 0], outputOnly, "(SPEC_X3D33)"],
			position_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_X3D33)"],
			geoOrigin => [SFNode, NULL, initializeOnly, "(SPEC_X3D33)"],
			geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D33)"],


			# These fields are used for the info.
			__hit => [SFInt32, 0, inputOutput, 0],
			__t1 => [SFVec3f, [10000000, 0, 0], inputOutput, 0],
			__t2 => [SFRotation, [0, 1, 0, 0], inputOutput, 0],

			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly, 0],
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__localOrient => [SFVec4d, [0, 0, 1, 0], inputOutput, 0],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
			__oldGeoCenter => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__oldSize => [SFVec3f, [0, 0, 0], inputOutput, 0],
					   },"X3DEnvironmentalSensorNode"),

	GeoTouchSensor=> new VRML::NodeType("GeoTouchSensor", {
			description => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			enabled => [SFBool, FALSE,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			hitNormal_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			hitPoint_changed => [SFVec3f, [0, 0, 0], outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			hitTexCoord_changed => [SFVec2f, [0, 0], outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			hitGeoCoord_changed => [SFVec3d, [0, 0, 0] ,outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isActive => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isOver => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			touchTime => [SFTime, -1, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoOrigin => [SFNode, NULL, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__geoSystem => [MFInt32,[],initializeOnly, 0],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			_oldhitNormal => [SFVec3f, [0, 0, 0], outputOnly, 0], 	# send event only if changed
			_oldhitPoint => [SFVec3f, [0, 0, 0], outputOnly, 0], 	# send event only if changed
			_oldhitTexCoord => [SFVec2f, [0, 0], outputOnly, 0], 	# send event only if changed
			__oldEnabled => [SFBool, TRUE, inputOutput, 0],
					},"X3DPointingDeviceSensorNode"),


	GeoTransform => new VRML::NodeType ("GeoTransform", {
			addChildren => [MFNode, undef, inputOnly, "(SPEC_X3D33)"],
			removeChildren => [MFNode, undef, inputOnly, "(SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_X3D33)"],
			geoCenter => [SFVec3d, [0, 0, 0], inputOutput, "(SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D33)"],
			rotation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D33)"],
			scale => [SFVec3f, [1, 1, 1], inputOutput, "(SPEC_X3D33)"],
			scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D33)"],
			translation => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D33)"],
			geoOrigin => [SFNode, NULL, initializeOnly, "(SPEC_X3D33)"],
			geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D33)"],

			# fields for reducing redundant calls
			__do_center => [SFInt32, 0, initializeOnly, 0],
			__do_trans => [SFInt32, 0, initializeOnly, 0],
			__do_rotation => [SFInt32, 0, initializeOnly, 0],
			__do_scaleO => [SFInt32, 0, initializeOnly, 0],
			__do_scale => [SFInt32, 0, initializeOnly, 0],

			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly, 0],
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__localOrient => [SFVec4d, [0, 0, 1, 0], inputOutput, 0],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			__oldGeoCenter => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__oldChildren => [MFNode, [], inputOutput, 0],
		_sortedChildren => [MFNode, [], inputOutput, 0],
					},"X3DGroupingNode"),

	GeoViewpoint => new VRML::NodeType("GeoViewpoint", {
			set_bind => [SFBool, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			set_orientation => [SFRotation, [IO_FLOAT, IO_FLOAT, IO_FLOAT, IO_FLOAT], inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			set_position => [SFVec3d, [IO_FLOAT, IO_FLOAT, IO_FLOAT], inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			description => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			fieldOfView => [SFFloat, 0.785398, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			headlight => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			jump => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			navType => [MFString, ["EXAMINE","ANY"],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bindTime => [SFTime, -1, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isBound => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

			geoOrigin => [SFNode, NULL, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			orientation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			position => [SFVec3d,[0, 0, 100000], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			speedFactor => [SFFloat,1.0,initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__BGNumber => [SFInt32,-1,initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # for ordering backgrounds for binding

			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly, 0],
			__movedPosition => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__movedOrientation => [SFRotation, [0, 0, 1, 0], initializeOnly, 0],

			__oldSFString => [SFString, "", inputOutput, 0], #the description field
			__oldFieldOfView => [SFFloat, 0.785398, inputOutput, 0],
			__oldHeadlight => [SFBool, TRUE, inputOutput, 0],
			__oldJump => [SFBool, TRUE, inputOutput, 0],
			__oldMFString => [MFString, [],inputOutput, 0], # the navType
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro

					
					   },"X3DBindableNode"),

	GeoOrigin => new VRML::NodeType("GeoOrigin", {
			geoCoords => [SFVec3d, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			rotateYUp => [SFBool, TRUE,initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

			# these are now static in CFuncs/GeoVRML.c
			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly, 0],
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__oldgeoCoords => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__oldMFString => [MFString, [],inputOutput, 0], # the navType
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			
					},"X3DChildNode"),

	GeoLocation => new VRML::NodeType("GeoLocation", {
			addChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			removeChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoCoords => [SFVec3d, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoOrigin => [SFNode, NULL, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			geoSystem => [MFString,["GD","WE"],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

			# "compiled" versions of strings above
			__geoSystem => [MFInt32,[],initializeOnly, 0],
			__movedCoords => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__localOrient => [SFVec4d, [0, 0, 1, 0], inputOutput, 0],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			__oldgeoCoords => [SFVec3d, [0, 0, 0], inputOutput, 0],
			__oldChildren => [MFNode, [], inputOutput, 0],
		_sortedChildren => [MFNode, [], inputOutput, 0],
					},"X3DGroupingNode"),


	###################################################################################

	#		H-Anim Component

	###################################################################################

	HAnimDisplacer => new VRML::NodeType("HAnimDisplacer", {
			coordIndex => [MFInt32, [], inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			displacements => [MFVec3f, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			weight => [SFFloat, 0.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DGeometricPropertyNode"),

	HAnimHumanoid => new VRML::NodeType("HAnimHumanoid", {
			center => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			info => [MFString, [],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			joints => [MFNode,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			rotation => [SFRotation,[0,0,1,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			scale => [SFVec3f,[1,1,1],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			segments => [MFNode,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			sites => [MFNode,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			skeleton => [MFNode,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			skin => [MFNode,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			skinCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			skinNormal => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			translation => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			version => [SFString,"",inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			viewpoints => [MFNode,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DChildNode"),

	HAnimJoint => new VRML::NodeType("HAnimJoint", {

			addChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			removeChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			center => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			rotation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			scale => [SFVec3f, [1, 1, 1], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			translation => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			displacers => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			limitOrientation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			llimit => [MFFloat,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			skinCoordIndex => [MFInt32,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			skinCoordWeight => [MFFloat,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			stiffness => [MFFloat,[0,0,0],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			ulimit => [MFFloat,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro

			 # fields for reducing redundant calls
			 __do_center => [SFInt32, 0, initializeOnly, 0],
			 __do_trans => [SFInt32, 0, initializeOnly, 0],
			 __do_rotation => [SFInt32, 0, initializeOnly, 0],
			 __do_scaleO => [SFInt32, 0, initializeOnly, 0],
			 __do_scale => [SFInt32, 0, initializeOnly, 0],
					},"X3DChildNode"),

	HAnimSegment => new VRML::NodeType("HAnimSegment", {
			addChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			removeChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			centerOfMass => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			coord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			displacers => [MFNode,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			mass => [SFFloat, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			momentsOfInertia =>[MFFloat, [0, 0, 0, 0, 0, 0, 0, 0, 0],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DChildNode"),



	HAnimSite => new VRML::NodeType("HAnimSite", {
			addChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			removeChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			center => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			rotation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			scale => [SFVec3f, [1, 1, 1], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			scaleOrientation => [SFRotation, [0, 0, 1, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			translation => [SFVec3f, [0, 0, 0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro

			 # fields for reducing redundant calls
			 __do_center => [SFInt32, 0, initializeOnly, 0],
			 __do_trans => [SFInt32, 0, initializeOnly, 0],
			 __do_rotation => [SFInt32, 0, initializeOnly, 0],
			 __do_scaleO => [SFInt32, 0, initializeOnly, 0],
			 __do_scale => [SFInt32, 0, initializeOnly, 0],
					},"X3DGroupingNode"),


	###################################################################################

	#		NURBS Component

	###################################################################################

	Contour2D => new VRML::NodeType("Contour2D", {
			addChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			removeChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DParametricGeometryNode"),


	ContourPolyLine2D =>
	new VRML::NodeType("ContourPolyline2D",
					{
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DParametricGeometryNode"
					),

	NurbsCurve =>
	new VRML::NodeType("NurbsCurve",
					{
			controlPoint =>[MFVec3f,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			weight => [MFFloat,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			tessellation => [SFInt32,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			knot => [MFFloat,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			order => [SFInt32,3,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DParametricGeometryNode"
					),

	NurbsCurve2D =>
	new VRML::NodeType("NurbsCurve2D",
					{
			controlPoint =>[MFVec2f,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			weight => [MFFloat,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			tessellation => [SFInt32,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			knot => [MFFloat,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			order => [SFInt32,3,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DParametricGeometryNode"
					),

	NurbsGroup =>
	new VRML::NodeType("NurbsGroup",
					{
			addChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			removeChildren => [MFNode, undef, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			tessellationScale => [SFFloat,1.0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DGroupingNode"
					),

	NurbsPositionInterpolator =>
	new VRML::NodeType("NurbsPositionInterpolator",
					{

			set_fraction => [SFFloat,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			dimension => [SFInt32,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			keyValue => [MFVec3f,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			keyWeight => [MFFloat,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			knot => [MFFloat,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],# see note top of file
			order => [SFInt32,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value_changed => [SFVec3f,[0,0,0],outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DInterpolatorNode"
					),

	NurbsSurface =>
	new VRML::NodeType("NurbsSurface",
					{
			controlPoint =>[MFVec3f,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			texCoord => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			uTessellation => [SFInt32,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			vTessellation => [SFInt32,0,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			weight => [MFFloat,[],inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			ccw => [SFBool, FALSE,initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

			knot => [MFFloat,[],initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			order => [SFInt32,3,inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DParametricGeometryNode"
					),
	NurbsTextureSurface =>
	new VRML::NodeType("NurbsTextureSurface",
					{
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DParametricGeometryNode"
					),

	NurbsTrimmedSurface =>
	new VRML::NodeType("NurbsTrimmedSurface",
					{
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
					},"X3DParametricGeometryNode"
					),

	###################################################################################

	# Chapter 28: Distributed Interactive Simulation Component

	###################################################################################


	DISEntityManager => new VRML::NodeType("DISEntityManager", {
		address => [SFString, "localhost", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		applicationID => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		mapping => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		port => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		siteID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		addedEntities => [MFNode, [], outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		removedEntities => [MFNode, [], outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	},"X3DChildNode"),

	DISEntityTypeMapping => new VRML::NodeType("DISEntityTypeMapping", {
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		url => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		category => [SFInt32, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		country => [SFInt32, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		domain => [SFInt32, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		extra => [SFInt32, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		kind => [SFInt32, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		specific => [SFInt32, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		subcategory => [SFInt32, 0, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		
	},"X3DInfoNode"),


	EspduTransform => new VRML::NodeType("EspduTransform", {
		addChildren => [MFNode, [], inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		removeChildren => [MFNode, [], inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_articulationParameterValue0 => [SFFloat, 0.0, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_articulationParameterValue1 => [SFFloat, 0.0, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_articulationParameterValue2 => [SFFloat, 0.0, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_articulationParameterValue3 => [SFFloat, 0.0, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_articulationParameterValue4 => [SFFloat, 0.0, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_articulationParameterValue5 => [SFFloat, 0.0, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_articulationParameterValue6 => [SFFloat, 0.0, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		set_articulationParameterValue7 => [SFFloat, 0.0, inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		address => [SFString, "localhost", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		applicationID => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterCount => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterDesignatorArray => [MFInt32, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterChangeIndicatorArr => [MFInt32, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterIdPartAttachedToAr => [MFInt32, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterTypeArray => [MFInt32, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterArray => [MFFloat, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		center => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		children => [MFNode, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		collisionType => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		deadReckoning => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		detonationLocation => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		detonationRelativeLocation => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		detonationResult => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entityCategory => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entityCountry => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entityDomain => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entityExtra => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entityID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entityKind => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entitySpecific => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entitySubCategory => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		eventApplicationID => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		eventEntityID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		eventNumber => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		eventSiteID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fired1 => [SFBool, FALSE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fired2 => [SFBool, FALSE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fireMissionIndex => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		firingRange => [SFFloat, 0.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		firingRate => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		forceID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		fuse => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		linearVelocity => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		linearAcceleration => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		marking => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		multicastRelayHost => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		multicastRelayPort => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		munitionApplicationID => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		munitionEndPoint => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		munitionEntityID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		munitionQuantity => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		munitionSiteID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		munitionStartPoint => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		networkMode => [SFString, "standAlone", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		port => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		readInterval => [SFTime, 0.1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		rotation => [SFRotation, [0,0,1,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		scale => [SFVec3f, [1,1,1], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		scaleOrientation => [SFRotation, [0,0,1,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		siteID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		translation => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		warhead => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		writeInterval => [SFTime, 1.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterValue0_changed => [SFFloat, 0.0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterValue1_changed => [SFFloat, 0.0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterValue2_changed => [SFFloat, 0.0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterValue3_changed => [SFFloat, 0.0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterValue4_changed => [SFFloat, 0.0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterValue5_changed => [SFFloat, 0.0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterValue6_changed => [SFFloat, 0.0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		articulationParameterValue7_changed => [SFFloat, 0.0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		collideTime => [SFTime, 0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		detonateTime => [SFTime, 0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		firedTime => [SFTime, 0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isActive => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isCollided => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isDetonated => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isNetworkReader => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isNetworkWriter => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isRtpHeaderHeard => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isStandAlone => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		timestamp => [SFTime, 0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0,0,0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1,-1,-1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		rtpHeaderExpected => [SFBool, FALSE, initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	}, "X3DGroupingNode"),


	ReceiverPdu => new VRML::NodeType("ReceiverPdu", {
		address => [SFString, "localhost", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		applicationID => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entityID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		multicastRelayHost => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		multicastRelayPort => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		networkMode => [SFString, "standAlone", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		port => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radioID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		readInterval => [SFFloat, 0.1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		receivedPower => [SFFloat, 0.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		receiverState => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		rtpHeaderExpected => [SFBool, FALSE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		siteID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transmitterApplicationID => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transmitterEntityID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transmitterRadioID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transmitterSiteID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		whichGeometry => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		writeInterval => [SFFloat, 1.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isActive => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isNetworkReader => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isNetworkWriter => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isRtpHeaderHeard => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isStandAlone => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		timestamp => [SFTime, 0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0,0,0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1,-1,-1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	}, "X3DChildNode"),

	SignalPdu => new VRML::NodeType("SignalPdu", {
		address => [SFString, "localhost", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		applicationID => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		data => [MFInt32, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		dataLength => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		encodingScheme => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entityID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		multicastRelayHost => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		multicastRelayPort => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		networkMode => [SFString, "standAlone", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		port => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radioID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		readInterval => [SFFloat, 0.1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		rtpHeaderExpected => [SFBool, FALSE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		sampleRate => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		samples => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		siteID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		tdlType => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		whichGeometry => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		writeInterval => [SFFloat, 1.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isActive => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isNetworkReader => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isNetworkWriter => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isRtpHeaderHeard => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isStandAlone => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		timestamp => [SFTime, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0,0,0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1,-1,-1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	}, "X3DChildNode"),

	TransmitterPdu => new VRML::NodeType("TransmitterPdu", {
		address => [SFString, "localhost", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		antennaLocation => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		antennaPatternLength => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		antennaPatternType => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		applicationID => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		cryptoKeyID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		cryptoSystem => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		enabled => [SFBool, TRUE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		entityID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		frequency => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		inputSource => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		lengthOfModulationParameters => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		modulationTypeDetail => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		modulationTypeMajor => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		modulationTypeSpreadSpectrum => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		modulationTypeSystem => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		multicastRelayHost => [SFString, "", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		multicastRelayPort => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		networkMode => [SFString, "standAlone", inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		port => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		power => [SFFloat, 0.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radioEntityTypeCategory => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radioEntityTypeCountry => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radioEntityTypeDomain => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radioEntityTypeKind => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radioEntityTypeNomenclature => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radioEntityTypeNomenclatureVersion => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		radioID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		readInterval => [SFFloat, 0.1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		relativeAntennaLocation => [SFVec3f, [0,0,0], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		rtpHeaderExpected => [SFBool, FALSE, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		siteID => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transmitFrequencyBandwidth => [SFFloat, 0.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		transmitState => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		whichGeometry => [SFInt32, 1, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		writeInterval => [SFFloat, 1.0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isActive => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isNetworkReader => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isNetworkWriter => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isRtpHeaderHeard => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		isStandAlone => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		timestamp => [SFTime, 0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0,0,0], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1,-1,-1], initializeOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
	}, "X3DChildNode"),





	###################################################################################

	#		Scripting Component

	###################################################################################
	Script =>
	new VRML::NodeType("Script",
					   {
			url => [MFString, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			directOutput => [SFBool, FALSE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			mustEvaluate => [SFBool, FALSE, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			 __scriptObj => [FreeWRLPTR, 0, initializeOnly, 0],
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
					   },"X3DScriptNode"
					  ),

	###################################################################################

	#		EventUtilities Component

	###################################################################################

	BooleanFilter => 
	new VRML::NodeType("BooleanFilter", {
			set_boolean =>[SFBool,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			inputFalse => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			inputNegate => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			inputTrue => [SFBool, TRUE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DChildNode"),


	BooleanSequencer => 
	new VRML::NodeType("BooleanSequencer", {
			next =>[SFBool,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			previous =>[SFBool,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			set_fraction =>[SFFloat,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			key => [MFFloat, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			keyValue => [MFBool, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value_changed => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DSequencerNode"),


	BooleanToggle => 
	new VRML::NodeType("BooleanToggle", {
			set_boolean =>[SFBool,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			toggle => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DChildNode"),


	BooleanTrigger => 
	new VRML::NodeType("BooleanTrigger", {
			set_triggerTime => [SFTime,undef ,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			triggerTrue => [SFBool, FALSE, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DTriggerNode"),


	IntegerSequencer => 
	new VRML::NodeType("IntegerSequencer", {
			next =>[SFBool,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			previous =>[SFBool,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			set_fraction =>[SFFloat,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			key => [MFFloat, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			keyValue => [MFInt32, [], inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value_changed => [SFInt32, 0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DSequencerNode"),

	IntegerTrigger => 
	new VRML::NodeType("IntegerTrigger", {
			set_triggerTime => [SFTime,undef ,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			integerKey => [SFInt32, 0, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			triggerValue => [SFInt32, 0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DTriggerNode"),

	TimeTrigger => 
	new VRML::NodeType("TimeTrigger", {
			set_boolean =>[SFBool,undef,inputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			triggerTime => [SFTime, 0, outputOnly, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DTriggerNode"),


	###################################################################################

	#		EventUtilities Component

	###################################################################################

	ComposedShader => new VRML::NodeType("ComposedShader", {
			activate =>[SFBool,undef,inputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			parts => [MFNode,[],inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isSelected => [SFBool, TRUE,outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isValid => [SFBool, TRUE,outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			language => [SFString, "", initializeOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__initialized => [SFBool, FALSE ,initializeOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__shaderIDS => [MFInt32, [], initializeOnly, 0], 
			 __shaderObj => [FreeWRLPTR, 0, initializeOnly, 0],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	},"X3DShaderNode"),

	FloatVertexAttribute => new VRML::NodeType("FloatVertexAttribute", {
			value => [MFFloat,[],inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString,"",inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			numComponents => [SFInt32, 4, initializeOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # 1...4 valid values
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	}, "X3DVertexAttributeNode"),

	Matrix3VertexAttribute => new VRML::NodeType("Matrix3VertexAttribute", {
			value => [MFMatrix3f,[],inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString,"",inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	}, "X3DVertexAttributeNode"),

	Matrix4VertexAttribute => new VRML::NodeType("Matrix4VertexAttribute", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			value => [MFMatrix4f,[],inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			name => [SFString,"",inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
	}, "X3DVertexAttributeNode"),

	PackagedShader => new VRML::NodeType("PackagedShader", {
			activate =>[SFBool,undef,inputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			url => [MFString, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isSelected => [SFBool, TRUE,outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isValid => [SFBool, TRUE,outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			language => [SFString,"",initializeOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			 __shaderObj => [FreeWRLPTR, 0, initializeOnly, 0],
	}, "X3DProgrammableShaderObject"),

	ProgramShader => new VRML::NodeType("ProgramShader", {
			activate =>[SFBool,undef,inputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			programs => [MFNode, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isSelected => [SFBool, TRUE,outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			isValid => [SFBool, TRUE,outputOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			language => [SFString,"",initializeOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__initialized => [SFBool, FALSE ,initializeOnly, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
			__shaderIDS => [MFInt32, [], initializeOnly, 0], 
	}, "X3DProgrammableShaderObject"),

	ShaderPart => new VRML::NodeType("ShaderPart", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			url => [MFString, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			type => [SFString,"VERTEX",inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
	}, "X3DUrlObject"),

	ShaderProgram => new VRML::NodeType("ShaderProgram", {
			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			url => [MFString, [], inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			type => [SFString,"",inputOutput, "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # see note top of file
			 __shaderObj => [FreeWRLPTR, 0, initializeOnly, 0],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
	}, "X3DUrlObject"),


	###################################################################################

	# Metadata nodes ...

	###################################################################################



	#used mainly for PROTO invocation parameters 
	MetadataSFFloat => new VRML::NodeType("MetadataSFFloat", { 
			value => [SFFloat,0.0,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFFloat,0.0,outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFFloat,0.0,inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFFloat => new VRML::NodeType("MetadataMFFloat", { 
			value => [MFFloat,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFFloat,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFFloat,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFRotation => new VRML::NodeType("MetadataSFRotation", { 
			value => [SFRotation,[0,0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFRotation,[0,0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFRotation,[0,0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFRotation => new VRML::NodeType("MetadataMFRotation", { 
			value => [MFRotation,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFRotation,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFRotation,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec3f => new VRML::NodeType("MetadataSFVec3f", { 
			value => [SFVec3f,[0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFVec3f,[0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFVec3f,[0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec3f => new VRML::NodeType("MetadataMFVec3f", { 
			value => [MFVec3f,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFVec3f,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFVec3f,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFBool => new VRML::NodeType("MetadataSFBool", { 
			value => [SFBool,FALSE,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFBool,FALSE,outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFBool,FALSE,inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFBool => new VRML::NodeType("MetadataMFBool", { 
			value => [MFBool,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFBool,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFBool,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFInt32 => new VRML::NodeType("MetadataSFInt32", { 
			value => [SFInt32,0,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFInt32,0,outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFInt32,0,inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFInt32 => new VRML::NodeType("MetadataMFInt32", { 
			value => [MFInt32,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFInt32,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFInt32,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFNode => new VRML::NodeType("MetadataSFNode", { 
			value => [SFNode,0,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFNode,0,outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFNode,0,inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFNode => new VRML::NodeType("MetadataMFNode", { 
			value => [MFNode,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFNode,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFNode,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFColor => new VRML::NodeType("MetadataSFColor", { 
			value => [SFColor,[0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFColor,[0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFColor,[0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFColor => new VRML::NodeType("MetadataMFColor", { 
			value => [MFColor,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFColor,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFColor,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFColorRGBA => new VRML::NodeType("MetadataSFColorRGBA", { 
			value => [SFColorRGBA,[0,0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFColorRGBA,[0,0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFColorRGBA,[0,0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFColorRGBA => new VRML::NodeType("MetadataMFColorRGBA", { 
			value => [MFColorRGBA,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFColorRGBA,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFColorRGBA,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFTime => new VRML::NodeType("MetadataSFTime", { 
			value => [SFTime,0,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFTime,0,outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFTime,0,inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFTime => new VRML::NodeType("MetadataMFTime", { 
			value => [MFTime,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFTime,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFTime,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFString => new VRML::NodeType("MetadataSFString", { 
			value => [SFString,"",inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFString,"",outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFString,"",inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFString => new VRML::NodeType("MetadataMFString", { 
			value => [MFString,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFString,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFString,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec2f => new VRML::NodeType("MetadataSFVec2f", { 
			value => [SFVec2f,[0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFVec2f,[0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFVec2f,[0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec2f => new VRML::NodeType("MetadataMFVec2f", { 
			value => [MFVec2f,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFVec2f,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFVec2f,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFImage => new VRML::NodeType("MetadataSFImage", { 
			value => [SFImage,[0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFImage,[0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFImage,[0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec3d => new VRML::NodeType("MetadataSFVec3d", { 
			value => [SFVec3d,[0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFVec3d,[0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFVec3d,[0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec3d => new VRML::NodeType("MetadataMFVec3d", { 
			value => [MFVec3d,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFVec3d,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFVec3d,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFDouble => new VRML::NodeType("MetadataSFDouble", { 
			value => [SFDouble,0,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFDouble,0,outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFDouble,0,inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFDouble => new VRML::NodeType("MetadataMFDouble", { 
			value => [MFDouble,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFDouble,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFDouble,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFMatrix3f => new VRML::NodeType("MetadataSFMatrix3f", { 
			value => [SFMatrix3f,[0,0,0,0,0,0,0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFMatrix3f,[0,0,0,0,0,0,0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFMatrix3f,[0,0,0,0,0,0,0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFMatrix3f => new VRML::NodeType("MetadataMFMatrix3f", { 
			value => [MFMatrix3f,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFMatrix3f,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFMatrix3f,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFMatrix3d => new VRML::NodeType("MetadataSFMatrix3d", { 
			value => [SFMatrix3d,[0,0,0,0,0,0,0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFMatrix3d,[0,0,0,0,0,0,0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFMatrix3d,[0,0,0,0,0,0,0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFMatrix3d => new VRML::NodeType("MetadataMFMatrix3d", { 
			value => [MFMatrix3d,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFMatrix3d,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFMatrix3d,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFMatrix4f => new VRML::NodeType("MetadataSFMatrix4f", { 
			value => [SFMatrix4f,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFMatrix4f,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFMatrix4f,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFMatrix4f => new VRML::NodeType("MetadataMFMatrix4f", { 
			value => [MFMatrix4f,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFMatrix4f,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFMatrix4f,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFMatrix4d => new VRML::NodeType("MetadataSFMatrix4d", { 
			value => [SFMatrix4d,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFMatrix4d,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFMatrix4d,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFMatrix4d => new VRML::NodeType("MetadataMFMatrix4d", { 
			value => [MFMatrix4d,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFMatrix4d,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFMatrix4d,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec2d => new VRML::NodeType("MetadataSFVec2d", { 
			value => [SFVec2d,[0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFVec2d,[0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFVec2d,[0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec2d => new VRML::NodeType("MetadataMFVec2d", { 
			value => [MFVec2d,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFVec2d,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFVec2d,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec4f => new VRML::NodeType("MetadataSFVec4f", { 
			value => [SFVec4f,[0,0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFVec4f,[0,0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFVec4f,[0,0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec4f => new VRML::NodeType("MetadataMFVec4f", { 
			value => [MFVec4f,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFVec4f,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFVec4f,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataSFVec4d => new VRML::NodeType("MetadataSFVec4d", { 
			value => [SFVec4d,[0,0,0,0],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[SFVec4d,[0,0,0,0],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[SFVec4d,[0,0,0,0],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),

	#used mainly for PROTO invocation parameters 
	MetadataMFVec4d => new VRML::NodeType("MetadataMFVec4d", { 
			value => [MFVec4d,[],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			valueChanged=>[MFVec4d,[],outputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			setValue =>[MFVec4d,[],inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], 
			tickTime=>[SFTime,0,inputOnly,0],
	}, "X3DChildNode"),


	###################################################################################
	
	# VRML 1

	###################################################################################


	VRML1_AsciiText => new VRML::NodeType("VRML1_AsciiText", {
		string =>[MFString,[],inputOutput, "SPEC_VRML1"], 
		spacing => [SFFloat,1.0,inputOutput,"SPEC_VRML1"],
		justification => [SFString,"LEFT",inputOutput,"SPEC_VRML1"],
		width => [MFFloat,[0],inputOutput,"SPEC_VRML1"],
		_ILS => [SFNode, NULL, inputOutput, 0],
	}, "X3DChildNode"),

	VRML1_Cone => new VRML::NodeType("VRML1_Cone", {
		parts =>[SFString,"ALL",inputOutput,"SPEC_VRML1"],
		bottomRadius => [SFFloat,1.0,inputOutput,"SPEC_VRML1"],
		height => [SFFloat,2.0,inputOutput,"SPEC_VRML1"],
		_ILS => [SFNode, NULL, inputOutput, 0],
	}, "X3DChildNode"),


	VRML1_Cube => new VRML::NodeType("VRML1_Cube", {
		width => [SFFloat,2.0,inputOutput,"SPEC_VRML1"],
		height => [SFFloat,2.0,inputOutput,"SPEC_VRML1"],
		depth => [SFFloat,2.0,inputOutput,"SPEC_VRML1"],
		_ILS => [SFNode, NULL, inputOutput, 0],
	}, "X3DChildNode"),

	VRML1_Cylinder => new VRML::NodeType("VRML1_Cylinder", {
		parts =>[SFString,"ALL",inputOutput,"SPEC_VRML1"],
		radius => [SFFloat,1.0,inputOutput,"SPEC_VRML1"],
		height => [SFFloat,2.0,inputOutput,"SPEC_VRML1"],
		_ILS => [SFNode, NULL, inputOutput, 0],
	}, "X3DChildNode"),

	VRML1_IndexedFaceSet => new VRML::NodeType("VRML1_IndexedFaceSet", {
		coordIndex => [MFInt32, [0],inputOutput,"SPEC_VRML1"],
		materialIndex => [MFInt32,[-1],inputOutput,"SPEC_VRML1"],
		normalIndex => [MFInt32,[-1],inputOutput,"SPEC_VRML1"],
		textureCoordIndex =>[MFInt32,[-1],inputOutput,"SPEC_VRML1"],
		_color => [SFNode, NULL, inputOutput, 0],
		_coord => [SFNode, NULL, inputOutput, 0],
		_normal => [SFNode, NULL, inputOutput, 0],
		_texCoord => [SFNode, NULL, inputOutput, 0],
		_ccw => [SFBool, TRUE, initializeOnly, 0],
		_convex => [SFBool, TRUE, initializeOnly, 0],
		_creaseAngle => [SFFloat, 0, initializeOnly, 0],
		_npv => [SFBool, TRUE, initializeOnly, 0],
		_cpv => [SFBool, TRUE, initializeOnly, 0],
		_solid => [SFBool, TRUE, initializeOnly, 0],

	}, "X3DChildNode"),

	VRML1_IndexedLineSet => new VRML::NodeType("VRML1_IndexedLineSet", {
		coordIndex => [MFInt32, [0],inputOutput,"SPEC_VRML1"],
		materialIndex => [MFInt32,[-1],inputOutput,"SPEC_VRML1"],
		normalIndex => [MFInt32,[-1],inputOutput,"SPEC_VRML1"],
		textureCoordIndex =>[MFInt32,[-1],inputOutput,"SPEC_VRML1"],
		_ILS => [SFNode, NULL, inputOutput, 0],
	}, "X3DChildNode"),

	VRML1_PointSet => new VRML::NodeType("VRML1_PointSet", {
		startIndex => [SFInt32, 0,inputOutput,"SPEC_VRML1"],
		numPoints => [SFInt32,-1,inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),


	VRML1_Sphere => new VRML::NodeType("VRML1_Sphere", {
		radius => [SFFloat,1.0,inputOutput,"SPEC_VRML1"],
		_ILS => [SFNode, NULL, inputOutput, 0],
	}, "X3DChildNode"),

	VRML1_Coordinate3 => new VRML::NodeType("VRML1_Coordinate3", {
		point => [MFVec3f,[[0,0,0]],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_FontStyle => new VRML::NodeType("VRML1_FontStyle", {
		size => [SFFloat,10,inputOutput,"SPEC_VRML1"],
		family => [SFString,"SERIF",inputOutput,"SPEC_VRML1"],
		style => [SFString,"NONE",inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_Info => new VRML::NodeType("VRML1_Info", {
		string => [SFString,"",inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_Material => new VRML::NodeType("VRML1_Material", {
		ambientColor => [MFColor,[[0.2, 0.2, 0.2]],inputOutput,"SPEC_VRML1"],
		diffuseColor => [MFColor,[[0.8, 0.8, 0.8]],inputOutput,"SPEC_VRML1"],
		specularColor => [MFColor,[[0, 0, 0]],inputOutput,"SPEC_VRML1"],
		emissiveColor => [MFColor,[[0, 0, 0]],inputOutput,"SPEC_VRML1"],
		shininess => [MFFloat,0.2,inputOutput,"SPEC_VRML1"],
		transparency =>[MFFloat,[0],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_MaterialBinding => new VRML::NodeType("VRML1_MaterialBinding", {
		value => [SFString,"OVERALL",inputOutput,"SPEC_VRML1"],
		_initialized => [SFInt32,FALSE,inputOutput, 0],
		_Value =>[SFInt32,-1,inputOutput, 0],
	}, "X3DChildNode"),

	VRML1_Normal => new VRML::NodeType("VRML1_Normal", {
		vector => [MFVec3f,[],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_NormalBinding => new VRML::NodeType("VRML1_NormalBinding", {
		value => [SFString,"DEFAULT",inputOutput,"SPEC_VRML1"],
		_initialized => [SFInt32,FALSE,inputOutput, 0],
		_Value =>[SFInt32,-1,inputOutput,0],
	}, "X3DChildNode"),

	VRML1_Texture2 => new VRML::NodeType("VRML1_Texture2", {
		filename => [MFString,[],inputOutput,"SPEC_VRML1"], #MFString, fits in to X3D code better.
		image => [SFImage,[0,0,0],inputOutput,"SPEC_VRML1"],
		wrapS => [SFString,"REPEAT",inputOutput,"SPEC_VRML1"],
		wrapT => [SFString,"REPEAT",inputOutput,"SPEC_VRML1"],
		__textureTableIndex => [SFInt32, 0, initializeOnly, 0],
		_parentResource =>[FreeWRLPTR,0,initializeOnly, 0],
		_initialized => [SFInt32,FALSE,inputOutput, 0],
		_wrapS =>[SFInt32,-1,inputOutput,0],
		_wrapT =>[SFInt32,-1,inputOutput,0],
	}, "X3DChildNode"),

	VRML1_Texture2Transform => new VRML::NodeType("VRML1_Texture2Transform", {
		translation => [SFVec2f,[0,0],inputOutput,"SPEC_VRML1"],
		rotation => [SFFloat,0,inputOutput,"SPEC_VRML1"],
		scaleFactor=>[SFVec2f,[1,1],inputOutput,"SPEC_VRML1"],
		center=> [SFVec2f,[0,0],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_TextureCoordinate2 => new VRML::NodeType("VRML1_TextureCoordinate2", {
		point => [MFVec2f,[[0,0]],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_ShapeHints => new VRML::NodeType("VRML1_ShapeHints", {
		vertexOrdering => [SFString,"CLOCKWISE",inputOutput,"SPEC_VRML1"],
		shapeType => [SFString,"SOLID",inputOutput,"SPEC_VRML1"],
		faceType => [SFString,"CONVEX",inputOutput,"SPEC_VRML1"],
		creaseAngle => [SFDouble, 0.5, initializeOnly, "SPEC_VRML1"],

		_initialized => [SFInt32,FALSE,inputOutput, 0],
		_vertValue =>[SFInt32,0,inputOutput,0],
		_typeValue =>[SFInt32,0,inputOutput,0],
		_faceValue =>[SFInt32,0,inputOutput,0],
		
	}, "X3DChildNode"),

	VRML1_MatrixTransform => new VRML::NodeType("VRML1_MatrixTransform", {
		value => [SFMatrix4f,[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1],inputOutput, "SPEC_VRML1"], 
	}, "X3DChildNode"),

	VRML1_Rotation => new VRML::NodeType("VRML1_Rotation", {
		rotation => [SFRotation,[0,0,1,0],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_Scale => new VRML::NodeType("VRML1_Scale", {
		scaleFactor => [SFVec3f,[1,1,1],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_Transform => new VRML::NodeType("VRML1_Transform", {
		translation =>[SFVec3f,[0,0,0],inputOutput,"SPEC_VRML1"],
		rotation =>[SFRotation,[0,0,1,0],inputOutput,"SPEC_VRML1"],
		scaleFactor => [SFVec3f,[1,1,1],inputOutput,"SPEC_VRML1"],
		scaleOrientation =>[SFRotation,[0,0,1,0],inputOutput,"SPEC_VRML1"],
		center =>[SFVec3f,[0,0,0],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_Translation => new VRML::NodeType("VRML1_Translation", {
		translation =>[SFVec3f,[0,0,0],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_Separator => new VRML::NodeType("VRML1_Separator", {
		renderCulling => [SFString,"AUTO",inputOutput,"SPEC_VRML1"],
		VRML1children => [MFNode, [], inputOutput, "SPEC_VRML1"],
		_sortedChildren => [MFNode, [], inputOutput, 0],
	}, "X3DChildNode"),

	VRML1_Switch => new VRML::NodeType("VRML1_Switch", {
		whichChild=>[SFInt32,-1,inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_WWWAnchor => new VRML::NodeType("VRML1_WWWAnchor", {
		name=>[SFString,"",inputOutput,"SPEC_VRML1"],
		description=>[SFString,"",inputOutput,"SPEC_VRML1"],
		map=>[SFString,"",inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_LOD => new VRML::NodeType("VRML1_LOD", {
		range=>[MFFloat,[],inputOutput,"SPEC_VRML1"],
		center=>[SFVec3f,[0,0,0],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_OrthographicCamera => new VRML::NodeType("VRML1_OrthographicCamera", {
		position=>[SFVec3f,[0,0,1],inputOutput,"SPEC_VRML1"],
		orientation=>[SFRotation,[0,0,1,0],inputOutput,"SPEC_VRML1"],
		focalDistance=>[SFFloat,5,inputOutput,"SPEC_VRML1"],
		height=>[SFFloat,2,inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_PerspectiveCamera => new VRML::NodeType("VRML1_PerspectiveCamera", {
		position=>[SFVec3f,[0,0,1],inputOutput,"SPEC_VRML1"],
		orientation=>[SFRotation,[0,0,1,0],inputOutput,"SPEC_VRML1"],
		focalDistance=>[SFFloat,5,inputOutput,"SPEC_VRML1"],
		heightAngle=>[SFFloat,0.785398,inputOutput,"SPEC_VRML1"],
		
	}, "X3DChildNode"),

	VRML1_DirectionalLight => new VRML::NodeType("VRML1_DirectionalLight", {
		on=>[SFBool,TRUE,inputOutput,"SPEC_VRML1"],
		intensity=>[SFFloat,1,inputOutput,"SPEC_VRML1"],
		color=>[SFColor,[1,1,1],inputOutput,"SPEC_VRML1"],
		direction=>[SFVec3f,[0,0,-1],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_PointLight => new VRML::NodeType("VRML1_PointLight", {
		on=>[SFBool,TRUE,inputOutput,"SPEC_VRML1"],
		intensity=>[SFFloat,1,inputOutput,"SPEC_VRML1"],
		color=>[SFColor,[1,1,1],inputOutput,"SPEC_VRML1"],
		location=>[SFVec3f,[0,0,1],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_SpotLight => new VRML::NodeType("VRML1_SpotLight", {
		on=>[SFBool,TRUE,inputOutput,"SPEC_VRML1"],
		intensity=>[SFFloat,1,inputOutput,"SPEC_VRML1"],
		color=>[SFColor,[1,1,1],inputOutput,"SPEC_VRML1"],
		location=>[SFVec3f,[0,0,1],inputOutput,"SPEC_VRML1"],
		direction=>[SFVec3f,[0,0,-1],inputOutput,"SPEC_VRML1"],
		dropOffRate=>[SFFloat,0,inputOutput,"SPEC_VRML1"],
		cutOffAngle=>[SFFloat,0.785398,inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),

	VRML1_WWWInline => new VRML::NodeType("VRML1_WWWInline", {
		name=>[SFString,"",inputOutput,"SPEC_VRML1"],
		bboxSize=>[SFVec3f,[0,0,0],inputOutput,"SPEC_VRML1"],
		bboxCenter=>[SFVec3f,[0,0,0],inputOutput,"SPEC_VRML1"],
	}, "X3DChildNode"),



	###################################################################################

	# testing...

	###################################################################################

	# A PickableGroup node is an X3DGroupingNode that contains children that are marked
	# as being of a given classification of picking types, as well as the ability to enable or disable picking of the children.

# DJTRACK_PICKSENSORS
	PickableGroup => new VRML::NodeType("PickableGroup", {
		addChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		removeChildren => [MFNode, undef, inputOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		children => [MFNode, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		objectType => [MFString, ["ALL","NONE","TERRAIN"],inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		pickable => [SFBool, TRUE,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxCenter => [SFVec3f, [0, 0, 0], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		bboxSize => [SFVec3f, [-1, -1, -1], initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],

		__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro
		FreeWRL__protoDef => [SFInt32, INT_ID_UNDEFINED, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # tell renderer that this is a proto...
		FreeWRL_PROTOInterfaceNodes =>[MFNode, [], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		_sortedChildren => [MFNode, [], inputOutput, 0],
	},"X3DGroupingNode"),

	MidiControl =>
	new VRML::NodeType("MidiControl",
					{
			deviceName => [SFString,"",inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# "Subtractor 1"
			channel => [SFInt32,-1,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],		# channel in range 0-16, on MIDI bus
			controller => [SFString,"",inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# "Osc1 Wave"
			_deviceNameIndex => [SFInt32, -99, initializeOnly, 0],	#  name in name table index
			_controllerIndex => [SFInt32, -99, initializeOnly, 0],		#  name in name table index


			# encoded bus,channel,controller
			_bus => [SFInt32,-99,initializeOnly, 0],			# internal for efficiency
			_channel => [SFInt32,-99,initializeOnly, 0],			# internal for efficiency
			_controller => [SFInt32,-99,initializeOnly, 0],		# internal for efficiency

			deviceMinVal => [SFInt32, 0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],		# what the device sets
			deviceMaxVal => [SFInt32, 0, initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],		# what the device sets

			velocity => [SFInt32, 100, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# velocity field for buttonPress
									# controller types.
			_vel => [SFInt32, 100, initializeOnly, 0],			# internal copy of velocity
			_sentVel => [SFInt32, 100, initializeOnly, 0],		# send this velocity - if <0, noteOff

			minVal => [SFInt32, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],		# used to scale floats, and 
			maxVal => [SFInt32, 10000, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# bounds check ints. The resulting
									# value will be <= maxVal <= deviceMaxVal
									# and >=minVal >= deviceMinVal

			intValue => [SFInt32, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],		# integer value for i/o
			_oldintValue => [SFInt32, 0, initializeOnly, 0],		# old integer value for i/o
			floatValue => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# float value for i/o
			useIntValue => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# which value to use for input

			highResolution => [SFBool, TRUE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# high resolution controller
			controllerType => [SFString, "Slider", inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# "Slider" "ButtonPress"
			_intControllerType => [SFInt32,999, initializeOnly, 0], 	# use ReWire definitions
			controllerPresent => [SFBool, FALSE, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# TRUE when ReWire is working

			buttonPress => [SFBool,FALSE,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# is the key pressed when in "ButtonPress" mode?"
			_butPr => [SFBool,FALSE,inputOutput, 0],		# used to determine toggle state for buttonPress

			autoButtonPress => [SFBool,TRUE,inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],# send a NoteOn when the int/float 
									# value changes. if False, send only
									# when buttonPressed happens.
			pressLength => [SFFloat, 0.05, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],	# time before noteOff in AutoButtonPress mode.
			pressTime => [SFTime, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],		# when the press went in

			metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
			__oldmetadata => [SFNode, 0, inputOutput, 0], # see code for event macro

					}, "X3DNetworkSensorNode"
					),
); 


1;

