// SharedStateEnum.h : Declaration of the CSharedStateEnum

#pragma once
#include "resource.h"       // main symbols
#include "RWConceptSharedState.h"


// CSharedStateEnum

class ATL_NO_VTABLE CSharedStateEnum : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateEnum, &CLSID_SharedStateEnum>,
	public ISharedState
{
public:
	CSharedStateEnum()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSharedStateEnum)
	COM_INTERFACE_ENTRY(ISharedState)
	COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IEnumUnknowns), m_pEnum.p)
	COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IEnumUnknownsInit), m_pEnum.p)
END_COM_MAP()


	DECLARE_GET_CONTROLLING_UNKNOWN()
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return RWCoCreateInstance(__uuidof(EnumUnknowns), GetControllingUnknown(), CLSCTX_ALL, __uuidof(IUnknown), reinterpret_cast<void**>(&m_pEnum));
	}
	
	void FinalRelease() 
	{
	}

	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID);
	STDMETHOD(ToText)(BSTR* a_pbstrText);
	STDMETHOD(FromText)(BSTR a_bstrText);

private:
	CComPtr<IUnknown> m_pEnum;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateEnum), CSharedStateEnum)
