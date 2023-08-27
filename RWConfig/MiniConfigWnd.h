// MiniConfigWnd.h : Declaration of the CMiniConfigWnd

#pragma once
#include "resource.h"       // main symbols
#include "RWConfig.h"

#include "ConfigItemCtl.h"
#include <ObserverImpl.h>
#include <Win32LangEx.h>


// CMiniConfigWnd

class ATL_NO_VTABLE CMiniConfigWnd : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMiniConfigWnd, &CLSID_MiniConfigWnd>,
	public Win32LangEx::CLangDialogImpl<CMiniConfigWnd>,
	public CChildWindowImpl<CMiniConfigWnd, IConfigWnd>,
	public CObserverImpl<CMiniConfigWnd, IConfigObserver, IUnknown*>,
	public CConfigItemParent
{
public:
	CMiniConfigWnd() : m_pValueCtl(NULL)
	{
	}
	enum { IDD = IDD_MINICONFIG };

	// CConfigItemParent methods
public:
	IConfig* GetConfig() const
	{
		return m_pConfig;
	}
	HWND GetHWND() const
	{
		return m_hWnd;
	}
	HFONT GetFont() const
	{
		return Win32LangEx::CLangDialogImpl<CMiniConfigWnd>::GetFont();
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CMiniConfigWnd)
	COM_INTERFACE_ENTRY(IChildWindow)
	COM_INTERFACE_ENTRY(IConfigWnd)
END_COM_MAP()

BEGIN_MSG_MAP(CMiniConfigWnd)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	COMMAND_RANGE_HANDLER(IDC_MCF_ITEMID+1, IDC_MCF_ITEMID+16, ReflectEvent)
	MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	COMMAND_HANDLER(IDC_MCF_ITEMID, CBN_SELCHANGE, OnItemIDSelchange)
	MESSAGE_HANDLER(WM_HELP, OnHelp)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

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

	void OwnerNotify(TCookie a_tCookie, IUnknown* a_pIDs);

	// IConfigWnd methods
public:
	STDMETHOD(ConfigSet)(IConfig* a_pConfig, EConfigPanelMode a_eMode);
	STDMETHOD(TopWindowSet)(BOOL a_bIsTopWindow, DWORD a_clrBackground);
	STDMETHOD(Create)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, EConfigWindowBorderMode a_eBorderMode);
	STDMETHOD(OptimumSize)(SIZE *a_pSize) { return E_NOTIMPL; }
	STDMETHOD(ChangeLanguage)(LCID UNREF(a_tLocaleID)) { return E_NOTIMPL; }

	// message handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnItemIDSelchange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_Handled);
	LRESULT OnHelp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	virtual void OnFinalMessage(HWND a_hWnd);

	LRESULT ReflectEvent(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	bool RefreshItemIDs();
	void RefreshValue();
	void GetControlsWidths(LONG a_nXSize, LONG* a_pComboLeft, LONG* a_pComboRight, LONG* a_pControlLeft, LONG* a_pControlRight) const;

private:
	CComPtr<IConfig> m_pConfig;
	CComboBoxEx m_wndItemIDs;
	CComBSTR m_bstrSelected;
	CComPtr<IEnumStrings> m_pItemIDs;
	CConfigItemCtl* m_pValueCtl;
	int m_nXGap;
	int m_nXMaxComboSize;
};

OBJECT_ENTRY_AUTO(__uuidof(MiniConfigWnd), CMiniConfigWnd)
