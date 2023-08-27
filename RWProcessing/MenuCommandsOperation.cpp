// MenuCommandsOperation.cpp : Implementation of CMenuCommandsOperation

#include "stdafx.h"
#include "MenuCommandsOperation.h"
#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include <RWConceptDesignerExtension.h>

const OLECHAR CFGID_OPERATION_ID[] = L"Operation";
const OLECHAR CFGID_OPERATION_NAME[] = L"Name";
const OLECHAR CFGID_OPERATION_DESCRITPION[] = L"Description";
const OLECHAR CFGID_OPERATION_ICONID[] = L"IconID";
const OLECHAR CFGID_OPERATION_SHORTCUT[] = L"Shortcut";

#include "ConfigGUIOperation.h"


// CMenuCommandsOperation

STDMETHODIMP CMenuCommandsOperation::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENUCOMMANDSOPERATION_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsOperation::ConfigCreate(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_OPERATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_NAME_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_NAME_DESC), CConfigValue(L"Operation"), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_OPERATION_DESCRITPION), _SharedStringTable.GetStringAuto(IDS_CFGID_DESCRITPION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DESCRITPION_DESC), CConfigValue(L""), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_OPERATION_SHORTCUT), _SharedStringTable.GetStringAuto(IDS_CFGID_SHORTCUT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SHORTCUT_DESC), CConfigValue(0L), NULL, 0, NULL);

		// icon
		CComBSTR cCFGID_ICONID(CFGID_OPERATION_ICONID);
		CComPtr<IConfigItemCustomOptions> pCustIconIDs;
		RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
		if (pCustIconIDs != NULL)
			pCfg->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
		else
			pCfg->ItemInsSimple(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), NULL, 0, NULL);

		CComQIPtr<IOperationManager> pMgr(a_pManager);

		M_OperationManager()->InsertIntoConfigAs(pMgr ? pMgr : M_OperationManager(), pCfg, CComBSTR(CFGID_OPERATION_ID), _SharedStringTable.GetStringAuto(IDS_CFGID_OPERATION_ID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_OPERATION_ID_DESC), 0, NULL);

		CConfigCustomGUI<&CLSID_MenuCommandsOperation, CConfigGUIOperationDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsOperation::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CConfigValue cName;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION_NAME), &cName);
		CConfigValue cDesc;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION_DESCRITPION), &cDesc);
		CConfigValue cShortcut;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION_SHORTCUT), &cShortcut);
		CConfigValue cIconID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION_ICONID), &cIconID);
		CComBSTR bstrCFGID_OPERATION_ID(CFGID_OPERATION_ID);
		CConfigValue cSubItem;
		a_pConfig->ItemValueGet(bstrCFGID_OPERATION_ID, &cSubItem);
		CComPtr<IConfig> pSubCfg;
		a_pConfig->SubConfigGet(bstrCFGID_OPERATION_ID, &pSubCfg);

		CComObject<CDocumentMenuCommand>* p = NULL;
		CComObject<CDocumentMenuCommand>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;

		CComPtr<ILocalizedString> pName;
		pName.Attach(new CMultiLanguageString(cName.Detach().bstrVal));
		CComPtr<ILocalizedString> pDesc;
		pDesc.Attach(new CMultiLanguageString(cDesc.Detach().bstrVal));

		CComQIPtr<IOperationManager> pMgr(a_pManager);

		p->Init(pName, pDesc, cShortcut, cIconID, pMgr ? pMgr : M_OperationManager(), cSubItem, pSubCfg, a_pStates, a_pDocument, a_pView);

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
		pItems->Insert(p);

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOperation::CDocumentMenuCommand::Name(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		(*a_ppText = m_pName)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOperation::CDocumentMenuCommand::Description(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		(*a_ppText = m_pDesc)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOperation::CDocumentMenuCommand::IconID(GUID* a_pIconID)
{
	try
	{
		*a_pIconID = m_tIconID;
		return S_OK;
	}
	catch (...)
	{
		return a_pIconID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOperation::CDocumentMenuCommand::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		CComPtr<IDesignerFrameIcons> pIcons;
		RWCoCreateInstance(pIcons, __uuidof(DesignerFrameIconsManager));
		return pIcons->GetIcon(m_tIconID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOperation::CDocumentMenuCommand::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	if ((m_nShortcut&0xffff) == 0)
		return E_NOTIMPL;

	try
	{
		a_pAccel->wKeyCode = m_nShortcut;
		a_pAccel->fVirtFlags = m_nShortcut>>16;
		return S_OK;
	}
	catch (...)
	{
		return a_pAccel ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOperation::CDocumentMenuCommand::SubCommands(IEnumUnknowns** a_ppSubCommands)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsOperation::CDocumentMenuCommand::State(EMenuCommandState* a_peState)
{
	try
	{
		*a_peState = m_pOpsMgr->CanActivate(m_pOpsMgr, m_pDocument, m_cItemID, m_pItemCfg, m_pStates) == S_OK ? EMCSNormal : EMCSDisabled;
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOperation::CDocumentMenuCommand::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		if (m_pView)
			m_pView->DeactivateAll(FALSE);

		CUndoBlock cUndo(m_pDocument, m_pName);
		return m_pOpsMgr->Activate(m_pOpsMgr, m_pDocument, m_cItemID, m_pItemCfg, m_pStates, a_hParent, a_tLocaleID);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

