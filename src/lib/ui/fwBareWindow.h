/*
=INSERT_TEMPLATE_HERE=

$Id: fwBareWindow.h,v 1.1 2009/02/18 13:37:50 istakenv Exp $

UI declarations.

*/

#ifndef __FREEWRL_BAREWINDOW_UI_H__
#define __FREEWRL_BAREWINDOW_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

void setMessageBar(void);
void getBareWindowedGLwin(Window *);
void openBareMainWindow(int, char **);
void createBareMainWindow();

#ifdef __cplusplus
}
#endif

#endif /* __FREEWRL_BAREWINDOW_UI_H__ */
