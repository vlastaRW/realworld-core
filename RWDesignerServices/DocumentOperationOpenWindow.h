// DocumentOperationOpenWindow.h : Declaration of the CDocumentOperationOpenWindow

#pragma once
#include "resource.h"       // main symbols
#include <RWProcessing.h>
#include <WeakSingleton.h>
#include <RWProcessingTags.h>

 // 3FD694FF-C545-477B-BCE6-D200D013BABB
extern __declspec(selectany) CLSID const CLSID_DocumentOperationOpenWindow = { 0x3fd694ff, 0xc545, 0x477b, { 0xbc, 0xe6, 0xd2, 0x00, 0xd0, 0x13, 0xba, 0xbb } };

// CDocumentOperationOpenWindow

class ATL_NO_VTABLE CDocumentOperationOpenWindow : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationOpenWindow, &CLSID_DocumentOperationOpenWindow>,
	public IDocumentOperation
{
public:
	CDocumentOperationOpenWindow()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CDocumentOperationOpenWindow)

BEGIN_CATEGORY_MAP(CDocumentOperationOpenWindow)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagInteractive)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationOpenWindow)
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

OBJECT_ENTRY_AUTO(CLSID_DocumentOperationOpenWindow, CDocumentOperationOpenWindow)
