// freeWRLCtrl.cpp : Implementation of the CfreeWRLCtrl ActiveX Control class.

#include "stdafx.h"
#include "../dllFreeWRL/dllFreeWRL.h"
#include "freeWRLAx.h"
#include "freeWRLCtrl.h"
#include "freeWRLPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CfreeWRLCtrl, COleControl)



// Message map

BEGIN_MESSAGE_MAP(CfreeWRLCtrl, COleControl)
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_WM_MOUSEMOVE()
	//ON_WM_MOUSEWHEEL()  
	ON_WM_LBUTTONDOWN()  
	ON_WM_LBUTTONUP()  
	//ON_WM_LBUTTONDBLCLK() 
	ON_WM_RBUTTONDOWN()  
	ON_WM_RBUTTONUP()  
	//ON_WM_RBUTTONDBLCLK()  
	ON_WM_MBUTTONDOWN() 
	ON_WM_MBUTTONUP()  
	//ON_WM_MBUTTONDBLCLK() 
	ON_WM_KEYDOWN() 
	ON_WM_KEYUP() 
	ON_WM_CHAR() 
	ON_WM_TIMER()
END_MESSAGE_MAP()



// Dispatch map

BEGIN_DISPATCH_MAP(CfreeWRLCtrl, COleControl)
END_DISPATCH_MAP()



// Event map

BEGIN_EVENT_MAP(CfreeWRLCtrl, COleControl)
END_EVENT_MAP()



// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CfreeWRLCtrl, 1)
	PROPPAGEID(CfreeWRLPropPage::guid)
END_PROPPAGEIDS(CfreeWRLCtrl)



// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CfreeWRLCtrl, "FREEWRLAX.freeWRLCtrl.1",
	0x582c9301, 0xa2c8, 0x45fc, 0x83, 0x1b, 0x65, 0x4d, 0xe7, 0xf3, 0xaf, 0x11)



// Type library ID and version

IMPLEMENT_OLETYPELIB(CfreeWRLCtrl, _tlid, _wVerMajor, _wVerMinor)



// Interface IDs

const IID BASED_CODE IID_DfreeWRLAx =
		{ 0xC3C32307, 0x89C2, 0x4C1A, { 0xB7, 0xF1, 0xD5, 0x61, 0xAA, 0x6, 0x53, 0xA3 } };
const IID BASED_CODE IID_DfreeWRLAxEvents =
		{ 0x91DEB0AA, 0x9E7D, 0x43F4, { 0x80, 0xC4, 0x12, 0x5C, 0x53, 0x5B, 0x30, 0xEC } };



// Control type information

static const DWORD BASED_CODE _dwfreeWRLAxOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CfreeWRLCtrl, IDS_FREEWRLAX, _dwfreeWRLAxOleMisc)



// CfreeWRLCtrl::CfreeWRLCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CfreeWRLCtrl

BOOL CfreeWRLCtrl::CfreeWRLCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegInsertable | afxRegApartmentThreading to afxRegInsertable.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_FREEWRLAX,
			IDB_FREEWRLAX,
			afxRegInsertable | afxRegApartmentThreading,
			_dwfreeWRLAxOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}



// CfreeWRLCtrl::CfreeWRLCtrl - Constructor

CfreeWRLCtrl::CfreeWRLCtrl()
{
	InitializeIIDs(&IID_DfreeWRLAx, &IID_DfreeWRLAxEvents);
	// TODO: Initialize your control's instance data here.
    m_cstrFileName = "";
	m_initialized = 0;
	m_frontEndGettingFile = false;
	//this->m_timerID = this->SetTimer(14, 50, NULL);
	//TRACE("timer %d is set\n", m_pCtrl->m_timerID);
}



// CfreeWRLCtrl::~CfreeWRLCtrl - Destructor

CfreeWRLCtrl::~CfreeWRLCtrl()
{
	// TODO: Cleanup your control's instance data here.
	m_initialized = 0;
	this->KillTimer(m_timerID);
	m_dllfreewrl.onClose();
}



// CfreeWRLCtrl::OnDraw - Drawing function

void CfreeWRLCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	if (!pdc)
		return;
	if(m_initialized ==0)return;
	if(m_initialized ==10)
	{
		m_initialized = 1;
		m_dllfreewrl.onInit((unsigned long)this->m_hWnd,rcBounds.right-rcBounds.left,rcBounds.bottom-rcBounds.top);
		m_dllfreewrl.onLoad(m_cstrFileName.GetBuffer()); 
		//m_dllfreewrl.onLoad("C:/source2/tests/2.wrl"); 
		//AfxMessageBox("Initialized freewrl");
		this->m_timerID = this->SetTimer(14, 100, NULL);

	}
	m_dllfreewrl.onResize(rcBounds.right-rcBounds.left,rcBounds.bottom-rcBounds.top);
}

void CfreeWRLCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.
	// MimeType sample program says SRC property is where Mime handler 
	//   passes in URL
	PX_String(pPX, "SRC", m_cstrFileName); 
	//AfxMessageBox("DoPropExchange");
	m_initialized = 10;
}



// CfreeWRLCtrl::OnResetState - Reset control to default state

void CfreeWRLCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}
void CfreeWRLCtrl::OnLButtonDown(UINT nFlags,CPoint point)
{
	m_dllfreewrl.onMouse( 4, 1,point.x,point.y);
	COleControl::OnLButtonDown(nFlags,point);
}
void CfreeWRLCtrl::OnLButtonUp(UINT nFlags,CPoint point)
{
	m_dllfreewrl.onMouse( 5, 1,point.x,point.y);
	COleControl::OnLButtonUp(nFlags,point);
}
void CfreeWRLCtrl::OnMButtonDown(UINT nFlags,CPoint point)
{
	m_dllfreewrl.onMouse( 4, 2,point.x,point.y);
	COleControl::OnMButtonDown(nFlags,point);
}
void CfreeWRLCtrl::OnMButtonUp(UINT nFlags,CPoint point)
{
	m_dllfreewrl.onMouse( 5, 2,point.x,point.y);
	COleControl::OnMButtonUp(nFlags,point);
}
void CfreeWRLCtrl::OnRButtonDown(UINT nFlags,CPoint point)
{
	m_dllfreewrl.onMouse( 4, 3,point.x,point.y);
	COleControl::OnRButtonDown(nFlags,point);
}
void CfreeWRLCtrl::OnRButtonUp(UINT nFlags,CPoint point)
{
	m_dllfreewrl.onMouse( 5, 3,point.x,point.y);
	COleControl::OnRButtonUp(nFlags,point);
}

void CfreeWRLCtrl::OnMouseMove(UINT nFlags,CPoint point)
{
//#define KeyChar         1
//#if defined(AQUA) || defined(WIN32)
//#define KeyPress        2
//#define KeyRelease      3
//#define ButtonPress     4
//#define ButtonRelease   5
//#define MotionNotify    6
//#define MapNotify       19
	/* butnum=1 left butnum=3 right (butnum=2 middle, not used by freewrl) */

	//m_dllfreewrl.onMouse(int mouseAction,int mouseButton,int x, int y);
	m_dllfreewrl.onMouse( 6, 0,point.x,point.y);
	COleControl::OnMouseMove(nFlags,point);
}

void CfreeWRLCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_dllfreewrl.onKey(m_dllfreewrl.KEYDOWN,nChar);
	COleControl::OnKeyDown(nChar,nRepCnt,nFlags);
}
void CfreeWRLCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_dllfreewrl.onKey(m_dllfreewrl.KEYUP,nChar);
	COleControl::OnKeyUp(nChar,nRepCnt,nFlags);
}
void CfreeWRLCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_dllfreewrl.onKey(m_dllfreewrl.KEYPRESS,nChar);
	COleControl::OnChar(nChar,nRepCnt,nFlags);
}
void CfreeWRLCtrl::OnTimer(UINT_PTR nIDEvent)
{
	//m_dllfreewrl.print("t");
	if(nIDEvent == this->m_timerID && (m_initialized ==1))
	{
		//m_dllfreewrl.print(".");
		char *url = m_dllfreewrl.downloadFileName();
		if(url != NULL)
		{
			char stemp[1000];
			sprintf(stemp,"ontick FE got [%s] m_fegf=%s\n",url,m_frontEndGettingFile ? "true" : "false");
			m_dllfreewrl.print(stemp);
			if(!m_frontEndGettingFile) 
			{
				m_dllfreewrl.print("UrlDownloading...\n");
				m_frontEndGettingFile = true;
				int iret;
				int ii = 0;
				if(url[0] == '/') ii = 1;
				char localfile[1024];
				HRESULT hr;
				hr = URLDownloadToCacheFileA(
					(LPUNKNOWN)this->GetControllingUnknown(), //target,   // control's IUnknown
					&url[ii],                         // URL to download
					localfile, 
					1023,                               // buffer length
					0,                                      // reserved
					NULL );
				iret = (hr == 0? 1 : -1);
				sprintf(stemp,"localf [%s] hr=%d\n",localfile,(int)hr);
				m_dllfreewrl.print(stemp);
				m_dllfreewrl.downloadComplete(localfile,iret);
				m_frontEndGettingFile = false;
			}
		}
	}
	COleControl::OnTimer(nIDEvent); 
	m_dllfreewrl.onTick(m_initialized);
}

// CfreeWRLCtrl message handlers
