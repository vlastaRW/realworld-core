// StorageWindowSQLite.h : Declaration of the CStorageBrowserSQLite

#pragma once
#include "resource.h"       // main symbols
#include "RWStorageSQLite.h"
#include <Win32LangEx.h>
#include <RWThumbnails.h>
#include <ContextMenuWithIcons.h>
#include <ContextHelpDlg.h>
#include "StorageWindowSQLite.h"



// CStorageBrowserSQLite

class ATL_NO_VTABLE CStorageBrowserSQLite :
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangDialogImpl<CStorageBrowserSQLite>,
	public CContextHelpDlg<CStorageBrowserSQLite>,
	public CChildWindowImpl<CStorageBrowserSQLite, IStorageFilterWindow>,
	public CContextMenuWithIcons<CStorageBrowserSQLite>,
	public IThumbnailCallback,
	public IDropTarget
{
public:
	CStorageBrowserSQLite() :
		m_bDestroyed(false), m_cDb(NULL), m_iActiveItem(-1), m_nDocTypes(0),
		m_bInitialValid(false), m_dwLastDropEffect(DROPEFFECT_NONE), m_bDragHelperCreated(false),
		m_bCmdsValid(false)
	{
	}
	~CStorageBrowserSQLite()
	{
		m_cMenuImages.Destroy();
		m_cThumbnails.Destroy();
		if (!m_strLastDraggedFile.empty())
			DeleteFile(m_strLastDraggedFile.c_str());
	}

	enum
	{
		IDD = IDD_FILTERBROWSERSQLITE,
		WM_THUMBNAILREADY = WM_APP+825,
		ID_DELETEFILE = 524,
		ID_RENAMEFILE,
		ID_MODIFYTAGS,
		ID_EXPORTFILE,
		ID_FOLLOWLINK,
		ID_DEFAULTCOMMAND,
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

BEGIN_COM_MAP(CStorageBrowserSQLite)
	COM_INTERFACE_ENTRY(IStorageFilterWindow)
	COM_INTERFACE_ENTRY(IThumbnailCallback)
	COM_INTERFACE_ENTRY(IDropTarget)
END_COM_MAP()

BEGIN_MSG_MAP(CStorageBrowserSQLite)
	CHAIN_MSG_MAP(CContextMenuWithIcons<CStorageBrowserSQLite>)
	CHAIN_MSG_MAP(CContextHelpDlg<CStorageBrowserSQLite>)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
	COMMAND_HANDLER(ID_EXPORTFILE, BN_CLICKED, OnFileExport)
	COMMAND_HANDLER(ID_DELETEFILE, BN_CLICKED, OnFileDelete)
	COMMAND_HANDLER(ID_RENAMEFILE, BN_CLICKED, OnFileRename)
	COMMAND_HANDLER(ID_MODIFYTAGS, BN_CLICKED, OnFileTagsModify)
	COMMAND_HANDLER(ID_FOLLOWLINK, BN_CLICKED, OnFollowLink)
	COMMAND_HANDLER(ID_SHOWTHUMBNAILS, BN_CLICKED, OnShowThumbnails)
	COMMAND_HANDLER(ID_GROUPITEMS, BN_CLICKED, OnShowGroups)
	COMMAND_HANDLER(ID_DEFAULTCOMMAND, BN_CLICKED, OnOK)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_ITEMACTIVATE, OnItemActivateList)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_ITEMCHANGED, OnItemChangedList)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_GETINFOTIP, OnGetInfoTipList)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_GETEMPTYMARKUP, OnGetEmptyMarkup)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_BEGINLABELEDIT, OnBeginLabelEdit)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_ENDLABELEDIT, OnEndLabelEdit)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_KEYDOWN, OnKeyDown)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, NM_RCLICK, OnRClick)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, NM_CUSTOMDRAW, OnCustomDraw)
	NOTIFY_HANDLER(IDC_FWSQL_LISTING, LVN_BEGINDRAG, OnBeginDrag)
	MESSAGE_HANDLER(WM_THUMBNAILREADY, OnThumbnailReady)
END_MSG_MAP()

BEGIN_CTXHELP_MAP(CStorageBrowserSQLite)
	CTXHELP_CONTROL_RESOURCE_NOTIP(IDC_FWSQL_LISTING, IDS_HELP_FILELIST)
END_CTXHELP_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IStorageFilterWindow methods
public:
	STDMETHOD(FilterCreate)(IStorageFilter** a_ppFilter);
	STDMETHOD(FiltersCreate)(IEnumUnknowns** a_ppFilters);
	STDMETHOD(DocTypesEnum)(IEnumUnknowns** a_pFormatFilters);
	STDMETHOD(DocTypeGet)(IDocumentType** a_pFormatFilter);
	STDMETHOD(DocTypeSet)(IDocumentType* a_pFormatFilter);
	STDMETHOD(NavigationCommands)(IEnumUnknowns** a_ppCommands);
	STDMETHOD(OnIdle)() { return S_FALSE; }

	HRESULT FilterCreate(int iSel, IStorageFilter** a_ppFilter);

	// IDropTarget methods
public:
	STDMETHOD(DragEnter)(IDataObject* a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);
	STDMETHOD(DragOver)(DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject *a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);

	// message handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileExport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileDelete(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileRename(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileTagsModify(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFollowLink(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnShowThumbnails(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnShowGroups(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnItemActivateList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnItemChangedList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnGetInfoTipList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnGetEmptyMarkup(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnBeginLabelEdit(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnEndLabelEdit(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnThumbnailReady(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnRClick(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
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
		return CChildWindowImpl<CStorageBrowserSQLite, IStorageFilterWindow>::Destroy();
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


	// internal methods
	HRESULT TagToggle(BSTR a_bstrTag)
	{
		std::wstring strTag(a_bstrTag);
		CInitialTags::const_iterator i;
		for (i = m_cInitialTags.begin(); i != m_cInitialTags.end(); ++i)
		{
			if (*i == strTag)
				break;
		}
		if (i != m_cInitialTags.end())
			m_cInitialTags.erase(i);
		else
			m_cInitialTags.push_back(strTag);

		// save last used tags
		CComBSTR bstrTags;
		for (i = m_cInitialTags.begin(); i != m_cInitialTags.end(); ++i)
		{
			if (bstrTags)
			{
				bstrTags += L" ";
				bstrTags += i->c_str();
			}
			else
			{
				bstrTags = i->c_str();
			}
		}
		if (bstrTags == NULL)
			bstrTags = L"";
		SetConfigValue(CComBSTR(CFGID_SQL_LASTTAGS), CConfigValue(bstrTags));

		// update items
		m_bCmdsValid = false;

		UpdateListedItems();
		return S_OK;
	}

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
	void InitTagCloud();
	void InitMenuIcons();
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

	CImageList m_cMenuImages;

	// tags
	CTags m_cTags;
	CComPtr<IEnumUnknownsInit> m_pCmds;
	bool m_bCmdsValid;
	CComAutoCriticalSection m_cCmdsLock;

	// files of type:
	CComPtr<IEnumUnknownsInit> m_pFormatFilters;
	CComPtr<IDocumentType> m_pActiveDocType;
	int m_nDefDocType;

	// list of files
	int m_iActiveItem;
	CListViewCtrl m_wndList;
	CComPtr<IEnumUnknowns> m_pDocTypes;
	ULONG m_nDocTypes;
	CSortSpec m_cSortSpec;
	CFont m_cFontSmall;

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

