// EnumStrings.cpp : Implementation of CEnumStrings

#include "stdafx.h"
#include "RWBase.h"
#include "EnumStrings.h"


// CEnumStrings

STDMETHODIMP CEnumStrings::Size(ULONG *a_pnSize)
{
	CHECKPOINTER(a_pnSize);

	*a_pnSize = static_cast<ULONG>(m_aItems.size());

	return S_OK;
}

STDMETHODIMP CEnumStrings::Get(ULONG a_nIndex, BSTR* a_pbstrItem)
{
	CHECKPOINTER(a_pbstrItem);
	*a_pbstrItem = NULL;

	if (a_nIndex >= m_aItems.size())
		return E_RW_INDEXOUTOFRANGE;

	Lock();
	*a_pbstrItem = SysAllocString(m_aItems[a_nIndex]);
	Unlock();

	return S_OK;
}

STDMETHODIMP CEnumStrings::GetMultiple(ULONG a_nIndexFirst, ULONG a_nCount, BSTR* a_abstrItems)
{
	CHECKPOINTER(a_abstrItems);
	ZeroMemory(a_abstrItems, sizeof(*a_abstrItems)*a_nCount);

	if ((a_nIndexFirst+a_nCount) > m_aItems.size())
		return E_RW_INDEXOUTOFRANGE;

	vector<BSTR>::const_iterator i;

	Lock();
	for (i = m_aItems.begin() + a_nIndexFirst; a_nCount; a_nCount--, i++, a_abstrItems++)
	{
		// TODO: error checks
		*a_abstrItems = SysAllocString(*i);
	}
	Unlock();

	return S_OK;
}

STDMETHODIMP CEnumStrings::Insert(BSTR a_bstrItem)
{
	CHECKPOINTER(a_bstrItem);

	HRESULT hRet = S_OK;
	Lock();
	try
	{
		m_aItems.push_back(SysAllocString(a_bstrItem));
	}
	catch (...)
	{
		hRet = E_FAIL; // TODO: better return code
		// TODO: rollback
	}
	Unlock();

	return hRet;
}

STDMETHODIMP CEnumStrings::InsertMultiple(ULONG a_nCount, BSTR const* a_abstrItems)
{
	CHECKPOINTER(a_abstrItems);

	HRESULT hRet = S_OK;
	Lock();
	try
	{
		ULONG i;
		for (i = 0; i < a_nCount; i++)
		{
			m_aItems.push_back(SysAllocString(a_abstrItems[i]));
		}
	}
	catch (...)
	{
		hRet = E_FAIL; // TODO: better return code
		// TODO: rollback
	}
	Unlock();

	return hRet;
}

STDMETHODIMP CEnumStrings::InsertFromEnum(IEnumStrings* a_pSource)
{
	CHECKPOINTER(a_pSource);

	HRESULT hRet = S_OK;
	Lock();
	try
	{
		ULONG i;
		BSTR bstrTmp = NULL;
		for (i = 0; SUCCEEDED(a_pSource->Get(i, &bstrTmp)); i++)
		{
			m_aItems.push_back(SysAllocString(bstrTmp));
			SysFreeString(bstrTmp);
			bstrTmp = NULL;
		}
	}
	catch (...)
	{
		hRet = E_FAIL; // TODO: better return code
		// TODO: rollback
	}
	Unlock();

	return hRet;
}

