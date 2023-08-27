
#pragma once

template<size_t t_nCacheSize>
class CDataStoreOutput
{
public:
	CDataStoreOutput(IReturnedData* a_pDst) :
		m_cCache(new BYTE[t_nCacheSize]), m_nInCache(0)
	{
		m_pDst = a_pDst;
	}
	~CDataStoreOutput()
	{
		if (m_pDst)
		{
			FlushCache();
		}
	}

	void Write(BYTE const* a_pData, size_t a_nSize)
	{
		if ((a_nSize+m_nInCache) <= t_nCacheSize)
		{
			// fits into cache
			memcpy(m_cCache.m_p+m_nInCache, a_pData, a_nSize);
			m_nInCache += a_nSize;
		}
		else if (a_nSize > t_nCacheSize)
		{
			// bigger than cache - write it directly
			FlushCache();
			if (FAILED(m_pDst->Write(static_cast<ULONG>(a_nSize), a_pData)))
				throw E_FAIL; // TODO: error code
		}
		else
		{
			size_t const nFirstPart = t_nCacheSize-m_nInCache;
			memcpy(m_cCache.m_p+m_nInCache, a_pData, nFirstPart);
			m_nInCache = t_nCacheSize;
			FlushCache();
			memcpy(m_cCache.m_p, a_pData+nFirstPart, a_nSize-nFirstPart);
			m_nInCache = a_nSize-nFirstPart;
		}
	}

private:
	void FlushCache()
	{
		if (m_nInCache != 0)
		{
			if (FAILED(m_pDst->Write(static_cast<ULONG>(m_nInCache), m_cCache)))
				throw E_FAIL; // TODO: error code
			m_nInCache = 0;
		}
	}

private:
	CDataStoreOutput(CDataStoreOutput const&);
	CDataStoreOutput const& operator =(CDataStoreOutput const&);

private:
	CComPtr<IReturnedData> m_pDst;
	CAutoVectorPtr<BYTE> m_cCache;
	size_t m_nInCache;
};

