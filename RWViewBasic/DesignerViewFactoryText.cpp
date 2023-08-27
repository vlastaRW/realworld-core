// DesignerViewFactoryText.cpp : Implementation of CDesignerViewFactoryText

#include "stdafx.h"
#include "DesignerViewFactoryText.h"

#include "DesignerViewText.h"
#include <MultiLanguageString.h>


// CDesignerViewFactoryText

STDMETHODIMP CDesignerViewFactoryText::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Text - Editor[0405]Text - editor");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryText::ConfigCreate(IViewManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
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

STDMETHODIMP CDesignerViewFactoryText::CreateWnd(IViewManager* UNREF(a_pManager), IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* UNREF(a_pStatusBar), IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;
		CComObject<CDesignerViewText>* pWnd = NULL;
		CComObject<CDesignerViewText>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pDummy = pWnd;
		pWnd->Init(a_pFrame, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_pDoc, a_tLocaleID, this);
		*a_ppDVWnd = pDummy.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryText::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IDocumentText> p;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentText), reinterpret_cast<void**>(&p));
	if (p) a_pCallback->Used(__uuidof(IDocumentText));
	else a_pCallback->Missing(__uuidof(IDocumentText));
	return S_OK;
}

#include "Platform.h"
#include "Scintilla.h"

STDMETHODIMP CDesignerViewFactoryText::RegisterClasses()
{
	if (InterlockedIncrement(&m_nScintillaRefs) == 1)
	{
		m_bValid = Scintilla_RegisterClasses(_pModule->get_m_hInst());
		Scintilla_LinkLexers();
	}
	return m_bValid ? S_OK : E_FAIL;
}

STDMETHODIMP CDesignerViewFactoryText::UnregisterClasses()
{
	if (InterlockedDecrement(&m_nScintillaRefs) == 0)
	{
		if (m_bValid)
			Scintilla_ReleaseResources();
	}
	return S_OK;
}

//STDMETHODIMP CDesignerViewFactoryText::CreateWindow(RWHWND a_hParent, RECT const* a_prc, RWHWND* a_phWnd)
//{
//}

