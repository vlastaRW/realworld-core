// AutoUpdater.cpp : Implementation of CAutoUpdater

#include "stdafx.h"
#include "AutoUpdater.h"

#include <StringParsing.h>
#include <SharedStringTable.h>
//#include "ConfigureAutoUpdateDlg.h"
//#include "AutoUpdateStatusBarControl.h"
#include <wininet.h>


#pragma comment(lib, "wininet")

static LONG const g_nCheckPeriod = 7200; // 5 days (in minutes)


// CAutoUpdater

bool CAutoUpdater::PeriodEllapsed() const
{
	CConfigValue cVal;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_LASTCHECK), &cVal);
	LONG tNow = _time64(NULL)/60ULL;
	return (cVal.operator LONG()+g_nCheckPeriod) < tNow;
}

int CAutoUpdater::DownloadFile(LPCTSTR a_pszAgent, LPCTSTR a_pszServer, LPCTSTR a_pszPath, CAutoVectorPtr<BYTE>& szResponse)
{
	HINTERNET hSession = InternetOpen(a_pszAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession == NULL)
		return 0;

	if (m_eCommand == ETCExit)
		return -1;

	HINTERNET hConnect = InternetConnect(hSession, a_pszServer, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if (!hConnect)
	{
		InternetCloseHandle(hSession);
		return 0;
	}

	if (m_eCommand == ETCExit)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return -1;
	}

	HINTERNET hRequest = HttpOpenRequest(hConnect, _T("GET"), a_pszPath, NULL, NULL, NULL,
							INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
							INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | 
							INTERNET_FLAG_KEEP_CONNECTION |
							INTERNET_FLAG_NO_AUTH |
							INTERNET_FLAG_NO_AUTO_REDIRECT |
							INTERNET_FLAG_NO_COOKIES |
							INTERNET_FLAG_NO_UI |
							INTERNET_FLAG_RELOAD, NULL);
	if (!hRequest)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return 0;
	}

	if (m_eCommand == ETCExit)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return -1;
	}

	if (!HttpSendRequest(hRequest, NULL, 0, NULL, 0))
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return 0;
	}

	DWORD dwStatusCodeSize = sizeof(DWORD);
	DWORD dwStatusCode = 0;
	HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwStatusCodeSize, NULL);
	if (dwStatusCode != 200)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return 0;
	}

	if (m_eCommand == ETCExit)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return -1;
	}

	DWORD nResponseLength = 0;
	DWORD dwBytesAvailable = 0;
	while (InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
	{
		if (m_eCommand == ETCExit)
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return -1;
		}

		BYTE* p = new BYTE[nResponseLength+dwBytesAvailable+1];
		CopyMemory(p, szResponse.m_p, nResponseLength);
		szResponse.Free();
		szResponse.Attach(p);
		p += nResponseLength;
		p[dwBytesAvailable] = '\0';
		nResponseLength += dwBytesAvailable;

		DWORD dwBytesRead;
		BOOL bResult = InternetReadFile(hRequest, p, dwBytesAvailable, &dwBytesRead);
		if (!bResult)
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return 0;
		}

		if (m_eCommand == ETCExit)
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return -1;
		}

		if (dwBytesRead == 0)
		{
			break;	// End of File.
		}
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hSession);

	return nResponseLength;
}

typedef void (__stdcall fnEnumModuleCallback) (void* a_pContext, LPCWSTR a_pszName, ULONG a_nVer1, ULONG a_nVer2);
typedef void (STDAPICALLTYPE fnEnumModules)(void* a_pContext, fnEnumModuleCallback* a_pfnCallback);

extern __declspec(selectany) fnEnumModules* s_pfnEnumModules = NULL;

void EnumModules(void* a_pContext, fnEnumModuleCallback* a_pfnCallback)
{
	if (s_pfnEnumModules == NULL)
	{
		HMODULE hMod = GetModuleHandle(NULL);
#ifdef WIN64
		s_pfnEnumModules = (fnEnumModules*) GetProcAddress(hMod, "RWEnumModules");
#else
		s_pfnEnumModules = (fnEnumModules*) GetProcAddress(hMod, "_RWEnumModules@8");
#endif
		if (s_pfnEnumModules == NULL)
			return;
	}
	return (*s_pfnEnumModules)(a_pContext, a_pfnCallback);
}

struct ci_less
{
	bool operator() (std::wstring const& s1, std::wstring const& s2) const
	{
		return _wcsicmp(s1.c_str(), s2.c_str()) < 0;
	}
};
typedef std::map<std::wstring, std::pair<ULONG, ULONG>, ci_less> CInstalled;

void __stdcall SaveModules(void* a_pContext, LPCWSTR a_pszName, ULONG a_nVer1, ULONG a_nVer2)
{
	CInstalled& cInstalled = *reinterpret_cast<CInstalled*>(a_pContext);
	CInstalled::iterator i = cInstalled.find(a_pszName);
	if (i == cInstalled.end())
	{
		cInstalled[a_pszName] = std::make_pair(a_nVer1, a_nVer2);
	}
	else if (i->second.first < a_nVer1 || (i->second.first == a_nVer1 && i->second.second < a_nVer2))
	{
		i->second = std::make_pair(a_nVer1, a_nVer2);
	}
	//(*reinterpret_cast<CInstalled*>(a_pContext))[a_pszName] = std::make_pair(a_nVer1, a_nVer2);
}

unsigned __stdcall CAutoUpdater::AutoUpdateProc(void* a_pThis)
{
	return reinterpret_cast<CAutoUpdater*>(a_pThis)->AutoUpdate();
}

unsigned CAutoUpdater::AutoUpdate()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	m_eUpdateStatus = EUSWaiting;
	try
	{
		DWORD dwRes = WaitForSingleObject(m_hChange, 10000); // wait untill more important things are finished
		CComPtr<IApplicationInfo> pAppInfo;
		RWCoCreateInstance(pAppInfo, __uuidof(ApplicationInfo));
		CComBSTR bstrServer;
		pAppInfo->Account(&bstrServer, NULL, NULL);
		CComBSTR bstrAppID;
		pAppInfo->Identifier(&bstrAppID);
		ULONG nVer = 0;
		pAppInfo->Version(&nVer);
		if (bstrServer.m_str == NULL || _wcsnicmp(bstrServer.m_str, L"http://", 7) != 0 || bstrAppID.m_str == NULL)
		{
			m_eUpdateStatus = EUSFinished;
			return 0;
		}
		CComBSTR bstrPath;
		pAppInfo->AppDataFolder(&bstrPath);
		bstrPath += L"\\PlugIns";
		CreateDirectory(COLE2CT(bstrPath.m_str), NULL);
		pAppInfo = NULL;

		CInstalled cInstalled;

		while (true)
		{
			if (m_eCommand == ETCExit)
				break;
			if (m_eCommand == ETCCheckNow || (UpdatesEnabled() && PeriodEllapsed()))
			{
				m_eCommand = ETCStart; // potentially dangerous (can overwrite exit)
				m_eUpdateStatus = EUSChecking;

				OLECHAR szPath[128];
				OLECHAR szDomain[128];
				{
					LPWSTR p = wcschr(bstrServer.m_str+7, L'/');
					swprintf(szPath, L"%supdate/%s/%i.%i.%i", p, bstrAppID.m_str, (nVer>>16)&0xffff, (nVer>>8)&0xff, nVer&0xff);
					wcsncpy(szDomain, bstrServer.m_str+7, p-(bstrServer.m_str+7));
					szDomain[p-(bstrServer.m_str+7)] = L'\0';
				}

				CAutoVectorPtr<BYTE> szResponse;
				int nResponseLength = DownloadFile(_T("RealWorld - updater"), COLE2CT(szDomain), COLE2CT(szPath), szResponse);
				if (nResponseLength == -1 || m_eCommand == ETCExit)
					break;
				if (nResponseLength > 0)
				{
					EnumModules(&cInstalled, SaveModules);
					ULONG iChar = 0;
					while (iChar < ULONG(nResponseLength))
					{
						ULONG iLineEnd = iChar;
						while (iLineEnd < ULONG(nResponseLength) && szResponse[iLineEnd] != '\r' && szResponse[iLineEnd] != '\n')
							++iLineEnd;
						if (iChar < iLineEnd)
						{
							CAutoVectorPtr<WCHAR> pszLine(new WCHAR[(iLineEnd-iChar)+1]);
							int nWLen = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char const*>(szResponse.m_p+iChar), iLineEnd-iChar, pszLine.m_p, (iLineEnd-iChar)+1);
							pszLine[nWLen] = L'\0';
							WCHAR* pVer = pszLine;
							while (*pVer && *pVer != L' ' && *pVer != L'\t')
								++pVer;
							if (*pVer && pVer > pszLine.m_p)
							{
								*pVer = L'\0';
								++pVer;
								ULONG v1 = 0;
								ULONG v2 = 0;
								ULONG v3 = 0;
								ULONG v4 = 0;
								int nExpectedLength = 0;
								int nExpectedCRC = 0;
								int nPars = swscanf(pVer, L"%i.%i.%i.%i %i %x", &v1, &v2, &v3, &v4, &nExpectedLength, &nExpectedCRC);
								CInstalled::iterator iIM = cInstalled.find(pszLine.m_p);
								if (iIM != cInstalled.end() && 5 <= nPars &&
									(((v1<<16)|v2) > iIM->second.first || (((v1<<16)|v2) == iIM->second.first && ((v3<<16)|v4) > iIM->second.second)))
								{
									// candidate for update
									OLECHAR szPath[128];
									{
										LPWSTR p = wcschr(bstrServer.m_str+7, L'/');
#ifdef _WIN64
										swprintf(szPath, L"%splugin/download64/%i.%i.%i.%i/%s", p, v1, v2, v3, v4, pszLine.m_p);
#else
										swprintf(szPath, L"%splugin/download32/%i.%i.%i.%i/%s", p, v1, v2, v3, v4, pszLine.m_p);
#endif
									}

									CAutoVectorPtr<BYTE> szDll;
									int nDllLength = DownloadFile(_T("RealWorld - updater"), COLE2CT(szDomain), COLE2CT(szPath), szDll);
									if (nDllLength != nExpectedLength || m_eCommand == ETCExit)
										break;
									// TODO: check CRC

									CComBSTR bstrFolder(bstrPath);
									bstrFolder += L"\\Updates";
									CComBSTR bstrName(bstrFolder);
									bstrName += L"\\";
									bstrName += iIM->first.c_str();
									CreateDirectory(COLE2CT(bstrFolder.m_str), NULL);
									HANDLE h = CreateFile(COLE2CT(bstrName.m_str), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
									if (h == INVALID_HANDLE_VALUE)
										break;
									DWORD dw = 0;
									WriteFile(h, szDll.m_p, nDllLength, &dw, NULL);
									CloseHandle(h);
									if (dw != nDllLength)
										DeleteFile(COLE2CT(bstrName.m_str));
									iIM->second.first = (v1<<16)|v2;
									iIM->second.second = (v3<<16)|v4;
								}
							}
						}
						iChar = iLineEnd+1;
						if (m_eCommand == ETCExit)
							break;
					}
					if (iChar >= ULONG(nResponseLength))
					{
						// success - save time stamp
						CComBSTR bstr(CFGID_LASTCHECK);
						m_pConfig->ItemValuesSet(1, &(bstr.m_str), CConfigValue(LONG(_time64(NULL)/60ULL)));
					}
					if (m_eCommand == ETCExit)
						break;
				}

				m_eUpdateStatus = EUSWaiting;
			}
			WaitForSingleObject(m_hChange, 60000);
		}
	}
	catch (...)
	{
	}
	CoUninitialize();

	return 0;
}

