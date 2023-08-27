
#include "stdafx.h"
#include "RWBase.h"
#include <WeakSingleton.h>


class ATL_NO_VTABLE CThreadPool :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CThreadPool, &CLSID_ThreadPool>,
	public IThreadPoolControl
{
public:
	CThreadPool() : m_nProcessors(0), m_bRunning(false), m_nMaxThreads(0)
	{
		Init();
	}
	~CThreadPool()
	{
		End();
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CThreadPool)


BEGIN_COM_MAP(CThreadPool)
	COM_INTERFACE_ENTRY(IThreadPool)
	COM_INTERFACE_ENTRY(IThreadPoolControl)
END_COM_MAP()


	// IThreadPool methods
public:
	STDMETHOD_(ULONG, MaxThreads)() { return m_nMaxThreads ? min(m_nMaxThreads, m_nProcessors) : m_nProcessors; }
	STDMETHOD(Execute)(ULONG a_nMaxThreads, IThreadedTask* a_pTask)
	{
		try
		{
			if ((m_nMaxThreads && m_nMaxThreads < a_nMaxThreads) || a_nMaxThreads == 0) a_nMaxThreads = m_nMaxThreads;
			ULONG const nActual = a_nMaxThreads ? min(a_nMaxThreads, m_nProcessors) : m_nProcessors;
			if (nActual < 2)
				return a_pTask->Execute(0, 1);

			bool bRunSingle = false;
			HRESULT hRes = E_FAIL;
			{
				ObjectLock lock(this);
				if (!m_bRunning)
				{
					m_bRunning = true;
					for (ULONG i = 0; i < nActual-1; ++i)
					{
						m_aThreads[i].total = nActual;
						ResetEvent(m_aThreads[i].hFinished);
						m_aThreads[i].pTask = a_pTask;
						SetEvent(m_aThreads[i].hStart);
					}
					hRes = a_pTask->Execute(nActual-1, nActual);
					WaitForMultipleObjects(nActual-1, m_aFinished, TRUE, INFINITE);
					for (ULONG i = 0; i < nActual-1; ++i)
						if (FAILED(m_aThreads[i].hRes))
							hRes = m_aThreads[i].hRes;
					m_bRunning = false;
				}
				else
				{
					bRunSingle = true;
				}
			}
			if (bRunSingle)
				return a_pTask->Execute(0, 1);
			return hRes;
		}
		catch (...)
		{
			m_bRunning = false;
			return E_UNEXPECTED;
		}
	}

	// IThreadPoolControl methods
public:
	STDMETHOD(MaxThreadsSet)(ULONG a_nMaxThreads)
	{
		m_nMaxThreads = a_nMaxThreads;
		return S_OK;
	}
	STDMETHOD(ProcessorInfoGet)(ULONG* a_pCores, ULONG* a_pThreads)
	{
		if (a_pCores)
			*a_pCores = m_nProcessors;
		if (a_pThreads)
			*a_pThreads = m_nThreads;
		return S_OK;
	}

private:
	struct SThreadInfo
	{
		SThreadInfo() : hThread(0), idThread(0), hStart(0), hFinished(0) {}
		HANDLE hThread;
		unsigned idThread;
		HANDLE hStart;
		HANDLE hFinished;
		IThreadedTask* pTask;
		ULONG index;
		ULONG total;
		HRESULT hRes;
	};

private:
	static unsigned __stdcall ThreadPoolProc(void* a_p)
	{
		SThreadInfo* const pInfo = reinterpret_cast<SThreadInfo*>(a_p);
		try
		{
			while (true)
			{
				WaitForSingleObject(pInfo->hStart, INFINITE);
				if (pInfo->pTask == NULL)
				{
					SetEvent(pInfo->hFinished);
					return 0;
				}
				pInfo->hRes = pInfo->pTask->Execute(pInfo->index, pInfo->total);
				SetEvent(pInfo->hFinished);
			}
		}
		catch (...)
		{
			ATLASSERT(FALSE);
		}
		return 0;
	}

	void Init()
	{
		m_nProcessors = GetProcessorCount(&m_nThreads);
		if (m_nProcessors < 2)
		{
			m_nProcessors = 1;
			return;
		}
		m_aThreads.resize(m_nProcessors-1);
		m_aFinished.Allocate(m_nProcessors-1);
		for (ULONG i = 0; i < m_nProcessors-1; ++i)
		{
			SThreadInfo& sti = m_aThreads[i];
			sti.index = i;
			m_aFinished[i] = sti.hFinished = CreateEvent(NULL, TRUE, FALSE, NULL);
			sti.hStart = CreateEvent(NULL, FALSE, FALSE, NULL);
			sti.hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadPoolProc, &sti, 0, &sti.idThread);
		}
	}
	void End()
	{
		for (ULONG i = 0; i < m_nProcessors-1; ++i)
		{
			SThreadInfo& sti = m_aThreads[i];
			CloseHandle(sti.hFinished);
			CloseHandle(sti.hStart);
			CloseHandle(sti.hThread);
		}
	}

	typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

	static DWORD CountSetBits(ULONG_PTR bitMask)
	{
		DWORD bitSetCount = 0;
		while (bitMask)
		{
			bitSetCount += (bitMask&1);
			bitMask >>= 1;
		}
		return bitSetCount;
	}

	static ULONG GetProcessorCount(ULONG* a_pThreads)
	{
		LPFN_GLPI glpi = reinterpret_cast<LPFN_GLPI>(GetProcAddress(GetModuleHandle(_T("kernel32")), "GetLogicalProcessorInformation"));
		if (NULL != glpi)
		{
			DWORD needed = 0;
			CAutoVectorPtr<BYTE> buffer;
			//PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
			DWORD rc = glpi(reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(buffer.m_p), &needed);
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				buffer.Allocate(needed);
				rc = glpi(reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(buffer.m_p), &needed);
			}
			if (rc)
			{
				DWORD logicalProcessorCount = 0;
				DWORD numaNodeCount = 0;
				DWORD processorCoreCount = 0;
				//DWORD processorL1CacheCount = 0;
				//DWORD processorL2CacheCount = 0;
				//DWORD processorL3CacheCount = 0;
				DWORD processorPackageCount = 0;

				int count = needed/sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
				PSYSTEM_LOGICAL_PROCESSOR_INFORMATION p = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(buffer.m_p);
				for (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION e = p+count; p != e; ++p)
				{
					switch (p->Relationship) 
					{
					case RelationNumaNode: ++numaNodeCount; break;
					case RelationProcessorCore: ++processorCoreCount; logicalProcessorCount += CountSetBits(p->ProcessorMask); break;
					//case RelationCache:
					//{
					//	PCACHE_DESCRIPTOR pCache = &p->Cache;
					//	if (pCache->Level == 1)
					//		++processorL1CacheCount;
					//	else if (pCache->Level == 2)
					//		++processorL2CacheCount;
					//	else if (pCache->Level == 3)
					//		++processorL3CacheCount;
					//	break;
					//}
					case RelationProcessorPackage: ++processorPackageCount; break;
					}
				}
				if (processorCoreCount > 0)
				{
					if (a_pThreads) *a_pThreads = logicalProcessorCount;
					return processorCoreCount;
				}
			}
		}
		DWORD_PTR dwProcess = 0, dwSystem = 0;
		GetProcessAffinityMask(GetCurrentProcess(), &dwProcess, &dwSystem);
		ULONG nProc = CountSetBits(dwProcess);
		if (a_pThreads) *a_pThreads = nProc;
		return nProc;
	}

private:
	ULONG m_nProcessors;
	ULONG m_nThreads;
	std::vector<SThreadInfo> m_aThreads;
	CAutoVectorPtr<HANDLE> m_aFinished;
	bool m_bRunning;
	ULONG m_nMaxThreads;
};

OBJECT_ENTRY_AUTO(CLSID_ThreadPool, CThreadPool)

