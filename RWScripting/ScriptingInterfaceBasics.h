// ScriptingInterfaceBasics.h : Declaration of the CScriptingInterfaceBasics

#pragma once
#include "RWScripting.h"


// CScriptingInterfaceBasics

class ATL_NO_VTABLE CScriptingInterfaceBasics :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CScriptingInterfaceBasics, &CLSID_ScriptingInterfaceBasics>,
	public IScriptingInterface,
	public IScriptingInterfaceBasics
{
public:
	CScriptingInterfaceBasics()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CScriptingInterfaceBasics)

BEGIN_CATEGORY_MAP(CScriptingInterfaceBasics)
	IMPLEMENTED_CATEGORY(CATID_ScriptingInterface)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CScriptingInterfaceBasics)
	COM_INTERFACE_ENTRY(IScriptingInterface)
	COM_INTERFACE_ENTRY(IScriptingInterfaceBasics)
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

	// IScriptingInterfaceBasics methods
public:
	STDMETHOD(InitGlobals)(BSTR a_bstrAppName);

};

OBJECT_ENTRY_AUTO(__uuidof(ScriptingInterfaceBasics), CScriptingInterfaceBasics)
