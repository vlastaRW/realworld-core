
#pragma once

#define SHAREDSTRINGTABLE_STRING_INCLUDED

class CSharedStringTable
{
public:
	ILocalizedString* GetString(UINT a_uID)
	{
		return new CStr(a_uID);
	}

	CComPtr<ILocalizedString> GetStringAuto(UINT a_uID)
	{
		CComPtr<ILocalizedString> p;
		p.Attach(new CStr(a_uID));
		return p;
	}

private:
	class CStr : public ILocalizedString
	{
	public:
		CStr(UINT a_nID) : m_uID(a_nID), m_nRef(1)
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
					InterlockedIncrement(&m_nRef);
					return S_OK;
				}
				else if (IsEqualIID(a_tIID, IID_IUnknown))
				{
					*a_pp = static_cast<IUnknown*>(this);
					InterlockedIncrement(&m_nRef);
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

				HRSRC hRes = FindResourceEx(_pModule->get_m_hInst(), RT_STRING, MAKEINTRESOURCE(1+(m_uID>>4)), LANGIDFROMLCID(a_tLCID));
				bool bTranslate = false;
				if (hRes == NULL)
				{
					hRes = FindResourceEx(_pModule->get_m_hInst(), RT_STRING, MAKEINTRESOURCE(1+(m_uID>>4)), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT));
					if (hRes)
						bTranslate = true;
					else
						hRes = FindResource(_pModule->get_m_hInst(), MAKEINTRESOURCE(1+(m_uID>>4)), RT_STRING);
				}
				if (hRes == NULL) return E_FAIL; // TODO: ret. value?

				HGLOBAL hGlb = LoadResource(_pModule->get_m_hInst(), hRes);
				WCHAR *pszStrings = (WCHAR*)LockResource(hGlb);
				UINT i;
				for (i = 0; i < (m_uID&0xf); i++)
				{
					pszStrings += 1 + *pszStrings;
				}
				if (*pszStrings == 0)
					return E_FAIL;

				if (bTranslate)
				{
					CComPtr<ITranslator> pTr;
					RWCoCreateInstance(pTr, __uuidof(Translator));
					if (pTr)
					{
						CComBSTR bstr(*pszStrings, pszStrings+1);
						if (SUCCEEDED(pTr->Translate(bstr, a_tLCID, a_pbstrString)))
							return S_OK;
					}
				}

				*a_pbstrString = SysAllocStringLen(pszStrings+1, *pszStrings);

				return S_OK;
			}
			catch (...)
			{
				return a_pbstrString == NULL ? E_POINTER : E_UNEXPECTED;
			}
		}

	private:
		~CStr()
		{
			_pAtlModule->Unlock();
		}

	private:
		LONG volatile m_nRef;
		UINT m_uID;
	};
};

extern __declspec(selectany) CSharedStringTable _SharedStringTable;
