// DesignerViewRolldown.h : Declaration of the CDesignerViewRolldown

#pragma once
#include "resource.h"       // main symbols
#include "RWViewAreaLayout.h"
//#include "AtlRollOutCtrl.h"
#include "ConfigIDsRolldown.h"
#include "PanelCtrl.h"

class CGradientFillHelper
{
public:
	typedef WINGDIAPI BOOL WINAPI GradientFillProc(HDC,PTRIVERTEX,ULONG,PVOID,ULONG,ULONG);

public:
	CGradientFillHelper() : m_hLib(NULL), m_pfnGradinetFill(NULL), m_bGradientFillTried(false)
	{
	}
	~CGradientFillHelper()
	{
		if (m_hLib)
		{
			FreeLibrary(m_hLib);
		}
	}

	GradientFillProc* Proc()
	{
		if (!m_bGradientFillTried)
		{
			m_bGradientFillTried = true;
			m_hLib = LoadLibrary(_T("msimg32.dll"));
			if (m_hLib)
			{
				m_pfnGradinetFill = (GradientFillProc*)GetProcAddress(m_hLib, "GradientFill");
			}
		}
		return m_pfnGradinetFill;
	}

private:
	HINSTANCE m_hLib;
	GradientFillProc* m_pfnGradinetFill;
	bool m_bGradientFillTried;
};


// CDesignerViewRolldown

class ATL_NO_VTABLE CDesignerViewRolldown : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewRolldown>,
	public CThemeImpl<CDesignerViewRolldown>,
	public CChildWindowImpl<CDesignerViewRolldown, IDesignerView>,
	public CGradientFillHelper
{
public:
	CDesignerViewRolldown() : m_pLastActive(NULL), m_nControls(0), m_nOffset(0),
		m_nGapX(12), m_nGapY(10), m_nHeaderSizeY(23), m_nBorderSize(1), m_nGripperSize(4),
		m_hThemeBar(NULL), m_hThemeGripper(NULL), m_bDisableStyle(false), m_nWheelDelta(0), m_fScale(1.0f)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	HRESULT Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IViewManager *a_pSubSpec, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, IDocument* a_pDoc, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID);
	void PanelChanged(CPanelCtrl* a_pPanel, bool a_bOpen);

	DECLARE_WND_CLASS_EX(_T("RWDesignerViewPanels"), CS_VREDRAW|CS_HREDRAW, COLOR_WINDOW);

BEGIN_MSG_MAP(CDesignerViewRolldown)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewRolldown>)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChanged)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
END_MSG_MAP()

BEGIN_COM_MAP(CDesignerViewRolldown)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	friend class CPanelCtrl;

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnThemeChanged(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return (LRESULT)GetSysColorBrush(COLOR_WINDOW); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ SetBkColor((HDC)a_wParam, GetSysColor(COLOR_WINDOW)); return (LRESULT)GetSysColorBrush(COLOR_WINDOW); }
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseWheel(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IDesignerView methods
public:
	STDMETHOD(OnIdle)();
	STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges);
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces);
	STDMETHOD(OptimumSize)(SIZE* a_pSize);
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges)
	{
		GetParent().SendMessage(WM_RW_DEACTIVATE, a_bCancelChanges, 0);
		return S_OK;
	}

private:
	void UpdateDrawConstants();
	void RepositionPanels();
	void OnRWGotFocus(CPanelCtrl* a_pPanel, bool a_bForward);
	LRESULT OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam);
	HBRUSH GetPanelBGBrush() { return m_cPanelBGBrush; }

private:
	CPanelCtrl* m_pLastActive;
	CAutoVectorPtr<CPanelCtrl> m_cControls;
	size_t m_nControls;
	LCID m_tLocaleID;
	CComPtr<IViewManager> m_pViewMgr;
	CComPtr<ISharedStateManager> m_pStateMgr;
	CComPtr<IStatusBarObserver> m_pStatusBar;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IConfig> m_pConfig;
	int m_nGapX;
	int m_nGapY;
	int m_nHeaderSizeY;
	int m_nBorderSize;
	int m_nGripperSize;
	CFont m_cHeaderFont;
	COLORREF m_clrHeader;
	COLORREF m_clrHeaderHot;
	HCURSOR m_hSizeNS;
	HCURSOR m_hArrow;
	int m_nTotalHeight;
	int m_nOffset;
	int m_nDrag;
	int m_nWheelDelta;
	bool m_bDisableStyle;
	HANDLE m_hThemeBar;
	HANDLE m_hThemeGripper;
	float m_fScale;
	CBrush m_cPanelBGBrush;
};

