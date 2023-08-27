
#pragma once


class CFileImportDlg : 
	public Win32LangEx::CLangDialogImpl<CFileImportDlg>,
	public CDialogResize<CFileImportDlg>,
	public IStorageFilterWindowCallback
{
public:
	CFileImportDlg(LCID a_tLocaleID, IStorageFilter** a_ppFilter, LPTSTR a_pszTags, size_t a_nTags) :
		Win32LangEx::CLangDialogImpl<CFileImportDlg>(a_tLocaleID),
		m_ppFilter(a_ppFilter), m_pszTags(a_pszTags), m_nTags(a_nTags)
	{
	}
	~CFileImportDlg()
	{
	}

	enum { IDD = IDD_IMPORTFILE };

	BEGIN_MSG_MAP(CFileImportDlg)
		CHAIN_MSG_MAP(CDialogResize<CFileImportDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CFileImportDlg)
		DLGRESIZE_CONTROL(IDC_IF_LOCATOR, DLSZ_SIZE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_IF_TAGSLABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_IF_TAGS, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y|DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y|DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	bool DoModalPreTranslate(MSG const* a_pMsg)
	{
		if (!m_pWnd)
			return false;
		return m_pWnd->PreTranslateMessage(a_pMsg, TRUE) == S_OK || m_pWnd->PreTranslateMessage(a_pMsg, FALSE) == S_OK;
	}

	// IStorageFilterWindowCallback methods
public:
	STDMETHOD(QueryInterface)(const IID &,void **)
	{
		return E_NOTIMPL;
	}
	STDMETHOD_(ULONG, AddRef)()
	{
		return 1;
	}
	STDMETHOD_(ULONG, Release)()
	{
		return 0;
	}
	STDMETHOD(ForwardOK)()
	{
		if (S_OK == m_pWnd->FilterCreate(m_ppFilter))
		{
			GetDlgItemText(IDC_IF_TAGS, m_pszTags, m_nTags);
			m_pszTags[m_nTags-1] = _T('\0');
			EndDialog(IDOK);
			return S_OK;
		}
		return S_FALSE;
	}
	STDMETHOD(ForwardCancel)()
	{
		EndDialog(IDCANCEL);
		return S_OK;
	}
	STDMETHOD(DefaultCommand)(ILocalizedString** UNREF(a_ppName), ILocalizedString** UNREF(a_ppDesc), GUID* UNREF(a_pIconID)) { return E_NOTIMPL; }
	STDMETHOD(DefaultCommandIcon)(ULONG UNREF(a_nSize), HICON* UNREF(a_phIcon)) { return E_NOTIMPL; }

	// handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CWindow wnd = GetDlgItem(IDC_IF_LOCATOR);
		RECT rc;
		wnd.GetWindowRect(&rc);
		ScreenToClient(&rc);
		CComPtr<IStorageFilterFactory> pFct;
		RWCoCreateInstance(pFct, __uuidof(StorageFilterFactoryFileSystem));
		CComPtr<IConfig> pCfg;
		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		if (pMgr)
		{
			CComPtr<IConfig> pCfg2;
			pMgr->Config(CLSID_StorageFactorySQLite, &pCfg2);
			if (pCfg2)
				pCfg2->SubConfigGet(CComBSTR(CFGID_STOREFILE), &pCfg);
		}
		if (pCfg == NULL)
			pFct->ContextConfigGetDefault(&pCfg);
		pFct->WindowCreate(NULL, EFTOpenExisting, m_hWnd, NULL, pCfg, this, NULL, m_tLocaleID, &m_pWnd);
		RWHWND h = NULL;
		m_pWnd->Handle(&h);
		::MoveWindow(h, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
		::SetWindowLong(h, GWL_ID, IDC_IF_LOCATOR);
		wnd.DestroyWindow();
		m_pWnd->Show(TRUE);
		DlgResize_Init();
		CenterWindow(GetParent());
		return 1;  // Let the system set the focus
	}
	LRESULT OnClickedOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		ForwardOK();
		return 0;
	}
	LRESULT OnClickedCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		ForwardCancel();
		return 0;
	}

private:
	CComPtr<IStorageFilterWindow> m_pWnd;
	IStorageFilter** m_ppFilter;
	LPTSTR m_pszTags;
	size_t m_nTags;
};


