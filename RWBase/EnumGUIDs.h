// EnumGUIDs.h : Declaration of the CEnumGUIDs

#pragma once
#include "resource.h"       // main symbols
#include "EnumXxxxImpl.h"



// CEnumGUIDs

class ATL_NO_VTABLE CEnumGUIDs : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEnumGUIDs, &CLSID_EnumGUIDs>,
	public CEnumXxxxImpl<CEnumGUIDs, IEnumGUIDs, IEnumGUIDsInit, GUID>
{
public:
	CEnumGUIDs()
	{
	}

	DECLARE_NO_REGISTRY()


	BEGIN_COM_MAP(CEnumGUIDs)
		COM_INTERFACE_ENTRY(IEnumGUIDs)
		COM_INTERFACE_ENTRY(IEnumGUIDsInit)
	END_COM_MAP()
};

OBJECT_ENTRY_AUTO(__uuidof(EnumGUIDs), CEnumGUIDs)
