
#pragma once

#ifdef __cplusplus

} // pause: extern "C"{

#include <map>

// multiple threads can have read locks
// if one thread has write lock, all other threads must not have any locks
// (if a thread has the write lock, it can gain the readlock too and vice versa)
// T::WriteFinished is called during last WriteUnlock
// methods throw exceptions if there are problems with resources
template<typename T, typename TBase = IBlockOperations>
class CBlockOperationsImpl : public TBase
{
public:
	CBlockOperationsImpl() : m_uWriteLockThreadID(0), m_nWriteLocks(0), m_hContinueWrite(0)
	{
		InitializeCriticalSection(&m_tInternCS);
		m_hReadReady = CreateEvent(NULL, TRUE, TRUE, NULL);
		m_hWriteReady = CreateEvent(NULL, TRUE, TRUE, NULL);
		m_hContinueWrite = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hReadReady == NULL || m_hWriteReady == NULL || m_hContinueWrite == NULL)
		{
			DeleteCriticalSection(&m_tInternCS);
			if (m_hReadReady != NULL)
				CloseHandle(m_hReadReady);
			if (m_hWriteReady != NULL)
				CloseHandle(m_hWriteReady);
			if (m_hContinueWrite != NULL)
				CloseHandle(m_hContinueWrite);
			throw E_FAIL;
		}
	}
	~CBlockOperationsImpl()
	{
		DeleteCriticalSection(&m_tInternCS);
		CloseHandle(m_hReadReady);
		CloseHandle(m_hWriteReady);
		CloseHandle(m_hContinueWrite);
	}

	STDMETHOD(ReadLock)()
	{
		DWORD dwThreadID = GetCurrentThreadId();
		do
		{
			CInternLock cLock(this);
			if (m_nWriteLocks == 0 || m_uWriteLockThreadID == dwThreadID || !m_cReadLocks.empty())
			{
				// can be locked right now
				CReadLocks::iterator i = m_cReadLocks.find(dwThreadID);
				if (i != m_cReadLocks.end())
				{
					i->second++;
				}
				else
				{
					m_cReadLocks[dwThreadID] = 1;
					UpdateState();
				}
				return S_OK; // success
			}
		}
		while (WaitForSingleObject(m_hWriteReady, INFINITE) == WAIT_OBJECT_0);
		//while (AtlWaitWithMessageLoop(m_hReadReady));
		return E_FAIL; // wait failed
	}
	STDMETHOD(ReadUnlock)()
	{
		DWORD dwThreadID = GetCurrentThreadId();
		bool bUpdate = false;
		bool bWaitForReadLocks = false;
		{
			CInternLock cLock(this);
			CReadLocks::iterator i = m_cReadLocks.find(dwThreadID);
			if (i == m_cReadLocks.end())
			{
				ATLASSERT(0);
				return E_FAIL; // was not locked
			}
			if (i->second == 1)
			{
				m_cReadLocks.erase(i);
				bUpdate = true;
				if (m_cReadLocks.empty())
				{
					if (m_uWriteLockThreadID && dwThreadID != m_uWriteLockThreadID)
						SetEvent(m_hContinueWrite);
				}
				else
				{
					bWaitForReadLocks = dwThreadID == m_uWriteLockThreadID;
				}
			}
			else
			{
				i->second--;
			}
		}
		if (bWaitForReadLocks)
		{
			WaitForSingleObject(m_hContinueWrite, INFINITE);
			//AtlWaitWithMessageLoop(m_hContinueWrite);
		}
		if (bUpdate)
		{
			UpdateState();
		}
		return S_OK;
	}
	STDMETHOD(WriteLock)()
	{
		DWORD dwThreadID = GetCurrentThreadId();
		do
		{
			CInternLock cLock(this);
			if (m_cReadLocks.empty())
			{
				if (m_uWriteLockThreadID == dwThreadID)
				{
					++m_nWriteLocks;
					return S_OK; // anoher lock for same thread
				}
				if (m_nWriteLocks)
				{
					continue; // 2 threads wanting to write, wait for our turn
				}
				m_nWriteLocks = 1;
				m_uWriteLockThreadID = dwThreadID;
				UpdateState();
				return S_OK; // success
			}
			else if (m_cReadLocks.size() == 1 && m_cReadLocks.begin()->first == dwThreadID)
			{
				ATLASSERT(0);
				return E_FAIL; // WriteLock is not allowed while holding a read lock
			}
		}
		while (WaitForSingleObject(m_hWriteReady, INFINITE) == WAIT_OBJECT_0);
		//while (AtlWaitWithMessageLoop(m_hWriteReady));
		return E_FAIL; // wait failed
	}
	STDMETHOD(WriteUnlock)()
	{
		DWORD dwThreadID = GetCurrentThreadId();
		bool bUpdate = false;
		{
			CInternLock cLock(this);
			if (m_uWriteLockThreadID != dwThreadID)
			{
				ATLASSERT(0);
				return E_FAIL; // was not locked
			}
			m_nWriteLocks--;
			if (m_nWriteLocks == 0)
			{
				m_uWriteLockThreadID = 0;
				bUpdate = true;
				ReadLock();
			}
		}
		if (bUpdate)
		{
			UpdateState();
			static_cast<T*>(this)->WriteFinished();
			ReadUnlock();
		}
		return S_OK;
	}

	void WriteFinished()
	{
		// override in T to customize behavior (send notifications, clear cache, ...)
	}

private:
	typedef std::map<DWORD, size_t> CReadLocks; // ThreadID to number of read-locks
	friend class CInternLock;
	class CInternLock
	{
	public:
		CInternLock(CBlockOperationsImpl<T, TBase>* a_pRoot) : m_tRoot(*a_pRoot)
		{
			EnterCriticalSection(&m_tRoot.m_tInternCS);
		}
		~CInternLock()
		{
			LeaveCriticalSection(&m_tRoot.m_tInternCS);
		}

	private:
		CBlockOperationsImpl<T, TBase>& m_tRoot;
	};

private:
	void UpdateState()
	{
		if (m_uWriteLockThreadID == 0)
		{
			SetEvent(m_hReadReady);
			if (m_cReadLocks.empty())
				SetEvent(m_hWriteReady);
			else
				ResetEvent(m_hWriteReady);
		}
		else
		{
			if (m_cReadLocks.empty())
				ResetEvent(m_hReadReady);
			else
				SetEvent(m_hReadReady);
			ResetEvent(m_hWriteReady);
		}
	}

private:
	CReadLocks m_cReadLocks;
	DWORD m_uWriteLockThreadID;
	size_t m_nWriteLocks;
	CRITICAL_SECTION m_tInternCS;
	HANDLE m_hReadReady;
	HANDLE m_hWriteReady;
	HANDLE m_hContinueWrite;
};

// simplified version of IBlockOperations with no exclusive read access
// checks for use scenario misuse in debug version
template<typename T, typename TBase = IBlockOperations>
class CBlockOperationsSimpleImpl : public TBase
{
public:
	CBlockOperationsSimpleImpl() : m_nWriteLocks(0), m_nReadLocks(0)
	{
		InitializeCriticalSection(&m_tInternCS);
	}
	~CBlockOperationsSimpleImpl()
	{
		DeleteCriticalSection(&m_tInternCS);
		ATLASSERT(m_nWriteLocks == 0 && m_nReadLocks == 0);
	}

	STDMETHOD(ReadLock)()
	{
		EnterCriticalSection(&m_tInternCS);
		m_nReadLocks++;
		return S_OK;
	}
	STDMETHOD(ReadUnlock)()
	{
#ifdef _DEBUG
		if (m_nReadLocks == 0)
			return E_FAIL;
#endif
		m_nReadLocks--;
		LeaveCriticalSection(&m_tInternCS);
		return S_OK;
	}
	STDMETHOD(WriteLock)()
	{
		EnterCriticalSection(&m_tInternCS);
		if (m_nReadLocks != 0)
		{
			LeaveCriticalSection(&m_tInternCS);
			return E_FAIL; // cannot write lock while keeping read lock
		}
		m_nWriteLocks++;
		return S_OK;
	}
	STDMETHOD(WriteUnlock)()
	{
		// if called from other thread, it may corrupt internal state
#ifdef _DEBUG
		if (m_nWriteLocks == 0)
			return E_FAIL;
#endif
		m_nWriteLocks--;
		if (m_nWriteLocks == 0)
		{
			static_cast<T*>(this)->WriteFinished();
		}
		LeaveCriticalSection(&m_tInternCS);
		return S_OK;
	}

	void WriteFinished()
	{
		// override in T to customize behavior (send notifications, clear cache, ...)
	}

private:
	CRITICAL_SECTION m_tInternCS;
	size_t m_nReadLocks;
	size_t m_nWriteLocks;
};

template<typename T>
class CReadLock
{
public:
	CReadLock(T* a_pObject) : m_pObject(a_pObject)
	{
		if (FAILED(m_pObject->ReadLock()))
			throw E_FAIL;
	}
	~CReadLock()
	{
		ATLVERIFY(SUCCEEDED(m_pObject->ReadUnlock()));
	}

private:
	T* const m_pObject;
};

template<typename T>
class CWriteLock
{
public:
	CWriteLock(T* a_pObject) : m_pObject(a_pObject)
	{
		if (FAILED(m_pObject->WriteLock()))
			throw E_FAIL;
	}
	~CWriteLock()
	{
		ATLVERIFY(SUCCEEDED(m_pObject->WriteUnlock()));
	}

private:
	T* const m_pObject;
};

extern "C"{ // continue: extern "C"{

#endif//__cplusplus