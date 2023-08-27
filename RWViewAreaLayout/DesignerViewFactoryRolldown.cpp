// DesignerViewFactoryRolldown.cpp : Implementation of CDesignerViewFactoryRolldown

#include "stdafx.h"
#include "DesignerViewFactoryRolldown.h"
#include "DesignerViewRolldown.h"
#include <SharedStringTable.h>
#include "ConfigIDsRolldown.h"
#include "ConfigGUIRolldown.h"
#include "ConfigGUIRolldownSubWindow.h"


// CDesignerViewFactoryRolldown

STDMETHODIMP CDesignerViewFactoryRolldown::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_ROLLDOWN_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryRolldown::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pConfigPattern;
		RWCoCreateInstance(pConfigPattern, __uuidof(ConfigWithDependencies));
		CComPtr<ISubConfigVector> pConfigVector;
		RWCoCreateInstance(pConfigVector, __uuidof(SubConfigVector));

		// insert values to the pattern
		a_pManager->InsertIntoConfigAs(a_pManager, pConfigPattern, CComBSTR(CFGID_RD_VIEWTYPE), _SharedStringTable.GetStringAuto(IDS_CFGID_RD_VIEWTYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_RD_VIEWTYPE_DESC), 0, NULL);

		// icon
		CComBSTR cCFGID_ICONID(CFGID_PANELS_ICONID);
		CComPtr<IConfigItemCustomOptions> pCustIconIDs;
		RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
		if (pCustIconIDs != NULL)
			pConfigPattern->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
		else
			pConfigPattern->ItemInsSimple(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), NULL, 0, NULL);

		CComBSTR cCFGID_RD_CUSTOMHEIGHT(CFGID_RD_CUSTOMHEIGHT);
		pConfigPattern->ItemInsSimple(cCFGID_RD_CUSTOMHEIGHT, _SharedStringTable.GetStringAuto(IDS_CFGID_RD_CUSTOMHEIGHT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_RD_CUSTOMHEIGHT_DESC), CConfigValue(false), NULL, 0, NULL);
		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_RD_CUSTOMHEIGHT;
		tCond.eConditionType = ECOCEqual;
		tCond.tValue = CConfigValue(true);
		pConfigPattern->ItemInsSimple(CComBSTR(CFGID_RD_HEIGHT), _SharedStringTable.GetStringAuto(IDS_CFGID_RD_HEIGHT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_RD_HEIGHT_DESC), CConfigValue(200.0f), NULL, 1, &tCond);
		pConfigPattern->ItemInsSimple(CComBSTR(CFGID_RD_COLLAPSED), _SharedStringTable.GetStringAuto(IDS_CFGID_RD_COLLAPSED_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_RD_COLLAPSED_DESC), CConfigValue(false), NULL, 0, NULL);
		// finalize pattern
		CConfigCustomGUI<&TROLLDOWNSUBWINDOWCONFIGGUIID, CConfigGUIRolldownSubWindow>::FinalizeConfig(pConfigPattern);

		// insert pattern to the vector
		pConfigVector->Init(TRUE, pConfigPattern);

		// insert values and vector to the config
		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RD_ITEMS), _SharedStringTable.GetStringAuto(IDS_CFGID_RD_ITEMS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_RD_ITEMS_DESC), CConfigValue(0L), pConfigVector.p, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_DesignerViewFactoryRolldown, CConfigGUIRolldownDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryRolldown::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComObject<CDesignerViewRolldown>* pWnd = NULL;
		CComObject<CDesignerViewRolldown>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pTmp = pWnd;
		pWnd->Init(a_pFrame, a_pStatusBar, a_pManager, a_pConfig, a_hParent, a_prcWindow, a_pDoc, a_nStyle, a_tLocaleID);

		*a_ppDVWnd = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryRolldown::CheckSuitability(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	try
	{
		CConfigValue cTabCount;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_RD_ITEMS), &cTabCount);
		LONG i;
		for(i = 0; i < implicit_cast<LONG>(cTabCount); i++)
		{
			OLECHAR szID[64];
			swprintf(szID, L"%s\\%08x\\%s", CFGID_RD_ITEMS, i, CFGID_RD_VIEWTYPE);
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
