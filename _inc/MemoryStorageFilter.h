
#pragma once


class CMemoryStorageFilter :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStorageFilter,
	public IDataSrcDirect,
	public IDataDstStream
{
public:
	CMemoryStorageFilter() : 
#ifdef _DEBUG
	m_nLocks(0),
#endif
		m_pData(NULL), m_nUsed(0), m_nAlloc(0) {}
	~CMemoryStorageFilter() { delete[] m_pData; }

	void AppendData(BYTE const* a_pData, size_t a_nLength)
	{
		if (m_nAlloc < m_nUsed+a_nLength)
		{
			ULONG nNew = m_nUsed ? m_nUsed<<1 : 1024;
			while (nNew < m_nUsed+a_nLength)
				nNew <<= 1;
			BYTE* pNew = new BYTE[nNew];
			CopyMemory(pNew, m_pData, m_nUsed);
			delete[] m_pData;
			m_pData = pNew;
			m_nAlloc = nNew;
		}
		ATLASSERT(m_nLocks == 0);
		CopyMemory(m_pData+m_nUsed, a_pData, a_nLength);
		m_nUsed += a_nLength;
	}
	ULONG M_Len() const { return m_nUsed; }
	BYTE const* M_Data() const { return m_pData; }
	BYTE* Detach()
	{
		ATLASSERT(m_nLocks == 0);
		BYTE* p = m_pData;
		m_nAlloc = m_nUsed = 0;
		m_pData = NULL;
		return p;
	}

	BEGIN_COM_MAP(CMemoryStorageFilter)
		COM_INTERFACE_ENTRY(IStorageFilter)
		COM_INTERFACE_ENTRY(IDataSrcDirect)
		COM_INTERFACE_ENTRY(IDataDstStream)
	END_COM_MAP()

	// IStorageFilter methods
public:
	STDMETHOD(ToText)(IStorageFilter* a_pRoot, BSTR* a_pbstrFilter)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SubFilterGet)(BSTR a_bstrRelativeLocation, IStorageFilter** a_ppFilter)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SrcOpen)(IDataSrcDirect** a_ppSrc)
	{
		try
		{
			*a_ppSrc = NULL;
			AddRef();
			*a_ppSrc = this;
			return S_OK;
		}
		catch (...)
		{
			return a_ppSrc == NULL ?  E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(DstOpen)(IDataDstStream** a_ppDst)
	{
		try
		{
			*a_ppDst = NULL;
			AddRef();
			*a_ppDst = this;
			return S_OK;
		}
		catch (...)
		{
			return a_ppDst == NULL ?  E_POINTER : E_UNEXPECTED;
		}
	}

	// IDataSrcDirect methods
public:
	STDMETHOD(SizeGet)(ULONG* a_pnSize)
	{
		try
		{
			*a_pnSize = m_nUsed;
			return S_OK;
		}
		catch (...)
		{
			return a_pnSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SrcLock)(ULONG a_nOffset, ULONG a_nSize, BYTE const** a_ppBuffer)
	{
		try
		{
			if ((a_nOffset+a_nSize) > m_nUsed)
				return E_FAIL;
			*a_ppBuffer = m_pData+a_nOffset;
#ifdef _DEBUG
			m_nLocks++;
#endif
			return S_OK;
		}
		catch (...)
		{
			return a_ppBuffer == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SrcUnlock)(ULONG UNREF(a_nSize), BYTE const* UNREF(a_pBuffer))
	{
#ifdef _DEBUG
			m_nLocks--;
#endif
			return S_OK;
	}

	// IDataDstStream methods
public:
	STDMETHOD(Write)(ULONG a_nSize, BYTE const* a_pBuffer)
	{
		try
		{
			AppendData(a_pBuffer, a_nSize);
			return S_OK;
		}
		catch (...)
		{
			return E_OUTOFMEMORY;
		}
	}
	STDMETHOD(Seek)(ULONG a_nSize)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Close)()
	{
		return S_OK;
	}

private:
	BYTE* m_pData;
	ULONG m_nUsed;
	ULONG m_nAlloc;

#ifdef _DEBUG
	ULONG m_nLocks;
#endif
};


