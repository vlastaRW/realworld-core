// MenuCommandsToolbar.cpp : Implementation of CMenuCommandsToolbar

#include "stdafx.h"
#include "MenuCommandsToolbar.h"
#include <MultiLanguageString.h>


// CMenuCommandsToolbar

STDMETHODIMP CMenuCommandsToolbar::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENUTOOLBAR_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_TOOLBAR_NAME[] = L"CommandName"; 
static OLECHAR const CFGID_TOOLBAR_ID[] = L"ToolbarID";

STDMETHODIMP CMenuCommandsToolbar::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		pCfg->ItemInsSimple(CComBSTR(CFGID_TOOLBAR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_NAME_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_NAME_MENUDESC), CConfigValue(L"?"), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_TOOLBAR_ID), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_ID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_ID_DESC), CConfigValue(L""), NULL, 0, NULL);
		pCfg->Finalize(NULL);
		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsToolbar::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* UNREF(a_pView), IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CConfigValue cName;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TOOLBAR_NAME), &cName);
		CConfigValue cID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TOOLBAR_ID), &cID);

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		CComObject<CDocumentMenuCommand>* p = NULL;
		CComObject<CDocumentMenuCommand>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		CComPtr<ILocalizedString> pName;
		pName.Attach(new CMultiLanguageString(cName.Detach().bstrVal));
		cName.Detach();
		p->Init(a_pStates, cID, pName);
		pItems->Insert(pTmp);

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsToolbar::CDocumentMenuCommand::Name(ILocalizedString** a_ppText)
{
	try
	{
		(*a_ppText = m_pName)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsToolbar::CDocumentMenuCommand::State(EMenuCommandState* a_peState)
{
	try
	{
		CComPtr<ISharedStateToolbar> pState;
		m_pStates->StateGet(m_bstrID, __uuidof(ISharedStateToolbar), reinterpret_cast<void**>(&pState));
		*a_peState = pState ? (pState->IsVisible() == S_OK ? EMCSChecked : EMCSNormal) : EMCSDisabled;
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsToolbar::CDocumentMenuCommand::Execute(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	try
	{
		CComPtr<ISharedStateToolbar> pState;
		m_pStates->StateGet(m_bstrID, __uuidof(ISharedStateToolbar), reinterpret_cast<void**>(&pState));
		if (pState == NULL)
			return E_FAIL;
		pState->SetVisible(pState->IsVisible() == S_FALSE);
		return m_pStates->StateSet(m_bstrID, CComQIPtr<ISharedState>(pState));
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

