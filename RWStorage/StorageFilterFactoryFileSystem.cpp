// StorageFilterFactoryFileSystem.cpp : Implementation of CStorageFilterFactoryFileSystem

#include "stdafx.h"
#include "StorageFilterFactoryFileSystem.h"
#include "StorageFilterWindowFileSystem.h"
#include "StorageFilterBrowserFileSystem.h"
#include "StorageFilterFileSystem.h"

#include "ConfigIDsFileSystem.h"
#include <MultiLanguageString.h>

#include <XPGUI.h>
#include <IconRenderer.h>


// CStorageFilterFactoryFileSystem

STDMETHODIMP CStorageFilterFactoryFileSystem::NameGet(ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]This computer[0405]Tento počítač");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFactoryFileSystem::IconGet(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		static IRPathPoint const casePoints[] =
		{
			{184, 84, 0, -4.97056, 0, 0},
			{193, 75, 0, 0, -4.97056, 0},
			{247, 75, 4.97056, 0, 0, 0},
			{256, 84, 0, 0, 0, -4.97056},
			{256, 217, 0, 4.97056, 0, 0},
			{247, 226, 0, 0, 4.97056, 0},
			{193, 226, -4.97056, 0, 0, 0},
			{184, 217, 0, 0, 0, 4.97056},
		};
		static IRPathPoint const buttonPoints[] =
		{
			{241, 159, 0, -3.86599, 0, 3.86599},
			{234, 152, -3.86599, 0, 3.86599, 0},
			{227, 159, 0, 3.86599, 0, -3.86599},
			{234, 166, 3.86599, 0, -3.86599, 0},
		};
		static IRPolyPoint const line1Points[] = { {199.5, 98.5}, {240.5, 98.5} };
		static IRPolyPoint const line2Points[] = { {199.5, 117.5}, {240.5, 117.5} };
		static IRPolyPoint const line3Points[] = { {199.5, 136.5}, {240.5, 136.5} };
		static IRPathPoint const standPoints[] =
		{
			{50, 226, 11, -9, 0, 0},
			{73, 174, 0, 0, 0, 20},
			{97, 174, 0, 20, 0, 0},
			{120, 226, 0, 0, -11, -9},
		};
		static IRPathPoint const barPoints[] =
		{
			{0, 157, 0, 0, 0, 0},
			{170, 157, 0, 0, 0, 0},
			{170, 179, 0, 4.97056, 0, 0},
			{161, 188, 0, 0, 4.97056, 0},
			{9, 188, -4.97056, 0, 0, 0},
			{0, 179, 0, 0, 0, 4.97056},
		};
		static IRPathPoint const screenPoints[] =
		{
			{0, 62, 0, -4.97056, 0, 0},
			{9, 53, 0, 0, -4.97056, 0},
			{161, 53, 4.97056, 0, 0, 0},
			{170, 62, 0, 0, 0, -4.97056},
			{170, 162, 0, 4.97056, 0, 0},
			{161, 171, 0, 0, 4.97056, 0},
			{9, 171, -4.97056, 0, 0, 0},
			{0, 162, 0, 0, 0, 4.97056},
		};
		static IRGridItem const gridDisplayX[] = {{0, 0}, {0, 170}};
		static IRGridItem const gridDisplayY[] = {{0, 53}, {0, 171}, {0, 188}};
		static IRCanvas const canvasDisplay[] = {0, 53, 256, 226, itemsof(gridDisplayX), itemsof(gridDisplayY), gridDisplayX, gridDisplayY};
		static IRGridItem const gridCaseX[] = {{0, 184}, {0, 256}};
		static IRGridItem const gridCaseY[] = {{0, 75}, {0, 226}};
		static IRCanvas const canvasCase[] = {0, 53, 256, 226, itemsof(gridCaseX), itemsof(gridCaseY), gridCaseX, gridCaseY};
		IRFill matGlassFill(0xffd5ebf1);
		IROutlinedFill matGlass(&matGlassFill, pSI->GetMaterial(ESMContrast));
		CIconRendererReceiver cRenderer(a_nSize);
		cRenderer(canvasCase, itemsof(casePoints), casePoints, pSI->GetMaterial(ESMInterior));
		cRenderer(canvasCase, itemsof(buttonPoints), buttonPoints, pSI->GetMaterial(ESMContrast));
		cRenderer(canvasCase, itemsof(line1Points), line1Points, pSI->GetMaterial(ESMOutline));
		cRenderer(canvasCase, itemsof(line2Points), line2Points, pSI->GetMaterial(ESMOutline));
		cRenderer(canvasCase, itemsof(line3Points), line3Points, pSI->GetMaterial(ESMOutline));
		cRenderer(canvasDisplay, itemsof(standPoints), standPoints, pSI->GetMaterial(ESMInterior));
		cRenderer(canvasDisplay, itemsof(barPoints), barPoints, pSI->GetMaterial(ESMInterior));
		cRenderer(canvasDisplay, itemsof(screenPoints), screenPoints, &matGlass);
		*a_phIcon = cRenderer.get();
		return (*a_phIcon) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFactoryFileSystem::SupportsGUI(DWORD UNREF(a_dwFlags))
{
	return S_OK;
}

STDMETHODIMP CStorageFilterFactoryFileSystem::FilterCreate(BSTR a_bstrFilter, DWORD a_dwFlags, IStorageFilter** a_ppFilter)
{
	CHECKPOINTER(a_ppFilter);
	*a_ppFilter = NULL;
	CHECKPOINTER(a_bstrFilter);

	try
	{
		USES_CONVERSION;
		LPTSTR pszTmp = OLE2T(a_bstrFilter);
		if (_tcsnicmp(pszTmp, _T("file://"), 7) == 0)
		{
			pszTmp += 7;
		}
		if (_tcsnicmp(pszTmp, _T("\\\\?\\UNC\\"), 8) == 0)
		{
			LPTSTR pszUnc = reinterpret_cast<LPTSTR>(_alloca(_tcslen(pszTmp)-6));
			pszUnc[0] = pszUnc[1] = _T('\\');
			_tcscpy(pszUnc+2, pszTmp+8);
			pszTmp = pszUnc;
		}
		if (_tcsnicmp(pszTmp, _T("\\\\?\\"), 4) == 0)
		{
			pszTmp += 4;
		}
		int nLen = _tcslen(pszTmp);
		if (nLen > 2 && ((pszTmp[0] >= _T('A') && pszTmp[0] <= _T('Z')) || ((pszTmp[0] >= _T('a') && pszTmp[0] <= _T('z')))) &&
			pszTmp[1] == _T(':') && pszTmp[2] == _T('\\'))
		{
			// classic c:\...
		}
		else if (nLen > 1 && pszTmp[0] == _T('\\') && pszTmp[1] == _T('\\'))
		{
			// UNC \\...
		}
		else
		{
			return E_FAIL; // TODO: error code
		}
		CComObject<CStorageFilterFileSystem>* pObj = NULL;
		CComObject<CStorageFilterFileSystem>::CreateInstance(&pObj);
		pObj->Init(pszTmp);
		return pObj->QueryInterface<IStorageFilter>(a_ppFilter);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFactoryFileSystem::WindowCreate(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilterWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;

		if (a_dwFlags & EFTFileBrowser)
		{
			CComObject<CStorageFilterBrowserFileSystem>* pWnd = NULL;
			CComObject<CStorageFilterBrowserFileSystem>::CreateInstance(&pWnd);
			CComPtr<IStorageFilterWindow> pTmp = pWnd;
			HRESULT hRes = pWnd->Init(a_bstrInitial, a_dwFlags, a_pFormatFilters, a_pContextConfig, a_pCallback, a_pListener, a_hParent, a_tLocaleID);
			if (FAILED(hRes)) return hRes;

			*a_ppWindow = pTmp.Detach();
			return hRes;
		}
		else
		{
			CComObject<CStorageFilterWindowFileSystem>* pWnd = NULL;
			CComObject<CStorageFilterWindowFileSystem>::CreateInstance(&pWnd);
			CComPtr<IStorageFilterWindow> pTmp = pWnd;
			HRESULT hRes = pWnd->Init(a_bstrInitial, a_dwFlags, a_pFormatFilters, a_pContextConfig, a_pCallback, a_pListener, a_hParent, a_tLocaleID);
			if (FAILED(hRes)) return hRes;

			*a_ppWindow = pTmp.Detach();
			return hRes;
		}
	}
	catch (...)
	{
		return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFactoryFileSystem::ContextConfigGetDefault(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		CComPtr<ILocalizedStringInit> pDummy;
		RWCoCreateInstance(pDummy, __uuidof(LocalizedString));

		pCfg->ItemInsSimple(CComBSTR(CFGID_FS_LASTDIRECTORY), pDummy, pDummy, CConfigValue(L""), NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_FS_RECENTFILES), pDummy, pDummy, CConfigValue(L""), NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_FS_LASTFILTER), pDummy, pDummy, CConfigValue(L""), NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_FS_VIEWMODE), pDummy, pDummy, CConfigValue(LONG(FVM_LIST)), NULL, 0, NULL);

		wchar_t szTmp[64];
		swprintf(szTmp, L"0x%04x|0x%04x|0x%04x", CSIDL_PERSONAL, CSIDL_DESKTOP, CSIDL_DRIVES);
		pCfg->ItemInsSimple(CComBSTR(CFGID_FS_FAVFOLDERS), pDummy, pDummy, CConfigValue(szTmp), NULL, 0, NULL);

		pCfg->Finalize(NULL);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

int InitToolbarIcons(CImageList& imageList)
{
	int nIconSize = XPGUI::GetSmallIconSize();
	ULONG nIconDelta = (nIconSize>>1)-8;
	ULONG nGapLeft = nIconDelta>>1;
	ULONG nGapRight = nIconDelta-nGapLeft;
	RECT padding = {nGapLeft, 0, nGapRight, 0};
	imageList.Create(nIconSize+nIconDelta, nIconSize, XPGUI::GetImageListColorFlags(), 5, 0);
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	{
		static IRPathPoint const circlePoints[] =
		{
			{256, 128, 0, -70.6925, 0, 70.6925},
			{128, 0, -70.6925, 0, 70.6925, 0},
			{0, 128, 0, 70.6925, 0, -70.6925},
			{128, 256, 70.6925, 0, -70.6925, 0},
		};
		static IRPolyPoint const arrowPoints[] =
		{
			{216, 160}, {112, 160}, {112, 208}, {32, 128}, {112, 48}, {112, 96}, {216, 96},
		};
		static IRCanvas const canvasCircle[] = {0, 0, 256, 256, 0, 0, NULL, NULL};
		static IRGridItem const gridX[] = {{0, 112}, {0, 216}};
		static IRGridItem const gridY[] = {{0, 96}, {0, 160}};
		static IRCanvas const canvasArrow[] = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
		IRTarget target(0.92f);
		CIconRendererReceiver cRenderer(nIconSize);
		cRenderer(canvasCircle, itemsof(circlePoints), circlePoints, pSI->GetMaterial(ESMConfirm), target);
		cRenderer(canvasArrow, itemsof(arrowPoints), arrowPoints, pSI->GetMaterial(ESMInterior), target);
		HICON hIcon = cRenderer.get(padding);
		imageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.9f, -1, 1));
		pSI->GetLayers(ESIMoveUp, cRenderer, IRTarget(0.85f, 1, -1));
		HICON hIcon = cRenderer.get(padding);
		imageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIHeart, cRenderer, IRTarget(0.85f, -1, 1));
		pSI->GetLayers(ESICreate, cRenderer, IRTarget(0.65f, 1, -1));
		HICON hIcon = cRenderer.get(padding);
		imageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		static IRPolyPoint const icon1Points[] = { {0, 16}, {56, 16}, {56, 72}, {0, 72} };
		static IRPolyPoint const icon2Points[] = { {0, 100}, {56, 100}, {56, 156}, {0, 156} };
		static IRPolyPoint const icon3Points[] = { {0, 184}, {56, 184}, {56, 240}, {0, 240} };
		static IRPolyPoint const line1Points[] = { {86, 16}, {256, 16}, {256, 72}, {86, 72} };
		static IRPolyPoint const line2Points[] = { {86, 100}, {256, 100}, {256, 156}, {86, 156} };
		static IRPolyPoint const line3Points[] = { {86, 184}, {256, 184}, {256, 240}, {86, 240} };
		static IRPolygon const polygons[] = { {itemsof(icon1Points), icon1Points}, {itemsof(icon2Points), icon2Points}, {itemsof(icon3Points), icon3Points}, {itemsof(line1Points), line1Points}, {itemsof(line2Points), line2Points}, {itemsof(line3Points), line3Points} };
		static IRGridItem const gridX[] = {{0, 0}, {0, 56}, {0, 86}, {0, 256}};
		static IRGridItem const gridY[] = {{0, 16}, {1, 72}, {2, 100}, {1, 156}, {2, 184}, {1, 240}};
		static IRCanvas const canvas[] = {0, 16, 256, 240, itemsof(gridX), itemsof(gridY), gridX, gridY};
		IRTarget target(0.92f);
		CIconRendererReceiver cRenderer(nIconSize);
		cRenderer(canvas, itemsof(polygons), polygons, pSI->GetMaterial(ESMInterior), target);
		HICON hIcon = cRenderer.get(padding);
		imageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIHeart, cRenderer, IRTarget(0.85f, -1, 1));
		pSI->GetLayers(ESIMinus, cRenderer, IRTarget(0.65f, 1, -1));
		HICON hIcon = cRenderer.get(padding);
		imageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	return nIconSize;
}