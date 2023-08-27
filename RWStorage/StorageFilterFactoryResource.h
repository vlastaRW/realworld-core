// StorageFilterFactoryResource.h : Declaration of the CStorageFilterFactoryResource

#pragma once
#include "RWStorage.h"


// CStorageFilterFactoryResource

class ATL_NO_VTABLE CStorageFilterFactoryResource : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStorageFilterFactoryResource, &CLSID_StorageFilterFactoryResource>,
	public IStorageFilterFactory
{
public:
	CStorageFilterFactoryResource()
	{
	}

DECLARE_NO_REGISTRY()

BEGIN_CATEGORY_MAP(CStorageFilterFactoryResource)
	IMPLEMENTED_CATEGORY(CATID_StorageFilterFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CStorageFilterFactoryResource)
	COM_INTERFACE_ENTRY(IStorageFilterFactory)
END_COM_MAP()


	// IStorageFilterFactory methods
public:
	STDMETHOD(NameGet)(ILocalizedString** a_ppName);
	STDMETHOD(IconGet)(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(SupportsGUI)(DWORD a_dwFlags);
	STDMETHOD(FilterCreate)(BSTR a_bstrFilter, DWORD a_dwFlags, IStorageFilter** a_ppFilter);
	STDMETHOD(WindowCreate)(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilterWindow** a_ppWindow);
	STDMETHOD(ContextConfigGetDefault)(IConfig** a_ppConfig);
};

OBJECT_ENTRY_AUTO(__uuidof(StorageFilterFactoryResource), CStorageFilterFactoryResource)
