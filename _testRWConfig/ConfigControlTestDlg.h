// DeviceListDlg.h : interface of the CConfigControlTestDlg class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEVICELISTDLG_H__09045258_F08B_40DA_886D_535DC527F508__INCLUDED_)
#define AFX_DEVICELISTDLG_H__09045258_F08B_40DA_886D_535DC527F508__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "_testUtils.h"

class CConfigControlTestDlg :
	public CAxDialogImpl<CConfigControlTestDlg>,
	public CDialogResize<CConfigControlTestDlg>
{
public:
	enum { IDD = IDD_CONFIGCONTROLTEST };

	CConfigControlTestDlg()
	{
	}

private:
	BEGIN_MSG_MAP(CConfigControlTestDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnEnd)
		COMMAND_ID_HANDLER(IDCANCEL, OnEnd)
		COMMAND_HANDLER(IDC_BORDER, BN_CLICKED, OnBorder)
		COMMAND_HANDLER(IDC_BUTTON1, BN_CLICKED, OnBnClickedButton1)
		COMMAND_HANDLER(IDC_BUTTON2, BN_CLICKED, OnBnClickedButton2)
		COMMAND_HANDLER(IDC_BUTTON3, BN_CLICKED, OnBnClickedButton3)
		CHAIN_MSG_MAP(CDialogResize<CConfigControlTestDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigControlTestDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_BUTTON1, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_BUTTON2, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_BUTTON3, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_BORDER, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CLTR1, DLSZ_DIVSIZE_X(2)|DLSZ_DIVSIZE_Y(2))
		DLGRESIZE_CONTROL(IDC_CLTR2, DLSZ_DIVSIZE_X(2)|DLSZ_DIVSIZE_Y(2)|DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CLTR3, DLSZ_DIVSIZE_X(2)|DLSZ_DIVSIZE_Y(2)|DLSZ_DIVMOVE_Y(2))
		DLGRESIZE_CONTROL(IDC_CLTR4, DLSZ_DIVSIZE_X(2)|DLSZ_DIVSIZE_Y(2)|DLSZ_DIVMOVE_X(2)|DLSZ_DIVMOVE_Y(2))
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		m_pCfg.Attach(CreateCompositeConfig());

		SetDlgItemText(IDC_DEFAULTS_EDIT, _T("IconEditor"));
		SetDlgItemText(IDC_VERSION_EDIT, _T("2008.1"));
		BOOL b;
		OnBorder(0, 0, 0, b);

		DlgResize_Init(true, false);

		return TRUE;
	}


	LRESULT OnEnd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnBorder(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		RECT rc1;
		CWindow wnd1(GetDlgItem(IDC_CLTR1));
		wnd1.GetWindowRect(&rc1);
		wnd1.DestroyWindow();
		ScreenToClient(&rc1);
		m_pCfg1 = NULL;

		RECT rc2;
		CWindow wnd2(GetDlgItem(IDC_CLTR2));
		wnd2.GetWindowRect(&rc2);
		wnd2.DestroyWindow();
		ScreenToClient(&rc2);
		m_pCfg2 = NULL;

		RECT rc3;
		CWindow wnd3(GetDlgItem(IDC_CLTR3));
		wnd3.GetWindowRect(&rc3);
		wnd3.DestroyWindow();
		ScreenToClient(&rc3);
		m_pCfg3 = NULL;

		RECT rc4;
		CWindow wnd4(GetDlgItem(IDC_CLTR4));
		wnd4.GetWindowRect(&rc4);
		wnd4.DestroyWindow();
		ScreenToClient(&rc4);
		m_pCfg4 = NULL;

		BOOL bBorder = IsDlgButtonChecked(IDC_BORDER);

		RWCoCreateInstance(m_pCfg1, __uuidof(TreeConfigWnd));
		m_pCfg1->Create(m_hWnd, &rc1, IDC_CLTR1, GetThreadLocale(), TRUE, bBorder ? ECWBMMarginAndOutline : ECWBMMargin);
		m_pCfg1->ConfigSet(m_pCfg, ECPMFull);

		RWCoCreateInstance(m_pCfg2, __uuidof(MiniConfigWnd));
		m_pCfg2->Create(m_hWnd, &rc2, IDC_CLTR2, GetThreadLocale(), TRUE, bBorder ? ECWBMMarginAndOutline : ECWBMMargin);
		m_pCfg2->ConfigSet(m_pCfg, ECPMFull);

		RWCoCreateInstance(m_pCfg3, __uuidof(FullConfigWnd));
		m_pCfg3->Create(m_hWnd, &rc3, IDC_CLTR3, GetThreadLocale(), TRUE, bBorder ? ECWBMMarginAndOutline : ECWBMMargin);
		m_pCfg3->ConfigSet(m_pCfg, ECPMFull);

		RWCoCreateInstance(m_pCfg4, __uuidof(AutoConfigWnd));
		m_pCfg4->Create(m_hWnd, &rc4, IDC_CLTR4, GetThreadLocale(), TRUE, bBorder ? ECWBMMarginAndOutline : ECWBMMargin);
		m_pCfg4->ConfigSet(m_pCfg, ECPMFull);

		return 0;
	}

private:
	CComPtr<IConfigWnd> m_pCfg1;
	CComPtr<IConfigWnd> m_pCfg2;
	CComPtr<IConfigWnd> m_pCfg3;
	CComPtr<IConfigWnd> m_pCfg4;
	CComPtr<IConfig> m_pCfg;

public:
	LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		//CComPtr<IConfigInRegistry> pReg;
		//RWCoCreateInstance(pReg, __uuidof(ConfigInRegistry));
		//TCHAR szApp[64] = _T("");
		//GetDlgItemText(IDC_DEFAULTS_EDIT, szApp, 64);
		//CComBSTR bstrRoot(L"Software\\RealWorld\\");
		//bstrRoot += szApp;
		//bstrRoot += L"\\Designer";
		//pReg->RegistryKeySet(ERKRCurrentUser, bstrRoot);
		CComPtr<IConfigInMemory> pReg;
		RWCoCreateInstance(pReg, __uuidof(ConfigInMemory));
		TCHAR szApp[64] = _T("");
		GetDlgItemText(IDC_DEFAULTS_EDIT, szApp, 64);
		TCHAR szVer[64] = _T("");
		GetDlgItemText(IDC_VERSION_EDIT, szVer, 64);
		TCHAR szAppData[MAX_PATH];
		SHGetSpecialFolderPath(NULL, szAppData, CSIDL_APPDATA, FALSE);
		TCHAR szTmp[MAX_PATH];
		_stprintf(szTmp, _T("RealWorld\\RW%s\\Config.%s"), szApp, szVer);
		PathAppend(szAppData, szTmp);
		HANDLE hSrc = CreateFile(szAppData, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (hSrc == INVALID_HANDLE_VALUE)
			return 0;
		DWORD dummy;
		ULONG nSizeSrc = GetFileSize(hSrc, &dummy);
		CAutoVectorPtr<BYTE> pSrcData(new BYTE[nSizeSrc]);
		ReadFile(hSrc, pSrcData.m_p, nSizeSrc, &dummy, NULL);
		CloseHandle(hSrc);
		pReg->DataBlockSet(nSizeSrc, pSrcData);

		CComPtr<IConfigInMemory> pMem;
		RWCoCreateInstance(pMem, __uuidof(ConfigInMemory));
		LPCOLESTR aFragmets[] =
		{
			L"Layouts", // layouts
			L"BatchOpsPhoto", // photos batch operations
			L"BatchOperation", // icon editor batch operations
			L"Globals\\ADE9631E-DFCC-4D98-88D7-A1A3B80DBD67", // raster image globals
			L"Globals\\2CBE06C7-4847-4766-AA01-226AF52D5488", // color globals
			L"Globals\\4ED2E209-CB79-4849-9175-BFCC033132E1", // favorite formats
			NULL
		};
		for (LPCOLESTR* pFrg = aFragmets; *pFrg; ++pFrg)
		{
			CComBSTR bstrViewProfiles(*pFrg);
			CConfigValue cViewProfiles;
			pReg->ItemValueGet(bstrViewProfiles, &cViewProfiles);
			if (cViewProfiles.TypeGet() == ECVTEmpty)
				continue;
			pMem->ItemValuesSet(1, &(bstrViewProfiles.m_str), cViewProfiles);
			CComPtr<IConfig> pRegSub;
			pReg->SubConfigGet(bstrViewProfiles, &pRegSub);
			CComPtr<IConfig> pMemSub;
			pMem->SubConfigGet(bstrViewProfiles, &pMemSub);
			CopyConfigValues(pMemSub, pRegSub);
		}
		ULONG nSize;
		pMem->DataBlockGetSize(&nSize);
		BYTE* pData(new BYTE[nSize]);
		pMem->DataBlockGet(nSize, pData);
		TCHAR szPath[MAX_PATH] = _T("");
		SHGetSpecialFolderPath(m_hWnd, szPath, CSIDL_DESKTOP, FALSE);
		size_t nPath = _tcslen(szPath);
		if (szPath[nPath-1] != _T('\\'))
			szPath[nPath++] = _T('\\');
		_tcscpy(szPath+nPath, _T("Defaults.bin"));
		HANDLE h = CreateFile(szPath, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (h == (HANDLE)-1)
			h = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		DWORD n;
		WriteFile(h, pData, nSize, &n, NULL);
		CloseHandle(h);
		delete[] pData;
		return 0;
	}

	class CClipboardHandler
	{
	public:
		CClipboardHandler(HWND a_hWnd)
		{
			if (!::OpenClipboard(a_hWnd))
				throw E_FAIL;
		}
		~CClipboardHandler()
		{
			::CloseClipboard();
		}
	};
	LRESULT OnBnClickedButton2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		UINT m_nClipboardFormat = RegisterClipboardFormat(_T("RWCONFIG"));

		CClipboardHandler cClipboard(*this);

		// try internal format first
		HANDLE hMem = GetClipboardData(m_nClipboardFormat);
		if (hMem == NULL)
			return 0;

		ULONG const* pMem = reinterpret_cast<ULONG const*>(GlobalLock(hMem));

		TCHAR szPath[MAX_PATH] = _T("");
		SHGetSpecialFolderPath(m_hWnd, szPath, CSIDL_DESKTOP, FALSE);
		size_t nPath = _tcslen(szPath);
		if (szPath[nPath-1] != _T('\\'))
			szPath[nPath++] = _T('\\');
		_tcscpy(szPath+nPath, _T("FromClipboard.bin"));
		HANDLE h = CreateFile(szPath, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (h == (HANDLE)-1)
			h = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		DWORD n;
		WriteFile(h, pMem+1, *pMem, &n, NULL);
		CloseHandle(h);

		GlobalUnlock(hMem);

		return 0;
	}

	LRESULT OnBnClickedButton3(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CComPtr<IConfigInMemory> pReg;
		RWCoCreateInstance(pReg, __uuidof(ConfigInMemory));
		TCHAR szApp[64] = _T("");
		GetDlgItemText(IDC_DEFAULTS_EDIT, szApp, 64);
		TCHAR szVer[64] = _T("");
		GetDlgItemText(IDC_VERSION_EDIT, szVer, 64);
		TCHAR szAppData[MAX_PATH];
		SHGetSpecialFolderPath(NULL, szAppData, CSIDL_APPDATA, FALSE);
		TCHAR szTmp[MAX_PATH];
		_stprintf(szTmp, _T("RealWorld\\RW%s\\Config.%s"), szApp, szVer);
		PathAppend(szAppData, szTmp);
		HANDLE hSrc = CreateFile(szAppData, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (hSrc == INVALID_HANDLE_VALUE)
			return 0;
		DWORD dummy;
		ULONG nSizeSrc = GetFileSize(hSrc, &dummy);
		CAutoVectorPtr<BYTE> pSrcData(new BYTE[nSizeSrc]);
		ReadFile(hSrc, pSrcData.m_p, nSizeSrc, &dummy, NULL);
		CloseHandle(hSrc);
		pReg->DataBlockSet(nSizeSrc, pSrcData);

		//CComPtr<IConfigInRegistry> pRegCfg;
		//RWCoCreateInstance(pRegCfg, __uuidof(ConfigInRegistry));
		//pRegCfg->RegistryKeySet(ERKRCurrentUser, CComBSTR(L"Software\\RealWorld\\IconEditor\\Designer"));
		m_pCfg1->ConfigSet(pReg, ECPMFull);
		m_pCfg2->ConfigSet(pReg, ECPMFull);
		m_pCfg3->ConfigSet(pReg, ECPMFull);
		m_pCfg4->ConfigSet(pReg, ECPMFull);

		return 0;
	}
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVICELISTDLG_H__09045258_F08B_40DA_886D_535DC527F508__INCLUDED_)
