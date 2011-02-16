/*
  $Id: MainLoop.h,v 1.11 2011/02/16 17:46:00 crc_canada Exp $

  FreeWRL support library.
  UI declarations.

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



#ifndef __FREEWRL_MAINLOOP_MAIN_H__
#define __FREEWRL_MAINLOOP_MAIN_H__

extern int currentX[20], currentY[20];

void setDisplayed(int);
void First_ViewPoint();
void Last_ViewPoint();
void Prev_ViewPoint();
void Next_ViewPoint();
void setTextures_take_priority (int x);
void setUseShapeThreadIfPossible(int x);
void toggle_headlight();
void RenderSceneUpdateScene();

/* should be in OpenGL_Utils.h but this would grab all X3D defs.... */
void setglClearColor(float *val);
int isTextureParsing();

/* where this should be ? */
const char* freewrl_get_browser_program();

void resetSensorEvents(void);

#endif /* __FREEWRL_MAINLOOP_MAIN_H__ */
