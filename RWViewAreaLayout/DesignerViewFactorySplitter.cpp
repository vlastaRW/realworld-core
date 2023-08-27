// DesignerViewFactorySplitter.cpp : Implementation of CDesignerViewFactorySplitter

#include "stdafx.h"
#include "DesignerViewFactorySplitter.h"

#include "ConfigIDsSplitter.h"
#include "DesignerViewSplitter.h"
#include "ConfigGUISplitter.h"
#include <SharedStringTable.h>


// CDesignerViewFactorySplitter

STDMETHODIMP CDesignerViewFactorySplitter::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_SPLITTER_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactorySplitter::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		// insert the values

		// CFGID_SPLITTYPE
		CComBSTR cCFGID_SPLITTYPE(CFGID_SPLITTYPE);
		CConfigValue cESTVertical(ESTVertical);
		CConfigValue cESTHoriznotal(ESTHorizontal);
		CConfigValue cESTBoth(ESTBoth);
		pCfgInit->ItemIns1ofN(cCFGID_SPLITTYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_SPLITTYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SPLITTYPE_HELP), cESTVertical, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_SPLITTYPE, cESTVertical, _SharedStringTable.GetStringAuto(IDS_CFGID_SPLITTYPE_VERT), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_SPLITTYPE, cESTHoriznotal, _SharedStringTable.GetStringAuto(IDS_CFGID_SPLITTYPE_HOR), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_SPLITTYPE, cESTBoth, _SharedStringTable.GetStringAuto(IDS_CFGID_SPLITTYPE_BOTH), 0, NULL);

		CConfigValue cEDTRelativeFixed(EDTRelativeFixed);
		CConfigValue cEDTRelativeAdjustable(EDTRelativeAdjustable);
		CConfigValue cEDTAbsoluteLTFixed(EDTAbsoluteLTFixed);
		CConfigValue cEDTAbsoluteLTAdjustable(EDTAbsoluteLTAdjustable);
		CConfigValue cEDTAbsoluteRBFixed(EDTAbsoluteRBFixed);
		CConfigValue cEDTAbsoluteRBAdjustable(EDTAbsoluteRBAdjustable);
		// CFGID_VERDIVTYPE
		{
			CComBSTR cCFGID_VERDIVTYPE(CFGID_VERDIVTYPE);
			pCfgInit->ItemIns1ofN(cCFGID_VERDIVTYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_VERDIVTYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VERDIVTYPE_HELP), cEDTRelativeAdjustable, NULL);
			TConfigOptionCondition tVerCond;
			tVerCond.bstrID = cCFGID_SPLITTYPE;
			tVerCond.eConditionType = ECOCLessEqual;
			tVerCond.tValue = cESTBoth;
			pCfgInit->ItemOptionAdd(cCFGID_VERDIVTYPE, cEDTRelativeFixed, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_RELFIX), 1, &tVerCond);
			pCfgInit->ItemOptionAdd(cCFGID_VERDIVTYPE, cEDTRelativeAdjustable, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_RELADJ), 1, &tVerCond);
			pCfgInit->ItemOptionAdd(cCFGID_VERDIVTYPE, cEDTAbsoluteLTFixed, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_ABSFIX), 1, &tVerCond);
			pCfgInit->ItemOptionAdd(cCFGID_VERDIVTYPE, cEDTAbsoluteLTAdjustable, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_ABSADJ), 1, &tVerCond);
			pCfgInit->ItemOptionAdd(cCFGID_VERDIVTYPE, cEDTAbsoluteRBFixed, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_ABSBTMFIX), 1, &tVerCond);
			pCfgInit->ItemOptionAdd(cCFGID_VERDIVTYPE, cEDTAbsoluteRBAdjustable, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_ABSBTMADJ), 1, &tVerCond);
			// CFGID_VERRELATIVE
			{
				CComBSTR cCFGID_VERRELATIVE(CFGID_VERRELATIVE);
				TConfigOptionCondition tVerRelCond[2];
				tVerRelCond[0] = tVerCond;
				tVerRelCond[1].bstrID = cCFGID_VERDIVTYPE;
				tVerRelCond[1].eConditionType = ECOCLessEqual;
				tVerRelCond[1].tValue = cEDTRelativeAdjustable;
				pCfgInit->ItemInsRanged(cCFGID_VERRELATIVE, _SharedStringTable.GetStringAuto(IDS_CFGID_VERRELATIVE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VERRELATIVE_HELP), CConfigValue(50.0f), NULL, CConfigValue(0.0f), CConfigValue(100.0f), CConfigValue(1.0f), static_cast<ULONG>(itemsof(tVerRelCond)), tVerRelCond);
			}
			// CFGID_VERABSOLUTE
			{
				CComBSTR cCFGID_VERABSOLUTE(CFGID_VERABSOLUTE);
				TConfigOptionCondition tVerAbsCond[2];
				tVerAbsCond[0] = tVerCond;
				tVerAbsCond[1].bstrID = cCFGID_VERDIVTYPE;
				tVerAbsCond[1].eConditionType = ECOCGreaterEqual;
				tVerAbsCond[1].tValue = cEDTAbsoluteLTFixed;
				pCfgInit->ItemInsRanged(cCFGID_VERABSOLUTE, _SharedStringTable.GetStringAuto(IDS_CFGID_VERABSOLUTE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VERABSOLUTE_HELP), CConfigValue(200.0f), NULL, CConfigValue(0.0f), CConfigValue(2048.0f), CConfigValue(1.0f), static_cast<ULONG>(itemsof(tVerAbsCond)), tVerAbsCond);
			}
		}

		// CFGID_HORDIVTYPE
		{
			CComBSTR cCFGID_HORDIVTYPE(CFGID_HORDIVTYPE);
			pCfgInit->ItemIns1ofN(cCFGID_HORDIVTYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_HORDIVTYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HORDIVTYPE_HELP), cEDTRelativeAdjustable, NULL);
			TConfigOptionCondition tHorCond;
			tHorCond.bstrID = cCFGID_SPLITTYPE;
			tHorCond.eConditionType = ECOCGreaterEqual;
			tHorCond.tValue = cESTBoth;
			pCfgInit->ItemOptionAdd(cCFGID_HORDIVTYPE, cEDTRelativeFixed, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_RELFIX), 1, &tHorCond);
			pCfgInit->ItemOptionAdd(cCFGID_HORDIVTYPE, cEDTRelativeAdjustable, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_RELADJ), 1, &tHorCond);
			pCfgInit->ItemOptionAdd(cCFGID_HORDIVTYPE, cEDTAbsoluteLTFixed, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_ABSFIX), 1, &tHorCond);
			pCfgInit->ItemOptionAdd(cCFGID_HORDIVTYPE, cEDTAbsoluteLTAdjustable, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_ABSADJ), 1, &tHorCond);
			pCfgInit->ItemOptionAdd(cCFGID_HORDIVTYPE, cEDTAbsoluteRBFixed, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_ABSBTMFIX), 1, &tHorCond);
			pCfgInit->ItemOptionAdd(cCFGID_HORDIVTYPE, cEDTAbsoluteRBAdjustable, _SharedStringTable.GetStringAuto(IDS_CFGID_DIVTYPE_ABSBTMADJ), 1, &tHorCond);
			// CFGID_HORRELATIVE
			{
				CComBSTR cCFGID_HORRELATIVE(CFGID_HORRELATIVE);
				TConfigOptionCondition tHorRelCond[2];
				tHorRelCond[0] = tHorCond;
				tHorRelCond[1].bstrID = cCFGID_HORDIVTYPE;
				tHorRelCond[1].eConditionType = ECOCLessEqual;
				tHorRelCond[1].tValue = cEDTRelativeAdjustable;
				pCfgInit->ItemInsRanged(cCFGID_HORRELATIVE, _SharedStringTable.GetStringAuto(IDS_CFGID_HORRELATIVE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HORRELATIVE_HELP), CConfigValue(50.0f), NULL, CConfigValue(0.0f), CConfigValue(100.0f), CConfigValue(1.0f), static_cast<ULONG>(itemsof(tHorRelCond)), tHorRelCond);
			}
			// CFGID_HORABSOLUTE
			{
				CComBSTR cCFGID_HORABSOLUTE(CFGID_HORABSOLUTE);
				TConfigOptionCondition tHorAbsCond[2];
				tHorAbsCond[0] = tHorCond;
				tHorAbsCond[1].bstrID = cCFGID_HORDIVTYPE;
				tHorAbsCond[1].eConditionType = ECOCGreaterEqual;
				tHorAbsCond[1].tValue = cEDTAbsoluteLTFixed;
				pCfgInit->ItemInsRanged(cCFGID_HORABSOLUTE, _SharedStringTable.GetStringAuto(IDS_CFGID_HORABSOLUTE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HORABSOLUTE_HELP), CConfigValue(200.0f), NULL, CConfigValue(0.0f), CConfigValue(2048.0f), CConfigValue(1.0f), static_cast<ULONG>(itemsof(tHorAbsCond)), tHorAbsCond);
			}
		}

		// CFGID_SUBVIEWxx
		TConfigOptionCondition tSubCond;
		tSubCond.bstrID = cCFGID_SPLITTYPE;
		tSubCond.eConditionType = ECOCEqual;
		tSubCond.tValue = cESTBoth;
		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_SUBVIEWLT), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBVIEWLT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBVIEWLT_NAME), 0, NULL);
		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_SUBVIEWLB), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBVIEWLB_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBVIEWLB_NAME), 1, &tSubCond);
		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_SUBVIEWRT), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBVIEWRT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBVIEWRT_NAME), 1, &tSubCond);
		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_SUBVIEWRB), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBVIEWRB_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBVIEWRB_NAME), 0, NULL);

		CComObject<CConfigGUISplitter>* pGUI = NULL;
		CComObject<CConfigGUISplitter>::CreateInstance(&pGUI);
		CComPtr<IConfigCustomGUI> pTmp = pGUI;

		if (FAILED(pCfgInit->Finalize(pTmp)))
			return E_FAIL;

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactorySplitter::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComObject<CDesignerViewSplitter>* pWnd = NULL;
		CComObject<CDesignerViewSplitter>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pTmp = pWnd;
		pWnd->Init(a_pFrame, a_pStatusBar, a_pManager, a_pConfig, a_hParent, a_prcWindow, (a_nStyle & EDVWSBorderMask) == EDVWSBorder, a_pDoc, a_tLocaleID);

		*a_ppDVWnd = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactorySplitter::CheckSuitability(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	try
	{
		CConfigValue cSplitType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SPLITTYPE), &cSplitType);

		{
			CConfigValue cViewType;
			CComPtr<IConfig> pSubCfg;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBVIEWLT), &cViewType);
			a_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWLT), &pSubCfg);
			a_pManager->CheckSuitability(a_pManager, cViewType, pSubCfg, a_pDocument, a_pCallback);
		}

		if (cSplitType.operator LONG() == ESTBoth)
		{
			CConfigValue cViewType;
			CComPtr<IConfig> pSubCfg;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBVIEWRT), &cViewType);
			a_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWRT), &pSubCfg);
			a_pManager->CheckSuitability(a_pManager, cViewType, pSubCfg, a_pDocument, a_pCallback);
		}

		if (cSplitType.operator LONG() == ESTBoth)
		{
			CConfigValue cViewType;
			CComPtr<IConfig> pSubCfg;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBVIEWLB), &cViewType);
			a_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWLB), &pSubCfg);
			a_pManager->CheckSuitability(a_pManager, cViewType, pSubCfg, a_pDocument, a_pCallback);
		}

		{
			CConfigValue cViewType;
			CComPtr<IConfig> pSubCfg;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBVIEWRB), &cViewType);
			a_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWRB), &pSubCfg);
			a_pManager->CheckSuitability(a_pManager, cViewType, pSubCfg, a_pDocument, a_pCallback);
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
