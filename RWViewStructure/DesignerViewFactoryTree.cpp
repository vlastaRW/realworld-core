// DesignerViewFactoryTree.cpp : Implementation of CDesignerViewFactoryTree

#include "stdafx.h"
#include "DesignerViewFactoryTree.h"

#include "ConfigIDsTree.h"
#include "DesignerViewTree.h"
#include <SharedStringTable.h>
#include "ConfigGUITree.h"


// CDesignerViewFactoryTree

STDMETHODIMP CDesignerViewFactoryTree::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_TREE_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryTree::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		// insert the values
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TREE_SELSYNCGROUP), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_SELSYNCGROUP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_SELSYNCGROUP_DESC), CConfigValue(L"SELECTION"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TREE_STRUCTUREROOT), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_STRUCTUREROOT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_STRUCTUREROOT_DESC), CConfigValue(GUID_NULL), NULL, 0, NULL);
		CComBSTR cCFGID_TREE_KEYDOWNACTION(CFGID_TREE_KEYDOWNACTION);
		pCfgInit->ItemIns1ofN(cCFGID_TREE_KEYDOWNACTION, _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_KEYDOWNACTION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_KEYDOWNACTION_DESC), CConfigValue(CFGVAL_TKDA_STARTEDITING), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TREE_KEYDOWNACTION, CConfigValue(CFGVAL_TKDA_JUMPTOITEM), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TKDA_JUMPTOITEM), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TREE_KEYDOWNACTION, CConfigValue(CFGVAL_TKDA_STARTEDITING), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TKDA_STARTEDITING), 0, NULL);
		CComBSTR cCFGID_TREE_VIEWMODE(CFGID_TREE_VIEWMODE);
		pCfgInit->ItemIns1ofN(cCFGID_TREE_VIEWMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_VIEWMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_VIEWMODE_DESC), CConfigValue(CFGVAL_TVM_SMALLICONS), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TREE_VIEWMODE, CConfigValue(CFGVAL_TVM_SMALLICONS), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TVM_SMALLICONS), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TREE_VIEWMODE, CConfigValue(CFGVAL_TVM_MEDIUMICONS), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TVM_MEDIUMICONS), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TREE_VIEWMODE, CConfigValue(CFGVAL_TVM_LARGEICONS), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TVM_LARGEICONS), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TREE_VIEWMODE, CConfigValue(CFGVAL_TVM_THUMBNAILS), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TVM_THUMBNAILS), 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TREE_VIEWLINES), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_VIEWLINES_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_VIEWLINES_DESC), CConfigValue(true), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TREE_SHOWITEMONFOCUS), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_SHOWITEMONFOCUS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_SHOWITEMONFOCUS_DESC), CConfigValue(false), NULL, 0, NULL);

		// insert values to the pattern
		CComPtr<IMenuCommandsManager> pMenuCmds;
		a_pManager->QueryInterface(__uuidof(IMenuCommandsManager), reinterpret_cast<void**>(&pMenuCmds));
		if (pMenuCmds == NULL)
		{
			RWCoCreateInstance(pMenuCmds, __uuidof(MenuCommandsManager));
		}
		pMenuCmds->InsertIntoConfigAs(pMenuCmds, pCfgInit, CComBSTR(CFGID_TREE_CONTEXTMENU), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_CONTEXTMENU_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TREE_CONTEXTMENU_DESC), 0, NULL);

		CConfigCustomGUI<&CLSID_DesignerViewFactoryTree, CConfigGUITreeDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryTree::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;
		CComObject<CDesignerViewTree>* pWnd = NULL;
		CComObject<CDesignerViewTree>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pDummy = pWnd;

		CComQIPtr<IMenuCommandsManager> pCmdMgr(a_pManager);
		if (pCmdMgr == NULL)
			RWCoCreateInstance(pCmdMgr, __uuidof(MenuCommandsManager));
		pWnd->Init(a_pFrame, a_pStatusBar, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_pDoc, a_tLocaleID, pCmdMgr);
		*a_ppDVWnd = pDummy.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryTree::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IStructuredDocument> p1;
	a_pDocument->QueryFeatureInterface(__uuidof(IStructuredDocument), reinterpret_cast<void**>(&p1));
	if (p1) a_pCallback->Used(__uuidof(IStructuredDocument));
	CComPtr<ISubDocumentsMgr> p2;
	a_pDocument->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&p2));
	if (p2) a_pCallback->Used(__uuidof(ISubDocumentsMgr));
	if (p1 == NULL && p2 == NULL)
		a_pCallback->Missing(__uuidof(IStructuredDocument));
	return S_OK;
}
