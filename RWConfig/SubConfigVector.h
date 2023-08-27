// SubConfigVector.h : Declaration of the CSubConfigVector

#pragma once
#include "resource.h"       // main symbols

#include "RWConfig.h"
#include <SubjectImpl.h>
#include <ObserverImpl.h>

struct SConfigVector : public ISubConfigVector, public IConfigVector {};
// CSubConfigVector

class ATL_NO_VTABLE CSubConfigVector : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSubConfigVector, &CLSID_SubConfigVector>,
	public CSubjectImpl<SConfigVector, IConfigObserver, IUnknown*>,
	public CObserverImpl<CSubConfigVector, IConfigObserver, IUnknown*>
{
public:
	CSubConfigVector() : m_bUpdating(false)
	{
	}
	~CSubConfigVector();

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSubConfigVector)
	COM_INTERFACE_ENTRY2(IConfig, ISubConfig)
	COM_INTERFACE_ENTRY(ISubConfig)
	COM_INTERFACE_ENTRY(ISubConfigVector)
	COM_INTERFACE_ENTRY(IConfigVector)
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

	// IConfigVector methods
public:
	STDMETHOD(Swap)(ULONG a_nIndex1, ULONG a_nIndex2);
	STDMETHOD(Move)(ULONG a_nIndexSrc, ULONG a_nIndexDst);

	// ISubConfig methods
public:
	STDMETHOD(ControllerSet)(const TConfigValue* a_ptValue);

	// ISubConfigVector methods
public:
	STDMETHOD(Init)(BOOL a_bEditableNames, IConfig* a_pPattern);
	STDMETHOD(InitName)(IVectorItemName* a_pCustomName, IConfig* a_pPattern);

	// internal IObserver wrapped method
public:
	HRESULT OwnerNotify(TCookie a_tCookie, IUnknown* a_pChangedParam);

private:
	struct SItem
	{
		wstring strName;
		CComPtr<IConfig> pConfig;
	};
	typedef vector<SItem> AItems;

private:
	bool ParseID(const wchar_t* a_pszID, AItems::const_iterator& a_iItem, CComBSTR& a_bstrSubID) const;
	static void InsertSubCfgIDs(IEnumStringsInit* a_pESInit, const SItem& a_sItem, LPCTSTR a_pszPrefix);

private:
	CComPtr<IConfig> m_pPattern;
	bool m_bEditableNames;
	CComPtr<IVectorItemName> m_pCustomName;
	AItems m_aItems;
	bool m_bUpdating;
	bool m_bChanged;
};

OBJECT_ENTRY_AUTO(__uuidof(SubConfigVector), CSubConfigVector)
