// DesignerViewFactoryBinary.cpp : Implementation of CDesignerViewFactoryBinary

#include "stdafx.h"
#include "DesignerViewFactoryBinary.h"

#include "DesignerViewBinary.h"
#include <MultiLanguageString.h>


// CDesignerViewFactoryBinary

STDMETHODIMP CDesignerViewFactoryBinary::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Binary Data - Hex Editor[0405]Binární data - hex-editor");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryBinary::ConfigCreate(IViewManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		//CComPtr<IConfigWithDependencies> pCfgInit;
		//RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		//if (FAILED(pCfgInit->Finalize(NULL/*pTmp*/)))
		//	return E_FAIL;

		//*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryBinary::CreateWnd(IViewManager* UNREF(a_pManager), IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* UNREF(a_pStatusBar), IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;
		CComObject<CDesignerViewBinary>* pWnd = NULL;
		CComObject<CDesignerViewBinary>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pDummy = pWnd;
		pWnd->Init(a_pFrame, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_pDoc, a_tLocaleID);
		*a_ppDVWnd = pDummy.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryBinary::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IDocumentBinary> p;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentBinary), reinterpret_cast<void**>(&p));
	if (p) a_pCallback->Used(__uuidof(IDocumentBinary));
	else a_pCallback->Missing(__uuidof(IDocumentBinary));
	return S_OK;
}

