// ConfigVctItem.h : Declaration of the CConfigVctItem

#pragma once
#include "resource.h"       // main symbols

#include "RWConfig.h"


// CConfigVctItem

class ATL_NO_VTABLE CConfigVctItem : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigItemSimple
{
public:
	CConfigVctItem()
	{
	}


BEGIN_COM_MAP(CConfigVctItem)
	COM_INTERFACE_ENTRY(IConfigItem)
	COM_INTERFACE_ENTRY(IConfigItemSimple)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	HRESULT Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, IVectorItemName* a_pCust = NULL, IConfig* a_pConfig = NULL, ULONG a_nIndex = 0);

	// IConfigItem methods
public:
	STDMETHOD(NameGet)(ILocalizedString** a_ppName, ILocalizedString** a_ppHelpText);
	STDMETHOD(ValueGetName)(const TConfigValue* a_ptValue, ILocalizedString** a_ppName);
	STDMETHOD(ValueIsValid)(const TConfigValue* a_ptValue);
	STDMETHOD(Default)(TConfigValue* a_ptValue) { return E_NOTIMPL; }

	// IConfigItemSimple methods
public:
	STDMETHOD(IsEnabled)();

public:
	CComPtr<ILocalizedString> m_pName;
	CComPtr<ILocalizedString> m_pDesc;
	CComPtr<IVectorItemName> m_pCust;
	CComPtr<IConfig> m_pConfig;
	ULONG m_nIndex;
};

