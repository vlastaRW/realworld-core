// OperationManager.cpp : Implementation of COperationManager

#include "stdafx.h"
#include "OperationManager.h"


// COperationManager

STDMETHODIMP COperationManager::Activate(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, const TConfigValue* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentOperation> p;
		if (a_ptOperationID && a_ptOperationID->eTypeID == ECVTGUID)
			RWCoCreateInstance(p, a_ptOperationID->guidVal);
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;
		return p->Activate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_pConfig, a_pStates, a_hParent, a_tLocaleID);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COperationManager::CanActivate(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, const TConfigValue* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		CComPtr<IDocumentOperation> p;
		if (a_ptOperationID && a_ptOperationID->eTypeID == ECVTGUID)
			RWCoCreateInstance(p, a_ptOperationID->guidVal);
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;
		return p->CanActivate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_pConfig, a_pStates);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COperationManager::Visit(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor)
{
	try
	{
		CComPtr<IDocumentOperation> p;
		if (a_ptOperationID && a_ptOperationID->eTypeID == ECVTGUID)
			RWCoCreateInstance(p, a_ptOperationID->guidVal);
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;
		CComQIPtr<ICustomOperationVisitor> pCustomVisitor(p);
		if (pCustomVisitor)
			return pCustomVisitor->Visit(a_pOverrideForItem ? a_pOverrideForItem : this, a_pConfig, a_pVisitor);
		return a_pVisitor->Run(a_pOverrideForItem ? a_pOverrideForItem : this, a_pConfig, p);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

