// DocumentOperationExtractSubDocument.h : Declaration of the CDocumentOperationExtractSubDocument

#pragma once
#include "resource.h"       // main symbols
#include "RWViewStructure.h"


// CDocumentOperationExtractSubDocument

class ATL_NO_VTABLE CDocumentOperationExtractSubDocument : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationExtractSubDocument, &CLSID_DocumentOperationExtractSubDocument>,
	public IDocumentOperation
{
public:
	CDocumentOperationExtractSubDocument()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationExtractSubDocument)

BEGIN_CATEGORY_MAP(CDocumentOperationExtractSubDocument)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationExtractSubDocument)
	COM_INTERFACE_ENTRY(IDocumentOperation)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationExtractSubDocument), CDocumentOperationExtractSubDocument)
