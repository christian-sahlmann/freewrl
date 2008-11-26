/*
  aquaInterface.h
  FreeWRL

  Created by Sarah Dumoulin on Mon Jan 19 2004.
  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
*/

void updateContext();
float getWidth();
float getHeight();
void  setAquaCursor(int ctype);
void setMenuButton_collision(int val);
void setMenuButton_texSize(int size);
void setMenuStatus(char* stat);
void setMenuButton_headlight(int val);
void setMenuFps(float fps);
int aquaSetConsoleMessage(char* str);
void setMenuButton_navModes(int type);
void createAutoReleasePool();
