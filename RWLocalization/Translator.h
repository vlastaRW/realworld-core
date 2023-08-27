// Translator.h : Declaration of the CTranslator

#pragma once
#include "resource.h"       // main symbols

#include <RWLocalization.h>
#include <WeakSingleton.h>
#include <SubjectImpl.h>


// CTranslator

class ATL_NO_VTABLE CTranslator :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTranslator, &CLSID_Translator>,
	public ITranslator,
	public CSubjectImpl<ITranslatorManager, ITranslatorObserver, ULONG>,
	public IGlobalConfigFactory,
	public ISubConfig
{
public:
	CTranslator() : m_bMonitor(false), m_bHilight(false), m_nVersion(0)
	{
		m_szAppPath[0] = L'\0';
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CTranslator)

BEGIN_CATEGORY_MAP(CTranslator)
	IMPLEMENTED_CATEGORY(CATID_GlobalConfigFactory)
END_CATEGORY_MAP()

	static HRESULT WINAPI QIConfig(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
	{
		CTranslator* const p = reinterpret_cast<CTranslator*>(pv);
		if (p->m_pConfig)
			return p->m_pConfig->QueryInterface(riid, ppv);
		return E_NOINTERFACE;
	}

BEGIN_COM_MAP(CTranslator)
	COM_INTERFACE_ENTRY(ITranslator)
	COM_INTERFACE_ENTRY(ITranslatorManager)
	COM_INTERFACE_ENTRY(IGlobalConfigFactory)
	COM_INTERFACE_ENTRY(IConfig)
	COM_INTERFACE_ENTRY(ISubConfig)
	COM_INTERFACE_ENTRY_FUNC_BLIND(0, QIConfig)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return CreateConfig();
	}

	void FinalRelease()
	{
	}

	// ITranslator methods
public:
	STDMETHOD(Translate)(BSTR a_bstrEng, LCID a_tLocaleID, BSTR* a_pTranslated);
	STDMETHOD(TranslateInPlace)(ULONG a_nBufLen, OLECHAR* a_pszBuffer, LCID a_tLocaleID);

	// ITranslatorManager methods
public:
	STDMETHOD(Initialize)(IApplicationInfo* a_pInfo);
	STDMETHOD(Finalize)(BOOL a_bAllowPOFileUpdates);

	STDMETHOD(LangIcon)(WORD a_wLangID, HICON* a_pLangIcon);
	STDMETHOD(LangInfo)(WORD a_wLangID, BOOL* a_pBuiltIn, ULONG* a_pTimestamp, ULONG* a_pStandard, ULONG* a_pCustom, ULONG* a_pMissing);

	STDMETHOD(StringsEnum)(WORD a_wLangID, IEnumStrings** a_ppStrings);
	STDMETHOD(StringInfo)(WORD a_wLangID, BSTR a_bstrOrig, BSTR* a_pbstrTransl, BOOL* a_pCustom, ULONG* a_pPriority);
	STDMETHOD(StringSet)(WORD a_wLangID, BSTR a_bstrOrig, BSTR a_bstrTransl, BOOL a_bCustom);

	STDMETHOD(Synchronize)(WORD a_wLangID, BYTE a_bUntranslatedStrings);

	// IGlobalConfigFactory methods
public:
	STDMETHOD(Interactive)(BYTE* a_pPriority);
	STDMETHOD(Name)(ILocalizedString** a_ppName);
	STDMETHOD(Description)(ILocalizedString** a_ppDesc);
	STDMETHOD(Config)(IConfig** a_ppConfig);

	// IConfig methods
public:
	STDMETHOD(ItemIDsEnum)(IEnumStrings** a_ppIDs)
	{
		return m_pConfig->ItemIDsEnum(a_ppIDs);
	}
	STDMETHOD(ItemValueGet)(BSTR a_bstrID, TConfigValue* a_ptValue)
	{
		return m_pConfig->ItemValueGet(a_bstrID, a_ptValue);
	}
	STDMETHOD(ItemValuesSet)(ULONG a_nCount, BSTR* a_aIDs, const TConfigValue* a_atValues)
	{
		CConfigValue old;
		m_pConfig->ItemValueGet(m_bstrCFGID, &old);
		HRESULT hRes = m_pConfig->ItemValuesSet(a_nCount, a_aIDs, a_atValues);
		CConfigValue cur;
		m_pConfig->ItemValueGet(m_bstrCFGID, &cur);
		SyncCachedValues();
		if (old != cur)
			Fire_Notify(ETCCurrentLanguage);
		return hRes;
	}
	STDMETHOD(ItemGetUIInfo)(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
	{
		return m_pConfig->ItemGetUIInfo(a_bstrID, a_iidInfo, a_ppItemInfo);
	}

	STDMETHOD(SubConfigGet)(BSTR a_bstrID, IConfig** a_ppSubConfig)
	{
		return m_pConfig->SubConfigGet(a_bstrID, a_ppSubConfig);
	}
	STDMETHOD(DuplicateCreate)(IConfig** a_ppCopiedConfig)
	{
		return m_pConfig->DuplicateCreate(a_ppCopiedConfig);
	}
	STDMETHOD(CopyFrom)(IConfig* a_pSource, BSTR a_bstrIDPrefix)
	{
		HRESULT hRes = m_pConfig->CopyFrom(a_pSource, a_bstrIDPrefix);
		SyncCachedValues();
		return hRes;
	}

	STDMETHOD(ObserverIns)(IConfigObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pConfig->ObserverIns(a_pObserver, a_tCookie);
	}
	STDMETHOD(ObserverDel)(IConfigObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pConfig->ObserverDel(a_pObserver, a_tCookie);
	}

	// ISubConfig methods
public:
	STDMETHOD(ControllerSet)(TConfigValue const* a_ptValue)
	{
		return S_FALSE;
	}

private:
	struct STranslation
	{
		STranslation() : nUses(0), bCustom(false) {}
		CComBSTR bstrTranslation;
		size_t nUses;
		bool bCustom;
	};
	typedef std::map<CComBSTR, STranslation> CTable;
	struct STable
	{
		CTable cTable;
		ULONG nTimestamp;
		DWORD aIcPal[10];
		BYTE aIcon[256];
		bool bChanged;

		bool IconValid() const;
	};
	typedef std::map<WORD, STable> CLanguages;

private:
	STable& GetTable(WORD a_wLangID);
	static void ParsePOFile(char const* a_pData, ULONG a_nLen, STable& a_cTbl);
	static void SerializePOFile(STable const& a_cTbl, std::vector<char>& a_cOut);
	void GetPathToPOFile(WORD a_wLangID, TCHAR* a_pszBuffer, ULONG a_nLength);
	HRESULT CreateConfig();
	void SyncCachedValues();

private:
	CLanguages m_cLanguages;
	OLECHAR m_szAppPath[MAX_PATH];
	CComBSTR m_bstrAppID;
	ULONG m_nVersion;
	CComPtr<IApplicationInfo> m_pInfo;

	CComPtr<IConfigWithDependencies> m_pConfig;
	bool m_bMonitor;
	bool m_bHilight;
	CComBSTR m_bstrCFGID;
};

OBJECT_ENTRY_AUTO(__uuidof(Translator), CTranslator)
