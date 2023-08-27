// StartPageOnline.cpp : Implementation of CStartPageOnline

#include "stdafx.h"
#include "StartPageOnline.h"
#include <wininet.h>
#include "ConfigIDsApp.h"

#pragma comment(lib, "wininet")


// CStartPageOnline

// StartPageOnline.cpp : Implementation of CStartPageOnline

#include "stdafx.h"
#include "StartPageOnline.h"
#include <atltime.h>
#define AFX_RESOURCE_DLL
#include <RWVersionInfo.rc2>
#include <MultiLanguageString.h>


STDMETHODIMP CStartPageOnline::Destroy()
{
	m_pClbk = NULL;
	DestroyWindow();
	return S_OK;
}

STDMETHODIMP CStartPageOnline::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	if (a_bBeforeAccel && m_pWBObject && m_pWBObject->TranslateAccelerator(const_cast<MSG*>(a_pMsg)) == S_OK)
		return S_OK;
	return S_FALSE;
}

STDMETHODIMP CStartPageOnline::Activate()
{
	//m_pClbk->SetDefaultButtonState(CMultiLanguageString::GetAuto(L"[0409]Refresh[0405]Obnovit"), true, NULL);
	Show(TRUE);
	GotoDlgCtrl(GetDlgItem(IDC_IECTL));
	return S_OK;
}

STDMETHODIMP CStartPageOnline::Deactivate()
{
	return Show(FALSE);
}

STDMETHODIMP CStartPageOnline::ClickedDefault()
{
	Refresh();
	return S_FALSE;
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
static DWORD RGB2BGR(DWORD rgb) { return ((rgb&0xff)<<16)|(rgb&0xff00)|((rgb>>16)&0xff);}

void CStartPageOnline::Refresh()
{
	if (m_pWebBrowser == NULL)
		return;
	static WCHAR szOS[32] = L"";
	static LPCWSTR pszBuild = NULL;
	if (pszBuild == NULL)
	{
#ifdef _WIN64
		pszBuild = L"64";
		OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		GetVersionEx(&tVersion);
		swprintf(szOS, L"%i.%i|64", tVersion.dwMajorVersion, tVersion.dwMinorVersion);
#else
#ifdef _UNICODE
		pszBuild = L"32";
#else
		pszBuild = L"9x";
#endif
		OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		GetVersionEx(&tVersion);
		BOOL bIsWow64 = FALSE;
		if (tVersion.dwMajorVersion > 5 || (tVersion.dwMajorVersion == 5 && tVersion.dwMinorVersion >= 1))
		{
			LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process");
		    if (NULL != fnIsWow64Process)
			{
				fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
			}
		}
		swprintf(szOS, L"%i.%i|%s", tVersion.dwMajorVersion, tVersion.dwMinorVersion, tVersion.dwPlatformId == VER_PLATFORM_WIN32_NT ? (bIsWow64 ? L"64" : L"32") : L"9x");
#endif
	}

	DWORD clr3D = RGB2BGR(GetSysColor(COLOR_3DFACE));
	DWORD clrWnd = RGB2BGR(GetSysColor(COLOR_WINDOW));
	DWORD clrTxt = RGB2BGR(GetSysColor(COLOR_WINDOWTEXT));
	NONCLIENTMETRICS ncm;
	ZeroMemory(&ncm, sizeof ncm);
	ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
	static int nLogPixels = 0;
	if (nLogPixels == 0)
	{
		HDC hDC = ::GetDC(NULL);
		nLogPixels = GetDeviceCaps(hDC, LOGPIXELSY);
		::ReleaseDC(NULL, hDC);
	}
	int wFontSize = (WORD)MulDiv(ncm.lfMessageFont.lfHeight < 0 ? -ncm.lfMessageFont.lfHeight : ncm.lfMessageFont.lfHeight, 72, nLogPixels);

	CComPtr<IApplicationInfo> pAppInfo;
	RWCoCreateInstance(pAppInfo, __uuidof(ApplicationInfo));
	ULONG nVer = 0;
	pAppInfo->Version(&nVer);
	CComBSTR bstrServer;
	CComBSTR bstrEmail;
	CComBSTR bstrPassHash;
	pAppInfo->Account(&bstrServer, &bstrEmail, &bstrPassHash);
	CComBSTR bstrSerial;
	ULONG nEval = 0;
	pAppInfo->License(NULL, NULL, &bstrSerial, NULL, &nEval);

	CComPtr<ITranslatorManager> pTr;
	RWCoCreateInstance(pTr, __uuidof(Translator));

	OLECHAR szTmp[1024];
	if (m_bstrURL == NULL)
	{
		CComBSTR bstrAppID;
		pAppInfo->Identifier(&bstrAppID);
		swprintf(szTmp, L"%sstart/%s", bstrServer.m_str, bstrAppID.m_str);
		m_bstrURL = szTmp;
	}
	swprintf(szTmp, L"appver=%i.%i.%i|%s&osver=%s&pgiface=1&localizable=%i&font=%s&fontsize=%i&resolution=%i&color3D=%06x&colorBg=%06x&colorFg=%06x",
		(nVer>>16)&0xffff, (nVer>>8)&0xff, nVer&0xff, pszBuild, szOS, 
		pTr.p ? 1 : 0,
		ncm.lfMessageFont.lfFaceName, wFontSize, nLogPixels,
		clr3D, clrWnd, clrTxt);
	if (pTr.p)
	{
		swprintf(szTmp+wcslen(szTmp), L"&applang=%04x&oslang=%04x", LANGIDFROMLCID(m_tLocaleID), GetUserDefaultUILanguage());
	}
	if (bstrEmail.Length() && bstrPassHash.Length())
	{
		swprintf(szTmp+wcslen(szTmp), L"&email=%s&pass=%s", bstrEmail.m_str, bstrPassHash.m_str);
	}
	if (bstrSerial.Length())
	{
		swprintf(szTmp+wcslen(szTmp), L"&serial=%s", bstrSerial.m_str);
	}

	CComVariant cHeaders(L"Content-Type:application/x-www-form-urlencoded\r\n");

	CComVariant cPost;
	cPost.vt = VT_ARRAY|VT_UI1;
	ULONG nTmpLen = wcslen(szTmp);
	ULONG nUTF8Len = WideCharToMultiByte(CP_UTF8, 0, szTmp, nTmpLen, NULL, 0, NULL, NULL);
	cPost.parray = SafeArrayCreateVector(VT_UI1, 0, nUTF8Len);
	char* pData = NULL;
	SafeArrayAccessData(cPost.parray, reinterpret_cast<void**>(&pData));
	WideCharToMultiByte(CP_UTF8, 0, szTmp, nTmpLen, pData, nUTF8Len, NULL, NULL);
	SafeArrayUnaccessData(cPost.parray);

	m_pWebBrowser->Navigate(m_bstrURL, NULL, NULL, &cPost, &cHeaders);
}

LRESULT CStartPageOnline::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DispEventUnadvise(m_pWebBrowser, &DIID_DWebBrowserEvents2);
	m_pWebBrowser = NULL;
	bHandled = FALSE;
	return 0;
}

LRESULT CStartPageOnline::OnRefreshPage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Refresh();
	return 0;
}

LRESULT CStartPageOnline::OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	CComQIPtr<IAxWinHostWindow> pAxWin(m_pUnkContainer);
	//TCHAR szInitial[] = _T("mshtml:<html><head></head><body>\n")
	//					_T("<p>Loading online resources...<br />\n")
	//					_T("<small>Active connection to Internet is needed.</small></p>\n")
	//					_T("<script type=\"text/javascript\">\n")
	//					_T("<!--\n")
	//					_T("window.location = \"http://localhost/RW/start/RWPaint?2009.1.0\"\n")
	//					_T("//-->\n")
	//					_T("</script><a href=\"http://localhost/RW/start/RWPaint?2009.1.0\">xxxx</a></body></html>");
	pAxWin->CreateControl(/*szInitial*/_T("{8856F961-340A-11D0-A96B-00C04FD705A2}"), GetDlgItem(IDC_IECTL), NULL);
	GetDlgControl(IDC_IECTL, __uuidof(IWebBrowser2), reinterpret_cast<void**>(&m_pWebBrowser));
	m_pWebBrowser->QueryInterface(&m_pWBObject);
	m_pWebBrowser->put_RegisterAsDropTarget(VARIANT_FALSE);
	DispEventAdvise(m_pWebBrowser, &DIID_DWebBrowserEvents2);
	Refresh();

	return TRUE;  // Let the system set the focus
}

LRESULT CStartPageOnline::OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	GetDlgItem(IDC_IECTL).SetWindowPos(NULL, 0, 0, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam), SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	return 0;
}

STDMETHODIMP CStartPageOnline::GetHostInfo(DOCHOSTUIINFO* a_pInfo)
{
    a_pInfo->cbSize = sizeof*a_pInfo;
    a_pInfo->dwFlags =
		DOCHOSTUIFLAG_DIALOG |
		DOCHOSTUIFLAG_NO3DBORDER |
		/*DOCHOSTUIFLAG_SCROLL_NO |*/
		DOCHOSTUIFLAG_THEME |
		DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL |
		DOCHOSTUIFLAG_URL_ENCODING_ENABLE_UTF8 |
		DOCHOSTUIFLAG_BROWSER |
		DOCHOSTUIFLAG_NOPICS |
		DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION;// |
		//DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE;
    a_pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
	DWORD clr3D = RGB2BGR(GetSysColor(COLOR_3DFACE));
	DWORD clrWnd = RGB2BGR(GetSysColor(COLOR_WINDOW));
	DWORD clrTxt = RGB2BGR(GetSysColor(COLOR_WINDOWTEXT));
	NONCLIENTMETRICS ncm;
	ZeroMemory(&ncm, sizeof ncm);
	ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
	static int nLogPixels = 0;
	if (nLogPixels == 0)
	{
		HDC hDC = ::GetDC(NULL);
		nLogPixels = GetDeviceCaps(hDC, LOGPIXELSY);
		::ReleaseDC(NULL, hDC);
	}
	int wFontSize = (WORD)MulDiv(ncm.lfMessageFont.lfHeight < 0 ? -ncm.lfMessageFont.lfHeight : ncm.lfMessageFont.lfHeight, 72, nLogPixels);
	wchar_t* pszCSS = reinterpret_cast<wchar_t*>(CoTaskMemAlloc(512*sizeof(wchar_t)));
	swprintf(pszCSS, 512,
		L"body {background-color:#%06x;font-family:%s,Arial;font-size:%ipt;line-height:135%%;text-align:left;color:#%06x;} h1,h2 {color:#%06x;} a {text-decoration:none;} a:link, a:visited, a:hover {color:#06c;}",
		clr3D, ncm.lfMessageFont.lfFaceName, wFontSize, clrTxt, clrTxt);
	a_pInfo->pchHostCss = pszCSS;
    a_pInfo->pchHostNS = NULL;

    return S_OK;
}

STDMETHODIMP CStartPageOnline::GetOptionKeyPath(LPOLESTR* a_pchKey, DWORD a_dw)
{
	return S_FALSE;
	//if (a_pchKey == NULL)
	//	return E_INVALIDARG;
	//static WCHAR const[] szMyKey = L"Software\\RealWorld\\Designer\\IEOptions";
	//*a_pchKey = (LPOLESTR)CoTaskMemAlloc(szMyKey);
	//if (*pchKey == NULL)
	//	return E_OUTOFMEMORY;
	//wcscpy(*pchKey, szMyKey);
	//static bool bKeysSet = false;
	//if (!bKeysSet)
	//{
	//	CRegKey cKey();
	//	bKeysSet = true;
	//}
	//return S_OK;
}

// status bar access from JavaScript:
//[HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings\Zones\3]
//value name "2103" change from '0' (disable) to '3' (enable)


STDMETHODIMP CStartPageOnline::CExternal::IsInstalled(BSTR plugInID, VARIANT_BOOL* res)
{
	try
	{
		*res = VARIANT_FALSE;
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		CComBSTR bstrPath;
		pAI->AppDataFolder(&bstrPath);
		bstrPath += L"\\PlugIns\\";
		CComBSTR bstrUpdates(bstrPath);
		bstrPath += plugInID;
		bstrPath += L".dll";
		if (GetFileAttributes(bstrPath) != 0xffffffff)
		{
			*res = VARIANT_TRUE;
			bstrUpdates += L"Updates\\";
			bstrUpdates += plugInID;
			bstrUpdates += L".dll";
			if (GetFileAttributes(bstrUpdates) != 0xffffffff)
			{
				// is it scheduled for deletition ?
				HANDLE hFile = CreateFile(bstrUpdates, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					if (0 == GetFileSize(hFile, NULL))
						*res = VARIANT_FALSE;
					CloseHandle(hFile);
				}
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return res ? E_UNEXPECTED : E_POINTER;
	}
}

int DownloadFile(LPCTSTR a_pszAgent, LPCTSTR a_pszServer, LPCTSTR a_pszPath, CAutoVectorPtr<BYTE>& szResponse)
{
	HINTERNET hSession = InternetOpen(a_pszAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession == NULL)
		return 0;

	HINTERNET hConnect = InternetConnect(hSession, a_pszServer, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if (!hConnect)
	{
		InternetCloseHandle(hSession);
		return 0;
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

	DWORD nResponseLength = 0;
	DWORD dwBytesAvailable = 0;
	while (InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
	{
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

typedef void (STDAPICALLTYPE fnLoadPlugIns)(LPCWSTR a_pszFilename);

extern __declspec(selectany) fnLoadPlugIns* s_pfnLoadPlugIns = NULL;

void STDAPICALLTYPE DummyLoadPlugIns(LPCWSTR) {}
void LoadPlugIns(LPCWSTR a_pszFilename)
{
	if (s_pfnLoadPlugIns == NULL)
	{
		HMODULE hMod = GetModuleHandle(NULL);
#ifdef WIN64
		s_pfnLoadPlugIns = (fnLoadPlugIns*) GetProcAddress(hMod, "RWLoadPlugIns");
#else
		s_pfnLoadPlugIns = (fnLoadPlugIns*) GetProcAddress(hMod, "_RWLoadPlugIns@4");
#endif
		if (s_pfnLoadPlugIns == NULL)
			s_pfnLoadPlugIns = &DummyLoadPlugIns;
	}
	return (*s_pfnLoadPlugIns)(a_pszFilename);
}

STDMETHODIMP CStartPageOnline::CExternal::InstallPlugIn(BSTR plugInID, BSTR version, VARIANT_BOOL* res)
{
	try
	{
		*res = VARIANT_FALSE;

		CComPtr<IApplicationInfo> pAppInfo;
		RWCoCreateInstance(pAppInfo, __uuidof(ApplicationInfo));
		CComBSTR bstrServer;
		pAppInfo->Account(&bstrServer, NULL, NULL);
		if (bstrServer.m_str == NULL || _wcsnicmp(bstrServer.m_str, L"http://", 7) != 0)
			return S_FALSE;
		OLECHAR szPath[128];
		{
			LPWSTR p = wcschr(bstrServer.m_str+7, L'/');
#ifdef _WIN64
			swprintf(szPath, L"%splugin/download64/%s/%s.dll", p, version, plugInID);
#else
			swprintf(szPath, L"%splugin/download32/%s/%s.dll", p, version, plugInID);
#endif
			*p = L'\0';
		}

		CAutoVectorPtr<BYTE> m_szResponse;
		DWORD m_nResponseLength = DownloadFile(_T("RealWorld - plug-in download"), COLE2CT(bstrServer.m_str+7), COLE2CT(szPath), m_szResponse);
		if (m_nResponseLength == 0)
			return S_FALSE;

		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		CComBSTR bstrPath;
		pAI->AppDataFolder(&bstrPath);
		bstrPath += L"\\PlugIns";
		CComBSTR bstrFolder(bstrPath);
		bstrPath += L"\\";
		bstrPath += plugInID;
		bstrPath += L".dll";
		bool bUpdate = false;
		if (GetFileAttributes(bstrPath) != 0xffffffff)
		{
			bstrFolder += L"\\Updates";
			bstrPath = bstrFolder;
			bstrPath += L"\\";
			bstrPath += plugInID;
			bstrPath += L".dll";
			bUpdate = true;
		}
		CreateDirectory(COLE2CT(bstrFolder.m_str), NULL);
		HANDLE h = CreateFile(COLE2CT(bstrPath.m_str), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD dw;
			WriteFile(h, m_szResponse.m_p, m_nResponseLength, &dw, NULL);
			CloseHandle(h);
			if (dw == m_nResponseLength)
			{
				// TODO: uncomment when IPlugInCache and CooperatingObjectsManager are updated
				if (bUpdate)
					*res = VARIANT_TRUE;
				else
					LoadPlugIns(bstrPath.m_str);
			}
		}

		return S_OK;
	}
	catch (...)
	{
		return res ? E_UNEXPECTED : E_POINTER;
	}
}

extern __declspec(selectany) fnLoadPlugIns* s_pfnUnloadPlugIns = NULL;

void UnloadPlugIns(LPCWSTR a_pszFilename)
{
	if (s_pfnUnloadPlugIns == NULL)
	{
		HMODULE hMod = GetModuleHandle(NULL);
#ifdef WIN64
		s_pfnUnloadPlugIns = (fnLoadPlugIns*) GetProcAddress(hMod, "RWUnloadPlugIns");
#else
		s_pfnUnloadPlugIns = (fnLoadPlugIns*) GetProcAddress(hMod, "_RWUnloadPlugIns@4");
#endif
		if (s_pfnUnloadPlugIns == NULL)
			s_pfnUnloadPlugIns = &DummyLoadPlugIns;
	}
	return (*s_pfnUnloadPlugIns)(a_pszFilename);
}

STDMETHODIMP CStartPageOnline::CExternal::RemovePlugIn(BSTR plugInID, VARIANT_BOOL* res)
{
	try
	{
		*res = VARIANT_FALSE;

		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		CComBSTR bstrPath;
		pAI->AppDataFolder(&bstrPath);
		bstrPath += L"\\PlugIns";
		CComBSTR bstrFolder(bstrPath);
		bstrPath += L"\\";
		bstrPath += plugInID;
		bstrPath += L".dll";
		if (GetFileAttributes(bstrPath) == 0xffffffff)
			return S_FALSE; // plug-in is not present
		// TODO: uncomment when IPlugInCache and CooperatingObjectsManager are updated
		//UnloadPlugIns(bstrPath);
		//if (DeleteFile(bstrPath))
		//	return S_OK; // plug-in was successfully deleted
		bstrFolder += L"\\Updates";
		bstrPath = bstrFolder;
		bstrPath += L"\\";
		bstrPath += plugInID;
		bstrPath += L".dll";
		CreateDirectory(COLE2CT(bstrFolder.m_str), NULL);
		HANDLE h = CreateFile(COLE2CT(bstrPath.m_str), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(h);
			*res = VARIANT_TRUE;
		}

		return S_OK;
	}
	catch (...)
	{
		return res ? E_UNEXPECTED : E_POINTER;
	}
}

#pragma comment(lib, "version.lib")

STDMETHODIMP CStartPageOnline::CExternal::GetPlugInVersion(BSTR plugInID, BSTR* ver)
{
	try
	{
		*ver = NULL;

		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		CComBSTR bstrPath;
		pAI->AppDataFolder(&bstrPath);
		bstrPath += L"\\PlugIns";
		CComBSTR bstrUpdatesPath(bstrPath);
		bstrPath += L"\\";
		bstrPath += plugInID;
		bstrPath += L".dll";
		if (GetFileAttributes(bstrPath) == 0xffffffff)
			return S_FALSE; // plug-in is not present
		bstrUpdatesPath += L"\\Updates";
		bstrUpdatesPath += L"\\";
		bstrUpdatesPath += plugInID;
		bstrUpdatesPath += L".dll";
		if (GetFileAttributes(bstrUpdatesPath) != 0xffffffff)
			bstrPath = bstrUpdatesPath; // the updated version is more important
		COLE2CT strPath(bstrPath.m_str);
		DWORD dwVer = 0;
		DWORD nLen = GetFileVersionInfoSize(strPath, &dwVer);
		if (nLen == 0)
			return S_FALSE;
		CAutoVectorPtr<BYTE> pVerData(new BYTE[nLen]);
		if (!GetFileVersionInfo(strPath, dwVer, nLen, pVerData.m_p))
			return S_FALSE;

		LPVOID pBlock = NULL;
		UINT nBlock = 0;
		BOOL bRes = VerQueryValue(pVerData.m_p, _T("\\VarFileInfo\\Translation"), &pBlock, &nBlock);
		if (!bRes) return S_FALSE;
		TCHAR szVerKey[64];
		if (nBlock == 4)
		{
			DWORD dwLang = *reinterpret_cast<DWORD const*>(pBlock);
			_stprintf(szVerKey, _T("\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion"), (dwLang & 0xff00)>>8, dwLang & 0xff, (dwLang & 0xff000000)>>24, (dwLang & 0xff0000)>>16);
		}
		else
		{
			_stprintf(szVerKey, _T("\\StringFileInfo\\%04X04B0\\FileVersion"), GetUserDefaultLangID());
		}
		bRes = VerQueryValue(pVerData.m_p, szVerKey, &pBlock, &nBlock);
		if (!bRes) return S_FALSE;

		CComBSTR bstr((char*)pBlock);
		*ver = bstr.Detach();

		return S_OK;
	}
	catch (...)
	{
		return ver ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_AppLangID(ULONG* pnLangID)
{
	try
	{
		*pnLangID = LANGIDFROMLCID(m_tLocaleID);
		return S_OK;
	}
	catch (...)
	{
		return pnLangID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::put_AppLangID(ULONG nLangID)
{
	try
	{
		if (LANGIDFROMLCID(m_tLocaleID) == nLangID)
			return S_OK;
		CComPtr<ITranslatorManager> pTM;
		RWCoCreateInstance(pTM, __uuidof(Translator));
		pTM->Synchronize(LANGIDFROMLCID(nLangID), 0);
		CComQIPtr<IConfig> pCfg(pTM);
		CComBSTR bstrID(CFGID_LANGUAGECODE);
		pCfg->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(LONG(nLangID)));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_AppLangStamp(ULONG* pnLangStamp)
{
	try
	{
		*pnLangStamp = 0;
		CComPtr<ITranslatorManager> pTM;
		RWCoCreateInstance(pTM, __uuidof(Translator));
		pTM->LangInfo(LANGIDFROMLCID(m_tLocaleID), NULL, pnLangStamp, 0, 0, 0);
		return S_OK;
	}
	catch (...)
	{
		return pnLangStamp ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_OSLangID(ULONG* pnLangID)
{
	try
	{
		*pnLangID = GetUserDefaultUILanguage();
		return S_OK;
	}
	catch (...)
	{
		return pnLangID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::UpdateLanguage(VARIANT_BOOL* res)
{
	try
	{
		*res = VARIANT_FALSE;
		CComPtr<ITranslatorManager> pTM;
		RWCoCreateInstance(pTM, __uuidof(Translator));
		if (SUCCEEDED(pTM->Synchronize(LANGIDFROMLCID(m_tLocaleID), 0)))
			*res = VARIANT_TRUE;
		return S_OK;
	}
	catch (...)
	{
		return res ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_EvalMax(ULONG* pnVal)
{
	try
	{
		*pnVal = 0;
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		ELicensingMode eMode = ELMDonate;
		pAI->LicensingMode(&eMode);
		if (eMode == ELMEnterSerial)
			*pnVal = 30;
		return S_OK;
	}
	catch (...)
	{
		return pnVal ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_EvalDay(ULONG* pnVal)
{
	try
	{
		*pnVal = 0;
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		return pAI->License(NULL, NULL, NULL, NULL, pnVal);
	}
	catch (...)
	{
		return pnVal ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_LicName(BSTR* pVal)
{
	try
	{
		*pVal = NULL;
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		pAI->License(pVal, NULL, NULL, NULL, NULL);
		return S_OK;
	}
	catch (...)
	{
		return pVal ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_LicOrg(BSTR* pVal)
{
	try
	{
		*pVal = NULL;
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		pAI->License(NULL, pVal, NULL, NULL, NULL);
		return S_OK;
	}
	catch (...)
	{
		return pVal ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_LicCode(BSTR* pVal)
{
	try
	{
		*pVal = NULL;
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		ULONG n = 0;
		pAI->License(NULL, NULL, NULL, &n, NULL);
		if (n == 0)
			return S_FALSE;
		OLECHAR sz[16] = L"";
		_itow(n, sz, 16);
		*pVal = SysAllocString(sz);
		return S_OK;
	}
	catch (...)
	{
		return pVal ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::SetValue(BSTR key, BSTR val)
{
	try
	{
		if (m_pContext == NULL)
			return S_FALSE;
		if (val == NULL || *val == L'\0')
		{
			m_pContext->DeleteItems(key);
			return S_OK;
		}
		TConfigValue tVal;
		ZeroMemory(&tVal, sizeof tVal);
		tVal.eTypeID = ECVTString;
		tVal.bstrVal = val;
		m_pContext->ItemValuesSet(1, &key, &tVal);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::GetValue(BSTR key, BSTR* pVal)
{
	try
	{
		*pVal = NULL;
		if (m_pContext == NULL)
			return S_FALSE;
		CConfigValue cVal;
		m_pContext->ItemValueGet(key, &cVal);
		if (cVal.TypeGet() != ECVTString)
			return S_FALSE;
		*pVal = cVal.Detach().bstrVal;
		return S_OK;
	}
	catch (...)
	{
		return pVal ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::Translate(BSTR eng, BSTR* pVal)
{
	try
	{
		*pVal = NULL;
		if (eng == NULL)
			return S_FALSE;
		CComPtr<ITranslator> pTr;
		RWCoCreateInstance(pTr, __uuidof(Translator));
		if (pTr)
			pTr->Translate(eng, m_tLocaleID, pVal);
		if (*pVal == NULL)
			*pVal = SysAllocString(eng);

		return S_OK;
	}
	catch (...)
	{
		return pVal ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::ChangeAccount(BSTR email, BSTR password)
{
	try
	{
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		HRESULT hRes = pAI->AccountSet(email, password);
		if (m_pOwner) m_pOwner->Refresh();
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::ChangeLicense(BSTR name, BSTR organization, BSTR serial)
{
	try
	{
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		HRESULT hRes = pAI->LicenseSet(name, organization, serial);
		if (m_pOwner) m_pOwner->Refresh();
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::OpenFile(BSTR path)
{
	try
	{
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::RunCommand(BSTR commandCode)
{
	try
	{
		if (commandCode == NULL)
			return E_INVALIDARG;
		if (wcscmp(commandCode, L"ChangeLicense") == 0)
		{
			CComPtr<IApplicationInfo> pAI;
			RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
			CComBSTR bstrOld;
			pAI->License(NULL, NULL, &bstrOld, NULL, NULL);
			pAI->LicenseBox(m_pOwner->m_hWnd, m_tLocaleID);
			CComBSTR bstrNew;
			pAI->License(NULL, NULL, &bstrNew, NULL, NULL);
			if (bstrOld != bstrNew)
				::PostMessage(m_pOwner->m_hWnd, WM_REFRESHPAGE, 0, 0);
			return S_OK;
		}

		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_status(BSTR* pVal)
{
	try
	{
		*pVal = NULL;
		return m_pOwner->m_bstrStatusTextOverride.CopyTo(pVal);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::put_status(BSTR val)
{
	try
	{
		if (val && *val)
		{
			if (m_pOwner->m_bstrStatusTextOverride != val)
			{
				m_pOwner->m_bstrStatusTextOverride = val;
				m_pOwner->m_pClbk->UpdateStatusBar();
			}

		}
		else
		{
			if (m_pOwner->m_bstrStatusTextOverride != NULL)
			{
				m_pOwner->m_bstrStatusTextOverride.Empty();
				m_pOwner->m_pClbk->UpdateStatusBar();
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_TagLib(VARIANT_BOOL* res)
{
	static int iTagLib = -1;
	if (iTagLib == -1)
	{
		CComPtr<IStorageFilterFactory> p;
		static CLSID const CLSID_StorageFactorySQLite = {0x9BDB81B6, 0x14DF, 0x4093, {0xB9,0x33,0xF2,0x5A,0xFA,0x41,0x6E,0x60} };
		RWCoCreateInstance(p, CLSID_StorageFactorySQLite);
		iTagLib = p != NULL;
	}
	*res = iTagLib ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CStartPageOnline::CExternal::TagLibCheck(BSTR fileID, VARIANT_BOOL* res)
{
	try
	{
		*res = VARIANT_FALSE;
		CComPtr<IStorageManager> pSM;
		RWCoCreateInstance(pSM, __uuidof(StorageManager));
		CComPtr<IStorageFilter> p;
		CComBSTR bstrPath(L"tags://#");
		bstrPath += fileID;
		pSM->FilterCreate(bstrPath, EFTOpenExisting|EFTHintNoStream, &p);
		*res = p && SUCCEEDED(p->SrcOpen(NULL)) ? VARIANT_TRUE : VARIANT_FALSE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::TagLibAdd(BSTR fileID, BSTR tagsAndName, VARIANT_BOOL* res)
{
	try
	{
		*res = VARIANT_FALSE;

		CComPtr<IStorageManager> pSM;
		RWCoCreateInstance(pSM, __uuidof(StorageManager));
		CComPtr<IStorageFilter> p;
		CComBSTR bstrPath(L"tags://#");
		bstrPath += fileID;
		pSM->FilterCreate(bstrPath, EFTOpenExisting|EFTHintNoStream, &p);
		if (p && SUCCEEDED(p->SrcOpen(NULL)))
		{
			if (!m_pOwner->m_bOverwrite)
			{
				CComPtr<IApplicationInfo> pAppInfo;
				RWCoCreateInstance(pAppInfo, __uuidof(ApplicationInfo));
				CComPtr<ILocalizedString> pAppName;
				pAppInfo->Name(&pAppName);
				CComBSTR bstrAppName;
				pAppName->GetLocalized(m_tLocaleID, &bstrAppName);
				CComBSTR bstrMessage;
				CMultiLanguageString::GetLocalized(L"[0409]File already exists in the media library. Overwrite the current file?[0405]Soubor již v knihovně médií existuje. Chcete současný soubor přepsat?", m_tLocaleID, &bstrMessage);
				m_pOwner->m_bOverwrite = IDYES == m_pOwner->MessageBox(COLE2CT(bstrMessage.m_str), COLE2CT(bstrAppName.m_str), MB_YESNO);
			}
			if (!m_pOwner->m_bOverwrite)
				return S_FALSE;
		}
		p = NULL;

		CComPtr<IApplicationInfo> pAppInfo;
		RWCoCreateInstance(pAppInfo, __uuidof(ApplicationInfo));
		CComBSTR bstrServer;
		pAppInfo->Account(&bstrServer, NULL, NULL);
		if (bstrServer.m_str == NULL && _wcsnicmp(bstrServer.m_str, L"http://", 7) != 0)
			return S_FALSE;
		OLECHAR szPath[128];
		{
			LPWSTR p = wcschr(bstrServer.m_str+7, L'/');
			swprintf(szPath, L"%staglib/download/%s", p, fileID);
			*p = L'\0';
		}

		CAutoVectorPtr<BYTE> szResponse;
		DWORD nResponseLength = DownloadFile(_T("RealWorld - media download"), COLE2CT(bstrServer.m_str+7), COLE2CT(szPath), szResponse);
		if (nResponseLength == 0)
			return S_FALSE;

		bstrPath += L"\\";
		bstrPath += tagsAndName;
		pSM->FilterCreate(bstrPath, EFTCreateNew, &p);
		if (p == NULL)
			return S_FALSE;
		CComPtr<IDataDstStream> pDst;
		p->DstOpen(&pDst);
		pDst->Write(nResponseLength, szResponse);
		pDst->Close();
		*res = VARIANT_TRUE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static GUID const s_guidBatchProcessor = {0x62388dc7, 0x0664, 0x4504, {0x89, 0x0d, 0xac, 0x11, 0x75, 0x64, 0x41, 0x80}}; // CLSID_StartPageBatchImageProcessor

STDMETHODIMP CStartPageOnline::CExternal::get_BatchOp(VARIANT_BOOL* res)
{
	try
	{
		static int iStatus = -1;
		if (iStatus == -1)
		{
			CComPtr<IDesignerBatchOpManager> p;
			RWCoCreateInstance(p, s_guidBatchProcessor);
			iStatus = p.p ? 0 : 1;
		}
		*res = iStatus ? VARIANT_FALSE : VARIANT_TRUE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::BatchOpCheck(BSTR opName, VARIANT_BOOL* res)
{
	try
	{
		CComPtr<IDesignerBatchOpManager> p;
		RWCoCreateInstance(p, s_guidBatchProcessor);
		*res = p && p->HasOperation(m_pMainConfig, opName) == S_OK ? VARIANT_TRUE : VARIANT_FALSE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::BatchOpAdd(BSTR fileName, VARIANT_BOOL* res)
{
	try
	{
		*res = VARIANT_FALSE;

		CComPtr<IApplicationInfo> pAppInfo;
		RWCoCreateInstance(pAppInfo, __uuidof(ApplicationInfo));
		CComBSTR bstrServer;
		pAppInfo->Account(&bstrServer, NULL, NULL);
		if (bstrServer.m_str == NULL && _wcsnicmp(bstrServer.m_str, L"http://", 7) != 0)
			return S_FALSE;
		OLECHAR szPath[128];
		{
			LPWSTR p = wcschr(bstrServer.m_str+7, L'/');
			swprintf(szPath, L"%sbatchop-download/%s.rwbatchop", p, fileName);
			*p = L'\0';
		}

		CAutoVectorPtr<BYTE> szResponse;
		DWORD nResponseLength = DownloadFile(_T("RealWorld - media download"), COLE2CT(bstrServer.m_str+7), COLE2CT(szPath), szResponse);
		if (nResponseLength == 0)
			return S_FALSE;

		CComPtr<IDesignerBatchOpManager> p;
		RWCoCreateInstance(p, s_guidBatchProcessor);
		*res = p && p->AddOperation(m_pMainConfig, nResponseLength, szResponse) == S_OK ? VARIANT_TRUE : VARIANT_FALSE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::LayoutAdd(BSTR fileName, VARIANT_BOOL* res)
{
	try
	{
		*res = VARIANT_FALSE;

		CComPtr<IApplicationInfo> pAppInfo;
		RWCoCreateInstance(pAppInfo, __uuidof(ApplicationInfo));
		CComBSTR bstrServer;
		pAppInfo->Account(&bstrServer, NULL, NULL);
		if (bstrServer.m_str == NULL && _wcsnicmp(bstrServer.m_str, L"http://", 7) != 0)
			return S_FALSE;
		OLECHAR szPath[128];
		{
			LPWSTR p = wcschr(bstrServer.m_str+7, L'/');
			swprintf(szPath, L"%slayout-download/%s.rwlayout", p, fileName);
			*p = L'\0';
		}

		CAutoVectorPtr<BYTE> szResponse;
		DWORD nResponseLength = DownloadFile(_T("RealWorld - media download"), COLE2CT(bstrServer.m_str+7), COLE2CT(szPath), szResponse);
		if (nResponseLength == 0)
			return S_FALSE;

		CComPtr<IConfigInMemory> pMemCfg;
		RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
		pMemCfg->DataBlockSet(nResponseLength, szResponse);

		CConfigValue cVal;
		CComBSTR bstrRootCfgID(CFGID_VIEWPROFILES);
		m_pMainConfig->ItemValueGet(bstrRootCfgID, &cVal);
		LONG nCount = cVal;
		OLECHAR szNameID[128];
		LONG i = 0;
		for (; i < nCount; ++i)
		{
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, i);
			CConfigValue cName;
			m_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cName);
			if (wcscmp(fileName, cName) == 0)
				break;
		}
		if (i == nCount)
		{
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, nCount);
			CComBSTR bstrProfileName(szNameID);
			CConfigValue cCount(nCount + 1L);
			BSTR aIDs[2];
			aIDs[0] = bstrRootCfgID;
			aIDs[1] = bstrProfileName;
			TConfigValue aVals[2];
			aVals[0] = cCount;
			aVals[1].eTypeID = ECVTString;
			aVals[1].bstrVal = fileName;
			m_pMainConfig->ItemValuesSet(2, aIDs, aVals);
		}
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, i);
		CComPtr<IConfig> pDstCfg;
		m_pMainConfig->SubConfigGet(CComBSTR(szNameID), &pDstCfg);
		if (SUCCEEDED(CopyConfigValues(pDstCfg, pMemCfg)))
			*res = VARIANT_TRUE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_autoUpdates(LONG* pVal)
{
	try
	{
		*pVal = 0;
		CComPtr<IGlobalConfigManager> pGCM;
		RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
		CComPtr<IConfig> pConfig;
		pGCM->Config(__uuidof(GlobalConfigMainFrame), &pConfig);
		CConfigValue cVal;
		pConfig->ItemValueGet(CComBSTR(CFGID_AUTOUPDATE), &cVal);
		*pVal = cVal;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::put_autoUpdates(LONG val)
{
	try
	{
		CComPtr<IGlobalConfigManager> pGCM;
		RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
		CComPtr<IConfig> pConfig;
		pGCM->Config(__uuidof(GlobalConfigMainFrame), &pConfig);
		CComBSTR bstr(CFGID_AUTOUPDATE);
		return pConfig->ItemValuesSet(1, &(bstr.m_str), CConfigValue(val));
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::UpdateNow()
{
	try
	{
		CComPtr<IAutoUpdater> pAU;
		RWCoCreateInstance(pAU, __uuidof(AutoUpdater));
		return pAU->CheckNow();
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::get_lastUpdateCheck(LONG* pVal)
{
	try
	{
		*pVal = 0;
		CComPtr<IGlobalConfigManager> pGCM;
		RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
		CComPtr<IConfig> pConfig;
		pGCM->Config(__uuidof(GlobalConfigMainFrame), &pConfig);
		CConfigValue cVal;
		pConfig->ItemValueGet(CComBSTR(CFGID_LASTCHECK), &cVal);
		*pVal = cVal;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageOnline::CExternal::ReloadPage()
{
	try
	{
		if (m_pOwner)
			m_pOwner->Refresh();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

