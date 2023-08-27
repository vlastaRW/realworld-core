// DocumentOperationSetFileFormat.h : Declaration of the CDocumentOperationSetFileFormat

#pragma once
#include "resource.h"       // main symbols

#include <RWProcessing.h>


// CE490E68-3571-4736-84A1-7AF7E9270938
extern __declspec(selectany) CLSID const CLSID_DocumentOperationSetFileFormat = { 0xce490e68, 0x3571, 0x4736, { 0x84, 0xa1, 0x7a, 0xf7, 0xe9, 0x27, 0x09, 0x38 } };

// CDocumentOperationSetFileFormat

class ATL_NO_VTABLE CDocumentOperationSetFileFormat :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationSetFileFormat, &CLSID_DocumentOperationSetFileFormat>,
	public IDocumentOperation,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationSetFileFormat()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationSetFileFormat)

BEGIN_CATEGORY_MAP(CDocumentOperationSetFileFormat)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationSetFileFormat)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);
};

OBJECT_ENTRY_AUTO(CLSID_DocumentOperationSetFileFormat, CDocumentOperationSetFileFormat)
