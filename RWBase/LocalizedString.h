// LocalizedString.h : Declaration of the CLocalizedString

#pragma once


// CLocalizedString

class ATL_NO_VTABLE CLocalizedString : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CLocalizedString, &CLSID_LocalizedString>,
	public ILocalizedStringInit
{
public:
	CLocalizedString() : m_tDefaultLCID(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT))
	{
	}
	~CLocalizedString()
	{
		XLCID2LPWSTR::iterator i;
		for (i = m_xString.begin(); i != m_xString.end(); i++)
		{
			if (i->second)
				free(i->second);
		}
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CLocalizedString)
	COM_INTERFACE_ENTRY(ILocalizedString)
	COM_INTERFACE_ENTRY(ILocalizedStringInit)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// ILocalizedString methods
public:
	STDMETHOD(Get)(BSTR* a_pbstrString);
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString);

	// ILocalizedStringInit methods
public:
	STDMETHOD(SetDefault)(LCID a_tDefaultLCID);
	STDMETHOD(Insert)(LCID a_tLCID, BSTR a_bstrString);

private:
	typedef map<LCID, LPWSTR> XLCID2LPWSTR;

private:
	XLCID2LPWSTR m_xString;
	LCID m_tDefaultLCID;
};

OBJECT_ENTRY_AUTO(__uuidof(LocalizedString), CLocalizedString)
