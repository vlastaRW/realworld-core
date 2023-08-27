// StorageManager.h : Declaration of the CStorageManager

#pragma once
#include "RWStorage.h"
#include <PlugInCache.h>
#include <WeakSingleton.h>


// CStorageManager

class ATL_NO_VTABLE CStorageManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStorageManager, &CLSID_StorageManager>,
	public IStorageManager,
	public IGlobalConfigFactory
{
public:
	CStorageManager()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CStorageManager)
DECLARE_CLASSFACTORY_WEAKSINGLETON(CStorageManager)

BEGIN_CATEGORY_MAP(CStorageManager)
	IMPLEMENTED_CATEGORY(CATID_StorageFilterFactory)
	IMPLEMENTED_CATEGORY(CATID_GlobalConfigFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CStorageManager)
	COM_INTERFACE_ENTRY(IStorageManager)
	COM_INTERFACE_ENTRY(IGlobalConfigFactory)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		RWCoCreateInstance(m_pGlobalConfig, __uuidof(ConfigInMemory));
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IStorageManager methods
public:
	STDMETHOD(FilterCreate)(BSTR a_bstrFilter, DWORD a_dwFlags, IStorageFilter** a_ppFilter)
	{ return FilterCreateEx(NULL, a_bstrFilter, a_dwFlags, a_ppFilter); }
	STDMETHOD(FilterCreateInteractivelyCfg)(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, IConfig* a_pContextConfig, ILocalizedString* a_pCaption, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilter** a_ppFilter);
	STDMETHOD(FilterCreateInteractivelyCfgHelp)(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, IConfig* a_pContextConfig, ILocalizedString* a_pCaption, BSTR a_bstrHelpLink, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilter** a_ppFilter);
	STDMETHOD(FilterCreateInteractivelyUID)(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, REFGUID a_tContextID, ILocalizedString* a_pCaption, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilter** a_ppFilter);
	STDMETHOD(FilterWindowCreate)(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilterWindow** a_ppWindow);
	STDMETHOD(ConfigGetDefault)(IConfig** a_pConfig);
	STDMETHOD(FilterCreateEx)(IStorageFilter* a_pRoot, BSTR a_bstrLocation, DWORD a_dwFlags, IStorageFilter** a_ppFilter);

	// IGlobalConfigFactory methods
public:
	STDMETHOD(Interactive)(BYTE*) { return S_FALSE; }
	STDMETHOD(Name)(ILocalizedString** a_ppName) { return E_NOTIMPL; }
	STDMETHOD(Description)(ILocalizedString** a_ppDesc) { return E_NOTIMPL; }
	STDMETHOD(Config)(IConfig** a_ppConfig);

private:
	CComPtr<IConfigInMemory> m_pGlobalConfig;
};

OBJECT_ENTRY_AUTO(__uuidof(StorageManager), CStorageManager)
