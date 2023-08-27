
#pragma once


class CSimpleLocalizedString : public ILocalizedString
{
public:
	CSimpleLocalizedString(BSTR a_bstr) : m_bstr(a_bstr), m_nRef(1)
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

			if (m_bstr)
			{
				*a_pbstrString = ::SysAllocStringLen(m_bstr, ::SysStringLen(m_bstr));
			}
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrString == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

private:
	~CSimpleLocalizedString()
	{
		SysFreeString(m_bstr);
		_pAtlModule->Unlock();
	}

private:
	LONG volatile m_nRef;
	BSTR m_bstr;
};
