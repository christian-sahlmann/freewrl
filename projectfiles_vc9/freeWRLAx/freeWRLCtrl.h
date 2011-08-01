#pragma once

// freeWRLCtrl.h : Declaration of the CfreeWRLCtrl ActiveX Control class.


// CfreeWRLCtrl : See freeWRLCtrl.cpp for implementation.

class CfreeWRLCtrl : public COleControl
{
	DECLARE_DYNCREATE(CfreeWRLCtrl)

// Constructor
public:
	CfreeWRLCtrl();

// Overrides
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void OnMouseMove(UINT nFlags,CPoint point);
	virtual void OnLButtonDown(UINT nFlags,CPoint point);
	virtual void OnLButtonUp(UINT nFlags,CPoint point);
	virtual void OnMButtonDown(UINT nFlags,CPoint point);
	virtual void OnMButtonUp(UINT nFlags,CPoint point);
	virtual void OnRButtonDown(UINT nFlags,CPoint point);
	virtual void OnRButtonUp(UINT nFlags,CPoint point);
	virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//virtual void OnTimer(UINT_PTR nIDEvent);
// Implementation
protected:
	~CfreeWRLCtrl();

	DECLARE_OLECREATE_EX(CfreeWRLCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CfreeWRLCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CfreeWRLCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CfreeWRLCtrl)		// Type name and misc status

// Message maps
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	DECLARE_DISPATCH_MAP()

// Event maps
	DECLARE_EVENT_MAP()

// member data
    CString     m_cstrFileName;  // current file name
    CString     m_cstrCacheFileName;    // file name of local cache file
	CString     m_cstrContainerURL; //IE/container document URL
	CdllFreeWRL m_dllfreewrl;
	void* m_Hwnd;
	int m_initialized;
	//bool m_frontEndGettingFile;
	//int m_filerequested;
	//void* m_res;
	//char * m_localfile;
	//char * m_url;
	//int m_lfsize;
	//UINT_PTR m_timerID;

// Dispatch and event IDs
public:
	enum {
	};
};

