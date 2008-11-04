/*******************************************************************
 *
 * FreeX3D main program
 *
 * main.c
 *
 *******************************************************************/

#include "config.h"
#include "system.h"

#include "display.h"
#include "libFreeX3D.h"

#include "options.h"

/**
 * Local variables
 */
static int CaughtSEGV = FALSE;

/**
 * Signal handlers 
 */
void catch_SIGQUIT();
void catch_SIGSEGV();
void catch_SIGHUP();
void catch_SIGALRM(int);

/**
 * Main
 */
int main(int argc, char *argv[])
{
  char *pwd;
  char *initialFilename;	/* file to start FreeWRL with */

  /**
   * TODO: check the library version
   */

  /* set the screen width and height before getting into arguments */
  win_width = 600;
  win_height = 400;

  fullscreen = 0;

  /* install the signal handler for SIGQUIT */
  signal(SIGQUIT, (void(*)(int))catch_SIGQUIT);
  signal(SIGTERM, (void(*)(int))catch_SIGQUIT);
  signal(SIGSEGV,(void(*)(int))catch_SIGSEGV);
  signal(SIGALRM,(void(*)(int))catch_SIGALRM);
  signal(SIGHUP, (void(*)(int))catch_SIGHUP);

  if (parseCommandLine (argc, argv)) {
  }

  if (!initFreeX3D()) {
      fprintf(stderr,"init error\n");
      exit(1);
  }

  sleep(3);

  closeFreeX3D();
}

void catch_SIGQUIT()
{
  /* Shut up any SIGSEGVs we might get now. */
  CaughtSEGV = TRUE;
  /* doQuit(); */
}

void catch_SIGHUP()
{
  /* Anchor_ReplaceWorld(BrowserFullPath); */
}

void catch_SIGSEGV()
{
  if (!CaughtSEGV) {
    CaughtSEGV = TRUE;
  }
  exit(1);
}

void catch_SIGALRM(int sig)
{
  signal(SIGALRM, SIG_IGN);

  alarm(0);
  signal(SIGALRM, catch_SIGALRM);
}
