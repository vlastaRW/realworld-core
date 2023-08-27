// ThumbnailCache.cpp : Implementation of CThumbnailCache

#include "stdafx.h"
#include "ThumbnailCache.h"

#include <StringParsing.h>


// CThumbnailCache

STDMETHODIMP CThumbnailCache::Init(IThumbnailRenderer* a_pInternalRenderer, ULONG a_nMemoryItem, ULONG a_nFilesystemItems, BSTR a_bstrTempPath)
{
	m_pInternalRenderer = a_pInternalRenderer;
	//m_nMemoryItems = a_nMemoryItem;
	m_nFilesystemItems = a_nFilesystemItems;
	m_bstrTempPath = a_bstrTempPath;
	return S_OK;
}

STDMETHODIMP CThumbnailCache::Remove(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY)
{
	try
	{
		CComBSTR bstr;
		a_pFile->ToText(NULL, &bstr);
		if (bstr == NULL)
			return E_RW_INVALIDPARAM;
		ObjectLock cLock(this);
		LockDirectory();
		if (m_hDirectory == INVALID_HANDLE_VALUE)
			return E_FAIL;
		for (CFileSystemItems::const_iterator i = m_cFSItems.begin(); i != m_cFSItems.end(); ++i)
		{
			if (_wcsicmp(bstr, i->bstrLocation) == 0 && a_nSizeX == i->nSizeX && a_nSizeY == i->nSizeY)
			{
				DeleteThumbnail(i->tThumbnailID);
				m_cFSItems.erase(i);
				WriteDirectory();
				UnlockDirectory();
				return S_OK;
			}
		}
		UnlockDirectory();
		return S_FALSE;
	}
	catch (...)
	{
		UnlockDirectory();
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CThumbnailCache::Clear()
{
	return E_NOTIMPL;
}

STDMETHODIMP CThumbnailCache::GetThumbnail(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo)
{
	try
	{
		CComBSTR bstr;
		a_pFile->ToText(NULL, &bstr);
		if (bstr == NULL)
			return E_RW_INVALIDPARAM;

		if (a_pbstrInfo)
			*a_pbstrInfo = NULL;

		RECT rc;
		if (a_prcBounds == NULL)
			a_prcBounds = &rc;

		ObjectLock cLock(this);
		LockDirectory();
		if (m_hDirectory == INVALID_HANDLE_VALUE)
		{
			// maybe someone else is using the file, wait (at most 15 seconds)
			for (int nTries = 30; nTries > 0; --nTries)
			{
				Sleep(500);
				LockDirectory();
				if (m_hDirectory != INVALID_HANDLE_VALUE)
					break;
			}
		}
		if (m_hDirectory == INVALID_HANDLE_VALUE)
		{
			ATLASSERT(m_hDirectory != INVALID_HANDLE_VALUE);
			return m_pInternalRenderer->GetThumbnail(a_pFile, a_nSizeX, a_nSizeY, a_pBGRAData, a_prcBounds, a_tLocaleID, a_pbstrInfo);
		}
		CComQIPtr<IStorageLocatorAttrs> pAttrs(a_pFile);
		for (CFileSystemItems::const_iterator i = m_cFSItems.begin(); i != m_cFSItems.end(); ++i)
		{
			if (_wcsicmp(bstr, i->bstrLocation) == 0 && a_nSizeX == i->nSizeX && a_nSizeY == i->nSizeY)
			{
				if (pAttrs)
				{
					ULONGLONG nTime = 0;
					if (SUCCEEDED(pAttrs->GetTime(ESTTModification, &nTime)) && nTime > i->nTime)
					{
						DeleteThumbnail(i->tThumbnailID);
						m_cFSItems.erase(i);
						break;
					}
				}
				*a_prcBounds = i->rcBounds;
				if (LoadThumbnail(i->nSizeX, i->nSizeY, i->tThumbnailID, a_pBGRAData, a_pbstrInfo))
				{
					UnlockDirectory();
					return S_OK;
				}
				else
				{
					DeleteThumbnail(i->tThumbnailID);
					m_cFSItems.erase(i);
					break;
				}
			}
		}
		CComBSTR bstrInfo;
		HRESULT hRes = m_pInternalRenderer->GetThumbnail(a_pFile, a_nSizeX, a_nSizeY, a_pBGRAData, a_prcBounds, a_tLocaleID, &bstrInfo);
		if (SUCCEEDED(hRes))
		{
			SFileSystemItem sItem;
			sItem.bstrLocation = bstr;
			sItem.nTime = 0;
			if (pAttrs)
				pAttrs->GetTime(ESTTModification, &sItem.nTime);
			sItem.nNativeSizeX = a_prcBounds->right-a_prcBounds->left;
			sItem.nNativeSizeY = a_prcBounds->bottom-a_prcBounds->top;
			sItem.nSizeX = a_nSizeX;
			sItem.nSizeY = a_nSizeY;
			sItem.rcBounds = *a_prcBounds;
			CoCreateGuid(&sItem.tThumbnailID);
			SaveThumbnail(a_nSizeX, a_nSizeY, sItem.tThumbnailID, a_pBGRAData, a_tLocaleID, bstrInfo);
			while (m_cFSItems.size() >= m_nFilesystemItems)
			{
				int i = rand()%m_cFSItems.size();
				DeleteThumbnail(m_cFSItems[i].tThumbnailID);
				m_cFSItems.erase(m_cFSItems.begin()+i);
			}
			m_cFSItems.push_back(sItem);
			WriteDirectory();
		}
		UnlockDirectory();
		if (a_pbstrInfo)
			*a_pbstrInfo = bstrInfo.Detach();
		return hRes;
	}
	catch (...)
	{
		UnlockDirectory();
		return E_UNEXPECTED;
	}
}

void CThumbnailCache::LockDirectory()
{
	OLECHAR szTmp[MAX_PATH];
	wcscpy(szTmp, m_bstrTempPath);
	size_t nLen = wcslen(szTmp);
	if (szTmp[nLen-1] != L'\\')
		szTmp[nLen++] = L'\\';
	wcscpy(szTmp+nLen, L"index");
	m_cFSItems.clear();
	if (m_hDirectory != INVALID_HANDLE_VALUE)
		CloseHandle(m_hDirectory);
	m_hDirectory = CreateFile(CW2T(szTmp), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	DWORD dw = 0;
	if (m_hDirectory == INVALID_HANDLE_VALUE)
	{
		m_hDirectory = CreateFile(CW2T(szTmp), GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hDirectory != INVALID_HANDLE_VALUE)
			WriteFile(m_hDirectory, "RWT1\0\0\0\0", 8, &dw, NULL);
		return;
	}
	char sz[5] = "xxxx";
	ReadFile(m_hDirectory, sz, 4, &dw, NULL);
	if (dw != 4 || strcmp(sz, "RWT1"))
		return;
	DWORD nItems = 0;
	ReadFile(m_hDirectory, &nItems, 4, &dw, NULL);
	m_cFSItems.resize(nItems);
	for (CFileSystemItems::iterator i = m_cFSItems.begin(); i != m_cFSItems.end(); ++i)
	{
		ReadFile(m_hDirectory, &i->tThumbnailID, sizeof i->tThumbnailID, &dw, NULL);
		ReadFile(m_hDirectory, &i->nTime, sizeof i->nTime, &dw, NULL);
		ReadFile(m_hDirectory, &i->nSizeX, sizeof i->nSizeX, &dw, NULL);
		ReadFile(m_hDirectory, &i->nSizeY, sizeof i->nSizeY, &dw, NULL);
		ReadFile(m_hDirectory, &i->nNativeSizeX, sizeof i->nNativeSizeX, &dw, NULL);
		ReadFile(m_hDirectory, &i->nNativeSizeY, sizeof i->nNativeSizeY, &dw, NULL);
		ReadFile(m_hDirectory, &i->rcBounds, sizeof i->rcBounds, &dw, NULL);
		DWORD nLen = 0;
		ReadFile(m_hDirectory, &nLen, 4, &dw, NULL);
		i->bstrLocation.Attach(SysAllocStringLen(NULL, nLen));
		ReadFile(m_hDirectory, i->bstrLocation.m_str, ((nLen+1)&~1)*sizeof(OLECHAR), &dw, NULL);
		i->bstrLocation.m_str[nLen] = L'\0';
	}
}

void CThumbnailCache::UnlockDirectory()
{
	m_cFSItems.clear();
	if (m_hDirectory != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDirectory);
		m_hDirectory = INVALID_HANDLE_VALUE;
	}
}

void CThumbnailCache::WriteDirectory()
{
	if (m_hDirectory == INVALID_HANDLE_VALUE)
	{
		ATLASSERT(0);
		return;
	}
	SetFilePointer(m_hDirectory, 0, NULL, FILE_BEGIN);
	DWORD dw;
	WriteFile(m_hDirectory, "RWT1", 4, &dw, NULL);
	ULONG n = m_cFSItems.size();
	WriteFile(m_hDirectory, &n, 4, &dw, NULL);
	for (CFileSystemItems::const_iterator i = m_cFSItems.begin(); i != m_cFSItems.end(); ++i)
	{
		WriteFile(m_hDirectory, &i->tThumbnailID, sizeof i->tThumbnailID, &dw, NULL);
		WriteFile(m_hDirectory, &i->nTime, sizeof i->nTime, &dw, NULL);
		WriteFile(m_hDirectory, &i->nSizeX, sizeof i->nSizeX, &dw, NULL);
		WriteFile(m_hDirectory, &i->nSizeY, sizeof i->nSizeY, &dw, NULL);
		WriteFile(m_hDirectory, &i->nNativeSizeX, sizeof i->nNativeSizeX, &dw, NULL);
		WriteFile(m_hDirectory, &i->nNativeSizeY, sizeof i->nNativeSizeY, &dw, NULL);
		WriteFile(m_hDirectory, &i->rcBounds, sizeof i->rcBounds, &dw, NULL);
		n = SysStringLen(i->bstrLocation);
		WriteFile(m_hDirectory, &n, 4, &dw, NULL);
		WriteFile(m_hDirectory, i->bstrLocation.m_str, ((n+1)&~1)*sizeof(OLECHAR), &dw, NULL);
	}
}

bool CThumbnailCache::LoadThumbnail(int a_nSizeX, int a_nSizeY, GUID const& a_tID, DWORD* a_pRGBAData, BSTR* a_pbstrInfo)
{
	OLECHAR szTmp[MAX_PATH];
	wcscpy(szTmp, m_bstrTempPath);
	size_t nLen = wcslen(szTmp);
	if (szTmp[nLen-1] != L'\\')
		szTmp[nLen++] = L'\\';
	StringFromGUID(a_tID, szTmp+nLen);
	HANDLE h = CreateFile(CW2T(szTmp), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD dw = 0;
		ReadFile(h, a_pRGBAData, a_nSizeX*a_nSizeY*sizeof*a_pRGBAData, &dw, NULL);
		bool bRet = dw == a_nSizeX*a_nSizeY*sizeof*a_pRGBAData;
		if (a_pbstrInfo && bRet)
		{
			ULONG nLen = GetFileSize(h, NULL);
			if (nLen > a_nSizeX*a_nSizeY*sizeof*a_pRGBAData+4)
			{
				WORD wLang = 0;
				ReadFile(h, &wLang, 2, &dw, NULL);
				WORD wLen = 0;
				ReadFile(h, &wLen, 2, &dw, NULL);
				if (nLen >= a_nSizeX*a_nSizeY*sizeof*a_pRGBAData+4+wLen*2)
				{
					*a_pbstrInfo = SysAllocStringLen(NULL, wLen);
					ReadFile(h, *a_pbstrInfo, wLen*2, &dw, NULL);
				}
			}
		}
		CloseHandle(h);
		return bRet;
	}
	return false;
}

bool CThumbnailCache::SaveThumbnail(int a_nSizeX, int a_nSizeY, GUID const& a_tID, DWORD* a_pRGBAData, LCID a_tLocaleID, BSTR a_bstrInfo)
{
	OLECHAR szTmp[MAX_PATH];
	wcscpy(szTmp, m_bstrTempPath);
	size_t nLen = wcslen(szTmp);
	if (szTmp[nLen-1] != L'\\')
		szTmp[nLen++] = L'\\';
	StringFromGUID(a_tID, szTmp+nLen);
	HANDLE h = CreateFile(CW2T(szTmp), GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE)
		h = CreateFile(CW2T(szTmp), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD dw = 0;
		WriteFile(h, a_pRGBAData, a_nSizeX*a_nSizeY*sizeof*a_pRGBAData, &dw, NULL);
		bool bRet = dw == a_nSizeX*a_nSizeY*sizeof*a_pRGBAData;
		if (a_bstrInfo && a_bstrInfo[0])
		{
			WORD nLang = LANGIDFROMLCID(a_tLocaleID);
			WORD nLen = SysStringLen(a_bstrInfo);
			WriteFile(h, &nLang, 2, &dw, NULL);
			WriteFile(h, &nLen, 2, &dw, NULL);
			WriteFile(h, a_bstrInfo, 2*nLen, &dw, NULL);
		}
		CloseHandle(h);
		return bRet;
	}
	return false;
}

bool CThumbnailCache::DeleteThumbnail(GUID const& a_tID)
{
	OLECHAR szTmp[MAX_PATH];
	wcscpy(szTmp, m_bstrTempPath);
	size_t nLen = wcslen(szTmp);
	if (szTmp[nLen-1] != L'\\')
		szTmp[nLen++] = L'\\';
	StringFromGUID(a_tID, szTmp+nLen);
	return DeleteFile(CW2T(szTmp));
}

