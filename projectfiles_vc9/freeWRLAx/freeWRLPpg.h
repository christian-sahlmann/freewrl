#pragma once

// freeWRLPpg.h : Declaration of the CfreeWRLPropPage property page class.


// CfreeWRLPropPage : See freeWRLPpg.cpp for implementation.

class CfreeWRLPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CfreeWRLPropPage)
	DECLARE_OLECREATE_EX(CfreeWRLPropPage)

// Constructor
public:
	CfreeWRLPropPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_FREEWRLAX };

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	DECLARE_MESSAGE_MAP()
};

