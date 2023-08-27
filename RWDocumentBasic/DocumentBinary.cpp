// DocumentBinary.cpp : Implementation of CDocumentBinary

#include "stdafx.h"
#include "DocumentBinary.h"


// CDocumentBinary

STDMETHODIMP CDocumentBinary::DataCopy(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
{
	try
	{
		CComObject<CDocumentBinary>* p = NULL;
		CComObject<CDocumentBinary>::CreateInstance(&p);
		CComPtr<IDocumentData> pTmp = p;
		p->Init(m_cData.size(), &m_cData[0]); // vector hack
		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentBinary::Size(ULONG* a_pSize)
{
	try
	{
		CDocumentReadLock cLock(this);
		*a_pSize = m_cData.size();
		return S_OK;
	}
	catch (...)
	{
		return a_pSize ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentBinary::Data(ULONG a_nOffset, ULONG a_nSize, BYTE* a_pData)
{
	try
	{
		CDocumentReadLock cLock(this);
		if (a_nOffset+a_nSize > m_cData.size())
			return E_RW_INDEXOUTOFRANGE;
		std::copy(m_cData.begin()+a_nOffset, m_cData.begin()+a_nOffset+a_nSize, a_pData);
		return S_OK;
	}
	catch (...)
	{
		return a_pData ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentBinary::Insert(ULONG a_nOffset, ULONG a_nSize, BYTE const* a_pData)
{
	try
	{
		if (a_nSize == 0)
			return S_FALSE;
		CDocumentWriteLock cLock(this);
		if (a_nOffset > m_cData.size())
			return E_RW_INDEXOUTOFRANGE;
		m_cData.insert(m_cData.begin()+a_nOffset, a_pData, a_pData+a_nSize);
		AddChange(a_nOffset, m_cData.size()-a_nOffset+a_nSize);
		return S_OK;
	}
	catch (...)
	{
		return a_pData ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentBinary::Replace(ULONG a_nOffset, ULONG a_nSize, BYTE const* a_pData)
{
	try
	{
		if (a_nSize == 0)
			return S_FALSE;
		CDocumentWriteLock cLock(this);
		if (a_nOffset+a_nSize > m_cData.size())
			return E_RW_INDEXOUTOFRANGE;
		std::copy(a_pData, a_pData+a_nSize, m_cData.begin()+a_nOffset);
		AddChange(a_nOffset, m_cData.size()-a_nOffset-a_nSize);
		return S_OK;
	}
	catch (...)
	{
		return a_pData ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentBinary::Delete(ULONG a_nOffset, ULONG a_nSize)
{
	try
	{
		if (a_nSize == 0)
			return S_FALSE;
		CDocumentWriteLock cLock(this);
		if (a_nOffset+a_nSize > m_cData.size())
			return E_RW_INDEXOUTOFRANGE;
		m_cData.erase(m_cData.begin()+a_nOffset, m_cData.begin()+a_nOffset+a_nSize);
		AddChange(a_nOffset, m_cData.size()-a_nOffset);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

