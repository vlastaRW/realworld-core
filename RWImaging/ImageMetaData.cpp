// ImageMetaData.cpp : Implementation of CImageMetaData

#include "stdafx.h"
#include "ImageMetaData.h"


// CImageMetaData

STDMETHODIMP CImageMetaData::EnumIDs(IEnumStrings** a_ppBlockIDs)
{
	try
	{
		*a_ppBlockIDs = NULL;
		CComPtr<IEnumStringsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
		for (CBlocks::const_iterator i = m_cBlocks.begin(); i != m_cBlocks.end(); ++i)
			pTmp->Insert(i->first);

		*a_ppBlockIDs = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppBlockIDs ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CImageMetaData::SetBlock(BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData)
{
	try
	{
		if (a_nSize == 0)
			return DeleteBlock(a_bstrID);

		CAutoVectorPtr<BYTE> p(new BYTE[a_nSize]);
		CopyMemory(p.m_p, a_pData, a_nSize);
		CBlocks::iterator i = m_cBlocks.find(a_bstrID);
		if (i != m_cBlocks.end())
		{
			i->second.first = a_nSize;
			std::swap(i->second.second.m_p, p.m_p);
		}
		else
		{
			std::pair<ULONG, CAutoVectorPtr<BYTE> >& s = m_cBlocks[a_bstrID];
			s.first = a_nSize;
			s.second.Attach(p.Detach());
		}
		return S_OK;
	}
	catch (...)
	{
		return a_pData ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CImageMetaData::GetBlockSize(BSTR a_bstrID, ULONG* a_pSize)
{
	try
	{
		*a_pSize = 0;

		CBlocks::iterator i = m_cBlocks.find(a_bstrID);
		if (i != m_cBlocks.end())
		{
			*a_pSize = i->second.first;
			return S_OK;
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_pSize ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CImageMetaData::GetBlock(BSTR a_bstrID, ULONG a_nSize, BYTE* a_pData)
{
	try
	{
		CBlocks::iterator i = m_cBlocks.find(a_bstrID);
		if (i != m_cBlocks.end())
		{
			CopyMemory(a_pData, i->second.second.m_p, min(a_nSize, i->second.first));
			return S_OK;
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_pData ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CImageMetaData::DeleteBlock(BSTR a_bstrID)
{
	try
	{
		CBlocks::iterator i = m_cBlocks.find(a_bstrID);
		if (i != m_cBlocks.end())
			m_cBlocks.erase(i);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

