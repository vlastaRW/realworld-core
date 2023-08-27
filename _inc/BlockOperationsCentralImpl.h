
#pragma once

#include <ObserverImpl.h>


MIDL_INTERFACE("EC77927D-E710-447A-A32C-63F486BA1F28")
IBlockOperationsObserver : public IUnknown
{
public:
	STDMETHOD(Notify)(TCookie a_tCookie, int) = 0;
};

MIDL_INTERFACE("AF9256EB-F9BB-4D24-BC30-46A459132691")
IBlockOperationsRoot : public IBlockOperations
{
public:
	STDMETHOD(ObserverIns)(IBlockOperationsObserver* a_pObserver, TCookie a_tCookie);
	STDMETHOD(ObserverDel)(IBlockOperationsObserver* a_pObserver, TCookie a_tCookie);
};


// this implementation of IBlockOperations uses a shared object and
// centralizes the locking (simplest option for sub-documents)
template<typename T, typename TBase = IBlockOperations>
class CBlockOperationsCentralImpl :
	public TBase,
	public CObserverImpl<CBlockOperationsCentralImpl<T, TBase>, IBlockOperationsObserver, int>
{
public:
	CBlockOperationsCentralImpl()
	{
	}
	~CBlockOperationsCentralImpl()
	{
		if (m_pShared) m_pShared->ObserverDel(ObserverGet(), 0);
	}
protected:
	void Init(IBlockOperationsRoot* a_pShared)
	{
		ATLASSERT(m_pShared == NULL);
		m_pShared = a_pShared;
		m_pShared->ObserverIns(ObserverGet(), 0);
	}

	// IBlockOperations methods
public:
	STDMETHOD(ReadLock)()
	{
		return m_pShared->ReadLock();
	}
	STDMETHOD(ReadUnlock)()
	{
		return m_pShared->ReadUnlock();
	}
	STDMETHOD(WriteLock)()
	{
		return m_pShared->WriteLock();
	}
	STDMETHOD(WriteUnlock)()
	{
		return m_pShared->WriteUnlock();
	}

	void WriteFinished()
	{
		// override in T to customize behavior (send notifications, clear cache, ...)
	}
	void OwnerNotify(TCookie, int)
	{
		static_cast<T*>(this)->WriteFinished();
	}

private:
	CComPtr<IBlockOperationsRoot> m_pShared;
};


template<template<class, class> class TBlockOpsImpl>
class CBlockOperationsRoot :
	public CComObjectRootEx<CComMultiThreadModel>,
	public TBlockOpsImpl<CBlockOperationsRoot<TBlockOpsImpl>, CSubjectImpl<IBlockOperationsRoot, IBlockOperationsObserver, int> >
{
public:
	CBlockOperationsRoot()
	{
	}

BEGIN_COM_MAP(CBlockOperationsRoot<TBlockOpsImpl>)
	COM_INTERFACE_ENTRY(IBlockOperationsRoot)
	COM_INTERFACE_ENTRY(IBlockOperations)
END_COM_MAP()

	void WriteFinished()
	{
		Fire_Notify(0);
	}
};