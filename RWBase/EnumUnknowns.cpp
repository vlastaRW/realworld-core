// EnumUnknowns.cpp : Implementation of CEnumUnknowns

#include "stdafx.h"
#include "RWBase.h"
#include "EnumUnknowns.h"


// CEnumUnknowns

STDMETHODIMP CEnumUnknowns::Size(ULONG *a_pnSize)
{
	CHECKPOINTER(a_pnSize);

	*a_pnSize = static_cast<ULONG>(m_aItems.size());

	return S_OK;
}

STDMETHODIMP CEnumUnknowns::Get(ULONG a_nIndex, REFIID a_iid, void** a_ppItem)
{
	CHECKPOINTER(a_ppItem);
	*a_ppItem = NULL;

	if (a_nIndex >= m_aItems.size())
		return E_RW_INDEXOUTOFRANGE;

	Lock();
	HRESULT hRes = m_aItems[a_nIndex]->QueryInterface(a_iid, a_ppItem);
	Unlock();

	return hRes;
}

STDMETHODIMP CEnumUnknowns::GetMultiple(ULONG a_nIndexFirst, ULONG a_nCount, REFIID a_iid, void** a_apItems)
{
	CHECKPOINTER(a_apItems);
	ZeroMemory(a_apItems, sizeof(*a_apItems)*a_nCount);

	if ((a_nIndexFirst+a_nCount) > m_aItems.size())
		return E_RW_INDEXOUTOFRANGE;

	vector<IUnknown*>::const_iterator i;

	Lock();
	for (i = m_aItems.begin() + a_nIndexFirst; a_nCount; a_nCount--, i++, a_apItems++)
	{
		// TODO: error checks
		(*i)->QueryInterface(a_iid, a_apItems);
	}
	Unlock();

	return S_OK;
}

STDMETHODIMP CEnumUnknowns::Insert(IUnknown* a_pItem)
{
	CHECKPOINTER(a_pItem);

	HRESULT hRet = S_OK;
	Lock();
	try
	{
		m_aItems.push_back(a_pItem);
		a_pItem->AddRef();
	}
	catch (...)
	{
		hRet = E_FAIL; // TODO: better return code
		// TODO: rollback
	}
	Unlock();

	return hRet;
}

STDMETHODIMP CEnumUnknowns::InsertMultiple(ULONG a_nCount, IUnknown* const* a_apItems)
{
	HRESULT hRet = S_OK;
	Lock();
	try
	{
		for (ULONG i = 0; i < a_nCount; i++)
		{
			m_aItems.push_back(a_apItems[i]);
			a_apItems[i]->AddRef();
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

STDMETHODIMP CEnumUnknowns::InsertFromEnum(IEnumUnknowns* a_pSource)
{
	CHECKPOINTER(a_pSource);

	HRESULT hRet = S_OK;
	Lock();
	try
	{
		ULONG size = 0;
		a_pSource->Size(&size);
		for(ULONG i=0; i<size; i++)
		{
			CComPtr<IUnknown> pItem;
			a_pSource->Get(i, __uuidof(IUnknown), reinterpret_cast<void**>(&pItem));
			if(pItem)
			{
				m_aItems.push_back(pItem);
				pItem.Detach();
			}
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
