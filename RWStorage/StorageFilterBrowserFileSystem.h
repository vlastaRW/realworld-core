// StorageFilterWindowFileSystem.h : interface of the CStorageFilterBrowserFileSystem class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "RWStorage.h"

#include "PIDL.h"
#include <MultiLanguageString.h>
#include <Win32LangEx.h>
#include <ContextHelpDlg.h>

#ifndef WM_GETISHELLBROWSER
    #define WM_GETISHELLBROWSER (WM_USER+7)
#endif

class ATL_NO_VTABLE CStorageFilterBrowserFileSystem :
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangIndirectDialogImpl<CStorageFilterBrowserFileSystem>,
	public CChildWindowImpl<CStorageFilterBrowserFileSystem, IStorageFilterWindow>,
	public CDialogResize<CStorageFilterBrowserFileSystem>,
	public CContextHelpDlg<CStorageFilterBrowserFileSystem>,
	public IShellBrowser,
	public ICommDlgBrowser2
{
public:
	LCID m_tLocaleID;

	CStorageFilterBrowserFileSystem() : m_bDestroying(false)
	{
		m_tViewSettings.ViewMode = FVM_LIST;
		m_tViewSettings.fFlags = FWF_SINGLESEL | FWF_SHOWSELALWAYS | FWF_NOCLIENTEDGE;
		m_hFavFoldersBackground = CreateSolidBrush(((GetSysColor(COLOR_3DFACE)>>1)&0x7f7f7f)+((GetSysColor(COLOR_WINDOW)>>1)&0x7f7f7f));
	}
	~CStorageFilterBrowserFileSystem()
	{
		m_hTBList.Destroy();
	}

	enum
	{
		IDC_FWFILE_FOLDER = 100,
		IDC_FWFILE_TOOLBAR,
		IDC_FWFILE_HORSEP,
		IDC_FWFILE_SHELLVIEW = 1666,
		ID_FWFILE_BACK = 204,
		ID_FWFILE_UP,
		ID_FWFILE_NEWFOLDER,
		ID_FWFILE_VIEWTYPE,
		ID_FWFILE_REMOVEFOLDER,
		ID_VIEWTYPE_BASE = 20000,
	};

	HRESULT Init(LPCOLESTR a_pszInitial, DWORD a_dwFlags, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, HWND a_hWnd, LCID a_tLocaleID);
	virtual void OnFinalMessage(HWND a_hWnd)
	{
		Release();
	}

	BEGIN_DIALOG_EX(0, 0, 200, 115, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_SYSMENU | DS_CONTROL)
		DIALOG_EXSTYLE(WS_EX_CONTEXTHELP | WS_EX_CONTROLPARENT)
		END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_FWFILE_FOLDER, WC_COMBOBOXEX, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 1, 0, 137, 193, 0)
		CONTROL_CONTROL(_T(""), IDC_FWFILE_TOOLBAR, TOOLBARCLASSNAME, 0x994e | WS_VISIBLE, 144, 0, 54, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_FWFILE_HORSEP, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 14, 199, 1, 0)
	END_CONTROLS_MAP()

	BEGIN_COM_MAP(CStorageFilterBrowserFileSystem)
		COM_INTERFACE_ENTRY(IStorageFilterWindow)
		COM_INTERFACE_ENTRY(IOleWindow)
		COM_INTERFACE_ENTRY(IShellBrowser)
		COM_INTERFACE_ENTRY_IID(IID_ICommDlgBrowser, ICommDlgBrowser2)
		COM_INTERFACE_ENTRY_IID(IID_ICommDlgBrowser2, ICommDlgBrowser2)
	END_COM_MAP()

	BEGIN_MSG_MAP(CStorageFilterBrowserFileSystem)
		CHAIN_MSG_MAP(CContextHelpDlg<CStorageFilterBrowserFileSystem>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(ID_FWFILE_BACK, OnFolderBack)
		COMMAND_ID_HANDLER(ID_FWFILE_UP, OnFolderUp)
		NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnViewTypeMenu)
		COMMAND_RANGE_HANDLER(ID_VIEWTYPE_BASE+FVM_FIRST, ID_VIEWTYPE_BASE+FVM_LAST, OnViewTypeChange)
		COMMAND_HANDLER(IDC_FWFILE_FOLDER, CBN_SELCHANGE, OnFolderChange)
		COMMAND_HANDLER(IDC_FWFILE_FOLDER, CBN_DROPDOWN, OnFolderDropDown)
		MESSAGE_HANDLER(WM_GETISHELLBROWSER, OnGetIShellBrowser)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		CHAIN_MSG_MAP(CDialogResize<CStorageFilterBrowserFileSystem>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CStorageFilterBrowserFileSystem)
		DLGRESIZE_CONTROL(IDC_FWFILE_FOLDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_FWFILE_TOOLBAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_FWFILE_HORSEP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_FWFILE_SHELLVIEW, DLSZ_SIZE_Y | DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CStorageFilterBrowserFileSystem)
		CTXHELP_CONTROL_STRING(IDC_FWFILE_FOLDER, L"[0409]This field displays your current folder and allows you to quickly switch to parent folders.[0405]Toto pole zobrazuje jméno aktivní složky a umožňuje rychle přepnout do nadřazených složek.")
		CTXHELP_CONTROL_STRING_NOTIP(IDC_FWFILE_TOOLBAR, L"[0409]Click the first and second button to go to previously visited folder or parent folder. Use the third button to add or remove the current folder from favorites. The last button allows you to switch the way files and folders are represented.[0405]Stiskněte první a druhé tlačítko pro přechod do naposled navštívené nebo nadřazené složky. Použijte třetí tlačítko pro přidání nebo odebrání aktivní složky z panelu oblíbených. Poslední tlačítko určuje způsob zobrazení souborů a složek.")
	END_CTXHELP_MAP()

	bool MessagePumpPretranslateMessage(LPMSG a_pMsg);
	bool Destroying() { return m_bDestroying; }

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (a_bBeforeAccel && !Destroying() && m_hWnd != NULL)
		{
			if (MessagePumpPretranslateMessage(const_cast<LPMSG>(a_pMsg)))
				return S_OK;
			if (a_pMsg->message == WM_KEYDOWN && a_pMsg->wParam == VK_BACK && (a_pMsg->lParam&0x40000000) == 0)
			{
				if (!DispatchMessage(const_cast<LPMSG>(a_pMsg)))
				{
					BOOL b;
					OnFolderUp(0, 0, 0, b);
					return S_OK;
				}
			}
		}
		return S_FALSE;
	}
	STDMETHOD(Show)(BOOL a_bShow)
	{
		if (a_bShow)
		{
			ShowWindow(SW_SHOW);
			if (m_pActiveShellView)
				m_pActiveShellView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
		}
		else
		{
			if (m_pActiveShellView)
				m_pActiveShellView->UIActivate(SVUIA_DEACTIVATE);
			ShowWindow(SW_HIDE);
		}
		return S_OK;
	}

	// IStorageFilterWindow methods
public:
	STDMETHOD(FilterCreate)(IStorageFilter** a_ppFilter);
	STDMETHOD(FiltersCreate)(IEnumUnknowns** a_ppFilters);
	STDMETHOD(DocTypesEnum)(IEnumUnknowns** a_pFormatFilters) { return E_NOTIMPL; }
	STDMETHOD(DocTypeGet)(IDocumentType** a_pFormatFilter) { return E_NOTIMPL; }
	STDMETHOD(DocTypeSet)(IDocumentType* a_pFormatFilter)
	{
		if (m_pActiveDocType != a_pFormatFilter)
		{
			m_pActiveDocType = a_pFormatFilter;
			m_pActiveShellView->Refresh();
		}
		return S_OK;
	}
	STDMETHOD(NavigationCommands)(IEnumUnknowns** a_ppCommands) { return E_NOTIMPL; }
	STDMETHOD(OnIdle)() { return S_FALSE; }

	// IOleWindow methods
public:
	STDMETHOD(GetWindow)(HWND* a_phWnd);
	STDMETHOD(ContextSensitiveHelp)(BOOL a_fEnterMode);

	// IShellBrowser methods
public:
	STDMETHOD(InsertMenusSB)(HMENU a_hMenuShared, LPOLEMENUGROUPWIDTHS a_pMenuWidths);
	STDMETHOD(SetMenuSB)(HMENU a_hMenuShared, HOLEMENU a_hOleMenuReserved, HWND a_hWndActiveObject);
	STDMETHOD(RemoveMenusSB)(HMENU a_hMenuShared);
	STDMETHOD(SetStatusTextSB)(LPCOLESTR a_pszStatusText);
	STDMETHOD(EnableModelessSB)(BOOL a_fEnable);
	STDMETHOD(BrowseObject)(LPCITEMIDLIST a_pIDL, UINT a_wFlags);
	STDMETHOD(GetViewStateStream)(DWORD a_dwGrfMode, LPSTREAM *a_ppStrm);
	STDMETHOD(OnViewWindowActive)(IShellView* a_pShellView);
	STDMETHOD(SetToolbarItems)(LPTBBUTTON a_pButtons, UINT a_nButtons, UINT a_uFlags);
	STDMETHOD(TranslateAcceleratorSB)(LPMSG a_pMsg, WORD a_wID);
	STDMETHOD(QueryActiveShellView)(IShellView** a_ppShellView);
	STDMETHOD(GetControlWindow)(UINT a_uID, HWND* a_phWnd);
	STDMETHOD(SendControlMsg)(UINT a_uID, UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, LRESULT* a_pResult);

	// ICommDlgBrowser methods
public:
	STDMETHOD(OnDefaultCommand)(IShellView* a_pShellView);
	STDMETHOD(OnStateChange)(IShellView* a_pShellView, ULONG a_uChange);
	STDMETHOD(IncludeObject)(IShellView* a_pShellView, LPCITEMIDLIST a_pIDL);

	// ICommDlgBrowser2 methods
public:
	STDMETHOD(GetDefaultMenuText)(IShellView* a_pShellView, WCHAR *a_pszText, int a_cchMax);
	STDMETHOD(GetViewFlags)(DWORD *a_pdwFlags);
	STDMETHOD(Notify)(IShellView* a_pShellView, DWORD a_dwNotifyType);

	// handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnGetIShellBrowser(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnFolderBack(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFolderUp(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnViewTypeChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnViewTypeMenu(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnFolderChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFolderDropDown(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return (LRESULT)GetSysColorBrush((m_dwFlags&EFTBackgroundMask) == EFTBackground3D ? COLOR_3DFACE : COLOR_WINDOW); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ SetBkColor((HDC)a_wParam, GetSysColor((m_dwFlags&EFTBackgroundMask) == EFTBackground3D ? COLOR_3DFACE : COLOR_WINDOW)); return (LRESULT)GetSysColorBrush((m_dwFlags&EFTBackgroundMask) == EFTBackground3D ? COLOR_3DFACE : COLOR_WINDOW); }

private:
	typedef stack<CPIDL> CFolderHistory;
	typedef std::vector<CComPtr<IDocumentType> > CDocTypes;
	struct localized_doctype_compare
	{
		localized_doctype_compare(LCID a_tLocaleID) : m_tLocaleID(a_tLocaleID) {}
		bool operator ()(CComPtr<IDocumentType>& a_1, CComPtr<IDocumentType>& a_2) const
		{
			if (a_1 == NULL)
				return false;
			CComPtr<ILocalizedString> pName1;
			a_1->FilterNameGet(&pName1);
			if (pName1 == NULL)
				return false;
			CComBSTR bstrName1;
			pName1->GetLocalized(m_tLocaleID, &bstrName1);
			if (bstrName1 == NULL)
				return false;

			if (a_2 == NULL)
				return true;
			CComPtr<ILocalizedString> pName2;
			a_2->FilterNameGet(&pName2);
			if (pName2 == NULL)
				return true;
			CComBSTR bstrName2;
			pName2->GetLocalized(m_tLocaleID, &bstrName2);
			if (bstrName2 == NULL)
				return true;

			return wcscmp(bstrName1, bstrName2) < 0;
		}

	private:
		LCID m_tLocaleID;
	};

private:
	HRESULT ShowFolder(IShellFolder* a_pNewFolder);
	RECT GetShellWndRectangle() const;
	void LookInClear();
	void LookInInit(const CPIDL& a_cActual);
	int LookInInsert(const CPIDL& a_cRoot, const CPIDL& a_cActual, int a_iIndent, int a_iItem);
	void GetFolder(tstring& a_strOut) const;
	void SetConfigValue(BSTR a_bstrID, TConfigValue const& a_tValue) const
	{
		if (m_pContextConfig != NULL)
			m_pContextConfig->ItemValuesSet(1, &a_bstrID, &a_tValue);
	}
	void GetConfigValue(BSTR a_bstrID, TConfigValue* a_pValue) const
	{
		if (m_pContextConfig != NULL)
			m_pContextConfig->ItemValueGet(a_bstrID, a_pValue);
	}
	void NotifyListenerDocumentChange() const
	{
		if (m_pActiveDocType != NULL && m_pListener != NULL)
			m_pListener->DocumentChanged(m_pActiveDocType);
	}
	void AddFavFolder(CPIDL const& a_cPIDL);

private:
	CComPtr<IConfig> m_pContextConfig;
	CComPtr<IStorageFilterWindowCallback> m_pCallback;
	CComPtr<IStorageFilterWindowListener> m_pListener;

	TCHAR m_szName[MAX_PATH];
	CComPtr<IShellView> m_pActiveShellView;
	CComPtr<IShellFolder> m_pLastFolder;
	HWND m_hActiveShellWnd;
	DWORD m_dwFlags;

	CComboBoxEx m_wndFolder;
	CToolBarCtrl m_wndToolBar;
	CImageList m_hTBList;
	CDocTypes m_cDocTypes;
	CComPtr<IDocumentType> m_pActiveDocType;
	std::vector<CPIDL> m_cFavFolders;
	std::wstring m_strFavFolders;
	HBRUSH m_hFavFoldersBackground;
	CWindow m_wndSeparator;

	CComPtr<IMalloc> m_pShellAlloc;
	CComPtr<IShellFolder> m_pDesktopFolder;
	CPIDL m_cFolder;
	CFolderHistory m_cFolderHistory;
	FOLDERSETTINGS m_tViewSettings;

	bool m_bDestroying;
	//HWND m_hParent;
	int m_nDefDocType;

	// special folders
	CPIDL m_cDesktopDir;
	CPIDL m_cDesktop;
};
