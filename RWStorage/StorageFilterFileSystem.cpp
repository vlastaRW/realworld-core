// StorageFilterFileSystem.cpp : Implementation of CStorageFilterFileSystem

#include "stdafx.h"
#include "StorageFilterFileSystem.h"


// CStorageFilterFileSystem

bool CStorageFilterFileSystem::Init(LPCTSTR a_pszFilter)
{
	try
	{
//		USES_CONVERSION;
		LPTSTR pszTemp = const_cast<LPTSTR>(a_pszFilter);
		if (_tcsnicmp(pszTemp, _T("file://"), 7) == 0)
		{
			pszTemp += 7;
		}
		if (_tcsnicmp(pszTemp, _T("\\\?\\UNC\\"), 8) == 0)
		{
			LPTSTR pszUnc = reinterpret_cast<LPTSTR>(_alloca(_tcslen(pszTemp)-6));
			pszUnc[0] = pszUnc[1] = _T('\\');
			_tcscpy(pszUnc+2, pszTemp);
			pszTemp = pszUnc;
		}
		if (_tcsnicmp(pszTemp, _T("\\\\?\\"), 4) == 0)
		{
			pszTemp += 4;
		}
		int nLen = _tcslen(pszTemp);
		if (nLen > 2 && ((pszTemp[0] >= _T('A') && pszTemp[0] <= _T('Z')) || ((pszTemp[0] >= _T('a') && pszTemp[0] <= _T('z')))) &&
			pszTemp[1] == _T(':') && pszTemp[2] == _T('\\'))
		{
			// classic c:\...
		}
		else if (nLen > 1 && pszTemp[0] == _T('\\') && pszTemp[1] == _T('\\'))
		{
			// UNC \\...
		}
		else
		{
			return false;
		}
		m_pszLocation = _tcsdup(pszTemp);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

void CStorageFilterFileSystem::SrcFinished()
{
	ObjectLock cLock(this);
	m_pSrcDirect = NULL;
}

void CStorageFilterFileSystem::DstFinished()
{
	ObjectLock cLock(this);
	m_bWrite = false;
}

STDMETHODIMP CStorageFilterFileSystem::ToText(IStorageFilter* a_pRoot, BSTR* a_pbstrFilter)
{
	try
	{
		*a_pbstrFilter = NULL;
		if (a_pRoot != NULL)
		{
			CComBSTR bstr;
			a_pRoot->ToText(NULL, &bstr);
			if (bstr == NULL)
			{
				*a_pbstrFilter = CComBSTR(m_pszLocation).Detach();
			}
			else
			{
				TCHAR szFinal[MAX_PATH] = _T("");
				if (PathRelativePathTo(szFinal, CW2CT(bstr), 0, m_pszLocation, 0))
				{
					if (szFinal[0] == _T('.') && szFinal[1] == _T('\\'))
						*a_pbstrFilter = CComBSTR(szFinal+2).Detach();
					else
						*a_pbstrFilter = CComBSTR(szFinal).Detach();
				}
				else
				{
					*a_pbstrFilter = CComBSTR(m_pszLocation).Detach();
				}
			}
		}
		else
		{
			*a_pbstrFilter = CComBSTR(m_pszLocation).Detach();
		}

		return S_OK;
	}
	catch (...)
	{
		return a_pbstrFilter == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFileSystem::SubFilterGet(BSTR a_bstrRelativeLocation, IStorageFilter** a_ppFilter)
{
	try
	{
		*a_ppFilter = NULL;
		if (a_bstrRelativeLocation == NULL)
			return E_RW_INVALIDPARAM;

		TCHAR pszPath[MAX_PATH];
		_tcscpy(pszPath, m_pszLocation);
		int iEnd;
		for(iEnd=0; pszPath[iEnd] != 0; iEnd++);
		for(int i=iEnd-1; i>=0 && pszPath[i]!=_T('\\'); i--) pszPath[i] = 0;
		USES_CONVERSION;
		LPTSTR pszRelativeLocation = OLE2T(a_bstrRelativeLocation);
		if(pszRelativeLocation[1] == _T(':'))
			_tcscpy(pszPath, pszRelativeLocation);
		else
			_tcscat(pszPath, pszRelativeLocation);

		CComObject<CStorageFilterFileSystem>* pSF = NULL;
		CComObject<CStorageFilterFileSystem>::CreateInstance(&pSF);
		CComPtr<IStorageFilter> pTmp = pSF;
		pSF->Init(pszPath);

		*a_ppFilter = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFilter == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFileSystem::SrcOpen(IDataSrcDirect** a_ppSrc)
{
	try
	{
		if (a_ppSrc) *a_ppSrc = NULL;

		ObjectLock cLock(this);

		if (m_bWrite)
		{
			// cannot read while writing
			return E_ACCESSDENIED;
		}
		if (m_pSrcDirect)
		{
			if (a_ppSrc)
			{
				m_pSrcDirect->AddRef();
				*a_ppSrc = m_pSrcDirect;
			}
			return S_OK;
		}
		CComObject<CSrcDirect>::CreateInstance(&m_pSrcDirect);
		m_pSrcDirect->AddRef();
		HRESULT hRes = m_pSrcDirect->Init(this, m_pszLocation);
		if (FAILED(hRes))
		{
			m_pSrcDirect->Release();
			ATLASSERT(m_pSrcDirect == NULL);
			return hRes;
		}

		if (a_ppSrc)
			*a_ppSrc = m_pSrcDirect;
		else
			m_pSrcDirect->Release();

		return S_OK;
	}
	catch (...)
	{
		return a_ppSrc == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFileSystem::DstOpen(IDataDstStream** a_ppDst)
{
	try
	{
		*a_ppDst = NULL;

		ObjectLock cLock(this);

		if (m_bWrite)
		{
			// single write operation allowed
			return E_FAIL; // TODO: error code
		}
		if (m_pSrcDirect != 0)
		{
			// cannot start reading while someone is still reading
			return E_FAIL; // TODO: error code
		}
		CComObject<CDstStream>* pDst = NULL;
		CComObject<CDstStream>::CreateInstance(&pDst);
		CComPtr<IDataDstStream> pTmp = pDst;
		HRESULT hRes = pDst->Init(this, m_pszLocation);
		if (SUCCEEDED(hRes))
			*a_ppDst = pTmp.Detach();

		return hRes;
	}
	catch (...)
	{
		return a_ppDst == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFileSystem::GetTime(EStorageTimeType a_eType, ULONGLONG* a_pTime)
{
	try
	{
		HANDLE h = CreateFile(m_pszLocation, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (h == INVALID_HANDLE_VALUE)
			return E_FAIL;
		BOOL b;
		FILETIME t;
		switch (a_eType)
		{
		case ESTTCreation:		b = GetFileTime(h, &t, NULL, NULL); break;
		case ESTTModification:	b = GetFileTime(h, NULL, NULL, &t); break;
		case ESTTAccess:		b = GetFileTime(h, NULL, &t, NULL); break;
		default:
			CloseHandle(h);
			return E_INVALIDARG;
		}
		CloseHandle(h);
		if (!b)
			return E_FAIL;
		*a_pTime = *reinterpret_cast<ULONGLONG*>(&t);
		return S_OK;
	}
	catch (...)
	{
		return a_pTime ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageFilterFileSystem::SetTime(EStorageTimeType a_eType, ULONGLONG a_nTime)
{
	try
	{
		HANDLE h = CreateFile(m_pszLocation,  FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (h == INVALID_HANDLE_VALUE)
			return E_FAIL;
		BOOL b;
		FILETIME t;
		switch (a_eType)
		{
		case ESTTCreation:		b = SetFileTime(h, reinterpret_cast<FILETIME*>(&a_nTime), NULL, NULL); break;
		case ESTTModification:	b = SetFileTime(h, NULL, NULL, reinterpret_cast<FILETIME*>(&a_nTime)); break;
		case ESTTAccess:		b = SetFileTime(h, NULL, reinterpret_cast<FILETIME*>(&a_nTime), NULL); break;
		default:
			CloseHandle(h);
			return E_INVALIDARG;
		}
		CloseHandle(h);
		if (!b)
			return E_FAIL;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "PIDL.h"

STDMETHODIMP CStorageFilterFileSystem::OpenInFolder()
{
	CComPtr<IShellFolder> pDesktopFolder;
	SHGetDesktopFolder(&pDesktopFolder);
	CPIDL cFolder;
	ULONG nEaten = 0;
	ULONG nAttribs = SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM|SFGAO_FOLDER;
	if (FAILED(pDesktopFolder->ParseDisplayName(NULL, NULL, const_cast<LPOLESTR>(m_pszLocation), &nEaten, &cFolder, &nAttribs)))
		return E_FAIL;

	CPIDL folder(cFolder.GetLastItem());
	LPCITEMIDLIST p = folder;
	HINSTANCE hLib = LoadLibrary(_T("shell32.dll"));
	if (hLib == NULL)
		return E_FAIL;

	typedef HRESULT STDAPICALLTYPE SHOpenFolderAndSelectItemsProc(PCIDLIST_ABSOLUTE, UINT, PCUITEMID_CHILD_ARRAY, DWORD);
	SHOpenFolderAndSelectItemsProc* pfn = (SHOpenFolderAndSelectItemsProc*)GetProcAddress(hLib, "SHOpenFolderAndSelectItems");
	HRESULT hRes = E_FAIL;
	if (pfn)
		hRes = (*pfn)(cFolder.GetParent(), 1, &p, 0);
	FreeLibrary(hLib);
	//ShellExecute(NULL, _T("open"), m_pszTopic, NULL, NULL, SW_SHOW);
	//CreateProcess(L"explorer.exe", m_pszLocation, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
	return hRes;
}

// CStorageFilterFileSystem::CSrcDirect

HRESULT CStorageFilterFileSystem::CSrcDirect::Init(CStorageFilterFileSystem* a_pOwner, LPCTSTR a_pszLocation)
{
	ATLASSERT(m_pOwner == NULL);
	ATLASSERT(a_pOwner != NULL);
	(m_pOwner = a_pOwner)->AddRef();

	HANDLE hFile = CreateFile(a_pszLocation, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return E_FAIL;

	m_nDataSize = GetFileSize(hFile, NULL);

	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	void *pData = hFileMapping == NULL ? NULL : MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	m_hFile = hFile;
	m_hFileMapping = hFileMapping;
	m_pData = reinterpret_cast<BYTE*>(pData);

	return S_OK;
}

STDMETHODIMP CStorageFilterFileSystem::CSrcDirect::SizeGet(ULONG* a_pnSize)
{
	try
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		*a_pnSize = m_nDataSize;
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CStorageFilterFileSystem::CSrcDirect::SrcLock(ULONG a_nOffset, ULONG a_nSize, BYTE const** a_ppBuffer)
{
	try
	{
		if ((a_nOffset + a_nSize) <= m_nDataSize)
		{
			*a_ppBuffer = m_pData+a_nOffset;
			return S_OK;
		}
		else
		{
			*a_ppBuffer = NULL;
			return E_FAIL; // TODO: better return code}
		}
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CStorageFilterFileSystem::CSrcDirect::SrcUnlock(ULONG UNREF(a_nSize), BYTE const* UNREF(a_pBuffer))
{
	// no error cheking now
	return S_OK;
}

// CStorageFilterFileSystem::CDstStream

bool MyCreateDirectory(TCHAR* a_pszPath)
{
	if (CreateDirectory(a_pszPath, NULL))
		return true;

	DWORD dwErr = GetLastError();
	if (dwErr == ERROR_ALREADY_EXISTS)
		return true;

	if (dwErr == ERROR_PATH_NOT_FOUND)
	{
		LPTSTR p = _tcsrchr(a_pszPath, _T('\\'));
		if (p == NULL || (p-a_pszPath) < 2)
			return false;
		*p = _T('\0');
		bool bRet = MyCreateDirectory(a_pszPath);
		*p = _T('\\');
		if (bRet && CreateDirectory(a_pszPath, NULL))
			return true;
	}
	return false;
}

typedef WINGDIAPI BOOL WINAPI fnOpenProcessToken(HANDLE,DWORD,PHANDLE);
typedef WINADVAPI BOOL WINAPI fnGetTokenInformation(HANDLE,TOKEN_INFORMATION_CLASS,LPVOID,DWORD,PDWORD);

HRESULT GetElevationType(TOKEN_ELEVATION_TYPE* ptet)
{
	static TOKEN_ELEVATION_TYPE tTET = TokenElevationTypeDefault;
	static HRESULT hRes = S_FALSE;

	if (hRes == S_FALSE)
	{
		HMODULE hMod = LoadLibrary(_T("Advapi32.dll"));
		fnOpenProcessToken* pfnOpenProcessToken = (fnOpenProcessToken*)GetProcAddress(hMod, "OpenProcessToken");
		fnGetTokenInformation* pfnGetTokenInformation = (fnGetTokenInformation*)GetProcAddress(hMod, "GetTokenInformation");
		hRes = E_FAIL;
		if (pfnOpenProcessToken && pfnGetTokenInformation)
		{
			HANDLE hToken = NULL;
			DWORD dwReturnLength = 0;
			if (pfnOpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken) &&
				pfnGetTokenInformation(hToken, TokenElevationType, &tTET, sizeof tTET, &dwReturnLength) &&
				dwReturnLength == sizeof tTET)
				hRes = S_OK;
			if (hToken)
				::CloseHandle(hToken);
		}
		FreeModule(hMod);
	}

	*ptet = tTET;
	return hRes;
}

#include <XPGUI.h>

HRESULT CStorageFilterFileSystem::CDstStream::Init(CStorageFilterFileSystem* a_pOwner, LPCTSTR a_pszLocation)
{
	ATLASSERT(m_pOwner == NULL);
	ATLASSERT(a_pOwner != NULL);
	(m_pOwner = a_pOwner)->AddRef();

	m_hFile = CreateFile(a_pszLocation, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_PATH_NOT_FOUND)
		{
			// try to create folders
			TCHAR szFolder[MAX_PATH];
			_tcscpy(szFolder, a_pszLocation);
			TCHAR* pBS = _tcsrchr(szFolder, _T('\\'));
			if (pBS)
			{
				*pBS = _T('\0');
				if (MyCreateDirectory(szFolder))
				{
					m_hFile = CreateFile(a_pszLocation, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
					if (m_hFile != INVALID_HANDLE_VALUE)
						return S_OK;
				}
			}
		}
		else if (dwError == ERROR_ACCESS_DENIED)
		{
			TOKEN_ELEVATION_TYPE tTET;
			if (XPGUI::IsVista() && S_OK == GetElevationType(&tTET) && tTET == TokenElevationTypeLimited)
			{
				BIND_OPTS3 tBO3;
				ZeroMemory(&tBO3, sizeof tBO3);
				tBO3.cbStruct = sizeof tBO3;
				tBO3.dwClassContext = CLSCTX_LOCAL_SERVER;
				HRESULT hRes = CoGetObject(L"Elevation:Administrator!new:{3ad05575-8857-4850-9277-11b85bdb8e09}", &tBO3, __uuidof(IFileOperation), reinterpret_cast<void**>(&m_pFO));
				if (m_pFO)
				{
					TCHAR szTemp[MAX_PATH] = _T("");
					GetTempPath(MAX_PATH, szTemp);
					TCHAR szFile[MAX_PATH] = _T("");
					GetTempFileName(szTemp, _T("rwd"), 0, szFile);
					m_hFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
					if (m_hFile != INVALID_HANDLE_VALUE)
					{
						ULONG nTmp = _tcslen(szFile);
						m_pszTemp = new TCHAR[nTmp+1];
						_tcscpy(m_pszTemp, szFile);
						ULONG nLoc = _tcslen(a_pszLocation);
						m_pszTarget = new TCHAR[nLoc+1];
						_tcscpy(m_pszTarget, a_pszLocation);
						return S_OK;
					}
				}
			}
		}
		return HRESULT_FROM_WIN32(dwError);
	}

	return S_OK;
}

STDMETHODIMP CStorageFilterFileSystem::CDstStream::SizeGet(ULONG* a_pnSize)
{
	ATLASSERT(0);
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterFileSystem::CDstStream::Write(ULONG a_nSize, BYTE const* a_pBuffer)
{
	CHECKPOINTER(a_pBuffer);

	if (m_hFile == INVALID_HANDLE_VALUE)
		return E_FAIL;

	DWORD dw = 0;
	WriteFile(m_hFile, a_pBuffer, a_nSize, &dw, NULL);

	return dw == a_nSize ? S_OK : E_FAIL;
}

STDMETHODIMP CStorageFilterFileSystem::CDstStream::Seek(ULONG a_nSize)
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return E_FAIL;

	SetFilePointer(m_hFile, a_nSize, NULL, FILE_BEGIN);

	return S_OK;
}

typedef HRESULT STDAPICALLTYPE fnSHCreateItemFromParsingName(PCWSTR,IBindCtx*,REFIID,void**);

STDMETHODIMP CStorageFilterFileSystem::CDstStream::Close()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		if (m_pFO && m_pszTarget && m_pszTemp)
		{
			LPTSTR pszNewName = _tcsrchr(m_pszTarget, _T('\\'));
			LPTSTR pszNewName2 = _tcsrchr(m_pszTarget, _T('/'));
			if (pszNewName < pszNewName2) pszNewName = pszNewName2;
			if (pszNewName)
			{
				*pszNewName = _T('\0');
				++pszNewName;
			}
			if (FAILED(m_pFO->SetOperationFlags(FOF_NO_UI)))
				return E_FAIL;
			HMODULE hMod = LoadLibrary(_T("Shell32.dll"));
			fnSHCreateItemFromParsingName* pfnSHCreateItemFromParsingName = (fnSHCreateItemFromParsingName*)GetProcAddress(hMod, "SHCreateItemFromParsingName");
			CComPtr<IShellItem> psiFrom;
			CComPtr<IShellItem> psiTo;
			if (pfnSHCreateItemFromParsingName == NULL ||
				FAILED(pfnSHCreateItemFromParsingName(m_pszTemp, NULL, IID_PPV_ARGS(&psiFrom))) ||
				FAILED(pfnSHCreateItemFromParsingName(m_pszTarget, NULL, IID_PPV_ARGS(&psiTo))))
			{
				FreeModule(hMod);
				return E_FAIL;
			}
			FreeModule(hMod);
			if (FAILED(m_pFO->CopyItem(psiFrom, psiTo, pszNewName, NULL)) || FAILED(m_pFO->PerformOperations()))
			{
				DeleteFile(m_pszTemp);
				return E_FAIL;
			}
			DeleteFile(m_pszTemp);
		}
	}

	return S_OK;
}

