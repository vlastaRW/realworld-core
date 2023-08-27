// DesignerViewSubDocumentFrame.cpp : Implementation of CDesignerViewSubDocumentFrame

#include "stdafx.h"
#include "DesignerViewFactorySubDocumentFrame.h"

#include "DesignerViewSubDocumentFrame.h"
#include "ConfigIDsSubDocumentFrame.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include "ConfigGUISubDocumentFrame.h"


// CDesignerViewSubDocumentFrame

STDMETHODIMP CDesignerViewSubDocumentFrame::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_FRM_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewSubDocumentFrame::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		// CFGID_FRM_SUBVIEW
		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_FRM_SUBVIEW), _SharedStringTable.GetStringAuto(IDS_CFGID_FRM_SUBVIEW_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FRM_SUBVIEW_HELP), 0, NULL);

		// CFGID_IMG_SELSYNCGROUP
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_FRM_SELSYNCGROUP), _SharedStringTable.GetStringAuto(IDS_CFGID_FRM_SELSYNCGROUP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FRM_SELSYNCGROUP_HELP), CConfigValue(L"SUBDOC"), NULL, 0, NULL);

		// CFGID_FRM_CACHEVIEWS
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_FRM_CACHEVIEWS), _SharedStringTable.GetStringAuto(IDS_CFGID_FRM_CACHEVIEWS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FRM_CACHEVIEWS_HELP), CConfigValue(false), NULL, 0, NULL);

		// CFGID_FRM_NOVIEWMSG
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_FRM_NOVIEWMSG), CMultiLanguageString::GetAuto(L"[0409]Empty message[0405]Prázdná zpráva"), CMultiLanguageString::GetAuto(L"[0409]Message displayed when no sub-document is available.[0405]Zpráva zobrazená, když není k dispozici žádný pod-dokument."), CConfigValue(L""), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DesignerViewSubDocumentFrame, CConfigGUISubDocumentFrameDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewSubDocumentFrame::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComPtr<ISubDocumentsMgr> pSubDocMgr;
		a_pDoc->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSubDocMgr));
		if (pSubDocMgr == NULL)
			return E_FAIL;

		CComObject<CDesignerViewWndSubDocumentFrame>* pWnd = NULL;
		CComObject<CDesignerViewWndSubDocumentFrame>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pDummy = pWnd;

		pWnd->Init(a_pManager, a_pFrame, a_pStatusBar, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, pSubDocMgr);

		*a_ppDVWnd = pDummy.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewSubDocumentFrame::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<ISubDocumentsMgr> p;
	a_pDocument->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&p));
	if (p) a_pCallback->Used(__uuidof(ISubDocumentsMgr));
	else a_pCallback->Missing(__uuidof(ISubDocumentsMgr));
	return S_OK;
}
