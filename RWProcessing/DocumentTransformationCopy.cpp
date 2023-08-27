
#pragma once
#include "stdafx.h"
#include "RWProcessing.h"
#include <MultiLanguageString.h>

// {32D04317-8A6C-4abb-ADE9-D811D312136C}
extern GUID const CLSID_DocumentTransformationCopy = {0x32d04317, 0x8a6c, 0x4abb, {0xad, 0xe9, 0xd8, 0x11, 0xd3, 0x12, 0x13, 0x6c}};


// CDocumentTransformationCopy

class ATL_NO_VTABLE CDocumentTransformationCopy : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentTransformationCopy, &CLSID_DocumentTransformationCopy>,
	public IDocumentTransformation
{
public:
	CDocumentTransformationCopy()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentTransformationCopy)

BEGIN_CATEGORY_MAP(CDocumentOperationPipe)
	IMPLEMENTED_CATEGORY(CATID_DocumentTransformation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentTransformationCopy)
	COM_INTERFACE_ENTRY(IDocumentTransformation)
END_COM_MAP()

	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(ITransformationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		try
		{
			if (a_ppOperationName == NULL)
				return E_POINTER;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Copy document[0405]Kopírovat dokument");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(ITransformationManager* UNREF(a_pManager), IConfig** UNREF(a_ppDefaultConfig))
	{
		return S_FALSE;
	}
	STDMETHOD(CanActivate)(ITransformationManager* UNREF(a_pManager), IDocument* UNREF(a_pDocument), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
	{
		return S_OK;
	}
	STDMETHOD(Activate)(ITransformationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID), BSTR a_bstrPrefix, IDocumentBase* a_pBase)
	{
		return a_pDocument->DocumentCopy(a_bstrPrefix, a_pBase, NULL, NULL);
	}

};

OBJECT_ENTRY_AUTO(CLSID_DocumentTransformationCopy, CDocumentTransformationCopy)
