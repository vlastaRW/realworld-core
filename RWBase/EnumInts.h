// EnumInts.h : Declaration of the CEnumInts

#pragma once
#include "resource.h"       // main symbols
#include "EnumXxxxImpl.h"



// CEnumInts

class ATL_NO_VTABLE CEnumInts : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEnumInts, &CLSID_EnumInts>,
	public CEnumXxxxImpl<CEnumInts, IEnumInts, IEnumIntsInit, LONG>
{
public:
	CEnumInts()
	{
	}

	DECLARE_NO_REGISTRY()


	BEGIN_COM_MAP(CEnumInts)
		COM_INTERFACE_ENTRY(IEnumInts)
		COM_INTERFACE_ENTRY(IEnumIntsInit)
	END_COM_MAP()
};

OBJECT_ENTRY_AUTO(__uuidof(EnumInts), CEnumInts)
