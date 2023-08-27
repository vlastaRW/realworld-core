// DesignerFrameIconsManager.h : Declaration of the CDesignerFrameIconsManager

#pragma once
#include "resource.h"       // main symbols
#include "RWConceptDesignerExtension.h"
#include <WeakSingleton.h>


// CDesignerFrameIconsManager

class ATL_NO_VTABLE CDesignerFrameIconsManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerFrameIconsManager, &CLSID_DesignerFrameIconsManager>,
	public IDesignerFrameIcons,
	public IConfigItemCustomOptions,
	public IEnumConfigItemOptions
{
public:
	CDesignerFrameIconsManager() : m_bInitialized(false), m_nTimeStamp(0), m_nPluginTimeStamp(0)
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CDesignerFrameIconsManager)

BEGIN_COM_MAP(CDesignerFrameIconsManager)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
	COM_INTERFACE_ENTRY(IConfigItemCustomOptions)
	COM_INTERFACE_ENTRY(IEnumConfigItemOptions)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDesignerFrameIcons methods
public:
	STDMETHOD(TimeStamp)(ULONG* a_pTimeStamp);
	STDMETHOD(EnumIconIDs)(IEnumGUIDs** a_ppIDs);
	STDMETHOD(GetIcon)(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon);

	// IConfigItemCustomOptions methods
public:
	STDMETHOD(GetValueName)(const TConfigValue *a_pValue, ILocalizedString **a_ppName);

	// IEnumConfigItemOptions methods
public:
	STDMETHOD(Size)(ULONG *a_pnSize);
	STDMETHOD(Get)(ULONG a_nIndex, TConfigValue *a_ptItem);
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue *a_atItems);

private:
	template<class T>
	struct lessBinary
	{
		bool operator()(const T& a_1, const T& a_2) const
		{
			return memcmp(&a_1, &a_2, sizeof(T)) < 0;
		}
	};

	typedef std::map<GUID, CComPtr<IDesignerFrameIcons>, lessBinary<GUID> > CCache;
	typedef std::map<CComPtr<IDesignerFrameIcons>, ULONG> CTimeStamps;

private:
	void Init();

private:
	CCache m_cCache;
	CTimeStamps m_cTimeStamps;
	bool m_bInitialized;
	ULONG m_nTimeStamp;
	ULONG m_nPluginTimeStamp;
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerFrameIconsManager), CDesignerFrameIconsManager)
