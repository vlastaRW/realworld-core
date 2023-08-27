// OpenFilterDlg.h : interface of the COpenFilterDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "RWStorage.h"
#include <PlugInCache.h>
#include <Win32LangEx.h>
#include <atlgdix.h>
#include <CustomTabCtrl.h>
#include <DotNetTabCtrl.h>
#include <ObserverImpl.h>


class COpenFilterDlg :
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangIndirectDialogImpl<COpenFilterDlg>,
	public CChildWindowImpl<COpenFilterDlg, IStorageBrowserWindow>,
	public CObserverImpl<COpenFilterDlg, IConfigObserver, IUnknown*>
{
public:
	COpenFilterDlg() : m_nActivePage(-1), m_nTabHeight(0), m_nConfigHeight(0)
	{
	}
	~COpenFilterDlg()
	{
		m_cImageList.Destroy();
	}
	void Create(HWND a_hParent, BSTR a_bstrInitial, DWORD a_dwFlags, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID);
	virtual void OnFinalMessage(HWND a_hWnd);


	BEGIN_DIALOG_EX(0, 0, 278, 166, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_CONTROL | WS_VISIBLE)
		DIALOG_EXSTYLE(WS_EX_CONTEXTHELP | WS_EX_CONTROLPARENT)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
	END_CONTROLS_MAP()

	BEGIN_COM_MAP(COpenFilterDlg)
		COM_INTERFACE_ENTRY(IStorageFilterWindow)
		COM_INTERFACE_ENTRY(IStorageBrowserWindow)
	END_COM_MAP()

	BEGIN_MSG_MAP(COpenFilterDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
		NOTIFY_HANDLER(IDC_OF_SWITCHSOURCE, CTCN_SELCHANGE, OnTcnSelchangeSwitchSource)
	END_MSG_MAP()


	void OwnerNotify(TCookie, IUnknown*)
	{
		if (m_hWnd == NULL)
			return;
		SIZE tSize = {0, 0};
		m_pMiniCfg->OptimumSize(&tSize);
		if (m_nConfigHeight != tSize.cy)
		{
			m_nConfigHeight = tSize.cy;
			RECT rc;
			GetClientRect(&rc);
			BOOL b;
			OnSize(0, 0, MAKELPARAM(rc.right, rc.bottom), b);
		}
	}

	// IChildWindow methods (partial)
public:
	STDMETHOD(Destroy)();
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IStorageFilterWindow methods
public:
	STDMETHOD(FilterCreate)(IStorageFilter** a_ppFilter);
	STDMETHOD(FiltersCreate)(IEnumUnknowns** a_ppFilters);
	STDMETHOD(DocTypesEnum)(IEnumUnknowns** a_pFormatFilters);
	STDMETHOD(DocTypeGet)(IDocumentType** a_pFormatFilter);
	STDMETHOD(DocTypeSet)(IDocumentType* a_pFormatFilter);
	STDMETHOD(NavigationCommands)(IEnumUnknowns** a_ppCommands);
	STDMETHOD(OnIdle)();

	// IStorageBrowserWindow methods
public:
	STDMETHOD(ActiveStorageGet)(CLSID* a_pclsidPage);
	STDMETHOD(ActiveStorageSet)(REFCLSID a_clsidPage);
	STDMETHOD(StoragesEnum)(IEnumGUIDs** a_ppStorageIDs);
	STDMETHOD(StoragesName)(REFCLSID a_clsidPage, ILocalizedString** a_ppName);
	STDMETHOD(StoragesIcon)(REFCLSID a_clsidPage, ULONG a_nSize, HICON* a_phIcon);

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTcnSelchangeSwitchSource(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

private:
	enum { IDC_OF_SWITCHSOURCE = 201, IDC_OF_TABPAGE0 = 210 };

	struct STabContext : public CCustomTabItem
	{
		STabContext(GUID a_tID, IStorageFilterFactory* a_pFactory) : tID(a_tID), pFactory(a_pFactory)
		{
		}
		STabContext(STabContext const& a_rhs) : tID(a_rhs.tID), pWindow(a_rhs.pWindow)
		{
			CCustomTabItem::operator =(a_rhs);
		}
		GUID tID;
		CComPtr<IStorageFilterWindow> pWindow;
		CComPtr<IStorageFilterFactory> pFactory;
	};

	template <class TItem = CCustomTabItem>
	class CContrastTabCtrl : public CDotNetTabCtrlImpl<CContrastTabCtrl<TItem>, TItem>
	{
	public:
		DECLARE_WND_CLASS_EX(_T("WTL_ContrastDotNetTabCtrl"), CS_DBLCLKS, COLOR_WINDOW)

		void InitializeDrawStruct(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
		{
			CDotNetTabCtrlImpl<CContrastTabCtrl, TItem>::InitializeDrawStruct(lpNMCustomDraw);
			lpNMCustomDraw->clrBtnHighlight = ::GetSysColor(COLOR_3DSHADOW);
		}
	};
	friend struct initdialog;

private:
	CComPtr<IEnumUnknowns> m_pFormatFilters;
	CComPtr<IConfig> m_pContextConfig;
	CComPtr<IConfig> m_pUserConfig;
	CContrastTabCtrl<STabContext> m_wndTab;
	CImageList m_cImageList;
	int m_nActivePage;
	CComPtr<IConfigWnd> m_pMiniCfg;
	LONG m_nTabHeight;
	LONG m_nConfigHeight;
	RECT m_rcGaps;

	// initial params
	CComPtr<IStorageFilterWindowCallback> m_pCallback;
	CComPtr<IStorageFilterWindowListener> m_pListener;
	CComBSTR m_bstrInitial;
	DWORD m_dwFlags;
};
