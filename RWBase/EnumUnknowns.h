// EnumUnknown.h : Declaration of the CEnumUnknowns

#pragma once
#include "resource.h"       // main symbols



// CEnumUnknowns

class ATL_NO_VTABLE CEnumUnknowns : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEnumUnknowns, &CLSID_EnumUnknowns>,
	public IEnumUnknownsInit
{
public:
	CEnumUnknowns()
	{
	}
	~CEnumUnknowns()
	{
		vector<IUnknown*>::const_iterator i;
		for (i = m_aItems.begin(); i != m_aItems.end(); i++)
		{
			(*i)->Release();
		}
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CEnumUnknowns)
	COM_INTERFACE_ENTRY(IEnumUnknowns)
	COM_INTERFACE_ENTRY(IEnumUnknownsInit)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IEnumUnknowns methods
public:
	STDMETHOD(Size)(ULONG *a_pnSize);
	STDMETHOD(Get)(ULONG a_nIndex, REFIID a_iid, void** a_ppItem);
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, REFIID a_iid, void** a_apItems);

	// IEnumUnknownsInit methods
public:
	STDMETHOD(Insert)(IUnknown* a_pItem);
	STDMETHOD(InsertMultiple)(ULONG a_nCount, IUnknown* const* a_apItems);
	STDMETHOD(InsertFromEnum)(IEnumUnknowns* a_pSource);

private:
	vector<IUnknown*> m_aItems;
};

OBJECT_ENTRY_AUTO(__uuidof(EnumUnknowns), CEnumUnknowns)
