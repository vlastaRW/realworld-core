// EnumStructs.cpp : Implementation of CEnumStructs

#include "stdafx.h"
#include "EnumStructs.h"


// CEnumStructs

// TODO: locks

STDMETHODIMP CEnumStructs_::Size(ULONG a_nStructSize, ULONG *a_pnSize)
{
	try
	{
		ATLASSERT(a_nStructSize != 0);
		ATLASSERT((m_aItems.size()/a_nStructSize)*a_nStructSize == m_aItems.size());

		*a_pnSize = static_cast<ULONG>(m_aItems.size())/a_nStructSize;
		return S_OK;
	}
	catch (...)
	{
		return a_pnSize == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CEnumStructs_::Get(ULONG a_nIndex, ULONG a_nStructSize, BYTE* a_pItem)
{
	try
	{
		ATLASSERT(a_nStructSize != 0);
		ATLASSERT((m_aItems.size()/a_nStructSize)*a_nStructSize == m_aItems.size());

		ULONG nFirst = a_nIndex*a_nStructSize;
		if ((nFirst+a_nStructSize) > m_aItems.size())
			return E_RW_INDEXOUTOFRANGE;

		std::copy(m_aItems.begin()+nFirst, m_aItems.begin()+nFirst+a_nStructSize, a_pItem);

		return S_OK;
	}
	catch (...)
	{
		return a_pItem == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CEnumStructs_::GetMultiple(ULONG a_nIndexFirst, ULONG a_nCount, ULONG a_nStructSize, BYTE* a_aItems)
{
	try
	{
		ATLASSERT(a_nStructSize != 0);
		ATLASSERT((m_aItems.size()/a_nStructSize)*a_nStructSize == m_aItems.size());

		ULONG nFirst = a_nIndexFirst*a_nStructSize;
		ULONG nLen = a_nCount*a_nStructSize;
		if ((nFirst+nLen) > m_aItems.size())
			return E_RW_INDEXOUTOFRANGE;

		std::copy(m_aItems.begin()+nFirst, m_aItems.begin()+nFirst+nLen, a_aItems);

		return S_OK;
	}
	catch (...)
	{
		return a_aItems == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CEnumStructs_::Insert(ULONG a_nStructSize, BYTE const* a_pItem)
{
	try
	{
		ATLASSERT(a_nStructSize != 0);
		ATLASSERT((m_aItems.size()/a_nStructSize)*a_nStructSize == m_aItems.size());

		size_t const nOldSize = m_aItems.size();
		m_aItems.resize(nOldSize+a_nStructSize);
		std::copy(a_pItem, a_pItem+a_nStructSize, m_aItems.begin()+nOldSize);
		return S_OK;
	}
	catch (...)
	{
		return a_pItem == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CEnumStructs_::InsertMultiple(ULONG a_nCount, ULONG a_nStructSize, BYTE const* a_aItems)
{
	try
	{
		ATLASSERT(a_nStructSize != 0);
		ATLASSERT((m_aItems.size()/a_nStructSize)*a_nStructSize == m_aItems.size());

		size_t const nOldSize = m_aItems.size();
		size_t const nNewSize = nOldSize+a_nStructSize*a_nCount;
		m_aItems.resize(nNewSize);
		std::copy(a_aItems, a_aItems+a_nStructSize*a_nCount, m_aItems.begin()+nOldSize);
		return S_OK;
	}
	catch (...)
	{
		return a_aItems == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CEnumStructs_::InsertFromEnum(ULONG a_nStructSize, IEnumStructs* a_pSource)
{
	try
	{
		ULONG nItems = 0;
		HRESULT hRes = a_pSource->Size(a_nStructSize, &nItems);
		if (nItems == 0) return hRes;

		BYTE *pBuffer = new BYTE[nItems*a_nStructSize];
		hRes = a_pSource->GetMultiple(0, nItems, a_nStructSize, pBuffer);
		if (FAILED(hRes)) {delete[] pBuffer; return hRes;}

		hRes = InsertMultiple(nItems, a_nStructSize, pBuffer);
		delete[] pBuffer;

		return hRes;
	}
	catch (...)
	{
		return a_pSource == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

