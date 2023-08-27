// DocumentOperationExtractSubDocument.cpp : Implementation of CDocumentOperationExtractSubDocument

#include "stdafx.h"
#include "DocumentOperationExtractSubDocument.h"

#include <SharedStringTable.h>


// CDocumentOperationExtractSubDocument

STDMETHODIMP CDocumentOperationExtractSubDocument::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_DOCOPEXTRACT_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

const OLECHAR CFGID_SUBDOCSOURCE[] = L"SubDocSource";
const OLECHAR CFGID_SUBDOCMANUAL[] = L"SubDocManual";
const OLECHAR CFGID_SUBDOCCATID[] = L"SubDocCategoryID";
const OLECHAR CFGID_EXTSD_SUBOP[] = L"SubOperation";
const LONG CFGVAL_SUBDOCSRC_ALLSUBDOCS = -1;
const LONG CFGVAL_SUBDOCSRC_MANUAL = 0;
const LONG CFGVAL_SUBDOCSRC_CATID = 1;

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIExtractSubDocumentDlg :
	public CCustomConfigWndImpl<CConfigGUIExtractSubDocumentDlg>,
	public CDialogResize<CConfigGUIExtractSubDocumentDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_EXTRACTSUBDOC };

	BEGIN_MSG_MAP(CConfigGUIExtractSubDocumentDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIExtractSubDocumentDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIExtractSubDocumentDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIExtractSubDocumentDlg)
		DLGRESIZE_CONTROL(IDC_CGEXTSUBDOC_EXTBY, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGEXTSUBDOC_SYNCGROUP, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGEXTSUBDOC_MANUALID, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGEXTSUBDOC_SUBOP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGEXTSUBDOC_SUBCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIExtractSubDocumentDlg)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGEXTSUBDOC_MANUALID, CFGID_SUBDOCMANUAL)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGEXTSUBDOC_SYNCGROUP, CFGID_SUBDOCCATID)
		CONFIGITEM_COMBOBOX(IDC_CGEXTSUBDOC_EXTBY, CFGID_SUBDOCSOURCE)
		CONFIGITEM_COMBOBOX(IDC_CGEXTSUBDOC_SUBOP, CFGID_EXTSD_SUBOP)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CWindow wnd(GetDlgItem(IDC_CGEXTSUBDOC_SUBCONFIG));
		RECT rc;
		wnd.GetWindowRect(&rc);
		ScreenToClient(&rc);
		wnd.DestroyWindow();

		RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
		m_pConfigWnd->Create(m_hWnd, &rc, IDC_CGEXTSUBDOC_SUBCONFIG, m_tLocaleID, TRUE, ECWBMMarginAndOutline);
		CComPtr<IConfig> pDeviceCfg;
		M_Config()->SubConfigGet(CComBSTR(CFGID_EXTSD_SUBOP), &pDeviceCfg);
		m_pConfigWnd->ConfigSet(pDeviceCfg, M_Mode());

		DlgResize_Init(false, false, 0);

		return 1;
	}

private:
	CComPtr<IConfigWnd> m_pConfigWnd;
};

STDMETHODIMP CDocumentOperationExtractSubDocument::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		// sub-doc source
		CComBSTR cCFGID_SUBDOCSOURCE(CFGID_SUBDOCSOURCE);
		CConfigValue cCFGVAL_SUBDOCSRC_MANUAL(CFGVAL_SUBDOCSRC_MANUAL);
		CConfigValue cCFGVAL_SUBDOCSRC_CATID(CFGVAL_SUBDOCSRC_CATID);
		pCfg->ItemIns1ofN(cCFGID_SUBDOCSOURCE, _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCSOURCE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCSOURCE_DESC), CConfigValue(CFGVAL_SUBDOCSRC_CATID), NULL);
		pCfg->ItemOptionAdd(cCFGID_SUBDOCSOURCE, CConfigValue(CFGVAL_SUBDOCSRC_ALLSUBDOCS), _SharedStringTable.GetStringAuto(IDS_CFGVAL_SUBDOCSRC_ALLSUBDOCS), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_SUBDOCSOURCE, CConfigValue(CFGVAL_SUBDOCSRC_MANUAL), _SharedStringTable.GetStringAuto(IDS_CFGVAL_SUBDOCSRC_MANUAL), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_SUBDOCSOURCE, CConfigValue(CFGVAL_SUBDOCSRC_CATID), _SharedStringTable.GetStringAuto(IDS_CFGVAL_SUBDOCSRC_CATID), 0, NULL);

		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_SUBDOCSOURCE;
		tCond.eConditionType = ECOCEqual;
		tCond.tValue = cCFGVAL_SUBDOCSRC_MANUAL;
		CComBSTR cCFGID_SUBDOCMANUAL(CFGID_SUBDOCMANUAL);
		pCfg->ItemInsSimple(cCFGID_SUBDOCMANUAL, _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCMANUAL_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCMANUAL_DESC), CConfigValue(L""), NULL, 1, &tCond);

		tCond.eConditionType = ECOCGreaterEqual;
		tCond.tValue = cCFGVAL_SUBDOCSRC_CATID;
		CComBSTR cCFGID_SUBDOCCATID(CFGID_SUBDOCCATID);
		pCfg->ItemInsSimple(cCFGID_SUBDOCCATID, _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCCATID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCCATID_DESC), CConfigValue(L"SUBDOC"), NULL, 1, &tCond);

		// sub-operation
		a_pManager->InsertIntoConfigAs(a_pManager, pCfg, CComBSTR(CFGID_EXTSD_SUBOP), _SharedStringTable.GetStringAuto(IDS_CFGID_EXTSD_SUBOP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_EXTSD_SUBOP_DESC), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationExtractSubDocument, CConfigGUIExtractSubDocumentDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void GetSelectedDocs(ISubDocumentsMgr* a_pSDM, LPWSTR a_pszIDs, IOperationContext* a_pStates, IEnumUnknownsInit* a_pItems, bool a_bAllSubDocs, bool a_bFirstOnly = false)
{
	while (*a_pszIDs == L' ' || *a_pszIDs == L',') ++a_pszIDs;
	LPWSTR psz = a_pszIDs;
	while (*psz && *psz != L' ' && *psz != L',') ++psz;
	if (psz == a_pszIDs)
		return;

	CComBSTR bstrPref;
	a_pSDM->StatePrefix(&bstrPref);
	ULONG const nPrefLen = bstrPref.Length();
	CComBSTR bstrID(nPrefLen+(psz-a_pszIDs));
	CopyMemory(bstrID.m_str, bstrPref.m_str, nPrefLen*sizeof(wchar_t));
	CopyMemory(bstrID.m_str+nPrefLen, a_pszIDs, (psz-a_pszIDs)*sizeof(wchar_t));

	while (*psz == L' ' || *psz == L',') ++psz;

	CComPtr<IEnumUnknowns> pItems;
	ULONG nItems = 0;
	if (a_bAllSubDocs)
	{
		a_pSDM->ItemsEnum(NULL, &pItems);
		if (pItems) pItems->Size(&nItems);
	}
	else
	{
		CComPtr<ISharedState> pState;
		a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		if (pState)
		{
			a_pSDM->StateUnpack(pState, &pItems);
			if (pItems) pItems->Size(&nItems);
		}
		else
		{
			a_pSDM->ItemsEnum(NULL, &pItems);
			if (pItems)
			{
				pItems->Size(&nItems);
				if (nItems) nItems = 1;
			}
		}
	}
	for (ULONG i = 0; i < nItems; ++i)
	{
		CComPtr<ISubDocumentID> pID;
		pItems->Get(i, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pID));
		if (pID == NULL)
			continue;
		CComPtr<IDocument> pDoc;
		pID->SubDocumentGet(&pDoc);
		if (pDoc == NULL)
			continue;
		if (*psz)
		{
			CComPtr<ISubDocumentsMgr> pSDM;
			pDoc->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM));
			if (pSDM)
				GetSelectedDocs(pSDM, psz, a_pStates, a_pItems, nItems > 1, a_bFirstOnly);
		}
		else
		{
			a_pItems->Insert(pDoc);
			if (a_bFirstOnly)
				return;
		}
	}
}

STDMETHODIMP CDocumentOperationExtractSubDocument::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		if (a_pDocument == NULL)
			return E_POINTER;

		if (a_pConfig == NULL)
			return S_OK;

		CComBSTR cCFGID_EXTSD_SUBOP(CFGID_EXTSD_SUBOP);
		CConfigValue cOperID;
		a_pConfig->ItemValueGet(cCFGID_EXTSD_SUBOP, &cOperID);
		CComPtr<IConfig> pOperCfg;
		a_pConfig->SubConfigGet(cCFGID_EXTSD_SUBOP, &pOperCfg);

		CComPtr<ISubDocumentsMgr> pSDM;
		a_pDocument->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM));
		if (pSDM == NULL)
			return a_pManager->CanActivate(a_pManager, a_pDocument, cOperID, pOperCfg, a_pStates);//S_FALSE;

		CConfigValue cSubDocSrc;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBDOCSOURCE), &cSubDocSrc);
		if (cSubDocSrc.operator LONG() == CFGVAL_SUBDOCSRC_CATID)
		{
			CConfigValue cStateID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBDOCCATID), &cStateID);

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
			GetSelectedDocs(pSDM, cStateID, a_pStates, pItems, false);

			HRESULT hRes = S_FALSE;
			ULONG nSize = 0;
			pItems->Size(&nSize);
			for (ULONG i = 0; i < nSize; ++i)
			{
				CComPtr<IDocument> pDoc;
				pItems->Get(i, &pDoc);
				if (pDoc && S_OK == a_pManager->CanActivate(a_pManager, pDoc, cOperID, pOperCfg, a_pStates))
				{
					hRes = S_OK;
					break;
				}
			}
			return hRes;
		}
		else if (cSubDocSrc.operator LONG() == CFGVAL_SUBDOCSRC_ALLSUBDOCS)
		{
			CComPtr<IEnumUnknowns> pItems;
			pSDM->ItemsEnum(NULL, &pItems);
			ULONG n = 0;
			return pItems && SUCCEEDED(pItems->Size(&n)) && n ? S_OK : S_FALSE;
		}
		else
		{
			CConfigValue cSubDocID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBDOCMANUAL), &cSubDocID);
			CComPtr<ISubDocumentID> pID;
			pSDM->FindByName(cSubDocID.operator BSTR(), &pID);
			CComPtr<IDocument> pSubDoc;
			if (pID != NULL)
				pID->SubDocumentGet(&pSubDoc);
			if (pSubDoc == NULL)
				return S_FALSE;
			return a_pManager->CanActivate(a_pManager, pSubDoc, cOperID, pOperCfg, a_pStates);
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void GetAllDocs(ISubDocumentsMgr* a_pSDM, IComparable* a_pRoot, IEnumUnknownsInit* a_pItems)
{
	CComPtr<IEnumUnknowns> pItems;
	a_pSDM->ItemsEnum(a_pRoot, &pItems);
	if (pItems == NULL)
		return;
	ULONG n = 0;
	pItems->Size(&n);
	for (ULONG i = 0; i < n; ++i)
	{
		CComPtr<ISubDocumentID> pItem;
		pItems->Get(i, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pItem));
		if (pItem)
		{
			a_pItems->Insert(pItem);
			GetAllDocs(a_pSDM, pItem, a_pItems);
		}
	}
}

class ATL_NO_VTABLE CAllSubDocsOperationContext :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationContext
{
public:
	CAllSubDocsOperationContext() : m_nIndex(0), m_nRemaining(0)
	{
	}
	void Init(IOperationContext* a_pParent)
	{
		m_pParent = a_pParent;
	}
	void Step(ULONG a_nIndex, ULONG a_nRemaining)
	{
		m_nIndex = a_nIndex;
		m_nRemaining = a_nRemaining;
	}


BEGIN_COM_MAP(CAllSubDocsOperationContext)
	COM_INTERFACE_ENTRY(IOperationContext)
END_COM_MAP()

	// IOperationContext methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		return m_pParent ? m_pParent->StateGet(a_bstrCategoryName, a_iid, a_ppState) : E_RW_ITEMNOTFOUND;
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		return m_pParent ? m_pParent->StateSet(a_bstrCategoryName, a_pState) : E_NOTIMPL;
	}
	STDMETHOD(IsCancelled)()
	{
		return m_pParent ? m_pParent->IsCancelled() : S_FALSE;
	}
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		if (a_pItemIndex) *a_pItemIndex = m_nIndex;
		if (a_pItemsRemaining) *a_pItemsRemaining = m_nRemaining;
		if (a_pStepIndex) *a_pStepIndex = 0;
		if (a_pStepsRemaining) *a_pStepsRemaining = 0;
		if (m_pParent)
		{
			ULONG nI = 0;
			ULONG nR = 0;
			m_pParent->GetOperationInfo(&nI, &nR, a_pStepIndex, a_pStepsRemaining);
			if (a_pItemIndex) *a_pItemIndex += nI;
			if (a_pItemsRemaining) *a_pItemsRemaining += nR;
		}
		return S_OK;
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		if (m_pParent)
			m_pParent->SetErrorMessage(a_pMessage);
		return S_OK;
	}

private:
	ULONG m_nIndex;
	ULONG m_nRemaining;
	CComPtr<IOperationContext> m_pParent;
};


STDMETHODIMP CDocumentOperationExtractSubDocument::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComBSTR cCFGID_EXTSD_SUBOP(CFGID_EXTSD_SUBOP);
		CConfigValue cOperID;
		a_pConfig->ItemValueGet(cCFGID_EXTSD_SUBOP, &cOperID);
		CComPtr<IConfig> pOperCfg;
		a_pConfig->SubConfigGet(cCFGID_EXTSD_SUBOP, &pOperCfg);

		CComPtr<ISubDocumentsMgr> pSDM;
		a_pDocument->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM));
		if (pSDM == NULL)
			return a_pManager->Activate(a_pManager, a_pDocument, cOperID, pOperCfg, a_pStates, a_hParent, a_tLocaleID);//S_FALSE;

		CConfigValue cSubDocSrc;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBDOCSOURCE), &cSubDocSrc);
		if (cSubDocSrc.operator LONG() == CFGVAL_SUBDOCSRC_CATID)
		{
			CConfigValue cStateID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBDOCCATID), &cStateID);

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
			GetSelectedDocs(pSDM, cStateID, a_pStates, pItems, false);

			HRESULT hRes = S_FALSE;
			ULONG nSize = 0;
			pItems->Size(&nSize);
			CUndoBlock cUndoLock(a_pDocument);
			CComObject<CAllSubDocsOperationContext>* pBOC = NULL;
			CComObject<CAllSubDocsOperationContext>::CreateInstance(&pBOC);
			CComPtr<IOperationContext> pBOCTmp = pBOC;
			pBOC->Init(a_pStates);
			ULONG j = 0;
			for (ULONG i = 0; i < nSize; ++i)
			{
				CComPtr<IDocument> pDoc;
				pItems->Get(i, &pDoc);
				if (pDoc && S_OK == a_pManager->CanActivate(a_pManager, pDoc, cOperID, pOperCfg, a_pStates))
				{
					pBOC->Step(j++, nSize-i-1);
					HRESULT hRes2 = a_pManager->Activate(a_pManager, pDoc, cOperID, pOperCfg, pBOC, a_hParent, a_tLocaleID);
					if (hRes2 == E_RW_CANCELLEDBYUSER)
						return E_RW_CANCELLEDBYUSER;
					if (SUCCEEDED(hRes) && hRes2 != S_FALSE)
						hRes = hRes2;
				}
			}
			return hRes;
		}
		else if (cSubDocSrc.operator LONG() == CFGVAL_SUBDOCSRC_ALLSUBDOCS)
		{
			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
			GetAllDocs(pSDM, NULL, pItems);
			HRESULT hRes = S_FALSE;
			ULONG nSize = 0;
			pItems->Size(&nSize);
			CUndoBlock cUndoLock(a_pDocument);
			CComObject<CAllSubDocsOperationContext>* pBOC = NULL;
			CComObject<CAllSubDocsOperationContext>::CreateInstance(&pBOC);
			CComPtr<IOperationContext> pBOCTmp = pBOC;
			pBOC->Init(a_pStates);
			ULONG j = 0;
			for (ULONG i = 0; i < nSize; ++i)
			{
				CComPtr<ISubDocumentID> pID;
				pItems->Get(i, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pID));
				CComPtr<IDocument> pDoc;
				if (pID != NULL)
					pID->SubDocumentGet(&pDoc);
				if (pDoc && S_OK == a_pManager->CanActivate(a_pManager, pDoc, cOperID, pOperCfg, a_pStates))
				{
					pBOC->Step(j++, nSize-i-1);
					HRESULT hRes2 = a_pManager->Activate(a_pManager, pDoc, cOperID, pOperCfg, pBOC, a_hParent, a_tLocaleID);
					if (hRes2 == E_RW_CANCELLEDBYUSER)
						return E_RW_CANCELLEDBYUSER;
					if (SUCCEEDED(hRes) && hRes2 != S_FALSE)
						hRes = hRes2;
				}
			}
			return hRes;
		}
		else
		{
			CConfigValue cSubDocID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBDOCMANUAL), &cSubDocID);
			CComPtr<ISubDocumentID> pID;
			pSDM->FindByName(cSubDocID.operator BSTR(), &pID);
			CComPtr<IDocument> pSubDoc;
			if (pID != NULL)
				pID->SubDocumentGet(&pSubDoc);
			if (pSubDoc == NULL)
				return S_FALSE;
			return a_pManager->Activate(a_pManager, pSubDoc, cOperID, pOperCfg, a_pStates, a_hParent, a_tLocaleID);
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

