// DocumentOperationNULL.cpp : Implementation of CDocumentOperationNULL

#include "stdafx.h"
#include "DocumentOperationNULL.h"
#include <SharedStringTable.h>


// CDocumentOperationNULL

STDMETHODIMP CDocumentOperationNULL::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_OPERATIONNULL_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationNULL::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
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

STDMETHODIMP CDocumentOperationNULL::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* UNREF(a_pDocument), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	return S_OK;
}

STDMETHODIMP CDocumentOperationNULL::Activate(IOperationManager* UNREF(a_pManager), IDocument* UNREF(a_pDocument), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	return S_OK;
}

