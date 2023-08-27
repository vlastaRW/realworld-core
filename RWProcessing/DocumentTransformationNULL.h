// DocumentTransformationNULL.h : Declaration of the CDocumentTransformationNULL

#pragma once
#include "resource.h"       // main symbols

#include "RWProcessing.h"


// CDocumentTransformationNULL

class ATL_NO_VTABLE CDocumentTransformationNULL : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentTransformationNULL, &CLSID_DocumentTransformationNULL>,
	public IDocumentTransformation
{
public:
	CDocumentTransformationNULL()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentTransformationNULL)

BEGIN_CATEGORY_MAP(CDocumentTransformationNULL)
	IMPLEMENTED_CATEGORY(CATID_DocumentTransformation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentTransformationNULL)
	COM_INTERFACE_ENTRY(IDocumentTransformation)
END_COM_MAP()


	// IDocumentTransformation methods
public:
	STDMETHOD(NameGet)(ITransformationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(ITransformationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentTransformationNULL), CDocumentTransformationNULL)
