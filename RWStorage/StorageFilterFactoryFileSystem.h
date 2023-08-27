// StorageFilterFactoryFileSystem.h : Declaration of the CStorageFilterFactoryFileSystem

#pragma once
#include "RWStorage.h"


// CStorageFilterFactoryFileSystem

class ATL_NO_VTABLE CStorageFilterFactoryFileSystem : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStorageFilterFactoryFileSystem, &CLSID_StorageFilterFactoryFileSystem>,
	public IStorageFilterFactory
{
public:
	CStorageFilterFactoryFileSystem()
	{
	}

DECLARE_NO_REGISTRY()

BEGIN_CATEGORY_MAP(CStorageFilterFactoryFileSystem)
	IMPLEMENTED_CATEGORY(CATID_StorageFilterFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CStorageFilterFactoryFileSystem)
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

OBJECT_ENTRY_AUTO(__uuidof(StorageFilterFactoryFileSystem), CStorageFilterFactoryFileSystem)
