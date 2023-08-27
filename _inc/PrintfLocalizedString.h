
#pragma once


class CPrintfLocalizedString :
	public CComObjectRootEx<CComMultiThreadModel>,
	public ILocalizedString
{
public:
	void Init(ILocalizedString* a_pTemplate, ILocalizedString* a_pString, int a_nInt)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString;
		m_aInts[0] = a_nInt;
		m_nInts = 1;
		m_bStringFirst = true;
	}
	void Init(ILocalizedString* a_pTemplate, int a_nInt, ILocalizedString* a_pString)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString;
		m_aInts[0] = a_nInt;
		m_nInts = 1;
		m_bStringFirst = false;
	}
	void Init(ILocalizedString* a_pTemplate, ILocalizedString* a_pString)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString;
		m_nInts = 0;
	}
	void Init(ILocalizedString* a_pTemplate, int a_nInt)
	{
		m_pTemplate = a_pTemplate;
		m_aInts[0] = a_nInt;
		m_nInts = 1;
	}
	void Init(ILocalizedString* a_pTemplate, int a_nInt1, int a_nInt2)
	{
		m_pTemplate = a_pTemplate;
		m_aInts[0] = a_nInt1;
		m_aInts[1] = a_nInt2;
		m_nInts = 2;
	}
	void Init(ILocalizedString* a_pTemplate, int a_nInt1, int a_nInt2, int a_nInt3)
	{
		m_pTemplate = a_pTemplate;
		m_aInts[0] = a_nInt1;
		m_aInts[1] = a_nInt2;
		m_aInts[2] = a_nInt3;
		m_nInts = 3;
	}
	void Init(ILocalizedString* a_pTemplate, ILocalizedString* a_pString1, ILocalizedString* a_pString2)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString1;
		m_p2ndString = a_pString2;
		m_nInts = 0;
	}

	BEGIN_COM_MAP(CPrintfLocalizedString)
		COM_INTERFACE_ENTRY(ILocalizedString)
	END_COM_MAP()

	// ILocalizedString methods
public:
	STDMETHOD(Get)(BSTR* a_pbstrString)
	{
		return GetLocalized(GetThreadLocale(), a_pbstrString);
	}
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
	{
		try
		{
			*a_pbstrString = NULL;

			CComBSTR bstrTemplate;
			m_pTemplate->GetLocalized(a_tLCID, &bstrTemplate);
			CComBSTR bstrString;
			if (m_pString)
				m_pString->GetLocalized(a_tLCID, &bstrString);
			CComBSTR bstrString2;
			if (m_p2ndString)
				m_p2ndString->GetLocalized(a_tLCID, &bstrString2);
			CAutoVectorPtr<wchar_t> szBuffer(new wchar_t[bstrTemplate.Length()+bstrString.Length()+bstrString2.Length()+128]);
			if (m_nInts > 0)
			{
				if (m_pString)
					if (m_bStringFirst)
						swprintf(szBuffer.m_p, bstrTemplate, bstrString.m_str, m_aInts[0]);
					else
						swprintf(szBuffer.m_p, bstrTemplate, m_aInts[0], bstrString.m_str);
				else if (m_nInts == 1)
					swprintf(szBuffer.m_p, bstrTemplate, m_aInts[0]);
				else if (m_nInts == 2)
					swprintf(szBuffer.m_p, bstrTemplate, m_aInts[0], m_aInts[1]);
				else if (m_nInts == 3)
					swprintf(szBuffer.m_p, bstrTemplate, m_aInts[0], m_aInts[1], m_aInts[2]);
			}
			else
			{
				if (m_pString)
				{
					if (bstrString2.m_str)
						swprintf(szBuffer.m_p, bstrTemplate, bstrString.m_str, bstrString2.m_str);
					else
						swprintf(szBuffer.m_p, bstrTemplate, bstrString.m_str);
				}
				else
					wcscpy(szBuffer.m_p, bstrTemplate);
			}
			*a_pbstrString = SysAllocString(szBuffer.m_p);
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrString == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

private:
	CComPtr<ILocalizedString> m_pTemplate;
	CComPtr<ILocalizedString> m_pString;
	CComPtr<ILocalizedString> m_p2ndString;
	int m_aInts[3];
	int m_nInts;
	bool m_bStringFirst;
};

