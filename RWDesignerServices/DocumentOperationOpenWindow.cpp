// DocumentOperationOpenWindow.cpp : Implementation of CDocumentOperationOpenWindow

#include "stdafx.h"
#include "DocumentOperationOpenWindow.h"

#include <MultiLanguageString.h>
#include <RWDesignerCore.h>


// CDocumentOperationOpenWindow

STDMETHODIMP CDocumentOperationOpenWindow::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Document - Open in new window[0405]Dokument - otevřít v novém okně");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationOpenWindow::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		return E_NOTIMPL;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationOpenWindow::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		return a_pDocument ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationOpenWindow::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	try
	{
		CComPtr<IDesignerCore> pDesignerCore;
		RWCoCreateInstance(pDesignerCore, __uuidof(DesignerCore));
		return pDesignerCore->NewWindowDocument(a_pDocument, NULL);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

