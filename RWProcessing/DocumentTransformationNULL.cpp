// DocumentTransformationNULL.cpp : Implementation of CDocumentTransformationNULL

#include "stdafx.h"
#include "DocumentTransformationNULL.h"
#include <SharedStringTable.h>

// CDocumentTransformationNULL

STDMETHODIMP CDocumentTransformationNULL::NameGet(ITransformationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_TRANFORMATIONNULL_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTransformationNULL::ConfigCreate(ITransformationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
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

STDMETHODIMP CDocumentTransformationNULL::CanActivate(ITransformationManager* UNREF(a_pManager), IDocument* UNREF(a_pDocument), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	return S_OK;
}

STDMETHODIMP CDocumentTransformationNULL::Activate(ITransformationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID), BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		return a_pDocument->DocumentCopy(a_bstrPrefix, a_pBase, NULL, NULL);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

