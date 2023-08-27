// DocumentOperationJScript.h : Declaration of the CDocumentOperationJScript

#pragma once
#include "RWScripting.h"
#include <RWProcessingTags.h>
#include <RWConceptDesignerExtension.h>
#include <DesignerFrameIconsImpl.h>

HICON GetIconScript(ULONG a_nSize);

extern __declspec(selectany) pfnGetDFIcon const s_aScriptIconFns[] = { GetIconScript };


// CDocumentOperationJScript

class ATL_NO_VTABLE CDocumentOperationJScript :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationJScript, &CLSID_DocumentOperationJScript>,
	public IDocumentOperation,
	public CDesignerFrameIconsImpl<1, &CLSID_DocumentOperationJScript, nullptr, s_aScriptIconFns>
{
public:
	CDocumentOperationJScript()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationJScript)

BEGIN_CATEGORY_MAP(CDocumentOperationJScript)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagMetaOp)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationJScript)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
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

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationJScript), CDocumentOperationJScript)
