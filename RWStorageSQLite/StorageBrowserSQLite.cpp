// StorageWindowSQLite.cpp : Implementation of CStorageBrowserSQLite

#include "stdafx.h"
#include "StorageSQLite.h"
#include "StorageBrowserSQLite.h"

#include <XPGUI.h>
#include <SharedStringTable.h>

#ifdef WIN64
#define MY_LVGROUP_V5_SIZE 0x38
#else
#define MY_LVGROUP_V5_SIZE LVGROUP_V5_SIZE
#endif

// CStorageBrowserSQLite

namespace std
{
	template<typename T>
	void swap(CComPtr<T>& p1, CComPtr<T>& p2)
	{
		T* p = p2;
		p2.p = p1.p;
		p1.p = p;
	}
}

HRESULT CStorageBrowserSQLite::Init(LPCOLESTR a_pszInitial, DWORD a_dwFlags, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, HWND a_hWnd, LCID a_tLocaleID)
{
	m_tLocaleID = a_tLocaleID;
	m_pCallback = a_pCallback;
	m_pListener = a_pListener;
	m_dwFlags = a_dwFlags;
	CComPtr<IDocumentType> pDefDocType;
	if (m_pListener)
		m_pListener->DefaultDocumentGet(&pDefDocType);

	static bool bCommonCompsInitialized = false;
	if (!bCommonCompsInitialized)
	{
		bCommonCompsInitialized = true;
		AtlInitCommonControls(ICC_BAR_CLASSES | ICC_USEREX_CLASSES);
	}

	m_pContextConfig = a_pContextConfig;

	// init doc types
	CComPtr<IInputManager> pIM;
	RWCoCreateInstance(pIM, __uuidof(InputManager));
	if (pIM)
		pIM->DocumentTypesEnum(&m_pDocTypes);
	if (m_pDocTypes)
		m_pDocTypes->Size(&m_nDocTypes);

	RWCoCreateInstance(m_pFormatFilters, __uuidof(EnumUnknowns));
	if (a_pFormatFilters)
	{
		CDocTypes cDocTypes;
		CComPtr<IDocumentTypeComposed> pAllSup;
		RWCoCreateInstance(pAllSup, __uuidof(DocumentTypeComposed));
		pAllSup->InitAsAllSupportedFiles();
		ULONG nSize = 0;
		a_pFormatFilters->Size(&nSize);
		for (ULONG i = 0; i < nSize; ++i)
		{
			CComPtr<IDocumentType> pDT;
			a_pFormatFilters->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pDT));
			if (pDT != NULL)
			{
				cDocTypes.push_back(pDT);
				pAllSup->DocTypesAddFromList(1, &(pDT.p));
			}
		}
		if (cDocTypes.size() > 1)
		{
			std::sort(cDocTypes.begin(), cDocTypes.end(), localized_doctype_compare(m_tLocaleID));
			cDocTypes.insert(cDocTypes.begin(), pAllSup.p);
		}
		{
			CComPtr<IDocumentTypeComposed> pAll;
			RWCoCreateInstance(pAll, __uuidof(DocumentTypeComposed));
			m_pActiveDocType = pAll;
			pAll->InitAsAllFiles();
			cDocTypes.push_back(m_pActiveDocType);
		}
		for (CDocTypes::const_iterator i = cDocTypes.begin(); i != cDocTypes.end(); ++i)
		{
			m_pFormatFilters->Insert(*i);
		}
	}

	m_nDefDocType = -1;
	if (a_pFormatFilters && pDefDocType)
	{
		CComPtr<IDocumentType> pDT;
		for (ULONG i = 0; SUCCEEDED(a_pFormatFilters->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pDT))); ++i)
		{
			if (pDT == pDefDocType)
			{
				m_nDefDocType = i;
				break;
			}
			pDT = NULL;
		}
	}

	// analyze initial locator
	HRESULT hRes = S_FALSE;
	if (a_pszInitial && wcsncmp(a_pszInitial, L"tags://", 7) == 0)
	{
		hRes = S_OK;
		m_bInitialValid = AnalyzeLocator(a_pszInitial+7);
	}
	else if (a_pszInitial)
	{
		// probably just a name
		wchar_t const* psz1 = wcsrchr(a_pszInitial, L'\\');
		wchar_t const* psz2 = wcsrchr(a_pszInitial, L'/');
		m_szInitialFileName = psz1 > psz2 ? (psz1+1) : (psz2 == NULL ? a_pszInitial : (psz2+1));
	}

	if (m_bInitialValid)
	{
		if (m_szDatabase.empty())
		{
			// use default database path
			CStorageSQLite::GetDefaultDatabase(m_szDatabase);
		}
		m_cDb.Open(m_szDatabase.c_str());
		if (m_cDb.ResCode() != SQLITE_OK)
		{
			m_bInitialValid = false;
			m_szDatabase.clear();
			m_cInitialTags.clear();
			m_szInitialGUID[0] = L'\0';
		}
	}
	if (!m_bInitialValid)
	{
		CConfigValue cVal;
		m_pContextConfig->ItemValueGet(CComBSTR(CFGID_SQL_LASTDATABASE), &cVal);
		if (cVal.operator BSTR() && cVal.operator BSTR()[0])
			m_szDatabase = cVal.operator BSTR();
		else
			CStorageSQLite::GetDefaultDatabase(m_szDatabase);
		m_cDb.Open(m_szDatabase.c_str());
	}

	if (m_cDb.ResCode() != SQLITE_OK)
		return E_FAIL;

	// read sort order
	{
		CConfigValue cVal;
		GetConfigValue(CComBSTR(CFGID_SQL_LASTSORTSPEC), &cVal);
		if (cVal.operator BSTR() && cVal.operator BSTR()[0])
		{
			for (LPWSTR psz = cVal; psz[0] && psz[1]; psz += 2)
			{
				SSortItem t = {psz[0]-L'0', psz[1] == L'A' ? 'A' : 'D'};
				if (t.iCol > 6)
					break;
				m_cSortSpec.push_back(t);
			}
		}
	}

	if (NULL == Create(a_hWnd, reinterpret_cast<LPARAM>(a_pFormatFilters)))
		return E_FAIL;

	return hRes;
}

bool CStorageBrowserSQLite::AnalyzeLocator(LPCOLESTR a_pszName)
{
	OLECHAR const* pszL = wcschr(a_pszName, L'<');
	OLECHAR const* pszG = wcschr(a_pszName, L'>');
	if (pszL && pszG && pszL < pszG)
	{
		if (wcschr(pszL+1, L'<') || wcschr(pszG+1, L'>'))
			return false; // invalid path - both < and > must be used exactly once if at all
		m_szDatabase.assign(pszL+1, pszG);
		a_pszName = pszG+1;
	}
	else if (pszL || pszG)
		return false; // invalid path - both < and > must be used exactly once if at all
	while (*a_pszName == L'\\' || *a_pszName == L'/')
		++a_pszName;

	m_szInitialGUID[0] = L'\0';
	//m_bUIDValid = false;
	//m_nUID = -1LL;

	// parse tags, GUID and filename
	while (true)
	{
		OLECHAR const* pszSep = a_pszName;
		while (*pszSep && *pszSep != L'\\' && *pszSep != L'/')
			++pszSep;
		if (pszSep > a_pszName)
		{
			if (*a_pszName == L'#' && pszSep-a_pszName == 23)
			{
				// GUID
				wcsncpy(m_szInitialGUID, a_pszName+1, 22);
				m_szInitialGUID[22] = L'\0';
			}
			else if (*pszSep == L'\0')
			{
				// filename
				m_szInitialFileName.assign(a_pszName, pszSep);
			}
			else
			{
				// tag
				size_t n = m_cInitialTags.size();
				m_cInitialTags.resize(n+1);
				m_cInitialTags[n].assign(a_pszName, pszSep);
			}
		}
		if (*pszSep == L'\0')
			break;
		a_pszName = pszSep+1;
	}
	return true;
}

STDMETHODIMP CStorageBrowserSQLite::FilterCreate(IStorageFilter** a_ppFilter)
{
	int iSel = m_wndList.GetSelectedIndex();
	if (iSel < 0 || iSel >= m_wndList.GetItemCount())
		return E_FAIL;
	return FilterCreate(iSel, a_ppFilter);
}

HRESULT CStorageBrowserSQLite::FilterCreate(int iSel, IStorageFilter** a_ppFilter)
{
	try
	{
		*a_ppFilter = NULL;

		SListItemInfo* pInfo = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(iSel));

		CComBSTR bstrName;
		m_wndList.GetItemText(iSel, 0, bstrName.m_str);
		CComBSTR bstrTags;
		m_wndList.GetItemText(iSel, 5, bstrTags.m_str);
		CStorageSQLite::CTags cTags;
		if (bstrTags.Length())
		{
			wchar_t const* pStart = NULL;
			for (wchar_t const* p = bstrTags; *p; ++p)
			{
				if (*p <= ' ')
				{
					if (pStart)
					{
						std::wstring str(pStart, p);
						cTags.push_back(str);
						pStart = NULL;
					}
				}
				else
				{
					if (pStart == NULL)
						pStart = p;
				}
			}
			if (pStart && *pStart)
			{
				std::wstring str(pStart);
				cTags.push_back(str);
			}
		}

		CComObject<CStorageSQLite>* p = NULL;
		CComObject<CStorageSQLite>::CreateInstance(&p);
		CComPtr<IStorageFilter> pTmp = p;
		p->Init(m_szDatabase.c_str(), bstrName, pInfo->szGUID, cTags);

		*a_ppFilter = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFilter == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageBrowserSQLite::FiltersCreate(IEnumUnknowns** a_ppFilters)
{
	try
	{
		*a_ppFilters = NULL;
		CComPtr<IStorageFilter> pFlt;
		HRESULT hRes = FilterCreate(&pFlt);
		if (pFlt == NULL)
			return hRes;
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		p->Insert(pFlt);
		*a_ppFilters = p.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFilters ? E_UNEXPECTED : E_NOTIMPL;
	}
}

STDMETHODIMP CStorageBrowserSQLite::DocTypesEnum(IEnumUnknowns** a_ppFormatFilters)
{
	try
	{
		*a_ppFormatFilters = NULL;
		(*a_ppFormatFilters = m_pFormatFilters)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFormatFilters ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageBrowserSQLite::DocTypeGet(IDocumentType** a_ppFormatFilter)
{
	try
	{
		*a_ppFormatFilter = NULL;
		(*a_ppFormatFilter = m_pActiveDocType)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFormatFilter ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageBrowserSQLite::DocTypeSet(IDocumentType* a_pFormatFilter)
{
	try
	{
		m_pActiveDocType = a_pFormatFilter;
		if (m_pActiveDocType != NULL)
		{
			CComBSTR bstrUID;
			m_pActiveDocType->UniqueIDGet(&bstrUID);
			SetConfigValue(CComBSTR(CFGID_SQL_LASTFILTER), CConfigValue(bstrUID));
		}
		InitTagCloud();
		UpdateListedItems();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <RWProcessing.h>
#include <DocumentMenuCommandImpl.h>
#include <IconRenderer.h>

void GetLayersTag(IStockIcons* pSI, CIconRendererReceiver& cRenderer, IRTarget const* target);

extern GUID const IconIDTagADD = {0x86e64c95, 0x1451, 0x42f3, {0x8c, 0xba, 0x6e, 0x63, 0x5, 0x1d, 0x76, 0xe2}};

class ATL_NO_VTABLE CMenuCommandTagAddGroup :
	public CDocumentMenuCommandImpl<CMenuCommandTagAddGroup, IDC_MN_TAGADD, IDC_MD_TAGADD, &IconIDTagADD, 0>
{
public:
	CMenuCommandTagAddGroup()
	{
		RWCoCreateInstance(m_pCmds, __uuidof(EnumUnknowns));
	}
	void Insert(IDocumentMenuCommand* a_pCmd)
	{
		m_pCmds->Insert(a_pCmd);
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			GetLayersTag(pSI, cRenderer, IRTarget(0.8f, -1, 1));
			pSI->GetLayers(ESIPlus, cRenderer, IRTarget(0.65f, 1, -1));
			*a_phIcon = cRenderer.get();
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	EMenuCommandState IntState() { return EMCSSubMenu; }
	STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands)
	{
		*a_ppSubCommands = NULL;
		(*a_ppSubCommands = m_pCmds)->AddRef();
		return S_OK;
	}

private:
	CComPtr<IEnumUnknownsInit> m_pCmds;
};

class ATL_NO_VTABLE CMenuCommandTagToggle :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand,
	public ILocalizedString
{
public:
	CMenuCommandTagToggle() : m_pWindow(NULL)
	{
	}
	~CMenuCommandTagToggle()
	{
		if (m_pWindow)
			m_pWindow->Release();
	}
	void Init(CStorageBrowserSQLite* a_pWindow, LPCWSTR a_pszTag, bool a_bDel)
	{
		(m_pWindow = a_pWindow)->AddRef();
		m_bstrTag = a_pszTag;
		m_bDel = a_bDel;
	}

BEGIN_COM_MAP(CMenuCommandTagToggle)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	COM_INTERFACE_ENTRY(ILocalizedString)
END_COM_MAP()

	// ILocalizedString methods
	STDMETHOD(Get)(BSTR* a_pbstrString)
	{
		return m_bstrTag.CopyTo(a_pbstrString);
	}
	STDMETHOD(GetLocalized)(LCID UNREF(a_tLCID), BSTR* a_pbstrString) { return Get(a_pbstrString); }

	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		(*a_ppText = this)->AddRef();
		return S_OK;
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			return E_NOTIMPL;
			//(*a_ppText = this)->AddRef();
			//return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		if (!m_bDel) return E_NOTIMPL;
		static const GUID tDel = {0x9d327383, 0xe79e, 0x4888, {0xb2, 0x95, 0x85, 0x38, 0x98, 0xfa, 0x79, 0x4e}};
		*a_pIconID = tDel;
		return S_OK;
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		if (!m_bDel) return E_NOTIMPL;
		try
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			GetLayersTag(pSI, cRenderer, IRTarget(0.8f, -1, 1));
			pSI->GetLayers(m_bDel ? ESIDelete : ESIPlus, cRenderer, IRTarget(0.65f, 1, -1));
			*a_phIcon = cRenderer.get();
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel) { return E_NOTIMPL; }
	STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands) { return E_NOTIMPL; }
	STDMETHOD(State)(EMenuCommandState* a_peState)
	{
		try
		{
			*a_peState = EMCSShowButtonText;
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		return m_pWindow->TagToggle(m_bstrTag);
	}

private:
	CStorageBrowserSQLite* m_pWindow;
	CComBSTR m_bstrTag;
	bool m_bDel;
};

STDMETHODIMP CStorageBrowserSQLite::NavigationCommands(IEnumUnknowns** a_ppCommands)
{
	try
	{
		*a_ppCommands = NULL;
		CComPtr<IEnumUnknowns> pCmds;
		{
			m_cCmdsLock.Lock();
			if (!m_bCmdsValid)
			{
				m_pCmds = NULL;
				RWCoCreateInstance(m_pCmds, __uuidof(EnumUnknowns));
				CComObject<CMenuCommandTagAddGroup>* pAdd = NULL;
				CComObject<CMenuCommandTagAddGroup>::CreateInstance(&pAdd);
				CComPtr<IDocumentMenuCommand> pAddCmd = pAdd;
				m_pCmds->Insert(pAddCmd);
				CComPtr<IDocumentMenuCommand> pSep;
				RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
				m_pCmds->Insert(pSep);
				std::set<std::wstring> cAct;
				for (CInitialTags::const_iterator i = m_cInitialTags.begin(); i != m_cInitialTags.end(); ++i)
				{
					cAct.insert(*i);
					CComObject<CMenuCommandTagToggle>* p = NULL;
					CComObject<CMenuCommandTagToggle>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pTmp = p;
					p->Init(this, i->c_str(), true);
					m_pCmds->Insert(pTmp);
				}

				CTags cTags = m_cTags;
				if (!cTags.empty())
				{
					std::vector<size_t> aCnts;
					for (CTags::const_iterator i = cTags.begin(); i != cTags.end(); ++i)
						aCnts.push_back(i->second);

					std::sort(aCnts.begin(), aCnts.end());
					size_t nMedian = aCnts[aCnts.size()>>1];
					size_t nBorder = aCnts[(aCnts.size()*3)>>2];
					if (nBorder <= nMedian) nBorder = nMedian+1;
					size_t nTop = aCnts[(aCnts.size()*7)>>3];
					if (nTop <= nBorder) nTop = nBorder+1;
					size_t aLimits[4] = {nTop, nBorder, nMedian, nMedian-1};
					std::sort(cTags.begin(), cTags.end(), compare_tags_card(aLimits));

					for (CTags::const_iterator i = cTags.begin(); i != cTags.end(); ++i)
					{
						if (i != cTags.begin() && i->second < nBorder && (i-1)->second >= nBorder)
							pAdd->Insert(pSep);
						if (cAct.find(i->first) != cAct.end())
							continue;
						CComObject<CMenuCommandTagToggle>* p = NULL;
						CComObject<CMenuCommandTagToggle>::CreateInstance(&p);
						CComPtr<IDocumentMenuCommand> pTmp = p;
						p->Init(this, i->first.c_str(), false);
						pAdd->Insert(pTmp);
					}
				}
				m_bCmdsValid = true;
			}
			pCmds = m_pCmds;
			m_cCmdsLock.Unlock();
		}
		*a_ppCommands = pCmds.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppCommands ? E_UNEXPECTED : E_POINTER;
	}
}

void CStorageBrowserSQLite::InitTagCloud()
{
	if (m_cDb.ResCode() != SQLITE_OK)
		return;

	m_cTags.clear();

	if (m_pActiveDocType)
	{
		std::map<std::wstring, size_t> cTags;
		CSQLiteStatement cStmt(m_cDb, "SELECT tags.name,files.name FROM tags LEFT JOIN files ON tags.fid=files.id");
		while (sqlite3_step(cStmt) == SQLITE_ROW)
		{
			wchar_t const* pszTag = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0));
			wchar_t const* pszName = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 1));
			if (m_pActiveDocType->MatchFilename(CComBSTR(pszName)) == S_OK)
			{
				std::wstring strTag(pszTag);
				std::map<std::wstring, size_t>::iterator i = cTags.find(strTag);
				if (i != cTags.end())
					++(i->second);
				else
					cTags[strTag] = 1;
			}
			m_cTags.resize(cTags.size());
		}
		std::copy(cTags.begin(), cTags.end(), m_cTags.begin());
	}
	else
	{
		CSQLiteStatement cStmt(m_cDb, "SELECT name,COUNT(*) FROM tags GROUP BY name"); // ORDER BY COUNT(*) DESC
		while (sqlite3_step(cStmt) == SQLITE_ROW)
		{
			size_t nCnt = sqlite3_column_int64(cStmt, 1);
			wchar_t const* psz = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0));
			m_cTags.push_back(std::make_pair(std::wstring(psz), nCnt));
		}
	}
	m_bCmdsValid = false;
}

void InitMenuItemIcons(IStockIcons* pSI, CImageList& cToolBar, int nIconSize, RECT const padding);

void CStorageBrowserSQLite::InitMenuIcons()
{
	int nIconSize = XPGUI::GetSmallIconSize();
	m_cMenuImages.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 9, 0);
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	RECT r = {0, 0, 0, 0};
	InitMenuItemIcons(pSI, m_cMenuImages, nIconSize, r);
}

LRESULT CStorageBrowserSQLite::OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	AddRef();

	m_wndList = GetDlgItem(IDC_FWSQL_LISTING);

	RegisterDragDrop(m_wndList, this);

	InitMenuIcons();

	// init thumbnails
	HDC hdc = GetDC();
	int nScale = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(hdc);
	m_szThumbnail.cx = (64 * nScale + 48) / 96;
	m_szThumbnail.cy = (48 * nScale + 48) / 96;
	m_cThumbnails.Create(m_szThumbnail.cx, m_szThumbnail.cy, XPGUI::GetImageListColorFlags(), 16, 8);
	/*if (XPGUI::IsXP())*/ m_wndList.SetImageList(m_cThumbnails, LVSIL_NORMAL);
	int const nSpacing = (80 * nScale + 48) / 96;
	m_wndList.SetIconSpacing(nSpacing, nSpacing);

	m_wndList.SetExtendedListViewStyle(LVS_EX_INFOTIP|(XPGUI::IsXP() ? LVS_EX_DOUBLEBUFFER : 0));
	if (XPGUI::IsVista() && CTheme::IsThemingSupported())
	{
		::SetWindowTheme(m_wndList, L"explorer", NULL);
	}

	TCHAR szColumn[] = _T("");
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = szColumn;
	lvc.cx = 5;
	lvc.iSubItem = 0;
	m_wndList.InsertColumn(0, &lvc);
	lvc.iSubItem++;
	m_wndList.InsertColumn(1, &lvc);
	lvc.iSubItem++;
	m_wndList.InsertColumn(2, &lvc);
	lvc.iSubItem++;
	m_wndList.InsertColumn(3, &lvc);
	lvc.iSubItem++;
	m_wndList.InsertColumn(4, &lvc);
	lvc.iSubItem++;
	m_wndList.InsertColumn(5, &lvc);
	lvc.iSubItem++;
	m_wndList.InsertColumn(6, &lvc);

	if (XPGUI::IsXP())
	{
		CConfigValue cVal;
		GetConfigValue(CComBSTR(CFGID_SQL_USEGROUPS), &cVal);
		if (cVal)
			m_wndList.EnableGroupView(TRUE);
	}

	CComPtr<IThumbnailRenderer> pRenderer;
	RWCoCreateInstance(pRenderer, __uuidof(ThumbnailRenderer));
	RWCoCreateInstance(m_pCache, __uuidof(ThumbnailCache));
	TCHAR szTmpPath[MAX_PATH+30];
	GetTempPath(MAX_PATH, szTmpPath);
	TCHAR szModule[MAX_PATH] = _T("");
	GetModuleFileName(NULL, szModule, MAX_PATH);
	GetLongPathName(szModule, szModule, MAX_PATH);
	TCHAR const* pszModuleName = _tcsrchr(szModule, _T('\\'));
	if (pszModuleName == NULL) pszModuleName = szModule;
	TCHAR const* pszDot = _tcsrchr(pszModuleName, _T('.'));
	if (pszDot) *const_cast<TCHAR*>(pszDot) = _T('\0');
	_tcscat(const_cast<TCHAR*>(pszModuleName), _T("Thumbnails"));
	_tcscat(szTmpPath, pszModuleName);
	CreateDirectory(szTmpPath, NULL);
	m_pCache->Init(pRenderer, 0, 256, CComBSTR(szTmpPath));
	RWCoCreateInstance(m_pThumbnails, __uuidof(AsyncThumbnailRenderer));
	m_pThumbnails->Init(m_pCache);
	CAutoVectorPtr<DWORD> cNoThumbnail(new DWORD[m_szThumbnail.cx*m_szThumbnail.cy]);
	pRenderer->GetThumbnail(NULL, m_szThumbnail.cx, m_szThumbnail.cy, cNoThumbnail, NULL, 0, NULL);
	HICON h = IconFromThumbnail(m_szThumbnail.cx, m_szThumbnail.cy, cNoThumbnail);
	m_cThumbnails.AddIcon(h);
	DestroyIcon(h);

	{
		LOGFONT lf = {0};
		::GetObject(GetFont(), sizeof(lf), &lf);
		LONG l = lf.lfHeight;
		lf.lfHeight = (lf.lfHeight*5)/6;
		m_cFontSmall.CreateFontIndirect(&lf);
	}

	// check intial file
	LONGLONG nInitialUID = 0;
	bool bReadLastTags = true;
	if (m_bInitialValid)
	{
		CComObject<CStorageSQLite>* p = NULL;
		CComObject<CStorageSQLite>::CreateInstance(&p);
		CComPtr<IStorageFilter> pTmp = p;
		p->Init(m_szDatabase.c_str(), m_szInitialFileName.c_str(), m_szInitialGUID, m_cInitialTags);
		nInitialUID = p->M_UID();
		if (nInitialUID > 0)
		{
			char szQuery[128];
			bReadLastTags = false;
			m_cInitialTags.clear();
			sprintf(szQuery, "SELECT name FROM tags WHERE fid=%lli", nInitialUID);
			CSQLiteStatement cStmt(m_cDb, szQuery);
			while (sqlite3_step(cStmt) == SQLITE_ROW)
			{
				m_cInitialTags.push_back(std::wstring(reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0))));
			}
		}
	}
	if (bReadLastTags)
	{
		m_cInitialTags.clear();
		CConfigValue cTags;
		GetConfigValue(CComBSTR(CFGID_SQL_LASTTAGS), &cTags);
		if (cTags.TypeGet() == ECVTString && cTags.operator BSTR() && cTags.operator BSTR()[0])
		{
			wchar_t* pStart = NULL;
			for (wchar_t* p = cTags.operator BSTR(); *p; ++p)
			{
				if (*p <= L' ')
				{
					*p = L'\0';
					if (pStart)
					{
						m_cInitialTags.push_back(pStart);
						pStart = NULL;
					}
				}
				else
				{
					if (pStart == NULL)
						pStart = p;
				}
			}
			if (pStart && *pStart)
				m_cInitialTags.push_back(pStart);
		}
	}

	CConfigValue cLastFilter;
	GetConfigValue(CComBSTR(CFGID_SQL_LASTFILTER), &cLastFilter);
	if (cLastFilter.TypeGet() == ECVTString)
	{
		if (m_pFormatFilters)
		{
			ULONG nTypes = 0;
			m_pFormatFilters->Size(&nTypes);
			for (ULONG i = 0; i < nTypes; ++i)
			{
				CComPtr<IDocumentType> pDT;
				m_pFormatFilters->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pDT));
				CComBSTR bstrUID;
				pDT->UniqueIDGet(&bstrUID);
				if (bstrUID != NULL && cLastFilter.operator BSTR() != NULL && wcscmp(bstrUID, cLastFilter) == 0)
				{
					m_pActiveDocType = pDT;
					break;
				}
			}
		}
	}

	InitTagCloud();

	UpdateListedItems(nInitialUID);

	return TRUE;
}

LRESULT CStorageBrowserSQLite::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_wndList.MoveWindow(0, 0, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
	return 0;
}

LRESULT CStorageBrowserSQLite::OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	GotoDlgCtrl(m_wndList);
	return 0;
}

LRESULT CStorageBrowserSQLite::OnItemActivateList(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(a_pNMHDR);

	m_iActiveItem = pNMIA->iItem;

	if (m_pCallback != NULL)
		m_pCallback->ForwardOK();

	return 0;
}

LRESULT CStorageBrowserSQLite::OnItemChangedList(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
{
	m_iActiveItem = m_wndList.GetSelectedIndex();
	return 0;
}

LRESULT CStorageBrowserSQLite::OnGetInfoTipList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(a_pNMHDR);
	TCHAR szType[MAX_PATH] = _T("");
	m_wndList.GetItemText(pGetInfoTip->iItem, 1, szType, itemsof(szType));
	//TCHAR szSize[MAX_PATH] = _T("");
	//m_wndList.GetItemText(pGetInfoTip->iItem, 2, szSize, itemsof(szSize));
	//TCHAR szPath[MAX_PATH] = _T("");
	//m_wndList.GetItemText(pGetInfoTip->iItem, 6, szPath, itemsof(szPath));
	TCHAR szTags[MAX_PATH] = _T("");
	m_wndList.GetItemText(pGetInfoTip->iItem, 5, szTags, itemsof(szTags));
	//TCHAR szCrtd[64] = _T("");
	//m_wndList.GetItemText(pGetInfoTip->iItem, 4, szCrtd, itemsof(szCrtd));
	TCHAR szDate[64] = _T("");
	m_wndList.GetItemText(pGetInfoTip->iItem, 3, szDate, itemsof(szDate));

	TCHAR szTypeName[64] = _T("");
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_TYPE, szTypeName, itemsof(szTypeName), LANGIDFROMLCID(m_tLocaleID));
	//TCHAR szSizeName[64] = _T("");
	//Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_SIZE, szSizeName, itemsof(szSizeName), LANGIDFROMLCID(m_tLocaleID));
	TCHAR szTagsName[64] = _T("");
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_TAGS, szTagsName, itemsof(szTagsName), LANGIDFROMLCID(m_tLocaleID));
	TCHAR szDateName[64] = _T("");
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_DATEMOD, szDateName, itemsof(szDateName), LANGIDFROMLCID(m_tLocaleID));
	//TCHAR szCrtdName[64] = _T("");
	//Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_DATECRE, szCrtdName, itemsof(szCrtdName), LANGIDFROMLCID(m_tLocaleID));
	SListItemInfo* pItem = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(pGetInfoTip->iItem));

	if (pItem == NULL || pItem->strNote.empty())
	{
		if (pItem == NULL || pItem->strInfo.empty())
		{
			_sntprintf(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, _T("%s: %s\n%s: %s\n%s: %s"), szTypeName, szType, szTagsName, szTags, szDateName, szDate);
		}
		else
		{
			CW2CT cInfo(pItem->strInfo.c_str());
			_sntprintf(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, _T("%s: %s\n%s: %s\n%s: %s\n%s"), szTypeName, szType, szTagsName, szTags, szDateName, szDate, static_cast<LPCTSTR>(cInfo));
		}
	}
	else
	{
		if (pItem->strInfo.empty())
		{
			CW2CT cNote(pItem->strNote.c_str());
			_sntprintf(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, _T("%s: %s\n%s: %s\n%s: %s\n-----\n%s"), szTypeName, szType, szTagsName, szTags, szDateName, szDate, static_cast<LPCTSTR>(cNote));
		}
		else
		{
			CW2CT cNote(pItem->strNote.c_str());
			CW2CT cInfo(pItem->strInfo.c_str());
			_sntprintf(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, _T("%s: %s\n%s: %s\n%s: %s\n%s\n-----\n%s"), szTypeName, szType, szTagsName, szTags, szDateName, szDate, static_cast<LPCTSTR>(cInfo), static_cast<LPCTSTR>(cNote));
		}
	}

	return 0;
}

LRESULT CStorageBrowserSQLite::OnGetEmptyMarkup(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	NMLVEMPTYMARKUP* markupInfo = reinterpret_cast<NMLVEMPTYMARKUP*>(a_pNMHDR);
	markupInfo->dwFlags = EMF_CENTERED;
	Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_NOMATCHINGFILES, markupInfo->szMarkup, itemsof(markupInfo->szMarkup), LANGIDFROMLCID(m_tLocaleID));
	return TRUE; // set the markup
}

LRESULT CStorageBrowserSQLite::OnKeyDown(int UNREF(a_nCtrlID), LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	LPNMTVKEYDOWN pNMTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(a_pNMHeader);

	if (m_iActiveItem >= 0 && pNMTVKeyDown->wVKey == VK_F2)
	{
		m_wndList.EnsureVisible(m_iActiveItem, FALSE);
		m_wndList.EditLabel(m_iActiveItem);
		return 1;
	}
	else if (m_iActiveItem >= 0 && pNMTVKeyDown->wVKey == VK_F4)
	{
		BOOL b;
		OnFileTagsModify(0, 0, NULL, b);
		return 1;
	}
	else if (m_iActiveItem >= 0 && pNMTVKeyDown->wVKey == VK_DELETE)
	{
		BOOL b;
		OnFileDelete(0, 0, NULL, b);
		return 1;
	}

	a_bHandled = FALSE;
	return 0;
}

LRESULT CStorageBrowserSQLite::OnRClick(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	NMITEMACTIVATE* pItem = reinterpret_cast<NMITEMACTIVATE*>(a_pNMHeader);

	if (pItem->iItem >= 0)
	{
		Reset(m_cMenuImages);
		CMenu cMenu;
		cMenu.CreatePopupMenu();
		TCHAR szTmp[256] = _T("");

		if (m_pCallback)
		{
			CComPtr<ILocalizedString> pDefCmd;
			GUID tIconID = GUID_NULL;
			m_pCallback->DefaultCommand(&pDefCmd, NULL, &tIconID);
			int nIcon = -1;
			if (!IsEqualGUID(tIconID, GUID_NULL))
			{
				HICON hIc = NULL;
				int x = 16, y = 16;
				m_cMenuImages.GetIconSize(x, y);
				m_pCallback->DefaultCommandIcon(x, &hIc);
				if (hIc)
				{
					m_cMenuImages.ReplaceIcon(0, hIc);
					DestroyIcon(hIc);
					nIcon = 0;
				}
			}
			CComBSTR bstrDefCmd;
			if (pDefCmd)
				pDefCmd->GetLocalized(m_tLocaleID, &bstrDefCmd);
			if (bstrDefCmd.Length())
			{
				AddItem(ID_DEFAULTCOMMAND, COLE2CT(bstrDefCmd.m_str), nIcon);
				cMenu.AppendMenu(MFT_OWNERDRAW, ID_DEFAULTCOMMAND, LPCTSTR(NULL));
				cMenu.AppendMenu(MFT_SEPARATOR);
			}
		}
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_RENAMEFILE, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_RENAMEFILE, szTmp, 4);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_RENAMEFILE, LPCTSTR(NULL));
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_MODIFYTAGS, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_MODIFYTAGS, szTmp, 5);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_MODIFYTAGS, LPCTSTR(NULL));
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_DELETEFILE, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_DELETEFILE, szTmp, 3);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_DELETEFILE, LPCTSTR(NULL));
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_EXPORTFILE, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_EXPORTFILE, szTmp, 2);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_EXPORTFILE, LPCTSTR(NULL));
		SListItemInfo* p = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(pItem->iItem));
		if (p && !p->strNote.empty() && p->strNote.find(L"http://") != std::wstring::npos)
		{
			cMenu.AppendMenu(MFT_SEPARATOR);
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_FOLLOWLINK, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
			AddItem(ID_FOLLOWLINK, szTmp, -1);
			cMenu.AppendMenu(MFT_OWNERDRAW, ID_FOLLOWLINK, LPCTSTR(NULL));
		}

		POINT tPt = pItem->ptAction;
		m_wndList.ClientToScreen(&tPt);
		cMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, tPt.x, tPt.y, m_hWnd, NULL);
	}
	return 0;
}

LRESULT CStorageBrowserSQLite::OnBeginLabelEdit(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	LPNMTVDISPINFO pNMTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(a_pNMHDR);
	CEdit wndEdit = m_wndList.GetEditControl();
	int nLen = wndEdit.GetWindowTextLength();
	CAutoVectorPtr<TCHAR> psz(new TCHAR[nLen+1]);
	wndEdit.GetWindowText(psz, nLen+1);
	psz[nLen] = _T('\0');
	LPCTSTR pszDot = _tcsrchr(psz, _T('.'));
	if (pszDot)
		wndEdit.SetSel(0, pszDot-psz.m_p, TRUE);
	return 0;
}

LRESULT CStorageBrowserSQLite::OnEndLabelEdit(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	NMLVDISPINFO* pDI = reinterpret_cast<NMLVDISPINFO*>(a_pNMHDR);
	SListItemInfo* pItem = reinterpret_cast<SListItemInfo*>(pDI->item.lParam);
	if (pDI->item.pszText == NULL)
		return 0;

	CT2CW str(pDI->item.pszText);
	char szQuery[512];
	sprintf(szQuery, "UPDATE files SET name=? WHERE id=%lli", pItem->nUID);
	CSQLiteStatement cStmt(m_cDb, szQuery);
	sqlite3_bind_text16(cStmt, 1, static_cast<LPCWSTR>(str), -1, SQLITE_STATIC/*SQLITE_TRANSIENT*/);
	if (sqlite3_step(cStmt) != SQLITE_DONE)
		return 0; // TODO: report error
	return 1;
}

int __stdcall CStorageBrowserSQLite::CompareListItems(LPARAM a_lParam1, LPARAM a_lParam2, LPARAM a_lParamSort)
{
	CStorageBrowserSQLite* pThis = reinterpret_cast<CStorageBrowserSQLite*>(a_lParamSort);

	CComBSTR bstr1;
	pThis->m_wndList.GetItemText(a_lParam1, 0, bstr1.m_str);
	CComBSTR bstr2;
	pThis->m_wndList.GetItemText(a_lParam2, 0, bstr2.m_str);
	return _wcsicmp(bstr1, bstr2);

	//for (CSortSpec::const_iterator i = pThis->m_cSortSpec.begin(); i != pThis->m_cSortSpec.end(); ++i)
	//{
	//	switch (i->iCol)
	//	{
	//	case 0: // name
	//	case 1: // type
	//	case 6: // path
	//		{
	//			CComBSTR bstr1;
	//			pThis->m_wndList.GetItemText(a_lParam1, i->iCol, bstr1.m_str);
	//			CComBSTR bstr2;
	//			pThis->m_wndList.GetItemText(a_lParam2, i->iCol, bstr2.m_str);
	//			int nCmp = _wcsicmp(bstr1, bstr2);
	//			if (nCmp)
	//				return i->iOrd == 'A' ? nCmp : -nCmp;
	//		}
	//		break;
	//	case 2: // size
	//		{
	//			SListItemInfo* p1 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam1));
	//			SListItemInfo* p2 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam2));
	//			if (p1->nSize != p2->nSize)
	//				return (p1->nSize < p2->nSize) == (i->iOrd == 'A') ? -1 : 1;
	//		}
	//		break;
	//	case 3: // modified
	//		{
	//			SListItemInfo* p1 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam1));
	//			SListItemInfo* p2 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam2));
	//			if (p1->nModified != p2->nModified)
	//				return (p1->nModified < p2->nModified) == (i->iOrd == 'A') ? -1 : 1;
	//		}
	//		break;
	//	case 4: // created
	//		{
	//			SListItemInfo* p1 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam1));
	//			SListItemInfo* p2 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam2));
	//			if (p1->nCreated != p2->nCreated)
	//				return (p1->nCreated < p2->nCreated) == (i->iOrd == 'A') ? -1 : 1;
	//		}
	//		break;
	//	case 5: // tags
	//		{
	//			SListItemInfo* p1 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam1));
	//			SListItemInfo* p2 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam2));
	//			if (p1->nRelevancy != p2->nRelevancy)
	//				return (p1->nRelevancy < p2->nRelevancy) == (i->iOrd == 'A') ? -1 : 1;
	//		}
	//		break;
	//	}
	//}
	//return 0;
}

STDMETHODIMP CStorageBrowserSQLite::SetThumbnail(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData, RECT const* UNREF(a_prcBounds), BSTR a_bstrInfo)
{
	try
	{
		if (m_bDestroyed || a_nSizeX != m_szThumbnail.cx || a_nSizeY != m_szThumbnail.cy)
			return E_FAIL;

		HICON h = IconFromThumbnail(a_nSizeX, a_nSizeY, a_pRGBAData);
		{
			ObjectLock cLock(this);
			if (!m_bDestroyed)
			{
				std::pair<HICON, BSTR> t;
				t.first = h;
				t.second = a_bstrInfo;
				CWindow::SendMessage(WM_THUMBNAILREADY, reinterpret_cast<WPARAM>(&t), reinterpret_cast<LPARAM>(a_pFile));
			}
		}
		DestroyIcon(h);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

LRESULT CStorageBrowserSQLite::OnThumbnailReady(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HICON hIcon = reinterpret_cast<std::pair<HICON, BSTR>*>(wParam)->first;
	BSTR bstrInfo = reinterpret_cast<std::pair<HICON, BSTR>*>(wParam)->second;
	IStorageFilter* pFilter = reinterpret_cast<IStorageFilter*>(lParam);
	{
		//ObjectLock cLock(this);
		int nItems = m_wndList.GetItemCount();
		for (int i = 0; i < nItems; ++i)
		{
			SListItemInfo* p = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(i));
			if (p && p->pLoc == pFilter)
			{
				if (bstrInfo)
					p->strInfo = bstrInfo;
				else
					p->strInfo.clear();
				int iImage = m_cThumbnails.AddIcon(hIcon);
				m_wndList.SetItem(i, 0, LVIF_IMAGE, NULL, iImage, 0, 0, 0);
				break;
			}
		}
	}
	return 0;
}


HICON CStorageBrowserSQLite::IconFromThumbnail(ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData)
{
	ULONG nMaskLineSize = (((a_nSizeX+7)>>3)+3)&~3;
	CAutoVectorPtr<BYTE> pIconRes(new BYTE[sizeof(BITMAPINFOHEADER)+a_nSizeX*a_nSizeY*4+nMaskLineSize*a_nSizeY]);
	BITMAPINFOHEADER* pBIH = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
	pBIH->biSize = sizeof*pBIH;
	pBIH->biWidth = a_nSizeX;
	pBIH->biHeight = a_nSizeY<<1;
	pBIH->biPlanes = 1;
	pBIH->biBitCount = 32;
	pBIH->biCompression = BI_RGB;
	pBIH->biSizeImage = a_nSizeX*a_nSizeY*4+nMaskLineSize*a_nSizeY;
	pBIH->biXPelsPerMeter = 0x8000;
	pBIH->biYPelsPerMeter = 0x8000;
	pBIH->biClrUsed = 0;
	pBIH->biClrImportant = 0;
	DWORD* pXOR = reinterpret_cast<DWORD*>(pBIH+1);
	for (ULONG y = 0; y < a_nSizeY; ++y)
		CopyMemory(pXOR+a_nSizeX*(a_nSizeY-y-1), a_pRGBAData+a_nSizeX*y, a_nSizeX*4);
	BYTE* pAND = reinterpret_cast<BYTE*>(pXOR+a_nSizeX*a_nSizeY);
	// create mask
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		BYTE* pA = pAND+nMaskLineSize*y;
		DWORD* pC = pXOR+a_nSizeX*y;
		for (ULONG x = 0; x < a_nSizeX; ++x, ++pC)
		{
			BYTE* p = pA+(x>>3);
			if (*pC&0xff000000)
				*p &= ~(0x80 >> (x&7));
			else
				*p |= 0x80 >> (x&7);
		}
	}

	return CreateIconFromResourceEx(pIconRes, sizeof(BITMAPINFOHEADER)+a_nSizeX*a_nSizeY*4+nMaskLineSize*a_nSizeY, TRUE, 0x00030000, a_nSizeX, a_nSizeY, LR_DEFAULTCOLOR);
}

LRESULT CStorageBrowserSQLite::OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	RevokeDragDrop(m_wndList);

	m_pCallback = NULL;
	m_pListener = NULL;
	{
		m_cCmdsLock.Lock();
		m_bCmdsValid = false;
		m_pCmds = NULL;
		m_cCmdsLock.Unlock();
	}

	int nItems = m_wndList.GetItemCount();
	for (int i = 0; i < nItems; ++i)
	{
		SListItemInfo* pInfo = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(i));
		if (pInfo && pInfo->pLoc)
			pInfo->pLoc->Release();
		delete pInfo;
	}
	m_wndList.DeleteAllItems();
	a_bHandled = FALSE;
	return 0;
}

LRESULT CStorageBrowserSQLite::OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCallback != NULL)
		m_pCallback->ForwardOK();

	return 0;
}

LRESULT CStorageBrowserSQLite::OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCallback != NULL)
		m_pCallback->ForwardCancel();

	return 0;
}

#include "ExportDlg.h"

LRESULT CStorageBrowserSQLite::OnFileExport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_iActiveItem < 0)
		return 0;
	SListItemInfo* pItem = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(m_iActiveItem));
	if (pItem == NULL)
		return 0;

	CComBSTR bstrName;
	m_wndList.GetItemText(m_iActiveItem, 0, bstrName.m_str);
	CComPtr<IStorageFilter> pLoc;
	CExportDlg cDlg(m_tLocaleID, &pLoc, bstrName, IDD_EXPORTFILE, CFGID_STOREFILE);
	if (cDlg.DoModal(m_hWnd) == IDOK && pLoc)
	{
		try
		{
			CComPtr<IDataSrcDirect> pSrc;
			pItem->pLoc->SrcOpen(&pSrc);
			ULONG nSize = 0;
			pSrc->SizeGet(&nSize);
			CDirectInputLock cLock(pSrc, nSize);

			CComPtr<IDataDstStream> pDst;
			pLoc->DstOpen(&pDst);
			pDst->Write(nSize, cLock);
			pDst->Close();

			CComQIPtr<IStorageLocatorAttrs> pSrcAttr(pItem->pLoc);
			CComQIPtr<IStorageLocatorAttrs> pDstAttr(pLoc);
			ULONGLONG nTime = 0;
			if (pSrcAttr && pDstAttr && SUCCEEDED(pSrcAttr->GetTime(ESTTCreation, &nTime)))
				pDstAttr->SetTime(ESTTCreation, nTime);
		}
		catch (...)
		{
		}
		// TODO: report errors
	}
	return 0;
}

LRESULT CStorageBrowserSQLite::OnFileDelete(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_iActiveItem >= 0)
	{
		SListItemInfo* pItem = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(m_iActiveItem));
		if (pItem == NULL)
			return 0;

		TCHAR szTempl[128] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CONFIRMDELETE, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
		TCHAR szName[MAX_PATH] = _T("");
		m_wndList.GetItemText(m_iActiveItem, 0, szName, MAX_PATH);
		TCHAR szMsg[MAX_PATH+128] = _T("");
		_stprintf(szMsg, szTempl, szName);
		TCHAR szCaption[128] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_SQLITEFILTERNAME, szCaption, itemsof(szCaption), LANGIDFROMLCID(m_tLocaleID));
		if (MessageBox(szMsg, szCaption, MB_YESNO) == IDYES)
		{
			{
				// delete file
				char szQuery[128];
				sprintf(szQuery, "DELETE FROM files WHERE id=%lli", pItem->nUID);
				CSQLiteStatement cStmt(m_cDb, szQuery);
				sqlite3_step(cStmt);
			}
			{
				// delete tags
				char szQuery[128];
				sprintf(szQuery, "DELETE FROM tags WHERE fid=%lli", pItem->nUID);
				CSQLiteStatement cStmt(m_cDb, szQuery);
				sqlite3_step(cStmt);
			}
		}
		m_iActiveItem = -1;
		InitTagCloud();
		UpdateListedItems();
		// TODO: report errors
	}
	return 0;
}

LRESULT CStorageBrowserSQLite::OnFileRename(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_iActiveItem >= 0)
	{
		m_wndList.EnsureVisible(m_iActiveItem, FALSE);
		m_wndList.EditLabel(m_iActiveItem);
	}
	return 0;
}

LRESULT CStorageBrowserSQLite::OnShowThumbnails(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	bool bTiles = m_wndList.GetView() == LV_VIEW_DETAILS;
	m_wndList.SetView(bTiles ? LV_VIEW_TILE : LV_VIEW_DETAILS);
	SetConfigValue(CComBSTR(CFGID_SQL_USETHUMBNAILS), CConfigValue(bTiles));
	return 0;
}

LRESULT CStorageBrowserSQLite::OnShowGroups(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	bool bGroups = !m_wndList.IsGroupViewEnabled();
	m_wndList.EnableGroupView(bGroups);
	SetConfigValue(CComBSTR(CFGID_SQL_USEGROUPS), CConfigValue(bGroups));
	return 0;
}

LRESULT CStorageBrowserSQLite::OnCustomDraw(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	NMLVCUSTOMDRAW* p = reinterpret_cast<NMLVCUSTOMDRAW*>(a_pNMHeader);
	switch (p->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
	case CDDS_PREERASE:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		{
			LVITEM tLVI;
			tLVI.mask = LVIF_IMAGE|LVIF_PARAM;
			tLVI.iItem = p->nmcd.dwItemSpec;
			tLVI.iSubItem = 0;
			if (m_wndList.GetItem(&tLVI) && tLVI.iImage == I_IMAGENONE)
			{
				SListItemInfo* pInfo = reinterpret_cast<SListItemInfo*>(tLVI.lParam);
				if (!pInfo->bImageRequested)
				{
					pInfo->bImageRequested = true;
					m_pThumbnails->PrepareThumbnail(pInfo->pLoc, m_tLocaleID, this);
				}
			}
			SelectObject(p->nmcd.hdc, m_cFontSmall);
		}
		return CDRF_NEWFONT;
	default:
		return CDRF_DODEFAULT;
	}
}

COLORREF CStorageBrowserSQLite::BlendColors(COLORREF a_clr1, COLORREF a_clr2, ULONG a_nWeight1)
{
	if (a_nWeight1 >= 256)
		return a_clr1;
	ULONG const nWeight2 = 256-a_nWeight1;
	return RGB(
		(GetRValue(a_clr1)*a_nWeight1+GetRValue(a_clr2)*nWeight2)>>8,
		(GetGValue(a_clr1)*a_nWeight1+GetGValue(a_clr2)*nWeight2)>>8,
		(GetBValue(a_clr1)*a_nWeight1+GetBValue(a_clr2)*nWeight2)>>8);
}

#include "DragDropSource.h"

LRESULT CStorageBrowserSQLite::OnBeginDrag(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	// delete last dragged file from temp folder
	if (!m_strLastDraggedFile.empty())
	{
		DeleteFile(m_strLastDraggedFile.c_str());
		m_strLastDraggedFile.clear();
	}

	NMLISTVIEW* pNMLV = reinterpret_cast<NMLISTVIEW*>(a_pNMHeader);

	CComBSTR bstrName;
	m_wndList.GetItemText(pNMLV->iItem, 0, bstrName.m_str);
	TCHAR szTemp[MAX_PATH+MAX_PATH] = _T("");
	_tcscpy(szTemp+GetTempPath(MAX_PATH, szTemp), CW2CT(bstrName));
	HANDLE h = CreateFile(szTemp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return 0; // cannot create file in temp...bad luck
	SListItemInfo* pInfo = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(pNMLV->iItem));
	CComPtr<IDataSrcDirect> pSrc;
	pInfo->pLoc->SrcOpen(&pSrc);
	if (pSrc == NULL)
	{
		CloseHandle(h);
		return 0; // problem with source file
	}
	ULONG nSize = 0;
	pSrc->SizeGet(&nSize);
	{
		CDirectInputLock cLock(pSrc, nSize);
		DWORD dw = 0;
		WriteFile(h, cLock.operator const BYTE *(), nSize, &dw, NULL);
		CloseHandle(h);
	}
	pSrc = NULL;

	CComObject<CDragDropSource>* pDropSrc = NULL;
	CComObject<CDragDropSource>::CreateInstance(&pDropSrc);
	CComPtr<IDropSource> pTmp = pDropSrc;

	// Init the drag/drop data object.
	CWindow wndFrame = m_hWnd;
	while (wndFrame.m_hWnd && wndFrame.GetStyle() & WS_CHILD)
		wndFrame = wndFrame.GetParent();

	if (!pDropSrc->Init(szTemp, RegisterClipboardFormat(_T("RWDND_SELFBLOCK")), wndFrame))
        return 0;   // do nothing

	{
		// add reference to original file location (for targets that only link to dragged files, like 3D layer)
		CComPtr<IStorageFilter> flt;
		FilterCreate(pNMLV->iItem, &flt);
		CComBSTR bstr;
		if (flt) flt->ToText(NULL, &bstr);
		if (bstr.Length())
		{
			FORMATETC fetc = { RegisterClipboardFormat(_T("MEDLIBLINK")), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM stg = { TYMED_HGLOBAL };
			stg.hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (bstr.Length()+1)*sizeof(wchar_t));
			CopyMemory(GlobalLock(stg.hGlobal), bstr.m_str, (bstr.Length()+1)*sizeof(wchar_t));
			GlobalUnlock(stg.hGlobal);
			pDropSrc->SetData(&fetc, &stg, TRUE);
		}
	}

	// On 2K+, init a drag source helper object that will do the fancy drag
	// image when the user drags into Explorer (or another target that supports
	// the drag/drop helper).
	CComPtr<IDragSourceHelper> pdsh;
	HRESULT hr = pdsh.CoCreateInstance ( CLSID_DragDropHelper );
	if (pdsh)
		pdsh->InitializeFromWindow(m_wndList, &pNMLV->ptAction, pDropSrc);

    // Start the drag/drop!
	DWORD dwEffect = DROPEFFECT_NONE;
	hr = ::DoDragDrop(pDropSrc, pDropSrc, /*DROPEFFECT_MOVE|*/DROPEFFECT_COPY, &dwEffect);
	if (dwEffect == DROPEFFECT_NONE)
	{
		// clean up? ...is it really correct?
	}

	if (FAILED(hr) || dwEffect == DROPEFFECT_NONE)
	{
		DeleteFile(szTemp);
	}
	else
	{
		m_strLastDraggedFile = szTemp;
	}

    return 0;
}


STDMETHODIMP CStorageBrowserSQLite::DragEnter(IDataObject* a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
{
	try
	{
		STGMEDIUM stgm;
		stgm.tymed = TYMED_HGLOBAL;
		stgm.hGlobal = NULL;
		stgm.pUnkForRelease = NULL;
		FORMATETC fmtetc = {RegisterClipboardFormat(_T("RWDND_SELFBLOCK")), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		if (SUCCEEDED(a_pDataObj->GetData(&fmtetc, &stgm)))
		{
			HWND h = *reinterpret_cast<HWND*>(GlobalLock(stgm.hGlobal));
			CWindow wndFrame = m_hWnd;
			while (wndFrame.m_hWnd && wndFrame.GetStyle() & WS_CHILD)
				wndFrame = wndFrame.GetParent();
			ReleaseStgMedium(&stgm);
			if (h == wndFrame)
			{
				// cannot drop on self
				m_dwLastDropEffect = *a_pdwEffect = DROPEFFECT_NONE;
				return S_OK;
			}
		}
		std::vector<std::tstring> cFiles;
		FilesFromDrop(a_pDataObj, cFiles);
		if (cFiles.empty())
		{
			m_dwLastDropEffect = *a_pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
		m_dwLastDropEffect = *a_pdwEffect = DROPEFFECT_COPY;
		if (GetDropHelper())
		{
			POINT tPt = {a_pt.x, a_pt.y};
			GetDropHelper()->DragEnter(m_hWnd, a_pDataObj, &tPt, *a_pdwEffect);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageBrowserSQLite::DragOver(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
{
	try
	{
		*a_pdwEffect = m_dwLastDropEffect;
		if (GetDropHelper())
		{
			POINT tPt = {a_pt.x, a_pt.y};
			GetDropHelper()->DragOver(&tPt, *a_pdwEffect);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageBrowserSQLite::DragLeave()
{
	m_dwLastDropEffect = DROPEFFECT_NONE;
	if (GetDropHelper())
		GetDropHelper()->DragLeave();
	return S_OK;
}

STDMETHODIMP CStorageBrowserSQLite::Drop(IDataObject *a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
{
	m_dwLastDropEffect = DROPEFFECT_NONE;
	POINT tPt = {a_pt.x, a_pt.y};
	if (GetDropHelper())
		GetDropHelper()->Drop(a_pDataObj, &tPt, *a_pdwEffect);
	STGMEDIUM stgm;
	stgm.tymed = TYMED_HGLOBAL;
	stgm.hGlobal = NULL;
	stgm.pUnkForRelease = NULL;
	FORMATETC fmtetc = {RegisterClipboardFormat(_T("RWDND_SELFBLOCK")), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	if (SUCCEEDED(a_pDataObj->GetData(&fmtetc, &stgm)))
	{
		HWND h = *reinterpret_cast<HWND*>(GlobalLock(stgm.hGlobal));
		CWindow wndFrame = m_hWnd;
		while (wndFrame.m_hWnd && wndFrame.GetStyle() & WS_CHILD)
			wndFrame = wndFrame.GetParent();
		ReleaseStgMedium(&stgm);
		if (h == wndFrame)
		{
			// cannot drop on self
			m_dwLastDropEffect = *a_pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
	}
	std::vector<std::tstring> cFiles;
	FilesFromDrop(a_pDataObj, cFiles);
	for (std::vector<std::tstring>::const_iterator i = cFiles.begin(); i != cFiles.end(); ++i)
	{
		try
		{
			CStorageFilter cLoc(i->c_str());
			IStorageFilter* pLoc = cLoc;
			CComBSTR bstrName;
			pLoc->ToText(NULL, &bstrName);
			OLECHAR const* p1 = wcsrchr(bstrName, '\\');
			OLECHAR const* p2 = wcsrchr(bstrName, '/');
			OLECHAR const* pszName = p1 > p2 ? p1+1 : (p2 ? p2+1 : bstrName);

			CStorageSQLite::CTags cTags;

			CComPtr<IDataSrcDirect> pSrc;
			pLoc->SrcOpen(&pSrc);
			ULONG nSize = 0;
			pSrc->SizeGet(&nSize);
			CDirectInputLock cLock(pSrc, nSize);

			CComObject<CStorageSQLite>* pDstLoc = NULL;
			CComObject<CStorageSQLite>::CreateInstance(&pDstLoc);
			CComPtr<IStorageFilter> pDst2 = pDstLoc;
			pDstLoc->Init(m_szDatabase.c_str(), pszName, L"", cTags);
			CComPtr<IDataDstStream> pDst;
			pDstLoc->DstOpen(&pDst);
			pDst->Write(nSize, cLock);
			pDst->Close();

			CComQIPtr<IStorageLocatorAttrs> pSrcAttr(pLoc);
			ULONGLONG nTime = 0;
			if (pSrcAttr && SUCCEEDED(pSrcAttr->GetTime(ESTTCreation, &nTime)))
				pDstLoc->SetTime(ESTTCreation, nTime);
			AddItemToList(pDstLoc->M_UID(), GRPID_OTHER, 0, i == cFiles.begin(), true);
		}
		catch (...)
		{
		}
	}
	return S_OK;
}

#include "FilePropertiesDlg.h"

LRESULT CStorageBrowserSQLite::OnFileTagsModify(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_iActiveItem < 0)
		return 0;
	SListItemInfo* pItem = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(m_iActiveItem));
	if (pItem == NULL)
		return 0;
	TCHAR szName[MAX_PATH] = _T("");
	m_wndList.GetItemText(m_iActiveItem, 0, szName, itemsof(szName));
	TCHAR szTags[512] = _T("");
	m_wndList.GetItemText(m_iActiveItem, 5, szTags, itemsof(szTags));
	TCHAR szCrtd[128] = _T("");
	int nLen = Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_DATECRE, szCrtd, itemsof(szCrtd)-3, LANGIDFROMLCID(m_tLocaleID));
	szCrtd[nLen++] = _T(':');
	szCrtd[nLen++] = _T(' ');
	m_wndList.GetItemText(m_iActiveItem, 4, szCrtd+nLen, itemsof(szCrtd)-nLen);
	TCHAR szDate[128] = _T("");
	nLen = Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_DATEMOD, szDate, itemsof(szDate)-3, LANGIDFROMLCID(m_tLocaleID));
	szDate[nLen++] = _T(':');
	szDate[nLen++] = _T(' ');
	m_wndList.GetItemText(m_iActiveItem, 3, szDate+nLen, itemsof(szDate)-nLen);
	LVITEM tItem;
	tItem.mask = LVIF_IMAGE;
	tItem.iItem = m_iActiveItem;
	tItem.iSubItem = 0;
	tItem.iImage = 0;
	m_wndList.GetItem(&tItem);
	HICON hIcon = m_cThumbnails.GetIcon(tItem.iImage, ILD_IMAGE);
	std::set<std::wstring> cOldTags;
	ParseTags(szTags, cOldTags);

	CFilePropertiesDlg cDlg(m_tLocaleID, szName, szCrtd, szDate, szTags, itemsof(szTags), pItem->strNote, hIcon);
	if (cDlg.DoModal(m_hWnd) == IDOK)
	{
		m_wndList.SetItemText(m_iActiveItem, 5, szTags);
		bool bFailed;
		{
			// update note
			char szQuery[128];
			sprintf(szQuery, "UPDATE files SET note=? WHERE id=%lli", pItem->nUID);
			CSQLiteStatement cStmt(m_cDb, szQuery);
			if (!pItem->strNote.empty())
				sqlite3_bind_text16(cStmt, 1, pItem->strNote.c_str(), -1, SQLITE_STATIC/*SQLITE_TRANSIENT*/);
			bFailed = sqlite3_step(cStmt) != SQLITE_DONE;
		}
		{
			// update tags
			std::set<std::wstring> cNewTags;
			ParseTags(szTags, cNewTags);
			std::vector<std::wstring> cToAdd;
			std::set_difference(cNewTags.begin(), cNewTags.end(), cOldTags.begin(), cOldTags.end(), std::back_inserter(cToAdd));
			std::vector<std::wstring> cToDel;
			std::set_difference(cOldTags.begin(), cOldTags.end(), cNewTags.begin(), cNewTags.end(), std::back_inserter(cToDel));

			// delete old tags (TODO: optimize using ids fetched before)
			for (std::vector<std::wstring>::const_iterator iT = cToDel.begin(); iT != cToDel.end(); ++iT)
			{
				char szQuery[256];
				sprintf(szQuery, "DELETE FROM tags WHERE fid=%lli AND name=?", pItem->nUID);
				CSQLiteStatement cStmt(m_cDb, szQuery);
				sqlite3_bind_text16(cStmt, 1, iT->c_str(), -1, SQLITE_STATIC/*SQLITE_TRANSIENT*/);
				if (SQLITE_DONE != sqlite3_step(cStmt))
					bFailed = true;
			}

			// add new tags
			for (std::vector<std::wstring>::const_iterator iT = cToAdd.begin(); iT != cToAdd.end(); ++iT)
			{
				char szQuery[256];
				sprintf(szQuery, "INSERT INTO tags(fid,name) VALUES(%lli,?)", pItem->nUID);
				CSQLiteStatement cStmt(m_cDb, szQuery);
				sqlite3_bind_text16(cStmt, 1, iT->c_str(), -1, SQLITE_STATIC/*SQLITE_TRANSIENT*/);
				if (SQLITE_DONE != sqlite3_step(cStmt))
					bFailed = true;
			}
		}
		InitTagCloud();
		UpdateListedItems();
		// TODO: report errors
	}
	if (hIcon)
		DestroyIcon(hIcon);
	return 0;
}

LRESULT CStorageBrowserSQLite::OnFollowLink(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_iActiveItem < 0)
		return 0;
	SListItemInfo* pItem = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(m_iActiveItem));
	if (pItem == NULL)
		return 0;
	size_t i = pItem->strNote.find(L"http://");
	if (i == std::wstring::npos)
		return 0;
	size_t j = i+1;
	while (pItem->strNote[j] && pItem->strNote[j] != L' ' && pItem->strNote[j] != L'\r' && pItem->strNote[j] != L'\n' && pItem->strNote[j] != L'\t') ++j;
	std::wstring str = pItem->strNote.substr(i, j > i && (pItem->strNote[j-1] == L'.' || pItem->strNote[j-1] == L',') ? j-i-1 : j-i);
	::ShellExecute(NULL, _T("open"), CW2CT(str.c_str()), NULL, NULL, SW_SHOW);
	return 0;
}

//LRESULT CStorageBrowserSQLite::OnFilterChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
//{
//	int nCurFilter = m_wndFilter.GetCurSel();
//	if (nCurFilter < 0 || nCurFilter >= m_wndFilter.GetCount())
//		return 0;
//	IDocumentType* pActiveDocType  = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(nCurFilter));
//	if (m_pActiveDocType == pActiveDocType)
//		return 0;
//
//	m_pActiveDocType = pActiveDocType;
//	NotifyListenerDocumentChange();
//
//	if (m_pActiveDocType != NULL)
//	{
//		CComBSTR bstrUID;
//		m_pActiveDocType->UniqueIDGet(&bstrUID);
//		SetConfigValue(CComBSTR(CFGID_SQL_LASTFILTER), CConfigValue(bstrUID));
//	}
//
//	UpdateListedItems();
//
//	return 0;
//}

void CStorageBrowserSQLite::GetItemTextUTF8(UINT a_uID, CAutoVectorPtr<char>& a_pszName)
{
	CAutoVectorPtr<TCHAR> pszName;
	CWindow wnd = GetDlgItem(a_uID);
	int nLen = wnd.GetWindowTextLength()+1;
	pszName.Allocate(nLen);
	wnd.GetWindowText(pszName, nLen);
	pszName[nLen-1] = _T('\0');
	USES_CONVERSION;
	wchar_t const* psz = T2CW(pszName.m_p);
	int nLen2 = WideCharToMultiByte(CP_UTF8, 0, psz, -1, NULL, 0, NULL, NULL);
	if (nLen2 == 0)
		return;
	a_pszName.Attach(new char[nLen2]);
	WideCharToMultiByte(CP_UTF8, 0, psz, -1, a_pszName.m_p, nLen2, NULL, NULL);
}

void CStorageBrowserSQLite::WStrToUTF8(wchar_t const* a_pszIn, CAutoVectorPtr<char>& a_pszOut)
{
	int nLen2 = WideCharToMultiByte(CP_UTF8, 0, a_pszIn, -1, NULL, 0, NULL, NULL);
	if (nLen2 == 0)
		return;
	a_pszOut.Attach(new char[nLen2]);
	WideCharToMultiByte(CP_UTF8, 0, a_pszIn, -1, a_pszOut.m_p, nLen2, NULL, NULL);
}

void CStorageBrowserSQLite::UpdateListedItems(LONGLONG a_nSelected)
{
	static int nExtraGroupFlags = XPGUI::IsVista() ? LVGS_COLLAPSIBLE : 0;
	m_wndList.SetRedraw(FALSE);
	LONGLONG nPrevSel = 0;
	bool bPrevSelKept = false;
	BOOL bGroupView = m_wndList.IsGroupViewEnabled();
	if (a_nSelected)
	{
		nPrevSel = a_nSelected;
	}
	else if (m_iActiveItem >= 0)
	{
		SListItemInfo* p = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(m_iActiveItem));
		if (p)
			nPrevSel = p->nUID;
	}
	int nItems = m_wndList.GetItemCount();
	for (int i = 0; i < nItems; ++i)
	{
		SListItemInfo* pInfo = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(i));
		if (pInfo && pInfo->pLoc)
			pInfo->pLoc->Release();
		delete pInfo;
	}
	m_wndList.DeleteAllItems();
	m_cThumbnails.RemoveAll();
	m_wndList.RemoveAllGroups();

	if (m_cDb.ResCode() != SQLITE_OK)
		return;

	typedef std::map<std::wstring, std::vector<LONGLONG> > CMap;
	CMap cMap;
	for (CInitialTags::const_iterator i = m_cInitialTags.begin(); i != m_cInitialTags.end(); ++i)
	{
		std::vector<LONGLONG>* pUIDs = NULL;
		CSQLiteStatement cStmt(m_cDb, "SELECT fid FROM tags WHERE name=? ORDER BY fid ASC");
		sqlite3_bind_text16(cStmt, 1, i->c_str(), -1, SQLITE_STATIC);
		while (sqlite3_step(cStmt) == SQLITE_ROW)
		{
			if (pUIDs == NULL)
				pUIDs = &cMap[*i];
			pUIDs->push_back(sqlite3_column_int64(cStmt, 0));
		}
	}
	if (cMap.size() == 0)
	{
		// no keywords -> recently modified files and files with no tags
		std::set<LONGLONG> cNoTagsFiles;
		{
			CSQLiteStatement cStmt(m_cDb, "SELECT id FROM files WHERE id NOT IN (SELECT DISTINCT fid FROM tags)");
			while (sqlite3_step(cStmt) == SQLITE_ROW)
			{
				cNoTagsFiles.insert(sqlite3_column_int64(cStmt, 0));
			}
		}
		LVGROUP tGrp;
		ZeroMemory(&tGrp, sizeof tGrp);
		tGrp.cbSize = MY_LVGROUP_V5_SIZE;// sizeof(tGrp);
		tGrp.mask = LVGF_HEADER|LVGF_GROUPID|LVGF_ALIGN|LVGF_STATE;
		wchar_t szGrpName[128] = L"";
		Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_GROUP_RECENT, szGrpName, itemsof(szGrpName), LANGIDFROMLCID(m_tLocaleID));
		tGrp.pszHeader = szGrpName;
		tGrp.cchHeader = wcslen(tGrp.pszHeader);
		tGrp.iGroupId = GRPID_RECENT;
		tGrp.stateMask = LVGS_NORMAL;
		tGrp.state = LVGS_NORMAL|nExtraGroupFlags;
		tGrp.uAlign = LVGA_HEADER_LEFT;
		m_wndList.AddGroup(&tGrp);
		ULONG nInserted = 0;
		CSQLiteStatement cStmt(m_cDb, "SELECT id FROM files ORDER BY modified DESC LIMIT 0,250");
		while (sqlite3_step(cStmt) == SQLITE_ROW && nInserted < 12)
		{
			LONGLONG n = sqlite3_column_int64(cStmt, 0);
			if (cNoTagsFiles.find(n) == cNoTagsFiles.end() &&
				AddItemToList(n, GRPID_RECENT, 0, n == nPrevSel))
			{
				if (n == nPrevSel)
				{
					bPrevSelKept = true;
				}
				++nInserted;
			}
		}
		if (!cNoTagsFiles.empty())
		{
			Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_GROUP_NOTAGS, szGrpName, itemsof(szGrpName), LANGIDFROMLCID(m_tLocaleID));
			tGrp.iGroupId = GRPID_NOTAGS;
			m_wndList.AddGroup(&tGrp);
			for (std::set<LONGLONG>::const_iterator i = cNoTagsFiles.begin(); i != cNoTagsFiles.end(); ++i)
			{
				if (AddItemToList(*i, GRPID_NOTAGS, 0, *i == nPrevSel) && *i == nPrevSel)
				{
					bPrevSelKept = true;
				}
			}
		}
	}
	else
	{
		OLECHAR szGrpName[128] = L"";
		Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_COL_TAGS, szGrpName, itemsof(szGrpName), LANGIDFROMLCID(m_tLocaleID));
		wcscat(szGrpName, L":");
		std::map<std::wstring, int> cGrpIDs;
		while (!cMap.empty())
		{
			std::vector<CMap::iterator> cBest;
			LONGLONG nBest = -1;
			for (CMap::iterator i = cMap.begin(); i != cMap.end(); ++i)
			{
				LONGLONG n = i->second[i->second.size()-1];
				if (n > nBest)
				{
					nBest = n;
					cBest.clear();
					cBest.push_back(i);
				}
				else if (n == nBest)
				{
					cBest.push_back(i);
				}
			}
			std::wstring strGrp = szGrpName;
			for (size_t i = 0; i < cBest.size(); ++i)
			{
				strGrp.append(L" ");
				strGrp.append(cBest[i]->first);
			}
			std::map<std::wstring, int>::iterator iGrp = cGrpIDs.find(strGrp);
			int iGrpID;
			if (iGrp == cGrpIDs.end())
			{
				iGrpID = cGrpIDs.size()+GRPID_TAGSBASE;
				cGrpIDs[strGrp] = iGrpID;
				if (XPGUI::IsXP())
				{
					LVINSERTGROUPSORTED tGS;
					LVGROUP& tGrp = tGS.lvGroup;
					ZeroMemory(&tGrp, sizeof tGrp);
					tGrp.cbSize = MY_LVGROUP_V5_SIZE;// sizeof(tGrp);
					tGrp.mask = LVGF_HEADER|LVGF_GROUPID|LVGF_ALIGN|LVGF_STATE;
					tGrp.pszHeader = const_cast<LPWSTR>(strGrp.c_str());
					tGrp.cchHeader = wcslen(tGrp.pszHeader);
					tGrp.iGroupId = (cBest.size()<<24) | iGrpID;
					tGrp.stateMask = LVGS_NORMAL;
					tGrp.state = LVGS_NORMAL|nExtraGroupFlags;
					tGrp.uAlign = LVGA_HEADER_LEFT;
					tGS.pfnGroupCompare = LVGroupCompare;
					m_wndList.InsertGroupSorted(&tGS);
				}
			}
			else
			{
				iGrpID = iGrp->second;
			}
			if (AddItemToList(nBest, (cBest.size()<<24) | iGrpID, cBest.size(), nBest == nPrevSel) && nBest == nPrevSel)
			{
				bPrevSelKept = true;
			}
			for (std::vector<CMap::iterator>::iterator i = cBest.begin(); i != cBest.end(); ++i)
			{
				(*i)->second.resize((*i)->second.size()-1);
				if ((*i)->second.empty())
					cMap.erase(*i);
			}
		}
	}

	{
		// add group for unclassified files
		LVGROUP tGrp;
		ZeroMemory(&tGrp, sizeof tGrp);
		tGrp.cbSize = MY_LVGROUP_V5_SIZE;// sizeof(tGrp);
		tGrp.mask = LVGF_HEADER|LVGF_GROUPID|LVGF_ALIGN|LVGF_STATE;
		wchar_t szGrpName[128] = L"";
		Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_GROUP_MISC, szGrpName, itemsof(szGrpName), LANGIDFROMLCID(m_tLocaleID));
		tGrp.pszHeader = szGrpName;
		tGrp.cchHeader = wcslen(tGrp.pszHeader);
		tGrp.iGroupId = GRPID_OTHER;
		tGrp.stateMask = LVGS_NORMAL;
		tGrp.state = LVGS_NORMAL|nExtraGroupFlags;
		tGrp.uAlign = LVGA_HEADER_LEFT;
		m_wndList.AddGroup(&tGrp);
	}

	if (!bPrevSelKept && (a_nSelected || (nPrevSel && (m_dwFlags&EFTOpenExisting) == 0)))
	{
		AddItemToList(nPrevSel, GRPID_OTHER, 0, true, true);
	}

	if (bGroupView && XPGUI::IsXP() && !m_wndList.IsGroupViewEnabled())
		m_wndList.EnableGroupView(TRUE);

	m_wndList.SortItemsEx(CompareListItems, reinterpret_cast<LPARAM>(this));
	//m_iActiveItem = m_wndList.GetSelectedIndex();
	m_wndList.SetRedraw();
}

bool CStorageBrowserSQLite::AddItemToList(LONGLONG a_nID, int a_nGrp, int a_nMatching, bool a_bSelect, bool a_bOverrideFilter)
{
	char szQuery[256];
	sprintf(szQuery, "SELECT name,uid,created,modified,accessed,size,note FROM files WHERE id=%lli", a_nID);
	CSQLiteStatement cStmt(m_cDb, szQuery);

	if (sqlite3_step(cStmt) == SQLITE_ROW)
	{
		wchar_t const* psz = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0));
		int n = wcslen(psz);
		CAutoVectorPtr<wchar_t> pszName(new wchar_t[n+1]);
		CopyMemory(pszName.m_p, psz, n*sizeof(wchar_t));
		pszName[n] = L'\0';
		wchar_t const* pszExt = wcsrchr(pszName.m_p, L'.');
		if (pszExt) ++pszExt;

		CAutoPtr<SListItemInfo> pItem(new SListItemInfo);
		SListItemInfo* pItem2 = pItem;
		pItem->nUID = a_nID;
		pItem->pLoc = NULL;
		pItem->bImageRequested = false;
		psz = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 1));
		if (psz)
		{
			wcsncpy(pItem->szGUID, psz, 22);
			pItem->szGUID[22] = L'\0';
		}
		else
		{
			ATLASSERT(0); // invalid file GUID?
			pItem->szGUID[0] = L'\0';
		}
		pItem->nCreated = sqlite3_column_int64(cStmt, 2);
		pItem->nModified = sqlite3_column_int64(cStmt, 3);
		pItem->nAccessed = sqlite3_column_int64(cStmt, 4);
		pItem->nSize = sqlite3_column_int(cStmt, 5);
		pItem->nRelevancy = a_nMatching*21;
		psz = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 6));
		if (psz)
			pItem->strNote = psz;

		if (!a_bOverrideFilter && m_pActiveDocType && S_OK != m_pActiveDocType->MatchFilename(CComBSTR(pszName.m_p)))
			return false;
		int iItem = m_wndList.GetItemCount();

		CStorageSQLite::CTags cTags;
		std::wstring strTags;
		{
			// assemble tags
			char szQuery[256];
			sprintf(szQuery, "SELECT name FROM tags WHERE fid=%lli", a_nID);
			CSQLiteStatement cStmt2(m_cDb, szQuery);
			while (sqlite3_step(cStmt2) == SQLITE_ROW)
			{
				std::wstring str(reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt2, 0)));
				cTags.push_back(str);
				if (!strTags.empty())
					strTags.append(L" ");
				strTags.append(str);
				--pItem->nRelevancy;
				if (pItem->nRelevancy < 0)
					pItem->nRelevancy = 0;
			}
		}

		CComObject<CStorageSQLite>* pLoc = NULL;
		CComObject<CStorageSQLite>::CreateInstance(&pLoc);
		CComPtr<IStorageFilter> pLocator = pLoc;
		pLoc->Init(m_szDatabase.c_str(), pszName, pItem->szGUID, cTags);
		pItem->pLoc = pLocator;
		//ObjectLock cLock(this);
		CComBSTR bstrLoc;
		pItem->pLoc->ToText(NULL, &bstrLoc);
		LVITEM tLVI;
		ZeroMemory(&tLVI, sizeof tLVI);
		tLVI.mask = XPGUI::IsXP() ? LVIF_TEXT|LVIF_IMAGE|LVIF_GROUPID|LVIF_COLUMNS|LVIF_PARAM : LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		tLVI.iItem = iItem;
		CW2CT szName(pszName.m_p);
		tLVI.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(szName));
		tLVI.iImage = I_IMAGENONE;
		tLVI.iGroupId = a_nGrp;
		tLVI.lParam = reinterpret_cast<DWORD_PTR>(pItem.m_p);
		static UINT aCols[2] = {1, 2};
		tLVI.cColumns = itemsof(aCols);
		tLVI.puColumns = aCols;
		m_wndList.InsertItem(&tLVI);
		pItem.Detach();
		pLocator.Detach();
		m_wndList.SetItem(iItem, 6, LVIF_TEXT, CW2CT(bstrLoc), -1, 0, 0, 0);
		bool bExtSet = false;
		if (pszExt)
		{
			for (ULONG j = 0; j < m_nDocTypes && !bExtSet; ++j)
			{
				CComPtr<IDocumentType> pDocType;
				m_pDocTypes->Get(j, __uuidof(IDocumentType), reinterpret_cast<void**>(&pDocType));
				if (pDocType == NULL)
					continue;
				CComPtr<IEnumStrings> pExts;
				pDocType->SupportedExtensionsGet(&pExts);
				ULONG nExts = 0;
				if (pExts)
					pExts->Size(&nExts);
				for (ULONG k = 0; k < nExts && !bExtSet; ++k)
				{
					CComBSTR bstr;
					pExts->Get(k, &bstr);
					if (bstr == NULL)
						continue;
					if (_wcsicmp(bstr, pszExt) == 0)
					{
						CComPtr<ILocalizedString> pStr;
						pDocType->TypeNameGet(bstr, &pStr);
						if (pStr)
						{
							CComBSTR bstrDocType;
							pStr->GetLocalized(m_tLocaleID, &bstrDocType);
							m_wndList.SetItem(iItem, 1, LVIF_TEXT, CW2CT(bstrDocType), -1, 0, 0, 0);
							bExtSet = true;
						}
					}
				}
			}
		}
		if (!bExtSet)
			m_wndList.SetItem(iItem, 1, LVIF_TEXT, _T("N/A"), -1, 0, 0, 0);
		{
			// set size
			TCHAR szTmp[64];
			TCHAR szTempl[32] = _T("");
			if (pItem2->nSize < 1024)
			{
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_FILELEN_BYTES, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
				_stprintf(szTmp, szTempl, pItem2->nSize);
			}
			else if (pItem2->nSize < 1024*1024)
			{
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_FILELEN_KBYTES, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
				_stprintf(szTmp, szTempl, 0.01f*int(pItem2->nSize*100.0f/1024.0f));
			}
			else
			{
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_FILELEN_MBYTES, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
				_stprintf(szTmp, szTempl, 0.01f*int(pItem2->nSize*100.0f/(1024.0f*1024.0f)));
			}
			m_wndList.SetItem(iItem, 2, LVIF_TEXT, szTmp, -1, 0, 0, 0);
		}
		{
			// set dates
			SYSTEMTIME tSysTime;
			FileTimeToSystemTime(reinterpret_cast<FILETIME*>(&pItem2->nModified), &tSysTime);
			TCHAR szDate[128];
			if (GetDateFormat(m_tLocaleID, DATE_LONGDATE, &tSysTime, NULL, szDate, itemsof(szDate)) ||
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &tSysTime, NULL, szDate, itemsof(szDate)))
			{
				m_wndList.SetItem(iItem, 3, LVIF_TEXT, szDate, -1, 0, 0, 0); // modified date
			}
			FileTimeToSystemTime(reinterpret_cast<FILETIME*>(&pItem2->nCreated), &tSysTime);
			if (GetDateFormat(m_tLocaleID, DATE_LONGDATE, &tSysTime, NULL, szDate, itemsof(szDate)) ||
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &tSysTime, NULL, szDate, itemsof(szDate)))
			{
				m_wndList.SetItem(iItem, 4, LVIF_TEXT, szDate, -1, 0, 0, 0); // modified date
			}
		}
		// set tags
		m_wndList.SetItem(iItem, 5, LVIF_TEXT, CW2CT(strTags.c_str()), -1, 0, 0, 0); // tags

		if (a_bSelect)
			m_wndList.SelectItem(iItem);

		//if (XPGUI::IsXP())
		//{
		//	LVTILEINFO tTileInfo;
		//	ZeroMemory(&tTileInfo, sizeof tTileInfo);
		//	tTileInfo.cbSize = LVTILEINFO_V5_SIZE;//sizeof tTileInfo;
		//	tTileInfo.iItem = iItem;
		//	static UINT aCols[2] = {1, 2};
		//	tTileInfo.cColumns = itemsof(aCols);
		//	tTileInfo.puColumns = aCols;
		//	m_wndList.SetTileInfo(&tTileInfo);
		//}
		++iItem;
	}
	return true;
}

void CStorageBrowserSQLite::ParseTags(char* a_psz, CTags2& a_cTags)
{
	if (a_psz && a_psz[0])
	{
		char* pStart = NULL;
		for (char* p = a_psz; *p; ++p)
		{
			if (*p <= ' ')
			{
				*p = '\0';
				if (pStart)
				{
					a_cTags.insert(pStart);
					pStart = NULL;
				}
			}
			else
			{
				if (pStart == NULL)
					pStart = p;
			}
		}
		if (pStart && *pStart)
			a_cTags.insert(pStart);
	}
}

void CStorageBrowserSQLite::ParseTags(char const* a_psz, std::set<std::wstring>& a_cTags)
{
	ParseTags(CA2W(a_psz), a_cTags);
}

void CStorageBrowserSQLite::ParseTags(wchar_t const* a_psz, std::set<std::wstring>& a_cTags)
{
	if (a_psz && a_psz[0])
	{
		wchar_t const* pStart = NULL;
		wchar_t const* p;
		for (p = a_psz; *p; ++p)
		{
			if (*p <= ' ')
			{
				if (pStart)
				{
					a_cTags.insert(std::wstring(pStart, p));
					pStart = NULL;
				}
			}
			else
			{
				if (pStart == NULL)
					pStart = p;
			}
		}
		if (pStart && *pStart)
			a_cTags.insert(std::wstring(pStart, p));
	}
}
