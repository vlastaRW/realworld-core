// DocumentOperationShowConfig.h : Declaration of the CDocumentOperationShowConfig

#pragma once
#include "resource.h"       // main symbols
#include "RWProcessing.h"
#include <RWProcessingTags.h>


// CDocumentOperationShowConfig

class ATL_NO_VTABLE CDocumentOperationShowConfig :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationShowConfig, &CLSID_DocumentOperationShowConfig>,
	public IDocumentOperation
{
public:
	CDocumentOperationShowConfig()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationShowConfig)

BEGIN_CATEGORY_MAP(CDocumentOperationShowConfig)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagInteractive)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationShowConfig)
	COM_INTERFACE_ENTRY(IDocumentOperation)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationShowConfig), CDocumentOperationShowConfig)
