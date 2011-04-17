#pragma once

// freeWRLAx.h : main header file for freeWRLAx.DLL

#if !defined( __AFXCTL_H__ )
#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols


// CfreeWRLAxApp : See freeWRLAx.cpp for implementation.

class CfreeWRLAxApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

