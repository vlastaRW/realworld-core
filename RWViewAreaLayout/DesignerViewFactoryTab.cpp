// DesignerViewFactoryTab.cpp : Implementation of CDesignerViewFactoryTab

#include "stdafx.h"
#include "DesignerViewFactoryTab.h"

#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include "ConfigIDsTab.h"
#include "ConfigGUITab.h"
#include "ConfigGUITabSubWindow.h"
#include "DesignerViewTab.h"


// CDesignerViewFactoryTab

STDMETHODIMP CDesignerViewFactoryTab::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_TABS_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryTab::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pConfigPattern;
		RWCoCreateInstance(pConfigPattern, __uuidof(ConfigWithDependencies));
		CComPtr<ISubConfigVector> pConfigVector;
		RWCoCreateInstance(pConfigVector, __uuidof(SubConfigVector));

		// insert values to the pattern
		a_pManager->InsertIntoConfigAs(a_pManager, pConfigPattern, CComBSTR(CFGID_TABS_VIEW), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_VIEW_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_VIEW_DESC), 0, NULL);

		// icon
		CComBSTR cCFGID_ICONID(CFGID_TABS_ICONID);
		CComPtr<IConfigItemCustomOptions> pCustIconIDs;
		RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
		if (pCustIconIDs != NULL)
			pConfigPattern->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
		else
			pConfigPattern->ItemInsSimple(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), NULL, 0, NULL);

		pConfigPattern->ItemInsSimple(CComBSTR(CFGID_TABS_CONDITION), CMultiLanguageString::GetAuto(L"[0409]Condition[0405]Podmínka"), CMultiLanguageString::GetAuto(L"[0409]The tab will only be visible if the specified module or class is installed.[0405]Záložka bude viditelná jen v případě, že uvedený modul nebo třída jsou nainstalovány."), CConfigValue(L""), NULL, 0, NULL);

		// finalize pattern
		CConfigCustomGUI<&TTABSUBWINDOWCONFIGGUIID, CConfigGUITabSubWindow>::FinalizeConfig(pConfigPattern);

		// insert pattern to the vector
		pConfigVector->Init(TRUE, pConfigPattern);

		// insert values and vector to the config
		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_TABS_HEADERPOS(CFGID_TABS_HEADERPOS);
		pCfgInit->ItemIns1ofN(cCFGID_TABS_HEADERPOS, _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_HEADERPOS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_HEADERPOS_DESC), CConfigValue(static_cast<LONG>(0)), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TABS_HEADERPOS, CConfigValue(0L), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TABS_POSTOP), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TABS_HEADERPOS, CConfigValue(LONG(CTCS_BOTTOM)), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TABS_POSBOTTOM), 0, NULL);
		//pCfgInit->ItemOptionAdd(CComBSTR(CFGID_TAB_POS), CConfigValue(static_cast<LONG>(TCS_VERTICAL | TCS_MULTILINE)), _SharedStringTable.GetStringAuto(IDS_TAB_LEFT), 0, NULL);
		//pCfgInit->ItemOptionAdd(CComBSTR(CFGID_TAB_POS), CConfigValue(static_cast<LONG>(TCS_VERTICAL | TCS_MULTILINE | TCS_RIGHT)), _SharedStringTable.GetStringAuto(IDS_TAB_RIGHT), 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TABS_ACTIVEINDEX), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_ACTIVEINDEX_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_ACTIVEINDEX_DESC), CConfigValue(0L), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TABS_ITEMS), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_ITEMS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_ITEMS_DESC), CConfigValue(1L), pConfigVector.p, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TABS_INVISIBLE), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_INVISIBLE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_INVISIBLE_DESC), CConfigValue(false), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TABS_TABID), CMultiLanguageString::GetAuto(L"[0409]Tab ID[0405]ID záložky"), CMultiLanguageString::GetAuto(L"[0409]Identifier of the tabs controlled.[0405]Identifikátor záložek, které budou ovládány."), CConfigValue(L""), NULL, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_DesignerViewFactoryTab, CConfigGUITabDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryTab::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComObject<CDesignerViewTab>* pWnd = NULL;
		CComObject<CDesignerViewTab>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pTmp = pWnd;
		pWnd->Init(a_pFrame, a_pStatusBar, a_pManager, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_pDoc, a_tLocaleID);

		*a_ppDVWnd = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryTab::CheckSuitability(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	try
	{
		CConfigValue cTabCount;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TABS_ITEMS), &cTabCount);
		LONG i;
		for(i = 0; i < implicit_cast<LONG>(cTabCount); i++)
		{
			OLECHAR szID[64];
			swprintf(szID, L"%s\\%08x\\%s", CFGID_TABS_ITEMS, i, CFGID_TABS_VIEW);
			CComPtr<IConfig> pSubCfg;
			a_pConfig->SubConfigGet(CComBSTR(szID), &pSubCfg);
			CConfigValue cViewType;
			a_pConfig->ItemValueGet(CComBSTR(szID), &cViewType);
			a_pManager->CheckSuitability(a_pManager, cViewType, pSubCfg, a_pDocument, a_pCallback);
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
