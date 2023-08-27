
#pragma once

class CPIDL
{
public:
	CPIDL() : m_pIDL(NULL) {}
	CPIDL(const ITEMIDLIST* a_pIDL) : m_pIDL(Copy(a_pIDL)) {}
	CPIDL(const CPIDL& a_cOrig) : m_pIDL(Copy(a_cOrig.m_pIDL)) {}
	~CPIDL()
	{
		Clear();
	}

	const CPIDL& operator=(const CPIDL& a_cOrig)
	{
		if (this != &a_cOrig)
		{
			Clear();
			m_pIDL = Copy(a_cOrig.m_pIDL);
		}
		return *this;
	}

	const CPIDL& Attach(ITEMIDLIST* a_pIDL)
	{
		Clear();
		m_pIDL = a_pIDL;
		return *this;
	}

	ITEMIDLIST* Detach()
	{
		ITEMIDLIST* pTmp = m_pIDL;
		m_pIDL = NULL;
		return pTmp;
	}

	operator const ITEMIDLIST*() const
	{
		return m_pIDL;
	}

	ITEMIDLIST** operator&()
	{
		Clear();
		return &m_pIDL;
	}

	bool operator==(const CPIDL& a_cOther) const
	{
		int nSizeThis = GetMemSize();
		int nSizeOther = a_cOther.GetMemSize();
		if (nSizeThis == 0 && nSizeOther == 0)
			return true;
		if (nSizeThis != nSizeOther)
			return false;
		return memcmp(m_pIDL, a_cOther.m_pIDL, nSizeThis) == 0;
	}
	bool operator!=(const CPIDL& a_cOther) const
	{
		return !operator==(a_cOther);
	}

	bool operator<(const CPIDL& a_cOther) const // means IsSubItemOf
	{
		int nSizeThis = GetMemSize();
		int nSizeOther = a_cOther.GetMemSize();
		if (GetIDCount() == 0 && a_cOther.GetMemSize() != 0)
			return true; // everything is greater than empty PIDL
		if (nSizeThis >= nSizeOther)
			return false;
		return memcmp(m_pIDL, a_cOther.m_pIDL, nSizeThis-2) == 0; // -2 ... the terminating item is ignored
	}

	CPIDL operator+(const CPIDL& a_cOther) const
	{
		int nSizeThis = GetMemSize();
		int nSizeOther = a_cOther.GetMemSize();
		if (nSizeThis == 0)
		{
			return a_cOther;
		}
		if (nSizeOther == 0)
			return *this;
		CComPtr<IMalloc> pMalloc;
		SHGetMalloc(&pMalloc);
		ITEMIDLIST* pIDL = reinterpret_cast<ITEMIDLIST*>(pMalloc->Alloc(nSizeThis+nSizeOther-sizeof(pIDL->mkid.cb)));
		CopyMemory(pIDL, m_pIDL, nSizeThis);
		CopyMemory(reinterpret_cast<BYTE*>(pIDL)+nSizeThis-sizeof(pIDL->mkid.cb), a_cOther.m_pIDL, nSizeOther);
		CPIDL cRet;
		cRet.Attach(pIDL);
		return cRet;
	}

	CPIDL GetParent() const
	{
		int nThisSize = GetIDCount();
		CPIDL cRet;
		if (nThisSize > 0)
		{
			cRet.Attach(Copy(m_pIDL, 0, nThisSize-1));
		}
		return cRet;
	}

	CPIDL GetLastItem() const
	{
		int nThisSize = GetIDCount();
		CPIDL cRet;
		if (nThisSize > 0)
		{
			cRet.Attach(Copy(m_pIDL, nThisSize-1, 1));
		}
		return cRet;
	}

	int GetMemSize() const {return GetMemSize(m_pIDL);}
	int GetIDCount() const {return GetIDCount(m_pIDL);}

private:
	static int GetMemSize(const ITEMIDLIST* a_p)
	{
		if (a_p)
		{
			int nLen = 0;
			const SHITEMID* pID = &a_p->mkid;
			do
			{
				nLen += pID->cb;
				pID = reinterpret_cast<const SHITEMID*>(reinterpret_cast<const BYTE*>(pID)+pID->cb);
			} while (pID->cb);
			nLen += sizeof(pID->cb);
			return nLen;
		}
		else
		{
			return 0;
		}
	}

	static int GetIDCount(const ITEMIDLIST* a_p)
	{
		if (a_p)
		{
			int nItems = 0;
			const SHITEMID* pID = &a_p->mkid;
			do
			{
				nItems++;
				pID = reinterpret_cast<const SHITEMID*>(reinterpret_cast<const BYTE*>(pID)+pID->cb);
			} while (pID->cb);
			return nItems;
		}
		else
		{
			return 0;
		}
	}

	static ITEMIDLIST* Copy(const ITEMIDLIST* a_pOrig)
	{
		if (a_pOrig)
		{
			int nLen = GetMemSize(a_pOrig);
			CComPtr<IMalloc> pMalloc;
			SHGetMalloc(&pMalloc);
			void* pCopy = pMalloc->Alloc(nLen);
			CopyMemory(pCopy, a_pOrig, nLen);
			return reinterpret_cast<ITEMIDLIST*>(pCopy);
		}
		else
		{
			return NULL;
		}
	}

	static ITEMIDLIST* Copy(const ITEMIDLIST* a_pOrig, int a_iFrom, int a_iCount)
	{
		if (a_pOrig && a_iCount)
		{
			int nItems = GetIDCount(a_pOrig);
			if ((a_iFrom+a_iCount) > nItems)
				return NULL;

			const SHITEMID* pFrom = NULL;
			int i;
			const SHITEMID* pID = &a_pOrig->mkid;
			for (i = 0; i < (a_iFrom+a_iCount); i++)
			{
				if (i == a_iFrom) pFrom = pID;
				pID = reinterpret_cast<const SHITEMID*>(reinterpret_cast<const BYTE*>(pID)+pID->cb);
			}
			int nLen = reinterpret_cast<const BYTE*>(pID)-reinterpret_cast<const BYTE*>(pFrom)+sizeof(pID->cb);
			CComPtr<IMalloc> pMalloc;
			SHGetMalloc(&pMalloc);
			void* pCopy = pMalloc->Alloc(nLen);
			ZeroMemory(reinterpret_cast<BYTE*>(pCopy)+nLen-sizeof(pID->cb), sizeof(pID->cb));
			CopyMemory(pCopy, pFrom, nLen-sizeof(pID->cb));
			return reinterpret_cast<ITEMIDLIST*>(pCopy);
		}
		else
		{
			return NULL;
		}
	}

	void Clear()
	{
		if (m_pIDL)
		{
			CComPtr<IMalloc> pMalloc;
			SHGetMalloc(&pMalloc);
			pMalloc->Free(m_pIDL);
			m_pIDL = NULL;
		}
	}

private:
	ITEMIDLIST* m_pIDL;
};
