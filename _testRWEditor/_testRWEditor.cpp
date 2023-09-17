// RWDesigner.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

#define RWAPPID L"_testRWEditor"
#define APP_MSGBOX_CAPTION _T("_testRWEditor")

#include <RWDesignerCore.h>
#include <RWLocalization.h>
#include <shlobj.h>
#include <XPGUI.h>
#include <MD5.h>
#include <RWCOMImpl.inl>


// Global Variables:
HINSTANCE hInst;								// current instance

#include <CommCtrlDependency.h>
#include <Win32LangEx.h>
#include <StringParsing.h>
#include <ContextHelpDlg.h>
#define AFX_RESOURCE_DLL
#include <RWVersionInfo.rc2>
#include <MultiLanguageString.h>

HRESULT AppLicenseBox(RWHWND a_hWndParent, LCID a_tLocaleID, IApplicationInfo* a_pAI)
{
	MessageBox(a_hWndParent, L"License Box", RWAPPID, MB_OK);
	return S_OK;
}

HRESULT AppAboutBox(RWHWND a_hWndParent, LCID a_tLocaleID, IApplicationInfo* a_pAI)
{
	MessageBox(a_hWndParent, L"About Box", RWAPPID, MB_OK);
	return S_OK;
}


class CApplicationInfo : public IClassFactory, public IApplicationInfo, public ILocalizedString
{
public:
	CApplicationInfo()
	{
		m_szUserFolder[0] = m_szAppFolder[0] = L'\0';
	}
	void Init()
	{
		GetModuleFileName(hInst, m_szAppFolder, itemsof(m_szAppFolder) - 10);
		GetLongPathName(m_szAppFolder, m_szAppFolder, itemsof(m_szAppFolder) - 10);
		m_szAppFolder[itemsof(m_szAppFolder) - 12] = L'\\';
		LPOLESTR pEnd = wcsrchr(m_szAppFolder, L'\\');
		*pEnd = L'\0';

		wcscpy(m_szUserFolder, m_szAppFolder);
		//wcscat(m_szUserFolder, L"\\AppData");
	}
	LPCOLESTR M_AppFolder() const { return m_szAppFolder; }
	LPCOLESTR M_UserFolder() const { return m_szUserFolder; }


	// IUnknown methods
public:
	STDMETHOD_(ULONG, AddRef)()
	{
		return 2;
	}
	STDMETHOD_(ULONG, Release)()
	{
		return 1;
	}
	STDMETHOD(QueryInterface)(REFIID a_iid, void** a_pp)
	{
		try
		{
			if (IsEqualIID(a_iid, __uuidof(IApplicationInfo)))
			{
				*a_pp = reinterpret_cast<void*>(static_cast<IApplicationInfo*>(this));
				return S_OK;
			}
			else if (IsEqualIID(a_iid, __uuidof(ILocalizedString)))
			{
				*a_pp = reinterpret_cast<void*>(static_cast<ILocalizedString*>(this));
				return S_OK;
			}
			else if (IsEqualIID(a_iid, __uuidof(IClassFactory)))
			{
				*a_pp = reinterpret_cast<void*>(static_cast<IClassFactory*>(this));
				return S_OK;
			}
			else if (IsEqualIID(a_iid, __uuidof(IUnknown)))
			{
				*a_pp = reinterpret_cast<void*>(static_cast<IUnknown*>(static_cast<IApplicationInfo*>(this)));
				return S_OK;
			}
			else
			{
				*a_pp = NULL;
				return E_NOINTERFACE;
			}
		}
		catch (...)
		{
			return E_POINTER;
		}
	}

	// IClassFactory methods
public:
	STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
	{
		if (pUnkOuter)
			return E_FAIL;
		return QueryInterface(riid, ppvObject);
	}
	STDMETHOD(LockServer)(BOOL fLock)
	{
		return S_OK;
	}

	// IDesignerAppInfo methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppName)
	{
		*a_ppName = this;
		AddRef();
		return S_OK;
	}
	STDMETHOD(Version)(ULONG* a_pVersion)
	{
		static int const iVer[4] = { RWVER0409_PRODUCTINFO };
		*a_pVersion = (iVer[0] << 16) | (iVer[1] << 8) | iVer[2];
		return S_OK;
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		*a_phIcon = NULL;
		*a_phIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
		return S_OK;
	}
	STDMETHOD(Identifier)(BSTR* a_pbstrAppID)
	{
		*a_pbstrAppID = NULL;
		*a_pbstrAppID = ::SysAllocString(RWAPPID);
		return S_OK;
	}

	STDMETHOD(Portable)()
	{
		return S_OK;
	}
	STDMETHOD(AppDataFolder)(BSTR* a_pbstrFolder)
	{
		*a_pbstrFolder = NULL;
		*a_pbstrFolder = ::SysAllocString(m_szUserFolder);
		return S_OK;
	}
	STDMETHOD(AppRootFolder)(BSTR* a_pbstrFolder)
	{
		*a_pbstrFolder = NULL;
		*a_pbstrFolder = ::SysAllocString(m_szAppFolder);
		return S_OK;
	}

	STDMETHOD(LicensingMode)(ELicensingMode* a_eLicensing)
	{
		if (a_eLicensing) *a_eLicensing = ELMDonate;
		return S_OK;
	}
	STDMETHOD(License)(BSTR* a_pbstrUserName, BSTR* a_pbstrOrganization, BSTR* a_pbstrSerial, ULONG* a_pAppLicCode, ULONG* a_pDaysInstalled)
	{
		try
		{
			if (a_pbstrUserName) *a_pbstrUserName = NULL;
			if (a_pbstrOrganization) *a_pbstrOrganization = NULL;
			if (a_pbstrSerial) *a_pbstrSerial = NULL;
			if (a_pAppLicCode) *a_pAppLicCode = 0;
			if (a_pDaysInstalled) *a_pDaysInstalled = 0;
			return S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Account)(BSTR* a_pbstrServer, BSTR* a_pbstrLogin, BSTR* a_pbstrPassHash)
	{
		try
		{
			if (a_pbstrServer) *a_pbstrServer = SysAllocString(L"http://www.rw-designer.com/");
			if (a_pbstrLogin) *a_pbstrLogin = NULL;
			if (a_pbstrPassHash) *a_pbstrPassHash = NULL;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(AboutBox)(RWHWND a_hWndParent, LCID a_tLocaleID)
	{
		try
		{
			return AppAboutBox(a_hWndParent, a_tLocaleID, this);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(LicenseBox)(RWHWND a_hWndParent, LCID a_tLocaleID)
	{
		try
		{
			return AppLicenseBox(a_hWndParent, a_tLocaleID, this);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AccountBox)(RWHWND a_hWndParent, LCID a_tLocaleID)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(AccountSet)(BSTR a_bstrLogin, BSTR a_bstrPassword)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(AccountHash)(BSTR a_bstrPassword, BSTR* a_pbstrHash)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(LicenseSet)(BSTR a_bstrName, BSTR a_bstrOrganization, BSTR a_bstrSerial)
	{
		return S_FALSE;
	}

	// ILocalizedString methods
public:
	STDMETHOD(Get)(BSTR* a_pbstrString)
	{
		return GetLocalized(GetThreadLocale(), a_pbstrString);
	}
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
	{
		if (a_pbstrString == NULL)
			return E_POINTER;
		*a_pbstrString = CComBSTR(APP_MSGBOX_CAPTION).Detach();
		return S_OK;
	}

private:
	OLECHAR m_szUserFolder[MAX_PATH];
	OLECHAR m_szAppFolder[MAX_PATH];
};


CApplicationInfo g_sAppInfo;
HRESULT __stdcall GetClassObjectAppInfo(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return g_sAppInfo.QueryInterface(riid, ppv);
}


class CDummyAtlModule : public CAtlModule
{
public:
	HRESULT AddCommonRGSReplacements(IRegistrarBase*) throw() { return S_OK; }
};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
	CDummyAtlModule cModule;


	if (FAILED(OleInitialize(NULL)))
		return 1;

	g_sAppInfo.Init();
	AddClass(&GetClassObjectAppInfo, __uuidof(ApplicationInfo), 0, NULL);

	CAutoVectorPtr<BYTE> pCfgData;
	ULONG nCfgLen = 0;
	OLECHAR szConfigPath[MAX_PATH];
	{
		// load configuration
		swprintf(szConfigPath, L"%s\\_testEditorConfig", g_sAppInfo.M_UserFolder());
		HANDLE h = CreateFile(szConfigPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD dw;
			DWORD dwLen = GetFileSize(h, &dw);
			if (dwLen)
			{
				pCfgData.Attach(new BYTE[dwLen]);
				dw = 0;
				if (ReadFile(h, pCfgData.m_p, dwLen, &dw, NULL) && dwLen == dw)
				{
					nCfgLen = dwLen;
				}
			}
			CloseHandle(h);
		}
	}

	{
		// load plug-ins
		OLECHAR szMaskApp[MAX_PATH];
		swprintf(szMaskApp, L"%s\\RW*.dll", g_sAppInfo.M_AppFolder());
		OLECHAR const* aSearchMasks[] = { szMaskApp, NULL };
		InitializePlugIns(aSearchMasks);
	}

	{
		// load or reset configuration
		CComPtr<IConfigInMemory> pCfgSrc;
		RWCoCreateInstance(pCfgSrc, __uuidof(ConfigInMemory));
		pCfgSrc->DataBlockSet(nCfgLen, pCfgData.m_p);

		// initialize translation service
		CComPtr<ITranslatorManager> pTrMan;
		RWCoCreateInstance(pTrMan, __uuidof(Translator));
		if (pTrMan)
			pTrMan->Initialize(&g_sAppInfo);

		CComPtr<IDesignerCore> pDesigner;
		RWCoCreateInstance(pDesigner, __uuidof(DesignerCore));
		// init application defaults
		pDesigner->Initialize(pCfgSrc, &g_sAppInfo, 0, NULL, GUID_NULL, NULL, EMTDefault);

		RWHWND hMainWnd = 0;
		pDesigner->NewWindowPage(GUID_NULL, &hMainWnd);

		ULONG nWaitResult = WAIT_TIMEOUT;
		pDesigner->MessagePump(0, NULL, &nWaitResult);

		pDesigner = NULL;
		if (pTrMan)
		{
			pTrMan->Finalize(TRUE);
			pTrMan = NULL;
		}
		ULONG dwLen = 0;
		pCfgSrc->DataBlockGetSize(&dwLen);
		if (dwLen)
		{
			CAutoVectorPtr<BYTE> pData(new BYTE[dwLen]);
			if (SUCCEEDED(pCfgSrc->DataBlockGet(dwLen, pData.m_p)))
			{
				TCHAR szConfigPath2[MAX_PATH];
				_tcscpy(szConfigPath2, szConfigPath);
				_tcscat(szConfigPath2, _T(".tmp"));
				HANDLE h = CreateFile(szConfigPath2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if (h != INVALID_HANDLE_VALUE)
				{
					DWORD dw = 0;
					WriteFile(h, pData.m_p, dwLen, &dw, NULL);
					CloseHandle(h);
					if (dwLen == dw)
					{
						DeleteFile(szConfigPath);
						MoveFile(szConfigPath2, szConfigPath);
					}
					else
					{
						DeleteFile(szConfigPath2);
					}
				}
			}
		}
	}

	ReleasePlugIns();

	OleUninitialize();

	return 0;
}
