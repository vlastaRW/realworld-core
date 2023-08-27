// MenuCommandsSubDocument.cpp : Implementation of CMenuCommandsSubDocument

#include "stdafx.h"
#include "MenuCommandsSubDocument.h"
#include "ConfigIDsSubDocumentFrame.h"
#include <SharedStringTable.h>

const OLECHAR CFGID_SUBDOCSOURCE[] = L"SubDocSource";
const OLECHAR CFGID_SUBDOCMANUAL[] = L"SubDocManual";
const OLECHAR CFGID_SUBDOCCATID[] = L"SubDocCategoryID";
const OLECHAR CFGID_EXTSD_SUBCMDS[] = L"SubCommands";
const LONG CFGVAL_SUBDOCSRC_MANUAL = 0;
const LONG CFGVAL_SUBDOCSRC_CATID = 1;

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIMenuSubDocumentDlg :
	public CCustomConfigWndImpl<CConfigGUIMenuSubDocumentDlg>,
	public CDialogResize<CConfigGUIMenuSubDocumentDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_SUBDOCCMDS };

	BEGIN_MSG_MAP(CConfigGUIMenuSubDocumentDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIMenuSubDocumentDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIMenuSubDocumentDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIMenuSubDocumentDlg)
		DLGRESIZE_CONTROL(IDC_CGEXTSUBDOC_SYNCGROUP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGEXTSUBDOC_MANUALID, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGEXTSUBDOC_SUBOP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGEXTSUBDOC_SUBCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIMenuSubDocumentDlg)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGEXTSUBDOC_MANUALID, CFGID_SUBDOCMANUAL)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGEXTSUBDOC_SYNCGROUP, CFGID_SUBDOCCATID)
		CONFIGITEM_COMBOBOX(IDC_CGEXTSUBDOC_EXTBY, CFGID_SUBDOCSOURCE)
		CONFIGITEM_COMBOBOX(IDC_CGEXTSUBDOC_SUBOP, CFGID_EXTSD_SUBCMDS)
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
		M_Config()->SubConfigGet(CComBSTR(CFGID_EXTSD_SUBCMDS), &pDeviceCfg);
		m_pConfigWnd->ConfigSet(pDeviceCfg, M_Mode());

		DlgResize_Init(false, false, 0);

		return 1;
	}

private:
	CComPtr<IConfigWnd> m_pConfigWnd;
};


// CMenuCommandsSubDocument

STDMETHODIMP CMenuCommandsSubDocument::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_SUBDOCUMENT);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsSubDocument::ConfigCreate(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
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
		pCfg->ItemOptionAdd(cCFGID_SUBDOCSOURCE, CConfigValue(CFGVAL_SUBDOCSRC_MANUAL), _SharedStringTable.GetStringAuto(IDS_CFGVAL_SUBDOCSRC_MANUAL), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_SUBDOCSOURCE, CConfigValue(CFGVAL_SUBDOCSRC_CATID), _SharedStringTable.GetStringAuto(IDS_CFGVAL_SUBDOCSRC_CATID), 0, NULL);

		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_SUBDOCSOURCE;
		tCond.eConditionType = ECOCEqual;
		tCond.tValue = cCFGVAL_SUBDOCSRC_MANUAL;
		CComBSTR cCFGID_SUBDOCMANUAL(CFGID_SUBDOCMANUAL);
		pCfg->ItemInsSimple(cCFGID_SUBDOCMANUAL, _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCMANUAL_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCMANUAL_DESC), CConfigValue(L""), NULL, 1, &tCond);

		tCond.tValue = cCFGVAL_SUBDOCSRC_CATID;
		CComBSTR cCFGID_SUBDOCCATID(CFGID_SUBDOCCATID);
		pCfg->ItemInsSimple(cCFGID_SUBDOCCATID, _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCCATID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCCATID_DESC), CConfigValue(L"SUBDOC"), NULL, 1, &tCond);

		// sub-operation
		a_pManager->InsertIntoConfigAs(a_pManager, pCfg, CComBSTR(CFGID_EXTSD_SUBCMDS), _SharedStringTable.GetStringAuto(IDS_CFGID_EXTSD_SUBOP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_EXTSD_SUBOP_DESC), 0, NULL);

		CConfigCustomGUI<&CLSID_MenuCommandsSubDocument, CConfigGUIMenuSubDocumentDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsSubDocument::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IDocument> pSubDoc;

		CComPtr<ISubDocumentsMgr> pSDM;
		a_pDocument->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM));
		if (pSDM)
			GetSubDoc(pSDM, a_pConfig, a_pStates, &pSubDoc);

		if (pSubDoc == NULL)
			return E_FAIL;

		CComBSTR cCFGID_EXTSD_SUBCMDS(CFGID_EXTSD_SUBCMDS);
		CConfigValue cCmdsID;
		a_pConfig->ItemValueGet(cCFGID_EXTSD_SUBCMDS, &cCmdsID);
		CComPtr<IConfig> pCmdsCfg;
		a_pConfig->SubConfigGet(cCFGID_EXTSD_SUBCMDS, &pCmdsCfg);

		return a_pManager->CommandsEnum(a_pManager, cCmdsID, pCmdsCfg, a_pStates, a_pView, pSubDoc, a_ppSubCommands);
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

void CMenuCommandsSubDocument::GetSubDoc(ISubDocumentsMgr* a_pSDM, IConfig* a_pConfig, IOperationContext* a_pStates, IDocument** a_ppDoc)
{
	CConfigValue cSubDocSrc;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBDOCSOURCE), &cSubDocSrc);

	if (cSubDocSrc.operator LONG() == CFGVAL_SUBDOCSRC_MANUAL)
	{
		CConfigValue cSubDocID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBDOCMANUAL), &cSubDocID);
		CComPtr<ISubDocumentID> pID;
		a_pSDM->FindByName(cSubDocID.operator BSTR(), &pID);
		if (pID != NULL)
		{
			pID->SubDocumentGet(a_ppDoc);
		}
	}
	else
	{
		CConfigValue cStateID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBDOCCATID), &cStateID);
		CComBSTR bstrID;
		a_pSDM->StatePrefix(&bstrID);
		if (bstrID.Length())
			bstrID += cStateID.operator BSTR();
		else
			bstrID.Attach(cStateID.Detach().bstrVal);
		CComPtr<ISharedState> pState;
		a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IEnumUnknowns> pItems;
		if (pState)
			a_pSDM->StateUnpack(pState, &pItems);
		CComPtr<IComparable> pItem;
		if (pItems)
			pItems->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
		CComPtr<ISubDocumentID> pID;
		if (pItem)
			a_pSDM->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pID));
		if (pID != NULL)
		{
			pID->SubDocumentGet(a_ppDoc);
			return;
		}
		pItems = NULL;
		a_pSDM->ItemsEnum(NULL, &pItems);
		if (pItems == NULL)
			return;
		pItem = NULL;
		pItems->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
		if (pItem == NULL)
			return;
		pID = NULL;
		a_pSDM->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pID));
		if (pID != NULL)
			pID->SubDocumentGet(a_ppDoc);
	}
}

