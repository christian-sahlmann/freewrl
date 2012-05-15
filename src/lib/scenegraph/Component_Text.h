/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Text.h,v 1.2 2012/05/15 23:09:10 crc_canada Exp $

X3D Text Component

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


#ifndef __FREEWRL_SCENEGRAPH_TEXT_H__
#define __FREEWRL_SCENEGRAPH_TEXT_H__

void render_Text (struct X3D_Text * node);

#ifdef _ANDROID
void fwg_AndroidFontFile(FILE *myFile,int len);
#endif //ANDROID

#endif  /* __FREEWRL_SCENEGRAPH_TEXT_H__ */
