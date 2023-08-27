// EnumUInts.h : Declaration of the CEnumUInts

#pragma once
#include "resource.h"       // main symbols
#include "EnumXxxxImpl.h"



// CEnumUInts

class ATL_NO_VTABLE CEnumUInts : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEnumUInts, &CLSID_EnumUInts>,
	public CEnumXxxxImpl<CEnumUInts, IEnumUInts, IEnumUIntsInit, ULONG>
{
public:
	CEnumUInts()
	{
	}

	DECLARE_NO_REGISTRY()


	BEGIN_COM_MAP(CEnumUInts)
		COM_INTERFACE_ENTRY(IEnumUInts)
		COM_INTERFACE_ENTRY(IEnumUIntsInit)
	END_COM_MAP()
};

OBJECT_ENTRY_AUTO(__uuidof(EnumUInts), CEnumUInts)
