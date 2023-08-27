// EnumUnknown.h : Declaration of the CEnumStructs

#pragma once
#include "resource.h"       // main symbols
#include "RWBase.h"


// CEnumStructs

class ATL_NO_VTABLE CEnumStructs_ : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEnumStructs_, &CLSID_EnumStructs>,
	public IEnumStructsInit
{
public:
	CEnumStructs_()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CEnumStructs_)
	COM_INTERFACE_ENTRY(IEnumStructs)
	COM_INTERFACE_ENTRY(IEnumStructsInit)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IEnumStructs methods
public:
	STDMETHOD(Size)(ULONG a_nStructSize, ULONG *a_pnSize);
	STDMETHOD(Get)(ULONG a_nIndex, ULONG a_nStructSize, BYTE* a_pItem);
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, ULONG a_nStructSize, BYTE* a_aItems);

	// IEnumStructsInit methods
public:
	STDMETHOD(Insert)(ULONG a_nStructSize, BYTE const* a_pItem);
	STDMETHOD(InsertMultiple)(ULONG a_nCount, ULONG a_nStructSize, BYTE const* a_aItems);
	STDMETHOD(InsertFromEnum)(ULONG a_nStructSize, IEnumStructs* a_pSource);

private:
	vector<BYTE> m_aItems;
};

OBJECT_ENTRY_AUTO(__uuidof(EnumStructs), CEnumStructs_)
