// StorageFilterWindowFileSystem.h : interface of the CStorageFilterWindowFileSystem class
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

class ATL_NO_VTABLE CStorageFilterWindowFileSystem :
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangIndirectDialogImpl<CStorageFilterWindowFileSystem>,
	public CChildWindowImpl<CStorageFilterWindowFileSystem, IStorageFilterWindow>,
	public CDialogResize<CStorageFilterWindowFileSystem>,
	public CContextHelpDlg<CStorageFilterWindowFileSystem>,
	public IShellBrowser,
	public ICommDlgBrowser2
{
public:
	LCID m_tLocaleID;

	CStorageFilterWindowFileSystem() : m_bDestroying(false)
	{
		m_tViewSettings.ViewMode = FVM_LIST;
		m_tViewSettings.fFlags = FWF_SINGLESEL | FWF_SHOWSELALWAYS;
		m_hFavFoldersBackground = CreateSolidBrush(((GetSysColor(COLOR_3DFACE)>>1)&0x7f7f7f)+((GetSysColor(COLOR_WINDOW)>>1)&0x7f7f7f));
	}
	~CStorageFilterWindowFileSystem()
	{
		m_hTBList.Destroy();
		m_hFilterImages.Destroy();
		::DeleteObject(m_hFavFoldersBackground);
	}

	enum
	{
		IDC_FWFILE_FOLDER_TEXT = 100,
		IDC_FWFILE_FOLDER,
		IDC_FWFILE_TOOLBAR,
		IDC_FWFILE_NAME_TEXT,
		IDC_FWFILE_NAME,
		IDC_FWFILE_FILTER_TEXT,
		IDC_FWFILE_FILTER,
		IDC_FWFILE_FOLDERS,
		IDC_FWFILE_SHELLVIEW = 1666,
		IDC_FWFILE_FAVFOLDER1 = 4535,
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

	BEGIN_DIALOG_EX(0, 0, 250, 125, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_SYSMENU | DS_CONTROL)
		DIALOG_EXSTYLE(WS_EX_CONTEXTHELP | WS_EX_CONTROLPARENT)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Look &in:[0405]&Složka:"), IDC_FWFILE_FOLDER_TEXT, 7, 9, 49, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_FWFILE_FOLDER, WC_COMBOBOXEX, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 58, 7, 108, 193, 0)
		CONTROL_CONTROL(_T(""), IDC_FWFILE_TOOLBAR, TOOLBARCLASSNAME, 0x994e | WS_VISIBLE, 173, 7, 70, 14, 0)
		CONTROL_LTEXT(_T("[0409]File &name:[0405]&Jméno:"), IDC_FWFILE_NAME_TEXT, 58, 90, 50, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_FWFILE_NAME, WC_COMBOBOXEX, CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL | WS_GROUP | WS_TABSTOP | WS_VISIBLE, 109, 88, 134, 150, 0)
		CONTROL_LTEXT(_T("[0409]Files of &type:[0405]&Typ souboru:"), IDC_FWFILE_FILTER_TEXT, 58, 108, 50, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_FWFILE_FILTER, WC_COMBOBOXEX, CBS_DROPDOWNLIST | WS_VSCROLL | WS_GROUP | WS_TABSTOP | WS_VISIBLE, 109, 106, 134, 200, 0)
		CONTROL_CONTROL(_T(""), IDC_FWFILE_FOLDERS, TOOLBARCLASSNAME, 0xaa4d | WS_VISIBLE, 7, 25, 47, 93, 0)
	END_CONTROLS_MAP()

	BEGIN_COM_MAP(CStorageFilterWindowFileSystem)
		COM_INTERFACE_ENTRY(IStorageFilterWindow)
		COM_INTERFACE_ENTRY(IOleWindow)
		COM_INTERFACE_ENTRY(IShellBrowser)
		COM_INTERFACE_ENTRY_IID(IID_ICommDlgBrowser, ICommDlgBrowser2)
		COM_INTERFACE_ENTRY_IID(IID_ICommDlgBrowser2, ICommDlgBrowser2)
	END_COM_MAP()

	BEGIN_MSG_MAP(CStorageFilterWindowFileSystem)
		CHAIN_MSG_MAP(CContextHelpDlg<CStorageFilterWindowFileSystem>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(ID_FWFILE_BACK, OnFolderBack)
		COMMAND_ID_HANDLER(ID_FWFILE_NEWFOLDER, OnFolderAddOrRemove)
		COMMAND_ID_HANDLER(ID_FWFILE_UP, OnFolderUp)
		NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnViewTypeMenu)
		NOTIFY_HANDLER(IDC_FWFILE_FOLDERS, NM_CUSTOMDRAW, OnFoldersCustomDraw)
		COMMAND_RANGE_HANDLER(ID_VIEWTYPE_BASE+FVM_FIRST, ID_VIEWTYPE_BASE+FVM_LAST, OnViewTypeChange)
		COMMAND_RANGE_HANDLER(IDC_FWFILE_FAVFOLDER1, IDC_FWFILE_FAVFOLDER1+25, OnFavoriteFolder)
		COMMAND_HANDLER(IDC_FWFILE_FILTER, CBN_SELCHANGE, OnFilterChange)
		COMMAND_HANDLER(IDC_FWFILE_FOLDER, CBN_SELCHANGE, OnFolderChange)
		COMMAND_HANDLER(IDC_FWFILE_FOLDER, CBN_DROPDOWN, OnFolderDropDown)
		COMMAND_HANDLER(IDC_FWFILE_NAME, CBN_EDITCHANGE, OnNameChange)
		MESSAGE_HANDLER(WM_GETISHELLBROWSER, OnGetIShellBrowser)
		CHAIN_MSG_MAP(CDialogResize<CStorageFilterWindowFileSystem>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CStorageFilterWindowFileSystem)
		DLGRESIZE_CONTROL(IDC_FWFILE_FOLDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_FWFILE_TOOLBAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_FWFILE_FOLDERS, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_FWFILE_NAME_TEXT, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_FWFILE_NAME, DLSZ_MOVE_Y | DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_FWFILE_FILTER_TEXT, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_FWFILE_FILTER, DLSZ_MOVE_Y | DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_FWFILE_SHELLVIEW, DLSZ_SIZE_Y | DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CStorageFilterWindowFileSystem)
		CTXHELP_CONTROL_STRING(IDC_FWFILE_FOLDER, L"[0409]This field displays your current folder and allows you to quickly switch to parent folders.[0405]Toto pole zobrazuje jméno aktivní složky a umožňuje rychle přepnout do nadřazených složek.")
		CTXHELP_CONTROL_STRING_NOTIP(IDC_FWFILE_TOOLBAR, L"[0409]Click the first and second button to go to previously visited folder or parent folder. Use the third button to add or remove the current folder from favorites. The last button allows you to switch the way files and folders are represented.[0405]Stiskněte první a druhé tlačítko pro přechod do naposled navštívené nebo nadřazené složky. Použijte třetí tlačítko pro přidání nebo odebrání aktivní složky z panelu oblíbených. Poslední tlačítko určuje způsob zobrazení souborů a složek.")
		CTXHELP_CONTROL_STRING(IDC_FWFILE_FOLDERS, L"[0409]Click on a button to quickly switch to the respective folder. You may add the current folder to this list using the buttons in the right upper part of this window.[0405]Stiskněte tlačítko pro přechod do dané oblíbené složky. Oblíbené složky můžete přidávat pomocí tlačítka v pravém horním rohu okna.")
		CTXHELP_CONTROL_STRING_NOTIP(IDC_FWFILE_NAME, L"[0409]The name or path of the selected file. If the path is not absolute, the folder selected above is considered the base of the relative path.[0405]Jméno nebo cesta ke zvolenému souboru. Není-li cesta absolutní, je složka vybraná nahoře považována za základ relativního umístění.")
		CTXHELP_CONTROL_STRING(IDC_FWFILE_FILTER, L"[0409]This box allows you to display only files of selected types. When storing files, this box may influence the file format of the saved file.[0405]Toto pole umožňuje zvolit typy souborů, které budou zobrazovány. Pokud se jedná o ukládání dat, může toto pole ovlivnit souborový formát ukládaného souboru.")
	END_CTXHELP_MAP()

	LPARAM GetFilter(tstring& a_strOut, bool a_bNoBeep = false);
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
			if (m_pListener)
				m_pListener->DocumentChanged(m_pActiveDocType);
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
	STDMETHOD(DocTypeSet)(IDocumentType* a_pFormatFilter) { return E_NOTIMPL; }
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

public:
	// handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnGetIShellBrowser(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnFolderBack(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFolderAddOrRemove(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFolderUp(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnViewTypeChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFoldersCustomDraw(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnViewTypeMenu(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnFilterChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFolderChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFolderDropDown(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnNameChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFavoriteFolder(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);

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
	void UpdateAutoFolders();

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
	CComboBoxEx m_wndName;
	CComPtr<IUnknown> m_pAutoComplete;
	CComboBoxEx m_wndFilter;
	CImageList m_hFilterImages;
	CToolBarCtrl m_wndToolBar;
	CImageList m_hTBList;
	CDocTypes m_cDocTypes;
	CComPtr<IDocumentType> m_pActiveDocType;
	CToolBarCtrl m_wndFavFolders;
	std::vector<std::wstring> m_cRecentFiles;
	std::vector<CPIDL> m_cFavFolders;
	std::vector<CPIDL> m_cAutoFolders;
	std::wstring m_strFavFolders;
	HBRUSH m_hFavFoldersBackground;

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
