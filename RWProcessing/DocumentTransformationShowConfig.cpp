// DocumentTransformationShowConfig.cpp : Implementation of CDocumentTransformationShowConfig

#include "stdafx.h"
#include "DocumentTransformationShowConfig.h"
#include "ShowConfigWithPreviewDlg.h"
#include <SharedStringTable.h>


const OLECHAR CFGID_CFG_TRANSFORMATION[] = L"CfgTransformation";

#include "ConfigGUIShowTransformationConfig.h"


// CDocumentTransformationShowConfig

STDMETHODIMP CDocumentTransformationShowConfig::NameGet(ITransformationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_SHOWCONFIG);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTransformationShowConfig::ConfigCreate(ITransformationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		a_pManager->InsertIntoConfigAs(a_pManager, pCfg, CComBSTR(CFGID_CFG_TRANSFORMATION), _SharedStringTable.GetStringAuto(IDS_PIPE_TRANSFORMATION_NAME), _SharedStringTable.GetStringAuto(IDS_PIPE_TRANSFORMATION_DESC), 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_DLGSIZEX), _SharedStringTable.GetStringAuto(IDS_CFG_DLGSIZEX), _SharedStringTable.GetStringAuto(IDS_CFG_DLGSIZEX), CConfigValue(-1.0f), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_DLGSIZEY), _SharedStringTable.GetStringAuto(IDS_CFG_DLGSIZEY), _SharedStringTable.GetStringAuto(IDS_CFG_DLGSIZEY), CConfigValue(-1.0f), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_CAPTION), CMultiLanguageString::GetAuto(L"[0409]Window caption[0405]Titulek okna"), CMultiLanguageString::GetAuto(L"[0409]Text in the caption of the window displayed during execution.[0405]Text v titulku onka konfiguračního dialogu."), CConfigValue(L"[0409]Configure Transformation[0405]Konfigurovat transformaci"), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_HELPTOPIC), CMultiLanguageString::GetAuto(L"[0409]Help topic[0405]Téma nápovědy"), CMultiLanguageString::GetAuto(L"[0409]Path to a html help topic that will be displayed if user clicks on a Help button. If this field is left blank, the Help button will be hidden.[0405]Cesta k tématu v html nápovědě, který bude zobrazen po kliknutí na tlačítko Nápověda. Pokud je toto pole prázdné, tlačítko bude skryté."), CConfigValue(L""), NULL, 0, NULL);

		CComBSTR cCFGID_ICONID(CFGID_CFG_ICONID);
		CComPtr<IConfigItemCustomOptions> pCustIconIDs;
		RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
		if (pCustIconIDs != NULL)
			pCfg->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME2), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC2), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
		else
			pCfg->ItemInsSimple(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), NULL, 0, NULL);

		CComQIPtr<IViewManager> pViewMgr(a_pManager);
		if (pViewMgr == NULL)
			RWCoCreateInstance(pViewMgr, __uuidof(ViewManager));
		pViewMgr->InsertIntoConfigAs(pViewMgr, pCfg, CComBSTR(CFGID_CFG_PREVIEW), _SharedStringTable.GetStringAuto(IDS_CFG_PREVIEW_NAME), _SharedStringTable.GetStringAuto(IDS_CFG_PREVIEW_DESC), 0, NULL);

		CComPtr<ISubConfig> pHistory;
		RWCoCreateInstance(pHistory, __uuidof(ConfigInMemory));
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_HISTORY), CMultiLanguageString::GetAuto(L"[0409]History[0405]Historie"), CMultiLanguageString::GetAuto(L"[0409]If enabled, previous selected values will be remembered.[0405]Je-li povoleno, budou se zaznamenávat dříve vybraná nastavení."), CConfigValue(true), pHistory, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentTransformationShowConfig, CConfigGUIShowTransformationConfigDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTransformationShowConfig::CanActivate(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		CComBSTR cCFGID_CFG_TRANSFORMATION(CFGID_CFG_TRANSFORMATION);
		CConfigValue cOperID;
		a_pConfig->ItemValueGet(cCFGID_CFG_TRANSFORMATION, &cOperID);
		CComPtr<IConfig> pOperCfg;
		//a_pConfig->SubConfigGet(cCFGID_CFG_TRANSFORMATION, &pOperCfg);
		return a_pManager->CanActivate(a_pManager, a_pDocument, cOperID, pOperCfg, a_pStates);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

struct CTransformationPreviewMaker : public IPreviewMaker
{
	CTransformationPreviewMaker(ITransformationManager* a_pManager, IDocument* a_pDocument, TConfigValue const& a_tVal, IConfig* a_pCfg, IOperationContext* a_pStates) :
		m_pManager(a_pManager), m_pDocument(a_pDocument), m_tVal(a_tVal), m_pCfg(a_pCfg), m_pStates(a_pStates)
	{
	}

	HRESULT MakePreview(RWHWND a_hParent, LCID a_tLocaleID, IDocument** a_ppPreviewDoc)
	{
		CComPtr<IDocumentBase> pBase;
		RWCoCreateInstance(pBase, __uuidof(DocumentBase));
		HRESULT hRes = m_pManager->Activate(m_pManager, m_pDocument, &m_tVal, m_pCfg, m_pStates, a_hParent, a_tLocaleID, NULL, pBase);
		if (FAILED(hRes)) return hRes;
		return pBase->QueryInterface(a_ppPreviewDoc);
	}

private:
	ITransformationManager* m_pManager;
	IDocument* m_pDocument;
	TConfigValue m_tVal;
	IConfig* m_pCfg;
	IOperationContext* m_pStates;
};


STDMETHODIMP CDocumentTransformationShowConfig::Activate(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		ULONG nItemIndex = 0;
		ULONG nStepsRemaining = 1;
		if (a_pStates) a_pStates->GetOperationInfo(&nItemIndex, NULL, NULL, &nStepsRemaining);
		bool bInSequence = nItemIndex != 0;

		CComBSTR cCFGID_CFG_TRANSFORMATION(CFGID_CFG_TRANSFORMATION);
		CConfigValue cOperID;
		a_pConfig->ItemValueGet(cCFGID_CFG_TRANSFORMATION, &cOperID);
		CComPtr<IConfig> pOperCfg;
		a_pConfig->SubConfigGet(cCFGID_CFG_TRANSFORMATION, &pOperCfg);

		CComPtr<IConfig> pCopy;
		pOperCfg->DuplicateCreate(&pCopy);

		if (!bInSequence && a_hParent)
		{
			CTransformationPreviewMaker cPreviewMaker(a_pManager, a_pDocument, cOperID, pCopy, a_pStates);
			CComPtr<IConfigDescriptor> pOA;
			RWCoCreateInstance(pOA, cOperID.operator const GUID &());
			CConfigValue cRuntimeConfig;
			CShowConfigWithPreviewDlg<> cDlg(a_tLocaleID, pCopy, a_pConfig, CComQIPtr<ITransformationManager>(a_pManager), pOA, CComQIPtr<IViewManager>(a_pManager), a_pDocument, &cPreviewMaker);
			if (cDlg.DoModal(a_hParent) != IDOK)
				return E_RW_CANCELLEDBYUSER;

			CopyConfigValues(pOperCfg, pCopy);
		}

		CWaitCursor cWait;
		HRESULT hRes = a_pManager->Activate(a_pManager, a_pDocument, cOperID, pCopy, a_pStates, a_hParent, a_tLocaleID, a_bstrPrefix, a_pBase);

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

