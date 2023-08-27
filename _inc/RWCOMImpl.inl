
#include "stdafx.h"
#include <vector>
#include <map>


typedef HRESULT (__stdcall fnAddClassCallback) (void* a_pContext, REFCLSID a_tClsID, ULONG a_nCategories, CATID const* a_aCategoryIDs);
typedef HRESULT (__stdcall fnDllEnumClasses) (void* a_pContext, fnAddClassCallback* a_pfnAddClass);
typedef HRESULT (__stdcall fnDllGetClassObject) (REFCLSID rclsid, REFIID riid, LPVOID* ppv);
typedef HRESULT (__stdcall fnDllCanUnloadNow) ();

struct SModule { HMODULE hModule; LPTSTR pszName; ULONG nVer1, nVer2; };
typedef std::vector<SModule> CModules; // list of modules with class factories
struct lessCLSID { bool operator()(CLSID const& a_1, CLSID const& a_2) const {return memcmp(&a_1, &a_2, sizeof a_1)<0;} };
typedef std::map<CLSID, IClassFactory*, lessCLSID> CClasses; // CLSID->module index
struct SCategory { SCategory() : nStamp(1) {} ULONG nStamp; std::vector<CLSID> aIDs; };
typedef std::map<CATID, SCategory, lessCLSID> CCategories; // classes in category

static bool s_bInitialized = false;
static bool s_bSyncCreation = false;
CRITICAL_SECTION s_tClassList;
CModules s_aModules;
CClasses s_aClasses;
CCategories s_aCategories;

void LoadPlugIns(LPCTSTR a_pszFilename);
void InitializePlugIns(LPCTSTR* a_apszSearchMasks);

//void InitializePlugIns()
//{
//	TCHAR szMax
//}

void ReleasePlugIns()
{
	s_bInitialized = false;
	s_aCategories.clear();
	for (CClasses::iterator i = s_aClasses.begin(); i != s_aClasses.end(); ++i)
	{
		if (i->second)
		{
			i->second->Release();
			i->second = NULL;
		}
	}
	s_aClasses.clear();
	for (CModules::iterator i = s_aModules.begin(); i != s_aModules.end(); ++i)
	{
		delete[] i->pszName;
		if (i->hModule)
			FreeLibrary(i->hModule);
	}
	s_aModules.clear();
	if (s_bSyncCreation)
		DeleteCriticalSection(&s_tClassList);
}

void ProcessPlugInUpdates(LPCTSTR a_pszFolder, LPCTSTR a_pszInstall, bool a_bDeleteAll)
{
	// If Updates contains a file that is in the installation folder,
	// attempt to overwrite the installed file.
	// If the attempt fails, move it to the plug-in folder.

	TCHAR szUpdatesPath[MAX_PATH];
	_tcscpy(szUpdatesPath, a_pszFolder);
	_tcscat(szUpdatesPath, _T("\\Updates"));

	TCHAR szTargetPath[MAX_PATH];
	_tcscpy(szTargetPath, a_pszFolder);
	TCHAR* pTargetName = szTargetPath+_tcslen(szTargetPath);
	*(pTargetName++) = _T('\\');

	WIN32_FIND_DATA w32fd;
	HANDLE hFindData;

	DWORD dw = GetFileAttributes(szUpdatesPath);
	if (dw != INVALID_FILE_ATTRIBUTES && (dw&FILE_ATTRIBUTE_DIRECTORY))
	{
		TCHAR* pUpdatesName = szUpdatesPath+_tcslen(szUpdatesPath);
		_tcscpy(pUpdatesName++, _T("\\*.*"));

		TCHAR szInstallPath[MAX_PATH];
		_tcscpy(szInstallPath, a_pszInstall);
		TCHAR* pInstallName = szInstallPath+_tcslen(szInstallPath);
		*(pInstallName++) = _T('\\');

		if ((hFindData = FindFirstFile(szUpdatesPath, &w32fd)) != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (_tcsicmp(w32fd.cFileName, _T(".")) == 0 || _tcsicmp(w32fd.cFileName, _T("..")) == 0)
					continue;

				// get full name of the found dll
				_tcscpy(pTargetName, w32fd.cFileName);
				_tcscpy(pUpdatesName, w32fd.cFileName);
				_tcscpy(pInstallName, w32fd.cFileName);

				HANDLE hFile = CreateFile(szUpdatesPath, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					ULONG nSize = GetFileSize(hFile, NULL);
					CloseHandle(hFile);
					if (nSize == 0 || a_bDeleteAll)
					{
						DeleteFile(szTargetPath);
						DeleteFile(szUpdatesPath);
					}
					else
					{
						// try to move the file to the installation folder
						if (GetFileAttributes(szInstallPath) == 0xffffffff ||
							!DeleteFile(szInstallPath) ||
							!MoveFile(szUpdatesPath, szInstallPath))
						{
							// failed -> use the default folder
							DeleteFile(szTargetPath);
							MoveFile(szUpdatesPath, szTargetPath);
						}
					}
				}
			} while (FindNextFile(hFindData, &w32fd));
			FindClose(hFindData);
		}
		pUpdatesName[-1] = _T('\0');
		RemoveDirectory(szUpdatesPath);
	}
	_tcscpy(pTargetName, _T("*.dll"));
	if (a_bDeleteAll && (hFindData = FindFirstFile(szTargetPath, &w32fd)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (_tcsicmp(w32fd.cFileName, _T(".")) == 0 || _tcsicmp(w32fd.cFileName, _T("..")) == 0)
				continue;

			_tcscpy(pTargetName, w32fd.cFileName);
			DeleteFile(szTargetPath);
		}
		while (FindNextFile(hFindData, &w32fd));
		FindClose(hFindData);
	}
}

struct SCandidate
{
	LPTSTR pszFullPath;
	LPCTSTR pszFileName;
};
void InitializePlugIns(LPCTSTR* a_apszSearchMasks)
{
	std::vector<SCandidate> aCandidates;
	for (; *a_apszSearchMasks; ++a_apszSearchMasks)
	{
		WIN32_FIND_DATA w32fd;
		HANDLE hFindData;

		TCHAR szCurrPath[MAX_PATH];
		_tcscpy(szCurrPath, *a_apszSearchMasks);
		LPTSTR pPathEnd = _tcsrchr(szCurrPath, _T('\\'));
		if (pPathEnd == NULL)
			continue;
		++pPathEnd;

		size_t const nCmpSize = aCandidates.size();
		if ((hFindData = FindFirstFile(*a_apszSearchMasks, &w32fd)) != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (_tcsicmp(w32fd.cFileName, _T(".")) == 0 || _tcsicmp(w32fd.cFileName, _T("..")) == 0 || _tcsicmp(w32fd.cFileName, _T("RWCRT.dll")) == 0 || _tcsicmp(w32fd.cFileName, _T("RWSTL.dll")) == 0)
					continue;

				// get full name of the found dll
				size_t nLen = _tcslen(w32fd.cFileName)+(pPathEnd-szCurrPath);
				LPTSTR pszFull = new TCHAR[nLen+1];
				memcpy(pszFull, szCurrPath, (pPathEnd-szCurrPath)*sizeof*pszFull);
				LPTSTR pszName = pszFull+(pPathEnd-szCurrPath);
				_tcscpy(pszName, w32fd.cFileName);

				std::vector<SCandidate>::iterator const iLast = aCandidates.begin()+nCmpSize;
				std::vector<SCandidate>::iterator i = aCandidates.begin();
				for (; i != iLast; ++i)
					if (_tcsicmp(i->pszFileName, pszName) == 0)
					{
						delete[] i->pszFullPath;
						i->pszFileName = pszName;
						i->pszFullPath = pszFull;
						break;
					}
				if (i == iLast)
				{
					SCandidate s = {pszFull, pszName};
					aCandidates.push_back(s);
				}

			} while (FindNextFile(hFindData, &w32fd));
			FindClose(hFindData);
		}
	}
	for (std::vector<SCandidate>::iterator i = aCandidates.begin(); i != aCandidates.end(); ++i)
	{
		LoadPlugIns(i->pszFullPath);
		delete[] i->pszFullPath;
	}
	s_bInitialized = true;
}

//struct SEnumContext
//{
//	CModules aModules;
//	CClasses aClasses;
//	CCategories aCategories;
//};

HRESULT __stdcall AddClass(void* a_pContext, REFCLSID a_tClsID, ULONG a_nCategories, CATID const* a_aCategoryIDs)
{
	fnDllGetClassObject* pfnGetCO = (fnDllGetClassObject*)a_pContext;
	try
	{
		CComPtr<IClassFactory> pCF;
		(*pfnGetCO)(a_tClsID, __uuidof(IClassFactory), reinterpret_cast<void**>(&pCF));
		if (pCF)
		{
			CClasses::iterator iC = s_aClasses.find(a_tClsID);
			if (iC == s_aClasses.end())
			{
				s_aClasses[a_tClsID] = pCF;
				pCF.Detach();
			}
			else
			{
				// same class in another dll - override the previous one
				// (this allows patches to be placed in user plug-in location)
				iC->second->Release();
				iC->second = pCF.Detach();
				for (CCategories::iterator iT = s_aCategories.begin(); iT != s_aCategories.end(); ++iT)
				{
					for (std::vector<CLSID>::iterator iL = iT->second.aIDs.begin(); iL != iT->second.aIDs.end(); ++iL)
						if (IsEqualCLSID(*iL, a_tClsID))
						{
							iT->second.aIDs.erase(iL);
							++iT->second.nStamp;
							break;
						}
				}
			}
			for (ULONG i = 0; i < a_nCategories; ++i)
			{

				SCategory& sCat = s_aCategories[a_aCategoryIDs[i]];
				sCat.aIDs.push_back(a_tClsID);
				++sCat.nStamp;
			}
		}
	}
	catch (...)
	{
		ATLASSERT(FALSE); // exception in plug-in enumeration, continue...
	}

	return S_OK;
}

void LoadPlugIns(LPCTSTR a_pszFilename)
{
	HMODULE hMod = LoadLibraryEx(a_pszFilename, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (hMod)
	{
		fnDllEnumClasses* pfnEnum = (fnDllEnumClasses*)GetProcAddress(hMod, "DllEnumClasses");
		if (pfnEnum == NULL)
		{
			FreeLibrary(hMod);
			return;
		}
		fnDllGetClassObject* pfnGetCO = (fnDllGetClassObject*)GetProcAddress(hMod, "DllGetClassObject");
		if (pfnGetCO == NULL)
		{
			FreeLibrary(hMod);
			return;
		}
		try
		{
			SModule sMod;
			sMod.hModule = hMod;
			sMod.nVer1 = sMod.nVer2 = 0;
			HRSRC hVer = FindResource(hMod, MAKEINTRESOURCE(1), RT_VERSION);
			if (hVer)
			{
				HGLOBAL hGlob = LoadResource(hMod, hVer);
				if (hGlob)
				{
					WORD* pRawVer = reinterpret_cast<WORD*>(LockResource(hGlob));
					VS_FIXEDFILEINFO* pVer = reinterpret_cast<VS_FIXEDFILEINFO*>(pRawVer+20);
					if (pRawVer && pRawVer[3] == L'V')
					{
						sMod.nVer1 = pVer->dwFileVersionMS;
						sMod.nVer2 = pVer->dwFileVersionLS;
					}
				}
			}
			LPCTSTR pPathEnd = _tcsrchr(a_pszFilename, _T('\\'));
			if (pPathEnd) ++pPathEnd; else pPathEnd = a_pszFilename;
			size_t const nLen = _tcslen(pPathEnd);
			sMod.pszName = new TCHAR[nLen+1];
			_tcscpy(sMod.pszName, pPathEnd);
			s_aModules.push_back(sMod);
			(*pfnEnum)(pfnGetCO, &AddClass);
		}
		catch (...)
		{
		}
	}
}

HRESULT __stdcall RemoveClass(void* a_pContext, REFCLSID a_tClsID, ULONG a_nCategories, CATID const* a_aCategoryIDs)
{
	try
	{
		CClasses::iterator iC = s_aClasses.find(a_tClsID);
		if (iC == s_aClasses.end())
			return S_OK;
		iC->second->Release();
		s_aClasses.erase(iC);
		CComPtr<IClassFactory> pCF;
		for (ULONG i = 0; i < a_nCategories; ++i)
		{

			SCategory& sCat = s_aCategories[a_aCategoryIDs[i]];
			for (std::vector<CLSID>::iterator j = sCat.aIDs.begin(); j != sCat.aIDs.end(); ++j)
				if (IsEqualGUID(*j, a_tClsID))
				{
					sCat.aIDs.erase(j);
					++sCat.nStamp;
					if (sCat.aIDs.empty())
						s_aCategories.erase(a_aCategoryIDs[i]);
					break;
				}
		}
	}
	catch (...)
	{
		ATLASSERT(FALSE); // exception in plug-in enumeration, continue...
	}

	return S_OK;
}

void UnloadPlugIns(LPCTSTR a_pszFilename)
{
	if (a_pszFilename == NULL) return;
	LPCTSTR pPathEnd = _tcsrchr(a_pszFilename, _T('\\'));
	if (pPathEnd) ++pPathEnd; else pPathEnd = a_pszFilename;
	for (CModules::iterator i = s_aModules.begin(); i != s_aModules.end(); ++i)
	{
		if (_tcsicmp(pPathEnd, i->pszName) == 0)
		{
			fnDllEnumClasses* pfnEnum = (fnDllEnumClasses*)GetProcAddress(i->hModule, "DllEnumClasses");
			(*pfnEnum)(NULL, &RemoveClass);
			fnDllCanUnloadNow* pfnCanUnload = (fnDllCanUnloadNow*)GetProcAddress(i->hModule, "DllCanUnloadNow");
			if (*pfnCanUnload && (*pfnCanUnload)() == S_OK)
			{
				FreeModule(i->hModule);
				s_aModules.erase(i);
				return;
			}
		}
	}
}

extern "C" __declspec(dllexport) HRESULT __stdcall RWCoCreateInstance(REFCLSID a_tClass, LPUNKNOWN a_pUnkOuter, DWORD a_dwClsContext, REFIID a_tInterface, void** a_ppInstance)
{
	ATLASSERT(s_bInitialized);

	if (!s_bSyncCreation)
	{
		CClasses::const_iterator iC = s_aClasses.find(a_tClass);
		if (iC != s_aClasses.end())
			return iC->second->CreateInstance(a_pUnkOuter, a_tInterface, a_ppInstance);
	}
	else
	{
		EnterCriticalSection(&s_tClassList);
		CClasses::const_iterator iC = s_aClasses.find(a_tClass);
		if (iC != s_aClasses.end())
		{
			CComPtr<IClassFactory> pCF(iC->second);
			LeaveCriticalSection(&s_tClassList);
			return pCF->CreateInstance(a_pUnkOuter, a_tInterface, a_ppInstance);
		}
		LeaveCriticalSection(&s_tClassList);
	}

#ifdef _DEBUG
	wchar_t sz[64];
	swprintf(sz, L"Cannot create class: %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", a_tClass.Data1, a_tClass.Data2, a_tClass.Data3, a_tClass.Data4[0], a_tClass.Data4[1], a_tClass.Data4[2], a_tClass.Data4[3], a_tClass.Data4[4], a_tClass.Data4[5], a_tClass.Data4[6], a_tClass.Data4[7]);
	ATLTRACE(sz);
#endif
	// fallback to ordinary COM
	return REGDB_E_CLASSNOTREG;//CoCreateInstance(a_tClass, a_pUnkOuter, a_dwClsContext, a_tInterface, a_ppInstance); 
}

typedef void (__stdcall fnEnumCategoryCLSIDsCallback) (void* a_pContext, ULONG a_nClasses, CLSID const* a_aClasses);
extern "C" __declspec(dllexport) void __stdcall RWEnumCategoryCLSIDs(REFCATID a_tCatID, void* a_pContext, fnEnumCategoryCLSIDsCallback* a_pfnCallback)
{
	if (!s_bSyncCreation)
	{
		CCategories::const_iterator i = s_aCategories.find(a_tCatID);
		if (i != s_aCategories.end())
			(*a_pfnCallback)(a_pContext, i->second.aIDs.size(), &(i->second.aIDs[0])); // vector hack
	}
	else
	{
		EnterCriticalSection(&s_tClassList);
		try
		{
			CCategories::const_iterator i = s_aCategories.find(a_tCatID);
			if (i != s_aCategories.end())
				(*a_pfnCallback)(a_pContext, i->second.aIDs.size(), &(i->second.aIDs[0])); // vector hack
		}
		catch (...)
		{
		}
		LeaveCriticalSection(&s_tClassList);
	}
}

extern "C" __declspec(dllexport) ULONG __stdcall RWGetCategoryTimestamp(REFCATID a_tCatID)
{
	if (!s_bSyncCreation)
	{
		CCategories::const_iterator i = s_aCategories.find(a_tCatID);
		return i != s_aCategories.end() ? i->second.nStamp : 0;
	}
	else
	{
		ULONG ret = 0;
		EnterCriticalSection(&s_tClassList);
		try
		{
			CCategories::const_iterator i = s_aCategories.find(a_tCatID);
			ret = i != s_aCategories.end() ? i->second.nStamp : 0;
		}
		catch (...)
		{
		}
		LeaveCriticalSection(&s_tClassList);
		return ret;
	}
}

extern "C" __declspec(dllexport) void __stdcall RWLoadPlugIns(LPCWSTR a_pszFilename)
{
	if (!s_bSyncCreation)
	{
		InitializeCriticalSection(&s_tClassList);
		s_bSyncCreation = true;
	}
	EnterCriticalSection(&s_tClassList);
	try { LoadPlugIns(a_pszFilename); } catch (...) { }
	LeaveCriticalSection(&s_tClassList);
}

extern "C" __declspec(dllexport) void __stdcall RWUnloadPlugIns(LPCWSTR a_pszFilename)
{
	if (!s_bSyncCreation)
	{
		InitializeCriticalSection(&s_tClassList);
		s_bSyncCreation = true;
	}
	EnterCriticalSection(&s_tClassList);
	try { UnloadPlugIns(a_pszFilename); } catch (...) { }
	LeaveCriticalSection(&s_tClassList);
}

typedef void (__stdcall fnEnumModuleCallback) (void* a_pContext, LPCWSTR a_pszName, ULONG a_nVer1, ULONG a_nVer2);
extern "C" __declspec(dllexport) void __stdcall RWEnumModules(void* a_pContext, fnEnumModuleCallback* a_pfnCallback)
{
	if (!s_bSyncCreation)
	{
		for (CModules::const_iterator i = s_aModules.begin(); i != s_aModules.end(); ++i)
			(*a_pfnCallback)(a_pContext, i->pszName, i->nVer1, i->nVer2);
	}
	else
	{
		EnterCriticalSection(&s_tClassList);
		try
		{
			for (CModules::const_iterator i = s_aModules.begin(); i != s_aModules.end(); ++i)
				(*a_pfnCallback)(a_pContext, i->pszName, i->nVer1, i->nVer2);
		}
		catch (...)
		{
		}
		LeaveCriticalSection(&s_tClassList);
	}
}
