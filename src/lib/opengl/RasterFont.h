/*
  $Id: RasterFont.h,v 1.3 2009/12/28 00:51:15 couannette Exp $

  FreeWRL support library.
  Raster fonts.

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



#ifndef __LIBFREEWRL_RASTER_FONT_H__
#define __LIBFREEWRL_RASTER_FONT_H__


typedef float  vec4f_t[4];


typedef enum {

    xf_white,
    xf_black,
    xf_user,
    e_xfont_color_max

} e_xfont_color_t;


void rf_print(const char *text);
void rf_printf(int x, int y, const char *format, ...);
void rf_layer2D();
void rf_leave_layer2D();
int  rf_xfont_init(const char *fontname);
void rf_xfont_set_color(e_xfont_color_t index);
void rf_xfont_set_usercolor(vec4f_t color);

#if 0
// used in my engine to print mvar=multi value variables...
void rf_mvar_print(int x, int y, s_mvar_t *mvar);
// mvar is declared like this
typedef union {

    bool_t         vbool;
    int32_t        vint32;
    int64_t        vint64;
    uint32_t       vuint32;
    uint64_t       vuint64;
    float          vfloat;
    double         vdouble;
    mvar_string_t  vstring;
    vec3f_t        vv3float;
    vec4f_t        vv4float;

} s_var_t;
typedef struct {

    mvar_t  type;
    char    name[MVAR_NAME_MAX];
    void   *trigger;
    char    flag;
    bool_t  optarg;
    bool_t  set;
    s_var_t var;
 
} s_mvar_t;
#endif


#endif /* __LIBFREEWRL_RASTER_FONT_H__ */
