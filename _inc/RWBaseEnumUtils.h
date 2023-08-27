#pragma once

template<class IInterface>
class CEnumInterfaces
{
public:
	CEnumInterfaces() : m_pEU(NULL)
	{
	}
	CEnumInterfaces(IEnumUnknowns *a_pEU) : m_pEU(a_pEU)
	{
		if (m_pEU) m_pEU->AddRef();
	}
	CEnumInterfaces(CEnumInterfaces& a_cOrig) : m_pEU(a_cOrig.m_pEU)
	{
		if (m_pEU) m_pEU->AddRef();
	}

	~CEnumInterfaces()
	{
		if (m_pEU) m_pEU->Release();
	}

	IEnumUnknowns** operator&()
	{
		ATLASSERT(m_pEU == NULL);
		return &m_pEU;
	}

	HRESULT Size(ULONG *a_pnSize)
	{
		ATLASSERT(m_pEU);
		return m_pEU->Size(a_pnSize);
	}

	HRESULT Get(ULONG a_nIndex, IInterface** a_ppItem)
	{
		ATLASSERT(m_pEU);
		return m_pEU->Get(a_nIndex, __uuidof(IInterface), reinterpret_cast<void**>(a_ppItem));
	}

	HRESULT GetMultiple(ULONG a_nIndexFirst, ULONG a_nCount, IInterface** a_apItems)
	{
		ATLASSERT(m_pEU);
		return m_pEU->Get(a_nIndex, a_nCount, __uuidof(IInterface), reinterpret_cast<void**>(a_apItems));
	}

private:
	IEnumUnknowns *m_pEU;
};

template<typename TStruct>
class CEnumStructs
{
public:
	CEnumStructs() : m_p(NULL)
	{
	}
	~CEnumStructs()
	{
		if (m_p) m_p->Release();
	}

	CEnumStructs const& operator=(IEnumStructs* a_p)
	{
		if (m_p != a_p)
		{
			if (m_p) m_p->Release();
			if (a_p) a_p->AddRef();
			m_p = a_p;
		}
		return *this;
	}
	CEnumStructs const& operator=(CEnumStructs const& a_p)
	{
		if (m_p != a_p.m_p)
		{
			if (m_p) m_p->Release();
			if (a_p.m_p) a_p.m_p->AddRef();
			m_p = a_p.m_p;
		}
		return *this;
	}
	void Attach(IEnumStructs* a_p)
	{
		if (m_p) m_p->Release();
		m_p = a_p;
	}
	IEnumStructs* Detach()
	{
		IEnumStructs* p = m_p;
		m_p = NULL;
		return p;
	}

	IEnumStructs** operator&()
	{
		ATLASSERT(m_p == NULL);
		return &m_p;
	}

	HRESULT Size(ULONG* a_nCount)
	{
		if (m_p == NULL)
			return 0;
		return m_p->Size(sizeof(TStruct), a_nCount);
	}
	HRESULT Get(ULONG a_nIndex, TStruct* a_pItem)
	{
		ATLASSERT(m_p);
		return m_p->Get(a_nIndex, sizeof(TStruct), reinterpret_cast<BYTE*>(a_pItem));
	}
	HRESULT GetMultiple(ULONG a_nIndexFirst, ULONG a_nCount, TStruct* a_aItem)
	{
		ATLASSERT(m_p);
		return m_p->GetMultiple(a_nIndexFirst, a_nCount, sizeof(TStruct), reinterpret_cast<BYTE*>(a_aItem));
	}

private:
	IEnumStructs* m_p;
};

template<typename TUnknown>
struct ATL_NO_VTABLE CStackUnknown : public TUnknown
{
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(TUnknown)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }
};

template<class IEnum2Xxxx, class Xxxx> // IEnumXxxx and Xxxx must match!
class CEnumItemCounter : public CStackUnknown<IEnum2Xxxx>
{
public:
	CEnumItemCounter(ULONG* a_pSize) : m_pSize(a_pSize) { *m_pSize = 0; }

	// IEnum2Xxxx
public:
	STDMETHOD(Range)(ULONG* a_pStart, ULONG* a_pCount)
	{
		if (a_pCount)
		{
			*m_pSize = *a_pCount;
			*a_pCount = 0;
		}
		return S_FALSE;
	}
	STDMETHOD(Consume)(ULONG a_nStart, ULONG a_nCount, Xxxx const* a_aVals)
	{
		if (*m_pSize < a_nStart+a_nCount)
			*m_pSize = a_nStart+a_nCount;
		return S_OK;
	}

private:
	ULONG* m_pSize;
};

template<class IEnum2Xxxx, class MapXxxx>
inline HRESULT EnumMapKeys(MapXxxx const& a_cMap, IEnum2Xxxx* a_pEnum)
{
	ULONG const nMapSize = a_cMap.size();
	ULONG nStart = 0;
	ULONG nCount = nMapSize;
	a_pEnum->Range(&nStart, &nCount);
	if (nCount > 0 && nStart < nMapSize)
	{
		ULONG const nEnd = nMapSize < nStart+nCount ? nMapSize : nStart+nCount;
		MapXxxx::const_iterator i = a_cMap.begin();
		ULONG iPos = 0;
		while (iPos < nStart) { ++i; ++iPos; }
		while (iPos < nEnd)
		{
			HRESULT hRes = a_pEnum->Consume(iPos, 1, &(i->first));
			if (FAILED(hRes))
				return hRes;
			++i;
			++iPos;
		}
	}
	return S_OK;
}

#ifdef _VECTOR_
template<class IEnum2Xxxx, class Xxxx, class StorageXxxx = Xxxx>
class CEnumToVector : public CStackUnknown<IEnum2Xxxx>
{
public:
	CEnumToVector(std::vector<StorageXxxx>& a_cDst) : m_cDst(a_cDst) {}

	operator IEnum2Xxxx*() {return this;}

	// IEnum2Xxxx methods
public:
	STDMETHOD(Range)(ULONG* a_pStart, ULONG* a_pCount)
	{
		if (a_pStart && a_pCount)
			m_cDst.reserve(*a_pStart+*a_pCount);
		return S_OK;
	}
	STDMETHOD(Consume)(ULONG a_nStart, ULONG a_nCount, Xxxx const* a_aVals)
	{
		if (a_nStart+a_nCount > m_cDst.size())
			m_cDst.resize(a_nStart+a_nCount);
		for (; a_nCount > 0; ++a_nStart, ++a_aVals, --a_nCount)
			m_cDst[a_nStart] = *a_aVals;
		return S_OK;
	}

private:
	std::vector<StorageXxxx>& m_cDst;
};
#endif//_VECTOR_

#ifdef _SET_
template<class IEnum2Xxxx, class Xxxx>
class CEnumToSet : public CStackUnknown<IEnum2Xxxx>
{
public:
	CEnumToSet(std::set<Xxxx>& a_cDst) : m_cDst(a_cDst) {}

	// IEnum2Xxxx methods
public:
	STDMETHOD(Range)(ULONG* a_pStart, ULONG* a_pCount)
	{
		return S_OK;
	}
	STDMETHOD(Consume)(ULONG a_nStart, ULONG a_nCount, Xxxx const* a_aVals)
	{
		for (; a_nCount > 0; ++a_aVals, --a_nCount)
			m_cDst.insert(*a_aVals);
		return S_OK;
	}

private:
	std::set<Xxxx>& m_cDst;
};
#endif//_SET_



