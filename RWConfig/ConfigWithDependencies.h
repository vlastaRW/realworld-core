// ConfigWithDependencies.h : Declaration of the CConfigWithDependencies

#pragma once
#include "resource.h"       // main symbols
#include "RWConfig.h"
#include <SubjectImpl.h>
#include <ObserverImpl.h>
#include "ConfigWDItem.h"


// CConfig

class ATL_NO_VTABLE CConfigWithDependencies : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CConfigWithDependencies, &CLSID_ConfigWithDependencies>,
	public CSubjectImpl<IConfigWithDependencies, IConfigObserver, IUnknown*>,
	public CObserverImpl<CConfigWithDependencies, IConfigObserver, IUnknown*>,
	public IConfigCustomGUI,
	public ISubConfig
{
public:
	CConfigWithDependencies() : m_bUpdating(false)
	{
#ifdef DEBUG
	m_bFinalized = false;
#endif
	}
	~CConfigWithDependencies();

	DECLARE_NO_REGISTRY()

	static HRESULT WINAPI QICustomGUI(void* a_pThis, REFIID a_iid, void** a_ppv, DWORD_PTR a_dw)
	{
		CConfigWithDependencies* pThis = reinterpret_cast<CConfigWithDependencies*>(a_pThis);
		if (pThis->m_pCustomGUI)
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

	BEGIN_COM_MAP(CConfigWithDependencies)
		COM_INTERFACE_ENTRY2(IConfig, IConfigWithDependencies)
		COM_INTERFACE_ENTRY(IConfigWithDependencies)
		COM_INTERFACE_ENTRY_FUNC(__uuidof(IConfigCustomGUI), 0, QICustomGUI)
		COM_INTERFACE_ENTRY(ISubConfig)
	END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease() 
	{
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

	// IConfigWithDependencies methods
public:
	STDMETHOD(ItemIns1ofN)(BSTR a_bstrID, ILocalizedString* a_pName, ILocalizedString* a_pHelpText, const TConfigValue* a_ptDefaultValue, ISubConfig* a_pSubConfig);
	STDMETHOD(ItemOptionAdd)(BSTR a_bstrID, const TConfigValue* a_ptValue, ILocalizedString* a_pName, ULONG a_nConditions, const TConfigOptionCondition* a_aConditions);
	STDMETHOD(ItemInsRanged)(BSTR a_bstrID, ILocalizedString* a_pName, ILocalizedString* a_pHelpText, const TConfigValue* a_ptDefaultValue, ISubConfig* a_pSubConfig, const TConfigValue* a_ptFrom, const TConfigValue* a_ptTo, const TConfigValue* a_ptStep, ULONG a_nConditions, const TConfigOptionCondition* a_aConditions);
	STDMETHOD(ItemInsSimple)(BSTR a_bstrID, ILocalizedString* a_pName, ILocalizedString* a_pHelpText, const TConfigValue* a_ptDefaultValue, ISubConfig* a_pSubConfig, ULONG a_nConditions, const TConfigOptionCondition* a_aConditions);
	STDMETHOD(ItemIns1ofNWithCustomOptions)(BSTR a_bstrID, ILocalizedString* a_pName, ILocalizedString* a_pHelpText, const TConfigValue* a_ptDefaultValue, IConfigItemCustomOptions* a_pOptions, ISubConfig* a_pSubConfig, ULONG a_nConditions, const TConfigOptionCondition* a_aConditions);
	STDMETHOD(Finalize)(IConfigCustomGUI* a_pCustomGUI);

	// IConfigCustomGUI methods
public:
	STDMETHOD(UIDGet)(GUID* a_pguid);
	STDMETHOD(RequiresMargins)();
	STDMETHOD(MinSizeGet)(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY);
	STDMETHOD(WindowCreate)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow);

	// ISubConfig methods
public:
	STDMETHOD(ControllerSet)(TConfigValue const* a_ptValue)
	{
		return S_FALSE;
	}
	STDMETHOD(ISubConfig::ObserverIns)(IConfigObserver* a_pObserver, TCookie a_tCookie)
	{
		return CSubjectImpl<IConfigWithDependencies, IConfigObserver, IUnknown*>::ObserverIns(a_pObserver, a_tCookie);
	}
	STDMETHOD(ISubConfig::ObserverDel)(IConfigObserver* a_pObserver, TCookie a_tCookie)
	{
		return CSubjectImpl<IConfigWithDependencies, IConfigObserver, IUnknown*>::ObserverDel(a_pObserver, a_tCookie);
	}

	// internal IObserver wrapped method
public:
	HRESULT OwnerNotify(TCookie a_tCookie, IUnknown* a_pChangedParam);

private:
	struct SItem
	{
		CComBSTR bstrID;
		CConfigValue tActualValue;
		CConfigValue tDefaultValue;
		CComPtr<IConfigWDControl> pItem;
		CComPtr<ISubConfig> pHandler;
	};
	typedef map<CComBSTR, size_t> XItemIDs;
	typedef vector<SItem> AItems;

private:
	// find if the ID is valid and decompose it if it is a subitem
	HRESULT FindItem(BSTR a_bstrID, const SItem** a_ppItem, BSTR* a_pbstrSubID) const;

private:
	// Map with items indexes.
	XItemIDs m_xItemIDs;
	// Items.
	AItems m_aItems;
	// changes from sub-items
	bool m_bUpdating;
	bool m_bChanged;
	// original custom GUI
	CComPtr<IConfigCustomGUI> m_pCustomGUI;
#ifdef DEBUG
	bool m_bFinalized;
#endif
};

OBJECT_ENTRY_AUTO(__uuidof(ConfigWithDependencies), CConfigWithDependencies)
