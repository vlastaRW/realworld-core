// DocumentTransformationJScript.h : Declaration of the CDocumentTransformationJScript

#pragma once
#include "RWScripting.h"


// CDocumentTransformationJScript

class ATL_NO_VTABLE CDocumentTransformationJScript :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentTransformationJScript, &CLSID_DocumentTransformationJScript>,
	public IDocumentTransformation
{
public:
	CDocumentTransformationJScript()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentTransformationJScript)

BEGIN_CATEGORY_MAP(CDocumentTransformationJScript)
	IMPLEMENTED_CATEGORY(CATID_DocumentTransformation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentTransformationJScript)
	COM_INTERFACE_ENTRY(IDocumentTransformation)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDocumentTransformation methods
public:
	STDMETHOD(NameGet)(ITransformationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(ITransformationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentTransformationJScript), CDocumentTransformationJScript)
