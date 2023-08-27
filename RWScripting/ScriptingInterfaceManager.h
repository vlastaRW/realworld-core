// ScriptingInterfaceManager.h : Declaration of the CScriptingInterfaceManager

#pragma once
#include "RWScripting.h"
#include <WeakSingleton.h>


// CScriptingInterfaceManager

class ATL_NO_VTABLE CScriptingInterfaceManager :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CScriptingInterfaceManager, &CLSID_ScriptingInterfaceManager>,
	public IScriptingInterfaceManager
{
public:
	CScriptingInterfaceManager() : m_bInitiated(false)
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CScriptingInterfaceManager)

BEGIN_COM_MAP(CScriptingInterfaceManager)
	COM_INTERFACE_ENTRY(IScriptingInterfaceManager)
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
	STDMETHOD(GetGlobalObjects)(IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
	STDMETHOD(GetInterfaceAdaptors)(IScriptingSite* a_pSite, IDocument* a_pDocument);
	STDMETHOD(GetKeywords)(IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary);
	STDMETHOD(WrapDocument)(IScriptingInterfaceManager* a_pOverride, IDocument* a_pDocument, IScriptedDocument** a_ppWrapped);
	STDMETHOD(CreateJScriptArray)(IJScriptArrayInit** a_ppArray);

private:
	void CheckState();

private:
	bool m_bInitiated;
	CComPtr<IEnumUnknowns> m_pPlugIns;
};

OBJECT_ENTRY_AUTO(__uuidof(ScriptingInterfaceManager), CScriptingInterfaceManager)
