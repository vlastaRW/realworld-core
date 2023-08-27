
#pragma once


class CReturnedData : public IReturnedData
{
public:
	CReturnedData() : m_pData(NULL), m_nSize(NULL), m_nAlloc(NULL)
	{
	}
	~CReturnedData()
	{
		delete[] m_pData;
	}

	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{
		if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, __uuidof(IReturnedData)))
		{
			*ppvObject = static_cast<IReturnedData*>(this);
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, Release)()
	{
		return 0;
	}
	STDMETHOD_(ULONG, AddRef)()
	{
		return 1;
	}
	STDMETHOD(Write)(ULONG a_nSize, BYTE const* a_pData)
	{
		try
		{
			if (m_nSize+a_nSize > m_nAlloc)
			{
				m_nAlloc = m_nAlloc ? max(m_nSize>>1, a_nSize+m_nSize) : a_nSize;
				BYTE* p = new BYTE[m_nAlloc];
				CopyMemory(p, m_pData, m_nSize);
				delete[] m_pData;
				m_pData = p;
			}
			CopyMemory(m_pData+m_nSize, a_pData, a_nSize);
			m_nSize += a_nSize;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	BYTE const* begin() const
	{
		return m_pData;
	}
	BYTE const* end() const
	{
		return m_pData+m_nSize;
	}
	size_t size() const
	{
		return m_nSize;
	}
	BYTE* Detach()
	{
		BYTE* p = m_pData;
		m_pData = NULL;
		m_nAlloc = m_nSize = 0;
		return p;
	}

private:
	BYTE* m_pData;
	size_t m_nSize;
	size_t m_nAlloc;
};

