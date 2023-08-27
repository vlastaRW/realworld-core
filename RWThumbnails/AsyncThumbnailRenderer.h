// AsyncThumbnailRenderer.h : Declaration of the CAsyncThumbnailRenderer

#pragma once
#include "resource.h"       // main symbols
#include "RWThumbnails.h"



// CAsyncThumbnailRenderer

class ATL_NO_VTABLE CAsyncThumbnailRenderer :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAsyncThumbnailRenderer, &CLSID_AsyncThumbnailRenderer>,
	public IAsyncThumbnailRenderer
{
public:
	CAsyncThumbnailRenderer() : m_pData(NULL), m_hThread(NULL)
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CAsyncThumbnailRenderer)
	COM_INTERFACE_ENTRY(IAsyncThumbnailRenderer)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		if (m_pData)
		{
			EnterCriticalSection(&m_pData->tLock);
			m_pData->cQueue.clear();
			LeaveCriticalSection(&m_pData->tLock);
			ReleaseSemaphore(m_pData->hSemaphore, 10, NULL);
		}
		if (m_hThread) CloseHandle(m_hThread);
	}

	// IAsyncThumbnailRenderer methods
public:
	STDMETHOD(Init)(IThumbnailRenderer* a_pInternalRenderer);
	STDMETHOD(PrepareThumbnail)(IStorageFilter* a_pFile, LCID a_tInfoLocaleID, IThumbnailCallback* a_pCallback);
	STDMETHOD(CancelRequests)(IThumbnailCallback* a_pCallback);

private:
	struct SItem
	{
		CComPtr<IStorageFilter> pFile;
		LCID tInfoLocaleID;
		CComPtr<IThumbnailCallback> pCallback;
	};
	typedef std::list<SItem> CQueue;
	struct SThreadData
	{
		CQueue cQueue;
		HANDLE hSemaphore;
		CRITICAL_SECTION tLock;
		CComPtr<IThumbnailRenderer> pInternal;
	};

	static unsigned __stdcall WorkerProc(void* a_pRawData);

private:
	unsigned m_uThreadID;
	HANDLE m_hThread;
	SThreadData* m_pData;
};

OBJECT_ENTRY_AUTO(__uuidof(AsyncThumbnailRenderer), CAsyncThumbnailRenderer)
