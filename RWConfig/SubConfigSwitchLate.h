// SubConfigSwitchLate.h : Declaration of the CSubConfigSwitchLate

#pragma once
#include "resource.h"       // main symbols
#include "RWConfig.h"
#include <SubjectImpl.h>
#include <ObserverImpl.h>


// CSubConfigSwitchLate

class ATL_NO_VTABLE CSubConfigSwitchLate : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSubConfigSwitchLate, &CLSID_SubConfigSwitchLate>,
	public CSubjectImpl<ISubConfigSwitchLate, IConfigObserver, IUnknown*>,
	public CObserverImpl<CSubConfigSwitchLate, IConfigObserver, IUnknown*>,
	public IConfigCustomGUI
{
public:
	CSubConfigSwitchLate() : m_pActiveConfig(NULL)
	{
	}
	~CSubConfigSwitchLate()
	{
		if (m_pActiveConfig)
		{
			m_pActiveConfig->ObserverDel(ObserverGet(), 0);
		}
	}

	DECLARE_NO_REGISTRY()

	static HRESULT WINAPI QICustomGUI(void* a_pThis, REFIID a_iid, void** a_ppv, DWORD_PTR a_dw)
	{
		CSubConfigSwitchLate* pThis = reinterpret_cast<CSubConfigSwitchLate*>(a_pThis);
		CComQIPtr<IConfigCustomGUI> pCust(pThis->m_pActiveConfig);
		if (pCust != NULL)
		{
			*a_ppv = reinterpret_cast<void*>(static_cast<IConfigCustomGUI*>(pThis));
			pThis->AddRef();
			return S_OK;
		}
		else
		{
			*a_ppv = NULL;
			return E_NOINTERFACE;
		}
	}

	BEGIN_COM_MAP(CSubConfigSwitchLate)
		COM_INTERFACE_ENTRY(IConfig)
		COM_INTERFACE_ENTRY(ISubConfig)
		COM_INTERFACE_ENTRY(ISubConfigSwitchLate)
		COM_INTERFACE_ENTRY_FUNC(__uuidof(IConfigCustomGUI), 0, QICustomGUI)
	END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pActiveConfig)
		{
			m_pActiveConfig->ObserverDel(ObserverGet(), 0);
		}
	}

	// IConfig methods
public:
	STDMETHOD(ItemIDsEnum)(IEnumStrings** a_ppIDs);
	STDMETHOD(ItemValueGet)(BSTR a_bstrID, TConfigValue* a_ptValue);
	STDMETHOD(ItemValuesSet)(ULONG a_nCount, BSTR* a_aIDs, const TConfigValue* a_atValues);
	STDMETHOD(ItemGetUIInfo)(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo);

	STDMETHOD(SubConfigGet)(BSTR a_bstrID, IConfig** a_ppSubConfig);
	STDMETHOD(DuplicateCreate)(IConfig** a_ppCopiedConfig);
	STDMETHOD(CopyFrom)(IConfig* a_pSource, BSTR a_bstrIDPrefix);

	// ISubConfig methods
public:
	STDMETHOD(ControllerSet)(const TConfigValue* a_ptValue);

	// ISubConfigSwitchLate methods
public:
	STDMETHOD(Init)(ILateConfigCreator* a_pCreator);

	// IConfigCustomGUI methods
public:
	STDMETHOD(UIDGet)(GUID* a_pguid);
	STDMETHOD(RequiresMargins)();
	STDMETHOD(MinSizeGet)(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY);
	STDMETHOD(WindowCreate)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow);

	// internal IObserver wrapped method
public:
	HRESULT OwnerNotify(TCookie a_tCookie, IUnknown* a_pChangedParam);

private:
	typedef map<CConfigValue, CComPtr<IConfig> > XSubConfigs;

private:
	CComPtr<ILateConfigCreator> m_pCreator;
	XSubConfigs m_xConfigCache;
	IConfig* m_pActiveConfig;
};

OBJECT_ENTRY_AUTO(__uuidof(SubConfigSwitchLate), CSubConfigSwitchLate)
