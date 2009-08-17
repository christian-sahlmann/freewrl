/*
=INSERT_TEMPLATE_HERE=

$Id: LoadTextures.h,v 1.1 2009/08/17 22:25:58 couannette Exp $

 */

#ifndef __FREEWRL_LOAD_TEXTURES_H__
#define __FREEWRL_LOAD_TEXTURES_H__


/* Definitions for renderer capabilities */
typedef bool bool_t;

typedef struct {

    bool_t av_multitexture; /* Multi textures available */
    bool_t av_glsl_shaders; /* GLSL shaders available   */ 
    bool_t av_npot_texture; /* Non power of 2 textures  */
/*     boot_t av_texture_rect; /\* Rectangle textures *\/ */

    int texture_units;
    unsigned max_texture_size[2];

} s_renderer_capabilities_t;

extern s_renderer_capabilities_t rdr_caps;

#define BOOL_STR(_bool) ((_bool) ? "true" : "false")

/* Loading functions */

/* transition function */
void findTextureFile_MB(int cwo);

/* new functions */
void texture_loader_initialize();
bool is_url(const char *url);
bool load_texture_from_file(struct X3D_ImageTexture *node, const char *filename);
bool bind_texture(struct X3D_ImageTexture *node);


#endif /* __FREEWRL_LOAD_TEXTURES_H__ */
