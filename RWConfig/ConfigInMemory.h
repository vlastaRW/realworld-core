// ConfigInMemory.h : Declaration of the CConfigInMemory

#pragma once
#include "resource.h"       // main symbols

#include "RWConfig.h"
#include <SubjectImpl.h>





// CConfigInMemory

class ATL_NO_VTABLE CConfigInMemory : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CConfigInMemory, &CLSID_ConfigInMemory>,
	public CSubjectImpl<IConfigInMemory, IConfigObserver, IUnknown*>,
	public ISubConfig
{
public:
	CConfigInMemory()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CConfigInMemory)
	COM_INTERFACE_ENTRY2(IConfig, IConfigInMemory)
	COM_INTERFACE_ENTRY(ISubConfig)
	COM_INTERFACE_ENTRY(IConfigInMemory)
END_COM_MAP()


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
	STDMETHOD(ControllerSet)(TConfigValue const* a_ptValue)
	{
		return S_FALSE;
	}
	STDMETHOD(ISubConfig::ObserverIns)(IConfigObserver* a_pObserver, TCookie a_tCookie)
	{
		return CSubjectImpl<IConfigInMemory, IConfigObserver, IUnknown*>::ObserverIns(a_pObserver, a_tCookie);
	}
	STDMETHOD(ISubConfig::ObserverDel)(IConfigObserver* a_pObserver, TCookie a_tCookie)
	{
		return CSubjectImpl<IConfigInMemory, IConfigObserver, IUnknown*>::ObserverDel(a_pObserver, a_tCookie);
	}

	// IConfigInMemory methods
public:
	STDMETHOD(DataBlockSet)(ULONG a_nSize, BYTE const* a_pData);
	STDMETHOD(DataBlockGet)(ULONG a_nSize, BYTE* a_pBuffer);
	STDMETHOD(DataBlockGetSize)(ULONG* a_pnSize);
	STDMETHOD(DeleteItems)(BSTR a_bstrPrefix);
	STDMETHOD(DataBlockGetData)(IReturnedData* buffer);
	STDMETHOD(TextBlockGet)(IReturnedData* buffer);

	HRESULT ParseTextConfig(BYTE const* b, BYTE const* const e);

	// internal methods
public:
	void ItemIDsEnum(LPCWSTR a_pszPrefix, IEnumStringsInit* a_pTmp);

private:
	class ATL_NO_VTABLE CSubConfigInMemory : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IConfig
	{
	public:
		CSubConfigInMemory()
		{
		}
		~CSubConfigInMemory()
		{
			if (m_pParent) m_pParent->Release();
		}

		void Init(CConfigInMemory* a_pParent, BSTR a_bstrPrefix)
		{
			m_bstrPrefix = a_bstrPrefix;
			(m_pParent = a_pParent)->AddRef();
		}

	BEGIN_COM_MAP(CSubConfigInMemory)
		COM_INTERFACE_ENTRY(IConfig)
	END_COM_MAP()

		// IConfig methods
	public:
		STDMETHOD(ItemIDsEnum)(IEnumStrings** a_ppIDs);
		STDMETHOD(ItemValueGet)(BSTR a_bstrID, TConfigValue* a_ptValue);
		STDMETHOD(ItemValuesSet)(ULONG a_nCount, BSTR* a_aIDs, const TConfigValue* a_atValues);
		STDMETHOD(ItemGetUIInfo)(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo);

		STDMETHOD(SubConfigGet)(BSTR a_bstrID, IConfig** a_ppSubConfig);
		STDMETHOD(DuplicateCreate)(IConfig** a_ppCopiedConfig);
		STDMETHOD(CopyFrom)(IConfig* a_pSource, BSTR a_bstrIDPrefix) { return E_NOTIMPL; }

		STDMETHOD(ObserverIns)(IConfigObserver* a_pObserver, TCookie a_tCookie);
		STDMETHOD(ObserverDel)(IConfigObserver* a_pObserver, TCookie a_tCookie);

	private:
		CComBSTR m_bstrPrefix;
		CConfigInMemory* m_pParent;
	};

	typedef std::map<std::wstring, CConfigValue> CValues;

private:
	static void ItemToText(std::wstring const& prefix, CValues::const_iterator& i, CValues::const_iterator const e, int tabs, std::vector<char>& dst);
	static bool ParseItem(BYTE const*& b, BYTE const* const e, wchar_t const* const prefix, int const pflen, CValues& vals);

	CValues m_cValues;
};

OBJECT_ENTRY_AUTO(__uuidof(ConfigInMemory), CConfigInMemory)
