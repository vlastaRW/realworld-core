// AsyncThumbnailRenderer.cpp : Implementation of CAsyncThumbnailRenderer

#include "stdafx.h"
#include "AsyncThumbnailRenderer.h"


// CAsyncThumbnailRenderer

STDMETHODIMP CAsyncThumbnailRenderer::Init(IThumbnailRenderer* a_pInternalRenderer)
{
	SThreadData* p = new SThreadData;
	p->pInternal = a_pInternalRenderer;
	p->hSemaphore = CreateSemaphore(NULL, 0, 1024, NULL);
	InitializeCriticalSection(&p->tLock);
	m_pData = p;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerProc, p, 0, &m_uThreadID);
	return S_OK;
}

STDMETHODIMP CAsyncThumbnailRenderer::PrepareThumbnail(IStorageFilter* a_pFile, LCID a_tInfoLocaleID, IThumbnailCallback* a_pCallback)
{
	try
	{
		SItem t;
		t.pFile = a_pFile;
		t.tInfoLocaleID = a_tInfoLocaleID;
		t.pCallback = a_pCallback;
		EnterCriticalSection(&m_pData->tLock);
		m_pData->cQueue.push_back(t);
		LeaveCriticalSection(&m_pData->tLock);
		ReleaseSemaphore(m_pData->hSemaphore, 1, NULL);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CAsyncThumbnailRenderer::CancelRequests(IThumbnailCallback* a_pCallback)
{
	try
	{
		EnterCriticalSection(&m_pData->tLock);
		CQueue::const_iterator i = m_pData->cQueue.begin();
		while (i != m_pData->cQueue.end())
		{
			if (i->pCallback == a_pCallback)
			{
				WaitForSingleObject(m_pData->hSemaphore, 0);
				CQueue::const_iterator j = i;
				++i;
				m_pData->cQueue.erase(j);
			}
			else
			{
				++i;
			}
		}
		LeaveCriticalSection(&m_pData->tLock);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

unsigned CAsyncThumbnailRenderer::WorkerProc(void* a_pRawData)
{
	SThreadData* a_pData = reinterpret_cast<SThreadData*>(a_pRawData);
	try
	{
		while (true)
		{
			if (WaitForSingleObject(a_pData->hSemaphore, INFINITE) == WAIT_OBJECT_0)
			{
				SItem t;
				EnterCriticalSection(&a_pData->tLock);
				if (a_pData->cQueue.empty())
					break;
				t = a_pData->cQueue.front();
				a_pData->cQueue.pop_front();
				LeaveCriticalSection(&a_pData->tLock);
				ULONG nSizeX = 0;
				ULONG nSizeY = 0;
				//a_pData->pInternal->GetNativeSize(t.first, &nSizeX, &nSizeY);
				t.pCallback->AdjustSize(&nSizeX, &nSizeY);
				if (nSizeX == 0 || nSizeY == 0)
					continue;
				CAutoVectorPtr<DWORD> cData;
				RECT rcBounds = {0, 0, 0, 0};
				CComBSTR bstrInfo;
				if (cData.Allocate(nSizeX*nSizeY) && SUCCEEDED(a_pData->pInternal->GetThumbnail(t.pFile, nSizeX, nSizeY, cData, &rcBounds, t.tInfoLocaleID, &bstrInfo)))
					t.pCallback->SetThumbnail(t.pFile, nSizeX, nSizeY, cData, &rcBounds, bstrInfo);
			}
			else
			{
				break;
			}
		}
	}
	catch (...)
	{
	}
	CloseHandle(a_pData->hSemaphore);
	DeleteCriticalSection(&a_pData->tLock);
	delete a_pData;
	return 0;
}
