// GlobalConfigManager.h : Declaration of the CGlobalConfigManager

#pragma once
#include "resource.h"       // main symbols
#include "RWConfig.h"
#include <WeakSingleton.h>
#include <CooperatingObjectsManager.h>
#include <SubjectImpl.h>
#include <ObserverImpl.h>


// CGlobalConfigManager

class ATL_NO_VTABLE CGlobalConfigManager :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGlobalConfigManager, &CLSID_GlobalConfigManager>,
	public IGlobalConfigManager,
	public CSubjectImpl<ISubConfig, IConfigObserver, IUnknown*>,
	public CObserverImpl<CGlobalConfigManager, IConfigObserver, IUnknown*>
{
public:
	CGlobalConfigManager() : m_nStamp(0), m_bUpdating(false), m_bChanged(false)
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CGlobalConfigManager)


BEGIN_COM_MAP(CGlobalConfigManager)
	COM_INTERFACE_ENTRY(IGlobalConfigManager)
	COM_INTERFACE_ENTRY(ISubConfig)
	COM_INTERFACE_ENTRY(IConfig)
END_COM_MAP()


	// IGlobalConfigManager methods
public:
	STDMETHOD(EnumIDs)(IEnumGUIDs** a_ppIDs);
	STDMETHOD(Interactive)(REFGUID a_tID, BYTE* a_pPriority);
	STDMETHOD(Name)(REFGUID a_tID, ILocalizedString** a_ppName);
	STDMETHOD(Description)(REFGUID a_tID, ILocalizedString** a_ppDesc);
	STDMETHOD(Config)(REFGUID a_tID, IConfig** a_ppConfig);

	STDMETHOD(GetValue)(REFGUID a_tID, BSTR a_bstrID, TConfigValue* a_pVal);

	// ISubConfig methods
public:
	STDMETHOD(ControllerSet)(TConfigValue const* UNREF(a_ptValue)) { return S_FALSE; }

	// IConfig methods
public:
	STDMETHOD(ItemIDsEnum)(IEnumStrings** a_ppIDs);
	STDMETHOD(ItemValueGet)(BSTR a_bstrID, TConfigValue* a_ptValue);
	STDMETHOD(ItemValuesSet)(ULONG a_nCount, BSTR* a_aIDs, TConfigValue const* a_atValues);
	STDMETHOD(ItemGetUIInfo)(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo);

	STDMETHOD(SubConfigGet)(BSTR a_bstrID, IConfig** a_ppSubConfig);
	STDMETHOD(DuplicateCreate)(IConfig** a_ppCopiedConfig);
	STDMETHOD(CopyFrom)(IConfig* a_pSource, BSTR a_bstrIDPrefix);

	HRESULT OwnerNotify(TCookie a_tCookie, IUnknown* a_pChangedParam)
	{
		if (m_bUpdating)
			m_bChanged = true;
		else
			Fire_Notify(a_pChangedParam);
		return S_OK;
	}

private:
	void Validate();

private:
	typedef std::map<CConfigValue, std::pair<CComPtr<IGlobalConfigFactory>, CComPtr<IConfig> > > CObjects;

private:
	CObjects m_cObjects;
	ULONG m_nStamp;
	bool m_bUpdating;
	bool m_bChanged;
};

OBJECT_ENTRY_AUTO(__uuidof(GlobalConfigManager), CGlobalConfigManager)
