// AutoConfigWnd.h : Declaration of the CAutoConfigWnd

#pragma once
#include "resource.h"       // main symbols
#include "RWConfig.h"
#include <ObserverImpl.h>
#include <Win32LangEx.h>


// CAutoConfigWnd

class ATL_NO_VTABLE CAutoConfigWnd : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAutoConfigWnd, &CLSID_AutoConfigWnd>,
	public Win32LangEx::CLangIndirectDialogImpl<CAutoConfigWnd>,
	public CThemeImpl<CAutoConfigWnd>,
	public CChildWindowImpl<CAutoConfigWnd, IConfigWnd>,
	public CObserverImpl<CAutoConfigWnd, IConfigObserver, IUnknown*>
{
public:
	CAutoConfigWnd() : m_tLastConfigUID(GUID_NULL), m_bUpdatePosted(false),
		m_bTopLevel(false), m_hBGBrush(NULL), m_clrBG(0), m_eBorderMode(ECWBMNothing), m_eMode(ECPMFull)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
		m_ptOrigin.x = m_ptOrigin.y = 0;
		m_szClient.cx = 600;
		m_szClient.cy = 400;
	}
	~CAutoConfigWnd()
	{
		if (m_hBGBrush)
			DeleteObject(m_hBGBrush);
	}

	BEGIN_DIALOG_EX(0, 0, 10, 10, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(WS_EX_CONTROLPARENT)
		if (m_eBorderMode == ECWBMMarginAndOutline)
			dwExStyle |= WS_EX_CLIENTEDGE;
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
	END_CONTROLS_MAP()

DECLARE_NO_REGISTRY()

	static HRESULT WINAPI QICustomInterface(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
	{
		CAutoConfigWnd* const p = reinterpret_cast<CAutoConfigWnd*>(pv);
		if (p->m_pChildWindow)
			return p->m_pChildWindow->QueryInterface(riid, ppv);
		return E_NOINTERFACE;
	}

BEGIN_COM_MAP(CAutoConfigWnd)
	COM_INTERFACE_ENTRY(IChildWindow)
	COM_INTERFACE_ENTRY(IConfigWnd)
	COM_INTERFACE_ENTRY_FUNC_BLIND(0, QICustomInterface)
END_COM_MAP()

	enum { WM_UPDATECLIENT = WM_APP+274 };

BEGIN_MSG_MAP(CAutoConfigWnd)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	COMMAND_HANDLER(IDC_CONFIGURE_BUTTON, STN_CLICKED, OnSwitchToTableClicked)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	CHAIN_MSG_MAP(CThemeImpl<CAutoConfigWnd>)
	if (m_bTopLevel)
	{
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic2)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
	}
	else
	{
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnColor2Parent)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnColor2Parent)
	MESSAGE_HANDLER(WM_CTLCOLORBTN, OnColor2Parent)
	}
	MESSAGE_HANDLER(WM_UPDATECLIENT, OnUpdateClient)
	MESSAGE_HANDLER(WM_RW_GETCFGDOC, OnGetCfgDoc)
END_MSG_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pConfig != NULL)
		{
			m_pConfig->ObserverDel(ObserverGet(), 0);
		}
	}

	// message handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSwitchToTableClicked(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		a_bHandled = FALSE;
		HWND h = SetFocus(); // ...? force the active control to lose focus
		::SetFocus(h);
		return 0;
	}
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	virtual void OnFinalMessage(HWND a_hWnd)
	{
		Release();
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return (LRESULT)m_hBGBrush; }
	LRESULT OnCtlColorStatic2(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ SetBkColor((HDC)a_wParam, m_clrBG); return (LRESULT)m_hBGBrush; }
	LRESULT OnColor2Parent(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
	LRESULT OnUpdateClient(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ UpdateConfigGUI2(m_szClient.cx, m_szClient.cy); return 0; }
	LRESULT OnGetCfgDoc(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);
	STDMETHOD(SendMessage)(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
	{
		if (a_uMsg == WM_RW_CFGSPLIT)
		{
			if (m_pCustGUI && m_pChildWindow == NULL)
			{
				RECT rcClient;
				GetClientRect(&rcClient);
				UpdateConfigGUI2(rcClient.right, rcClient.bottom);
			}
			if (m_pChildWindow)
				return m_pChildWindow->SendMessage(a_uMsg, a_wParam, a_lParam);
			return S_OK;
		}
		return CChildWindowImpl<CAutoConfigWnd, IConfigWnd>::SendMessage(a_uMsg, a_wParam, a_lParam);
	}

	// IConfigWnd methods
public:
	STDMETHOD(ConfigSet)(IConfig* a_pConfig, EConfigPanelMode a_eMode);
	STDMETHOD(TopWindowSet)(BOOL a_bIsTopWindow, DWORD a_clrBackground);
	STDMETHOD(Create)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, EConfigWindowBorderMode a_eBorderMode);
	STDMETHOD(OptimumSize)(SIZE *a_pSize);
	STDMETHOD(ChangeLanguage)(LCID a_tLocaleID);

	// internal IObserver wrapped method
public:
	void OwnerNotify(TCookie a_tCookie, IUnknown* a_pChangedParam);

private:
	enum {
		IDC_CONFIGURE_BUTTON = 3000,
		IDC_CUSTOM_GUI_WINDOW,
		IDC_FULL_GUI_WINDOW
	};

	void UpdateConfigGUI(int a_nSizeX, int a_nSizeY)
	{
		m_szClient.cx = a_nSizeX;
		m_szClient.cy = a_nSizeY;
		if (!m_bUpdatePosted)
		{
			PostMessage(WM_UPDATECLIENT);
			m_bUpdatePosted = true;
		}
	}
	void UpdateConfigGUI2(int a_nSizeX, int a_nSizeY);

private:
	CComPtr<IConfig> m_pConfig;
	CComPtr<IConfigCustomGUI> m_pCustGUI;
	CComPtr<IChildWindow> m_pChildWindow;
	CStatic m_wndCannotFit;
	LCID m_tLocaleID;
	GUID m_tLastConfigUID;
	EConfigWindowBorderMode m_eBorderMode;
	POINT m_ptOrigin;
	SIZE m_szClient;
	bool m_bUpdatePosted;
	bool m_bTopLevel;
	HBRUSH m_hBGBrush;
	COLORREF m_clrBG;
	EConfigPanelMode m_eMode;
};

OBJECT_ENTRY_AUTO(__uuidof(AutoConfigWnd), CAutoConfigWnd)
