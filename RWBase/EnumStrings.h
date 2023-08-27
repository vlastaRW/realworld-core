// EnumString.h : Declaration of the CEnumString

#pragma once
#include "resource.h"       // main symbols



// CEnumStrings

class ATL_NO_VTABLE CEnumStrings : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEnumStrings, &CLSID_EnumStrings>,
	public IEnumStringsInit
{
public:
	CEnumStrings()
	{
	}
	~CEnumStrings()
	{
		vector<BSTR>::const_iterator i;
		for (i = m_aItems.begin(); i != m_aItems.end(); i++)
		{
			SysFreeString(*i);
		}
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CEnumStrings)
	COM_INTERFACE_ENTRY(IEnumStrings)
	COM_INTERFACE_ENTRY(IEnumStringsInit)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IEnumStrings
public:
	STDMETHOD(Size)(ULONG *a_pnSize);
	STDMETHOD(Get)(ULONG a_nIndex, BSTR* a_pbstrItem);
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, BSTR* a_abstrItems);

	// IEnumStringsInit
public:
	STDMETHOD(Insert)(BSTR a_bstrItem);
	STDMETHOD(InsertMultiple)(ULONG a_nCount, BSTR const* a_abstrItems);
	STDMETHOD(InsertFromEnum)(IEnumStrings* a_pSource);

private:
	vector<BSTR> m_aItems;
};

OBJECT_ENTRY_AUTO(__uuidof(EnumStrings), CEnumStrings)
