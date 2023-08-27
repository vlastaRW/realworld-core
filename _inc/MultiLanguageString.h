
#pragma once

#define MULTIMANGUAGE_STRING_INCLUDED 1


class CMultiLanguageString : public ILocalizedString
{
public:
	CMultiLanguageString(LPCWSTR a_psz, ITranslator* a_pTranslator = NULL) : m_bstr(::SysAllocString(a_psz)), m_nRef(1), m_pTranslator(a_pTranslator)
	{
		_pAtlModule->Lock();
	}
	CMultiLanguageString(BSTR a_bstr, ITranslator* a_pTranslator = NULL) : m_bstr(a_bstr), m_nRef(1), m_pTranslator(a_pTranslator)
	{
		_pAtlModule->Lock();
	}

	STDMETHOD_(ULONG, AddRef)()
	{
		return InterlockedIncrement(&m_nRef);
	}
	STDMETHOD_(ULONG, Release)()
	{
		ULONG n = InterlockedDecrement(&m_nRef);
		if (n == 0)
			delete this;
		return n;
	}
	STDMETHOD(QueryInterface)(IID const& a_tIID, void** a_pp)
	{
		try
		{
			*a_pp = NULL;
			if (IsEqualIID(a_tIID, __uuidof(ILocalizedString)))
			{
				*a_pp = static_cast<ILocalizedString*>(this);
				return S_OK;
			}
			else if (IsEqualIID(a_tIID, IID_IUnknown))
			{
				*a_pp = static_cast<IUnknown*>(this);
				return S_OK;
			}
			return E_NOINTERFACE;
		}
		catch (...)
		{
			return a_pp == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	STDMETHOD(Get)(BSTR* a_pbstrString)
	{
		return GetLocalized(GetThreadLocale(), a_pbstrString);
	}
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
	{
		try
		{
			*a_pbstrString = NULL;

			CComBSTR bstrLoc;
			GetLocalized(m_bstr, a_tLCID, &bstrLoc, m_pTranslator);
			*a_pbstrString = bstrLoc.Detach();

			return S_OK;
		}
		catch (...)
		{
			return a_pbstrString == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	static void GetLocalized(LPCOLESTR a_psz, LCID a_tLCID, BSTR* a_pLoc, ITranslator* a_pTranslator = NULL)
	{
		LPCOLESTR a_p = NULL;
		size_t a_n = 0;
		bool bTranslate = false;
		*a_pLoc = NULL;

		OLECHAR szExact[7];
		swprintf(szExact, 7, L"[%04x]", LANGIDFROMLCID(a_tLCID));
		static OLECHAR szEng[] = L"[0409]";
		LPCOLESTR pExact = NULL;
		LPCOLESTR pEng = NULL;
		LPCOLESTR pEngEnd = NULL;
		LPCOLESTR pFirst = NULL;
		LPCOLESTR pFirstEnd = NULL;
		LPCOLESTR pStart = a_psz;
		while (*a_psz)
		{
			if (*a_psz == L'[' &&
				((a_psz[1] >= L'0' && a_psz[1] <= L'9') || (a_psz[1] >= L'A' && a_psz[1] <= L'F') || (a_psz[1] >= L'a' && a_psz[1] <= L'f')) &&
				((a_psz[2] >= L'0' && a_psz[2] <= L'9') || (a_psz[2] >= L'A' && a_psz[2] <= L'F') || (a_psz[2] >= L'a' && a_psz[2] <= L'f')) &&
				((a_psz[3] >= L'0' && a_psz[3] <= L'9') || (a_psz[3] >= L'A' && a_psz[3] <= L'F') || (a_psz[3] >= L'a' && a_psz[3] <= L'f')) &&
				((a_psz[4] >= L'0' && a_psz[4] <= L'9') || (a_psz[4] >= L'A' && a_psz[4] <= L'F') || (a_psz[4] >= L'a' && a_psz[4] <= L'f')) &&
				a_psz[5] == L']')
			{
				if (0 == _wcsnicmp(szExact, a_psz, 6) || 0 == _wcsnicmp(L"[0000]", a_psz, 6))
				{
					a_psz += 6;
					pExact = a_psz;
				}
				else if (pExact)
				{
					a_p = pExact;
					a_n = a_psz-pExact;
					*a_pLoc = ::SysAllocStringLen(a_p, UINT(a_n));
					return;
				}
				else if (0 == _wcsnicmp(szEng, a_psz, 6))
				{
					a_psz += 6;
					pEng = a_psz;
				}
				else if (pEng)
				{
					pEngEnd = a_psz;
					a_psz += 6;
				}
				else if (pFirst == NULL)
				{
					a_psz += 6;
					pFirst = a_psz;
				}
				else if (pFirstEnd == NULL)
				{
					pFirstEnd = a_psz;
					a_psz += 6;
				}
				else
				{
					++a_psz;
				}
			}
			else
			{
				++a_psz;
			}
		}
		if (pExact)
		{
			a_p = pExact;
			a_n = a_psz-pExact;
		}
		else if (pEng)
		{
			a_p = pEng;
			a_n = pEngEnd ? pEngEnd-pEng : a_psz-pEng;
			bTranslate = true;
		}
		else if (pFirst)
		{
			a_p = pFirst;
			a_n = pFirstEnd ? pFirstEnd-pFirst : a_psz-pFirst;
		}
		else
		{
			a_p = pStart;
			a_n = a_psz-pStart;
			bTranslate = LANGIDFROMLCID(a_tLCID) != MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);
		}
		*a_pLoc = ::SysAllocStringLen(a_p, UINT(a_n));
		if (bTranslate)
		{
			CComPtr<ITranslator> pTr;
			if (a_pTranslator == NULL)
			{
				RWCoCreateInstance(pTr, __uuidof(Translator));
				a_pTranslator = pTr;
			}
			if (a_pTranslator)
			{
				CComBSTR bstrTranslated;
				a_pTranslator->Translate(*a_pLoc, a_tLCID, &bstrTranslated);
				if (bstrTranslated.m_str)
				{
					::SysFreeString(*a_pLoc);
					*a_pLoc = bstrTranslated.Detach();
				}
			}
		}
	}

	static CComPtr<ILocalizedString> GetAuto(LPCWSTR a_psz, ITranslator* a_pTranslator = NULL)
	{
		CComPtr<ILocalizedString> p;
		p.Attach(new CMultiLanguageString(a_psz, a_pTranslator));
		return p;
	}

private:
	~CMultiLanguageString()
	{
		SysFreeString(m_bstr);
		_pAtlModule->Unlock();
	}

private:
	LONG volatile m_nRef;
	BSTR m_bstr;
	CComPtr<ITranslator> m_pTranslator;
};
