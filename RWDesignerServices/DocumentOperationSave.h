// DocumentOperationSave.h : Declaration of the CDocumentOperationSave

#pragma once
#include "resource.h"       // main symbols
#include <RWProcessing.h>
#include <WeakSingleton.h>
#include <RWProcessingTags.h>


// 4F862F0C-43FC-404E-959B-5C8DA1721B7B
extern __declspec(selectany) CLSID const CLSID_DocumentOperationSave = { 0x4f862f0c, 0x43fc, 0x404e, { 0x95, 0x9b, 0x5c, 0x8d, 0xa1, 0x72, 0x1b, 0x7b } };

// CDocumentOperationSave

class ATL_NO_VTABLE CDocumentOperationSave : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationSave, &CLSID_DocumentOperationSave>,
	public IDocumentOperation
{
public:
	CDocumentOperationSave()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CDocumentOperationSave)

BEGIN_CATEGORY_MAP(CDocumentOperationSave)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagInteractive)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationSave)
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

private:
	CComPtr<IStorageManager> m_pStorageMgr;
};

OBJECT_ENTRY_AUTO(CLSID_DocumentOperationSave, CDocumentOperationSave)
