// DocumentOperationSequence.cpp : Implementation of CDocumentOperationSequence

#include "stdafx.h"
#include "DocumentOperationSequence.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>

const OLECHAR CFGID_SEQ_STEPS[] = L"SeqSteps";
const OLECHAR CFGID_SEQ_TYPE[] = L"SeqType";
const LONG CFGVAL_SEQTYPE_OPERATION = 0;
const LONG CFGVAL_SEQTYPE_TRANSFORMATION = 1;
const OLECHAR CFGID_SEQ_OPERATION[] = L"SeqOperation";
const OLECHAR CFGID_SEQ_TRANSFORMATION[] = L"SeqTransformation";
const OLECHAR CFGID_SEQ_SKIPSTEP[] = L"SeqSkipStep";

#include "ConfigGUISequence.h"


// CDocumentOperationSequence

STDMETHODIMP CDocumentOperationSequence::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_OPERATIONSEQUENCE_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

extern GUID const TConfigGUISeqStepID = {0x1bfe5c7b, 0x1b42, 0x43ad, {0x8e, 0xba, 0xc3, 0x5c, 0xad, 0x34, 0x53, 0xc4}};

STDMETHODIMP CDocumentOperationSequence::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pStep;
		RWCoCreateInstance(pStep, __uuidof(ConfigWithDependencies));
		CComBSTR cCFGID_SEQ_TYPE(CFGID_SEQ_TYPE);
		pStep->ItemIns1ofN(cCFGID_SEQ_TYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_TYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_TYPE_DESC), CConfigValue(CFGVAL_SEQTYPE_OPERATION), NULL);
		pStep->ItemOptionAdd(cCFGID_SEQ_TYPE, CConfigValue(CFGVAL_SEQTYPE_OPERATION), _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_OPERATION_NAME), 0, NULL);
		CComQIPtr<ITransformationManager> pTransMgr(a_pManager);
		if (pTransMgr == NULL)
			RWCoCreateInstance(pTransMgr, __uuidof(TransformationManager));
		if (pTransMgr)
			pStep->ItemOptionAdd(cCFGID_SEQ_TYPE, CConfigValue(CFGVAL_SEQTYPE_TRANSFORMATION), _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_TRANSFORMATION_NAME), 0, NULL);

		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_SEQ_TYPE;
		tCond.eConditionType = ECOCEqual;
		tCond.tValue.eTypeID = ECVTInteger;
		tCond.tValue.iVal = CFGVAL_SEQTYPE_OPERATION;
		a_pManager->InsertIntoConfigAs(a_pManager, pStep, CComBSTR(CFGID_SEQ_OPERATION), _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_OPERATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_OPERATION_DESC), 1, &tCond);

		if (pTransMgr)
		{
			tCond.tValue.iVal = CFGVAL_SEQTYPE_TRANSFORMATION;
			pTransMgr->InsertIntoConfigAs(pTransMgr, pStep, CComBSTR(CFGID_SEQ_TRANSFORMATION), _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_TRANSFORMATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_TRANSFORMATION_DESC), 1, &tCond);
		}

		pStep->ItemInsSimple(CComBSTR(CFGID_SEQ_SKIPSTEP), CMultiLanguageString::GetAuto(L"[0409]Skip[0405]Přeskočit"), CMultiLanguageString::GetAuto(L"[0409]Skip this step and continue with the next one.[0405]Přeskočit tento krok a pokračovat dalším."), CConfigValue(false), NULL, 0, NULL);

		CConfigCustomGUI<&TConfigGUISeqStepID, CConfigGUISeqStepDlg>::FinalizeConfig(pStep);

		CComPtr<ISubConfigVector> pSteps;
		RWCoCreateInstance(pSteps, __uuidof(SubConfigVector));

		CComObject<CCustomName>* p2 = NULL;
		CComObject<CCustomName>::CreateInstance(&p2);
		CComPtr<IVectorItemName> pName = p2;

		if (FAILED(pSteps->InitName(pName, pStep)))
			return E_FAIL;

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_SEQ_STEPS), _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_STEPS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SEQ_STEPS_DESC), CConfigValue(0L), pSteps, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationSequence, CConfigGUISequenceDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationSequence::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		if (a_pConfig == NULL)
			return a_pDocument ? S_OK : S_FALSE;
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SEQ_STEPS), &cVal);
		if (cVal.operator LONG() <= 0)
			return S_FALSE;
		OLECHAR szTmp[64];
		swprintf(szTmp, L"%s\\00000000\\%s", CFGID_SEQ_STEPS, CFGID_SEQ_TYPE);
		a_pConfig->ItemValueGet(CComBSTR(szTmp), &cVal);
		if (cVal.operator LONG() == CFGVAL_SEQTYPE_OPERATION)
		{
			swprintf(szTmp, L"%s\\00000000\\%s", CFGID_SEQ_STEPS, CFGID_SEQ_OPERATION);
			CComBSTR bstrID(szTmp);
			CConfigValue cID;
			a_pConfig->ItemValueGet(bstrID, &cID);
			CComPtr<IConfig> pCfg;
			a_pConfig->SubConfigGet(bstrID, &pCfg);
			return a_pManager->CanActivate(a_pManager, a_pDocument, cID, pCfg, a_pStates);
		}
		else if (cVal.operator LONG() == CFGVAL_SEQTYPE_TRANSFORMATION)
		{
			swprintf(szTmp, L"%s\\00000000\\%s", CFGID_SEQ_STEPS, CFGID_SEQ_TRANSFORMATION);
			CComBSTR bstrID(szTmp);
			CConfigValue cID;
			a_pConfig->ItemValueGet(bstrID, &cID);
			CComPtr<IConfig> pCfg;
			a_pConfig->SubConfigGet(bstrID, &pCfg);
			CComQIPtr<ITransformationManager> pMgr(a_pManager);
			if (pMgr == NULL)
				RWCoCreateInstance(pMgr, __uuidof(TransformationManager));
			return pMgr->CanActivate(pMgr, a_pDocument, cID, pCfg, a_pStates);
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class ATL_NO_VTABLE CSequenceOperationContext :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationContext
{
public:
	CSequenceOperationContext() : m_nIndex(0), m_nRemaining(0)
	{
	}
	void Init(IOperationContext* a_pInternal)
	{
		m_pInternal = a_pInternal;
	}
	void Step(ULONG a_nIndex, ULONG a_nRemaining)
	{
		m_nIndex = a_nIndex;
		m_nRemaining = a_nRemaining;
	}


BEGIN_COM_MAP(CSequenceOperationContext)
	COM_INTERFACE_ENTRY(IOperationContext)
END_COM_MAP()

	// IOperationContext methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		if (m_pInternal) m_pInternal->StateGet(a_bstrCategoryName, a_iid, a_ppState);
		return E_NOTIMPL;
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (m_pInternal) m_pInternal->StateSet(a_bstrCategoryName, a_pState);
		return E_NOTIMPL;
	}
	STDMETHOD(IsCancelled)()
	{
		if (m_pInternal) m_pInternal->IsCancelled();
		return S_FALSE;
	}
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		if (a_pItemIndex) *a_pItemIndex = 0;
		if (a_pItemsRemaining) *a_pItemsRemaining = 0;
		if (a_pStepIndex) *a_pStepIndex = 0;
		if (a_pStepsRemaining) *a_pStepsRemaining = 0;
		if (m_pInternal) m_pInternal->GetOperationInfo(a_pItemIndex, a_pItemsRemaining, a_pStepIndex, a_pStepsRemaining);
		if (a_pStepIndex) *a_pStepIndex += m_nIndex;
		if (a_pStepsRemaining) *a_pStepsRemaining += m_nRemaining;
		return S_OK;
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		if (m_pInternal) m_pInternal->SetErrorMessage(a_pMessage);
		return S_FALSE;
	}

private:
	ULONG m_nIndex;
	ULONG m_nRemaining;
	CComPtr<IOperationContext> m_pInternal;
};


STDMETHODIMP CDocumentOperationSequence::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SEQ_STEPS), &cVal);
		LONG nSteps = cVal;

		CUndoBlock cBlock(a_pDocument);
		CComPtr<IDocument> pDoc(a_pDocument);
		CComPtr<ITransformationManager> pTranMgr;

		CComObject<CSequenceOperationContext>* pSOC = NULL;
		CComObject<CSequenceOperationContext>::CreateInstance(&pSOC);
		CComPtr<IOperationContext> pSOCTmp = pSOC;
		pSOC->Init(a_pStates);

		for (LONG i = 0; i < nSteps; ++i)
		{
			pSOC->Step(i, nSteps-i-1);

			OLECHAR szTmp[64];
			swprintf(szTmp, L"%s\\%08x\\%s", CFGID_SEQ_STEPS, i, CFGID_SEQ_SKIPSTEP);
			a_pConfig->ItemValueGet(CComBSTR(szTmp), &cVal);
			if (cVal)
				continue;

			swprintf(szTmp, L"%s\\%08x\\%s", CFGID_SEQ_STEPS, i, CFGID_SEQ_TYPE);
			a_pConfig->ItemValueGet(CComBSTR(szTmp), &cVal);
			if (cVal.operator LONG() == CFGVAL_SEQTYPE_OPERATION)
			{
				swprintf(szTmp, L"%s\\%08x\\%s", CFGID_SEQ_STEPS, i, CFGID_SEQ_OPERATION);
				CComBSTR bstrID(szTmp);
				CConfigValue cID;
				a_pConfig->ItemValueGet(bstrID, &cID);
				CComPtr<IConfig> pCfg;
				a_pConfig->SubConfigGet(bstrID, &pCfg);
				HRESULT hRes = a_pManager->Activate(a_pManager, pDoc, cID, pCfg, pSOC, a_hParent, a_tLocaleID);
				if (FAILED(hRes))
					return hRes;
			}
			else if (cVal.operator LONG() == CFGVAL_SEQTYPE_TRANSFORMATION)
			{
				swprintf(szTmp, L"%s\\%08x\\%s", CFGID_SEQ_STEPS, i, CFGID_SEQ_TRANSFORMATION);
				CComBSTR bstrID(szTmp);
				CConfigValue cID;
				a_pConfig->ItemValueGet(bstrID, &cID);
				CComPtr<IConfig> pCfg;
				a_pConfig->SubConfigGet(bstrID, &pCfg);
				if (pTranMgr == NULL)
				{
					a_pManager->QueryInterface(&pTranMgr);
					if (pTranMgr == NULL)
						RWCoCreateInstance(pTranMgr, __uuidof(TransformationManager));
					if (pTranMgr == NULL)
						return E_FAIL;
				}
				CComPtr<IDocumentBase> pBase;
				RWCoCreateInstance(pBase, __uuidof(DocumentBase));
				HRESULT hRes = pTranMgr->Activate(pTranMgr, pDoc, cID, pCfg, pSOC, a_hParent, a_tLocaleID, NULL, pBase);
				if (FAILED(hRes)) return hRes;
				CComQIPtr<IDocument> pNewDoc(pBase);
				std::swap(pNewDoc.p, pDoc.p);
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationSequence::Visit(IOperationManager* a_pManager, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor)
{
	try
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SEQ_STEPS), &cVal);
		LONG nSteps = cVal;

		CComPtr<ITransformationManager> pTranMgr;

		for (LONG i = 0; i < nSteps; ++i)
		{
			OLECHAR szTmp[64];
			swprintf(szTmp, L"%s\\%08x\\%s", CFGID_SEQ_STEPS, i, CFGID_SEQ_TYPE);
			a_pConfig->ItemValueGet(CComBSTR(szTmp), &cVal);
			if (cVal.operator LONG() == CFGVAL_SEQTYPE_OPERATION)
			{
				swprintf(szTmp, L"%s\\%08x\\%s", CFGID_SEQ_STEPS, i, CFGID_SEQ_OPERATION);
				CComBSTR bstrID(szTmp);
				CConfigValue cID;
				a_pConfig->ItemValueGet(bstrID, &cID);
				CComPtr<IConfig> pCfg;
				a_pConfig->SubConfigGet(bstrID, &pCfg);
				HRESULT hRes = a_pManager->Visit(a_pManager, cID, pCfg, a_pVisitor);
				//if (FAILED(hRes)) return hRes;
			}
			else if (cVal.operator LONG() == CFGVAL_SEQTYPE_TRANSFORMATION)
			{
				swprintf(szTmp, L"%s\\%08x\\%s", CFGID_SEQ_STEPS, i, CFGID_SEQ_TRANSFORMATION);
				CComBSTR bstrID(szTmp);
				CConfigValue cID;
				a_pConfig->ItemValueGet(bstrID, &cID);
				CComPtr<IConfig> pCfg;
				a_pConfig->SubConfigGet(bstrID, &pCfg);
				if (pTranMgr == NULL)
				{
					a_pManager->QueryInterface(&pTranMgr);
					if (pTranMgr == NULL)
						RWCoCreateInstance(pTranMgr, __uuidof(TransformationManager));
					if (pTranMgr == NULL)
						return E_FAIL;
				}
				HRESULT hRes = pTranMgr->Visit(pTranMgr, cID, pCfg, a_pVisitor);
				//if (FAILED(hRes)) return hRes;
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationSequence::CCustomNameString::GetLocalized(LCID a_tLCID, BSTR *a_pbstrString)
{
	try
	{
		*a_pbstrString = NULL;
		CComBSTR bstr;
		if (m_pItemConfig)
		{
			CConfigValue cVal;
			m_pItemConfig->ItemValueGet(CComBSTR(CFGID_SEQ_TYPE), &cVal);
			CComBSTR bstrID(cVal.operator LONG() == CFGVAL_SEQTYPE_TRANSFORMATION ? CFGID_SEQ_TRANSFORMATION : CFGID_SEQ_OPERATION);
			m_pItemConfig->ItemValueGet(bstrID, &cVal);
			CComPtr<IConfigItem> pItem;
			m_pItemConfig->ItemGetUIInfo(bstrID, __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
			if (pItem)
			{
				CComPtr<ILocalizedString> pStr;
				pItem->ValueGetName(cVal, &pStr);
				if (pStr)
				{
					pStr->GetLocalized(a_tLCID, &bstr);
				}
			}
		}
		OLECHAR sz[256] = L"";
		swprintf(sz, bstr == NULL ? L"%i" : L"%i: %s", m_nIndex+1, bstr);
		*a_pbstrString = CComBSTR(sz).Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrString ? E_UNEXPECTED : E_POINTER;
	}
}

