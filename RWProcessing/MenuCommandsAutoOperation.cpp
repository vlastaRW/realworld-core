// MenuCommandsAutoOperation.cpp : Implementation of CMenuCommandsAutoOperation

#include "stdafx.h"
#include "MenuCommandsAutoOperation.h"
#include "RWConceptDesignerExtension.h"
#include <SharedStringTable.h>


const OLECHAR CFGID_OPERATION_CLASS[] = L"Class";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIAutoOperationDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIAutoOperationDlg>,
	public CDialogResize<CConfigGUIAutoOperationDlg>
{
public:
	enum
	{
		IDC_CFGID_OPERATION_CLASS = 100,
		IDC_DELETE_SUBCONFIGS,
	};

	BEGIN_DIALOG_EX(0, 0, 120, 30, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Operation class:[0405]Třída operací:"), IDC_STATIC, 0, 2, 65, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CFGID_OPERATION_CLASS, 67, 0, 53, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_PUSHBUTTON(_T("Reset plug-in configuration"), IDC_DELETE_SUBCONFIGS, 0, 16, 150, 14, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIAutoOperationDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIAutoOperationDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIAutoOperationDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_DELETE_SUBCONFIGS, BN_CLICKED, OnDeleteSubConfigs)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIAutoOperationDlg)
		DLGRESIZE_CONTROL(IDC_CFGID_OPERATION_CLASS, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIAutoOperationDlg)
		CONFIGITEM_EDITBOX(IDC_CFGID_OPERATION_CLASS, CFGID_OPERATION_CLASS)
	END_CONFIGITEM_MAP()


	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
	LRESULT OnDeleteSubConfigs(WORD, WORD, HWND, BOOL&)
	{
		CComPtr<IConfig> pSubCfg;
		M_Config()->SubConfigGet(CComBSTR(CFGID_OPERATION_CLASS), &pSubCfg);
		CComQIPtr<IConfigInMemory> pMemCfg(pSubCfg);
		if (pMemCfg)
		{
			pMemCfg->DeleteItems(NULL);
			GetDlgItem(IDC_DELETE_SUBCONFIGS).EnableWindow(FALSE);
		}
		return 0;
	}
	void ExtraConfigNotify()
	{
		CComPtr<IConfig> pSubCfg;
		M_Config()->SubConfigGet(CComBSTR(CFGID_OPERATION_CLASS), &pSubCfg);
		CComPtr<IEnumStrings> pStrs;
		if (pSubCfg) pSubCfg->ItemIDsEnum(&pStrs);
		ULONG nStrs = 0;
		if (pStrs) pStrs->Size(&nStrs);
		GetDlgItem(IDC_DELETE_SUBCONFIGS).EnableWindow(nStrs != 0);
	}
};


// CMenuCommandsAutoOperation

STDMETHODIMP CMenuCommandsAutoOperation::NameGet(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENUCOMMANDSAUTOOPERATION_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsAutoOperation::ConfigCreate(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		CComPtr<ISubConfig> pSC;
		RWCoCreateInstance(pSC, __uuidof(ConfigInMemory));
		pCfg->ItemInsSimple(CComBSTR(CFGID_OPERATION_CLASS), _SharedStringTable.GetStringAuto(IDS_CFGID_OPERATION_CLASS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_OPERATION_CLASS_DESC), CConfigValue(GUID_NULL), pSC, 0, NULL);

		CConfigCustomGUI<&CLSID_MenuCommandsAutoOperation, CConfigGUIAutoOperationDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsAutoOperation::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;
		CComObject<CAutoCommands>* p = NULL;
		CComObject<CAutoCommands>::CreateInstance(&p);
		CComPtr<IEnumUnknowns> pTmp = p;
		CComPtr<IOperationManager> pMgr;
		a_pManager->QueryInterface(&pMgr);
		if (pMgr == NULL) RWCoCreateInstance(pMgr, __uuidof(OperationManager));
		p->Init(a_pConfig, pMgr, a_pStates, a_pDocument, a_pView);
		*a_ppSubCommands = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

void CMenuCommandsAutoOperation::CAutoCommands::Init(IConfig* a_pConfig, IOperationManager* a_pOpsMgr, IOperationContext* a_pStates, IDocument* a_pDocument, IDesignerView* a_pView)
{
	m_pConfig = a_pConfig;
	CConfigValue cID;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION_CLASS), &cID);
	CComPtr<IPlugInCache> pMgr;
	RWCoCreateInstance(pMgr, __uuidof(PlugInCache));
	pMgr->CLSIDsEnum(cID, 0xffffffff, &m_pCLSIDs);
	m_pOpsMgr = a_pOpsMgr;
	m_pStates = a_pStates;
	m_pDocument = a_pDocument;
	m_pView = a_pView;
}

STDMETHODIMP CMenuCommandsAutoOperation::CAutoCommands::Size(ULONG* a_pnSize)
{
	try
	{
		*a_pnSize = 0;
		if (m_pCLSIDs)
			m_pCLSIDs->Size(a_pnSize);
		return S_OK;
	}
	catch (...)
	{
		return a_pnSize ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsAutoOperation::CAutoCommands::Get(ULONG a_nIndex, REFIID a_iid, void** a_ppItem)
{
	return GetMultiple(a_nIndex, 1, a_iid, a_ppItem);
}

STDMETHODIMP CMenuCommandsAutoOperation::CAutoCommands::GetMultiple(ULONG a_nIndexFirst, ULONG a_nCount, REFIID a_iid, void** a_apItems)
{
	try
	{
		ZeroMemory(a_apItems, a_nCount*sizeof*a_apItems);
		ULONG nSize = 0;
		if (m_pCLSIDs) m_pCLSIDs->Size(&nSize);
		if (a_nIndexFirst+a_nCount > nSize)
			return E_RW_INDEXOUTOFRANGE;

		for (ULONG i = 0; i < a_nCount; ++i)
		{
			CLSID tID = GUID_NULL;
			m_pCLSIDs->Get(i+a_nIndexFirst, &tID);
			CComObject<CCommand>* pC = NULL;
			CComObject<CCommand>::CreateInstance(&pC);
			CComPtr<IDocumentMenuCommand> pTmp = pC;
			CComPtr<IConfig> pItemCfg;
			OLECHAR szItem[64];
			swprintf(szItem, L"%s\\%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
				CFGID_OPERATION_CLASS,
				tID.Data1, (DWORD)tID.Data2, (DWORD)tID.Data3,
				(DWORD)tID.Data4[0], (DWORD)tID.Data4[1],
				(DWORD)tID.Data4[2], (DWORD)tID.Data4[3],
				(DWORD)tID.Data4[4], (DWORD)tID.Data4[5],
				(DWORD)tID.Data4[6], (DWORD)tID.Data4[7]);
			m_pConfig->SubConfigGet(CComBSTR(szItem), &pItemCfg);
			pC->Init(tID, pItemCfg, m_pOpsMgr, m_pStates, m_pDocument, m_pView);
			a_apItems[i] = pTmp.Detach();
		}

		return S_OK;
	}
	catch (...)
	{
		return a_apItems ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsAutoOperation::CCommand::Name(ILocalizedString** a_ppText)
{
	return m_pAO ? m_pAO->Name(a_ppText) : E_FAIL;
}

STDMETHODIMP CMenuCommandsAutoOperation::CCommand::Description(ILocalizedString** a_ppText)
{
	return m_pAO ? m_pAO->Description(a_ppText) : E_FAIL;
}

STDMETHODIMP CMenuCommandsAutoOperation::CCommand::IconID(GUID* a_pIconID)
{
	return m_pAO ? m_pAO->IconID(a_pIconID) : E_FAIL;
}

STDMETHODIMP CMenuCommandsAutoOperation::CCommand::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		GUID tID = GUID_NULL;
		if (m_pAO == NULL)
			return E_FAIL;
		m_pAO->IconID(&tID);
		CComPtr<IDesignerFrameIcons> pIcMgr;
		RWCoCreateInstance(pIcMgr, __uuidof(DesignerFrameIconsManager));
		return pIcMgr->GetIcon(tID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsAutoOperation::CCommand::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	return m_pAO ? m_pAO->Accelerator(a_pAccel, a_pAuxAccel) : E_FAIL;
}

STDMETHODIMP CMenuCommandsAutoOperation::CCommand::State(EMenuCommandState* a_peState)
{
	*a_peState = EMCSNormal;// TODO: implement correctly?
	return S_OK;
}

STDMETHODIMP CMenuCommandsAutoOperation::CCommand::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		if (m_pView)
			m_pView->DeactivateAll(FALSE);

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		CComBSTR bstrOp(L"Operation");
		m_pOpsMgr->InsertIntoConfigAs(m_pOpsMgr, pCfg, bstrOp, NULL, NULL, 0, NULL);
		pCfg->Finalize(NULL);
		m_pAO->Configuration(pCfg);
		CConfigValue cOpID;
		pCfg->ItemValueGet(bstrOp, &cOpID);
		CComPtr<IConfig> pOpCfg;
		pCfg->SubConfigGet(bstrOp, &pOpCfg);
		if (!(GetAsyncKeyState(VK_SHIFT)&GetAsyncKeyState(VK_CONTROL)&0x8000))
			CopyConfigValues(pOpCfg, m_pItemCfg);
		HRESULT hRes = m_pOpsMgr->Activate(m_pOpsMgr, m_pDocument, cOpID, pOpCfg, m_pStates, a_hParent, a_tLocaleID);
		if (hRes != E_RW_CANCELLEDBYUSER)
			CopyConfigValues(m_pItemCfg, pOpCfg);
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

