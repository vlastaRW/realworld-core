// MenuCommandsSeparator.cpp : Implementation of CMenuCommandsSeparator

#include "stdafx.h"
#include "MenuCommandsSeparator.h"
#include <SharedStringTable.h>


// CMenuCommandsSeparator

STDMETHODIMP CMenuCommandsSeparator::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENUCOMMANDSSEPARATOR_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsSeparator::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		pCfg->Finalize(NULL);
		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsSeparator::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), IDesignerView* UNREF(a_pView), IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;
		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
		pItems->Insert(static_cast<IDocumentMenuCommand*>(this));
		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsSeparator::Name(ILocalizedString** UNREF(a_ppText))
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsSeparator::Description(ILocalizedString** UNREF(a_ppText))
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsSeparator::IconID(GUID* UNREF(a_pIconID))
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsSeparator::Icon(ULONG UNREF(a_nSize), HICON* UNREF(a_phIcon))
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsSeparator::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsSeparator::SubCommands(IEnumUnknowns** UNREF(a_ppSubCommands))
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsSeparator::State(EMenuCommandState* a_peState)
{
	try
	{
		*a_peState = EMCSSeparator;
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsSeparator::Execute(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	return E_NOTIMPL;
}

