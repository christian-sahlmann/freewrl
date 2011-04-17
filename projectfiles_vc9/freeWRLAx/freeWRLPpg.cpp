// freeWRLPpg.cpp : Implementation of the CfreeWRLPropPage property page class.

#include "stdafx.h"
#include "freeWRLAx.h"
#include "freeWRLPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CfreeWRLPropPage, COlePropertyPage)



// Message map

BEGIN_MESSAGE_MAP(CfreeWRLPropPage, COlePropertyPage)
END_MESSAGE_MAP()



// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CfreeWRLPropPage, "FREEWRLAX.freeWRLPropPage.1",
	0x3d084b9f, 0x2072, 0x437d, 0xba, 0x6c, 0x74, 0xb6, 0xb5, 0xe6, 0xef, 0xdf)



// CfreeWRLPropPage::CfreeWRLPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CfreeWRLPropPage

BOOL CfreeWRLPropPage::CfreeWRLPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_FREEWRLAX_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}



// CfreeWRLPropPage::CfreeWRLPropPage - Constructor

CfreeWRLPropPage::CfreeWRLPropPage() :
	COlePropertyPage(IDD, IDS_FREEWRLAX_PPG_CAPTION)
{
}



// CfreeWRLPropPage::DoDataExchange - Moves data between page and properties

void CfreeWRLPropPage::DoDataExchange(CDataExchange* pDX)
{
	DDP_PostProcessing(pDX);
}



// CfreeWRLPropPage message handlers
