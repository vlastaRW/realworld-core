// StorageWindowSQLite.h : Declaration of the CStorageWindowSQLite

#pragma once
#include "resource.h"       // main symbols
#include "RWStorageSQLite.h"
#include <Win32LangEx.h>
#include <RWThumbnails.h>
#include <ContextMenuWithIcons.h>
#include <ContextHelpDlg.h>



// CStorageWindowSQLite

class ATL_NO_VTABLE CStorageWindowSQLite :
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangDialogImpl<CStorageWindowSQLite>,
	public CDialogResize<CStorageWindowSQLite>,
	public CContextHelpDlg<CStorageWindowSQLite>,
	public CChildWindowImpl<CStorageWindowSQLite, IStorageFilterWindow>,
	public CContextMenuWithIcons<CStorageWindowSQLite>,
	public IThumbnailCallback,
	public IDropTarget
{
public:
	CStorageWindowSQLite() :
		m_bDestroyed(false), m_cDb(NULL), m_nActiveTags(0), m_iActiveItem(-1), m_nDocTypes(0),
		m_nLabelHeight(10), m_bInitialValid(false), m_dwLastDropEffect(DROPEFFECT_NONE),
		m_bDragHelperCreated(false)
	{
	}
	~CStorageWindowSQLite()
	{
		m_hFilterImages.Destroy();
		m_cToolBar.Destroy();
		m_cThumbnails.Destroy();
		if (!m_strLastDraggedFile.empty())
			DeleteFile(m_strLastDraggedFile.c_str());
	}

	enum
	{
		IDD = IDD_FILTERWINDOWSQLITE,
		WM_THUMBNAILREADY = WM_APP+825,
		ID_DELETEFILE = 524,
		ID_RENAMEFILE,
		ID_MODIFYTAGS,
		ID_PICKDATABASE,
		ID_IMPORTDATABASE,
		ID_EXPORTDATABASE,
		ID_IMPORTFILE,
		ID_EXPORTFILE,
		ID_ONLINEHELP,
		ID_MANAGEDATABASE,
		ID_DEFAULTDATABASE,
		ID_ONLINEDATABASE,
		ID_FOLLOWLINK,
		ID_SORTBASE = 600,
		ID_SHOWTHUMBNAILS = 616,
		ID_GROUPITEMS,
		GRPID_NOTAGS = 1,
		GRPID_OTHER = 2,
		GRPID_RECENT = 3,
		GRPID_TAGSBASE = 5,
	};

	HRESULT Init(LPCOLESTR a_pszInitial, DWORD a_dwFlags, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, HWND a_hWnd, LCID a_tLocaleID);
	virtual void OnFinalMessage(HWND a_hWnd)
	{
		Release();
	}

BEGIN_COM_MAP(CStorageWindowSQLite)
	COM_INTERFACE_ENTRY(IStorageFilterWindow)
	COM_INTERFACE_ENTRY(IThumbnailCallback)
	COM_INTERFACE_ENTRY(IDropTarget)
END_COM_MAP()

BEGIN_MSG_MAP(CStorageWindowSQLite)
	CHAIN_MSG_MAP(CDialogResize<CStorageWindowSQLite>)
	CHAIN_MSG_MAP(CContextMenuWithIcons<CStorageWindowSQLite>)
	CHAIN_MSG_MAP(CContextHelpDlg<CStorageWindowSQLite>)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	COMMAND_HANDLER(IDC_FWSQL_KEYWORDS, EN_CHANGE, OnKeywordsChange)
	COMMAND_HANDLER(IDC_FWSQL_NAME, EN_CHANGE, OnNameChange)
	COMMAND_HANDLER(IDC_FWSQL_TYPE, CBN_SELCHANGE, OnFilterChange)
	COMMAND_HANDLER(IDC_FWSQL_TAGCLOUD, LBN_SELCHANGE, OnTagCloudChange)
	COMMAND_HANDLER(IDC_FWSQL_TAGCLOUD, LBN_DBLCLK, OnTagCloudDoubleClick)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
	COMMAND_HANDLER(ID_PICKDATABASE, BN_CLICKED, OnDatabasePick)
	COMMAND_HANDLER(ID_DEFAULTDATABASE, BN_CLICKED, OnDatabaseUseDefault)
	COMMAND_HANDLER(ID_IMPORTDATABASE, BN_CLICKED, OnDatabaseImport)
	COMMAND_HANDLER(ID_EXPORTDATABASE, BN_CLICKED, OnDatabaseExport)
	COMMAND_HANDLER(ID_IMPORTFILE, BN_CLICKED, OnFileImport)
	COMMAND_HANDLER(ID_EXPORTFILE, BN_CLICKED, OnFileExport)
	COMMAND_HANDLER(ID_DELETEFILE, BN_CLICKED, OnFileDelete)
	COMMAND_HANDLER(ID_RENAMEFILE, BN_CLICKED, OnFileRename)
	COMMAND_HANDLER(ID_MODIFYTAGS, BN_CLICKED, OnFileTagsModify)
	COMMAND_HANDLER(ID_FOLLOWLINK, BN_CLICKED, OnFollowLink)
	COMMAND_HANDLER(ID_ONLINEHELP, BN_CLICKED, OnOnlineHelp)
	COMMAND_HANDLER(ID_SHOWTHUMBNAILS, BN_CLICKED, OnShowThumbnails)
	COMMAND_HANDLER(ID_GROUPITEMS, BN_CLICKED, OnShowGroups)
	COMMAND_RANGE_CODE_HANDLER(ID_SORTBASE, ID_SORTBASE+10, BN_CLICKED, OnSortByColumn)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_ITEMACTIVATE, OnItemActivateList)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_ITEMCHANGED, OnItemChangedList)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_GETINFOTIP, OnGetInfoTipList)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_GETEMPTYMARKUP, OnGetEmptyMarkup)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_BEGINLABELEDIT, OnBeginLabelEdit)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_ENDLABELEDIT, OnEndLabelEdit)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_COLUMNCLICK, OnColumnClick)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_COLUMNOVERFLOWCLICK, OnColumnOverflowClick)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_KEYDOWN, OnKeyDown)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, NM_RCLICK, OnRClick)
	NOTIFY_HANDLER(IDC_FWSQL_TOOLBAR, TBN_DROPDOWN, OnButtonDropDown)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, NM_CUSTOMDRAW, OnCustomDraw)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_BEGINDRAG, OnBeginDrag)
	MESSAGE_HANDLER(WM_THUMBNAILREADY, OnThumbnailReady)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CStorageWindowSQLite)
	DLGRESIZE_CONTROL(IDC_FWSQL_TOOLBAR, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_FWSQL_TAGCLOUD, DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_FWSQL_LISTING, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_FWSQL_NAMELABEL, DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_FWSQL_NAME, DLSZ_MOVE_Y|DLSZ_DIVSIZE_X(2))
	DLGRESIZE_CONTROL(IDC_FWSQL_TAGSLABEL, DLSZ_MOVE_Y|DLSZ_DIVMOVE_X(2))
	DLGRESIZE_CONTROL(IDC_FWSQL_KEYWORDS, DLSZ_MOVE_Y|DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
	DLGRESIZE_CONTROL(IDC_FWSQL_TYPELABEL, DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_FWSQL_TYPE, DLSZ_MOVE_Y|DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_FWSQL_NEWREVISION, DLSZ_MOVE_Y|DLSZ_MOVE_X)
END_DLGRESIZE_MAP()

BEGIN_CTXHELP_MAP(CStorageWindowSQLite)
	CTXHELP_CONTROL_RESOURCE(IDC_FWSQL_TAGCLOUD, IDS_HELP_TAGCLOUD)
	CTXHELP_CONTROL_RESOURCE_NOTIP(IDC_FWSQL_LISTING, IDS_HELP_FILELIST)
	CTXHELP_CONTROL_RESOURCE_NOTIP(IDC_FWSQL_TOOLBAR, IDS_HELP_TOOLBAR)
	CTXHELP_CONTROL_RESOURCE(IDC_FWSQL_NAME, IDS_HELP_FILENAME)
	CTXHELP_CONTROL_RESOURCE(IDC_FWSQL_KEYWORDS, IDS_HELP_KEYWORDS)
	CTXHELP_CONTROL_RESOURCE(IDC_FWSQL_TYPE, IDS_HELP_FILETYPE)
	CTXHELP_CONTROL_RESOURCE(IDC_FWSQL_NEWREVISION, IDS_HELP_NEWREVISION)
END_CTXHELP_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IChildWindow methods
public:
	STDMETHOD(Show)(BOOL a_bShow)
	{
		HRESULT hRes = CChildWindowImpl<CStorageWindowSQLite, IStorageFilterWindow>::Show(a_bShow);
		if (a_bShow)
		{
			if (m_pListener)
				m_pListener->DocumentChanged(m_pActiveDocType);
		}
		return hRes;
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

	// IDropTarget methods
public:
	STDMETHOD(DragEnter)(IDataObject* a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);
	STDMETHOD(DragOver)(DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject *a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);

	// message handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDrawItem(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMeasureItem(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnDatabasePick(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnDatabaseUseDefault(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnDatabaseImport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnDatabaseExport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileImport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileExport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileDelete(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileRename(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileTagsModify(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFollowLink(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnOnlineHelp(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnShowThumbnails(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnShowGroups(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnKeywordsChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnNameChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnItemActivateList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnItemChangedList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnGetInfoTipList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnGetEmptyMarkup(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnBeginLabelEdit(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnEndLabelEdit(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnColumnClick(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnColumnOverflowClick(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnThumbnailReady(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFilterChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnTagCloudChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnTagCloudDoubleClick(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnKeyDown(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnRClick(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnSortByColumn(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnButtonDropDown(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnCustomDraw(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnBeginDrag(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled);

	STDMETHOD(Destroy)()
	{
		{
			ObjectLock cLock(this);
			m_bDestroyed = true;
			if (m_wndList.m_hWnd)
			{
				int nItems = m_wndList.GetItemCount();
				for (int i = 0; i < nItems; ++i)
				{
					SListItemInfo* pInfo = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(i));
					if (pInfo && pInfo->pLoc)
						pInfo->pLoc->Release();
					delete pInfo;
				}
				m_wndList.DeleteAllItems();
			}
			if (m_pThumbnails)
			{
				m_pThumbnails->CancelRequests(this);
				m_pThumbnails = NULL;
			}
			m_pCache = NULL;
		}
		return CChildWindowImpl<CStorageWindowSQLite, IStorageFilterWindow>::Destroy();
	}

	// IThumbnailCallback methods
public:
	STDMETHOD(AdjustSize)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		*a_pSizeX = m_szThumbnail.cx;
		*a_pSizeY = m_szThumbnail.cy;
		return S_OK;
	}
	STDMETHOD(SetThumbnail)(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData, RECT const* a_prcBounds, BSTR a_bstrInfo);

	static HICON IconFromThumbnail(ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData);

private:
	struct SListItemInfo
	{
		LONGLONG nUID;
		IStorageFilter* pLoc;
		wchar_t szGUID[23];
		ULONGLONG nCreated;
		ULONGLONG nModified;
		ULONGLONG nAccessed;
		ULONG nSize;
		int nRelevancy;
		std::wstring strNote;
		std::wstring strInfo;
		bool bImageRequested;
	};
	struct lessstrcmp
	{
		bool operator()(char const* a_1, char const* a_2) const
		{
			return strcmp(a_1, a_2) < 0;
		}
	};
	typedef std::set<char*, lessstrcmp> CTags2;
	typedef std::vector<std::pair<std::wstring, size_t> > CTags;
	typedef std::vector<std::wstring> CInitialTags;
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
	struct SSortItem
	{
		BYTE iCol; // column index
		char iOrd; // D = desc, A = asc
	};
	typedef std::list<SSortItem> CSortSpec;
	struct compare_tags_card
	{
		compare_tags_card(size_t const* a_pLimits) : m_pLimits(a_pLimits)
		{
		}
		bool operator()(std::pair<std::wstring, size_t> const& a_lhs, std::pair<std::wstring, size_t> const& a_rhs) const
		{
			if (a_lhs.second >= m_pLimits[1])
			{
				if (a_rhs.second >= m_pLimits[1])
					return a_lhs.first.compare(a_rhs.first) < 0;

				return true;
			}

			if (a_rhs.second >= m_pLimits[1])
				return false;
			
			return a_lhs.first.compare(a_rhs.first) < 0;
		}

	private:
		size_t const* m_pLimits;
	};

private:
	void GetItemTextUTF8(UINT a_uID, CAutoVectorPtr<char>& a_pszName);
	static void WStrToUTF8(wchar_t const* a_pszIn, CAutoVectorPtr<char>& a_pszOut);
	void UpdateListedItems(LONGLONG a_nSelected = 0);
	bool AddItemToList(LONGLONG a_nID, int a_nGrp, int a_nMatching, bool a_bSelect, bool a_bOverrideFilter = false);
	static void ParseTags(char* a_psz, CTags2& a_cTags); // modifies the a_psz (replaces spaces with '\0's)
	static void ParseTags(char const* a_psz, std::set<std::wstring>& a_cTags);
	static void ParseTags(wchar_t const* a_psz, std::set<std::wstring>& a_cTags);
	void InitFiltersCombo(IEnumUnknowns* a_pFormatFilters);
	void InitTagCloud();
	void InitToolbar();
	bool AnalyzeLocator(LPCOLESTR a_pszName);
	void NotifyListenerDocumentChange() const
	{
		if (m_pActiveDocType != NULL && m_pListener != NULL)
			m_pListener->DocumentChanged(m_pActiveDocType);
	}
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
	void UpdateToolbarFile();
	//void UpdateToolbarDatabase();
	static int __stdcall CompareListItems(LPARAM a_lParam1, LPARAM a_lParam2, LPARAM a_lParamSort);
	static COLORREF BlendColors(COLORREF a_clr1, COLORREF a_clr2, ULONG a_nWeight1);
	IDropTargetHelper* GetDropHelper()
	{
		if (!m_bDragHelperCreated)
		{
			m_bDragHelperCreated = true;
			m_pDragHelper.CoCreateInstance(CLSID_DragDropHelper);
		}
		return m_pDragHelper;
	}

private:
	CComPtr<IConfig> m_pContextConfig;
	CComPtr<IStorageFilterWindowCallback> m_pCallback;
	CComPtr<IStorageFilterWindowListener> m_pListener;
	DWORD m_dwFlags;
	CEdit m_wndName;
	CButton m_wndNewRevision;

	// toolbar
	CToolBarCtrl m_wndToolBar;
	CImageList m_cToolBar;

	// tags
	CListBox m_wndTagCloud;
	CEdit m_wndTags;
	CTags m_cTags;
	int m_nActiveTags;
	CAutoVectorPtr<int> m_aActiveTags;
	int m_nLabelHeight;
	size_t m_aLimits[4];
	CFont m_cFontBold;
	CFont m_cFontSmall;

	// files of type:
	CDocTypes m_cDocTypes;
	CComboBoxEx m_wndFilter;
	CImageList m_hFilterImages;
	CComPtr<IDocumentType> m_pActiveDocType;
	int m_nDefDocType;

	// list of files
	int m_iActiveItem;
	CListViewCtrl m_wndList;
	CComPtr<IEnumUnknowns> m_pDocTypes;
	ULONG m_nDocTypes;
	CSortSpec m_cSortSpec;

	// drag and drop
	std::tstring m_strLastDraggedFile;
	DWORD m_dwLastDropEffect;
	CComPtr<IDropTargetHelper> m_pDragHelper;
	bool m_bDragHelperCreated;

	// thumbnails
	CImageList m_cThumbnails;
	SIZE m_szThumbnail;
	CComPtr<IAsyncThumbnailRenderer> m_pThumbnails;
	CComPtr<IThumbnailCache> m_pCache;

	// initial location
	CSQLiteWrapper m_cDb;
	bool m_bInitialValid;
	std::wstring m_szDatabase;
	std::wstring m_szInitialFileName;
	OLECHAR m_szInitialGUID[23];
	CInitialTags m_cInitialTags;

	bool m_bDestroyed;
};

extern __declspec(selectany) OLECHAR const CFGID_SQL_LASTDATABASE[] = L"LastDatabase";
extern __declspec(selectany) OLECHAR const CFGID_SQL_LASTTAGS[] = L"LastTags";
extern __declspec(selectany) OLECHAR const CFGID_SQL_LASTFILTER[] = L"LastFilter";
extern __declspec(selectany) OLECHAR const CFGID_SQL_LASTSORTSPEC[] = L"LastSortSpec";
extern __declspec(selectany) OLECHAR const CFGID_SQL_USETHUMBNAILS[] = L"UseThumbnails";
extern __declspec(selectany) OLECHAR const CFGID_SQL_USEGROUPS[] = L"UseGroups";

void FilesFromDrop(IDataObject* a_pDataObj, std::vector<std::tstring>& a_cFileList);
CComPtr<IEnumUnknowns> GetDatabaseFormatFilter();
int __stdcall LVGroupCompare(int a_nGroup1_ID, int a_nGroup2_ID, void*);
