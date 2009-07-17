/*
=INSERT_TEMPLATE_HERE=

$Id: statusbar.c,v 1.7 2009/07/17 15:07:04 crc_canada Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"

#include <float.h>

#include "../x3d_parser/Bindable.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"


/* DO NOT CHANGE THESE DEFINES WITHOUT CHECKING THE USE OF THE CODE, BELOW */
#define PROX "#VRML V2.0 utf8\rProximitySensor { enabled FALSE size 1000 1000 1000 }"

/* put the text (second translation) back behind where the clip plane will be (the z axis) and down near the bottom of the screen a bit */
/* look at the gluPerspective(fieldofview, screenRatio, nearPlane, farPlane); line in MainLoop.c */

#define TEXT "#VRML V2.0 utf8\rTransform{translation 0 0 9.9 children[Collision{collide FALSE children [Transform{scale 0.35 0.35 1 translation 0 -0.06 -0.11 children[Shape{geometry Text{fontStyle FontStyle{justify \"MIDDLE\" size 0.02}}}]}]}]}"



static int sb_initialized = FALSE;
static struct Uni_String *myline;
void render_init(void);
static void statusbar_init(void);
static struct X3D_ProximitySensor *proxNode = NULL;
static struct X3D_Transform *transNode =NULL;
static struct X3D_Text *textNode = NULL;

#define STATUS_LEN 2000

/* make sure that on a re-load that we re-init */
void kill_status (void) {
	/* hopefully, by this time, rendering has been stopped */
	sb_initialized = FALSE;
}


/* trigger a update */
void update_status(char* msg) {
	#ifdef VERBOSE
	printf ("update status, msg :%s:\n",msg);
	#endif

	if (!sb_initialized) {
		if (rootNode == NULL) return; /* system not running yet?? */
		statusbar_init();
	}

	/* did we initialize correctly? */
	if (proxNode == NULL) return;

	/* bounds check here - if the string is this long, it deserves to be ignored! */
	if (strlen(msg) > (STATUS_LEN-10)) return;

	strcpy(myline->strptr, msg);
	myline->len = strlen(msg)+1; /* length of message, plus the null terminator */
	#ifdef VERBOSE
	printf("myline-> strptr is %s, len is %d\n", myline->strptr, myline->len);
	#endif
	
	/* if this is a null string, dont bother running the ProximitySensor. */
	proxNode->enabled = (myline->len != 1);

	update_node((void*) textNode);
}

/* render the status bar. If it is required... */ 
static void statusbar_init() {
	int tmp;
	struct X3D_Group * myn;
	struct X3D_Node *tempn;

	/* put a ProximitySensor and text string in there. */
	myn = createNewX3DNode(NODE_Group);
	inputParse(FROMSTRING, PROX, FALSE, FALSE, myn, offsetof(struct X3D_Group, children), &tmp, FALSE);

	/* is there some error parsing? */
	if (myn->children.n==0) {
		sb_initialized = TRUE; 	/* dont bother again */
		proxNode = NULL; 	/* ENSURE that the update knows that this failed */
		return;			/* and get out of here */
	}


	/* get the ProximitySensor node from this parse. Note, errors are not checked. If this gives an errror,
	   then there are REALLY bad things happening somewhere else */

	/* remove this ProximitySensor node from the temporary variable, and reset the temp. variable */

printf ("myn %u, type %s\n",myn, stringNodeType (myn->_nodeType));
printf ("myn has %d children\n",myn->children.n);

	proxNode = myn->children.p[0];

	/* turn it off for now, until a non-zero length string comes in */
	proxNode->enabled = FALSE;
	myn->children.n = 0;

	inputParse(FROMSTRING, TEXT, FALSE, FALSE, myn, offsetof(struct X3D_Group, children), &tmp, FALSE);
	transNode = myn->children.p[0];

	/* because the routes will not act immediately, we need to place this manually first time */
	/*
	transNode->translation.c[0] = Viewer.Pos.x;
	transNode->translation.c[1] = Viewer.Pos.y;
	transNode->translation.c[2] = Viewer.Pos.z;
	transNode->rotation.r[0] = 0.000347;
	transNode->rotation.r[1] = 0.011514;
	transNode->rotation.r[2] =  -0.000348;
	transNode->rotation.r[3] = -0.011525;
	transNode->__do_trans = TRUE;
	transNode->__do_rotation = TRUE;
	*/



	myn->children.n = 0;

	/* get the Text node, as a pointer. The TEXT definition, above, gives us the following:
		Transform(children)->Collision(children)->Transform(childen)->Shape(geometry)->Text */

	tempn = X3D_NODE(transNode->children.p[0]);
	/* printf ("step 1; we should be at a Collision node: %s\n",stringNodeType(tempn->_nodeType)); */

	tempn =  X3D_NODE(((struct X3D_Collision *)tempn)->children.p[0]);
	/* printf ("step 2; we should be at a Transform node: %s\n",stringNodeType(tempn->_nodeType)); */

	tempn =  X3D_NODE(((struct X3D_Transform *)tempn)->children.p[0]);
	/* printf ("step 3; we should be at a Shape node: %s\n",stringNodeType(tempn->_nodeType)); */

	textNode  =  (struct X3D_Text*) ((struct X3D_Shape *)tempn)->geometry;
	/* printf ("step 4; we should be at a text node: %s\n",stringNodeType(textNode->_nodeType)); */


	/* create a 1 UniString entry to the MFString */
	textNode->string.p = MALLOC(sizeof (struct Uni_String));
	textNode->string.p[0] = newASCIIString("");	/* first string is blank */
	textNode->string.n = 1; 				/* we have 1 string in this X3D_Text node */
	myline=(struct Uni_String *)textNode->string.p[0];

	/* NOW - make the Uni_String large... in the first Unistring, make the string 2000 bytes long */
	myline->strptr  = MALLOC(STATUS_LEN);

	/* set the Uni_String to zero length */
	myline->len = 0;
	AddRemoveChildren(rootNode, rootNode+offsetof (struct X3D_Group, children), (uintptr_t*)&proxNode, 1, 1,__FILE__,__LINE__);
	AddRemoveChildren(rootNode, rootNode+offsetof (struct X3D_Group, children), (uintptr_t*)&transNode, 1, 1,__FILE__,__LINE__);

	CRoutes_RegisterSimple((void *)proxNode, offsetof (struct X3D_ProximitySensor, orientation_changed), 
		(void *)transNode, offsetof (struct X3D_Transform, rotation), sizeof (struct SFRotation));

	CRoutes_RegisterSimple((void *)proxNode, offsetof (struct X3D_ProximitySensor, position_changed), 
		(void *)transNode, offsetof (struct X3D_Transform, translation), sizeof (struct SFColor));
	
	sb_initialized = TRUE;
}
