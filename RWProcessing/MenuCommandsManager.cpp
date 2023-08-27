// MenuCommandsManager.cpp : Implementation of CMenuCommandsManager

#include "stdafx.h"
#include "MenuCommandsManager.h"


// CMenuCommandsManager

STDMETHODIMP CMenuCommandsManager::CommandsEnum(IMenuCommandsManager* a_pOverrideForItem, const TConfigValue* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		CComPtr<IDocumentMenuCommands> p;
		if (a_ptOperationID && a_ptOperationID->eTypeID == ECVTGUID)
			RWCoCreateInstance(p, a_ptOperationID->guidVal);
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;
		return p->CommandsEnum(a_pOverrideForItem ? a_pOverrideForItem : this, a_pConfig, a_pStates, a_pView, a_pDocument, a_ppSubCommands);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
