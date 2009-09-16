/*
=INSERT_TEMPLATE_HERE=

$Id: LoadTextures.h,v 1.2 2009/09/16 22:48:24 couannette Exp $

 */

#ifndef __FREEWRL_LOAD_TEXTURES_H__
#define __FREEWRL_LOAD_TEXTURES_H__


/* Definitions for renderer capabilities */
typedef struct {

    bool av_multitexture; /* Multi textures available */
    bool av_glsl_shaders; /* GLSL shaders available   */ 
    bool av_npot_texture; /* Non power of 2 textures  */
    bool av_texture_rect; /* Rectangle textures */

    int texture_units;
    unsigned max_texture_size[2];

} s_renderer_capabilities_t;

extern s_renderer_capabilities_t rdr_caps;

#define BOOL_STR(_bool) ((_bool) ? "true" : "false")

/* Loading functions */

/* transition function */
bool findTextureFile_MB(int cwo);

/* new functions */
void texture_loader_initialize();
bool is_url(const char *url);
bool load_texture_from_file(struct X3D_ImageTexture *node, char *filename);
bool bind_texture(struct X3D_ImageTexture *node);


#endif /* __FREEWRL_LOAD_TEXTURES_H__ */
