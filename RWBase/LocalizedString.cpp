// LocalizedString.cpp : Implementation of CLocalizedString

#include "stdafx.h"
#include "RWBase.h"
#include "LocalizedString.h"


// CLocalizedString


STDMETHODIMP CLocalizedString::Get(BSTR* a_pbstrString)
{
	try
	{
		*a_pbstrString = NULL;

		XLCID2LPWSTR::const_iterator i = m_xString.find(m_tDefaultLCID);
		if (i == m_xString.end())
		{
			i = m_xString.begin();
			if (i == m_xString.end())
				return E_FAIL;
		}
		*a_pbstrString = SysAllocString(i->second);
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrString == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CLocalizedString::GetLocalized(LCID a_tLCID, BSTR* a_pbstrString)
{
	try
	{
		*a_pbstrString = NULL;

		XLCID2LPWSTR::const_iterator i = m_xString.find(a_tLCID);
		if (i == m_xString.end())
		{
			i = m_xString.find(m_tDefaultLCID);
			if (i == m_xString.end())
			{
				i = m_xString.begin();
				if (i == m_xString.end())
					return E_FAIL;
			}
		}
		*a_pbstrString = SysAllocString(i->second);
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrString == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CLocalizedString::SetDefault(LCID a_tDefaultLCID)
{
	m_tDefaultLCID = a_tDefaultLCID;
	return S_OK;
}

STDMETHODIMP CLocalizedString::Insert(LCID a_tLCID, BSTR a_bstrString)
{
	ATLASSERT(m_xString.find(a_tLCID) == m_xString.end());
	try
	{
		m_xString[a_tLCID] = _wcsdup(a_bstrString);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

