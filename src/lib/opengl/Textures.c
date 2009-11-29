/*
  $Id: Textures.c,v 1.35 2009/11/29 16:38:21 crc_canada Exp $

  FreeWRL support library.
  Texture handling code.

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
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>
#include <list.h>
#include <resources.h>

#include <threads.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"

#include "../scenegraph/readpng.h"
#include "../input/InputFunctions.h"
#include "Textures.h"
#include "../opengl/Material.h"
#include "../opengl/OpenGL_Utils.h"

#include "LoadTextures.h"

#ifdef AQUA
# include <Carbon/Carbon.h>
# include <QuickTime/QuickTime.h>
#else
# if HAVE_JPEGLIB_H
#undef HAVE_STDLIB_H
#undef FAR
#undef HAVE_BOOLEAN
#  include <jpeglib.h>
#  include <setjmp.h>
# endif
#endif


#define DO_POSSIBLE_TEXTURE_SEQUENCE if (myTableIndex->status == TEX_NEEDSBINDING) { \
                do_possible_textureSequence(myTableIndex); \
                return;	\
		}

/* we keep track of which textures have been loaded, and which have not */
extern void *texParams[];


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

/* current index into loadparams that texture thread is working on */
int currentlyWorkingOn = -1;

int textureInProcess = -1;

/* for texture remapping in TextureCoordinate nodes */
int	*global_tcin;
int	global_tcin_count;
void 	*global_tcin_lastParent;

#if defined(AQUA) /* for AQUA OS X sharing of OpenGL Contexts */

/* # include "CGDirectDisplay.h" */

CGLPixelFormatAttribute attribs[] = { kCGLPFADisplayMask, 0,
                                      kCGLPFAFullScreen,
                                      kCGLPFADoubleBuffer,
                                      0 };

CGLPixelFormatObj pixelFormat = NULL;
long numPixelFormats = 0;
CGLContextObj aqtextureContext = NULL;
static int useQuicktime = FALSE;


#elif defined(WIN32)

#else

GLXContext textureContext = NULL;

#endif

/* function Prototypes */
int findTextureFile(struct textureTableIndexStruct *entry);
void _textureThread(void);

static void __reallyloadPixelTexure(void);
static void __reallyloadImageTexture(void);
static void __reallyloadMovieTexture(void);
void do_possible_textureSequence(struct textureTableIndexStruct*);
struct Uni_String *newASCIIString(char *str);

int readpng_init(FILE *infile, ulg *pWidth, ulg *pHeight);
void readpng_cleanup(int free_image_data);


/**
 *   texst: texture status string.
 */
const char *texst(int num)
{
	if (num == TEX_NOTLOADED) return "TEX_NOTLOADED";
	if (num == TEX_LOADING) return "TEX_LOADING";
	if (num == TEX_NEEDSBINDING)return "TEX_NEEDSBINDING";
	if (num == TEX_LOADED)return "TEX_LOADED";
	if (num == TEX_UNSQUASHED)return "UNSQUASHED";
	return "unknown";
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
    if (textureInProcess > 0) {
	printf("call to isTextureParsing %d, returning %d\n",
	       textureInProcess,textureInProcess > 0);
    }
#endif
	return textureInProcess >0;
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
		} else if (node->_nodeType == NODE_VRML1_Texture2) {
			tableIndex  = ((struct X3D_VRML1_Texture2 *)node)->__textureTableIndex;
		} else return;

#ifdef TEXVERBOSE
	printf ("releaseTexture, calling getTableIndex\n");
	ti = getTableIndex(tableIndex);
	printf ("releaseTexture, ti %u, ti->status %d\n",(int) ti,ti->status);
	ti->status = TEX_NOTLOADED;

	if (ti->OpenGLTexture != TEXTURE_INVALID) {
		printf ("deleting %d textures, starting at %u\n",ti->frames, ti->OpenGLTexture);
		ti->OpenGLTexture = TEXTURE_INVALID;
/* 		FREE_IF_NZ(ti->OpenGLTexture); */
	}
#endif

	ti = getTableIndex(tableIndex);
	ti->status = TEX_NOTLOADED;
	if (ti->OpenGLTexture != TEXTURE_INVALID) {
		glDeleteTextures(1, &ti->OpenGLTexture);
		ti->OpenGLTexture = TEXTURE_INVALID;
/* 		FREE_IF_NZ(ti->OpenGLTexture); */
	}
}

/* called on "kill oldworld" */
void kill_openGLTextures() 
{
	int count;
	struct textureTableStruct * listRunner;
	struct textureTableStruct * tmp;
	
	/* remove the OpenGL textures */
	listRunner = readTextureTable;
	
	while (listRunner != NULL) {
		/* zero out the fields in this new block */
		for (count = 0; count < 32; count ++) {
			if (listRunner->entry[count].OpenGLTexture != TEXTURE_INVALID) {
				DEBUG_TEX("deleting %u\n", listRunner->entry[count].OpenGLTexture);
				glDeleteTextures(1, &listRunner->entry[count].OpenGLTexture);
				listRunner->entry[count].OpenGLTexture = TEXTURE_INVALID;
/* 				FREE_IF_NZ (listRunner->entry[count].OpenGLTexture); */
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
	printf ("getTableIndex, going to return %d\n", (int) (&(currentBlock->entry[whichEntry])));
	printf ("textureTableIndexStruct, sgn0 is %d, sgn1 is %d\n",
		(int) currentBlock->entry[0].scenegraphNode,
		(int) currentBlock->entry[1].scenegraphNode);
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

		DEBUG_TEX("CREATING TEXTURE NODE: type %d url ", it->_nodeType);
		/* Multi_String_print(&it->url); */
		DEBUG_TEX("parent url: %s\n", it->__parenturl->strptr);

		if ((nextFreeTexture & 0x1f) == 0) {

			newStruct = (struct textureTableStruct*) MALLOC (sizeof (struct textureTableStruct));
			
			/* zero out the fields in this new block */
			for (count = 0; count < 32; count ++) {
				/* newStruct->entry[count].nodeType = 0; */
				newStruct->entry[count].status = TEX_NOTLOADED;
				newStruct->entry[count].OpenGLTexture = TEXTURE_INVALID;
				newStruct->entry[count].frames = 0;
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
			v1t = (struct X3D_VRML1_Texture2 *) tmp;
			v1t->__textureTableIndex = nextFreeTexture;
		}

		currentBlock->entry[whichEntry].nodeType = it->_nodeType;
		/* set the scenegraphNode here */
		currentBlock->entry[whichEntry].scenegraphNode = X3D_NODE(tmp);
#ifdef TEXVERBOSE
		printf ("registerNode, sgn0 is %d, sgn1 is %d\n",
			(int) currentBlock->entry[0].scenegraphNode,
			(int) currentBlock->entry[1].scenegraphNode);
#endif
		/* now, lets increment for the next texture... */
		nextFreeTexture += 1;
	}
}

/* do TextureBackground textures, if possible */
void loadBackgroundTextures (struct X3D_Background *node) {
	struct X3D_ImageTexture *thistex;
	struct Multi_String thisurl;
	int count;

	/* initialization */
	thisurl.n = 0; thisurl.p = NULL;
	thistex = NULL;

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

	    DEBUG_TEX("FORCE NODE RELOAD: %p %s\n", node, stringNodeType(node->_nodeType));

	/* force a node reload - make it a new texture. Don't change
	   the parameters for the original number, because if this
	   texture is shared, then ALL references will change! so,
	   we just accept that the current texture parameters have to
	   be left behind. */
	MARK_NODE_COMPILED;
	
	/* this will cause bind_image to create a new "slot" for this texture */
	/* cast to GLuint because __texture defined in VRMLNodes.pm as SFInt */
	
	switch (node->_nodeType) {

		case NODE_MovieTexture: {
	    		mym = (struct X3D_MovieTexture *)node;
	    		/*  did the URL's change? we can't test for _change here, because
				movie running will change it, so we look at the urls. */
			    if ((mym->url.p) != (mym->__oldurl.p)) {
				releaseTexture(node); 
				mym->__oldurl.p = mym->url.p;
			    }
		}
		break;

		case NODE_PixelTexture:
	    		releaseTexture(node); 
		break;

		case NODE_ImageTexture:
	    		releaseTexture(node); 
		break;

		case NODE_VRML1_Texture2:
	    		releaseTexture(node); 
		break;

		default: {
			printf ("loadTextureNode, unknown node type %s\n",stringNodeType(node->_nodeType));
			return;
		}

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
			(int) node->texture.p[0], (int) node->texture.p[1]);
	printf ("	change %d ichange %d\n",node->_change, node->_ichange);
#endif
	
	/* new node, or node paramaters changed */
        if (NODE_NEEDS_COMPILING) {
	    /*  have to regen the shape*/
	    MARK_NODE_COMPILED;

		/* alloc fields, if required - only do this once, even if node changes */
		if (node->__params == 0) {
			/* printf ("loadMulti, MALLOCing for params\n"); */
			node->__params = MALLOC (sizeof (struct multiTexParams) * rdr_caps.texture_units);
			paramPtr = (struct multiTexParams*) node->__params;

			/* set defaults for these fields */
			for (count = 0; count < rdr_caps.texture_units; count++) {
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
		if (max > rdr_caps.texture_units) max = rdr_caps.texture_units;

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
	if (max > rdr_caps.texture_units) max = rdr_caps.texture_units;

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


/* do we do 1 texture, or is this a series of textures, requiring final binding
   by this thread? */

void do_possible_textureSequence(struct textureTableIndexStruct* me) {
	int rx,ry,sx,sy;
	int x,y;
	GLint iformat;
	GLenum format;
	int count;

	/* default texture properties; can be changed by a TextureProperties node */
	float anisotropicDegree=0.0;
	struct SFColorRGBA borderColor;
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

	/* initialization */
	Src = 0; Trc = 0;
	tpNode = NULL;

	/* do we need to convert this to an OpenGL texture stream?*/

	/* we need to get parameters. */	
	if (me->OpenGLTexture == TEXTURE_INVALID) {
/* 		me->OpenGLTexture = MALLOC (sizeof (GLuint) * me->frames); */
		glGenTextures(1, &me->OpenGLTexture);
#ifdef TEXVERBOSE
		printf ("just glGend texture for block %d is %u\n",
			(int) me, me->OpenGLTexture);
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

/* 	for (count = 0; count < me->frames; count ++) { */
		glBindTexture (GL_TEXTURE_2D, me->OpenGLTexture);
	
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

	
		/* NOTE: trying BGRA format from textures */
		iformat = GL_RGBA; format = GL_BGRA;
	
		/* do the image. */
		if(x && y) {
			int texOk = FALSE;
			unsigned char *dest = mytexdata;

			/* do we have to do power of two textures? */
			if (GL_ARB_texture_non_power_of_two) {
				rx = x; ry = y;
			} else {
				/* find a power of two that fits */
				rx = 1;
				sx = x;
				while(sx) {sx /= 2; rx *= 2;}
				if(rx/2 == x) {rx /= 2;}
				ry = 1; 
				sy = y;
				while(sy) {sy /= 2; ry *= 2;}
				if(ry/2 == y) {ry /= 2;}
			}

			if (global_print_opengl_errors) {
				DEBUG_MSG("initial texture scale to %d %d\n",rx,ry);
			}

			if(rx != x || ry != y || rx > rdr_caps.max_texture_size || ry > rdr_caps.max_texture_size) {
				/* do we have texture limits??? */
				if (rx > rdr_caps.max_texture_size) rx = rdr_caps.max_texture_size;
				if (ry > rdr_caps.max_texture_size) ry = rdr_caps.max_texture_size;
			}

			if (global_print_opengl_errors) {
				DEBUG_MSG("texture size after maxTextureSize taken into account: %d %d, from %d %d\n",rx,ry,x,y);
			}
	
			/* try this texture on for size, keep scaling down until we can do it */
			texOk = FALSE;
			/* all textures are 4 bytes/pixel */
			dest = (unsigned char *)MALLOC((unsigned) 4 * rx * ry);
			while (!texOk) {
				GLint width, height;
				gluScaleImage(format, x, y, GL_UNSIGNED_BYTE, mytexdata, rx, ry, GL_UNSIGNED_BYTE, dest);
				glTexImage2D(GL_PROXY_TEXTURE_2D, 0, iformat,  rx, ry, 0, format, GL_UNSIGNED_BYTE, dest);

				glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0,GL_TEXTURE_WIDTH, &width); 
				glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0,GL_TEXTURE_HEIGHT, &height); 

				if ((width == 0) || (height == 0)) {
					rx= rx/2; ry = ry/2;
					if (global_print_opengl_errors) {
						DEBUG_MSG("width %d height %d going to try size %d %d, last time %d %d\n",
							  width, height, rx,ry,x,y);
					}
					if ((rx==0) || (ry==0)) {
					    ConsoleMessage ("out of texture memory");
					    me->status = TEX_LOADED; /* yeah, right */
					    return;
					}
				} else {
					texOk = TRUE;
				}
			}


			if (global_print_opengl_errors) {
				DEBUG_MSG("after proxy image stuff, size %d %d\n",rx,ry);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, iformat,  rx, ry, 0, format, GL_UNSIGNED_BYTE, dest);
			glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP, GL_TRUE);

			if(mytexdata != dest) {FREE_IF_NZ(dest);}

			/* we can get rid of the original texture data here */
			FREE_IF_NZ(me->texdata);
		}


/* 		/\* increment, used in movietextures for frames more than 1 *\/ */
/* 		mytexdata += x*y*me->depth; */
/* 	} */

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
	myTableIndex = getTableIndex(thisTexture);

	/* printf ("new_bind_image, I am %u, texture_count %d, thisTexture is %u status %s\n",
		node,texture_count,thisTexture,texst(myTableIndex->status));  */

	/* default here; this is just a blank texture */
	bound_textures[texture_count] = defaultBlankTexture;

	switch (myTableIndex->status) {
		case TEX_NOTLOADED:
			send_texture_to_loader(myTableIndex);
			break;

		case TEX_LOADING:
			currentlyWorkingOn = thisTexture;
			loadThisTexture = myTableIndex;
			break;

		case TEX_NEEDSBINDING:
                	do_possible_textureSequence(myTableIndex); 
			break;

		case TEX_LOADED:
			DEBUG_TEX("now binding to pre-bound tex %u\n", myTableIndex->OpenGLTexture);
	
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
				if (myTableIndex->OpenGLTexture == TEXTURE_INVALID) {
	
					DEBUG_TEX("no openGLtexture here status %s\n", texst(myTableIndex->status));
					return;
				}
	
				bound_textures[texture_count] = myTableIndex->OpenGLTexture;
			} else {
				bound_textures[texture_count] = 
					((struct X3D_MovieTexture *)myTableIndex->scenegraphNode)->__ctex;
			}
	
			/* save the texture params for when we go through the MultiTexture stack. Non
			   MultiTextures should have this texture_count as 0 */
			 
			texParams[texture_count] = param; 
	
			textureInProcess = -1; /* we have finished the whole process */
			break;
			
		case TEX_UNSQUASHED:
		default: {
			printf ("unknown texture status %d\n",myTableIndex->status);
		}
	}
}

/* FIXME: removed old "really load functions" ... needs to implement loading
          of movie textures.
*/
static void __reallyloadMovieTexture () {

        int x,y,depth,frameCount;
        void *ptr;

        ptr=NULL;

        mpg_main(loadThisTexture->filename, &x,&y,&depth,&frameCount,&ptr);

	#ifdef TEXVERBOSE
	printf ("have x %d y %d depth %d frameCount %d ptr %d\n",x,y,depth,frameCount,ptr);
	#endif

	/* store_tex_info(loadThisTexture, depth, x, y, ptr,depth==4); */

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

/* 	if (ti->frames>0) { */
		if (ti->OpenGLTexture != TEXTURE_INVALID) {
			*lowest = ti->OpenGLTexture;
			*highest = 0;
/* 			*highest = ti->OpenGLTexture[(ti->frames) -1]; */
		}
/* 	} */
}

