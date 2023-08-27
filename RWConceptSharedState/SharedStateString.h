// SharedStateString.h : Declaration of the CSharedStateString

#pragma once
#include "resource.h"       // main symbols
#include "RWConceptSharedState.h"



// CSharedStateString

class ATL_NO_VTABLE CSharedStateString :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateString, &CLSID_SharedStateString>,
	public ISharedState
{
public:
	CSharedStateString()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSharedStateString)
	COM_INTERFACE_ENTRY(ISharedState)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID)
	{
		try
		{
			*a_pCLSID = CLSID_SharedStateString;
			return S_OK;
		}
		catch (...)
		{
			return a_pCLSID ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrText)
	{
		return m_bstr.CopyTo(a_pbstrText);
	}
	STDMETHOD(FromText)(BSTR a_bstrText)
	{
		try
		{
			m_bstr = a_bstrText;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComBSTR m_bstr;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateString), CSharedStateString)
