// CustomNewFileTemplates.cpp : Implementation of CCustomNewFileTemplates

#include "stdafx.h"
#include "CustomNewFileTemplates.h"
#include <MultiLanguageString.h>
#include <shlobj.h>
#include <ApplicationFileName.h>


CCustomNewFileTemplates::CCustomNewFileTemplates() : m_bInitialized(false)
{
}

CCustomNewFileTemplates::~CCustomNewFileTemplates()
{
}

void CCustomNewFileTemplates::InternInit()
{
	if (!m_bInitialized)
	{
		CComPtr<ILocalizedString> pDesc;
		pDesc.Attach(new CMultiLanguageString(L"[0409]Custom template.[0405]Uživatelská šablona."));

		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		CComBSTR bstrDir;
		if (pAI == NULL || FAILED(pAI->AppDataFolder(&bstrDir)) || bstrDir.Length() == 0)
			throw E_FAIL;
		bstrDir += L"\\Templates\\*.*";
		COLE2CT strDir(bstrDir.m_str);

		TCHAR szFile[MAX_PATH];
		_tcscpy(szFile, strDir);
		int iLen = _tcslen(szFile)-3; // subtract "*.*"
		WIN32_FIND_DATA cFindData;
		HANDLE hDir = FindFirstFile(strDir, &cFindData);
		if (hDir != INVALID_HANDLE_VALUE)
		{
			USES_CONVERSION;

			do
			{
				if (_tcscmp(cFindData.cFileName, _T(".")) != 0 &&
					_tcscmp(cFindData.cFileName, _T("..")) != 0)
				{
					SItem sItem;
					_tcscpy(szFile + iLen, cFindData.cFileName);
					sItem.bstrPath = szFile;
					CComPtr<ILocalizedStringInit> pInit;
					RWCoCreateInstance(pInit, __uuidof(LocalizedString));
					pInit->Insert(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT), CComBSTR(cFindData.cFileName));
					sItem.pName.Attach(pInit.Detach());
					sItem.pDesc = pDesc;
					m_cItems.push_back(sItem);
				}
			} while(FindNextFile(hDir, &cFindData));
			FindClose(hDir);
		}
		m_bInitialized = true;
	}
}

HRESULT CCustomNewFileTemplates::TemplatesSize(ULONG* a_pnCount)
{
	try
	{
		ObjectLock cLock(this);
		InternInit();

		*a_pnCount = m_cItems.size();//+1;
		return S_OK;
	}
	catch (...)
	{
		return a_pnCount == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HRESULT CCustomNewFileTemplates::TemplatesName(ULONG a_nIndex, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		ObjectLock cLock(this);
		InternInit();

		if (a_nIndex == m_cItems.size())
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Add custom template[0405]Přidat vlastní šablonu");
			return S_OK;
		}

		if (a_nIndex >= m_cItems.size())
			return E_RW_INDEXOUTOFRANGE;

		SItem& sItem = m_cItems[a_nIndex];
		(*a_ppName = sItem.pName)->AddRef();

		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HRESULT CCustomNewFileTemplates::TemplatesHelpText(ULONG a_nIndex, ILocalizedString** a_ppHelpText)
{
	try
	{
		*a_ppHelpText = NULL;
		ObjectLock cLock(this);
		InternInit();

		if (a_nIndex == m_cItems.size())
		{
			*a_ppHelpText = new CMultiLanguageString(L"[0409]Make a template from a file. Copy of the file is opened when the template is used.[0405]Udělat šablonu ze souboru. Kopie souboru bude otevřena při použití šablony.");
			return S_OK;
		}

		if (a_nIndex >= m_cItems.size())
			return E_RW_INDEXOUTOFRANGE;

		SItem& sItem = m_cItems[a_nIndex];
		(*a_ppHelpText = sItem.pDesc)->AddRef();

		return S_OK;
	}
	catch (...)
	{
		return a_ppHelpText == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <IconRenderer.h>
HRESULT CCustomNewFileTemplates::TemplatesIcon(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		ObjectLock cLock(this);
		InternInit();

		if (a_nIndex == m_cItems.size())
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			static IRPathPoint const outer[] =
			{
				{256, 32, 0, 0, 0, -17.6731},
				{256, 224, 0, 17.6731, 0, 0},
				{224, 256, 0, 0, 17.6731, 0},
				{32, 256, -17.6731, 0, 0, 0},
				{0, 224, 0, 0, 0, 17.6731},
				{0, 32, 0, -17.6731, 0, 0},
				{32, 0, 0, 0, -17.6731, 0},
				{224, 0, 17.6731, 0, 0, 0},
			};
			static IRPathPoint const inner[] =
			{
				{128, 23, 0, 0, 0, 0},
				{152, 98, 0, 0, 0, 0},
				{232, 98, 0, 0, 0, 0},
				{168, 145, 0, 0, 0, 0},
				{192, 220, 0, 0, 0, 0},
				{128, 174, 0, 0, 0, 0},
				{64, 220, 0, 0, 0, 0},
				{88, 145, 0, 0, 0, 0},
				{24, 98, 0, 0, 0, 0},
				{104, 98, 0, 0, 0, 0},
			};
			static IRPath const shape[] = { {itemsof(outer), outer}, {itemsof(inner), inner} };
			IRCanvas canvas = { 0, 0, 256, 256, 0, 0, NULL, NULL };
			cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMScheme2Color2));
			*a_phIcon = cRenderer.get();
			return S_OK;
		}

		if (a_nIndex >= m_cItems.size())
			return E_RW_INDEXOUTOFRANGE;

		SItem& sItem = m_cItems[a_nIndex];
		SHFILEINFO tInfo;
		ZeroMemory(&tInfo, sizeof tInfo);
		USES_CONVERSION;
		SHGetFileInfo(OLE2CT(sItem.bstrPath), 0, &tInfo, sizeof tInfo, SHGFI_ICON|SHGFI_LARGEICON/*|SHGFI_SMALLICON*/);
		*a_phIcon = tInfo.hIcon;

		return S_OK;
	}
	catch (...)
	{
		return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CCustomNewFileTemplates::Category(ULONG a_nIndex, ILocalizedString** a_ppCategory)
{
	try
	{
		*a_ppCategory = 0;
		if (a_nIndex > m_cItems.size())
			return E_RW_INDEXOUTOFRANGE;

		*a_ppCategory = new CMultiLanguageString(L"[0409]Custom Templates[0405]Uživatelské šablony");
		return S_OK;
	}
	catch (...)
	{
		return a_ppCategory == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CCustomNewFileTemplates::CheckFeatures(ULONG a_nIndex, ULONG a_nCount, const IID* a_aiidRequired)
{
	try
	{
		if (a_nIndex > m_cItems.size())
			return E_RW_INDEXOUTOFRANGE;

		return S_OK; //TODO: implement correctly
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CCustomNewFileTemplates::DocumentCreate(ULONG a_nIndex, RWHWND UNREF(a_hParentWnd), LCID UNREF(a_tLocaleID), IConfig* UNREF(a_pConfig), BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		ObjectLock cLock(this);
		InternInit();

		CComPtr<IInputManager> pManager;
		RWCoCreateInstance(pManager, __uuidof(InputManager));

		if (a_nIndex >= m_cItems.size())
			return E_RW_INDEXOUTOFRANGE;

		return pManager->DocumentCreateData(0, NULL, CStorageFilter(m_cItems[a_nIndex].bstrPath), a_bstrPrefix, a_pBase);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CCustomNewFileTemplates::ToolsSize(ULONG* a_pnCount)
{
	try
	{
		*a_pnCount = 1;
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

HRESULT CCustomNewFileTemplates::ToolsName(ULONG a_nIndex, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_nIndex != 0)
			return E_RW_INDEXOUTOFRANGE;
		*a_ppName = new CMultiLanguageString(L"[0409]Document templates...[0405]Uživatelské šablony dokumentů...");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HRESULT CCustomNewFileTemplates::ToolsHelpText(ULONG a_nIndex, ILocalizedString** a_ppHelpText)
{
	try
	{
		*a_ppHelpText = NULL;
		if (a_nIndex != 0)
			return E_RW_INDEXOUTOFRANGE;
		*a_ppHelpText = new CMultiLanguageString(L"[0409]Add, remove and rename templates for new documents.[0405]Přidat, odebrat nebo přejmenovat šablony pro vytváření nových dokumentů.");
		return S_OK;
	}
	catch (...)
	{
		return a_ppHelpText == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HRESULT CCustomNewFileTemplates::ToolsIcon(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		return E_NOTIMPL;
	}
	catch (...)
	{
		return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CCustomNewFileTemplates::Activate(ULONG a_nIndex, RWHWND a_hFrameWnd, LCID a_tLocaleID, IDocument* a_pDocument)
{
	try
	{
		if (a_nIndex != 0)
			return E_RW_INDEXOUTOFRANGE;
		MessageBox(reinterpret_cast<HWND>(a_hFrameWnd), _T("Manage custom teplates"), _T("Information"), MB_OK);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

