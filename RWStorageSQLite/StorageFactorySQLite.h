// StorageFactorySQLite.h : Declaration of the CStorageFactorySQLite

#pragma once
#include "resource.h"       // main symbols

#include "RWStorageSQLite.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CStorageFactorySQLite

class ATL_NO_VTABLE CStorageFactorySQLite :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStorageFactorySQLite, &CLSID_StorageFactorySQLite>,
	public IStorageFilterFactory,
	public IGlobalConfigFactory,
	public IStorageFactorySQLite
{
public:
	CStorageFactorySQLite()
	{
	}

DECLARE_NO_REGISTRY()

BEGIN_CATEGORY_MAP(CStorageFactorySQLite)
	IMPLEMENTED_CATEGORY(CATID_StorageFilterFactory)
	IMPLEMENTED_CATEGORY(CATID_GlobalConfigFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CStorageFactorySQLite)
	COM_INTERFACE_ENTRY(IStorageFilterFactory)
	COM_INTERFACE_ENTRY(IGlobalConfigFactory)
	COM_INTERFACE_ENTRY(IStorageFactorySQLite)
END_COM_MAP()


	// IStorageFilterFactory methods
public:
	STDMETHOD(NameGet)(ILocalizedString** a_ppName);
	STDMETHOD(IconGet)(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(SupportsGUI)(DWORD a_dwFlags);
	STDMETHOD(FilterCreate)(BSTR a_bstrFilter, DWORD a_dwFlags, IStorageFilter** a_ppFilter);
	STDMETHOD(WindowCreate)(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilterWindow** a_ppWindow);
	STDMETHOD(ContextConfigGetDefault)(IConfig** a_ppConfig);

	// IGlobalConfigFactory methods
public:
	STDMETHOD(Interactive)(BYTE*) { return S_FALSE; }
	STDMETHOD(Name)(ILocalizedString** UNREF(a_ppName)) { return E_NOTIMPL; }
	STDMETHOD(Description)(ILocalizedString** UNREF(a_ppDesc)) { return E_NOTIMPL; }
	STDMETHOD(Config)(IConfig** a_ppConfig);

	// IStorageFactorySQLite methods
public:
	STDMETHOD(MergeDefaults)(ULONG a_nData, BYTE const* a_pData, ULONG a_nVersion, BSTR a_bstrDatabase);
	STDMETHOD(EnumFiles)(BSTR a_bstrNameFilter, ULONG a_nTags, BSTR* a_pTags, IEnum2Strings* a_pFiles, BSTR a_bstrDatabase);
};

OBJECT_ENTRY_AUTO(__uuidof(StorageFactorySQLite), CStorageFactorySQLite)
