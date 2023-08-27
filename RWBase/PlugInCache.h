// PlugInCache.h : Declaration of the CPlugInCache

#pragma once
#include "resource.h"       // main symbols
#include "RWBase.h"


// CPlugInCache

class ATL_NO_VTABLE CPlugInCache : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPlugInCache, &CLSID_PlugInCache>,
	public IPlugInCache
{
public:
	CPlugInCache()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CPlugInCache)

BEGIN_COM_MAP(CPlugInCache)
	COM_INTERFACE_ENTRY(IPlugInCache)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IPlugInCache methods
public:
	STDMETHOD(CLSIDsEnum)(REFGUID a_guidCategory, DWORD a_nMaxAge, IEnumGUIDs** a_ppCLSIDs);
	STDMETHOD(InterfacesEnum)(REFGUID a_guidCategory, REFIID a_iidRequiredInterface, DWORD a_nMaxAge, IEnumUnknowns** a_ppObjects, IEnumGUIDs** a_ppCLSIDs);
};

OBJECT_ENTRY_AUTO(__uuidof(PlugInCache), CPlugInCache)
