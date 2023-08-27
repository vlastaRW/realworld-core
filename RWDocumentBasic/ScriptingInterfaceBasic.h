// ScriptingInterfaceBasic.h : Declaration of the CScriptingInterfaceBasic

#pragma once
#include "resource.h"       // main symbols

#include "RWDocumentBasic.h"


// CScriptingInterfaceBasic

class ATL_NO_VTABLE CScriptingInterfaceBasic :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CScriptingInterfaceBasic, &CLSID_ScriptingInterfaceBasic>,
	public IScriptingInterface
{
public:
	CScriptingInterfaceBasic()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CScriptingInterfaceBasic)

BEGIN_CATEGORY_MAP(CScriptingInterfaceBasic)
	IMPLEMENTED_CATEGORY(CATID_ScriptingInterface)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CScriptingInterfaceBasic)
	COM_INTERFACE_ENTRY(IScriptingInterface)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IScriptingInterface methods
public:
	STDMETHOD(GetGlobalObjects)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
	STDMETHOD(GetInterfaceAdaptors)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument);
	STDMETHOD(GetKeywords)(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary);

};

OBJECT_ENTRY_AUTO(__uuidof(ScriptingInterfaceBasic), CScriptingInterfaceBasic)
