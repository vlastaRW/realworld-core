// StorageFilterResource.cpp : Implementation of CStorageFilterResource

#include "stdafx.h"
#include "StorageFilterResource.h"


// CStorageFilterResource

bool CStorageFilterResource::Init(LPCTSTR a_pszFilter)
{
	if (a_pszFilter)
	{
		m_strFilter = a_pszFilter;
		return true;
	}
	return false;
}

void CStorageFilterResource::SrcFinished()
{
	Lock();
	m_pSrcDirect = NULL;
	Unlock();
}

STDMETHODIMP CStorageFilterResource::ToText(IStorageFilter* a_pRoot, BSTR* a_pbstrFilter)
{
	ATLASSERT(a_pRoot == NULL); // not implemented
	*a_pbstrFilter = CComBSTR((std::tstring(_T("res://"))+m_strFilter).c_str()).Detach();
	return S_OK;
}

STDMETHODIMP CStorageFilterResource::SubFilterGet(BSTR a_bstrRelativeLocation, IStorageFilter** a_ppFilter)
{
	*a_ppFilter = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterResource::SrcOpen(IDataSrcDirect** a_ppSrc)
{
	if (a_ppSrc) *a_ppSrc = NULL;

	Lock();

	if (m_pSrcDirect)
	{
		if (a_ppSrc)
		{
			*a_ppSrc = m_pSrcDirect;
			m_pSrcDirect->AddRef();
		}
		Unlock();
		return S_OK;
	}
	CComObject<CSrcDirect>::CreateInstance(&m_pSrcDirect);
	CComPtr<IDataSrcDirect> pTmp = m_pSrcDirect;
	HRESULT hRes = m_pSrcDirect->Init(this, m_strFilter.c_str());
	if (FAILED(hRes))
	{
		Unlock();
		return hRes;
	}

	if (a_ppSrc)
		*a_ppSrc = pTmp.Detach();
	Unlock();

	return S_OK;
}

STDMETHODIMP CStorageFilterResource::DstOpen(IDataDstStream** a_ppDst)
{
	*a_ppDst = NULL;
	return E_NOTIMPL;
}

HRESULT CStorageFilterResource::CSrcDirect::Init(CStorageFilterResource* a_pOwner, LPCTSTR a_pszLocation)
{
	size_t nLen = _tcslen(a_pszLocation);
	if (nLen <= 2) // bare minimum
		return E_FAIL;

	ATLASSERT(m_pOwner == NULL);
	ATLASSERT(a_pOwner != NULL);
	(m_pOwner = a_pOwner)->AddRef();

	TCHAR szPath[MAX_PATH] = _T("");
	TCHAR szType[128] = _T("");
	TCHAR szName[128] = _T("");

	size_t nSlashCount = 0;
	size_t i;
	LPTSTR pDst = szPath;
	for (i = 0; i < nLen; i++)
	{
		if (a_pszLocation[i] == _T('/'))
		{
			switch (nSlashCount++)
			{
			case 0:
				*pDst = _T('\0');
				pDst = szType;
				break;
			case 1:
				*pDst = _T('\0');
				pDst = szName;
				break;
			default:
				return E_FAIL;
			}
		}
		else
		{
			*(pDst++) = a_pszLocation[i];
		}
	}
	if (nSlashCount == 0)
		return E_FAIL;
	if (nSlashCount == 1)
	{
		_tcscpy(szName, szType);
		_tcscpy(szType, _T("RT_HTML"));
	}

	m_hModule = LoadLibraryEx(szPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (m_hModule == NULL)
		return E_FAIL;

	HRSRC hRsrc = FindResource(m_hModule, szName, szType);
	if (hRsrc == NULL)
		return E_FAIL;

	HGLOBAL hGlb = LoadResource(m_hModule, hRsrc);
	m_pData = reinterpret_cast<BYTE*>(LockResource(hGlb));
	m_nDataSize = SizeofResource(m_hModule, hRsrc);
	return S_OK;
}

STDMETHODIMP CStorageFilterResource::CSrcDirect::SizeGet(ULONG* a_pnSize)
{
	try
	{
		*a_pnSize = m_nDataSize;
		return S_OK;
	}
	catch (...)
	{
		return a_pnSize == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterResource::CSrcDirect::SrcLock(ULONG a_nOffset, ULONG a_nSize, BYTE const** a_ppBuffer)
{
	try
	{
		*a_ppBuffer = NULL;
		if ((a_nOffset+a_nSize) > m_nDataSize)
			return E_FAIL;

		*a_ppBuffer = m_pData+a_nOffset;
		return S_OK;
	}
	catch (...)
	{
		return a_ppBuffer == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterResource::CSrcDirect::SrcUnlock(ULONG UNREF(a_nSize), BYTE const* UNREF(a_pBuffer))
{
	return S_OK;
}

