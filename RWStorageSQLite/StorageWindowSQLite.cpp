// StorageWindowSQLite.cpp : Implementation of CStorageWindowSQLite

#include "stdafx.h"
#include "StorageSQLite.h"
#include "StorageWindowSQLite.h"

#include <XPGUI.h>
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <IconRenderer.h>

#ifdef WIN64
#define MY_LVGROUP_V5_SIZE 0x38
#else
#define MY_LVGROUP_V5_SIZE LVGROUP_V5_SIZE
#endif

// CStorageWindowSQLite

HRESULT CStorageWindowSQLite::Init(LPCOLESTR a_pszInitial, DWORD a_dwFlags, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, HWND a_hWnd, LCID a_tLocaleID)
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

bool CStorageWindowSQLite::AnalyzeLocator(LPCOLESTR a_pszName)
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

STDMETHODIMP CStorageWindowSQLite::FilterCreate(IStorageFilter** a_ppFilter)
{
	try
	{
		*a_ppFilter = NULL;

		if (m_dwFlags & EFTOpenExisting)
		{
			int iSel = m_wndList.GetSelectedIndex();
			if (iSel < 0 || iSel >= m_wndList.GetItemCount())
				return E_FAIL;
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
		else
		{
			if (m_cDb.ResCode() != SQLITE_OK)
				return E_FAIL;

			// file name
			CComBSTR bstrName;
			m_wndName.GetWindowText(&bstrName);
			int nName = bstrName.Length();
			if (nName >= 2 && bstrName[0] == L'\"' && bstrName[nName-1] == L'\"')
			{
				wcscpy(bstrName.m_str, bstrName.m_str+1);
				bstrName[nName-2] = '\0';
				nName -= 2;
			}
			else if (wcschr(bstrName, L'.') == NULL && m_pActiveDocType)
			{
				CComBSTR bstrDefExt;
				m_pActiveDocType->DefaultExtensionGet(&bstrDefExt);
				if (bstrDefExt != NULL && bstrDefExt[0])
				{
					CComBSTR bstrName2(nName+bstrDefExt.Length()+2);
					wcscpy(bstrName2.m_str, bstrName);
					bstrName2[nName] = L'.';
					wcscpy(bstrName2.m_str+nName+1, bstrDefExt);
					std::swap(bstrName2.m_str, bstrName.m_str);
				}
			}

			// tags
			CComBSTR bstrTags;
			m_wndTags.GetWindowText(&bstrTags);
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

			// GUID
			wchar_t szGUID[23] = L"";
			bool bNewRevision = m_wndNewRevision.GetCheck() == BST_CHECKED;
			if (bNewRevision)
			{
				GUID tGUID;
				CoCreateGuid(&tGUID);
				BYTE const* p = reinterpret_cast<BYTE const*>(&tGUID);
				Base64Encode(p, p+16, szGUID);
				szGUID[22] = L'\0';
			}
			else if (m_iActiveItem >= 0)
			{
				SListItemInfo* pInfo = reinterpret_cast<SListItemInfo*>(m_wndList.GetItemData(m_iActiveItem));
				if (pInfo)
					wcscpy(szGUID, pInfo->szGUID);
			}

			CComObject<CStorageSQLite>* p = NULL;
			CComObject<CStorageSQLite>::CreateInstance(&p);
			CComPtr<IStorageFilter> pTmp = p;
			p->Init(m_szDatabase.c_str(), bstrName, szGUID, cTags);

			*a_ppFilter = pTmp.Detach();
			return S_OK;
		}
	}
	catch (...)
	{
		return a_ppFilter == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageWindowSQLite::FiltersCreate(IEnumUnknowns** a_ppFilters)
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

class ATL_NO_VTABLE CTagsAutoComplete :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IEnumString
{
public:
	~CTagsAutoComplete()
	{
		for (std::vector<wchar_t*>::iterator i = m_cTags.begin(); i != m_cTags.end(); ++i)
		{
			delete[] *i;
		}
	}
	void Init(wchar_t const* a_pszDB)
	{
		m_nActive = 0;

		CSQLiteWrapper pDb(a_pszDB);
		if (pDb.ResCode() != SQLITE_OK)
			return;

		char szQuery[256] = "SELECT name,COUNT(*) FROM tags GROUP BY name ORDER BY COUNT(*) DESC";

		sqlite3_stmt* pStm = NULL;
		char const* pTail = NULL;
		int resCode = sqlite3_prepare(pDb, szQuery, -1, &pStm, &pTail);
		resCode = sqlite3_step(pStm);
		while (resCode == SQLITE_ROW)
		{
			int nLen = sqlite3_column_bytes16(pStm, 0)>>1;
			CAutoVectorPtr<wchar_t> pName(new wchar_t[nLen+1]);
			pName[nLen] = L'\0';
			//wchar_t* pSrc = sqlite3_column_text16(pStm, 1);
			CopyMemory(pName.m_p, sqlite3_column_text16(pStm, 0), nLen*sizeof(wchar_t));
			//sqlite3_free(pSrc);
			m_cTags.push_back(pName);
			pName.Detach();
			resCode = sqlite3_step(pStm);
		}
		resCode = sqlite3_finalize(pStm);
	}

BEGIN_COM_MAP(CTagsAutoComplete)
	COM_INTERFACE_ENTRY(IEnumString)
END_COM_MAP()

	// IEnumString methods
public:
	STDMETHOD(Next)(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
	{
		*pceltFetched = 0;
		while (celt > 0 && m_nActive < m_cTags.size())
		{
			wchar_t* psz = m_cTags[m_nActive++];
			LPOLESTR p2 = reinterpret_cast<LPOLESTR>(CoTaskMemAlloc((wcslen(psz)+1)*sizeof(wchar_t)));
			wcscpy(p2, psz);
			*(rgelt++) = p2;
			++*pceltFetched;
			--celt;
		}
		return celt == 0 ? S_OK : S_FALSE;
	}
	STDMETHOD(Skip)(ULONG celt)
	{
		if ((celt+m_nActive) <= m_cTags.size())
		{
			m_nActive += celt;
			return S_OK;
		}
		else
		{
			m_nActive = m_cTags.size();
			return S_FALSE;
		}
	}
	STDMETHOD(Reset)()
	{
		m_nActive = 0;
		return S_OK;
	}
	STDMETHOD(Clone)(IEnumString **ppenum)
	{
		CComObject<CTagsAutoComplete>* p = NULL;
		CComObject<CTagsAutoComplete>::CreateInstance(&p);
		CComPtr<IEnumString> pTmp = p;
		CTagsAutoComplete* p2 = p;
		p2->m_nActive = m_nActive;
		p2->m_cTags.reserve(m_cTags.size());
		for (std::vector<wchar_t*>::iterator i = m_cTags.begin(); i != m_cTags.end(); ++i)
		{
			wchar_t* psz = new wchar_t[wcslen(*i)+1];
			wcscpy(psz, *i);
			p2->m_cTags.push_back(psz);
		}

		*ppenum = pTmp.Detach();
		return S_OK;
	}

private:
	ULONG m_nActive;
	std::vector<wchar_t*> m_cTags;
};

void CStorageWindowSQLite::InitFiltersCombo(IEnumUnknowns* a_pFormatFilters)
{
	CComPtr<IDocumentTypeComposed> pAllSup;
	RWCoCreateInstance(pAllSup, __uuidof(DocumentTypeComposed));
	pAllSup->InitAsAllSupportedFiles();
	IDocumentType* pDefType = NULL;
	if (a_pFormatFilters != NULL)
	{
		ULONG nSize = 0;
		a_pFormatFilters->Size(&nSize);
		for (ULONG i = 0; i < nSize; ++i)
		{
			CComPtr<IDocumentType> pDT;
			a_pFormatFilters->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pDT));
			if (pDT != NULL)
			{
				m_cDocTypes.push_back(pDT);
				pAllSup->DocTypesAddFromList(1, &(pDT.p));
				if (m_nDefDocType == i)
					pDefType = pDT;
			}
		}
	}
	if (m_cDocTypes.size() > 1)
	{
		std::sort(m_cDocTypes.begin(), m_cDocTypes.end(), localized_doctype_compare(m_tLocaleID));
		m_cDocTypes.insert(m_cDocTypes.begin(), pAllSup.p);
	}
	{
		CComPtr<IDocumentTypeComposed> pAll;
		RWCoCreateInstance(pAll, __uuidof(DocumentTypeComposed));
		m_pActiveDocType = pAll;
		pAll->InitAsAllFiles();
		m_cDocTypes.push_back(m_pActiveDocType);
	}
	// fill filters combo box
	int nIconSize = XPGUI::GetSmallIconSize();
	m_hFilterImages.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 4, 4);
	m_wndFilter.SetImageList(m_hFilterImages);
	COMBOBOXEXITEM tItem;
	tItem.mask = CBEIF_TEXT|CBEIF_IMAGE|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	tItem.iItem = 0;
	for (CDocTypes::const_iterator i = m_cDocTypes.begin(); i != m_cDocTypes.end(); ++i)
	{
		CComPtr<ILocalizedString> pLocName;
		(*i)->FilterNameGet(&pLocName);
		CComBSTR bstrName;
		if (pLocName != NULL)
			pLocName->GetLocalized(m_tLocaleID, &bstrName);
		if (bstrName == NULL)
			bstrName = L"";
		CW2T strName(bstrName);
		tItem.pszText = strName;

		HICON hIcon = NULL;
		(*i)->IconGet(0, nIconSize, &hIcon);
		if (hIcon)
		{
			m_hFilterImages.AddIcon(hIcon);
			DestroyIcon(hIcon);
			tItem.iImage = tItem.iSelectedImage = m_hFilterImages.GetImageCount()-1;
		}
		else
		{
			tItem.iImage = tItem.iSelectedImage = -1;
		}

		tItem.lParam = reinterpret_cast<INT_PTR>(i->p);
		m_wndFilter.InsertItem(&tItem);
		++tItem.iItem;
		//m_wndFilter.SetItemDataPtr(m_wndFilter.AddString(CW2T(bstrName)), i->p);
	}
	m_wndFilter.EnableWindow(m_cDocTypes.size() > 1);

	// set active filter
	if (m_wndFilter.GetCount() > 0)
	{
		// priority: default DocType in listener, (TODO: extension in name), last used filter
		if (pDefType)
		{
			int i = 0;
			while (i < m_wndFilter.GetCount())
			{
				IDocumentType* pType = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(i));
				if (pType == pDefType)
				{
					m_pActiveDocType = pType;
					break;
				}
				++i;
			}
			i %= m_wndFilter.GetCount();
			m_wndFilter.SetCurSel(i);
			m_pActiveDocType = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(i));
			NotifyListenerDocumentChange();
		}
		else
		{
			int i = 0;
			CConfigValue cLastFilter;
			GetConfigValue(CComBSTR(CFGID_SQL_LASTFILTER), &cLastFilter);
			if (cLastFilter.TypeGet() == ECVTString)
			{
				while (i < m_wndFilter.GetCount())
				{
					IDocumentType* pType = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(i));
					CComBSTR bstrUID;
					pType->UniqueIDGet(&bstrUID);
					if (bstrUID != NULL && cLastFilter.operator BSTR() != NULL && wcscmp(bstrUID, cLastFilter) == 0)
					{
						m_pActiveDocType = pType;
						break;
					}
					++i;
				}
			}
			i %= m_wndFilter.GetCount();
			m_wndFilter.SetCurSel(i);
			m_pActiveDocType = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(i));
			NotifyListenerDocumentChange();
		}
	}
}

void CStorageWindowSQLite::InitTagCloud()
{
	if (m_cDb.ResCode() != SQLITE_OK)
		return;

	m_wndTagCloud.ResetContent();
	m_cTags.clear();

	std::set<std::wstring> cTagMap;
	for (CInitialTags::const_iterator i = m_cInitialTags.begin(); i != m_cInitialTags.end(); ++i)
		cTagMap.insert(*i);

	std::vector<int> cSel;
	{
		CSQLiteStatement cStmt(m_cDb, "SELECT name,COUNT(*) FROM tags GROUP BY name"); // ORDER BY COUNT(*) DESC
		std::vector<size_t> aCnts;
		while (sqlite3_step(cStmt) == SQLITE_ROW)
		{
			size_t nCnt = sqlite3_column_int64(cStmt, 1);
			wchar_t const* psz = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0));
			m_cTags.push_back(std::make_pair(std::wstring(psz), nCnt));
			aCnts.push_back(nCnt);
		}
		if (!m_cTags.empty())
		{
			std::sort(aCnts.begin(), aCnts.end());
			size_t nMedian = aCnts[aCnts.size()>>1];
			size_t nBorder = aCnts[(aCnts.size()*3)>>2];
			if (nBorder <= nMedian) nBorder = nMedian+1;
			size_t nTop = aCnts[(aCnts.size()*7)>>3];
			if (nTop <= nBorder) nTop = nBorder+1;
			m_aLimits[0] = nTop;
			m_aLimits[1] = nBorder;
			m_aLimits[2] = nMedian;
			m_aLimits[3] = nMedian-1;
			std::sort(m_cTags.begin(), m_cTags.end(), compare_tags_card(m_aLimits));

			int j = 0;
			for (CTags::const_iterator i = m_cTags.begin(); i != m_cTags.end(); ++i)
			{
				int iItem = m_wndTagCloud.AddString(LPCTSTR(j++));
				if (cTagMap.find(i->first) != cTagMap.end())
				{
					m_wndTagCloud.SetSel(iItem);
					cSel.push_back(iItem);
				}
			}
		}
	}
	m_nActiveTags = 0;
	m_aActiveTags.Free();
	if (cSel.size())
	{
		m_nActiveTags = cSel.size();
		m_aActiveTags.Allocate(m_nActiveTags);
		std::copy(cSel.begin(), cSel.end(), m_aActiveTags.m_p);
	}
}

void InitMenuItemIcons(IStockIcons* pSI, CImageList& cToolBar, int nIconSize, RECT padding)
{
	{
		static IRPathPoint const pointsDisk1[] =
		{
			{24, 157, 0, 0, 0, 0},
			{24, 215, 0, 22.6437, 0, -22.6437},
			{128, 256, 57.4376, 0, -57.4376, 0},
			{232, 215, 0, -22.6437, 0, 22.6437},
			{232, 157, 0, 0, 0, 0},
		};
		static IRPathPoint const pointsDisk2[] =
		{
			{24, 99, 0, 0, 0, 0},
			{24, 157, 0, 22.6437, 0, -22.6437},
			{128, 198, 57.4376, 0, -57.4376, 0},
			{232, 157, 0, -22.6437, 0, 22.6437},
			{232, 99, 0, 0, 0, 0},
		};
		static IRPathPoint const pointsDisk3[] =
		{
			{24, 41, 0, 0, 0, 0},
			{24, 99, 0, 22.6437, 0, -22.6437},
			{128, 140, 57.4376, 0, -57.4376, 0},
			{232, 99, 0, -22.6437, 0, 22.6437},
			{232, 41, 0, 0, 0, 0},
		};
		static IRPathPoint const pointsTop[] =
		{
			{232, 41, 0, -22.6437, 0, 22.6437},
			{128, 0, -57.4376, 0, 57.4376, 0},
			{24, 41, 0, 22.6437, 0, -22.6437},
			{128, 82, 57.4376, 0, -57.4376, 0},
		};
		static IRGridItem const grid[] = { {0, 24}, {0, 232} };
		static IRCanvas const canvas = {0, 0, 256, 256, 2, 0, grid, NULL};
		CIconRendererReceiver cRenderer(nIconSize);
		cRenderer(&canvas, itemsof(pointsDisk1), pointsDisk1, pSI->GetMaterial(ESMInterior));
		cRenderer(&canvas, itemsof(pointsDisk2), pointsDisk2, pSI->GetMaterial(ESMInterior));
		cRenderer(&canvas, itemsof(pointsDisk3), pointsDisk3, pSI->GetMaterial(ESMInterior));
		cRenderer(&canvas, itemsof(pointsTop), pointsTop, pSI->GetMaterial(ESMInterior));
		HICON hIcon = cRenderer.get(padding);
		cToolBar.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.875f));
		HICON hIcon = cRenderer.get(padding);
		cToolBar.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIFloppySimple, cRenderer, IRTarget(0.875f));
		HICON hIcon = cRenderer.get(padding);
		cToolBar.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIDelete, cRenderer, IRTarget(0.8f));
		HICON hIcon = cRenderer.get(padding);
		cToolBar.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		static IRPolyPoint const box[] =
		{
			{0, 80}, {256, 80}, {256, 176}, {0, 176},
		};
		static IRPolyPoint const cursor[] =
		{
			{152, 48}, {168, 48}, {168, 208}, {152, 208}, {152, 232}, {208, 232}, {208, 208}, {192, 208}, {192, 48}, {208, 48}, {208, 24}, {152, 24},
		};
		static IRGridItem const boxGridX[] = {{0, 0}, {0, 256}};
		static IRGridItem const boxGridY[] = {{0, 80}, {0, 176}};
		static IRCanvas const canvasBox = {0, 0, 256, 256, itemsof(boxGridX), itemsof(boxGridY), boxGridX, boxGridY};
		static IRGridItem const cursorGridX[] = {{0, 152}, {0, 168}, {0, 192}, {0, 208}};
		static IRGridItem const cursorGridY[] = {{0, 24}, {0, 48}, {0, 208}, {0, 232}};
		static IRCanvas const canvasCursor = {0, 0, 256, 256, itemsof(cursorGridX), itemsof(cursorGridY), cursorGridX, cursorGridY};

		CIconRendererReceiver cRenderer(nIconSize);
		cRenderer(&canvasBox, itemsof(box), box, pSI->GetMaterial(ESMInterior));
		cRenderer(&canvasCursor, itemsof(cursor), cursor, pSI->GetMaterial(ESMInterior));
		HICON hIcon = cRenderer.get(padding);
		cToolBar.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIProperties, cRenderer);
		HICON hIcon = cRenderer.get(padding);
		cToolBar.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIHelp, cRenderer);
		HICON hIcon = cRenderer.get(padding);
		cToolBar.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIDirectionUp, cRenderer, IRTarget(0.75f));
		HICON hIcon = cRenderer.get(padding);
		cToolBar.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIDirectionDown, cRenderer, IRTarget(0.75f));
		HICON hIcon = cRenderer.get(padding);
		cToolBar.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
}

void CStorageWindowSQLite::InitToolbar()
{
	static UINT const s_aStringIDs[] =
	{
		IDS_CMD_MANAGEDATABASE,
		//IDI_DATABASEPICK, IDS_CMD_PICKDATABASE,
		//IDI_DATABASEIMPORT, IDS_CMD_IMPORTDATABASE,
		//IDI_DATABASEEXPORT, IDS_CMD_EXPORTDATABASE,
		IDS_CMD_IMPORTFILE,
		IDS_CMD_EXPORTFILE,
		IDS_CMD_DELETEFILE,
		IDS_CMD_RENAMEFILE,
		IDS_CMD_MODIFYTAGS,
		IDS_CMD_HELP,
	};
	int nIconSize = XPGUI::GetSmallIconSize();
	ULONG nIconDelta = (nIconSize>>1)-8;
	ULONG nGapLeft = nIconDelta>>1;
	ULONG nGapRight = nIconDelta-nGapLeft;
	RECT padding = {nGapLeft, 0, nGapRight, 0};
	m_cToolBar.Create(nIconSize+nIconDelta, nIconSize, XPGUI::GetImageListColorFlags(), 9, 0);
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	InitMenuItemIcons(pSI, m_cToolBar, nIconSize, padding);

	TCHAR szToolBarTips[1024] = _T("");
	TCHAR* psz = szToolBarTips;
	for (size_t i = 0; i < itemsof(s_aStringIDs); i++)
	{
		int iLen = Win32LangEx::LoadString(_pModule->get_m_hInst(), s_aStringIDs[i], psz, itemsof(szToolBarTips)-(psz-szToolBarTips)-(itemsof(s_aStringIDs)>>1), LANGIDFROMLCID(m_tLocaleID));
		psz[iLen+1] = _T('\0');
		psz += iLen+1;
	}

	m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	m_wndToolBar.SetImageList(m_cToolBar);
	m_wndToolBar.AddStrings(szToolBarTips);
	TBBUTTON aButtons[] =
	{
		{4, ID_RENAMEFILE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 4},
		{5, ID_MODIFYTAGS, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 5},
		{3, ID_DELETEFILE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 3},
		{2, ID_EXPORTFILE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 2},
		{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
		{1, ID_IMPORTFILE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 1},
		{0, ID_MANAGEDATABASE, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, 0},
		{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
		{6, ID_ONLINEHELP, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 6},
		//{0, ID_PICKDATABASE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 0},
		//{1, ID_IMPORTDATABASE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 1},
		//{2, ID_EXPORTDATABASE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 2},
	};
	m_wndToolBar.AddButtons(itemsof(aButtons), aButtons);
	m_wndToolBar.SetButtonSize(nIconSize+8, nIconSize+(nIconSize>>1)+1);

	// align the toolbar to the right
	RECT rcActual;
	RECT rcDesired;
	m_wndToolBar.GetWindowRect(&rcActual);
	ScreenToClient(&rcActual);
	m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcDesired);

	RECT rcList;
	m_wndList.GetWindowRect(&rcList);
	ScreenToClient(&rcList);
	m_wndToolBar.MoveWindow(rcList.right-rcDesired.right, rcList.top-rcDesired.bottom, rcDesired.right, rcDesired.bottom-rcDesired.top, FALSE);
}

LRESULT CStorageWindowSQLite::OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	AddRef();

	m_wndList = GetDlgItem(IDC_FWSQL_LISTING);
	m_wndTagCloud = GetDlgItem(IDC_FWSQL_TAGCLOUD);
	m_wndFilter = GetDlgItem(IDC_FWSQL_TYPE);
	m_wndName = GetDlgItem(IDC_FWSQL_NAME);
	m_wndTags = GetDlgItem(IDC_FWSQL_KEYWORDS);
	m_wndNewRevision = GetDlgItem(IDC_FWSQL_NEWREVISION);
	m_wndToolBar = GetDlgItem(IDC_FWSQL_TOOLBAR);

	RegisterDragDrop(m_wndList, this);

	if (m_dwFlags & EFTOpenExisting)
	{
		m_wndName.ShowWindow(FALSE);
		m_wndTags.ShowWindow(FALSE);
		m_wndNewRevision.ShowWindow(FALSE);
		GetDlgItem(IDC_FWSQL_TAGSLABEL).ShowWindow(FALSE);
		GetDlgItem(IDC_FWSQL_NAMELABEL).ShowWindow(FALSE);
		CStatic wndFilterLabel = GetDlgItem(IDC_FWSQL_TYPELABEL);
		RECT rcFilter;
		m_wndFilter.GetWindowRect(&rcFilter);
		ScreenToClient(&rcFilter);
		RECT rcItems;
		m_wndList.GetWindowRect(&rcItems);
		ScreenToClient(&rcItems);
		RECT rcTagCloud;
		m_wndTagCloud.GetWindowRect(&rcTagCloud);
		ScreenToClient(&rcTagCloud);
		RECT rcName;
		m_wndName.GetWindowRect(&rcName);
		ScreenToClient(&rcName);
		RECT rc;
		wndFilterLabel.GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.left += rcItems.left-rcTagCloud.left;
		rc.right += rcItems.left-rcTagCloud.left;
		wndFilterLabel.MoveWindow(&rc, FALSE);
		rc.left = rcFilter.left+rcItems.left-rcTagCloud.left;
		rc.right = rcItems.right;
		rc.top = rcFilter.top;
		rc.bottom = rcFilter.bottom;
		m_wndFilter.MoveWindow(&rc, FALSE);
		rc.left = rcTagCloud.left;
		rc.right = rcTagCloud.right;
		rc.top = rcTagCloud.top;
		rc.bottom = rcFilter.bottom;
		m_wndTagCloud.MoveWindow(&rc, FALSE);
		rc.left = rcItems.left;
		rc.right = rcItems.right;
		rc.top = rcItems.top;
		rc.bottom = rcName.bottom;
		m_wndList.MoveWindow(&rc, FALSE);
	}
	else
	{
		if (!m_szInitialFileName.empty())
		{
			m_wndName.SetWindowText(CW2CT(m_szInitialFileName.c_str()));
		}
		if (!m_cInitialTags.empty())
		{
			std::tstring str;
			for (CInitialTags::const_iterator i = m_cInitialTags.begin(); i != m_cInitialTags.end(); ++i)
			{
				if (!str.empty())
					str.append(_T(" "));
				str.append(CW2CT(i->c_str()));
			}
			m_wndTags.SetWindowText(str.c_str());
		}
	}

	InitToolbar();
	m_wndList.GetHeader().SetImageList(m_cToolBar);//HDSIL_NORMAL HDSIL_STATE

	InitFiltersCombo(reinterpret_cast<IEnumUnknowns*>(a_lParam));

	// init thumbnails
	HDC hdc = GetDC();
	int nScale = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(hdc);
	m_szThumbnail.cx = ((XPGUI::IsXP() ? 64 : 21) * nScale + 48) / 96;
	m_szThumbnail.cy = ((XPGUI::IsXP() ? 48 : 16) * nScale + 48) / 96;
	m_cThumbnails.Create(m_szThumbnail.cx, m_szThumbnail.cy, XPGUI::GetImageListColorFlags(), 16, 8);
	if (XPGUI::IsXP()) m_wndList.SetImageList(m_cThumbnails, /*XPGUI::IsXP() ? */LVSIL_NORMAL/* : LVSIL_SMALL*/);

	m_wndList.SetExtendedListViewStyle(LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT|(XPGUI::IsXP() ? LVS_EX_DOUBLEBUFFER|LVS_EX_BORDERSELECT|LVS_EX_JUSTIFYCOLUMNS|LVS_EX_HEADERINALLVIEWS|LVS_EX_COLUMNOVERFLOW|LVS_EX_AUTOSIZECOLUMNS : 0));
	if (XPGUI::IsVista() && CTheme::IsThemingSupported())
	{
		::SetWindowTheme(m_wndList, L"explorer", NULL);
	}

	TCHAR szColumn[128] = _T("");
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM|LVCF_IMAGE;
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = szColumn;
	lvc.cx = 200*nScale/96;
	lvc.iSubItem = 0;
	lvc.iImage = m_cSortSpec.size() && m_cSortSpec.front().iCol == lvc.iSubItem ? (m_cSortSpec.front().iOrd == 'A' ? 7 : 8) : I_IMAGENONE;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_NAME, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	m_wndList.InsertColumn(0, &lvc);
	lvc.iSubItem++;
	lvc.iImage = m_cSortSpec.size() && m_cSortSpec.front().iCol == lvc.iSubItem ? (m_cSortSpec.front().iOrd == 'A' ? 7 : 8) : I_IMAGENONE;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_TYPE, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 150*nScale/96;
	m_wndList.InsertColumn(1, &lvc);
	lvc.iSubItem++;
	lvc.iImage = m_cSortSpec.size() && m_cSortSpec.front().iCol == lvc.iSubItem ? (m_cSortSpec.front().iOrd == 'A' ? 7 : 8) : I_IMAGENONE;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_SIZE, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 100*nScale/96;
	m_wndList.InsertColumn(2, &lvc);
	lvc.iSubItem++;
	lvc.iImage = m_cSortSpec.size() && m_cSortSpec.front().iCol == lvc.iSubItem ? (m_cSortSpec.front().iOrd == 'A' ? 7 : 8) : I_IMAGENONE;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_DATEMOD, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 150*nScale/96;
	m_wndList.InsertColumn(3, &lvc);
	lvc.iSubItem++;
	lvc.iImage = m_cSortSpec.size() && m_cSortSpec.front().iCol == lvc.iSubItem ? (m_cSortSpec.front().iOrd == 'A' ? 7 : 8) : I_IMAGENONE;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_DATECRE, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 150*nScale/96;
	m_wndList.InsertColumn(4, &lvc);
	lvc.iSubItem++;
	lvc.iImage = m_cSortSpec.size() && m_cSortSpec.front().iCol == lvc.iSubItem ? (m_cSortSpec.front().iOrd == 'A' ? 7 : 8) : I_IMAGENONE;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_TAGS, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 250*nScale/96;
	m_wndList.InsertColumn(5, &lvc);
	lvc.iSubItem++;
	lvc.iImage = m_cSortSpec.size() && m_cSortSpec.front().iCol == lvc.iSubItem ? (m_cSortSpec.front().iOrd == 'A' ? 7 : 8) : I_IMAGENONE;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COL_PATH, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 500*nScale/96;
	m_wndList.InsertColumn(6, &lvc);
	CHeaderCtrl wndHdr = m_wndList.GetHeader();
	HDITEM tHD;
	tHD.mask = HDI_FORMAT;
	for (int i = 0; wndHdr.GetItem(i, &tHD); ++i)
	{
		tHD.fmt |= HDF_BITMAP_ON_RIGHT;
		wndHdr.SetItem(i, &tHD);
	}

	if (XPGUI::IsXP())
	{
		CConfigValue cVal;
		GetConfigValue(CComBSTR(CFGID_SQL_USETHUMBNAILS), &cVal);
		if (cVal)
			m_wndList.SetView(LV_VIEW_TILE);

		LVTILEVIEWINFO tTileInfo;
		ZeroMemory(&tTileInfo, sizeof(tTileInfo));
		tTileInfo.cbSize = sizeof(tTileInfo);
		tTileInfo.dwMask = LVTVIM_TILESIZE|LVTVIM_COLUMNS|LVTVIM_LABELMARGIN;
		tTileInfo.dwFlags = LVTVIF_AUTOSIZE;//LVTVIF_FIXEDWIDTH;
		tTileInfo.sizeTile.cx = (m_szThumbnail.cx<<2);//+(m_szThumbnail.cx>>1);
		tTileInfo.sizeTile.cy = m_szThumbnail.cy;
		tTileInfo.cLines = 2;
		tTileInfo.rcLabelMargin.left = 5;
		tTileInfo.rcLabelMargin.right = 5;
		tTileInfo.rcLabelMargin.top = 3;
		tTileInfo.rcLabelMargin.bottom = 3;
		m_wndList.SetTileViewInfo(&tTileInfo);

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
		lf.lfHeight = l;
		lf.lfWeight += 300; if (lf.lfWeight > FW_HEAVY) lf.lfWeight = FW_HEAVY;
		m_cFontBold.CreateFontIndirect(&lf);
	}

	// keywords autocomplete
	//CComObject<CTagsAutoComplete>* pTAC = NULL;
	//CComObject<CTagsAutoComplete>::CreateInstance(&pTAC);
	//CComPtr<IEnumString> pTAC2 = pTAC;
	//pTAC->Init(m_cDb);

	//CComPtr<IAutoComplete> pACInit;
	//RWCoCreateInstance(pACInit, CLSID_AutoComplete);
	//if (pACInit != NULL)
	//{
	//	pACInit->Init(GetDlgItem(IDC_FWSQL_KEYWORDS), pTAC2, NULL, NULL);
	//}
	//CComQIPtr<IAutoComplete2> pACInit2(pACInit);
	//if (pACInit2 != NULL)
	//{
	//	pACInit2->SetOptions(ACO_AUTOSUGGEST|ACO_AUTOAPPEND|ACO_UPDOWNKEYDROPSLIST);
	//}

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

	InitTagCloud();

	UpdateListedItems(nInitialUID);

	UpdateToolbarFile();
	//UpdateToolbarDatabase();

	DlgResize_Init(false, false, 0);

	return TRUE;
}

LRESULT CStorageWindowSQLite::OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_dwFlags & EFTOpenExisting)
		GotoDlgCtrl(m_wndList);
	else
		GotoDlgCtrl(m_wndName);
	return 0;
}

LRESULT CStorageWindowSQLite::OnItemActivateList(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(a_pNMHDR);

	m_iActiveItem = pNMIA->iItem;

	if (m_pCallback != NULL)
		m_pCallback->ForwardOK();

	return 0;
}

LRESULT CStorageWindowSQLite::OnItemChangedList(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
{
	int iSel = m_wndList.GetSelectedIndex();
	if (m_iActiveItem != iSel)
	{
		m_iActiveItem = iSel;
		UpdateToolbarFile();
		if ((m_dwFlags&EFTOpenExisting) == 0)
		{
			if (m_iActiveItem >= 0)
			{
				TCHAR szTmp[MAX_PATH] = _T("");
				m_wndList.GetItemText(m_iActiveItem, 0, szTmp, MAX_PATH);
				szTmp[itemsof(szTmp)-1] = _T('\0');
				m_wndName.SetWindowText(szTmp);
				m_wndList.GetItemText(m_iActiveItem, 5, szTmp, MAX_PATH);
				szTmp[itemsof(szTmp)-1] = _T('\0');
				m_wndTags.SetWindowText(szTmp);
				m_wndNewRevision.EnableWindow(TRUE);
			}
			else
			{
				m_wndNewRevision.EnableWindow(FALSE);
			}
		}
	}

	return 0;
}

void CStorageWindowSQLite::UpdateToolbarFile()
{
	m_wndToolBar.EnableButton(ID_DELETEFILE, m_iActiveItem != -1);
	m_wndToolBar.EnableButton(ID_RENAMEFILE, m_iActiveItem != -1);
	m_wndToolBar.EnableButton(ID_MODIFYTAGS, m_iActiveItem != -1);
	m_wndToolBar.EnableButton(ID_EXPORTFILE, m_iActiveItem != -1);
}

//void CStorageWindowSQLite::UpdateToolbarDatabase()
//{
//	m_wndToolBar.EnableButton(ID_IMPORTFILE, m_cDb.ResCode() == SQLITE_OK);
//	m_wndToolBar.EnableButton(ID_IMPORTDATABASE, m_cDb.ResCode() == SQLITE_OK);
//	m_wndToolBar.EnableButton(ID_EXPORTDATABASE, m_cDb.ResCode() == SQLITE_OK);
//}

LRESULT CStorageWindowSQLite::OnGetInfoTipList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(a_pNMHDR);
	//TCHAR szPath[MAX_PATH] = _T("");
	//m_wndList.GetItemText(pGetInfoTip->iItem, 6, szPath, itemsof(szPath));
	TCHAR szTags[MAX_PATH] = _T("");
	m_wndList.GetItemText(pGetInfoTip->iItem, 5, szTags, itemsof(szTags));
	//TCHAR szCrtd[64] = _T("");
	//m_wndList.GetItemText(pGetInfoTip->iItem, 4, szCrtd, itemsof(szCrtd));
	TCHAR szDate[64] = _T("");
	m_wndList.GetItemText(pGetInfoTip->iItem, 3, szDate, itemsof(szDate));

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
			_sntprintf(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, _T("%s: %s\n%s: %s"), szTagsName, szTags, szDateName, szDate);
		}
		else
		{
			CW2CT cInfo(pItem->strInfo.c_str());
			_sntprintf(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, _T("%s: %s\n%s: %s\n%s"), szTagsName, szTags, szDateName, szDate, static_cast<LPCTSTR>(cInfo));
		}
	}
	else
	{
		if (pItem->strInfo.empty())
		{
			CW2CT cNote(pItem->strNote.c_str());
			_sntprintf(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, _T("%s: %s\n%s: %s\n-----\n%s"), szTagsName, szTags, szDateName, szDate, static_cast<LPCTSTR>(cNote));
		}
		else
		{
			CW2CT cNote(pItem->strNote.c_str());
			CW2CT cInfo(pItem->strInfo.c_str());
			_sntprintf(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, _T("%s: %s\n%s: %s\n%s\n-----\n%s"), szTagsName, szTags, szDateName, szDate, static_cast<LPCTSTR>(cInfo), static_cast<LPCTSTR>(cNote));
		}
	}

	return 0;
}

LRESULT CStorageWindowSQLite::OnGetEmptyMarkup(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	NMLVEMPTYMARKUP* markupInfo = reinterpret_cast<NMLVEMPTYMARKUP*>(a_pNMHDR);
	markupInfo->dwFlags = EMF_CENTERED;
	Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_NOMATCHINGFILES, markupInfo->szMarkup, itemsof(markupInfo->szMarkup), LANGIDFROMLCID(m_tLocaleID));
	return TRUE; // set the markup
}

LRESULT CStorageWindowSQLite::OnKeyDown(int UNREF(a_nCtrlID), LPNMHDR a_pNMHeader, BOOL& a_bHandled)
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

LRESULT CStorageWindowSQLite::OnRClick(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	NMITEMACTIVATE* pItem = reinterpret_cast<NMITEMACTIVATE*>(a_pNMHeader);

	Reset(m_cToolBar);
	CMenu cMenu;
	cMenu.CreatePopupMenu();
	TCHAR szTmp[256] = _T("");

	CMenu cSort;
	cSort.CreatePopupMenu();
	LVCOLUMN tCol;
	ZeroMemory(&tCol, sizeof tCol);
	TCHAR szCol[256] = _T("");
	szCol[255] = _T('\0');
	tCol.mask = LVCF_TEXT|LVCF_SUBITEM|LVCF_IMAGE;
	tCol.pszText = szCol;
	tCol.cchTextMax = 255;
	for (int i = 0; m_wndList.GetColumn(i, &tCol); ++i)
	{
		AddItem(i+ID_SORTBASE, szCol, tCol.iImage);
		cSort.AppendMenu(tCol.iImage >= 0 ? MFT_OWNERDRAW|MFT_RADIOCHECK|MFS_CHECKED : MFT_OWNERDRAW, i+ID_SORTBASE, LPCTSTR(NULL));
	}

	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_SORTBY, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
	AddItem(reinterpret_cast<UINT>(cSort.operator HMENU()), szTmp, -1);
	cMenu.AppendMenu(MFT_OWNERDRAW|MF_POPUP, cSort, LPCTSTR(NULL));
	if (XPGUI::IsXP())
	{
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_THUMBNAILS, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_SHOWTHUMBNAILS, szTmp, -1);
		cMenu.AppendMenu(m_wndList.GetView() == LV_VIEW_TILE ? MFT_OWNERDRAW|MFS_CHECKED : MFT_OWNERDRAW, ID_SHOWTHUMBNAILS, LPCTSTR(NULL));
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_GROUPITEMS, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_GROUPITEMS, szTmp, -1);
		cMenu.AppendMenu(m_wndList.IsGroupViewEnabled() ? MFT_OWNERDRAW|MFS_CHECKED : MFT_OWNERDRAW, ID_GROUPITEMS, LPCTSTR(NULL));
	}

	if (pItem->iItem >= 0)
	{
		cMenu.AppendMenu(MFT_SEPARATOR);
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

	}
	POINT tPt = pItem->ptAction;
	m_wndList.ClientToScreen(&tPt);
	cMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, tPt.x, tPt.y, m_hWnd, NULL);
	return 0;
}

LRESULT CStorageWindowSQLite::OnBeginLabelEdit(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
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

LRESULT CStorageWindowSQLite::OnEndLabelEdit(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
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

LRESULT CStorageWindowSQLite::OnColumnClick(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	NMLISTVIEW* pItem = reinterpret_cast<NMLISTVIEW*>(a_pNMHDR);
	static char const s_aDefSort[] = {'A', 'A', 'A', 'D', 'D', 'D', 'A'};
	if (pItem->iSubItem < 0 || ULONG(pItem->iSubItem) >= itemsof(s_aDefSort))
		return 0; // something went wrong
	LVCOLUMN tCol;
	ZeroMemory(&tCol, sizeof tCol);
	tCol.mask = LVCF_IMAGE;
	if (!m_cSortSpec.empty())
	{
		if (m_wndList.GetColumn(m_cSortSpec.front().iCol, &tCol))
		{
			tCol.iImage = I_IMAGENONE;
			m_wndList.SetColumn(m_cSortSpec.front().iCol, &tCol);
		}
	}
	SSortItem t = {pItem->iSubItem, s_aDefSort[pItem->iSubItem]};
	for (CSortSpec::iterator i = m_cSortSpec.begin(); i != m_cSortSpec.end(); ++i)
		if (i->iCol == pItem->iSubItem)
		{
			t.iOrd = i == m_cSortSpec.begin() ? i->iOrd^('A'^'D') : i->iOrd;
			m_cSortSpec.erase(i);
			break;
		}
	m_cSortSpec.push_front(t);
	if (m_wndList.GetColumn(t.iCol, &tCol))
	{
		tCol.iImage = t.iOrd == 'A' ? 7 : 8;
		m_wndList.SetColumn(t.iCol, &tCol);
	}
	OLECHAR szTmp[32];
	LPOLESTR psz = szTmp;
	for (CSortSpec::iterator i = m_cSortSpec.begin(); i != m_cSortSpec.end(); ++i)
	{
		*(psz++) = L'0'+i->iCol;
		*(psz++) = i->iOrd;
	}
	SetConfigValue(CComBSTR(CFGID_SQL_LASTSORTSPEC), CConfigValue(szTmp));
	m_wndList.SortItemsEx(CompareListItems, reinterpret_cast<LPARAM>(this));
	//UpdateListedItems();
	return 0;
}

int __stdcall CStorageWindowSQLite::CompareListItems(LPARAM a_lParam1, LPARAM a_lParam2, LPARAM a_lParamSort)
{
	CStorageWindowSQLite* pThis = reinterpret_cast<CStorageWindowSQLite*>(a_lParamSort);
	for (CSortSpec::const_iterator i = pThis->m_cSortSpec.begin(); i != pThis->m_cSortSpec.end(); ++i)
	{
		switch (i->iCol)
		{
		case 0: // name
		case 1: // type
		case 6: // path
			{
				CComBSTR bstr1;
				pThis->m_wndList.GetItemText(a_lParam1, i->iCol, bstr1.m_str);
				CComBSTR bstr2;
				pThis->m_wndList.GetItemText(a_lParam2, i->iCol, bstr2.m_str);
				int nCmp = _wcsicmp(bstr1, bstr2);
				if (nCmp)
					return i->iOrd == 'A' ? nCmp : -nCmp;
			}
			break;
		case 2: // size
			{
				SListItemInfo* p1 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam1));
				SListItemInfo* p2 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam2));
				if (p1->nSize != p2->nSize)
					return (p1->nSize < p2->nSize) == (i->iOrd == 'A') ? -1 : 1;
			}
			break;
		case 3: // modified
			{
				SListItemInfo* p1 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam1));
				SListItemInfo* p2 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam2));
				if (p1->nModified != p2->nModified)
					return (p1->nModified < p2->nModified) == (i->iOrd == 'A') ? -1 : 1;
			}
			break;
		case 4: // created
			{
				SListItemInfo* p1 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam1));
				SListItemInfo* p2 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam2));
				if (p1->nCreated != p2->nCreated)
					return (p1->nCreated < p2->nCreated) == (i->iOrd == 'A') ? -1 : 1;
			}
			break;
		case 5: // tags
			{
				SListItemInfo* p1 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam1));
				SListItemInfo* p2 = reinterpret_cast<SListItemInfo*>(pThis->m_wndList.GetItemData(a_lParam2));
				if (p1->nRelevancy != p2->nRelevancy)
					return (p1->nRelevancy < p2->nRelevancy) == (i->iOrd == 'A') ? -1 : 1;
			}
			break;
		}
	}
	return 0;
}

LRESULT CStorageWindowSQLite::OnColumnOverflowClick(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	NMLISTVIEW* pHdr = reinterpret_cast<NMLISTVIEW*>(a_pNMHDR);
	Reset(m_cToolBar);
	LVCOLUMN tCol;
	ZeroMemory(&tCol, sizeof tCol);
	TCHAR szCol[256] = _T("");
	szCol[255] = _T('\0');
	tCol.mask = LVCF_TEXT|LVCF_SUBITEM|LVCF_IMAGE;
	tCol.pszText = szCol;
	tCol.cchTextMax = 255;
	CMenu cMenu;
	cMenu.CreatePopupMenu();
	for (int i = pHdr->iSubItem; m_wndList.GetColumn(i, &tCol); ++i)
	{
		AddItem(i+ID_SORTBASE, szCol, tCol.iImage);
		cMenu.AppendMenu(tCol.iImage >= 0 ? MFT_OWNERDRAW|MFT_RADIOCHECK|MFS_CHECKED : MFT_OWNERDRAW, i+ID_SORTBASE, LPCTSTR(NULL));
	}
	TPMPARAMS t;
	t.cbSize = sizeof t;
	CHeaderCtrl wndHead = m_wndList.GetHeader();
	wndHead.SendMessage(HDM_GETOVERFLOWRECT, 0, reinterpret_cast<LPARAM>(&t.rcExclude));
	wndHead.ClientToScreen(&t.rcExclude);
	cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, t.rcExclude.left, t.rcExclude.bottom, m_hWnd, &t);
	return 0;
}

LRESULT CStorageWindowSQLite::OnSortByColumn(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	a_wID -= ID_SORTBASE;
	if (a_wID > 6)
		return 0;
	NMLISTVIEW t;
	t.iSubItem = a_wID;
	BOOL b;
	OnColumnClick(IDC_FWSQL_LISTING, &t.hdr, b);
	return 0;
}

STDMETHODIMP CStorageWindowSQLite::SetThumbnail(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData, RECT const* UNREF(a_prcBounds), BSTR a_bstrInfo)
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

LRESULT CStorageWindowSQLite::OnThumbnailReady(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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


HICON CStorageWindowSQLite::IconFromThumbnail(ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData)
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

LRESULT CStorageWindowSQLite::OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	RevokeDragDrop(m_wndList);

	m_pCallback = NULL;
	m_pListener = NULL;

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

LRESULT CStorageWindowSQLite::OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCallback != NULL)
		m_pCallback->ForwardOK();

	return 0;
}

LRESULT CStorageWindowSQLite::OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCallback != NULL)
		m_pCallback->ForwardCancel();

	return 0;
}

LRESULT CStorageWindowSQLite::OnButtonDropDown(WPARAM UNREF(a_wParam), LPNMHDR a_pNMHdr, BOOL& UNREF(a_bHandled))
{
	NMTOOLBAR* pTB = reinterpret_cast<NMTOOLBAR*>(a_pNMHdr);
	if (pTB->iItem >= ID_MANAGEDATABASE)
	{
		POINT ptBtn = {pTB->rcButton.left, pTB->rcButton.bottom};
		m_wndToolBar.ClientToScreen(&ptBtn);

		Reset(m_cToolBar);

		CMenu cMenu;
		cMenu.CreatePopupMenu();

		TCHAR szTmp[256] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_EXPORTDATABASE, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_EXPORTDATABASE, szTmp, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_EXPORTDATABASE, LPCTSTR(NULL));
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_IMPORTDATABASE, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_IMPORTDATABASE, szTmp, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_IMPORTDATABASE, LPCTSTR(NULL));
		cMenu.AppendMenu(MFT_SEPARATOR);
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_PICKDATABASE, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_PICKDATABASE, szTmp, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_PICKDATABASE, LPCTSTR(NULL));
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CMD_DEFAULTDATABASE, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_DEFAULTDATABASE, szTmp, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_DEFAULTDATABASE, LPCTSTR(NULL));

		TPMPARAMS tPMParams;
		ZeroMemory(&tPMParams, sizeof tPMParams);
		tPMParams.cbSize = sizeof tPMParams;
		tPMParams.rcExclude = pTB->rcButton;
		::MapWindowPoints(pTB->hdr.hwndFrom, NULL, reinterpret_cast<POINT*>(&tPMParams.rcExclude), 2);
		cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, ptBtn.x, ptBtn.y, m_hWnd, &tPMParams);
	}
	return TBDDRET_DEFAULT;
}

CComPtr<IEnumUnknowns> GetDatabaseFormatFilter()
{
	CComPtr<IEnumUnknownsInit> pTmp;
	RWCoCreateInstance(pTmp, __uuidof(EnumUnknowns));
	CComPtr<IDocumentTypeWildcards> pFF;
	RWCoCreateInstance(pFF, __uuidof(DocumentTypeWildcards));
	CComBSTR bstrRWDB(L"rwdb");
	pFF->InitEx(_SharedStringTable.GetStringAuto(IDS_FORMATFILTER_RWDB), NULL, 1, &(bstrRWDB.m_str), NULL, NULL, 0, CComBSTR(L"*.rwdb"));
	pTmp->Insert(pFF);
	return pTmp.p;
}

#include "DatabasePickDlg.h"

LRESULT CStorageWindowSQLite::OnDatabasePick(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	CComBSTR bstrLoc = m_szDatabase.c_str();
	CDatabasePickDlg cDlg(m_tLocaleID, bstrLoc);
	if (cDlg.DoModal(m_hWnd) == IDOK && m_szDatabase != bstrLoc.m_str)
	{
		CSQLiteWrapper cDb(bstrLoc);
		if (cDb.ResCode() != SQLITE_OK)
		{
			// TODO: report error
			return 0;
		}
		cDb.Swap(m_cDb);
		m_szDatabase = bstrLoc.m_str;
		InitTagCloud();
		UpdateListedItems();
		SetConfigValue(CComBSTR(CFGID_SQL_LASTDATABASE), CConfigValue(bstrLoc.m_str));
	}
	return 0;
}

LRESULT CStorageWindowSQLite::OnDatabaseUseDefault(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	std::wstring strDef;
	CStorageSQLite::GetDefaultDatabase(strDef);
	if (m_szDatabase != strDef)
	{
		CSQLiteWrapper cDb(strDef.c_str());
		if (cDb.ResCode() != SQLITE_OK)
		{
			// TODO: report error
			return 0;
		}
		cDb.Swap(m_cDb);
		std::swap(strDef, m_szDatabase);
		InitTagCloud();
		UpdateListedItems();
		SetConfigValue(CComBSTR(CFGID_SQL_LASTDATABASE), CConfigValue(L""));
	}
	return 0;
}

#include "DatabaseImportDlg.h"

LRESULT CStorageWindowSQLite::OnDatabaseImport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	EDBMergeMode eMode = EDMMNewer;
	CComPtr<IStorageFilter> pLoc;
	CDatabaseImportDlg cDlg(m_tLocaleID, &pLoc, &eMode);
	if (cDlg.DoModal(m_hWnd) != IDOK || pLoc == NULL)
		return 0;
	CComBSTR bstrLoc;
	pLoc->ToText(NULL, &bstrLoc);
	if (m_szDatabase == bstrLoc.m_str)
		return 0;
	CSQLiteWrapper cImp(bstrLoc);
	if (cImp.ResCode() != SQLITE_OK)
		return 0;
	sqlite3_exec(m_cDb, "BEGIN EXCLUSIVE", NULL, NULL, NULL);
	try
	{
		if (eMode == EDMMReplace)
		{
			sqlite3_exec(m_cDb, "DELETE FROM files", NULL, NULL, NULL);
			sqlite3_exec(m_cDb, "DELETE FROM tags", NULL, NULL, NULL);
		}
		CSQLiteStatement cFImp(cImp, "SELECT uid,name,created,modified,accessed,data,size,note,id FROM files");
		while (sqlite3_step(cFImp) == SQLITE_ROW)
		{
			ULONGLONG nUID = 0;
			bool bSkip = false;
			char const* pszUID = reinterpret_cast<char const*>(sqlite3_column_text(cFImp, 0));
			if (eMode != EDMMReplace)
			{
				char szTmp[128] = "";
				sprintf(szTmp, "SELECT id,modified FROM files WHERE uid='%s'", pszUID);
				CSQLiteStatement cFCur(m_cDb, szTmp);
				if (sqlite3_step(cFCur) == SQLITE_ROW)
				{
					// conflict
					if (eMode == EDMMImported ||
						(eMode == EDMMNewer && sqlite3_column_int64(cFCur, 1) < sqlite3_column_int64(cFImp, 3)))
					{
						nUID = sqlite3_column_int64(cFCur, 0);
					}
					else
					{
						bSkip = true;
					}
				}
			}
			if (bSkip)
				continue;
			// update file
			ULONGLONG nCreated = sqlite3_column_int64(cFImp, 2);
			ULONGLONG nModified = sqlite3_column_int64(cFImp, 3);
			ULONGLONG nAccessed = sqlite3_column_int64(cFImp, 4);
			ULONG nSize = sqlite3_column_int(cFImp, 6);
			char szTmp[256];
			if (nUID)
			{
				// remove old tags (really?)
				sprintf(szTmp, "DELETE FROM tags WHERE fid=%lli", nUID);
				sqlite3_exec(m_cDb, szTmp, NULL, NULL, NULL);
				sprintf(szTmp, "UPDATE files SET uid='%s',name=?,created=%lli,modified=%lli,accessed=%lli,data=?,size=%i,note=? WHERE id=%lli", pszUID, nCreated, nModified, nAccessed, nSize, nUID);
			}
			else
			{
				sprintf(szTmp, "INSERT INTO files(uid,name,created,modified,accessed,data,size,note) VALUES('%s',?,%lli,%lli,%lli,?,%i,?)", pszUID, nCreated, nModified, nAccessed, nSize);
			}
			CSQLiteStatement cFUpd(m_cDb, szTmp);
			sqlite3_bind_text(cFUpd, 1, reinterpret_cast<char const*>(sqlite3_column_text(cFImp, 1)), -1, SQLITE_STATIC);
			void const* pData = sqlite3_column_blob(cFImp, 5);
			sqlite3_bind_blob(cFUpd, 2, pData, sqlite3_column_bytes(cFImp, 5), SQLITE_STATIC);
			sqlite3_bind_text(cFUpd, 3, reinterpret_cast<char const*>(sqlite3_column_text(cFImp, 7)), -1, SQLITE_STATIC);
			sqlite3_step(cFUpd);
			if (nUID == 0)
				nUID = sqlite3_last_insert_rowid(m_cDb);
			// add tags
			sprintf(szTmp, "SELECT name FROM tags WHERE fid=%lli", sqlite3_column_int64(cFImp, 8));
			CSQLiteStatement cTImp(cImp, szTmp);
			while (sqlite3_step(cTImp) == SQLITE_ROW)
			{
				sprintf(szTmp, "INSERT INTO tags(fid,name) VALUES(%lli,?)", nUID);
				CSQLiteStatement cTUpd(m_cDb, szTmp);
				sqlite3_bind_text(cTUpd, 1, reinterpret_cast<char const*>(sqlite3_column_text(cTImp, 0)), -1, SQLITE_STATIC);
				sqlite3_step(cTUpd);
			}
		}
	}
	catch (...)
	{
	}
	sqlite3_exec(m_cDb, "COMMIT", NULL, NULL, NULL);
	InitTagCloud();
	UpdateListedItems();
	// TODO: handle errors, update listed items and tags
	return 0;
}

#include "ExportDlg.h"

LRESULT CStorageWindowSQLite::OnDatabaseExport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_szDatabase.empty() || m_cDb.ResCode() != SQLITE_OK)
		return 0;

	CComBSTR bstrName = m_szDatabase.c_str();
	CComPtr<IStorageFilter> pLoc;
	CComPtr<IEnumUnknowns> pFilters = GetDatabaseFormatFilter();
	CExportDlg cDlg(m_tLocaleID, &pLoc, bstrName, IDD_EXPORTDATABASE, CFGID_STOREDATABASE, pFilters);
	if (cDlg.DoModal(m_hWnd) == IDOK && pLoc)
	{
		try
		{
			CComPtr<IDataDstStream> pDst;
			pLoc->DstOpen(&pDst);
			if (pDst)
			{
				sqlite3_exec(m_cDb, "BEGIN EXCLUSIVE", NULL, NULL, NULL);
				HANDLE h = INVALID_HANDLE_VALUE;
				try
				{
					h = CreateFile(CW2CT(m_szDatabase.c_str()), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
					if (h != INVALID_HANDLE_VALUE)
					{
						DWORD dw = GetFileSize(h, NULL);
						CAutoVectorPtr<BYTE> pTmp(new BYTE[min(0x10000, dw)]);
						while (dw)
						{
							DWORD dwRead = min(0x10000, dw);
							ReadFile(h, pTmp, dwRead, &dwRead, NULL);
							if (dwRead != min(0x10000, dw))
							{
								pDst->Close();
								break; // error
							}
							pDst->Write(dwRead, pTmp);
							dw -= dwRead;
						}
						pDst->Close();
					}
				}
				catch (...)
				{
				}
				if (h != INVALID_HANDLE_VALUE)
					CloseHandle(h);
				sqlite3_exec(m_cDb, "COMMIT", NULL, NULL, NULL);
			}
			// TODO: report errors
		}
		catch (...)
		{
		}
		// TODO: report errors
	}
	return 0;
}

#include "FileImportDlg.h"

LRESULT CStorageWindowSQLite::OnFileImport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	TCHAR szTags[512] = _T("");
	CComPtr<IStorageFilter> pLoc;
	CFileImportDlg cDlg(m_tLocaleID, &pLoc, szTags, itemsof(szTags));
	if (cDlg.DoModal(m_hWnd) == IDOK && pLoc)
	{
		try
		{
			CComBSTR bstrName;
			pLoc->ToText(NULL, &bstrName);
			OLECHAR const* p1 = wcsrchr(bstrName, '\\');
			OLECHAR const* p2 = wcsrchr(bstrName, '/');
			OLECHAR const* pszName = p1 > p2 ? p1+1 : (p2 ? p2+1 : bstrName);

			std::set<std::wstring> cTagSet;
			ParseTags(szTags, cTagSet);
			CStorageSQLite::CTags cTags;
			std::copy(cTagSet.begin(), cTagSet.end(), std::back_inserter(cTags));

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
			if (!cTags.empty())
				InitTagCloud();
			AddItemToList(pDstLoc->M_UID(), GRPID_OTHER, 0, true, true);
		}
		catch (...)
		{
		}
		// TODO: report errors
	}
	return 0;
}

LRESULT CStorageWindowSQLite::OnFileExport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
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

LRESULT CStorageWindowSQLite::OnFileDelete(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
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

LRESULT CStorageWindowSQLite::OnFileRename(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_iActiveItem >= 0)
	{
		m_wndList.EnsureVisible(m_iActiveItem, FALSE);
		m_wndList.EditLabel(m_iActiveItem);
	}
	return 0;
}

LRESULT CStorageWindowSQLite::OnOnlineHelp(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	ShellExecute(NULL, _T("open"), _T("http://www.rw-designer.com/tagged-storage"), NULL, NULL, SW_SHOW);
	return 0;
}

LRESULT CStorageWindowSQLite::OnShowThumbnails(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	bool bTiles = m_wndList.GetView() == LV_VIEW_DETAILS;
	m_wndList.SetView(bTiles ? LV_VIEW_TILE : LV_VIEW_DETAILS);
	SetConfigValue(CComBSTR(CFGID_SQL_USETHUMBNAILS), CConfigValue(bTiles));
	return 0;
}

LRESULT CStorageWindowSQLite::OnShowGroups(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	bool bGroups = !m_wndList.IsGroupViewEnabled();
	m_wndList.EnableGroupView(bGroups);
	SetConfigValue(CComBSTR(CFGID_SQL_USEGROUPS), CConfigValue(bGroups));
	return 0;
}

LRESULT CStorageWindowSQLite::OnCustomDraw(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	NMLVCUSTOMDRAW* p = reinterpret_cast<NMLVCUSTOMDRAW*>(a_pNMHeader);
	switch (p->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
	case CDDS_PREERASE:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		if (m_wndList.GetView() == LV_VIEW_TILE)
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
		}
		return CDRF_DODEFAULT;
	default:
		return CDRF_DODEFAULT;
	}
}

COLORREF CStorageWindowSQLite::BlendColors(COLORREF a_clr1, COLORREF a_clr2, ULONG a_nWeight1)
{
	if (a_nWeight1 >= 256)
		return a_clr1;
	ULONG const nWeight2 = 256-a_nWeight1;
	return RGB(
		(GetRValue(a_clr1)*a_nWeight1+GetRValue(a_clr2)*nWeight2)>>8,
		(GetGValue(a_clr1)*a_nWeight1+GetGValue(a_clr2)*nWeight2)>>8,
		(GetBValue(a_clr1)*a_nWeight1+GetBValue(a_clr2)*nWeight2)>>8);
}

LRESULT CStorageWindowSQLite::OnDrawItem(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	LPDRAWITEMSTRUCT pDrawItem = reinterpret_cast<LPDRAWITEMSTRUCT>(a_lParam);
	if (a_wParam == IDC_FWSQL_TAGCLOUD && pDrawItem->CtlID == IDC_FWSQL_TAGCLOUD && pDrawItem->itemID < UINT(m_wndTagCloud.GetCount()))
	{
		CDCHandle cDC(pDrawItem->hDC);
		RECT rcItem = {0, 0, 0, 0};
		m_wndTagCloud.GetItemRect(pDrawItem->itemID, &rcItem);
		cDC.FillRect(&rcItem, pDrawItem->itemState&ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW);
		COLORREF clrText = GetSysColor(pDrawItem->itemState&ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT);
		COLORREF clrBg = GetSysColor(pDrawItem->itemState&ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW);
		std::pair<std::wstring, size_t> const& tItem = m_cTags[pDrawItem->itemID];
		COLORREF clrItem = clrText;
		HFONT hOldFont = NULL;
		if (tItem.second >= m_aLimits[0])
		{
			hOldFont = cDC.SelectFont(m_cFontBold);
		}
		else if (tItem.second >= m_aLimits[1])
		{
		}
		else if (tItem.second >= m_aLimits[2])
		{
			clrItem = BlendColors(clrText, clrBg, m_cTags[pDrawItem->itemID].second > 2 ? 256 : 128);
		}
		else if (tItem.second >= m_aLimits[3])
		{
			hOldFont = cDC.SelectFont(m_cFontSmall);
		}
		else
		{
			hOldFont = cDC.SelectFont(m_cFontSmall);
			clrItem = BlendColors(clrText, clrBg, m_cTags[pDrawItem->itemID].second > 2 ? 256 : 128);
		}
		COLORREF clrOldText = cDC.SetTextColor(clrItem);
		COLORREF clrOldBg = cDC.SetBkColor(clrBg);
		cDC.DrawText(CW2CT(tItem.first.c_str()), -1, &rcItem, DT_VCENTER|DT_LEFT|DT_END_ELLIPSIS|DT_SINGLELINE);
		if (hOldFont)
			cDC.SelectFont(hOldFont);
		cDC.SetTextColor(clrOldText);
		cDC.SetBkColor(clrOldBg);
		if (tItem.second >= m_aLimits[1] &&
			m_cTags.size() > pDrawItem->itemID+1 &&
			m_cTags[pDrawItem->itemID+1].second < m_aLimits[1])
		{
			rcItem.top = rcItem.bottom-1;
			rcItem.left += 3; rcItem.right -= 3;
			cDC.FillRect(&rcItem, COLOR_WINDOWTEXT);
		}
		else if (tItem.second < m_aLimits[1] &&
			pDrawItem->itemID &&
			m_cTags[pDrawItem->itemID-1].second >= m_aLimits[1])
		{
			rcItem.bottom = rcItem.top+1;
			rcItem.left += 3; rcItem.right -= 3;
			cDC.FillRect(&rcItem, COLOR_WINDOWTEXT);
		}
		return 1;
	}
	return 0;
}

LRESULT CStorageWindowSQLite::OnMeasureItem(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	LPMEASUREITEMSTRUCT pMeasureItem = reinterpret_cast<LPMEASUREITEMSTRUCT>(a_lParam);
	if (a_wParam == IDC_FWSQL_TAGCLOUD && pMeasureItem->CtlID == IDC_FWSQL_TAGCLOUD)
	{
		TEXTMETRIC tm;
		HDC hDC = ::GetDC(NULL);
		HFONT hFontOld = (HFONT)SelectObject(hDC, GetFont());
		GetTextMetrics(hDC, &tm);
		m_nLabelHeight = tm.tmHeight + tm.tmExternalLeading + 2;
		SelectObject(hDC, hFontOld);
		::ReleaseDC(NULL, hDC);

		pMeasureItem->itemWidth = 60;//max(nX, 90);
		pMeasureItem->itemHeight = m_nLabelHeight;
		return 1;
	}
	return 0;
}

#include "DragDropSource.h"

LRESULT CStorageWindowSQLite::OnBeginDrag(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
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


void FilesFromDrop(IDataObject* a_pDataObj, std::vector<std::tstring>& a_cFileList)
{
	a_cFileList.clear();
	if (a_pDataObj == NULL)
		return;
	STGMEDIUM stgm;
	stgm.tymed = TYMED_HGLOBAL;
	stgm.hGlobal = NULL;
	stgm.pUnkForRelease = NULL;
	FORMATETC fmtetc = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	if (FAILED(a_pDataObj->GetData(&fmtetc, &stgm)))
		return;
	HDROP hDrop = static_cast<HDROP>(GlobalLock(stgm.hGlobal));
	if (hDrop)
	{
		UINT nCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		if (nCount)
		{
			for (UINT i = 0; i < nCount; i++)
			{
				TCHAR szFilePath[MAX_PATH] = _T("");
				DragQueryFile(hDrop, i, szFilePath, MAX_PATH);
				if (szFilePath[0] == _T('\0'))
					continue;
				a_cFileList.push_back(szFilePath);
			}
		}
	}
	GlobalUnlock(stgm.hGlobal);
	ReleaseStgMedium(&stgm);
}

STDMETHODIMP CStorageWindowSQLite::DragEnter(IDataObject* a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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

STDMETHODIMP CStorageWindowSQLite::DragOver(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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

STDMETHODIMP CStorageWindowSQLite::DragLeave()
{
	m_dwLastDropEffect = DROPEFFECT_NONE;
	if (GetDropHelper())
		GetDropHelper()->DragLeave();
	return S_OK;
}

STDMETHODIMP CStorageWindowSQLite::Drop(IDataObject *a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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

LRESULT CStorageWindowSQLite::OnFileTagsModify(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
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

LRESULT CStorageWindowSQLite::OnFollowLink(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
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

LRESULT CStorageWindowSQLite::OnFilterChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	int nCurFilter = m_wndFilter.GetCurSel();
	if (nCurFilter < 0 || nCurFilter >= m_wndFilter.GetCount())
		return 0;
	IDocumentType* pActiveDocType  = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(nCurFilter));
	if (m_pActiveDocType == pActiveDocType)
		return 0;
	if (m_pActiveDocType && pActiveDocType && (m_dwFlags&(EFTOpenExisting|EFTCreateNew)) == EFTCreateNew)
	{
		CComPtr<IEnumStrings> pDefExts;
		m_pActiveDocType->SupportedExtensionsGet(&pDefExts);
		TCHAR szName[MAX_PATH] = _T("");
		m_wndName.GetWindowText(szName, itemsof(szName));
		size_t nNameLen = _tcslen(szName);
		bool bMatch = false;
		size_t nExtLen = 0;
		ULONG i = 0;
		if (pDefExts)
		{
			CComBSTR bstrDefExt;
			for (; !bMatch && SUCCEEDED(pDefExts->Get(i, &bstrDefExt)); ++i)
			{
				if (bstrDefExt)
				{
					COLE2CT strDefExt(bstrDefExt);
					nExtLen = _tcslen(strDefExt);
					bMatch = nNameLen > nExtLen && szName[nNameLen-nExtLen-1] == _T('.') && 0 == _tcsicmp(strDefExt, szName+nNameLen-nExtLen);
					bstrDefExt.Empty();
				}
			}
		}
		if (!pDefExts || i == 0)
		{
			nExtLen = -1;
			bMatch = _tcsrchr(szName, _T('.')) == NULL;
		}
		if (bMatch)
		{
			CComBSTR bstrDefExt2;
			pActiveDocType->DefaultExtensionGet(&bstrDefExt2);
			COLE2CT strDefExt2(bstrDefExt2);
			if (bstrDefExt2)
			{
				szName[nNameLen-nExtLen-1] = _T('.');
				_tcscpy(szName+nNameLen-nExtLen, strDefExt2);
			}
			else
			{
				szName[nNameLen-nExtLen-1] = _T('\0');
			}
			m_wndName.SetWindowText(szName);
		}
	}

	m_pActiveDocType = pActiveDocType;
	NotifyListenerDocumentChange();

	if (m_pActiveDocType != NULL)
	{
		CComBSTR bstrUID;
		m_pActiveDocType->UniqueIDGet(&bstrUID);
		SetConfigValue(CComBSTR(CFGID_SQL_LASTFILTER), CConfigValue(bstrUID));
	}

	UpdateListedItems();

	return 0;
}

LRESULT CStorageWindowSQLite::OnTagCloudChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	m_nActiveTags = m_wndTagCloud.GetSelCount();
	std::tstring str;
	m_aActiveTags.Free();
	if (m_nActiveTags)
	{
		m_aActiveTags.Allocate(m_nActiveTags);
		m_wndTagCloud.GetSelItems(m_nActiveTags, m_aActiveTags);
	}

	m_cInitialTags.clear();
	CComBSTR bstrUID;
	for (int i = 0; i < m_nActiveTags; ++i)
	{
		m_cInitialTags.push_back(m_cTags[m_aActiveTags[i]].first);
		if (bstrUID)
		{
			bstrUID += L" ";
			bstrUID += m_cTags[m_aActiveTags[i]].first.c_str();
		}
		else
		{
			bstrUID = m_cTags[m_aActiveTags[i]].first.c_str();
		}
	}
	if (bstrUID == NULL)
		bstrUID = L"";
	// save last used tags
	SetConfigValue(CComBSTR(CFGID_SQL_LASTTAGS), CConfigValue(bstrUID));

	// update items
	UpdateListedItems();
	return 0;
}

LRESULT CStorageWindowSQLite::OnTagCloudDoubleClick(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	DWORD dwPos = GetMessagePos();
	POINT tPt = {GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)};
	m_wndTagCloud.ScreenToClient(&tPt);
	BOOL bOutSide = TRUE;
	UINT nPos = m_wndTagCloud.ItemFromPoint(tPt, bOutSide);
	if (bOutSide || nPos >= m_cTags.size())
		return 0;
	for (int i = m_wndTagCloud.GetCount()-1; i >= 0; --i)
		m_wndTagCloud.SetSel(i, i == nPos);
	return OnTagCloudChange(0, 0, NULL, a_bHandled);
}

LRESULT CStorageWindowSQLite::OnKeywordsChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	//UpdateListedItems();
	return 0;
}

LRESULT CStorageWindowSQLite::OnNameChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_wndList.GetSelectedCount() != 0)
		m_wndNewRevision.SetCheck(BST_CHECKED); // TODO: do not be that much aggressive
	return 0;
}

void CStorageWindowSQLite::GetItemTextUTF8(UINT a_uID, CAutoVectorPtr<char>& a_pszName)
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

void CStorageWindowSQLite::WStrToUTF8(wchar_t const* a_pszIn, CAutoVectorPtr<char>& a_pszOut)
{
	int nLen2 = WideCharToMultiByte(CP_UTF8, 0, a_pszIn, -1, NULL, 0, NULL, NULL);
	if (nLen2 == 0)
		return;
	a_pszOut.Attach(new char[nLen2]);
	WideCharToMultiByte(CP_UTF8, 0, a_pszIn, -1, a_pszOut.m_p, nLen2, NULL, NULL);
}

int __stdcall LVGroupCompare(int a_nGroup1_ID, int a_nGroup2_ID, void*)
{
	return ((a_nGroup1_ID>>24) < (a_nGroup2_ID>>24)) ? 1 :
		(((a_nGroup1_ID>>24) > (a_nGroup2_ID>>24)) ? -1 : 0);
}

void CStorageWindowSQLite::UpdateListedItems(LONGLONG a_nSelected)
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
	for (int i = 0; i != m_nActiveTags; ++i)
	{
		std::vector<LONGLONG>* pUIDs = NULL;
		CSQLiteStatement cStmt(m_cDb, "SELECT fid FROM tags WHERE name=? ORDER BY fid ASC");
		sqlite3_bind_text16(cStmt, 1, m_cTags[m_aActiveTags[i]].first.c_str(), -1, SQLITE_STATIC);
		while (sqlite3_step(cStmt) == SQLITE_ROW)
		{
			if (pUIDs == NULL)
				pUIDs = &cMap[m_cTags[m_aActiveTags[i]].first];
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
		CSQLiteStatement cStmt(m_cDb, "SELECT id FROM files ORDER BY modified DESC LIMIT 0,2500");
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

bool CStorageWindowSQLite::AddItemToList(LONGLONG a_nID, int a_nGrp, int a_nMatching, bool a_bSelect, bool a_bOverrideFilter)
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
			if (_wcsicmp(pszExt, L"rweffect") == 0)
			{
				CComBSTR bstrType;
				CMultiLanguageString::GetLocalized(L"[0409]Layer Style[0405]Styl vrstvy", m_tLocaleID, &bstrType);
				m_wndList.SetItem(iItem, 1, LVIF_TEXT, CW2CT(bstrType), -1, 0, 0, 0);
				bExtSet = true;
			}
			else for (ULONG j = 0; j < m_nDocTypes && !bExtSet; ++j)
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

void CStorageWindowSQLite::ParseTags(char* a_psz, CTags2& a_cTags)
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

void CStorageWindowSQLite::ParseTags(char const* a_psz, std::set<std::wstring>& a_cTags)
{
	ParseTags(CA2W(a_psz), a_cTags);
}

void CStorageWindowSQLite::ParseTags(wchar_t const* a_psz, std::set<std::wstring>& a_cTags)
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
