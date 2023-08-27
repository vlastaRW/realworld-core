#pragma once

#ifdef __cplusplus

class CStorageFilter
{
public:
	CStorageFilter(LPCSTR a_pszFilter, IStorageFilter* a_pRoot = NULL) : m_pSF(NULL)
	{
		USES_CONVERSION;
		BSTR bstr = SysAllocString(A2COLE(a_pszFilter));
		Create(bstr, a_pRoot);
		SysFreeString(bstr);
	}

	CStorageFilter(LPCWSTR a_pszFilter, IStorageFilter* a_pRoot = NULL) : m_pSF(NULL)
	{
		BSTR bstr = SysAllocString(a_pszFilter);
		Create(bstr, a_pRoot);
		SysFreeString(bstr);
	}

	CStorageFilter(BSTR a_bstrFilter, IStorageFilter* a_pRoot = NULL) : m_pSF(NULL)
	{
		Create(a_bstrFilter, a_pRoot);
	}

	CStorageFilter(const CStorageFilter& a_cOrig) : m_pSF(NULL)
	{
		if (a_cOrig.m_pSF)
		{
			(m_pSF = a_cOrig.m_pSF)->AddRef();
		}

	}

	~CStorageFilter()
	{
		Destroy();
	}

	const CStorageFilter& operator =(const CStorageFilter& a_cOrig)
	{
		if (this != &a_cOrig)
		{
			Destroy();
			if (a_cOrig.m_pSF)
			{
				(m_pSF = a_cOrig.m_pSF)->AddRef();
			}
		}
		return *this;
	}

	operator IStorageFilter*() const
	{
		return m_pSF;
	}

private:
	void Create(BSTR a_bstrFileName, IStorageFilter* a_pRoot)
	{
		CComPtr<IStorageManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(StorageManager));
		pMgr->FilterCreateEx(a_pRoot, a_bstrFileName, 0, &m_pSF);
	}
	void Destroy()
	{
		if (m_pSF)
		{
			m_pSF->Release();
			m_pSF = NULL;
		}
	}

	IStorageFilter* m_pSF;
};

class CDirectInputLock
{
public:
	CDirectInputLock(IStorageFilter* a_pFlt) : m_nSize(0), m_pBuffer(NULL)
	{
		HRESULT hRes = E_UNEXPECTED;
		if (a_pFlt == NULL || FAILED(hRes = a_pFlt->SrcOpen(&m_pSrc)))
			throw E_FAIL;
		if (FAILED(hRes = m_pSrc->SizeGet(&m_nSize)) || FAILED(hRes = m_pSrc->SrcLock(0, m_nSize, &m_pBuffer)))
		{
			m_pSrc = NULL;
			throw hRes;
		}
	}
	CDirectInputLock(IDataSrcDirect* a_pSrc, ULONG a_nSize, ULONG a_nOffset = 0) :
		m_nSize(a_nSize), m_pBuffer(NULL)
	{
		HRESULT hRes = a_pSrc->SrcLock(a_nOffset, a_nSize, &m_pBuffer);
		if (SUCCEEDED(hRes))
			m_pSrc = a_pSrc;
		else
			throw hRes;
	}
	~CDirectInputLock()
	{
		if (m_pSrc != NULL)
			m_pSrc->SrcUnlock(m_nSize, m_pBuffer);
	}
	operator BYTE const*() const
	{
		return m_pBuffer;
	}
	BYTE const* begin() const
	{
		return m_pBuffer;
	}
	BYTE const* end() const
	{
		return m_pBuffer+m_nSize;
	}
	ULONG size() const
	{
		return m_nSize;
	}

private:
	CComPtr<IDataSrcDirect> m_pSrc;
	ULONG m_nSize;
	BYTE const* m_pBuffer;
};


#endif//__cplusplus

