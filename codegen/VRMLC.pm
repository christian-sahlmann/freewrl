# $Id: VRMLC.pm,v 1.63 2011/10/11 17:53:58 crc_canada Exp $
#
# Copyright (C) 1998 Tuomas J. Lukka 1999 John Stewart CRC Canada
# Portions Copyright (C) 1998 Bernhard Reiter
# DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
# See the GNU Library General Public License (file COPYING in the distribution)
# for conditions of use and redistribution.

#
# $Log: VRMLC.pm,v $
# Revision 1.63  2011/10/11 17:53:58  crc_canada
# move the scenegraph Node parent structure to the Vector methodology.
#
# Revision 1.62  2011/06/11 01:14:25  couannette
# A mistake that prevented VRMLC.pm to generate file when it didn't exist.
#
# Revision 1.61  2011/06/04 19:05:42  crc_canada
# comment out MIDI code
#
# Revision 1.60  2011/03/23 18:26:02  crc_canada
# PolyReps now use GLuints for indicies; some compiler warnings removed
#
# Revision 1.59  2010/12/10 17:17:19  davejoubert
# Add OSC capability to FreeWRL. This update is spread across several files,
# but the two post important changed are in codegen/VRMLNodes.pm and
# src/lib/scenegraph/Component_Networking.c
# Modified Files:
# 	configure.ac codegen/VRMLC.pm codegen/VRMLNodes.pm
# 	codegen/VRMLRend.pm src/lib/Makefile.am
# 	src/lib/Makefile.sources src/lib/main/MainLoop.c
# 	src/lib/main/MainLoop.h
# 	src/lib/scenegraph/Component_Networking.c
# 	src/lib/scenegraph/Component_Networking.h
# 	src/lib/scenegraph/GeneratedCode.c
# 	src/lib/vrml_parser/NodeFields.h src/lib/vrml_parser/Structs.h
# 	src/lib/world_script/jsUtils.c src/libeai/GeneratedCode.c
# Added Files:
# 	src/lib/scenegraph/OSCcallbacks.c src/lib/scenegraph/ringbuf.c
# 	src/lib/scenegraph/ringbuf.h
#
# Revision 1.58  2010/12/07 18:27:49  crc_canada
# MALLOC changes;
# some hidden fields now have real types, not FreeWRLPTR;
# SFVec3f data type made.
#
# Revision 1.57  2010/12/03 19:55:21  crc_canada
# changing from "void *" to more specific types.
#
# Revision 1.56  2010/10/13 19:23:47  crc_canada
# X3D_Virt functions; make a table rather than keeping a list of pointers in EACH scenegraph node.
#
# Revision 1.55  2010/10/12 20:01:54  istakenv
# Fixed implicit declaration of add_picksensor in GeneratedCode.c
#
# Revision 1.55
# istakenv - codegeneration - added scenegraph/Component_Picking.h to GeneratedCode.c so that 'add_picksensor()' is not defined implicitly
#
# Revision 1.54  2010/10/12 00:34:12  dug9
# dug9 - codegeneration - added *other() virtual function, and assigned pointpicksensor, pickablegroup and sphere to implement it, put stubs for these other() functions for those that don't implement it.
#
# Revision 1.53  2010/10/11 20:47:55  dug9
# dug9 for Component_picking added X3D_POINTPICKSENSOR() macro to Structs.h
#
# Revision 1.52  2010/10/02 14:45:45  dug9
# dug9 touchups to get msvc to compile
#
# Revision 1.51  2010/10/02 06:42:25  davejoubert
# Pickables. Scan for DJTRACK_PICKSENSORS
# Modified Files:
#  	codegen/VRMLC.pm codegen/VRMLNodes.pm codegen/VRMLRend.pm
#  	src/lib/main/MainLoop.c src/lib/main/headers.h
#  	src/lib/scenegraph/Component_Geometry3D.c
#  	src/lib/scenegraph/Component_Grouping.c
#  	src/lib/scenegraph/Component_Shape.c
#  	src/lib/scenegraph/GeneratedCode.c
#  	src/lib/vrml_parser/CRoutes.c
#
# Revision 1.49  2010/09/30 18:58:19  davejoubert
# Changes to make dump_scene function in GeneratedCode.c
# accessible to the outside world via the EAI.
# Modified Files:
#  	codegen/VRMLC.pm src/lib/input/EAIEventsIn.c
#  	src/lib/input/EAIHeaders.h src/lib/main/MainLoop.c
#  	src/lib/main/ProdCon.c
#  	src/lib/scenegraph/Component_Navigation.c
#  	src/lib/scenegraph/GeneratedCode.c
#  	src/lib/vrml_parser/NodeFields.h src/lib/vrml_parser/Structs.h
#  	src/libeai/GeneratedCode.c
#
# Revision 1.49  2010/10/29 18:39:00  davejoubert
# Made dump_scene filedescriptor friendly
# This allows it to be called from the EAI via a new DUMPSCENE verb
#
# Revision 1.48  2010/09/16 18:32:58  crc_canada
# finish removing of "changed_" routines.
#
# Revision 1.47  2010/08/31 15:45:34  crc_canada
# 1) sortChildren - copy children to internal children field ONLY when required to reduce sorting.
# 2) rootNode now an X3D_Group, not a void *
#
# Revision 1.46  2010/08/19 02:05:37  crc_canada
# Work making Anchor load new files, including loading correct viewpoints.
#
# Revision 1.45  2010/08/10 21:15:59  crc_canada
# ImageCubeMapTexture - now works - loads png, jpg, etc, NOT DDS yet.
#
# Revision 1.44  2010/07/05 15:07:08  crc_canada
# add code to ensure that bound Backgrounds, Fog gets rendered when OcclusionCulling enabled, and nodes are within Transforms.
#
# Revision 1.43  2010/06/29 22:13:36  davejoubert
# Implement PickableGroup
# Modified Files:
# 	codegen/VRMLC.pm codegen/VRMLNodes.pm codegen/VRMLRend.pm
# 	src/lib/internal.h src/lib/input/SensInterps.h
# 	src/lib/scenegraph/Component_Grouping.c
# 	src/lib/scenegraph/GeneratedCode.c
# 	src/lib/vrml_parser/NodeFields.h src/lib/vrml_parser/Structs.h
# 	src/lib/world_script/fieldSet.c src/libeai/GeneratedCode.c
#
# Revision 1.42  2010/06/29 16:59:44  crc_canada
# Initial VBO work.
#
# Revision 1.41  2010/04/14 19:03:32  crc_canada
# backing out OpenGL-ES 2.0 changes.
#
# Revision 1.40  2010/04/03 20:11:05  crc_canada
# More shader work for OpenGL-ES shaders.
#
# Revision 1.39  2010/03/10 21:29:52  sdumoulin
# More IPHONE compile updates
#
# Revision 1.38  2010/03/01 22:39:48  crc_canada
# more 64 bit changes - note that all "integer" data is now ints, not a combo of size_t, unsigned int, etc, just INT.
#
# Revision 1.37  2010/02/26 21:47:50  crc_canada
# 64 bit conversion changes.
#
# Revision 1.36  2010/02/17 14:31:08  crc_canada
# More 64/32 bit compile changes. Major one - getInputResource fixed.
#
# Revision 1.35  2010/02/16 21:21:47  crc_canada
# more changes for 64 bit computing warning errors.
#
# Revision 1.34  2010/02/16 13:54:45  crc_canada
# more 64 bit compiler warnings removal. Mainly items that will be of no operational
# impact for FreeWRL.
#
# Revision 1.33  2009/12/28 15:57:46  crc_canada
# TextureProperties node now active.
#
# Revision 1.32  2009/12/18 17:30:07  istakenv
# fixed implicit declarations
#
# Revision 1.31  2009/12/09 22:19:11  crc_canada
# Anchor - load URLs, add reference count for nodes, start worrying about disposing of nodes
#
# Revision 1.30  2009/11/18 08:47:09  couannette
# Fix dump_scene generation. Activated with FW_DEBUG.
#
# Revision 1.29  2009/11/17 20:49:27  crc_canada
# Change code to determine routing length
#
# Revision 1.28  2009/10/26 09:07:07  couannette
# Fix a bizarre include of EAI_C.h
#
# Revision 1.27  2009/10/26 08:03:34  couannette
# First set of modifications (configure build, main program).
#
# Revision 1.26  2009/10/22 20:47:51  sdumoulin
# Fix for adding EAI_C.h
#
# Revision 1.25  2009/10/12 12:57:06  couannette
# Small improvements:
# - created a sub for the license block, together with a file in "templates"
# - checked for read-only files, used 'cvs edit' to make them writable
#
# Revision 1.24  2009/10/05 15:07:23  crc_canada
# changed license text in files to better indicate LGPL
#
# Revision 1.23  2009/10/01 19:35:36  crc_canada
# License update to LGPL in all FreeWRL specific files.
#
# Revision 1.22  2009/09/16 22:48:24  couannette
# Changed EAIheaders.h to EAIHeaders.h. And removed that include each time it was not required.
# To enable (not working) MB textures, compile with -DTEXTURES_MB.
#
# Revision 1.21  2009/08/25 19:53:28  crc_canada
# more XML PROTO parsing/routing. Still needs routing to/from PROTOS.
#
# Revision 1.20  2009/08/12 17:22:54  crc_canada
# Moving defines out of headers.h and into perl-generated code.
#
# Revision 1.19  2009/07/22 14:36:19  crc_canada
# make shadow copy of children fields in nodes that need to sort nodes; this keeps the node list in original order entry.
#
# Revision 1.18  2009/06/26 16:44:34  crc_canada
# put text in libFreeWRLEAI header files to indicate that they are part of the FreeWRL project.
#
# Revision 1.17  2009/06/22 19:40:41  crc_canada
# more VRML1 work.
#
# Revision 1.16  2009/06/19 16:21:44  crc_canada
# VRML1 work.
#
# Revision 1.15  2009/06/17 15:05:24  crc_canada
# VRML1 nodes added to build process.
#
# Revision 1.14  2009/06/12 20:13:00  crc_canada
# Verifying Triangle nodes.
#
# Revision 1.13  2009/05/13 20:30:49  crc_canada
# Shader fields in classic parser worked on, and javascript sfvec4f and sfvec4d support.
#
# Revision 1.12  2009/05/11 21:11:58  crc_canada
# local/global lighting rules applied to SpotLight, DirectionalLight and PointLight.
#
# Revision 1.11  2009/05/07 20:03:20  crc_canada
# Move node sorting out of scene graph traversal
#
# Revision 1.10  2009/05/07 17:01:24  crc_canada
# compile cleanup - verifying and removing warnings.
#
# Revision 1.9  2009/05/06 20:35:46  crc_canada
# Modify SFColorRGBA and SFRotation to have array named c, not r for ease of code generation
#
# Revision 1.8  2009/04/29 20:31:37  crc_canada
# remove the writing of libeai/X3DHeader.h
#
# Revision 1.7  2009/04/29 20:20:25  crc_canada
# Check X3D version and node fields, if running in strict_parsing mode.
#
# Revision 1.6  2009/04/03 18:21:57  crc_canada
# PROTO handling of Multi_ types
#
# Revision 1.5  2009/04/02 18:48:28  crc_canada
# PROTO Routing for MFNodes.
#
# Revision 1.4  2009/03/13 20:07:16  crc_canada
# More PROTO work in classic parser.
#
# Revision 1.3  2009/03/10 21:00:34  crc_canada
# checking in some ongoing PROTO support work in the Classic parser.
#
# Revision 1.2  2009/03/05 22:11:49  istakenv
# Updated code generators to act upon the right files in the new source tree.  WARNING, does not produce identical results, use at own risk
#
# Revision 1.1  2009/03/05 21:33:39  istakenv
# Added code-generator perl scripts to new freewrl tree.  Initial commit, still need to patch them to make them work.
#
# Revision 1.310  2009/03/03 16:59:14  crc_canada
# Metadata fields now verified to be in every X3D node.
#
# Revision 1.309  2009/02/02 16:25:55  crc_canada
# add mode defines
#
# Revision 1.308  2009/01/29 16:01:21  crc_canada
# more node definitions.
#
# Revision 1.307  2008/12/12 20:29:12  crc_canada
# change generated file contents (BUT NOT location) for freex3d compile
# (files need copying over to final location by hand)
#
# Revision 1.306  2008/10/27 20:29:22  crc_canada
# FaceSets with color field, and node transparency being worked on.
#
# Revision 1.305  2008/10/23 16:19:29  crc_canada
# More shader work.
#
# Revision 1.304  2008/09/22 16:06:47  crc_canada
# all fieldtypes now defined in freewrl code; some not parsed yet, though, as there are no supported
# nodes that use them.
#
# Revision 1.303  2008/09/05 17:46:49  crc_canada
# reduce warnings counts when compiled with warnings=all
#
# Revision 1.302  2008/08/27 19:24:45  crc_canada
# Occlusion culling with DEF/USE shapes reworked.
#
# Revision 1.301  2008/08/18 14:45:38  crc_canada
# Billboard node Scene Graph changes.
#
# Revision 1.300  2008/08/14 05:02:31  crc_canada
# Bindable threading issues, continued; EXAMINE mode default rotation distance, continued; LOD improvements.
#
# Revision 1.299  2008/08/12 00:23:04  crc_canada
# Switch mode X3D parsing changes.
#
# Revision 1.298  2008/08/11 22:51:53  crc_canada
# Viewpoint binding problem (threading) and general comment cleanup
#
# Revision 1.297  2008/08/07 18:59:20  crc_canada
# be more lax on MF number parsing with the SCANTONUMBER macro.
#
# Revision 1.296  2008/07/30 18:08:34  crc_canada
# GeoLOD, July 30 changes.
#
# Revision 1.295  2008/07/21 20:13:13  crc_canada
# For 1.20.5
#
# Revision 1.294  2008/07/21 16:19:59  crc_canada
# July 19 2008 Geospatial updates
#
# Revision 1.293  2008/07/11 19:28:44  crc_canada
# GeoElevationGrid node changes.
#
# Revision 1.292  2008/07/08 16:30:24  crc_canada
# more GeoElevationGrid changes.
#
# Revision 1.291  2008/07/04 18:19:44  crc_canada
# GeoPositionInterpolator, and start on GeoElevationGrid
#
# Revision 1.290  2008/07/03 20:01:29  crc_canada
# GeoCoordinate work.
#
# Revision 1.289  2008/06/24 19:37:47  crc_canada
# Geospatial, June 24 2008 checkin
#
# Revision 1.288  2008/06/13 18:33:21  crc_canada
# float/double parsing with plus signs in exponents.
#
# Revision 1.287  2008/06/13 13:50:47  crc_canada
# Geospatial, SF/MFVec3d support.
#
# Revision 1.286  2008/05/21 20:16:48  crc_canada
# Standard defines for node rebuilding
#
# Revision 1.285  2008/03/31 20:10:16  crc_canada
# Review texture transparency, use node table to update scenegraph to allow for
# node updating.
#
# Revision 1.284  2008/01/24 18:33:13  crc_canada
# Rendering speedups on large worlds via improved collision routines
#
# Revision 1.283  2007/12/13 20:12:52  crc_canada
# KeySensor and StringSensor
#
# Revision 1.282  2007/12/13 14:54:13  crc_canada
# code cleanup and change to inputOnly, outputOnly, initializeOnly, inputOutput
# ----------------------------------------------------------------------
#
# Revision 1.281  2007/12/12 23:24:58  crc_canada
# X3DParser work
#
# Revision 1.280  2007/12/10 19:13:53  crc_canada
# Add parsing for x3dv COMPONENT, EXPORT, IMPORT, META, PROFILE
#
# Revision 1.279  2007/12/06 21:50:57  crc_canada
# Javascript X3D initializers
#
# Revision 1.278  2007/11/06 20:25:27  crc_canada
# Lighting revisited - pointlights and spotlights should all now work ok
#
# Revision 1.277  2007/10/26 16:35:38  crc_canada
# XML encoded parsing changes
#
# Revision 1.276  2007/10/18 20:09:59  crc_canada
# changes that affect warnings on compiling.
#
# Revision 1.275  2007/09/21 17:55:55  crc_canada
# Routing changes - fixing a bug with MF* nodes (was fixed recently, but changes removed by accident)
#
# Revision 1.274  2007/08/24 15:13:42  crc_canada
# Sensitive nodes now are dynamically pruned, and, if disabled, do not
# change cursor shape
#
# Revision 1.273  2007/08/13 18:05:34  sdumoulin
# Fixed nested protos
#
# Revision 1.272  2007/08/10 18:03:20  sdumoulin
# Added function to parse a scene graph in search for a specific node in order to create an equivalent offset pointer in a protoexpansionn
#
# Revision 1.271  2007/07/24 14:30:22  crc_canada
# add scene-graph dump (debug) and fix snapshot parameters
#
# Revision 1.270  2007/05/04 15:12:39  sdumoulin
# Removed status bar code
#
# Revision 1.269  2007/03/21 18:07:34  dtrembla
# Free memory on scene graph
#
# Revision 1.268  2007/03/20 20:36:10  crc_canada
# MALLOC/REALLOC macros to check mallocs for errors.
#
# Revision 1.267  2007/02/27 13:32:14  crc_canada
# initialize inputOnly fields to a zero value.
#
# Revision 1.266  2007/02/22 13:41:09  crc_canada
# more ReWire work
#
# Revision 1.265  2007/02/13 22:45:24  crc_canada
# PixelTexture default image should now be ok
#
# Revision 1.264  2007/02/12 15:18:16  crc_canada
# ReWire work.
#
# Revision 1.263  2007/02/09 20:43:51  crc_canada
# More ReWire work.
#
# Revision 1.262  2007/02/08 21:49:37  crc_canada
# added Statusbar for OS X safari
#
# Revision 1.261  2007/02/07 16:03:44  crc_canada
# ReWire work
#
# Revision 1.260  2007/01/30 18:16:39  crc_canada
# PROTO transparent material; some EAI changes.
#
# Revision 1.259  2007/01/22 18:44:51  crc_canada
# EAI conversion from "Ascii" type to internal FIELDTYPE happens as close to the
# EAI interface as possible.
#
# Revision 1.258  2007/01/19 21:58:55  crc_canada
# Initial for 1.18.13 - changing internal types to simplify the numbers of changes.
#
# Revision 1.257  2007/01/18 18:06:34  crc_canada
# X3D Parser should parse Scripts now.
#
# Revision 1.256  2007/01/17 21:29:28  crc_canada
# more X3D XML parsing work.
#
# Revision 1.255  2007/01/13 18:17:10  crc_canada
# Makes ordering of fields in Uni_String same as other Multi_ fields
#
# Revision 1.254  2007/01/11 21:07:46  crc_canada
# X3D Parser work.
#
# Revision 1.253  2007/01/10 15:20:09  crc_canada
# reducing more perl code.
#
# Revision 1.252  2007/01/09 22:58:39  crc_canada
# containerField created.
#
# Revision 1.251  2006/12/21 20:51:51  crc_canada
# PROTO code added to make backlinks (parents).
#
# Revision 1.250  2006/11/22 21:50:56  crc_canada
# Modified Texture registration
#
# Revision 1.249  2006/10/19 18:28:46  crc_canada
# More changes for removing Perl from the runtime
#
# Revision 1.248  2006/10/18 20:22:43  crc_canada
# More removal of Perl code
#
# Revision 1.247  2006/10/17 18:51:52  crc_canada
# Step 1 in getting rid of PERL parsing.
#
# Revision 1.246  2006/09/21 08:24:54  domob
# Script fields *should* be parsed correctly now.
#
#

sub open_possible_cvs_file(*;$)
{
    my $handle = shift;
    my $filename = shift;

    if (! -f $filename) {
	print <STDERR>, "note: creating $filename\n";
	`touch $filename`;
    }

    if (! -w $filename) {
	print <STDERR>, "warning: $filename not writable, will try cvs edit...\n";
	my $top_srcdir = "..";

	my $rel_filename = $filename;
	$rel_filename =~ s/^\.\.\///;

	print <STDERR>, "trying: cvs edit $rel_filename\n";
	$r = `cd $top_srcdir;cvs edit $rel_filename`;
	if ($? != 0) {
	    die("cvs edit $filename failed\n");
	}
    }

    open $handle, ">$filename" or 
	die("Can't open: $filename\n");

    print <STDERR>, "Generating: $filename\n";
}


# To allow faster internal representations of nodes to be calculated,
# there is the field '_change' which can be compared between the node
# and the internal rep - if different, needs to be regenerated.
#
# the rep needs to be allocated if _intern == 0.
# XXX Freeing?!!?

require 'VRMLFields.pm';
require 'VRMLNodes.pm';
require 'VRMLRend.pm';

#######################################################################
#######################################################################
#######################################################################
#
# gen_struct - Generate a node structure, adding fields for
# internal use
my $interalNodeCommonFields = 
               "       int _renderFlags; /*sensitive, etc */ \n"                  	.
               "       int _hit; \n"                   	.
               "       int _change; \n"                	.
	       "       int _ichange; \n"		.
               "       struct Vector* _parentVector; \n"  .
	       "       double _dist; /*sorting for blending */ \n".
	       "       float _extent[6]; /* used for boundingboxes - +-x, +-y, +-z */ \n" .
               "       void *_intern; \n"              	.
               "       int _nodeType; /* unique integer for each type */ \n".
               "       int referenceCount; /* if this reaches zero, nobody wants it anymore */ \n".
	       "       int _defaultContainer; /* holds the container */\n".
               " 	/*** node specific data: *****/\n";

sub gen_struct {
	my($name,$node) = @_;

	my @unsortedfields = keys %{$node->{FieldTypes}};

	# sort the field array, so that we can ensure the order of structure
	# elements.

	my @sf = sort(@unsortedfields);
	my $nf = scalar @sf;
	# /* Store actual point etc. later */
       my $s = "/***********************/\nstruct X3D_$name {\n" . $interalNodeCommonFields;

	for(@sf) {
		my $cty = "VRML::Field::$node->{FieldTypes}{$_}"->ctype($_);
		$s .= "\t$cty;\n";
	}

	$s .= "};\n";
	return ($s);
}

#######################################################

sub get_rendfunc {
	my($n) = @_;
	# XXX
	my @f = qw/Prep Rend Child Fin RendRay GenPolyRep Proximity Other Collision Compile/;
	my $comma = "";
	my $v = "\n";

	# create function prototypes
	for (@f) {

		# does this function exist?
		if (exists ${$_."C"}{$n}) {
			# it exists in the specified hash; now is the function in CFuncs, 
			# or generated here? (different names)
			if ($_ eq "Rend") {
				$v .= $comma."void render_".${n}."(struct X3D_".${n}." *);\n";
			} elsif ($_ eq "Prep") {
				$v .= $comma."void prep_".${n}."(struct X3D_".${n}." *);\n";
			} elsif ($_ eq "Fin") {
				$v .= $comma."void fin_".${n}."(struct X3D_".${n}." *);\n";
			} elsif ($_ eq "Child") {
				$v .= $comma."void child_".${n}."(struct X3D_".${n}." *);\n";
			} elsif ($_ eq "Proximity") {
				$v .= $comma."void proximity_".${n}."(struct X3D_".${n}." *);\n";
			} elsif ($_ eq "Other") {
				$v .= $comma."void other_".${n}."(struct X3D_".${n}." *);\n";
			} elsif ($_ eq "Collision") {
				# some collide_XXX nodes are common
				if (("ElevationGrid" ne ${n}) &&
				("ElevationGrid" ne ${n}) &&
				("IndexedFaceSet" ne ${n}) &&
				("IndexedTriangleFanSet" ne ${n}) &&
				("IndexedTriangleSet" ne ${n}) &&
				("IndexedTriangleStripSet" ne ${n}) &&
				("TriangleFanSet" ne ${n}) &&
				("TriangleSet" ne ${n}) &&
				("TriangleStripSet" ne ${n}) &&
				("VRML1_IndexedFaceSet" ne ${n}) &&
				("GeoElevationGrid" ne ${n})) {
					$v .= $comma."void collide_".${n}."(struct X3D_".${n}." *);\n";
				}
			} elsif ($_ eq "GenPolyRep") {
				# some make_XXX nodes are common
				if (("ElevationGrid" ne ${n}) &&
				("ElevationGrid" ne ${n}) &&
				("IndexedFaceSet" ne ${n}) &&
				("IndexedTriangleFanSet" ne ${n}) &&
				("IndexedTriangleSet" ne ${n}) &&
				("IndexedTriangleStripSet" ne ${n}) &&
				("TriangleFanSet" ne ${n}) &&
				("TriangleSet" ne ${n}) &&
				("TriangleStripSet" ne ${n}) &&
				("VRML1_IndexedFaceSet" ne ${n}) &&
				("GeoElevationGrid" ne ${n})) {
					$v .= $comma."void make_".${n}."(struct X3D_".${n}." *);\n";
				}
			} elsif ($_ eq "RendRay") {
				# some rendray_XXX nodes are common
				if (("ElevationGrid" ne ${n}) &&
				("ElevationGrid" ne ${n}) &&
				("Extrusion" ne ${n}) &&
				("Text" ne ${n}) &&
				("IndexedFaceSet" ne ${n}) &&
				("IndexedTriangleFanSet" ne ${n}) &&
				("IndexedTriangleSet" ne ${n}) &&
				("IndexedTriangleStripSet" ne ${n}) &&
				("TriangleFanSet" ne ${n}) &&
				("TriangleSet" ne ${n}) &&
				("TriangleStripSet" ne ${n}) &&
				("VRML1_IndexedFaceSet" ne ${n}) &&
				("GeoElevationGrid" ne ${n})) {
					$v .= $comma."void rendray_".${n}."(struct X3D_".${n}." *);\n";
				}
			} elsif ($_ eq "Compile") {
				$v .= $comma."void compile_".${n}."(struct X3D_".${n}." *);\n";
			}	
		}
	}

	# now go and do the actual filling out of the virtual tables
	my $f = "extern struct X3D_Virt virt_${n};\n";
	$v .= "struct X3D_Virt virt_${n} = { ";

	for (@f) {
		# does this function exist?
		if (exists ${$_."C"}{$n}) {
			# it exists in the specified hash; now is the function in CFuncs, 
			# or generated here? (different names)
			if ($_ eq "Rend") {
				$v .= $comma."(void *)render_".${n};
			} elsif ($_ eq "Prep") {
				$v .= $comma."(void *)prep_".${n};
			} elsif ($_ eq "Fin") {
				$v .= $comma."(void *)fin_".${n};
			} elsif ($_ eq "Child") {
				$v .= $comma."(void *)child_".${n};
			} elsif ($_ eq "Proximity") {
				$v .= $comma."(void *)proximity_".${n};
			} elsif ($_ eq "Other") {
				$v .= $comma."(void *)other_".${n};
			} elsif ($_ eq "Collision") {
				$v .= $comma."(void *)collide_".${n};
			} elsif ($_ eq "GenPolyRep") {
				$v .= $comma."(void *)make_".${n};
			} elsif ($_ eq "RendRay") {
				$v .= $comma."(void *)rendray_".${n};
			} elsif ($_ eq "Compile") {
				$v .= $comma."(void *)compile_".${n};
			} else {
				$v .= $comma."${n}_$_";
			}	
		} else {
			$v .= $comma."NULL";
		}
		$comma = ",";
	}
	$v .= "};\n";

	# now we have a table of entries
	return ($f,$v);
}


######################################################################
######################################################################
######################################################################
#
# gen - the main function. this contains much verbatim code
#
#

#############################################################################################
sub gen {
	# make a table of nodetypes, so that at runtime we can determine what kind a
	# node is - comes in useful at times.
	# also create tables, functions, to create/manipulate VRML/X3D at runtime.


	# DESTINATIONS of arrays:
	#	@genFuncs1,	../src/lib/scenegraph/GeneratedCode.c (printed first)
	#	@genFuncs2,	../src/lib/scenegraph/GeneratedCode.c (printed second)
	#	@str,		../src/lib/vrml_parser/Structs.h
	#	@nodeField,	../src/lib/vrml_parser/NodeFields.h
	#	@EAICommon,	../src/libeai/GeneratedCode.c 

	my $nodeIntegerType = 0; # make sure this maps to the same numbers in VRMLCU.pm
	my $fieldTypeCount = 0; 
	my $fieldNameCount = 0;
	my $keywordIntegerType = 0; 
	my %totalfields = ();
	my %allfields = ();
	my %allinputOutputs = ();
	my %allinputOnlyFields = ();
	my %alloutputOnlyFields = ();

	open LICENSE_BLOCK, "../versions/template/license.c.in";
	while (<LICENSE_BLOCK>) {
	    push @license_block, $_;
	}
	close LICENSE_BLOCK;

	#####################
	# for scenegraph/GeneratedCode.c - create a header.
	push @genFuncs1,
		"/*\n".
		"  $I"."d: $\n\n".
		"  GeneratedCode.c: generated by VRMLC.pm. DO NOT MODIFY, MODIFY VRMLC.pm INSTEAD.\n".
		"*/\n".
		" \n".
		"#include <config.h> \n".
		"#include <system.h> \n".
		"#include <display.h> \n".
		"#include <internal.h> \n".
		" \n".
		"#include <libFreeWRL.h> \n".
		"#include <io_files.h> \n".
		" \n".
		"#include \"../vrml_parser/Structs.h\" \n".
		"#include \"../main/headers.h\" \n".
		"#include \"../main/ProdCon.h\" \n".
		"#include \"Component_Networking.h\" \n".
		"#include \"Component_Picking.h\" \n".
		"#include \"../list.h\" \n".
		"#include \"../resources.h\" \n".
		"#include \"../io_http.h\" \n".
		" \n".
		" \n".
		"/**********************************************************************************************/ \n".
		"/*                                                                                            */ \n".
		"/* This file is part of the FreeWRL/FreeX3D Distribution, from http://freewrl.sourceforge.net */ \n".
		"/*                                                                                            */ \n".
		"/**********************************************************************************************/ \n".
		" \n".
		"#include \"../input/EAIHeaders.h\" \n".
		"#include \"../input/EAIHelpers.h\" \n".
		"#include \"../x3d_parser/Bindable.h\" \n".
		" \n".
		"#include \"../opengl/Textures.h\" \n".
		"#include \"Component_CubeMapTexturing.h\" \n".
		"#include \"Polyrep.h\" \n".
		"void add_OSCsensor(struct X3D_Node* node); /* WANT_OSC*/\n".
		"void addNodeToKeySensorList(struct X3D_Node* node);\n".
		"void collide_genericfaceset (struct X3D_IndexedFaceSet *node );\n".
		"void make_genericfaceset(struct X3D_IndexedFaceSet *this_);\n".
		"void render_ray_polyrep(void *node);\n".
		"void dump_scene(FILE *fp, int level, struct X3D_Node* node);\n".
		"extern char *parser_getNameFromNode(struct X3D_Node* node);\n";

	my $st = "/* definitions to help scanning values in from a string */ \n".
		"#define SCANTONUMBER(value) while (isspace(*value) || (*value==',')) value++; \n".
		"#define SCANTOSTRING(value) while (isspace(*value) || (*value==',')) value++; \n".
		"#define OLDSCANTOSTRING(value) while ((*value==' ') || (*value==',')) value++; \n".
		"#define ISSTARTNUMBER(value) (isdigit(*value) \\\n".
		"		|| (*value == '+') || (*value == '-')) \n".
		"#define SCANPASTFLOATNUMBER(value) while (isdigit(*value) \\\n".
		"		|| (*value == '.') || \\\n".
		"		(*value == 'E') || (*value == 'e') || (*value == '+') || (*value == '-')) value++; \n".
		"#define SCANPASTINTNUMBER(value) if (isdigit(*value) || (*value == '-') || (*value == '+')) value++; \\\n".
		"		while (isdigit(*value) || \\\n".
		"		(*value == 'x') || (*value == 'X') ||\\\n".
		"		((*value >='a') && (*value <='f')) || \\\n".
		"		((*value >='A') && (*value <='F')) || \\\n".
		"		(*value == '-') || (*value == '+')) value++; \n";
	push @str, $st; 

	push @EAICommon,
		"#include <config.h>\n".
		"#include <system.h>\n".
		"#include <display.h>\n".
		"\n".
		"#include <vrml_parser/Structs.h>\n".
		"#include <main/headers.h>\n".
		"#include <input/EAIHeaders.h>\n".
		"\n";

	#####################
	

        push @str, "/* Data type for index into ID-table. */\ntypedef int indexT;\n\n";


	# now, go through nodes, and do a few things:
	#	- create a "#define NODE_Shape 300" line for CFuncs/Structs.h;
	#	- create a "case NODE_Shape: printf ("Shape")" for CFuncs/GeneratedCode.c;
	#	- create a definitive list of all fieldnames.

        my @unsortedNodeList = keys %VRML::Nodes;
        my @sortedNodeList = sort(@unsortedNodeList);
	for (@sortedNodeList) {
		#print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.

		push @str, "#define NODE_".$_."	$nodeIntegerType\n";
		$nodeIntegerType ++;


		#{ use Devel::Peek 'Dump'; print "start of dump\n"; Dump $VRML::Nodes{$_}{FieldKinds}, 30; print "end of dump\n"; } 

 		#foreach my $field (keys %{$VRML::Nodes{$_}{FieldKinds}}) {print "field1 $field ". $VRML::Nodes{$_}{FieldKinds}{$field}."\n";}
 		#foreach my $field (keys %{$VRML::Nodes{$_}{FieldKinds}}) {print "field2 $field ". $VRML::Nodes{$_}{Fields}."\n";}

		# capture all fields.
		# also check for metadata here
		my $mdf = 0;
		my $omdf = 0;
 		foreach my $field (keys %{$VRML::Nodes{$_}{FieldTypes}}) {
			$totalfields{$field} = "recorded";
			if ($field eq "metadata") {$mdf = $mdf + 1;}
			if ($field eq "__oldmetadata") {$omdf = $omdf + 1;}
			#print "field2 $field\n"
		};

		# now, tell what kind of field it is. Hopefully, all fields will
		# have  a valid fieldtype, if not, there is an error somewhere.
 		foreach my $field (keys %{$VRML::Nodes{$_}{FieldKinds}}) {
			my $fk = $VRML::Nodes{$_}{FieldKinds}{$field};
			if ($fk eq "initializeOnly") { $allfields{$field} = $fk;}
			elsif ($fk eq "inputOnly") { $allinputOnlys{$field} = $fk;}
			elsif ($fk eq "outputOnly") { $alloutputOnlys{$field} = $fk;}
			elsif ($fk eq "inputOutput") { $allinputOutputs{$field} = $fk;}
			else {
				print "field $field fieldKind $fk is invalid\n";
			}
		};
	}
	push @str, "\n";


	#####################
	# we have a list of fields from ALL nodes. 
	push @str, "\n/* Table of built-in fieldIds */\nextern const char *FIELDNAMES[];\n";
	push @str, "extern const int FIELDNAMES_COUNT;\n";

	push @genFuncs1, "\n/* Table of built-in fieldIds */\n       const char *FIELDNAMES[] = {\n";

	foreach (keys %totalfields) { 
		#print "totalfields $_\n";
		push @str, "#define FIELDNAMES_".$_."	$fieldNameCount\n";
		$fieldNameCount ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int FIELDNAMES_COUNT = ARR_SIZE(FIELDNAMES);\n\n";
	
	# make a function to print field name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the field type */\n". 
		"const char *stringFieldType (int st) {\n".
		"	if ((st < 0) || (st >= FIELDNAMES_COUNT)) return \"(fieldName invalid)\"; \n".
		"	return FIELDNAMES[st];\n}\n\n";
	push @str, "const char *stringFieldType(int st);\n";

	#####################
	# we have a lists  of field types from ALL nodes. print out the ones without the underscores at the beginning
	push @str, "\n/* Table of built-in fieldIds */\nextern const char *EVENT_OUT[];\n";
	push @str, "extern const int EVENT_OUT_COUNT;\n";

	push @genFuncs1, "\n/* Table of EVENT_OUTs */\n       const char *EVENT_OUT[] = {\n";

	$nodeIntegerType = 0;
	foreach (keys %alloutputOnlys) { 
		if (index($_,"_") !=0) {
			push @genFuncs1, "	\"$_\",\n";
			push @str, "#define EVENT_OUT_$_	$nodeIntegerType\n";
			$nodeIntegerType ++;
		}
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int EVENT_OUT_COUNT = ARR_SIZE(EVENT_OUT);\n\n";
	
	push @str, "\n/* Table of built-in fieldIds */\nextern const char *EVENT_IN[];\n";
	push @str, "extern const int EVENT_IN_COUNT;\n";

	push @genFuncs1, "\n/* Table of EVENT_INs */\n       const char *EVENT_IN[] = {\n";

	$nodeIntegerType = 0;
	foreach (keys %allinputOnlys) { 
		if (index($_,"_") !=0) {
			push @genFuncs1, "	\"$_\",\n";
			push @str, "#define EVENT_IN_$_	$nodeIntegerType\n";
			$nodeIntegerType ++;
		}
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int EVENT_IN_COUNT = ARR_SIZE(EVENT_IN);\n\n";
	
	push @str, "\n/* Table of built-in fieldIds */\nextern const char *EXPOSED_FIELD[];\n";
	push @str, "extern const int EXPOSED_FIELD_COUNT;\n";

	push @genFuncs1, "\n/* Table of EXPOSED_FIELDs */\n       const char *EXPOSED_FIELD[] = {\n";

	$nodeIntegerType = 0;
	foreach (keys %allinputOutputs) { 
		if (index($_,"_") !=0) {
			push @genFuncs1, "	\"$_\",\n";
			push @str, "#define EXPOSED_FIELD_$_	$nodeIntegerType\n";
			$nodeIntegerType ++;
		}
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int EXPOSED_FIELD_COUNT = ARR_SIZE(EXPOSED_FIELD);\n\n";
	
	push @str, "\n/* Table of built-in fieldIds */\nextern const char *FIELD[];\n";
	push @str, "extern const int FIELD_COUNT;\n";

	push @genFuncs1, "\n/* Table of FIELDs */\n       const char *FIELD[] = {\n";

	$nodeIntegerType = 0;
	foreach (keys %allfields) { 
		if (index($_,"_") !=0) {
			push @genFuncs1, "	\"$_\",\n";
			push @str, "#define FIELD_$_	$nodeIntegerType\n";
			$nodeIntegerType ++;
		}
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int FIELD_COUNT = ARR_SIZE(FIELD);\n\n";
	


	#####################
	# process keywords 
	push @str, "\n/* Table of built-in keywords */\nextern const char *KEYWORDS[];\n";
	push @str, "extern const int KEYWORDS_COUNT;\n";

	push @genFuncs1, "\n/* Table of keywords */\n       const char *KEYWORDS[] = {\n";

	$keywordIntegerType = 0;
        my @sf = keys %KeywordC;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.

		my $kw = $_;
		push @str, "#define KW_".$kw."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int KEYWORDS_COUNT = ARR_SIZE(KEYWORDS);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the keyword type */\n". 
		"const char *stringKeywordType (int st) {\n".
		"	if ((st < 0) || (st >= KEYWORDS_COUNT)) return \"(keyword invalid)\"; \n".
		"	return KEYWORDS[st];\n}\n\n";
	push @str, "const char *stringKeywordType(int st);\n";


	#####################
	# process  profiles
	push @str, "\n/* Table of built-in profiles */\nextern const char *PROFILES[];\n";
	push @str, "extern const int PROFILES_COUNT;\n";

	push @genFuncs1, "\n/* Table of profiles */\n       const char *PROFILES[] = {\n";

	my $profileIntegerType = 0;
        my @sf = keys %ProfileC;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.

		# NOTE: We can not have a profile of "MPEG-4", make it MPEG4 (no "-" allowed)
		my $kw = $_;
		if ($kw eq "MPEG-4") {$kw = "MPEG4";}
		push @str, "#define PRO_".$kw."	$profileIntegerType\n";
		$profileIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int PROFILES_COUNT = ARR_SIZE(PROFILES);\n\n";

	# make a function to print Profile name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the profile type */\n". 
		"const char *stringProfileType (int st) {\n".
		"	if ((st < 0) || (st >= PROFILES_COUNT)) return \"(profile invalid)\"; \n".
		"	return PROFILES[st];\n}\n\n";
	push @str, "const char *stringProfileType(int st);\n";

	#####################
	# process components 
	push @str, "\n/* Table of built-in components */\nextern const char *COMPONENTS[];\n";
	push @str, "extern const int COMPONENTS_COUNT;\n";

	push @genFuncs1, "\n/* Table of components */\n       const char *COMPONENTS[] = {\n";

	my $componentIntegerType = 0;
        my @sf = keys %ComponentC;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.

		# NOTE: We can not have a component of "H-ANIM", make it HANIM (no "-" allowed)
		my $kw = $_;
		if ($kw eq "H-Anim") {$kw = "HAnim";}
		push @str, "#define COM_".$kw."	$componentIntegerType\n";
		$componentIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int COMPONENTS_COUNT = ARR_SIZE(COMPONENTS);\n\n";

	# make a function to print Component name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the component type */\n". 
		"const char *stringComponentType (int st) {\n".
		"	if ((st < 0) || (st >= COMPONENTS_COUNT)) return \"(component invalid)\"; \n".
		"	return COMPONENTS[st];\n}\n\n";
	push @str, "const char *stringComponentType(int st);\n";


	#####################
	# process PROTO keywords 
	push @str, "\n/* Table of built-in PROTO keywords */\nextern const char *PROTOKEYWORDS[];\n";
	push @str, "extern const int PROTOKEYWORDS_COUNT;\n";

	push @genFuncs1, "\n/* Table of PROTO keywords */\n       const char *PROTOKEYWORDS[] = {\n";

        my @sf = keys %PROTOKeywordC;
	$keywordIntegerType = 0;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.
		push @str, "#define PKW_".$_."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int PROTOKEYWORDS_COUNT = ARR_SIZE(PROTOKEYWORDS);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the PROTO keyword type */\n". 
		"const char *stringPROTOKeywordType (int st) {\n".
		"	if ((st < 0) || (st >= PROTOKEYWORDS_COUNT)) return \"(proto keyword invalid)\"; \n".
		"	return PROTOKEYWORDS[st];\n}\n\n";
	push @str, "const char *stringPROTOKeywordType(int st);\n";

	#####################
	# process X3DSPECIAL keywords 
	push @str, "\n/* Table of built-in X3DSPECIAL keywords */\nextern const char *X3DSPECIAL[];\n";
	push @str, "extern const int X3DSPECIAL_COUNT;\n";

	push @genFuncs1, "\n/* Table of X3DSPECIAL keywords */\n       const char *X3DSPECIAL[] = {\n";

        my @sf = keys %X3DSpecialC;
	$keywordIntegerType = 0;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.
		push @str, "#define X3DSP_".$_."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int X3DSPECIAL_COUNT = ARR_SIZE(X3DSPECIAL);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the X3DSPECIAL keyword type */\n". 
		"const char *stringX3DSPECIALType (int st) {\n".
		"	if ((st < 0) || (st >= X3DSPECIAL_COUNT)) return \"(special keyword invalid)\"; \n".
		"	return X3DSPECIAL[st];\n}\n\n";
	push @str, "const char *stringX3DSPECIALType(int st);\n";


	#####################
	# process TEXTUREBOUNDARY keywords 
	push @str, "\n/* Table of built-in TEXTUREBOUNDARY keywords */\nextern const char *TEXTUREBOUNDARYKEYWORDS[];\n";
	push @str, "extern const int TEXTUREBOUNDARYKEYWORDS_COUNT;\n";

	push @genFuncs1, "\n/* Table of TEXTUREBOUNDARY keywords */\n       const char *TEXTUREBOUNDARYKEYWORDS[] = {\n";

        my @sf = keys %TextureBoundaryC;
	$keywordIntegerType = 0;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.
		push @str, "#define TB_".$_."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int TEXTUREBOUNDARYKEYWORDS_COUNT = ARR_SIZE(TEXTUREBOUNDARYKEYWORDS);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the TEXTUREBOUNDARY keyword type */\n". 
		"const char *stringTEXTUREBOUNDARYKeywordType (int st) {\n".
		"	if ((st < 0) || (st >= TEXTUREBOUNDARYKEYWORDS_COUNT)) return \"(texture param keyword invalid)\"; \n".
		"	return TEXTUREBOUNDARYKEYWORDS[st];\n}\n\n";
	push @str, "const char *stringTEXTUREBOUNDARYKeywordType(int st);\n";

	#####################
	# process TEXTUREMAGNIFICATION keywords 
	push @str, "\n/* Table of built-in TEXTUREMAGNIFICATION keywords */\nextern const char *TEXTUREMAGNIFICATIONKEYWORDS[];\n";
	push @str, "extern const int TEXTUREMAGNIFICATIONKEYWORDS_COUNT;\n";

	push @genFuncs1, "\n/* Table of TEXTUREMAGNIFICATION keywords */\n       const char *TEXTUREMAGNIFICATIONKEYWORDS[] = {\n";

        my @sf = keys %TextureMagnificationC;
	$keywordIntegerType = 0;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.
		push @str, "#define TMAG_".$_."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int TEXTUREMAGNIFICATIONKEYWORDS_COUNT = ARR_SIZE(TEXTUREMAGNIFICATIONKEYWORDS);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the TEXTUREMAGNIFICATION keyword type */\n". 
		"const char *stringTEXTUREMAGNIFICATIONKeywordType (int st) {\n".
		"	if ((st < 0) || (st >= TEXTUREMAGNIFICATIONKEYWORDS_COUNT)) return \"(texture param keyword invalid)\"; \n".
		"	return TEXTUREMAGNIFICATIONKEYWORDS[st];\n}\n\n";
	push @str, "const char *stringTEXTUREMAGNIFICATIONKeywordType(int st);\n";


	#####################
	# process TEXTUREMINIFICATION keywords 
	push @str, "\n/* Table of built-in TEXTUREMINIFICATION keywords */\nextern const char *TEXTUREMINIFICATIONKEYWORDS[];\n";
	push @str, "extern const int TEXTUREMINIFICATIONKEYWORDS_COUNT;\n";

	push @genFuncs1, "\n/* Table of TEXTUREMINIFICATION keywords */\n       const char *TEXTUREMINIFICATIONKEYWORDS[] = {\n";

        my @sf = keys %TextureMinificationC;
	$keywordIntegerType = 0;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.
		push @str, "#define TMIN_".$_."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int TEXTUREMINIFICATIONKEYWORDS_COUNT = ARR_SIZE(TEXTUREMINIFICATIONKEYWORDS);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the TEXTUREMINIFICATION keyword type */\n". 
		"const char *stringTEXTUREMINIFICATIONKeywordType (int st) {\n".
		"	if ((st < 0) || (st >= TEXTUREMINIFICATIONKEYWORDS_COUNT)) return \"(texture param keyword invalid)\"; \n".
		"	return TEXTUREMINIFICATIONKEYWORDS[st];\n}\n\n";
	push @str, "const char *stringTEXTUREMINIFICATIONKeywordType(int st);\n";

	#####################
	# process TEXTURECOMPRESSION keywords 
	push @str, "\n/* Table of built-in TEXTURECOMPRESSION keywords */\nextern const char *TEXTURECOMPRESSIONKEYWORDS[];\n";
	push @str, "extern const int TEXTURECOMPRESSIONKEYWORDS_COUNT;\n";

	push @genFuncs1, "\n/* Table of TEXTURECOMPRESSION keywords */\n       const char *TEXTURECOMPRESSIONKEYWORDS[] = {\n";

        my @sf = keys %TextureCompressionC;
	$keywordIntegerType = 0;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.
		push @str, "#define TC_".$_."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int TEXTURECOMPRESSIONKEYWORDS_COUNT = ARR_SIZE(TEXTURECOMPRESSIONKEYWORDS);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the TEXTURECOMPRESSION keyword type */\n". 
		"const char *stringTEXTURECOMPRESSIONKeywordType (int st) {\n".
		"	if ((st < 0) || (st >= TEXTURECOMPRESSIONKEYWORDS_COUNT)) return \"(texture param keyword invalid)\"; \n".
		"	return TEXTURECOMPRESSIONKEYWORDS[st];\n}\n\n";
	push @str, "const char *stringTEXTURECOMPRESSIONKeywordType(int st);\n";


	#####################
	# process VRML1Modifier keywords 
	push @str, "\n/* Table of built-in VRML1Modifier keywords */\nextern const char *VRML1Modifier[];\n";
	push @str, "extern const int VRML1Modifier_COUNT;\n";

	push @genFuncs1, "\n/* Table of VRML1Modifier keywords */\n       const char *VRML1Modifier[] = {\n";

        my @sf = keys %VRML1ModifierC;
	$keywordIntegerType = 0;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.
		push @str, "#define VRML1MOD_".$_."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int VRML1Modifier_COUNT = ARR_SIZE(VRML1Modifier);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the VRML1Modifier keyword type */\n". 
		"const char *stringVRML1ModifierType (int st) {\n".
		"	if ((st < 0) || (st >= VRML1Modifier_COUNT)) return \"(VRML1 modifier invalid)\"; \n".
		"	return VRML1Modifier[st];\n}\n\n";
	push @str, "const char *stringVRML1ModifierType(int st);\n";

	#####################
	# process GEOSPATIAL keywords 
	push @str, "\n/* Table of built-in GEOSPATIAL keywords */\nextern const char *GEOSPATIAL[];\n";
	push @str, "extern const int GEOSPATIAL_COUNT;\n";

	push @genFuncs1, "\n/* Table of GEOSPATIAL keywords */\n       const char *GEOSPATIAL[] = {\n";

        my @sf = keys %GEOSpatialKeywordC;
	$keywordIntegerType = 0;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.
		push @str, "#define GEOSP_".$_."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int GEOSPATIAL_COUNT = ARR_SIZE(GEOSPATIAL);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the GEOSPATIAL keyword type */\n". 
		"const char *stringGEOSPATIALType (int st) {\n".
		"	if ((st < 0) || (st >= GEOSPATIAL_COUNT)) return \"(keyword invalid)\"; \n".
		"	return GEOSPATIAL[st];\n}\n\n";
	push @str, "const char *stringGEOSPATIALType(int st);\n";

	#####################
	# process VRML1_ keywords 
	push @str, "\n/* Table of built-in VRML1_ keywords */\nextern const char *VRML1_[];\n";
	push @str, "extern const int VRML1__COUNT;\n";

	push @genFuncs1, "\n/* Table of VRML1_ keywords */\n       const char *VRML1_[] = {\n";

        my @sf = keys %VRML1_C;
	$keywordIntegerType = 0;
	for (@sf) {
		# print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.
		push @str, "#define VRML1_".$_."	$keywordIntegerType\n";
		$keywordIntegerType ++;
		push @genFuncs1, "	\"$_\",\n";
	}
	push @str, "\n";
	push @genFuncs1, "};\nconst int VRML1__COUNT = ARR_SIZE(VRML1_);\n\n";

	# make a function to print Keyword name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the VRML1_ keyword type */\n". 
		"const char *stringVRML1_Type (int st) {\n".
		"	if ((st < 0) || (st >= VRML1__COUNT)) return \"(VRML1 keyword invalid)\"; \n".
		"	return VRML1_[st];\n}\n\n";
	push @str, "const char *stringVRML1_Type(int st);\n";

	##############################################################

	# Convert TO/FROM EAI to Internal field types. (EAI types are ascii).
		my $st = "char mapFieldTypeToEAItype (int st) {\n".
		    "	switch (st) { \n";
	push @genFuncs2, $st; push @EAICommon, $st;


	for(@VRML::Fields) {
		# print "node $_ is tagged as $fieldTypeCount\n";
		# tag each node type with a integer key.
		my $st = "\t\tcase FIELDTYPE_".$_.":	return EAI_$_;\n";
		push @genFuncs2, $st; push @EAICommon, $st;
	}
	my $st = "\t\tdefault: return -1;\n\t}\n\treturn -1;\n}\n";
	push @genFuncs2, $st; push @EAICommon, $st;
	push @str, "char mapFieldTypeToEAItype (int st);\n";


	####################
	my $st = "/* convert an EAI type to an internal type */\n". 
		"int mapEAItypeToFieldType (char st) {\n".
		"	switch (st) { \n";
	push @genFuncs2, $st; push @EAICommon, $st;


	for(@VRML::Fields) {
		# print "node $_ is tagged as $fieldTypeCount\n";
		# tag each node type with a integer key.
		my $st = "\t\tcase EAI_".$_.":	return FIELDTYPE_$_;\n";
		push @genFuncs2, $st; push @EAICommon, $st;
	}
	my $st = "\t\tdefault: return -1;\n\t}\n\treturn -1;\n}\n";
	push @genFuncs2, $st; push @EAICommon, $st;
	push @str, "int mapEAItypeToFieldType (char st);\n";

	####################
	push @genFuncs2, "/* convert an MF type to an SF type */\n". 
		"int convertToSFType (int st) {\n".
		"	switch (st) { \n";


	for(@VRML::Fields) {
		# print "node $_ is tagged as $fieldTypeCount\n";
		# tag each node type with a integer key.
		my $sftype = $_;
		$sftype =~ s/MF/SF/;

		push @genFuncs2,  "\t\tcase FIELDTYPE_$_:	return FIELDTYPE_$sftype;\n";
	}
	push @genFuncs2, 	"	}\n	return -1;\n}\n";
	push @str, "int convertToSFType (int st);\n";





	#####################
	# give each field an identifier
	push @str, "\n/* Table of built-in fieldIds */\nextern const char *FIELDTYPES[];\n";
	push @str, "extern const int FIELDTYPES_COUNT;\n";

	my $ts = "\n/* Table of Field Types */\n       const char *FIELDTYPES[] = {\n";
	push @genFuncs1, $ts;
	push @EAICommon, $ts;

	$fieldTypeCount = 0;
	for(@VRML::Fields) {
		# print "node $_ is tagged as $fieldTypeCount\n";
		# tag each node type with a integer key.
		my $defstr = "#define FIELDTYPE_".$_."	$fieldTypeCount\n";
		push @str, $defstr; 
		$fieldTypeCount ++;
		$printNodeStr = "	\"$_\",\n";
		push @genFuncs1, $printNodeStr;
		push @EAICommon, $printNodeStr;
	}
	push @str, "\n";
	my $ts = "};\nconst int FIELDTYPES_COUNT = ARR_SIZE(FIELDTYPES);\n\n";
	push @genFuncs1, $ts;
	push @EAICommon, $ts;

	for(@VRML::Fields) {
		push @str, ("VRML::Field::$_")->cstruct . "\n";
	}
	# make a function to print fieldtype name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the fieldtype type */\n". 
		"const char *stringFieldtypeType (int st) {\n".
		"	if ((st < 0) || (st >= FIELDTYPES_COUNT)) return \"(fieldType invalid)\"; \n".
		"	return FIELDTYPES[st];\n}\n\n";
	push @str, "const char *stringFieldtypeType(int st);\n";



	#####################
	# handle the nodes themselves
	push @str, "\n/* Table of built-in nodeIds */\nextern const char *NODES[];\n";
	push @str, "extern const int NODES_COUNT;\n";

	push @genFuncs1, "\n/* Table of Node Types */\n       const char *NODES[] = {\n";

        push @str, "\n/* and now the structs for the nodetypes */ \n";
	for(@sortedNodeList) {
		$printNodeStr = "	\"$_\",\n";
		push @genFuncs1, $printNodeStr;
	}
	push @genFuncs1, "};\nconst int NODES_COUNT = ARR_SIZE(NODES);\n\n";

	# make a function to print node name from an integer type.
	push @genFuncs2, "/* Return a pointer to a string representation of the node type */\n". 
		"const char *stringNodeType (int st) {\n".
		"	if ((st < 0) || (st >= NODES_COUNT)) return \"(node invalid)\"; \n".
		"	return NODES[st];\n}\n\n";
	push @str, "const char *stringNodeType(int st);\n";


	###################
	# create the virtual tables for each node.
	push @str, "\n/* First, a generic struct, contains only the common elements */\n".
	"struct X3D_Node {\n". $interalNodeCommonFields .  "};\n".
	"#define X3D_NODE(node) ((struct X3D_Node*)node)\n".
	"#define X3D_GROUP(node) ((struct X3D_Group*)node)\n".
	"#define X3D_PICKABLEGROUP(node) ((struct X3D_PickableGroup*)node)\n".
	"#define X3D_POINTPICKSENSOR(node) ((struct X3D_PointPickSensor*)node)\n".
	"#define X3D_STATICGROUP(node) ((struct X3D_StaticGroup*)node)\n".
	"#define X3D_ANCHOR(node) ((struct X3D_Anchor*)node)\n".
	"#define X3D_COLLISION(node) ((struct X3D_Collision*)node)\n".
	"#define X3D_COMPOSEDSHADER(node) ((struct X3D_ComposedShader*)node)\n".
	"#define X3D_PACKAGEDSHADER(node) ((struct X3D_PackagedShader*)node)\n".
	"#define X3D_PROGRAMSHADER(node) ((struct X3D_ProgramShader*)node)\n".
	"#define X3D_SHADERPROGRAM(node) ((struct X3D_ShaderProgram*)node)\n".
	"#define X3D_SHAPE(node) ((struct X3D_Shape*)node)\n".
	"#define X3D_VISIBILITYSENSOR(node) ((struct X3D_VisibilitySensor*)node)\n".
	"#define X3D_BILLBOARD(node) ((struct X3D_Billboard*)node)\n".
	"#define X3D_BACKGROUND(node) ((struct X3D_Background*)node)\n".
	"#define X3D_TEXTUREBACKGROUND(node) ((struct X3D_TextureBackground*)node)\n".
	"#define X3D_FOG(node) ((struct X3D_Fog*)node)\n".
	"#define X3D_INLINE(node) ((struct X3D_Inline*)node)\n".
	"#define X3D_SWITCH(node) ((struct X3D_Switch*)node)\n".
	"#define X3D_SCRIPT(node) ((struct X3D_Script*)node)\n".
	"#define X3D_VIEWPOINT(node) ((struct X3D_Viewpoint*)node)\n".
	"#define X3D_ORTHOVIEWPOINT(node) ((struct X3D_OrthoViewpoint*)node)\n".
	"#define X3D_LODNODE(node) ((struct X3D_LOD*)node)\n".
	"#define X3D_TRANSFORM(node) ((struct X3D_Transform*)node)\n".
	"#define X3D_PROXIMITYSENSOR(node) ((struct X3D_ProximitySensor*)node)\n".
	"#define X3D_POINTLIGHT(node) ((struct X3D_PointLight*)node)\n".
	"#define X3D_SPOTLIGHT(node) ((struct X3D_SpotLight*)node)\n".
	"#define X3D_DIRECTIONALLIGHT(node) ((struct X3D_DirectionalLight*)node)\n".
	"#define X3D_INDEXEDFACESET(node) ((struct X3D_IndexedFaceSet*)node)\n".
	"#define VRML1_INDEXEDFACESET(node) ((struct X3D_VRML1_IndexedFaceSet*)node)\n".
	"#define VRML1_SEPARATOR(node) ((struct X3D_VRML1_Separator*)node)\n".
	"#define X3D_ELEVATIONGRID(node) ((struct X3D_ElevationGrid*)node)\n".
	"#define X3D_INDEXEDTRIANGLEFANSET(node) ((struct X3D_IndexedTriangleFanSet*)node)\n". 
	"#define X3D_INDEXEDTRIANGLESET(node) ((struct X3D_IndexedTriangleSet*)node)\n". 
	"#define X3D_INDEXEDTRIANGLESTRIPSET(node) ((struct X3D_IndexedTriangleStripSet*)node)\n". 
	"#define X3D_TRIANGLEFANSET(node) ((struct X3D_TriangleFanSet*)node)\n". 
	"#define X3D_TRIANGLESET(node) ((struct X3D_TriangleSet*)node)\n". 
	"#define X3D_TRIANGLESTRIPSET(node) ((struct X3D_TriangleStripSet*)node)\n". 


	"#define X3D_GEOORIGIN(node) ((struct X3D_GeoOrigin*)node)\n".
	"#define X3D_GEOLOD(node) ((struct X3D_GeoLOD*)node)\n".
	"#define X3D_GEOCOORD(node) ((struct X3D_GeoCoordinate*)node)\n".
	"#define X3D_GEOVIEWPOINT(node) ((struct X3D_GeoViewpoint*)node)\n".
	"#define X3D_GEOELEVATIONGRID(node) ((struct X3D_GeoElevationGrid*)node)\n".
	"#define X3D_GEOLOCATION(node) ((struct X3D_GeoLocation*)node)\n".
	"#define X3D_GEOTRANSFORM(node) ((struct X3D_GeoTransform*)node)\n".
	"#define X3D_GEOPROXIMITYSENSOR(node) ((struct X3D_GeoProximitySensor*)node)\n".

	"#define X3D_COLOR(node) ((struct X3D_Color*)node)\n".
	"#define X3D_COORDINATE(node) ((struct X3D_Coordinate*)node)\n".
	"#define X3D_NORMAL(node) ((struct X3D_Normal*)node)\n".
	"#define X3D_TEXTURECOORDINATE(node) ((struct X3D_TextureCoordinate*)node)\n".
	"#define X3D_IMAGETEXTURE(node) ((struct X3D_ImageTexture*)node)\n".
	"#define X3D_TEXTUREPROPERTIES(node) ((struct X3D_TextureProperties*)node)\n".
	"#define X3D_PIXELTEXTURE(node) ((struct X3D_PixelTexture*)node)\n".


	"#undef DEBUG_VALIDNODE\n".
	"#ifdef DEBUG_VALIDNODE	\n".
	"#define X3D_NODE_CHECK(node) checkNode(node,__FILE__,__LINE__)\n".
	"#define MARK_EVENT(node,offset) mark_event_check(node,(int) offset,__FILE__,__LINE__)\n".
	"#else\n".
	"#define X3D_NODE_CHECK(node)\n".
	"#define MARK_EVENT(node,offset)	mark_event(node,(int) offset)\n".
	"#endif\n".
	"#define COPY_SFVEC3F_TO_POINT_XYZ(too,from) { too.x = from[0]; too.y = from[1]; too.z = from[2];}\n".
	"#define COPY_POINT_XYZ_TO_SFVEC3F(too,from) { too[0] = (float) from.x; too[1] = (float) from.y; too[2] = (float) from.z;}\n".
	"#define offsetPointer_deref(t, me, offs) ((t)(((char*)(me))+offs))\n".

	"\n/* now, generated structures for each VRML/X3D Node*/\n";


	push @genFuncs1, "/* Virtual tables for each node */\n";

	for(@sortedNodeList) {
		# print "working on node $_\n";
		my $no = $VRML::Nodes{$_};
		my($strret) = gen_struct($_, $no);
		push @str, $strret;

		my($externdeclare, $vstru) = get_rendfunc($_);
		push @str, $externdeclare;

		push @genFuncs1, $vstru;
	}
	push @genFuncs1, "\n";

	push @genFuncs1, "/* table containing pointers to every virtual struct for each node type */ \n";
	push @genFuncs1, "struct X3D_Virt* virtTable[] = { \n";
	push @str, "extern struct X3D_Virt* virtTable[];\n";
	for(@sortedNodeList) {

		push @genFuncs1, "\t &virt_".$_.",\n";
	}
	push @genFuncs1, "\tNULL}; \n\n";

	#####################
	# create a routine to create a new node type

	push @genFuncs2,
	"/* create a new node of type. This can be generated by Perl code, much as the Structs.h is */\n".
	"void *createNewX3DNode (int nt) {\n".
	"	void * tmp;\n".
	"	struct X3D_Box *node;\n".
	"\n".
	"	tmp = NULL;\n".
	"	switch (nt) {\n";

	for (@sortedNodeList) {
		push @genFuncs2,
			"		case NODE_$_ : {tmp = MALLOC (struct X3D_$_ *, sizeof (struct X3D_$_)); break;}\n";
	}
	push @genFuncs2, "		default: {printf (\"createNewX3DNode = unknown type %d, this will fail\\n\",nt); return NULL;}\n";
	
	push @genFuncs2,
	"	}\n\n".
	"	/* now, fill in the node to DEFAULT values This mimics \"alloc_struct\" in the Perl code. */\n".
	"	/* the common stuff between all nodes. We'll use a X3D_Box struct, just because. It is used\n".
	"	   in this way throughought the code */\n".
	"	node = (struct X3D_Box *) tmp;\n".
	"	node->_renderFlags = 0; /*sensitive, etc */\n".
	"	node->_hit = 0;\n".
	"	node->_change = NODE_CHANGE_INIT_VAL; \n".
	"	node->_parentVector = newVector(struct X3D_Node*, 1);\n".
	"	node->_ichange = 0;\n".
	"	node->_dist = -10000.0; /*sorting for blending */\n".
	"	INITIALIZE_EXTENT\n".
	"	node->_intern = 0;\n".
	"	node->_nodeType = nt; /* unique integer for each type */\n".
	"	node->referenceCount = 1; /* we have requested this, we want it! */\n".
	"	\n";


	push @genFuncs2,
	"	/* now, fill in the node specific stuff here. the defaults are in VRMLNodes.pm */\n".
	"	switch (nt) {\n";

	for my $node (@sortedNodeList) {
		push @genFuncs2,
			"\t\tcase NODE_$node : {\n\t\t\tstruct X3D_$node * tmp2;\n";
		
		push @genFuncs2, "\t\t\ttmp2 = (struct X3D_$node *) tmp;\n\t\t\/* ttmp2->v = &virt_$node;*/ \n";


		#print "\nnode $node:\n";
 		foreach my $field (keys %{$VRML::Nodes{$node}{Defaults}}) {
			my $ft = $VRML::Nodes{$node}{FieldTypes}{$field};
			my $fk = $VRML::Nodes{$node}{FieldKinds}{$field};
			my $def = $VRML::Nodes{$node}{Defaults}{$field};
			#print "	fieldX $field \n";
			#print "		fieldDefaults ". $VRML::Nodes{$node}{Defaults}{$field}."\n";
			#print "		fieldKinds ". $VRML::Nodes{$node}{FieldKinds}{$field}."\n";
			#print "		fieldTypes ". $VRML::Nodes{$node}{FieldTypes}{$field}."\n";
	#		if ($fk ne "inputOnly") {
				#print "		do thisfield\n";

				# do we need to initialize the occlusion number for fields?
				my $cf;
				if ($field eq "__OccludeNumber") {
					$cf = "tmp2->__OccludeNumber = newOcclude();";
				} else {
					$cf = ("VRML::Field::$ft")->cInitialize("tmp2->".$field,$def);
				}

				push @genFuncs2, "\t\t\t$cf;\n";
	#		}
		}

	# rig in the default container for X3D parsing.
	if (exists $defaultContainerType{$node}) {
		#print "node $node, defaultContainer is " . $defaultContainerType{$node}."\n";
		push @genFuncs2, "\t\t\ttmp2->_defaultContainer = FIELDNAMES_".$defaultContainerType{$node}.";\n";
	} else {
		print "defaultContainerType for $node missing\n";
	}
		
		push @genFuncs2,"\t\tbreak;\n\t\t}\n";
	}
	push @genFuncs2, "\t};\n";
	push @genFuncs2,
	"	\n".
	#"	/* is this possibly the text node for the statusbar?? */ \n".
	#"	if (nt == NODE_Text) lastTextNode = (struct X3D_Text *) tmp; \n".
	"	/* is this a texture holding node? */\n".
	"	registerTexture(tmp);\n".
	"	/* Node Tracking */\n".
	"	registerX3DNode(tmp);\n".
	"	/* is this a bindable node? */\n".
	"	registerBindable(tmp);\n".
	"	/* is this a OSC sensor node? */\n".
	"	add_OSCsensor(tmp); /* WANT_OSC */\n".
	"	/* is this a pick sensor node? */\n".
	"	add_picksensor(tmp); /* DJTRACK_PICKSENSORS */\n".
	"	/* is this a time tick node? */\n".
	"       add_first(tmp);\n".
	"       /* possibly a KeySensor node? */\n".
	"       addNodeToKeySensorList(X3D_NODE(tmp));\n";



	push @genFuncs2, "\treturn tmp;\n}\n";


	#####################################################################
	# create a routine to dump scene graph. 

	push @genFuncs2,
	"/* Dump the scene graph.  */\n".
	"#define Boolean int\n".
	"void dump_scene (FILE *fp, int level, struct X3D_Node* node) {\n".
	"	#define spacer	for (lc=0; lc<level; lc++) fprintf (fp,\"\\t\");\n".
	"	int lc;\n".
	"	int i;\n".
        "	char *nodeName;\n".
	"	#ifdef FW_DEBUG\n\t\tBoolean allFields;\n".
	"		if (fileno(fp) == fileno(stdout)) { allFields = TRUE; } else { allFields = FALSE; }\n".
	"	#else\n\t\tBoolean allFields = FALSE;\n\t#endif\n".
	"	/* See vi +/double_conditional codegen/VRMLC.pm */\n".
	"	if (node==NULL) return; \n\n".
	"	fflush(fp);\n".
	"	if (level == 0) fprintf (fp,\"starting dump_scene\\n\");\n".
	"	nodeName = parser_getNameFromNode(node) ;\n".
	"	if (nodeName == NULL) {\n".
	"		spacer fprintf (fp,\"L%d: node (%p) () type %s\\n\",level,node,stringNodeType(node->_nodeType));\n".
        "	} else {\n".
	"		spacer fprintf (fp,\"L%d: node (%p) (DEF %s) type %s\\n\",level,node,nodeName,stringNodeType(node->_nodeType));\n".
        "	}\n".
	"	switch (node->_nodeType) {\n";

	for my $node (@sortedNodeList) {
		push @genFuncs2, "		case NODE_$node : {\n";
		push @genFuncs2, "			struct X3D_$node *tmp;\n";
		push @genFuncs2, "			tmp = (struct X3D_$node *) node;\n";
		if($node eq "PointPickSensor") {
			push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t_nparents (int) %d\\n\",vector_size(tmp->_parentVector)); /* DJTRACK_PICKSENSORS */\n";
			push @genFuncs2, "\t\t\tfor (i=0; i<vector_size(tmp->_parentVector); i++) { spacer fprintf (fp,\"    %d: %p\\n\",i, vector_get(struct X3D_Node *, tmp->_parentVector,i)); }\n";
		}
 		foreach my $field (keys %{$VRML::Nodes{$node}{Defaults}}) {

			my $ft = $VRML::Nodes{$node}{FieldTypes}{$field};
			my $fk = $VRML::Nodes{$node}{FieldKinds}{$field};
			if ( ($fk eq "field") || ($fk eq "inputOutput") ) {
				#
				# This is effectively a double_conditional,
				# ie the conditional is only inserted if
				# the variable meets certain criteria.
				#
				my $firstC = substr($field, 0, 1);
				if (($firstC eq "_") || $field eq "metadata") {
					push @genFuncs2, "\t\t    if(allFields) {\n";
				}
				if ($ft eq "FreeWRLPTR") {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft) (void pointer, not dumped)\\n\");\n";

				} elsif ($ft eq "SFInt32") {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft) \\t%d\\n\",tmp->$field);\n";

				} elsif (($ft eq "SFFloat") || ($ft eq "SFTime") || ($ft eq "SFDouble")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft) \\t%4.3f\\n\",tmp->$field);\n";

				} elsif ($ft eq "SFBool") {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft) \\t%d\\n\",tmp->$field);\n";

				} elsif ($ft eq "SFString") {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft) \\t%s\\n\",tmp->$field->strptr);\n";

				} elsif ($ft eq "SFNode") {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\"); ";
                        		push @genFuncs2, "dump_scene(fp,level+1,tmp->$field); \n";

				} elsif (($ft eq "SFRotation") || ($ft eq "SFColorRGBA")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft): \\t\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<4; i++) { fprintf (fp,\"%4.3f  \",tmp->$field.c[i]); }\n";
					push @genFuncs2,"\t\t\tfprintf (fp,\"\\n\");\n";

				} elsif (($ft eq "SFVec4f") || ($ft eq "SFVec4d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft): \\t\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<4; i++) { fprintf (fp,\"%4.3f  \",tmp->$field.c[i]); }\n";
					push @genFuncs2,"\t\t\tfprintf (fp,\"\\n\");\n";

				} elsif (($ft eq "SFColor") || ($ft eq "SFVec3f") || ($ft eq "SFVec3d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft): \\t\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<3; i++) { fprintf (fp,\"%4.3f  \",tmp->$field.c[i]); }\n";
					push @genFuncs2,"\t\t\tfprintf (fp,\"\\n\");\n";

				} elsif (($ft eq "SFVec2f") || ($ft eq "SFVec2d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft): \\t\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<2; i++) { fprintf (fp,\"%4.3f  \",tmp->$field.c[i]); }\n";
					push @genFuncs2,"\t\t\tfprintf (fp,\"\\n\");\n";

				} elsif ($ft eq "SFImage") {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft): (not dumped)\\t\");\n";
					push @genFuncs2,"\t\t\tfprintf (fp,\"\\n\");\n";

				} elsif (($ft eq "SFMatrix3f") || ($ft eq "SFMatrix3d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft): \\t\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<9; i++) { fprintf (fp,\"%4.3f  \",tmp->$field.c[i]); }\n";
					push @genFuncs2,"\t\t\tfprintf (fp,\"\\n\");\n";

				} elsif (($ft eq "SFMatrix4f") || ($ft eq "SFMatrix4d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft): \\t\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<16; i++) { fprintf (fp,\"%4.3f  \",tmp->$field.c[i]); }\n";
					push @genFuncs2,"\t\t\tfprintf (fp,\"\\n\");\n";

				} elsif ($ft eq "MFString") {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft): \\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { spacer fprintf (fp,\"\t\t\t%d: \\t%s\\n\",i,tmp->$field.p[i]->strptr); }\n";
				} elsif ($ft eq "MFNode") {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { dump_scene(fp,level+1,tmp->$field.p[i]); }\n";

				} elsif (($ft eq "MFInt32") || ($ft eq "MFBool")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { spacer fprintf (fp,\"\t\t\t%d: \\t%d\\n\",i,tmp->$field.p[i]); }\n";
				} elsif (($ft eq "MFFloat") || ($ft eq "MFTime") || ($ft eq "MFDouble")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { spacer fprintf (fp,\"\t\t\t%d: \\t%4.3f\\n\",i,tmp->$field.p[i]); }\n";
				} elsif (($ft eq "MFVec3f") || ($ft eq "MFColor") || ($ft eq "MFVec3d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { spacer ".
						"fprintf (fp,\"\t\t\t%d: \\t[%4.3f, %4.3f, %4.3f]\\n\",i,(tmp->$field.p[i]).c[0], (tmp->$field.p[i]).c[1],(tmp->$field.p[i]).c[2]); }\n";
				} elsif (($ft eq "MFVec2f") || ($ft eq "MFVec2d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { spacer ".
						"fprintf (fp,\"\t\t\t%d: \\t[%4.3f, %4.3f]\\n\",i,(tmp->$field.p[i]).c[0], (tmp->$field.p[i]).c[1]); }\n";

				} elsif (($ft eq "MFRotation") || ($ft eq "MFColorRGBA")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { spacer ".
						"fprintf (fp,\"\t\t\t%d: \\t[%4.3f, %4.3f, %4.3f, %4.3f]\\n\",i,(tmp->$field.p[i]).c[0], (tmp->$field.p[i]).c[1],(tmp->$field.p[i]).c[2],(tmp->$field.p[i]).c[3]); }\n";


				} elsif (($ft eq "MFVec4f") || ($ft eq "MFVec4d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { spacer ".
						"fprintf (fp,\"\t\t\t%d: \\t[%4.3f, %4.3f, %4.3f, %4.3f]\\n\",i,(tmp->$field.p[i]).c[0], (tmp->$field.p[i]).c[1],(tmp->$field.p[i]).c[2],(tmp->$field.p[i]).c[3]); }\n";


				} elsif (($ft eq "MFMatrix4f") || ($ft eq "MFMatrix4d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { spacer ".
						"fprintf (fp,\"\t\t\t%d: \\t[%4.3f, %4.3f, %4.3f, %4.3f, %4.3f,  %4.3f,  %4.3f,  %4.3f,  %4.3f, %4.3f, %4.3f, %4.3f, %4.3f, %4.3f,  %4.3f,  %4.3f ]\\n\",i,(tmp->$field.p[i]).c[0], (tmp->$field.p[i]).c[1],(tmp->$field.p[i]).c[2],(tmp->$field.p[i]).c[3],(tmp->$field.p[i]).c[4],(tmp->$field.p[i]).c[5],(tmp->$field.p[i]).c[6],(tmp->$field.p[i]).c[7],(tmp->$field.p[i]).c[8],(tmp->$field.p[i]).c[9],(tmp->$field.p[i]).c[10],(tmp->$field.p[i]).c[11],(tmp->$field.p[i]).c[12],(tmp->$field.p[i]).c[13],(tmp->$field.p[i]).c[14],(tmp->$field.p[i]).c[15]); }\n";

				} elsif (($ft eq "MFMatrix3f") || ($ft eq "MFMatrix3d")) {
					push @genFuncs2, "\t\t\tspacer fprintf (fp,\"\\t$field ($ft):\\n\");\n";
                        		push @genFuncs2, "\t\t\tfor (i=0; i<tmp->$field.n; i++) { spacer ".
						"fprintf (fp,\"\t\t\t%d: \\t[%4.3f, %4.3f, %4.3f, %4.3f, %4.3f,  %4.3f,  %4.3f,  %4.3f,  %4.3f ]\\n\",i,(tmp->$field.p[i]).c[0], (tmp->$field.p[i]).c[1],(tmp->$field.p[i]).c[2],(tmp->$field.p[i]).c[3],(tmp->$field.p[i]).c[4],(tmp->$field.p[i]).c[5],(tmp->$field.p[i]).c[6],(tmp->$field.p[i]).c[7],(tmp->$field.p[i]).c[8]); }\n";



				} else {
					print "type $ft not handled yet\n";
				}
				if (($firstC eq "_") || $field eq "metadata") {
					push @genFuncs2, "\t\t    }\n";
				}
			}
		}

		push @genFuncs2, "		    break;\n\t\t}\n";
	}
	push @genFuncs2, "		default: {}\n";

	push @genFuncs2, " }\n fflush(fp) ;\n spacer fprintf (fp,\"L%d end\\n\",level);\n if (level == 0) fprintf (fp,\"ending dump_scene\\n\");\n}\n";
	
	#####################
	# create an array for each node. The array contains the following:
	# const int OFFSETS_Text[
	# 	FIELDNAMES_string, offsetof (struct X3D_Text, string), MFSTRING, KW_inputOutput,
	#	FIELDNAMES_fontStype, offsetof (struct X3D_Text, fontStyle, SFNODE, KW_inputOutput,
	# ....
	# 	-1, -1, -1, -1];
	# NOTES:
	# 1) we skip any field starting with an "_" (underscore)
	# 
	for my $node (@sortedNodeList) {
		#print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.

		push @genFuncs1, "\nconst int OFFSETS_".$node."[] = {\n";

 		foreach my $field (keys %{$VRML::Nodes{$node}{Defaults}}) {
			#if (index($field,"_") !=0) {
				my $ft = $VRML::Nodes{$node}{FieldTypes}{$field};
				#$ft =~ tr/a-z/A-Z/; # convert to uppercase
				my $fk = $VRML::Nodes{$node}{FieldKinds}{$field};
				my $specVersion = $VRML::Nodes{$node}{SpecLevel}{$field};
				push @genFuncs1, "	(int) FIELDNAMES_$field, (int) offsetof (struct X3D_$node, $field), ".
					" (int) FIELDTYPE_$ft, (int) KW_$fk, (int) $specVersion,\n";
			#}
		};
		push @genFuncs1, "	-1, -1, -1, -1, -1};\n";
	}
	#####################
	# create an array for each node. The array contains the following:
	# const int OFFSETS_Text[
	# 	FIELDNAMES_string, offsetof (struct X3D_Text, string), MFSTRING, KW_inputOutput,
	#	FIELDNAMES_fontStype, offsetof (struct X3D_Text, fontStyle, SFNODE, KW_inputOutput,
	# ....
	# 	-1, -1, -1, -1];
	# NOTES:
	# 1) we skip any field starting with an "_" (underscore)
	#####################
	# make an array that contains all of the OFFSETS created above.
	push @str, "\nextern const int *NODE_OFFSETS[];\n";
	push @genFuncs1, "\nconst int *NODE_OFFSETS[] = {\n";
	for my $node (@sortedNodeList) {
		push @genFuncs1, "	OFFSETS_$node,\n";
	}
	push @genFuncs1, "	};\n";


	#####################
	# make the NodeFields.h for the C VRML parser.

	
	for my $node (@sortedNodeList) {
		#print "node $_ is tagged as $nodeIntegerType\n";
		# tag each node type with a integer key.

		push @fieldNodes, "\n/* $node node */\n";
		push @fieldNodes, "BEGIN_NODE($node)\n";

 		foreach my $field (keys %{$VRML::Nodes{$node}{Defaults}}) {
			if (index($field,"_") !=0) {
				my $fk = "";
				my $ofk = $VRML::Nodes{$node}{FieldKinds}{$field};
				if ("outputOnly" eq $ofk)     {$fk = "EVENT_OUT";}
				if ("inputOnly" eq $ofk)      {$fk = "EVENT_IN";}
				if ("inputOutput" eq $ofk) {$fk = "EXPOSED_FIELD";}
				if ("initializeOnly" eq $ofk)        {$fk = "FIELD";}

				if ("" eq $fk) {
					print "error in fieldKind for node $node, was $ofk\n";
				}

				my $ft = $VRML::Nodes{$node}{FieldTypes}{$field};
				my $origFt = "FIELDTYPE_".$VRML::Nodes{$node}{FieldTypes}{$field};
				$ft =~ tr/A-Z/a-z/; # convert to lowercase

				push @fieldNodes, "$fk($node,$field,$ft,$field,$origFt)\n";
			}
		};
		push @fieldNodes, "END_NODE($node)\n";
	}



	#####################
	# create a function to return the X3D component for each node type
	push @str, "\nint getSAI_X3DNodeType (int FreeWRLNodeType);\n";
	push @genFuncs2, "\nint getSAI_X3DNodeType (int FreeWRLNodeType) {\n\tswitch (FreeWRLNodeType) {\n";
	for my $node (@sortedNodeList) {
		push @genFuncs2, "	case NODE_$node: return ".
				$VRML::Nodes{$node}{X3DNodeType}."; break;\n";
	}
	push @genFuncs2,"\tdefault:return -1;\n\t}\n}\n";


	#####################
	# Scenegraph/GeneratedCode.c
	#
	open_possible_cvs_file(GENFUNC, "../src/lib/scenegraph/GeneratedCode.c");
	print GENFUNC '/*
  $I'.'d: $

  GeneratedCode.c: generated by VRMLC.pm. DO NOT MODIFY, MODIFY VRMLC.pm INSTEAD.

*/
';
	print GENFUNC @license_block;

	print GENFUNC join '',@genFuncs1;
	print GENFUNC join '',@genFuncs2;

	close GENFUNC;

	#####################
	# libeai/GeneratedCode.c
	#
	open_possible_cvs_file(GENFUNC, "../src/libeai/GeneratedCode.c");
	print GENFUNC '/*
  $I'.'d: $

  GeneratedCode.c: generated by VRMLC.pm. DO NOT MODIFY, MODIFY VRMLC.pm INSTEAD.

*/
';

	print GENFUNC @license_block;

	print GENFUNC join '',@EAICommon;

	close GENFUNC;

	#####################
	# vrml_parser/NodeFields.h
	#
	open_possible_cvs_file(FIELDNODES, "../src/lib/vrml_parser/NodeFields.h");
	print FIELDNODES '/*
  $I'.'d: $

  NodeFields.h: generated by VRMLC.pm. DO NOT MODIFY, MODIFY VRMLC.pm INSTEAD.

*/
';

	print FIELDNODES @license_block;

	print FIELDNODES '
/*
Information for all the fields of all the nodes.  

Format is as follows:
BEGIN_NODE(NodeName)
 FIELD(Node, field, type, varToAssignInStruct)
 EXPOSED_FIELD(Node, field, type, varToAssignInStruct)
 EVENT_IN(Node, event, type, varToAssignInStruct)
 EVENT_OUT(Node, event, type, varToAssignInStruct)
END_NODE(NodeName)

*/

';

	print FIELDNODES join '',@fieldNodes;

	close FIELDNODES;

	#####################
	# vrml_parser/Structs.h
	#
	open_possible_cvs_file(STRUCTS, "../src/lib/vrml_parser/Structs.h");
	print STRUCTS '/*
  $I'.'d: $

  Structs.h: generated by VRMLC.pm. DO NOT MODIFY, MODIFY VRMLC.pm INSTEAD.

  Code here comes almost verbatim from VRMLC.pm

*/
';

	print STRUCTS @license_block;

	print STRUCTS '
#ifndef __FREEWRL_STRUCTS_H__
#define __FREEWRL_STRUCTS_H__


struct point_XYZ {GLDOUBLE x,y,z;};
struct orient_XYZA {GLDOUBLE x,y,z,a;};

struct X3D_Virt {
	void (*prep)(void *);
	void (*rend)(void *);
	void (*children)(void *);
	void (*fin)(void *);
	void (*rendray)(void *);
	void (*mkpolyrep)(void *);
	void (*proximity)(void *);
	void (*other)(void *);
	void (*collision)(void *);
	void (*compile)(void *, void *, void *, void *, void *);
};

/* a string is stored as a pointer, and a length of that mallocd pointer */
struct Uni_String {
	int len;
	char * strptr;
	int touched;
};

/* Internal representation of IndexedFaceSet, Text, Extrusion & ElevationGrid:
 * set of triangles.
 * done so that we get rid of concave polygons etc.
 */
struct X3D_PolyRep { /* Currently a bit wasteful, because copying */
	int irep_change;
	int ccw;	/* ccw field for single faced structures */
	int ntri; /* number of triangles */
	int streamed;	/* is this done the streaming pass? */
	int alloc_tri; /* number of allocated triangles */

	/* indicies for arrays. OpenGL ES 2.0 - unsigned short for the DrawArrays call */
	GLuint *cindex;   /* triples (per triangle) */
	GLuint *colindex;   /* triples (per triangle) */
	GLuint *norindex;
        GLuint *tcindex; /* triples or null */

	float *actualCoord; /* triples (per point) */
	float *color; /* triples or null */
	float *normal; /* triples or null */
        float *GeneratedTexCoords;	/* triples (per triangle) of texture coords if there is no texCoord node */
	int tcoordtype; /* type of texture coord node - is this a NODE_TextureCoordGenerator... */
	GLfloat minVals[3];		/* for collision and default texture coord generation */
	GLfloat maxVals[3];		/* for collision and default texture coord generation */
	GLfloat transparency;		/* what the transparency value was during compile, put in color array if RGBA colors */
	int isRGBAcolorNode;		/* color was originally an RGBA, DO NOT re-write if transparency changes */
	GLuint VBO_buffers[VBO_COUNT];		/* VBO indexen */
};

/* viewer dimentions (for collision detection) */
struct sNaviInfo {
        double width;
        double height;
        double step;
};

';

	# print out the generated structures
	print STRUCTS join '',@NODEDEFS;
	print STRUCTS join '',@str;

	print STRUCTS '
#endif /* __FREEWRL_STRUCTS_H__ */
';

	close STRUCTS;
}


gen();


