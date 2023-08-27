// TransformationManager.cpp : Implementation of CTransformationManager

#include "stdafx.h"
#include "TransformationManager.h"


// CTransformationManager

STDMETHODIMP CTransformationManager::Activate(ITransformationManager* a_pOverrideForItem, IDocument* a_pDocument, const TConfigValue* a_ptTransformationID, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComPtr<IDocumentTransformation> p;
		if (a_ptTransformationID && a_ptTransformationID->eTypeID == ECVTGUID)
			RWCoCreateInstance(p, a_ptTransformationID->guidVal);
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;
		return p->Activate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_pConfig, a_pStates, a_hParent, a_tLocaleID, a_bstrPrefix, a_pBase);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CTransformationManager::CanActivate(ITransformationManager* a_pOverrideForItem, IDocument* a_pDocument, const TConfigValue* a_ptTransformationID, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		CComPtr<IDocumentTransformation> p;
		if (a_ptTransformationID && a_ptTransformationID->eTypeID == ECVTGUID)
			RWCoCreateInstance(p, a_ptTransformationID->guidVal);
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;
		return p->CanActivate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_pConfig, a_pStates);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CTransformationManager::Visit(ITransformationManager* a_pOverrideForItem, TConfigValue const* a_ptTransformationID, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor)
{
	try
	{
		CComPtr<IDocumentTransformation> p;
		if (a_ptTransformationID && a_ptTransformationID->eTypeID == ECVTGUID)
			RWCoCreateInstance(p, a_ptTransformationID->guidVal);
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;
		CComQIPtr<ICustomTransformationVisitor> pCustomVisitor(p);
		if (pCustomVisitor)
			return pCustomVisitor->Visit(a_pOverrideForItem ? a_pOverrideForItem : this, a_pConfig, a_pVisitor);
		return a_pVisitor->Run(a_pOverrideForItem ? a_pOverrideForItem : this, a_pConfig, p);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

