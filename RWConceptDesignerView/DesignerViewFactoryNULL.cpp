// DesignerViewFactoryNULL.cpp : Implementation of CDesignerViewFactoryNULL

#include "stdafx.h"
#include "DesignerViewFactoryNULL.h"

#include "DesignerViewNULL.h"
#include <MultiLanguageString.h>
#include <Win32LangEx.h>

#pragma comment(lib, "htmlhelp.lib")


// CDesignerViewFactoryNULL

STDMETHODIMP CDesignerViewFactoryNULL::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]» Empty View «[0405]» Prázdný pohled «");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryNULL::ConfigCreate(IViewManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		return E_NOTIMPL;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryNULL::CreateWnd(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), ISharedStateManager* UNREF(a_pFrame), IStatusBarObserver* UNREF(a_pStatusBar), IDocument* UNREF(a_pDoc), RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComObject<CDesignerViewNULL>* pWnd = NULL;
		CComObject<CDesignerViewNULL>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pTmp = pWnd;
		CComBSTR bstrMessage;
		CMultiLanguageString::GetLocalized(L"[0409]Configuration may be corrupted. Reset configuration by holding CTRL and SHIFT while starting the application.[0405]Konfigurace mohla být poškozena. Obnovte původní konfiguraci podržením kláves CTRL a SHIFT při startu aplikace.", a_tLocaleID, &bstrMessage);
		CComBSTR bstrHelp;
		CMultiLanguageString::GetLocalized(L"[0409]No view is occupying this area due to a invalid configuration. Please verify your configuration and eventually reset it to default values using the manager tool in Windows Start menu.[0405]Tuto oblast nezabírá žádný pohled, pravděpodobně kvůli neplatné konfiguraci. Prosím zkontrolujte Vaší konfiguraci a případně obnovte výchozí stav pomocí nástroje pro správu ve Start menu Windows.", a_tLocaleID, &bstrHelp);
		pWnd->SetTexts(bstrMessage, bstrHelp, reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)));
		if (pWnd->Create(a_hParent, const_cast<RECT*>(a_prcWindow), NULL, WS_VISIBLE|WS_CHILDWINDOW, (a_nStyle&EDVWSBorderMask) == EDVWSBorder ? WS_EX_CLIENTEDGE : 0) == NULL)
			return E_FAIL;

		*a_ppDVWnd = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryNULL::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* UNREF(a_pDocument), ICheckSuitabilityCallback* UNREF(a_pCallback))
{
	return S_OK; // this view does not affect anything
}

