/*
=INSERT_TEMPLATE_HERE=

$Id: Component_CubeMapTexturing.c,v 1.4 2009/05/07 17:01:24 crc_canada Exp $

X3D Cubemap Texturing Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"


/* testing */

#define CUBE_MAP_SIZE 256
static unsigned char CubeMap[6][CUBE_MAP_SIZE][CUBE_MAP_SIZE][4];


#ifndef GL_EXT_texture_cube_map
# define GL_NORMAL_MAP_EXT                   0x8511
# define GL_REFLECTION_MAP_EXT               0x8512
# define GL_TEXTURE_CUBE_MAP_EXT             0x8513
# define GL_TEXTURE_BINDING_CUBE_MAP_EXT     0x8514
# define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A
# define GL_PROXY_TEXTURE_CUBE_MAP_EXT       0x851B
# define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT    0x851C
#endif


enum {CUBE_POS_X, CUBE_NEG_X, CUBE_POS_Y, CUBE_NEG_Y, CUBE_POS_Z, CUBE_NEG_Z};


GLenum cubefaces[6] = {
  GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,
};

static float
Dot3(float *a, float *b)
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
  
static float *
Scale3(float *result, float *a, float scale)
{
  result[0] = a[0] * scale; 
  result[1] = a[1] * scale;
  result[2] = a[2] * scale;
  return result;
} 

static float *
Normalize3(float *result, float *a)
{
  float length;

  length = (float) sqrt(Dot3(a, a));
  return Scale3(result, a, 1 / length);
}

static unsigned char *
CubeFunc(unsigned char resultColor[3], float vec[3])
{
  int i;
  float faceVec[3];

  if (vec[0] == 1.0) {
    resultColor[0] = 255;
    resultColor[1] = 0;
    resultColor[2] = 0;
  } else if (vec[1] == 1.0) {
    resultColor[0] = 0;
    resultColor[1] = 255;
    resultColor[2] = 0;
  } else if (vec[2] == 1.0) {
    resultColor[0] = 0;
    resultColor[1] = 0;
    resultColor[2] = 255;
  } else if (vec[0] == -1.0) {
    resultColor[0] = 255;
    resultColor[1] = 0;
    resultColor[2] = 255;
  } else if (vec[1] == -1.0) {
    resultColor[0] = 255;
    resultColor[1] = 255;
    resultColor[2] = 0;
  } else if (vec[2] == -1.0) {
    resultColor[0] = 0;
    resultColor[1] = 255;
    resultColor[2] = 255;
  }
  return resultColor;

  Normalize3(faceVec, vec);
  for (i = 0; i < 3; i++) {
    resultColor[i] = 255 * (sin(6 * (faceVec[i] + faceVec[(i + 1) % 3])) + 1) / 2.0;
  }
  return resultColor;
}


void render_ComposedCubeMapTexture(struct X3D_ComposedCubeMapTexture *node) {
	int i,j,k;

  for (i = 0; i < CUBE_MAP_SIZE; i++) {
    float t = 1.0 / (2 * CUBE_MAP_SIZE) + (float) i / CUBE_MAP_SIZE;
    t = 2.0 * t - 1.0;
    for (j = 0; j < CUBE_MAP_SIZE; j++) {
      float s = 1.0 / (2 * CUBE_MAP_SIZE) + (float) j / CUBE_MAP_SIZE;
      float pt[3];
      s = 2.0 * s - 1.0;
      pt[0] = 1;
      pt[1] = t;
      pt[2] = -s;
      CubeFunc(CubeMap[CUBE_POS_X][i][j], pt);
      pt[0] = -1;
      pt[1] = t;
      pt[2] = s;
      CubeFunc(CubeMap[CUBE_NEG_X][i][j], pt);

      pt[1] = 1;
      pt[0] = s;
      pt[2] = -t;
      CubeFunc(CubeMap[CUBE_POS_Y][i][j], pt);
      pt[1] = -1;
      pt[0] = s;
      pt[2] = t;
      CubeFunc(CubeMap[CUBE_NEG_Y][i][j], pt);
      pt[2] = 1;
      pt[0] = s;
      pt[1] = t;
      CubeFunc(CubeMap[CUBE_POS_Z][i][j], pt);
      pt[2] = -1;
      pt[0] = -s;
      pt[1] = t;
      CubeFunc(CubeMap[CUBE_NEG_Z][i][j], pt);
      for (k = CUBE_POS_X; k <= CUBE_NEG_Z; k++) {
        CubeMap[k][i][j][3] = 255;
      }
    }
  }

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  for (i = 0; i < 6; i++) {
    glTexImage2D(
      cubefaces[i],
      0,                  // level
      GL_RGBA8,          // internal format
      CUBE_MAP_SIZE,     // width
      CUBE_MAP_SIZE,     // height
      0,                 // border
      GL_RGBA,           // format
      GL_UNSIGNED_BYTE,   // type
      CubeMap[CUBE_POS_X + i]); // pixel data
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
  glEnable(GL_TEXTURE_CUBE_MAP_EXT);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);
  glEnable(GL_NORMALIZE);
}



/* end of testing */
void ixxrender_ComposedCubeMapTexture(struct X3D_ComposedCubeMapTexture *node) {
	struct X3D_Node *thistex = 0;
	int count;

	for (count=0; count<6; count++) {
		/* go through these, back, front, top, bottom, right left */
		switch (count) {
			case 0: {POSSIBLE_PROTO_EXPANSION(node->front,thistex);  break;}
			case 1: {POSSIBLE_PROTO_EXPANSION(node->back,thistex);   break;}
			case 2: {POSSIBLE_PROTO_EXPANSION(node->top,thistex);    break;}
			case 3: {POSSIBLE_PROTO_EXPANSION(node->bottom,thistex); break;}
			case 4: {POSSIBLE_PROTO_EXPANSION(node->right,thistex);  break;}
			case 5: {POSSIBLE_PROTO_EXPANSION(node->left,thistex);   break;}
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
			} 
		}
	}
}

void render_GeneratedCubeMapTexture(struct X3D_GeneratedCubeMapTexture *node) {
}

void render_ImageCubeMapTexture(struct X3D_ImageCubeMapTexture *node) {
}

