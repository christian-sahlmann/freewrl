/*
=INSERT_TEMPLATE_HERE=

$Id: Textures.c,v 1.15 2009/07/24 19:46:19 crc_canada Exp $

General Texture objects.

*/

#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"

#include "../scenegraph/readpng.h"
#include "../input/InputFunctions.h"
#include "Textures.h"
#include "../opengl/Material.h"


#ifdef AQUA
# include <Carbon/Carbon.h>
# include <QuickTime/QuickTime.h>
#else
# if HAVE_JPEGLIB_H
#  include <jpeglib.h>
#  include <setjmp.h>
# endif
#endif


#define DO_POSSIBLE_TEXTURE_SEQUENCE if (myTableIndex->status == TEX_NEEDSBINDING) { \
                do_possible_textureSequence(myTableIndex); \
                return;	\
		}
/* lets check the max texture size */
static int checktexsize;

/* note - we reduce the max texture size on computers with the (incredibly inept) Intel GMA 9xx chipsets - like the Intel
   Mac minis, and macbooks up to November 2007 */
#define CHECK_MAX_TEXTURE_SIZE \
	if (global_texSize<=0) { \
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &checktexsize); \
		if (getenv("FREEWRL_256x256_TEXTURES")!= NULL) checktexsize = 256; \
		if (getenv("FREEWRL_512x512_TEXTURES")!= NULL) checktexsize = 512; \
		global_texSize = -global_texSize; \
		if (global_texSize == 0) global_texSize = checktexsize; \
		if (global_texSize > checktexsize) global_texSize = checktexsize; \
		if (strncmp(glGetString(GL_RENDERER),"NVIDIA GeForce2",strlen("NVIDIA GeForce2")) == 0) { \
		/* 	printf ("possibly reducing texture size because of NVIDIA GeForce2 chip\n"); */ \
			if (global_texSize > 1024) global_texSize = 1024; \
		}  \
		if (strncmp(glGetString(GL_RENDERER),"Intel GMA 9",strlen("Intel GMA 9")) == 0) { \
		/* 	printf ("possibly reducing texture size because of Intel GMA chip\n"); */ \
			if (global_texSize > 1024) global_texSize = 1024; \
		}  \
		if (displayOpenGLErrors) printf ("CHECK_MAX_TEXTURE_SIZE, ren %s ver %s ven %s ts %d\n",glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_VENDOR),global_texSize); \
		setMenuButton_texSize (global_texSize); \
	} 


/* we keep track of which textures have been loaded, and which have not */
extern void *texParams[];

/* newer Texture handling procedures */
/* each texture has this kind of structure */
struct textureTableIndexStruct {
	struct	X3D_Node*	scenegraphNode;
	int			nodeType;
	int	imageType;
	int 	status;
	int	depth;
	int 	hasAlpha;
	GLuint	*OpenGLTexture;
	int	frames;
	char    *filename;
        int x;
        int y;
        unsigned char *texdata;
	struct Multi_Int32 *pixelData;
        GLint Src;
        GLint Trc;
};
#define PNGTexture 200
#define JPGTexture 300

/* each block of allocated code contains this... */
struct textureTableStruct {
	struct textureTableStruct * next;
	struct textureTableIndexStruct entry[32];
};
struct textureTableStruct* readTextureTable = NULL;

static int nextFreeTexture = 0;
char *workingOnFileName = NULL;
static void new_bind_image(struct X3D_Node *node, void *param);
struct textureTableIndexStruct *getTableIndex(int i);
struct textureTableIndexStruct* loadThisTexture;

static struct Multi_Int32 invalidFilePixelDataNode;
static int	invalidFilePixelData[] = {1,1,3,0x707070};

/* threading variables for loading textures in threads */
pthread_t loadThread = 0;
static pthread_mutex_t texmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t texcond   = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t genmutex = PTHREAD_MUTEX_INITIALIZER;
#define TLOCK 		pthread_mutex_lock(&texmutex);
#define TUNLOCK 	pthread_mutex_unlock(&texmutex);
#define T_LOCK_SIGNAL 	pthread_cond_signal(&texcond);
#define T_LOCK_WAIT	pthread_cond_wait(&texcond,&texmutex);
/* lock the reallocs of data structures */
#define REGENLOCK 	pthread_mutex_lock(&genmutex);
#define REGENUNLOCK 	pthread_mutex_unlock(&genmutex);

/* is the texture thread up and running yet? */
int TextureThreadInitialized = FALSE;


/* are we currently active? */
int TextureParsing = FALSE;

/* current index into loadparams that texture thread is working on */
int currentlyWorkingOn = -1;

int textureInProcess = -1;

/* how many texel units; if -1, we have not tried to find out yet */
GLint maxTexelUnits = -1;


/* for texture remapping in TextureCoordinate nodes */
int	*global_tcin;
int	global_tcin_count;
void 	*global_tcin_lastParent;

#ifdef AQUA /* for AQUA OS X sharing of OpenGL Contexts */

/* # include "CGDirectDisplay.h" */

CGLPixelFormatAttribute attribs[] = { kCGLPFADisplayMask, 0,
                                      kCGLPFAFullScreen,
                                      kCGLPFADoubleBuffer,
                                      0 };

CGLPixelFormatObj pixelFormat = NULL;
long numPixelFormats = 0;
CGLContextObj aqtextureContext = NULL;

#else

GLXContext textureContext = NULL;

#endif

/* function Prototypes */
static int findTextureFile (int cwo);
void _textureThread(void);
void store_tex_info(
		struct textureTableIndexStruct *me,
		int depth,
		int x,
		int y,
		unsigned char *ptr,
		int hasAlpha);

static void __reallyloadPixelTexure(void);
static void __reallyloadImageTexture(void);
static void __reallyloadMovieTexture(void);
void do_possible_textureSequence(struct textureTableIndexStruct*);
struct Uni_String *newASCIIString(char *str);

int readpng_init(FILE *infile, ulg *pWidth, ulg *pHeight);
void readpng_cleanup(int free_image_data);


/************************************************************************/
/* start up the texture thread */
void initializeTextureThread() {
	if (loadThread == 0) {
		pthread_create (&loadThread, NULL, (void*(*)(void *))&_textureThread, NULL);
	}
}

/* does a texture have alpha?  - pass in a __tableIndex from a MovieTexture, ImageTexture or PixelTexture. */
int isTextureAlpha(int texno) {
	struct textureTableIndexStruct *ti;

	/* no, have not even started looking at this */
	/* if (texno == 0) return FALSE; */

	ti = getTableIndex(texno);
	if (ti->status==TEX_LOADED) {
		return ti->hasAlpha;
	}
	return FALSE;
}


/* is the texture thread initialized yet? */
int isTextureinitialized() {
	return TextureThreadInitialized;
}

/* is this texture loaded? used in LoadSensor */
int isTextureLoaded(int texno) {
	struct textureTableIndexStruct *ti;

	/* no, have not even started looking at this */
	/* if (texno == 0) return FALSE; */

	ti = getTableIndex(texno);
	return (ti->status==TEX_LOADED);
}

/* statusbar uses this to tell user that we are still loading */
int isTextureParsing() {
	/* return currentlyWorkingOn>=0; */
#ifdef TEXVERBOSE 
	printf("call to isTextureParsing %d, returning %d\n",
	       textureInProcess,textureInProcess > 0);
#endif
	return textureInProcess >0;
}

/* can our OpenGL implementation handle multitexturing?? */
void init_multitexture_handling() {
	char *glExtensions;

	/* first, lets check to see if we have a max texture size yet */
	CHECK_MAX_TEXTURE_SIZE;

	glExtensions = (char *)glGetString(GL_EXTENSIONS);

	if ((strstr (glExtensions, "GL_ARB_texture_env_combine")!=0) &&
		(strstr (glExtensions,"GL_ARB_multitexture")!=0)) {

		glGetIntegerv(GL_MAX_TEXTURE_UNITS,&maxTexelUnits);

		if (maxTexelUnits > MAX_MULTITEXTURE) {
			printf ("init_multitexture_handling - reducing number of multitexs from %d to %d\n",
				maxTexelUnits,MAX_MULTITEXTURE);
			maxTexelUnits = MAX_MULTITEXTURE;
		}
#ifdef TEXVERBOSE
		printf ("can do multitexture we have %d units\n",maxTexelUnits);
#endif


		/* we assume that GL_TEXTURE*_ARB are sequential. Lets to a little check */
		if ((GL_TEXTURE0 +1) != GL_TEXTURE1) {
			printf ("Warning, code expects GL_TEXTURE0 to be 1 less than GL_TEXTURE1\n");
		} 

	} else {
		printf ("can not do multitexture\n");
		maxTexelUnits = 0;
	}
}

/* this node has changed - if there was a texture, destroy it */
void releaseTexture(struct X3D_Node *node) {

	int tableIndex;
	struct textureTableIndexStruct *ti;

		if (node->_nodeType == NODE_ImageTexture) {
			tableIndex  = ((struct X3D_ImageTexture *)node)->__textureTableIndex;
		} else if (node->_nodeType == NODE_PixelTexture) {
			tableIndex  = ((struct X3D_PixelTexture *)node)->__textureTableIndex;
		} else if (node->_nodeType == NODE_MovieTexture) {
			tableIndex  = ((struct X3D_MovieTexture *)node)->__textureTableIndex;
		} else return;

#ifdef TEXVERBOSE
	printf ("releaseTexture, calling getTableIndex\n");
	ti = getTableIndex(tableIndex);
	printf ("releaseTexture, ti %u, ti->status %d\n",ti,ti->status);
	ti->status = TEX_NOTLOADED;

	if (ti->OpenGLTexture != NULL) {
		printf ("deleting %d textures, starting at %d\n",ti->frames, *(ti->OpenGLTexture));
		FREE_IF_NZ(ti->OpenGLTexture);
	}
#endif

	ti = getTableIndex(tableIndex);
	ti->status = TEX_NOTLOADED;
	if (ti->OpenGLTexture != NULL) {
		glDeleteTextures(ti->frames, ti->OpenGLTexture);
		FREE_IF_NZ(ti->OpenGLTexture);
	}
}

/* called on "kill oldworld" */
void kill_openGLTextures() {
	int count;
	struct textureTableStruct * listRunner;
	struct textureTableStruct * tmp;

	/* remove the OpenGL textures */
	listRunner = readTextureTable;

	while (listRunner != NULL) {
		/* zero out the fields in this new block */
		for (count = 0; count < 32; count ++) {
			if  (listRunner->entry[count].OpenGLTexture != NULL) {
#ifdef TEXVERBOSE
				printf ("deleting %d %d\n",listRunner->entry[count].frames, 
					listRunner->entry[count].OpenGLTexture);
#endif

				glDeleteTextures(listRunner->entry[count].frames, 
						 listRunner->entry[count].OpenGLTexture);
				FREE_IF_NZ (listRunner->entry[count].OpenGLTexture);
				listRunner->entry[count].OpenGLTexture = NULL;
				listRunner->entry[count].frames = 0;
			}
		}
		listRunner = listRunner->next;
	}

	/* now, delete the tables themselves */
	listRunner = readTextureTable;
	readTextureTable = NULL;
	nextFreeTexture = 0;
	while (listRunner != NULL) {
		tmp = listRunner;
		listRunner = listRunner->next;
		FREE_IF_NZ (tmp);
	}
}

/* copy the pixel raw pointer over- we need to store this here, as it is
   possible (if an external texture is invalid) that an ImageTexture or
   MovieTexture can "change" into a small PixelTexture indicating an error.
   This saves having missing geometry in the scene. */
void copyPixelTextureEntry (struct X3D_PixelTexture *me) {
	struct textureTableIndexStruct *myEntry;
	struct Multi_Int32* ptr;

#ifdef TEXVERBOSE
	printf ("copying pixelTexture entry calling getTableIndex\n");
#endif

	ptr = &(me->image);
	myEntry = getTableIndex(me->__textureTableIndex);
	myEntry->pixelData =  ptr;
	myEntry->status = TEX_NOTLOADED;
}


/* find ourselves - given an index, return the struct */
struct textureTableIndexStruct *getTableIndex(int indx) {
	int count;
	int whichBlock;
	int whichEntry;
	struct textureTableStruct * currentBlock;

	whichBlock = (indx & 0xffe0) >> 5;
	whichEntry = indx & 0x1f;
#ifdef TEXVERBOSE
	printf ("getTableIndex locating table entry %d\n",indx);
		printf ("whichBlock = %d, wichEntry = %d ",whichBlock, whichEntry);
#endif

	currentBlock = readTextureTable;
	for (count=0; count<whichBlock; count++) currentBlock = currentBlock->next;

#ifdef TEXVERBOSE
	printf ("getTableIndex, going to return %d\n", &(currentBlock->entry[whichEntry]));
	printf ("textureTableIndexStruct, sgn0 is %d, sgn1 is %d\n",
		currentBlock->entry[0].scenegraphNode,
		currentBlock->entry[1].scenegraphNode);
#endif
	return &(currentBlock->entry[whichEntry]);
}



/* is this node a texture node? if so, lets keep track of its textures. */
/* worry about threads - do not make anything reallocable */
void registerTexture(struct X3D_Node *tmp) {
	struct X3D_ImageTexture *it;
	struct X3D_PixelTexture *pt;
	struct X3D_MovieTexture *mt;
	struct X3D_VRML1_Texture2 *v1t;

	struct textureTableStruct * listRunner;
	struct textureTableStruct * newStruct;
	struct textureTableStruct * currentBlock;
	int count;
	int whichBlock;
	int whichEntry;

	it = (struct X3D_ImageTexture *) tmp;
	/* printf ("registerTexture, found a %s\n",stringNodeType(it->_nodeType)); */

	if ((it->_nodeType == NODE_ImageTexture) || (it->_nodeType == NODE_PixelTexture) ||
		(it->_nodeType == NODE_MovieTexture) || (it->_nodeType == NODE_VRML1_Texture2)) {

		if ((nextFreeTexture & 0x1f) == 0) {
			newStruct = (struct textureTableStruct*) MALLOC (sizeof (struct textureTableStruct));
			
			/* zero out the fields in this new block */
			for (count = 0; count < 32; count ++) {
				/* newStruct->entry[count].nodeType = 0; */
				newStruct->entry[count].status = TEX_NOTLOADED;
				newStruct->entry[count].OpenGLTexture = NULL;
				newStruct->entry[count].frames = 0;
				newStruct->entry[count].depth = 0;
				newStruct->entry[count].scenegraphNode = NULL;
				newStruct->entry[count].filename = NULL;
				newStruct->entry[count].nodeType = 0;
			}
			
			newStruct->next = NULL;
			
			/* link this one in */
			listRunner = readTextureTable;
			if (listRunner == NULL) readTextureTable = newStruct;
			else {
				while (listRunner->next != NULL) 
					listRunner = listRunner->next;
				listRunner->next = newStruct;
			}
		}

		/* record the info for this texture. */
		whichBlock = (nextFreeTexture & 0xffe0) >> 5;
		whichEntry = nextFreeTexture & 0x1f;

		currentBlock = readTextureTable;
		for (count=0; count<whichBlock; count++) currentBlock = currentBlock->next;

		/* save this index in the scene graph node */
		if (it->_nodeType == NODE_ImageTexture) {
			it->__textureTableIndex = nextFreeTexture;
		} else if (it->_nodeType == NODE_PixelTexture) {
			pt = (struct X3D_PixelTexture *) tmp;
			pt->__textureTableIndex = nextFreeTexture;
		} else if (it->_nodeType == NODE_MovieTexture) {
			mt = (struct X3D_MovieTexture *) tmp;
			mt->__textureTableIndex = nextFreeTexture;
		} else if (it->_nodeType == NODE_VRML1_Texture2) {
			v1t = (struct X3D_VRM1_Texture2 *) tmp;
			v1t->__textureTableIndex = nextFreeTexture;
		}

		currentBlock->entry[whichEntry].nodeType = it->_nodeType;
		/* set the scenegraphNode here */
		currentBlock->entry[whichEntry].scenegraphNode = X3D_NODE(tmp);
#ifdef TEXVERBOSE
		printf ("registerNode, sgn0 is %d, sgn1 is %d\n",
			currentBlock->entry[0].scenegraphNode,
			currentBlock->entry[1].scenegraphNode);
#endif
		/* now, lets increment for the next texture... */
		nextFreeTexture += 1;
	}
}

/* do TextureBackground textures, if possible */
void loadBackgroundTextures (struct X3D_Background *node) {
	struct X3D_ImageTexture *thistex = 0;
	struct Multi_String thisurl;
	int count;

	for (count=0; count<6; count++) {
		/* go through these, back, front, top, bottom, right left */
		switch (count) {
			case 0: {thistex = node->__frontTexture;  thisurl = node->frontUrl; break;}
			case 1: {thistex = node->__backTexture;   thisurl = node->backUrl; break;}
			case 2: {thistex = node->__topTexture;    thisurl = node->topUrl; break;}
			case 3: {thistex = node->__bottomTexture; thisurl = node->bottomUrl; break;}
			case 4: {thistex = node->__rightTexture;  thisurl = node->rightUrl; break;}
			case 5: {thistex = node->__leftTexture;   thisurl = node->leftUrl; break;}
		}
		if (thisurl.n != 0 ) {
			/* we might have to create a "shadow" node for the image texture */
			if (thistex == NULL) {
				int i;
				thistex = createNewX3DNode(NODE_ImageTexture);

#ifdef TEXVERBOSE
				printf ("bg, creating shadow texture node url.n = %d\n",thisurl.n);
#endif

				/* copy over the urls */
				thistex->url.p = MALLOC(sizeof (struct Uni_String) * thisurl.n);
				for (i=0; i<thisurl.n; i++) {
					thistex->url.p[i] = newASCIIString (thisurl.p[i]->strptr);
				}
				thistex->url.n = thisurl.n;

				switch (count) {
					case 0: {node->__frontTexture = thistex;  break;}
					case 1: {node->__backTexture = thistex;   break;}
					case 2: {node->__topTexture = thistex;    break;}
					case 3: {node->__bottomTexture = thistex; break;}
					case 4: {node->__rightTexture = thistex;  break;}
					case 5: {node->__leftTexture = thistex;   break;}
				}
			}

			/* we have an image specified for this face */
			texture_count = 0;
			/* render the proper texture */
			render_node((void *)thistex);
		        glColor3d(1.0,1.0,1.0);

        		textureDraw_start(NULL,Backtex);
        		glVertexPointer (3,GL_FLOAT,0,BackgroundVert);
        		glNormalPointer (GL_FLOAT,0,Backnorms);

        		FW_GL_DRAWARRAYS (GL_QUADS, count*4, 4);
        		textureDraw_end();
		}
	}
}

/* do TextureBackground textures, if possible */
void loadTextureBackgroundTextures (struct X3D_TextureBackground *node) {
	struct X3D_Node *thistex = 0;
	int count;

	for (count=0; count<6; count++) {
		/* go through these, back, front, top, bottom, right left */
		switch (count) {
			case 0: {POSSIBLE_PROTO_EXPANSION(node->frontTexture,thistex);  break;}
			case 1: {POSSIBLE_PROTO_EXPANSION(node->backTexture,thistex);   break;}
			case 2: {POSSIBLE_PROTO_EXPANSION(node->topTexture,thistex);    break;}
			case 3: {POSSIBLE_PROTO_EXPANSION(node->bottomTexture,thistex); break;}
			case 4: {POSSIBLE_PROTO_EXPANSION(node->rightTexture,thistex);  break;}
			case 5: {POSSIBLE_PROTO_EXPANSION(node->leftTexture,thistex);   break;}
		}
		if (thistex != 0) {
			/* we have an image specified for this face */
			/* the X3D spec says that a X3DTextureNode has to be one of... */
			if ((thistex->_nodeType == NODE_ImageTexture) ||
			    (thistex->_nodeType == NODE_PixelTexture) ||
			    (thistex->_nodeType == NODE_MovieTexture) ||
			    (thistex->_nodeType == NODE_MultiTexture)) {

				texture_count = 0;
				/* render the proper texture */
				render_node((void *)thistex);
		                glColor3d(1.0,1.0,1.0);

        			textureDraw_start(NULL,Backtex);
        			glVertexPointer (3,GL_FLOAT,0,BackgroundVert);
        			glNormalPointer (GL_FLOAT,0,Backnorms);

        			FW_GL_DRAWARRAYS (GL_QUADS, count*4, 4);
        			textureDraw_end();
			} 
		}
	}
}

/* load in a texture, if possible */
void loadTextureNode (struct X3D_Node *node, void *param) 
{
    struct X3D_MovieTexture *mym;
    
    if (NODE_NEEDS_COMPILING) {
	/* force a node reload - make it a new texture. Don't change
	   the parameters for the original number, because if this
	   texture is shared, then ALL references will change! so,
	   we just accept that the current texture parameters have to
	   be left behind. */
	MARK_NODE_COMPILED;
	
	/* this will cause bind_image to create a new "slot" for this texture */
	/* cast to GLuint because __texture defined in VRMLNodes.pm as SFInt */
	
	if (node->_nodeType != NODE_MovieTexture) {
	    releaseTexture(node); 
	} else {
	    
	    mym = (struct X3D_MovieTexture *)node;
	    /*  did the URL's change? we can't test for _change here, because
		movie running will change it, so we look at the urls. */
	    if ((mym->url.p) != (mym->__oldurl.p)) {
		releaseTexture(node); 
		mym->__oldurl.p = mym->url.p;
	    }
	}
	/* if this is a PixelTexture, copy the pixelImage parameter into
	   the table structure. (textureTableIndexStruct). This is because,
	   if we have an ImageTexture that is bad, we change *that* into a 
	   PixelTexture; so we need to have the pixeltexture raw data in
	   the textureTableIndexStruct entry */
	if (node->_nodeType == NODE_PixelTexture) {
	    copyPixelTextureEntry((struct X3D_PixelTexture*)node);
	}
    }
    new_bind_image (X3D_NODE(node), param);
}

void loadMultiTexture (struct X3D_MultiTexture *node) {
	int count;
	int max;
	struct multiTexParams *paramPtr;

	char *param;

	struct X3D_ImageTexture *nt;

#ifdef TEXVERBOSE
	 printf ("loadMultiTexture, this %d have %d textures %d %d\n",node->_nodeType,
			node->texture.n,
			node->texture.p[0], node->texture.p[1]);
	printf ("	change %d ichange %d\n",node->_change, node->_ichange);
#endif
	
	/* new node, or node paramaters changed */
        if (NODE_NEEDS_COMPILING) {
	    /*  have to regen the shape*/
	    MARK_NODE_COMPILED;

		/* have we initiated multitexture support yet? */
		if (maxTexelUnits < 0)  {
			init_multitexture_handling();
		}

		/* alloc fields, if required - only do this once, even if node changes */
		if (node->__params == 0) {
			/* printf ("loadMulti, MALLOCing for params\n"); */
			node->__params = MALLOC (sizeof (struct multiTexParams) * maxTexelUnits);
			paramPtr = (struct multiTexParams*) node->__params;

			/* set defaults for these fields */
			for (count = 0; count < maxTexelUnits; count++) {
				paramPtr->texture_env_mode  = GL_MODULATE; 
				paramPtr->combine_rgb = GL_MODULATE;
				paramPtr->source0_rgb = GL_TEXTURE;
				paramPtr->operand0_rgb = GL_SRC_COLOR;
				paramPtr->source1_rgb = GL_PREVIOUS;
				paramPtr->operand1_rgb = GL_SRC_COLOR;
				paramPtr->combine_alpha = GL_REPLACE;
				paramPtr->source0_alpha = GL_TEXTURE;
				paramPtr->operand0_alpha = GL_SRC_ALPHA;
				paramPtr->source1_alpha = 0;
				paramPtr->operand1_alpha = 0;
				/*
				paramPtr->source1_alpha = GL_PREVIOUS;
				paramPtr->operand1_alpha = GL_SRC_ALPHA;
				*/
				paramPtr->rgb_scale = 1;
				paramPtr->alpha_scale = 1;
				paramPtr++;
			}
		}

		/* how many textures can we use? no sense scanning those we cant use */
		max = node->mode.n; 
		if (max > maxTexelUnits) max = maxTexelUnits;

		/* go through the params, and change string name into a GLint */
		paramPtr = (struct multiTexParams*) node->__params;
		for (count = 0; count < max; count++) {
			param = node->mode.p[count]->strptr;
			/* printf ("param %d is %s len %d\n",count, param, xx); */

		        if (strcmp("MODULATE2X",param)==0) { 
				paramPtr->texture_env_mode  = GL_COMBINE; 
                                paramPtr->rgb_scale = 2;
                                paramPtr->alpha_scale = 2; } 

		        else if (strcmp("MODULATE4X",param)==0) {
				paramPtr->texture_env_mode  = GL_COMBINE; 
                                paramPtr->rgb_scale = 4;
                                paramPtr->alpha_scale = 4; } 
		        else if (strcmp("ADDSMOOTH",param)==0) {  
				paramPtr->texture_env_mode = GL_COMBINE;
				paramPtr->combine_rgb = GL_ADD;}
/* 
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305

*/
		        else if (strcmp("BLENDDIFFUSEALPHA",param)==0) {  
				paramPtr->texture_env_mode = GL_COMBINE;
				paramPtr->combine_rgb = GL_SUBTRACT;}
		        else if (strcmp("BLENDCURRENTALPHA",param)==0) {  
				paramPtr->texture_env_mode = GL_COMBINE;
				paramPtr->combine_rgb = GL_SUBTRACT;}
		        else if (strcmp("MODULATEALPHA_ADDCOLOR",param)==0) { 
				paramPtr->texture_env_mode = GL_COMBINE;
				paramPtr->combine_rgb = GL_SUBTRACT;}
		        else if (strcmp("MODULATEINVALPHA_ADDCOLOR",param)==0) { 
				paramPtr->texture_env_mode = GL_COMBINE;
				paramPtr->combine_rgb = GL_SUBTRACT;}
		        else if (strcmp("MODULATEINVCOLOR_ADDALPHA",param)==0) { 
				paramPtr->texture_env_mode = GL_COMBINE;
				paramPtr->combine_rgb = GL_SUBTRACT;}
		        else if (strcmp("SELECTARG1",param)==0) {  
				paramPtr->texture_env_mode = GL_REPLACE;
				paramPtr->combine_rgb = GL_TEXTURE0;}
		        else if (strcmp("SELECTARG2",param)==0) {  
				paramPtr->texture_env_mode = GL_REPLACE;
				paramPtr->combine_rgb = GL_TEXTURE1;}
		        else if (strcmp("DOTPRODUCT3",param)==0) {  
				paramPtr->texture_env_mode = GL_COMBINE;
				paramPtr->combine_rgb = GL_DOT3_RGB;}
/* */
		        else if (strcmp("MODULATE",param)==0) {
				/* defaults */}

		        else if (strcmp("REPLACE",param)==0) {
				paramPtr->texture_env_mode = GL_REPLACE;}

		        else if (strcmp("SUBTRACT",param)==0) {
				paramPtr->texture_env_mode = GL_COMBINE;
				paramPtr->combine_rgb = GL_SUBTRACT;}

		        else if (strcmp("ADDSIGNED2X",param)==0) {
				paramPtr->rgb_scale = 2;
				paramPtr->alpha_scale = 2;
				paramPtr->texture_env_mode = GL_COMBINE; 
				paramPtr->combine_rgb = GL_ADD_SIGNED;}

		        else if (strcmp("ADDSIGNED",param)==0) {
				paramPtr->texture_env_mode = GL_COMBINE; 
				paramPtr->combine_rgb = GL_ADD_SIGNED;}


		        else if (strcmp("ADD",param)==0) {
					paramPtr->texture_env_mode = GL_COMBINE;
					paramPtr->combine_rgb = GL_ADD; }


		        else if (strcmp("OFF",param)==0) { 
					paramPtr->texture_env_mode = 0; } 

			else {
				ConsoleMessage ("MultiTexture - invalid param or not supported yet- \"%s\"\n",param);
			}

			/* printf ("paramPtr for %d is %d\n",count,*paramPtr);  */
			paramPtr++;
		}

	/* coparamPtrile the sources */
/*
""
"DIFFUSE"
"SPECULAR"
"FACTOR"
*/
	/* coparamPtrile the functions */
/*""
"COMPLEMENT"
"ALPHAREPLICATE"
*/

	}

	/* ok, normally the scene graph contains function pointers. What we have
	   here is a set of pointers to datastructures of (hopefully!)
	   types like X3D_ImageTexture, X3D_PixelTexture, and X3D_MovieTexture.

	*/

	/* how many textures can we use? */
	max = node->texture.n; 
	if (max > maxTexelUnits) max = maxTexelUnits;

	/* go through and get all of the textures */
	paramPtr = (struct multiTexParams *) node->__params;

	for (count=0; count < max; count++) {
#ifdef TEXVERBOSE
		printf ("loadMultiTexture, working on texture %d\n",count);
#endif

		/* get the texture */
		nt = node->texture.p[count];

		switch (nt->_nodeType) {
			case NODE_PixelTexture:
			case NODE_ImageTexture : 
			case NODE_MovieTexture:
			case NODE_VRML1_Texture2:
				/* printf ("MultiTexture %d is a ImageTexture param %d\n",count,*paramPtr);  */
				loadTextureNode (X3D_NODE(nt), (void *)paramPtr);
				break;
			case NODE_MultiTexture:
				printf ("MultiTexture texture %d is a MULTITEXTURE!!\n",count);
				break;
			default:
				printf ("MultiTexture - unknown sub texture type %d\n",
						nt->_nodeType);
		}

		/* now, lets increment texture_count. The current texture will be
		   stored in bound_textures[texture_count]; texture_count will be 1
		   for "normal" textures; at least 1 for MultiTextures. */

        	texture_count++;
		paramPtr++;

#ifdef TEXVERBOSE
		printf ("loadMultiTexture, finished with texture %d\n",count);
#endif
	}
}

/************************************************************************/
void store_tex_info(
		struct textureTableIndexStruct *me,
		int depth,
		int x,
		int y,
		unsigned char *ptr,
		int hasAlpha) {

	me->frames=1;
	me->depth=depth;
	me->x = x;
	me->y = y;
	me->texdata = ptr;
	me->hasAlpha = hasAlpha;
}

/* do we do 1 texture, or is this a series of textures, requiring final binding
   by this thread? */

void do_possible_textureSequence(struct textureTableIndexStruct* me) {
	GLuint *texnums;
	int framecount;
	GLubyte *imageptr;
	int c;
	int rx,ry,sx,sy;
	int x,y;
	GLint iformat;
	GLenum format;
	int count;

	/* default texture properties; can be changed by a TextureProperties node */
	float anisotropicDegree=0.0;
	struct SFColorRGBA borderColor={0.0,0.0,0.0,0.0};
	int borderWidth=0;
	int boundaryModeS = GL_REPEAT;
	int boundaryModeT = GL_REPEAT;
	int boundaryModeR = GL_REPEAT;
	int magnificationFilter = GL_FASTEST;
	int minificationFilter = GL_FASTEST;
	int textureCompression = GL_FASTEST;
	int texturePriority = 0;
	int generateMipMaps = TRUE;
	
	/* for getting repeatS and repeatT info. */
	struct X3D_PixelTexture *pt = NULL;
	struct X3D_MovieTexture *mt = NULL;
	struct X3D_ImageTexture *it = NULL;
	struct X3D_VRML1_Texture2* v1t = NULL;
	struct X3D_TextureProperties *tpNode = NULL;
	GLint Src, Trc;
	unsigned char *mytexdata;


	/* see if we need to get the max texture size at runtime */
	CHECK_MAX_TEXTURE_SIZE

	/* do we need to convert this to an OpenGL texture stream?*/

	/* we need to get parameters. */	
	if (me->OpenGLTexture == NULL) {
		me->OpenGLTexture = MALLOC (sizeof (GLuint) * me->frames);
		glGenTextures(me->frames, me->OpenGLTexture);
#ifdef TEXVERBOSE
		printf ("just glGend %d textures  for block %x is %x\n",
			me->frames, me, me->OpenGLTexture);
#endif

		/* get the repeatS and repeatT info from the scenegraph node */
		if (me->nodeType == NODE_ImageTexture) {
			it = (struct X3D_ImageTexture *) me->scenegraphNode;
			Src = it->repeatS; Trc = it->repeatT;
			tpNode = it->textureProperties;
		} else if (me->nodeType == NODE_PixelTexture) {
			pt = (struct X3D_PixelTexture *) me->scenegraphNode;
			Src = pt->repeatS; Trc = pt->repeatT;
			tpNode = pt->textureProperties;
		} else if (me->nodeType == NODE_MovieTexture) {
			mt = (struct X3D_MovieTexture *) me->scenegraphNode;
			Src = mt->repeatS; Trc = mt->repeatT;
			tpNode = mt->textureProperties;
		} else if (me->nodeType == NODE_VRML1_Texture2) {
			v1t = (struct X3D_VRML1_Texture2 *) me->scenegraphNode;
			Src = v1t->_wrapS==VRML1MOD_REPEAT;
			Trc = v1t->_wrapT==VRML1MOD_REPEAT;
		}
		/* save texture params */
		me->Src = Src ? GL_REPEAT : GL_CLAMP;
		me->Trc = Trc ? GL_REPEAT : GL_CLAMP;

		if (tpNode) {
			printf ("Texture, have textureProperties\n");
/*
  SFFloat     [in,out] anisotropicDegree   1.0       [1,∞)
  SFColorRGBA [in,out] borderColor         0 0 0 0   [0,1]
  SFInt32     [in,out] borderWidth         0         [0,1]
  SFString    [in,out] boundaryModeS       "REPEAT"  [see Table 18.7]
  SFString    [in,out] boundaryModeT       "REPEAT"  [see Table 18.7]
  SFString    [in,out] boundaryModeR       "REPEAT"  [see Table 18.7]
  SFString    [in,out] magnificationFilter "FASTEST" [see Table 18.8]
  SFString    [in,out] minificationFilter  "FASTEST" [see Table 18.9]
  SFString    [in,out] textureCompression  "FASTEST" [see Table 18.10]
  SFFloat     [in,out] texturePriority     0         [0,1]
  SFBool      []       generateMipMaps     FALSE
if (generateMipMaps) printf ("generateMipMaps\n"); else printf ("NOT generateMipMaps\n");
*/
			if (tpNode->_nodeType == NODE_TextureProperties) {
				generateMipMaps = tpNode->generateMipMaps;
			} else {
				ConsoleMessage ("a texture node has a textureProperties of type %s - ignoring",
					stringNodeType(tpNode->_nodeType));
			}
		}
	}

	/* a pointer to the tex data. We increment the pointer for movie texures */
	mytexdata = me->texdata;

	for (count = 0; count < me->frames; count ++) {
		glBindTexture (GL_TEXTURE_2D, me->OpenGLTexture[count]);
	
		/* save this to determine whether we need to do material node
		  within appearance or not */
		
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, me->Src);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, me->Trc);
		glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP, GL_TRUE);
		x = me->x;
		y = me->y;

		/* choose smaller images to be NEAREST, larger ones to be LINEAR */
		if ((x<=256) || (y<=256)) {
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} else {
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

	
		if (me->hasAlpha) {
			iformat = GL_RGBA; format = GL_RGBA;
		} else {
			switch (me->depth) {
				case 1: iformat = GL_LUMINANCE; format = GL_LUMINANCE; break;
				case 2: iformat = GL_LUMINANCE_ALPHA; format = GL_LUMINANCE_ALPHA; break;
				case 3: iformat = GL_RGB; format = GL_RGB; break;
				default: iformat = GL_RGBA; format = GL_RGBA; break;
			}
		}
	
		/* do the image. */
		/* note -for some computers with poor graphics chips, large textures will just not
		   work, so we do PROXY textures. The old "just do it" code is here, in case the new
		   code will not compile on some machines */
#define DO_PROXY_IMAGE
#ifdef DO_PROXY_IMAGE
		if((me->depth) && x && y) {
			int texOk = FALSE;

			unsigned char *dest = mytexdata;
			rx = 1; sx = x;
			while(sx) {sx /= 2; rx *= 2;}
			if(rx/2 == x) {rx /= 2;}
			ry = 1; sy = y;
			while(sy) {sy /= 2; ry *= 2;}
			if(ry/2 == y) {ry /= 2;}

			if (displayOpenGLErrors) printf ("initial texture scale to %d %d\n",rx,ry);

			if(rx != x || ry != y || rx > global_texSize || ry > global_texSize) {
				/* do we have texture limits??? */
				if (rx > global_texSize) rx = global_texSize;
				if (ry > global_texSize) ry = global_texSize;
			}

			if (displayOpenGLErrors)
				printf ("texture size after maxTextureSize taken into account: %d %d, from %d %d\n",rx,ry,x,y);
	
			/* try this texture on for size, keep scaling down until we can do it */
			texOk = FALSE;
			dest = (unsigned char *)MALLOC((unsigned) (me->depth) * rx * ry);
			while (!texOk) {
				GLint width, height;
				gluScaleImage(format, x, y, GL_UNSIGNED_BYTE, mytexdata, rx, ry, GL_UNSIGNED_BYTE, dest);
				glTexImage2D(GL_PROXY_TEXTURE_2D, 0, iformat,  rx, ry, 0, format, GL_UNSIGNED_BYTE, dest);

				glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0,GL_TEXTURE_WIDTH, &width); 
				glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0,GL_TEXTURE_HEIGHT, &height); 

				if ((width == 0) || (height == 0)) {
					rx= rx/2; ry = ry/2;
					if (displayOpenGLErrors)
					    printf ("width %d height %d going to try size %d %d, last time %d %d\n",
						width, height, rx,ry,x,y);
					if ((rx==0) || (ry==0)) {
					    ConsoleMessage ("out of texture memory");
					    me->status = TEX_LOADED; /* yeah, right */
					    return;
					}
				} else {
					texOk = TRUE;
				}
			}


			if (displayOpenGLErrors)
				printf ("after proxy image stuff, size %d %d\n",rx,ry);

			glTexImage2D(GL_TEXTURE_2D, 0, iformat,  rx, ry, 0, format, GL_UNSIGNED_BYTE, dest);
			glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP, GL_TRUE);

			if(mytexdata != dest) FREE_IF_NZ(dest);
		}

#else /* DO_PROXY_IMAGE */

                if((me->depth) && x && y) {
                        unsigned char *dest = mytexdata;
                        rx = 1; sx = x;
                        while(sx) {sx /= 2; rx *= 2;}
                        if(rx/2 == x) {rx /= 2;}
                        ry = 1; sy = y;
                        while(sy) {sy /= 2; ry *= 2;}
                        if(ry/2 == y) {ry /= 2;}
                        if(rx != x || ry != y || rx > global_texSize || ry > global_texSize) {
                                /* do we have texture limits??? */
                                if (rx > global_texSize) rx = global_texSize;
                                if (ry > global_texSize) ry = global_texSize;

                                /* We have to scale */
                                dest = (unsigned char *)MALLOC((unsigned) (me->depth) * rx * ry);
                                gluScaleImage(format,
                                     x, y, GL_UNSIGNED_BYTE, mytexdata, rx, ry,
                                     GL_UNSIGNED_BYTE, dest);
        
                        }
        
                        if((mytexdata) != dest) FREE_IF_NZ(dest);

                }

#endif /* DO_PROXY_IMAGE */

		/* increment, used in movietextures for frames more than 1 */
		mytexdata += x*y*me->depth;
	}

	FREE_IF_NZ (me->texdata);

	/* ensure this data is written to the driver for the rendering context */
	glFlush();

	/* CGLError err = CGLFlushDrawable(aqtextureContext); */

	/* and, now, the Texture is loaded */
	me->status = TEX_LOADED;
}

/**********************************************************************************

 bind the image,

	itype 	tells us whether it is a PixelTexture, ImageTexture or MovieTexture.

	parenturl  is a pointer to the url of the parent (for relative loads) OR
		a pointer to the image data (PixelTextures only)

	url	the list of urls from the VRML file, or NULL (for PixelTextures)

	texture_num	the OpenGL texture identifier

	repeatS, repeatT VRML fields

	param - vrml fields, but translated into GL_TEXTURE_ENV_MODE, GL_MODULATE, etc.
************************************************************************************/

#ifdef TEXVERBOSE
char *texst (int num) {
	if (num == TEX_NOTLOADED) return "TEX_NOTLOADED";
	if (num == TEX_LOADING) return "TEX_LOADING";
	if (num == TEX_NEEDSBINDING)return "TEX_NEEDSBINDING";
	if (num == TEX_LOADED)return "TEX_LOADED";
	if (num == TEX_UNSQUASHED)return "UNSQUASHED";
	return "unknown";
}
#endif

void new_bind_image(struct X3D_Node *node, void *param) {
	int thisTexture;
	int thisTextureType;
	struct X3D_ImageTexture *it;
	struct X3D_PixelTexture *pt;
	struct X3D_MovieTexture *mt;
	struct X3D_VRML1_Texture2 *v1t;
	struct textureTableIndexStruct *myTableIndex;
	float dcol[] = {0.8, 0.8, 0.8, 1.0};

	GET_THIS_TEXTURE;

	bound_textures[texture_count] = 0;

	/* what is the status of this texture? */
#ifdef TEXVERBOSE
	printf ("new_bind_image, calling getTableIndex\n");
#endif

	myTableIndex = getTableIndex(thisTexture);

#ifdef TEXVERBOSE
	printf ("myTableIndex %x\n",myTableIndex);
	printf ("	scenegraphNode %d (%s)\n",myTableIndex->scenegraphNode,
				stringNodeType(X3D_NODE(myTableIndex->scenegraphNode)->_nodeType));
	printf ("	status %d\n",myTableIndex->status);
	printf ("	status %d\n",myTableIndex->status);
	printf ("	frames %d\n",myTableIndex->frames);
	printf ("	OpenGLTexture %d\n",myTableIndex->OpenGLTexture);

	printf ("texture status %s\n",texst(myTableIndex->status));
#endif

	/* have we already processed this one before? */
	if (myTableIndex->status == TEX_LOADED) {

#ifdef TEXVERBOSE 
		printf ("now binding to pre-bound mti%d tex%d\n",
			myTableIndex,myTableIndex->OpenGLTexture[0]);
#endif
		/* set the texture depth - required for Material diffuseColor selection */
		if (myTableIndex->hasAlpha) last_texture_type =  TEXTURE_ALPHA;
		else last_texture_type = TEXTURE_NO_ALPHA;

		/* if, we have RGB, or RGBA, X3D Spec 17.2.2.3 says ODrgb = IDrgb, ie, the diffuseColor is
		   ignored. We do this here, because when we do the Material node, we do not know what the
		   texture depth is (if there is any texture, even) */
		if (myTableIndex->hasAlpha) {
			do_glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dcol);
		}

		if (myTableIndex->nodeType != NODE_MovieTexture) {
			if (myTableIndex->OpenGLTexture == NULL) {
#ifdef TEXVERBOSE
				printf ("no openGLtexture here status %s\n",
					texst(myTableIndex->status));
#endif
				return;
			}

			bound_textures[texture_count] = myTableIndex->OpenGLTexture[0];
		} else {
			bound_textures[texture_count] = 
				((struct X3D_MovieTexture *)myTableIndex->scenegraphNode)->__ctex;
		}

		/* save the texture params for when we go through the MultiTexture stack. Non
		   MultiTextures should have this texture_count as 0 */
		 
		texParams[texture_count] = param; 

		textureInProcess = -1; /* we have finished the whole process */
		return;
	} 


	if (textureInProcess > 0) {
		sched_yield();
		/* we are already working on a texture. Is it THIS one? */
		if (textureInProcess != thisTexture) {
#ifdef TEXVERBOSE 
			printf ("bind_image, textureInProcess = %d, texture_num %d returning \n",
				textureInProcess,thisTexture);
#endif

			return;
		}
#ifdef TEXVERBOSE 
		printf ("bind_image, textureInProcess == texture_num\n");
#endif
	}

	/* signal that his is the one we want to work on */
	textureInProcess = thisTexture;


	/* is this one an unsquished movie texture? */
	if (myTableIndex->status == TEX_UNSQUASHED) { return; }


#ifdef AQUA
        if (RUNNINGASPLUGIN) {
	    DO_POSSIBLE_TEXTURE_SEQUENCE;
        }
#endif

#ifndef DO_MULTI_OPENGL_THREADS
        /* is this one read in, but requiring final manipulation
         * by THIS thread? */
	DO_POSSIBLE_TEXTURE_SEQUENCE;
#endif

	/* are we loading this one? */
	if (myTableIndex->status == TEX_LOADING) {
		return;
	}

	/* so, we really need to do this one... */

	/* is the thread currently doing something? */
	if (TextureParsing) return;

        TLOCK;
	if (currentlyWorkingOn <0) {
#ifdef TEXVERBOSE
	    printf ("currentlyWorkingOn WAS %d ",currentlyWorkingOn);
#endif
	    currentlyWorkingOn = thisTexture;
	    loadThisTexture = myTableIndex;
#ifdef TEXVERBOSE
	    printf ("just set currentlyWorkingOn to %d\n",currentlyWorkingOn);
#endif
	}
	T_LOCK_SIGNAL;
	TUNLOCK;
}

/****************************************************************/
/*								*/
/*	Texture loading thread and associated functions		*/
/*								*/
/*	only do 1 texture at a time				*/
/*								*/
/*								*/
/*								*/
/****************************************************************/

/* find the file, either locally or within the Browser. Note that
   this is almost identical to the one for Inlines, but running
   in different threads */

static int findTextureFile (int cwo) {
	char *filename;
	char *mypath;
	int count;
	char firstBytes[4];
#ifndef AQUA
	char *sysline;
#endif

        struct Uni_String *thisParent = NULL;
        struct Multi_String thisUrl;

	/* pattern matching, for finding internally handled types */
	char firstPNG[] = {0x89,0x50,0x4e,0x47};
	char firstJPG[] = {0xff,0xd8,0xff,0xe0};
#ifndef AQUA
	char firstMPGa[] = {0x00, 0x00, 0x01, 0xba};
	char firstMPGb[] = {0x00, 0x00, 0x01, 0xb3};
#endif

	filename = NULL;


#ifdef TEXVERBOSE 
	printf ("textureThread:start of findTextureFile for cwo %d type %d \n",
		cwo,loadThisTexture->nodeType);
#endif
	/* try to find this file. */

	if (loadThisTexture->nodeType !=NODE_PixelTexture) {

		/* lets make up the path and save it, and make it the global path */

		if (loadThisTexture->nodeType == NODE_ImageTexture) {
			thisParent = ((struct X3D_ImageTexture *)loadThisTexture->scenegraphNode)->__parenturl;
			thisUrl = ((struct X3D_ImageTexture *)loadThisTexture->scenegraphNode)->url;
		} else if (loadThisTexture->nodeType == NODE_MovieTexture) {
			thisParent = ((struct X3D_MovieTexture *)loadThisTexture->scenegraphNode)->__parenturl;
			thisUrl = ((struct X3D_MovieTexture *)loadThisTexture->scenegraphNode)->url;
		} else if (loadThisTexture->nodeType == NODE_VRML1_Texture2) {
			thisParent = ((struct X3D_VRML1_Texture2 *)loadThisTexture->scenegraphNode)->__parenturl;
			thisUrl = ((struct X3D_VRML1_Texture2 *)loadThisTexture->scenegraphNode)->filename;
		}
		mypath = STRDUP(thisParent->strptr);

		/* Dangerous, better alloc this string in function getValidFileFromUrl ... */
		filename = (char *)MALLOC(4096);

		if (getValidFileFromUrl (filename,mypath, &thisUrl, firstBytes)) {
#ifdef TEXVERBOSE 
		    printf ("textureThread: we were successful at locating %s\n",filename); 
#endif
		} else {
			if (count > 0) {
#ifndef ARCH_PPC
			    ConsoleMessage ("Could not locate URL for texture %d (last choice was %s)\n",cwo,filename);
#endif
			}
			/* So, we could not find the correct file. Make this into a blank PixelTexture, so that
			   at least this looks ok on the screen */
#ifdef TEXVERBOSE
			printf("textureThread: could not locate file from url: %s\n", thisUrl);
#endif
			FREE_IF_NZ(filename);
			loadThisTexture->nodeType = NODE_PixelTexture;
			invalidFilePixelDataNode.n = 4;
			invalidFilePixelDataNode.p = invalidFilePixelData;
			loadThisTexture->pixelData = &invalidFilePixelDataNode;
		}
	}

	/* pixelTextures - lets just make a specific string for this one */
	if (loadThisTexture->nodeType == NODE_PixelTexture) {
		FREE_IF_NZ(filename);
		filename = (char *)MALLOC(4096);
		sprintf (filename,"PixelTexture_%d",loadThisTexture);
	}

#ifdef AQUA
	/* on AQUA/OSX, let QuickTime do the conversion for us, but maybe we can help it out
	   by keeping a tab on what kind of image this is  */
	if ((loadThisTexture->nodeType == NODE_ImageTexture) || (loadThisTexture->nodeType == NODE_VRML1_Texture2)){
		loadThisTexture->imageType = INT_ID_UNDEFINED;
		if (strncmp(firstBytes,firstPNG,4) == 0) loadThisTexture->imageType = PNGTexture;
		if (strncmp(firstBytes,firstJPG,4) == 0) loadThisTexture->imageType = JPGTexture;
	}

#else /* AQUA */
	if ((loadThisTexture->nodeType == NODE_ImageTexture) || (loadThisTexture->nodeType == NODE_VRML1_Texture2)){
		loadThisTexture->imageType = INT_ID_UNDEFINED;
		if (strncmp(firstBytes,firstPNG,4) == 0) loadThisTexture->imageType = PNGTexture;
		if (strncmp(firstBytes,firstJPG,4) == 0) loadThisTexture->imageType = JPGTexture;

		/* is this a texture type that is *not* handled internally? */
		if ((strncmp(firstBytes,firstPNG,4) != 0) &&
		    (strncmp(firstBytes,firstJPG,4) != 0) &&
		    (strncmp(firstBytes,firstMPGa,4) != 0) &&
		    (strncmp(firstBytes,firstMPGb,4) != 0)) {

#ifdef TEXVERBOSE 
		    printf ("textureThread: trying to convert on %s\n", filename);
#endif
		    if (!filename) {
			printf("textureThread: error: trying to load null file\n");
			return FALSE;
		    }
		    sysline = (char *)MALLOC(sizeof(char)*(strlen(filename)+100));
		    sprintf(sysline,"%s %s /tmp/freewrl%d.png",
			    IMAGECONVERT, filename, getpid());
#ifdef TEXVERBOSE 
		    printf ("textureThread: running convert on %s\n",sysline);
#endif

		    if (freewrlSystem (sysline) != TRUE) {
			printf ("Freewrl: error running convert line %s\n",sysline);
		    } else {
			FREE_IF_NZ(filename);
			filename = (char *)MALLOC(4096);
			sprintf (filename,"/tmp/freewrl%d.png",getpid());
		    }
		    FREE_IF_NZ (sysline);
		}
	}
#endif /* AQUA */

	/* save filename in data structure for later comparisons */
#ifdef TEXVERBOSE
	printf ("textureThread: new name, save it %d, name %s\n",cwo,filename);
#endif

	FREE_IF_NZ(loadThisTexture->filename);
	if (filename != NULL) {
	    loadThisTexture->filename = STRDUP(filename);
	    /* printf ("textureThread, so we have CACHE filename as %s\n",
	       loadThisTexture->filename); */
	} else {
	    ConsoleMessage ("error getting Texturefile\n");
	    return FALSE;
	}
	
	FREE_IF_NZ (filename);
	return TRUE;
}

/*************************************************************/
/* _textureThread - work on textures, until the end of time. */

void _textureThread(void)
{
    /* printf ("textureThread is %u\n",pthread_self()); */

#ifdef AQUA

    /* To get this thread to be able to manipulate textures, first, get the 
       Display attributes */
    if (!RUNNINGASPLUGIN) {
	CGDirectDisplayID display = CGMainDisplayID ();
	attribs[1] = CGDisplayIDToOpenGLDisplayMask (display);
	
	/* now, for this thread, create and join OpenGL Contexts */
	CGLChoosePixelFormat (attribs, &pixelFormat, &numPixelFormats);
	CGLCreateContext(pixelFormat, myglobalContext, &aqtextureContext);
	
	/* set the context for this thread so that we can share textures with
	   the main context (myglobalContext) */
	
	CGLSetCurrentContext(aqtextureContext);
	/* printf ("textureThread, have to try to remember to destroy this context\n"); */
    }
    
#else /* AQUA */

# ifdef DO_MULTI_OPENGL_THREADS
    textureContext = glXCreateContext(Xdpy, Xvi, GLcx, GL_FALSE);
    glXMakeCurrent(Xdpy,Xwin,textureContext);
# endif /* DO_MULTI_OPENGL_THREADS */

#endif /* AQUA */

    /* set up some common storage info */
#ifdef DO_MULTI_OPENGL_THREADS
    if (!RUNNINGASPLUGIN) {
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    }
#endif

    /* we wait forever for the data signal to be sent */
    for (;;) {
	TLOCK;
	TextureThreadInitialized = TRUE;
	T_LOCK_WAIT;
	REGENLOCK;

#ifdef TEXVERBOSE
	printf ("textureThread - working on %d ",currentlyWorkingOn);
	printf ("which is node %d, nodeType %d status %s, opengltex %d, and frames %d\n",
		loadThisTexture->scenegraphNode, loadThisTexture->nodeType, 
		texst(loadThisTexture->status), loadThisTexture->OpenGLTexture, 
		loadThisTexture->frames);
#endif

	loadThisTexture->status = TEX_LOADING;
	TextureParsing = TRUE;
	
	/* look for the file. If one does not exist, or it
	   is a duplicate, just unlock and return */
#ifdef TEXVERBOSE
	printf ("textureThread, currentlyworking on %d\n",currentlyWorkingOn);
#endif

	if (findTextureFile(currentlyWorkingOn)) {
#ifdef TEXVERBOSE
	    printf ("textureThread, findTextureFile ok for %d\n",currentlyWorkingOn);
#endif
	    /* is this a pixeltexture? */
	    if (loadThisTexture->nodeType==NODE_ImageTexture) {
		__reallyloadImageTexture();
	    } else if (loadThisTexture->nodeType==NODE_MovieTexture) {
		__reallyloadMovieTexture();
	    } else if (loadThisTexture->nodeType==NODE_VRML1_Texture2) {
		__reallyloadImageTexture();
	    } else {
		__reallyloadPixelTexure();
	    }
	    
#ifdef TEXVERBOSE
	    printf ("textureThread, after reallyLoad for  %d\n",currentlyWorkingOn);
#endif

#ifdef DO_MULTI_OPENGL_THREADS
	    if (!RUNNINGASPLUGIN) {
# ifdef TEXVERBOSE 
		printf ("tex %d needs binding, name %s\n",loadThisTexture->OpenGLTexture,
			loadThisTexture->filename);
# endif /* TEXVERBOSE */

		do_possible_textureSequence(loadThisTexture);
		
#ifdef TEXVERBOSE 
		printf ("tex %d now loaded\n",loadThisTexture->OpenGLTexture);
#endif
	    } else {
		loadThisTexture->status = TEX_NEEDSBINDING;
	    }
	    
#else /* DO_MULTI_OPENGL_THREADS */

	    /* we can not do this in 2 threads, let the main OpenGL thread do this */
	    /* printf ("we can not do multi-threads, %d set to TEX_NEEDSBINDING\n",
	       loadThisTexture->scenegraphNode); */

	    loadThisTexture->status = TEX_NEEDSBINDING;
#endif /* DO_MULTI_OPENGL_THREADS */
	} else {
	    printf ("can not find file - error!!\n");
	}

	/* signal that we are finished */
#ifdef TEXVERBOSE
	printf ("textureThread: finished parsing texture for currentlyWorkingOn %d\n",
		currentlyWorkingOn);
#endif
	TextureParsing=FALSE;
	currentlyWorkingOn = -1;
	REGENUNLOCK;
	TUNLOCK;
    }
}

/********************************************************************************/
/* load specific types of textures						*/
/********************************************************************************/

/* load a PixelTexture that is stored as a MFInt32 */
static void __reallyloadPixelTexure() {
	/* PixelTexture variables */
	long hei,wid,depth;
	unsigned char *texture;
	int count;
	int ok;
	struct Multi_Int32 * myData;
	int *iptr;
	int tctr;

	myData = loadThisTexture->pixelData;
	iptr = (int *) myData->p;

	ok = TRUE;

	/* are there enough numbers for the texture? */
	if (myData->n < 3) {
		printf ("PixelTexture, need at least 3 elements, have %d\n",myData->n);
		ok = FALSE;
	} else {
		wid = *iptr; iptr++;
		hei = *iptr; iptr++;
		depth = *iptr; iptr++;

		if ((depth < 0) || (depth >4)) {
			printf ("PixelTexture, depth %d out of range, assuming 1\n",(int) depth);
			depth = 1;
		}
	
		if ((wid*hei-3) > myData->n) {
			printf ("PixelTexture, not enough data for wid %d hei %d, have %d\n",
					wid, hei, (wid*hei)-2);
			ok = FALSE;
		}
	}
		

	if (ok) {
		texture = (unsigned char *)MALLOC (wid*hei*4);
		tctr = 0;
		for (count = 0; count < (wid*hei); count++) {
			switch (depth) {
				case 1: {
					   texture[tctr++] = *iptr & 0xff;
					   break;
				   }
				case 2: {
					   texture[tctr++] = *iptr & 0x00ff;
					   texture[tctr++] = (*iptr>>8) & 0xff;
					   break;
				   }
				case 3: {
					   texture[tctr++] = (*iptr>>16) & 0xff; /*R*/
					   texture[tctr++] = (*iptr>>8) & 0xff;	 /*G*/
					   texture[tctr++] = (*iptr>>0) & 0xff; /*B*/
					   break;
				   }
				case 4: {
					   texture[tctr++] = (*iptr>>24) & 0xff; /*R*/
					   texture[tctr++] = (*iptr>>16) & 0xff; /*G*/
					   texture[tctr++] = (*iptr>>8) & 0xff;	 /*B*/
					   texture[tctr++] = (*iptr>>0) & 0xff; /*A*/
					   /* printf ("verify, %x %x %x %x\n",texture[tctr-4],texture[tctr-3],
						texture[tctr-2],texture[tctr-1]); */
					   break;
				   }
			}

			iptr++;
		}
		store_tex_info(loadThisTexture, (int)depth,(int)wid,(int)hei,texture,
			((depth==2)||(depth==4)) );
	}

}


#ifdef AQUA


/* render from aCGImageRef into a buffer, to get EXACT bits, as a CGImageRef contains only
estimates. */
/* from http://developer.apple.com/qa/qa2007/qa1509.html */

static inline double radians (double degrees) {return degrees * M_PI/180;}
CGContextRef CreateARGBBitmapContext (CGImageRef inImage) {
	CGContextRef    context = NULL;
	CGColorSpaceRef colorSpace;
	void *          bitmapData;
	int             bitmapByteCount;
	int             bitmapBytesPerRow;
	CGBitmapInfo	bitmapInfo;
	size_t		bitsPerComponent;

	 // Get image width, height. Well use the entire image.
	size_t pixelsWide = CGImageGetWidth(inImage);
	size_t pixelsHigh = CGImageGetHeight(inImage);

	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	bitmapBytesPerRow   = (pixelsWide * 4);
	bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);

	// Use the generic RGB color space.
	colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	if (colorSpace == NULL)
	{
	    fprintf(stderr, "Error allocating color space\n");
	    return NULL;
	}

	
	/* figure out the bitmap mapping */
	bitsPerComponent = CGImageGetBitsPerComponent(inImage);

	if (bitsPerComponent >= 8) {
		CGRect rect = {{0,0},{pixelsWide, pixelsHigh}};
		bitmapInfo = kCGImageAlphaNoneSkipLast;

		/* Allocate memory for image data. This is the destination in memory
		   where any drawing to the bitmap context will be rendered. */
		bitmapData = malloc( bitmapByteCount );
		if (bitmapData == NULL) {
		    fprintf (stderr, "Memory not allocated!");
		    CGColorSpaceRelease( colorSpace );
		    return NULL;
		}
	
		/* Create the bitmap context. We want pre-multiplied ARGB, 8-bits
		  per component. Regardless of what the source image format is
		  (CMYK, Grayscale, and so on) it will be converted over to the format
		  specified here by CGBitmapContextCreate. */
		context = CGBitmapContextCreate (bitmapData, pixelsWide, pixelsHigh,
			bitsPerComponent, bitmapBytesPerRow, colorSpace, bitmapInfo); 
	
		if (context == NULL) {
		    free (bitmapData);
		    fprintf (stderr, "Context not created!");
		} else {
	
			/* try scaling and rotating this image to fit our ideas on life in general */
			CGContextTranslateCTM (context, 0, pixelsHigh);
			CGContextScaleCTM (context,1.0, -1.0);
		}
		CGContextDrawImage(context, rect,inImage);
	} else {
		/* CGRect rect = {{0,0},{pixelsWide, pixelsHigh}}; */
		/* this is a mask. */

		printf ("bits per component of %d not handled\n",bitsPerComponent);
		return NULL;
	}

	/* Make sure and release colorspace before returning */
	CGColorSpaceRelease( colorSpace );

	return context;
}



static void __reallyloadImageTexture() {
	CGImageRef 	image;
	CFStringRef	path;
	CFURLRef 	url;
	size_t 		image_width;
	size_t 		image_height;


	CGContextRef 	cgctx;
#ifdef TRY_QUICKTIME
	OSErr 		err;
	GraphicsImportComponent gi;
	Handle 		dataRef;
	OSType 		dataRefType;
#endif

	unsigned char *	data;
	int		hasAlpha = FALSE;

	CGDataProviderRef provider;
	CGImageSourceRef 	sourceRef;


	/* printf ("loading %s imageType %d\n",getShadowFileNamePtr(loadThisTexture->filename), loadThisTexture->imageType);  */

	path = CFStringCreateWithCString(NULL, getShadowFileNamePtr(loadThisTexture->filename), kCFStringEncodingUTF8);
	url = CFURLCreateWithFileSystemPath (NULL, path, kCFURLPOSIXPathStyle, NULL);

	/* ok, we can define USE_CG_DATA_PROVIDERS or TRY_QUICKTIME...*/
#define USE_CG_DATA_PROVIDERS
#ifdef USE_CG_DATA_PROVIDERS
	/* can we directly import this a a jpeg or png?? */
	if (loadThisTexture->imageType != INT_ID_UNDEFINED) {
		/* printf ("this is a JPEG texture, try direct loading\n"); */
		provider = CGDataProviderCreateWithURL(url);
		if (loadThisTexture->imageType == JPGTexture)  {
			image = CGImageCreateWithJPEGDataProvider(provider, NULL, FALSE, kCGRenderingIntentDefault);
		} else {
			image = CGImageCreateWithPNGDataProvider(provider, NULL, FALSE, kCGRenderingIntentDefault);
		}
		CGDataProviderRelease(provider);

	} else {
#endif

#ifdef TRY_QUICKTIME
   /* I dont know whether to use quicktime or not... Probably not... as the other ways using core 
graphics seems to be ok. Anyway, I left this code in here, as maybe it might be of use for mpegs
*/

		/* lets let quicktime decide on what to do with this image */
		err = QTNewDataReferenceFromCFURL(url,0, &dataRef, &dataRefType);

		if (dataRef != NULL) {
			err = GetGraphicsImporterForDataRef (dataRef, dataRefType, &gi);
			err = GraphicsImportCreateCGImage (gi, &image, 0);
			DisposeHandle (dataRef);
			CloseComponent(gi);
		}
#else
		sourceRef = CGImageSourceCreateWithURL(url,NULL);

		if (sourceRef != NULL) {
			image = CGImageSourceCreateImageAtIndex(sourceRef, 0, NULL);
			CFRelease (sourceRef);
		}
#endif
#ifdef USE_CG_DATA_PROVIDERS
	}
#endif

	CFRelease(url);
	CFRelease(path);

	image_width = CGImageGetWidth(image);
	image_height = CGImageGetHeight(image);
	hasAlpha = CGImageGetAlphaInfo(image) != kCGImageAlphaNone;

	#ifdef TEXVERBOSE
	if (hasAlpha) printf ("Image has Alpha channel\n"); else printf ("image - no alpha channel \n");

	printf ("raw image, AlphaInfo %x\n",CGImageGetAlphaInfo(image));
	printf ("raw image, BitmapInfo %x\n",CGImageGetBitmapInfo(image));
	printf ("raw image, BitsPerComponent %d\n",CGImageGetBitsPerComponent(image));
	printf ("raw image, BitsPerPixel %d\n",CGImageGetBitsPerPixel(image));
	printf ("raw image, BytesPerRow %d\n",CGImageGetBytesPerRow(image));
	printf ("raw image, ImageHeight %d\n",CGImageGetHeight(image));
	printf ("raw image, ImageWidth %d\n",CGImageGetWidth(image));
	#endif
	
	/* now, lets "draw" this so that we get the exact bit values */
	cgctx = CreateARGBBitmapContext(image);

	 
	#ifdef TEXVERBOSE
	printf ("GetAlphaInfo %x\n",CGBitmapContextGetAlphaInfo(cgctx));
	printf ("GetBitmapInfo %x\n",CGBitmapContextGetBitmapInfo(cgctx));
	printf ("GetBitsPerComponent %d\n",CGBitmapContextGetBitsPerComponent(cgctx));
	printf ("GetBitsPerPixel %d\n",CGBitmapContextGetBitsPerPixel(cgctx));
	printf ("GetBytesPerRow %d\n",CGBitmapContextGetBytesPerRow(cgctx));
	printf ("GetHeight %d\n",CGBitmapContextGetHeight(cgctx));
	printf ("GetWidth %d\n",CGBitmapContextGetWidth(cgctx));
	#endif
	
#undef TEXVERBOSE

	data = (unsigned char *)CGBitmapContextGetData(cgctx);

	#ifdef TEXVERBOSE
	if (CGBitmapContextGetWidth(cgctx) < 65) {
		int i;

		printf ("dumping image\n");
		for (i=0; i<CGBitmapContextGetBytesPerRow(cgctx)*CGBitmapContextGetHeight(cgctx); i++) {
			printf ("%2x ",data[i]);
		}
		printf ("\n");
	}
	#endif

	/* is there possibly an error here, like a file that is not a texture? */
	if (CGImageGetBitsPerPixel(image) == 0) {
		ConsoleMessage ("texture file invalid: %s",loadThisTexture->filename);
	}

	if (data != NULL) {
		store_tex_info (loadThisTexture, 4, image_width, image_height,  data, hasAlpha);
	}

	CGContextRelease(cgctx);
}

#else

# if HAVE_JPEGLIB_H

/*********************************************************************************************/

/*
 * JPEG ERROR HANDLING: code from
 * http://courses.cs.deu.edu.tr/cse566/newpage2.htm
 *
 * The JPEG library's standard error handler (jerror.c) is divided into
 * several "methods" which you can override individually.  This lets you
 * adjust the behavior without duplicating a lot of code, which you might
 * have to update with each future release.
 *
 * Our example here shows how to override the "error_exit" method so that
 * control is returned to the library's caller when a fatal error occurs,
 * rather than calling exit() as the standard error_exit method does.
 *
 * We use C's setjmp/longjmp facility to return control.  This means that the
 * routine which calls the JPEG library must first execute a setjmp() call to
 * establish the return point.  We want the replacement error_exit to do a
 * longjmp().  But we need to make the setjmp buffer accessible to the
 * error_exit routine.  To do this, we make a private extension of the
 * standard JPEG error handler object.  (If we were using C++, we'd say we
 * were making a subclass of the regular error handler.)
 *
 * Here's the extended error handler struct:
 */

struct my_error_mgr {
	struct jpeg_error_mgr pub;    /* "public" fields */
	jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  	my_error_ptr myerr = (my_error_ptr) cinfo->err;

 	/* Always display the message. */
  	/* We could postpone this until after returning, if we chose. */
  	/* JAS (*cinfo->err->output_message) (cinfo); */

 	/* Return control to the setjmp point */
  	longjmp(myerr->setjmp_buffer, 1);
}

/*********************************************************************************************/

static void __reallyloadImageTexture() {
	FILE *infile;
	char *filename;
	GLuint texture_num;
	unsigned char *image_data = 0;

	/* png reading variables */
	int rc;
	unsigned long image_width = 0;
	unsigned long image_height = 0;
	unsigned long image_rowbytes = 0;
	int image_channels = 0;
	double display_exponent = 0.0;

	/* jpeg variables */
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	JDIMENSION nrows;
	JSAMPROW row = 0;
	JSAMPROW rowptr[1];
	unsigned rowcount, columncount;
	int dp;

	int tempInt;


	filename = loadThisTexture->filename;
	infile = openLocalFile(filename,"r");


	/* printf ("reallyLoad on linux, texture type %d\n",loadThisTexture->imageType); */

	/* ok - this is a JPGTexture if specified, or a PNG texture either directly,
	   or converted. */

	if (loadThisTexture->imageType == JPGTexture) {



		/* it is not a png file - assume a jpeg file */
		/* start from the beginning again */
		rewind (infile);

		/* Select recommended processing options for quick-and-dirty output. */
		cinfo.two_pass_quantize = FALSE;
		cinfo.dither_mode = JDITHER_ORDERED;
		cinfo.desired_number_of_colors = 216;
		cinfo.dct_method = JDCT_FASTEST;
		cinfo.do_fancy_upsampling = FALSE;

		/* call my error handler if there is an error */
		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = my_error_exit;
		if (setjmp(jerr.setjmp_buffer)) {
			/* if we are here, we have a JPEG error */
			printf ("FreeWRL Image problem - could not read %s\n", filename);
			jpeg_destroy_compress((j_compress_ptr)&cinfo);
			fclose (infile);
			releaseTexture(loadThisTexture->scenegraphNode); 
			return;
		}


		jpeg_create_decompress(&cinfo);

		/* Specify data source for decompression */
		jpeg_stdio_src(&cinfo, infile);

		/* Read file header, set default decompression parameters */
		/* (void) jpeg_read_header(&cinfo, TRUE); */
		tempInt = jpeg_read_header(&cinfo, TRUE);


		/* Start decompressor */
		(void) jpeg_start_decompress(&cinfo);



		row = (JSAMPLE*)MALLOC(cinfo.output_width * sizeof(JSAMPLE)*cinfo.output_components);
		rowptr[0] = row;
		image_data = (unsigned char *)MALLOC(cinfo.output_width * sizeof (JSAMPLE) * cinfo.output_height * cinfo.output_components);
		/* Process data */
		for (rowcount = 0; rowcount < cinfo.output_height; rowcount++) {
			nrows = jpeg_read_scanlines(&cinfo, rowptr, 1);
			/* yield for a bit */
			sched_yield();


			for (columncount = 0; columncount < cinfo.output_width; columncount++) {
				for(dp=0; dp<cinfo.output_components; dp++) {
					image_data[(cinfo.output_height-rowcount-1)
							*cinfo.output_width*cinfo.output_components
					       		+ columncount* cinfo.output_components	+dp]
						= row[columncount*cinfo.output_components + dp];
				}
			}
		}


		if (jpeg_finish_decompress(&cinfo) != TRUE) {
			printf("warning: jpeg_finish_decompress error\n");
			releaseTexture(loadThisTexture->scenegraphNode);
		}
		jpeg_destroy_decompress(&cinfo);
		FREE_IF_NZ(row);

		store_tex_info(loadThisTexture,
			cinfo.output_components, 
			(int)cinfo.output_width,
			(int)cinfo.output_height,image_data,cinfo.output_components==4);
	} else {
		/* assume a PNG, whether direct, or converted from (eg) gif */
		rc = readpng_init(infile, &image_width, &image_height);
		if (rc != 0) {
		releaseTexture(loadThisTexture->scenegraphNode);
		switch (rc) {
			case 1:
				printf("[%s] is not a PNG file: incorrect signature\n", filename);
				break;
			case 2:
				printf("[%s] has bad IHDR (libpng longjmp)\n", filename);
				break;
			case 4:
				printf("insufficient memory\n");
				break;
			default:
				printf("unknown readpng_init() error\n");
				break;
			}
		} else {
			image_data = readpng_get_image(display_exponent, &image_channels,
					&image_rowbytes);

			store_tex_info (loadThisTexture, 
				image_channels, 
				(int)image_width, (int)image_height,
				image_data,image_channels==4);
		}
		readpng_cleanup (FALSE);
	}
	fclose (infile);
}

#endif /* HAVE_JPEGLIB_H */

#endif


static void __reallyloadMovieTexture () {

        int x,y,depth,frameCount;
        void *ptr;

        ptr=NULL;

        mpg_main(loadThisTexture->filename, &x,&y,&depth,&frameCount,&ptr);

	#ifdef TEXVERBOSE
	printf ("have x %d y %d depth %d frameCount %d ptr %d\n",x,y,depth,frameCount,ptr);
	#endif

	store_tex_info (loadThisTexture, 
		depth, 
	x, y, ptr,depth==4);

	/* and, manually put the frameCount in. */
	loadThisTexture->frames = frameCount;
}

void getMovieTextureOpenGLFrames(int *highest, int *lowest,int myIndex) {
        struct textureTableIndexStruct *ti;

/*        if (myIndex  == 0) {
		printf ("getMovieTextureOpenGLFrames, myIndex is ZERL\n");
		*highest=0; *lowest=0;
	} else {
*/
	*highest=0; *lowest=0;
	
	#ifdef TEXVERBOSE
	printf ("in getMovieTextureOpenGLFrames, calling getTableIndex\n");
	#endif

       	ti = getTableIndex(myIndex);

	if (ti->frames>0) {
		if (ti->OpenGLTexture != NULL) {
			*lowest = ti->OpenGLTexture[0];
			*highest = ti->OpenGLTexture[(ti->frames) -1];
		}
	}
}

