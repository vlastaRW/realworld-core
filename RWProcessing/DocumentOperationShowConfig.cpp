// DocumentOperationShowConfig.cpp : Implementation of CDocumentOperationShowConfig

#include "stdafx.h"
#include "DocumentOperationShowConfig.h"
#include <SharedStringTable.h>
#include "ShowConfigWithPreviewDlg.h"


const OLECHAR CFGID_CFG_OPERATION[] = L"CfgOperation";
extern __declspec(selectany) OLECHAR const CFGID_CFG_PARSCALE[] = L"ParScale";
static LONG const CFGVAL_PS_LINEAR = 0;
static LONG const CFGVAL_PS_LOGARITHMIC = 1;
extern __declspec(selectany) OLECHAR const CFGID_CFG_PARAMETER[] = L"Parameter";
extern __declspec(selectany) OLECHAR const CFGID_CFG_PARLOWER[] = L"ParLower";
extern __declspec(selectany) OLECHAR const CFGID_CFG_PARUPPER[] = L"ParUpper";

#include "ConfigGUIShowOperationConfig.h"


// CDocumentOperationShowConfig

STDMETHODIMP CDocumentOperationShowConfig::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
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

STDMETHODIMP CDocumentOperationShowConfig::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		a_pManager->InsertIntoConfigAs(a_pManager, pCfg, CComBSTR(CFGID_CFG_OPERATION), _SharedStringTable.GetStringAuto(IDS_PIPE_OPERATION_NAME), _SharedStringTable.GetStringAuto(IDS_PIPE_OPERATION_DESC), 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_DLGSIZEX), _SharedStringTable.GetStringAuto(IDS_CFG_DLGSIZEX), _SharedStringTable.GetStringAuto(IDS_CFG_DLGSIZEX), CConfigValue(-1.0f), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_DLGSIZEY), _SharedStringTable.GetStringAuto(IDS_CFG_DLGSIZEY), _SharedStringTable.GetStringAuto(IDS_CFG_DLGSIZEY), CConfigValue(-1.0f), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_CAPTION), CMultiLanguageString::GetAuto(L"[0409]Window caption[0405]Titulek okna"), CMultiLanguageString::GetAuto(L"[0409]Text in the caption of the window displayed during execution.[0405]Text v titulku onka konfiguračního dialogu."), CConfigValue(L"[0409]Configure Operation[0405]Konfigurovat operaci"), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_HELPTOPIC), CMultiLanguageString::GetAuto(L"[0409]Help topic[0405]Téma nápovědy"), CMultiLanguageString::GetAuto(L"[0409]Path to a html help topic that will be displayed if user clicks on a Help button. If this field is left blank, the Help button will be hidden.[0405]Cesta k tématu v html nápovědě, který bude zobrazen po kliknutí na tlačítko Nápověda. Pokud je toto pole prázdné, tlačítko bude skryté."), CConfigValue(L""), NULL, 0, NULL);
		CComBSTR cCFGID_CFG_PREVIEWMODE(CFGID_CFG_PREVIEWMODE);
		pCfg->ItemIns1ofN(cCFGID_CFG_PREVIEWMODE, CMultiLanguageString::GetAuto(L"[0409]Display mode[0405]Způsob zobrazení"), CMultiLanguageString::GetAuto(L"[0409]Controls how is the original and the preview of the processed image displayed.[0405]Určuje, jak je zobrazen originál a náhled na zpracovný obrázek."), CConfigValue(CFGVAL_PM_AUTOSELECT), NULL);
		pCfg->ItemOptionAdd(cCFGID_CFG_PREVIEWMODE, CConfigValue(CFGVAL_PM_PROCESSED), CMultiLanguageString::GetAuto(L"[0409]Processed[0405]Výsledek"), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_CFG_PREVIEWMODE, CConfigValue(CFGVAL_PM_AUTOSELECT), CMultiLanguageString::GetAuto(L"[0409]Default[0405]Výchozí"), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_CFG_PREVIEWMODE, CConfigValue(CFGVAL_PM_SPLITTED), CMultiLanguageString::GetAuto(L"[0409]Both[0405]Oba"), 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_SPLITPOS), NULL, NULL, CConfigValue(-1.0f), NULL, 0, NULL);

		CComBSTR cCFGID_CFG_PARAMETER(CFGID_CFG_PARAMETER);
		CConfigValue cEmpty(L"");
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_PARAMETER), _SharedStringTable.GetStringAuto(IDS_CFG_PARAMETER_NAME), _SharedStringTable.GetStringAuto(IDS_CFG_PARAMETER_DESC), cEmpty, NULL, 0, NULL);
		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_CFG_PARAMETER;
		tCond.eConditionType = ECOCNotEqual;
		tCond.tValue = cEmpty;
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_PARLOWER), _SharedStringTable.GetStringAuto(IDS_CFG_PARLOWER_NAME), _SharedStringTable.GetStringAuto(IDS_CFG_PARLOWER_DESC), CConfigValue(1.0f), NULL, 1, &tCond);
		pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_PARUPPER), _SharedStringTable.GetStringAuto(IDS_CFG_PARUPPER_NAME), _SharedStringTable.GetStringAuto(IDS_CFG_PARUPPER_DESC), CConfigValue(1.0f), NULL, 1, &tCond);
		CComBSTR cCFGID_CFG_PARSCALE(CFGID_CFG_PARSCALE);
		pCfg->ItemIns1ofN(cCFGID_CFG_PARSCALE, _SharedStringTable.GetStringAuto(IDS_CFG_PARSCALE_NAME), _SharedStringTable.GetStringAuto(IDS_CFG_PARSCALE_DESC), CConfigValue(CFGVAL_PS_LINEAR), NULL);
		pCfg->ItemOptionAdd(cCFGID_CFG_PARSCALE, CConfigValue(CFGVAL_PS_LINEAR), _SharedStringTable.GetStringAuto(IDS_CFGVAL_LINEAR), 1, &tCond);
		pCfg->ItemOptionAdd(cCFGID_CFG_PARSCALE, CConfigValue(CFGVAL_PS_LOGARITHMIC), _SharedStringTable.GetStringAuto(IDS_CFGVAL_LOGARITHMIC), 1, &tCond);

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

		CConfigCustomGUI<&CLSID_DocumentOperationShowConfig, CConfigGUIShowOperationConfigDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationShowConfig::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		CComBSTR cCFGID_CFG_OPERATION(CFGID_CFG_OPERATION);
		CConfigValue cOperID;
		a_pConfig->ItemValueGet(cCFGID_CFG_OPERATION, &cOperID);
		CComPtr<IConfig> pOperCfg;
		//a_pConfig->SubConfigGet(cCFGID_CFG_OPERATION, &pOperCfg);
		return a_pManager->CanActivate(a_pManager, a_pDocument, cOperID, pOperCfg, a_pStates);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

struct COperationPreviewMaker : public IPreviewMaker
{
	COperationPreviewMaker(IOperationManager* a_pManager, IDocument* a_pDocument, TConfigValue const& a_tVal, IConfig* a_pCfg, IOperationContext* a_pStates) :
		m_pManager(a_pManager), m_pDocument(a_pDocument), m_tVal(a_tVal), m_pCfg(a_pCfg), m_pStates(a_pStates)
	{
	}

	HRESULT MakePreview(RWHWND a_hParent, LCID a_tLocaleID, IDocument** a_ppPreviewDoc)
	{
		CComPtr<IDocumentBase> pBase;
		RWCoCreateInstance(pBase, __uuidof(DocumentBase));
		CComPtr<IConfig> pCfg;
		if (m_pCfg) m_pCfg->DuplicateCreate(&pCfg);
		if (FAILED(m_pDocument->DocumentCopy(NULL, pBase, &m_tVal.guidVal, pCfg)))
			return E_NOTIMPL;
		pBase->QueryInterface(a_ppPreviewDoc);
		if (NULL == *a_ppPreviewDoc)
			return E_NOTIMPL;
		return m_pManager->Activate(m_pManager, *a_ppPreviewDoc, &m_tVal, pCfg, m_pStates, a_hParent, a_tLocaleID);
	}

private:
	IOperationManager* m_pManager;
	IDocument* m_pDocument;
	TConfigValue m_tVal;
	IConfig* m_pCfg;
	IOperationContext* m_pStates;
};

#include "RuntimeConfigSimple.h"

class ATL_NO_VTABLE CPreviewOperationContext :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationContext
{
public:
	CPreviewOperationContext()
	{
	}
	void Init(IOperationContext* a_pInternal)
	{
		m_pInternal = a_pInternal;
	}


BEGIN_COM_MAP(CPreviewOperationContext)
	COM_INTERFACE_ENTRY(IOperationContext)
END_COM_MAP()

	// IOperationContext methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
		if (i == m_cStates.end())
			return m_pInternal ? m_pInternal->StateGet(a_bstrCategoryName, a_iid, a_ppState) : E_RW_ITEMNOTFOUND;
		return i->second->QueryInterface(a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (a_pState)
			m_cStates[a_bstrCategoryName] = a_pState;
		else
			m_cStates.erase(a_bstrCategoryName);
		return S_OK;
	}
	STDMETHOD(IsCancelled)()
	{
		return m_pInternal ? m_pInternal->IsCancelled() : S_FALSE;
	}
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		return m_pInternal ? m_pInternal->GetOperationInfo(a_pItemIndex, a_pItemsRemaining, a_pStepIndex, a_pStepsRemaining) : E_NOTIMPL;
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		return m_pInternal ? m_pInternal->SetErrorMessage(a_pMessage) : E_NOTIMPL;
	}

private:
	typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

private:
	CStates m_cStates;
	CComPtr<IOperationContext> m_pInternal;
};

STDMETHODIMP CDocumentOperationShowConfig::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComBSTR cCFGID_CFG_OPERATION(CFGID_CFG_OPERATION);
		CConfigValue cOperID;
		a_pConfig->ItemValueGet(cCFGID_CFG_OPERATION, &cOperID);
		CComPtr<IConfig> pOperCfg;
		a_pConfig->SubConfigGet(cCFGID_CFG_OPERATION, &pOperCfg);

		
		ULONG nItemIndex = 0;
		ULONG nStepsRemaining = 1;
		ULONG nItemsRemaining = 0;
		if (a_pStates) a_pStates->GetOperationInfo(&nItemIndex, &nItemsRemaining, NULL, &nStepsRemaining);
		bool bInSequence = nItemIndex != 0;

		CConfigValue cParName;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CFG_PARAMETER), &cParName);
		if (cParName.operator BSTR()[0])
		{
			CConfigValue cParVal;
			pOperCfg->ItemValueGet(cParName, &cParVal);
			if (cParVal.TypeGet() == ECVTInteger || cParVal.TypeGet() == ECVTFloat)
			{
				CComObject<CSimplifiedConfig>* pSC = NULL;
				CComObject<CSimplifiedConfig>::CreateInstance(&pSC);
				CComPtr<IConfig> pSimCfg = pSC;
				GUID tGUID = GUID_NULL;
				CoCreateGuid(&tGUID);
				CConfigValue cMin;
				a_pConfig->ItemValueGet(CComBSTR(CFGID_CFG_PARLOWER), &cMin);
				CConfigValue cMax;
				a_pConfig->ItemValueGet(CComBSTR(CFGID_CFG_PARUPPER), &cMax);
				CConfigValue cScale;
				a_pConfig->ItemValueGet(CComBSTR(CFGID_CFG_PARSCALE), &cScale);
				pSC->Init(pOperCfg, cParName, cMin, cMax, cScale, tGUID);
				pOperCfg = pSimCfg;
			}
		}

		CComPtr<IConfig> pCopy;
		pOperCfg->DuplicateCreate(&pCopy);

		if (!bInSequence && a_hParent)
		{
			CComObjectStackEx<CPreviewOperationContext> cOpCtx;
			cOpCtx.Init(a_pStates);
			COperationPreviewMaker cPreviewMaker(a_pManager, a_pDocument, cOperID, pCopy, &cOpCtx);
			CComPtr<IConfigDescriptor> pOA;
			RWCoCreateInstance(pOA, cOperID.operator const GUID &());
			CConfigValue cRuntimeConfig;
			int nDlgRes = nStepsRemaining && nItemsRemaining == 0 ?
				CShowConfigWithPreviewDlg<IDCONTINUE>(a_tLocaleID, pCopy, a_pConfig, a_pManager, pOA, CComQIPtr<IViewManager>(a_pManager), a_pDocument, &cPreviewMaker).DoModal(a_hParent) :
				CShowConfigWithPreviewDlg<>(a_tLocaleID, pCopy, a_pConfig, a_pManager, pOA, CComQIPtr<IViewManager>(a_pManager), a_pDocument, &cPreviewMaker).DoModal(a_hParent);
			switch (nDlgRes)
			{
			case IDOK:
				break;
			case IDCONTINUE:
				return S_FALSE;
			default:
				return E_RW_CANCELLEDBYUSER;
			}

			CopyConfigValues(pOperCfg, pCopy);
		}

		CWaitCursor cWait;
		HRESULT hRes = a_pManager->Activate(a_pManager, a_pDocument, cOperID, pCopy, a_pStates, a_hParent, a_tLocaleID);

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

