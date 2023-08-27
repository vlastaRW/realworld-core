// FullConfigWnd.h : Declaration of the CFullConfigWnd

#pragma once
#include "resource.h"       // main symbols
#include "RWConfig.h"

#include <ObserverImpl.h>
#include <Win32LangEx.h>
#include "ConfigItemCtl.h"
#include "SyncedConfigData.h"
#include "SyncedData.h"
#include "TableManager.h"

// CFullConfigWnd

class ATL_NO_VTABLE CFullConfigWnd : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CFullConfigWnd, &CLSID_FullConfigWnd>,
	public Win32LangEx::CLangDialogImpl<CFullConfigWnd>,
	public CChildWindowImpl<CFullConfigWnd, IConfigWnd>,
	public CObserverImpl<CFullConfigWnd, IConfigObserver, IUnknown*>,
	public CSyncedDataListener<TScrollInfo>,
	public CThemeImpl<CFullConfigWnd>,
	private CSyncedData<TScrollInfo>
{
public:
	CFullConfigWnd() : m_cClient(this), IDD(IDD_FULLCONFIG_BORDER), m_eMode(ECPMFull)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
		ListenerAdd(this);
	}
	~CFullConfigWnd()
	{
		ListenerRemove(this);
		if (m_pConfig != NULL)
		{
			m_pConfig->ObserverDel(ObserverGet(), 0);
		}
	}

	UINT IDD;

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CFullConfigWnd)
	COM_INTERFACE_ENTRY(IChildWindow)
	COM_INTERFACE_ENTRY(IConfigWnd)
END_COM_MAP()

BEGIN_MSG_MAP(CFullConfigWnd)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
	MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	MESSAGE_HANDLER(WM_HELP, OnHelp)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	NOTIFY_CODE_HANDLER(HDN_ENDTRACK, OnHeaderTrack)
	NOTIFY_CODE_HANDLER(HDN_ITEMCHANGING , OnHeaderTrack)
	NOTIFY_CODE_HANDLER(HDN_TRACK, OnHeaderTrack)
	CHAIN_MSG_MAP(CThemeImpl<CFullConfigWnd>)
END_MSG_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// CSyncedDataListener<TScrollInfo>::Notify methods
	void Notify(const TScrollInfo& a_tScrollInfo);
	// internal IObserver wrapped method
	void OwnerNotify(TCookie a_tCookie, IUnknown* a_pIDs);

	// message handlers
public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClientResize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHeaderTrack(int wParam, LPNMHDR pNMHdr, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void OnFinalMessage(HWND a_hWnd)
	{
		Release();
	}

	// IConfigWnd methods
public:
	STDMETHOD(ConfigSet)(IConfig* a_pConfig, EConfigPanelMode a_eMode);
	STDMETHOD(TopWindowSet)(BOOL a_bIsTopWindow, DWORD a_clrBackground);
	STDMETHOD(Create)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, EConfigWindowBorderMode a_eBorderMode);
	STDMETHOD(OptimumSize)(SIZE *a_pSize) { return E_NOTIMPL; }
	STDMETHOD(ChangeLanguage)(LCID UNREF(a_tLocaleID)) { return E_NOTIMPL; }

private:
	CComPtr<IConfig> m_pConfig;
	CTableManager m_cClient;
	EConfigPanelMode m_eMode;
};

OBJECT_ENTRY_AUTO(__uuidof(FullConfigWnd), CFullConfigWnd)
