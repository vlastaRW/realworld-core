#pragma once

#include "resource.h"
#include "RWDesignerCore.h"

#include <Win32LangEx.h>
#include <ContextHelpDlg.h>
#include <DPIUtils.h>


template<LPCOLESTR t_pszCfgIDDlgSizeX, LPCOLESTR t_pszCfgIDDlgSizeY, LPCTSTR t_pszHelpTopic, UINT t_uCaptionTemplateID, HICON (*t_fnGetIcon)(ULONG)>
class CConfigFrameDlg :
	public Win32LangEx::CLangDialogImpl<CConfigFrameDlg<t_pszCfgIDDlgSizeX, t_pszCfgIDDlgSizeY, t_pszHelpTopic, t_uCaptionTemplateID, t_fnGetIcon> >,
	public CDialogResize<CConfigFrameDlg<t_pszCfgIDDlgSizeX, t_pszCfgIDDlgSizeY, t_pszHelpTopic, t_uCaptionTemplateID, t_fnGetIcon> >,
	public CContextHelpDlg<CConfigFrameDlg<t_pszCfgIDDlgSizeX, t_pszCfgIDDlgSizeY, t_pszHelpTopic, t_uCaptionTemplateID, t_fnGetIcon> >
{
public:
	CConfigFrameDlg(IConfig* a_pcConfig, IConfig* a_pGUIConfig, LCID a_tLocaleID, LPCTSTR a_pszItemName) :
		Win32LangEx::CLangDialogImpl<CConfigFrameDlg>(a_tLocaleID),
		CContextHelpDlg<CConfigFrameDlg>(t_pszHelpTopic), m_hIcon(NULL),
		m_pMainConfig(a_pcConfig), m_pGUIConfig(a_pGUIConfig), m_strName(a_pszItemName), m_bSaveConfig(false)
	{
		TCHAR szTempl[64] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), t_uCaptionTemplateID, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
		_sntprintf(m_szName, itemsof(m_szName), szTempl, a_pszItemName);
	}
	~CConfigFrameDlg()
	{
		if (m_hIcon) ::DestroyIcon(m_hIcon);
	}

	enum
	{
		IDD = IDD_CONFIGFRAME,
		IDC_CONFIGCONTROL = 1000
	};

	BEGIN_MSG_MAP(CConfigFrameDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CConfigFrameDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnEndDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnEndDialog)
		COMMAND_HANDLER(IDC_LS_SAVE, BN_CLICKED, OnClickedLayoutSave)
		CHAIN_MSG_MAP(CDialogResize<CConfigFrameDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigFrameDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CONFIGCONTROL, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_LS_NAME, DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_LS_SAVE, DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CConfigFrameDlg)
		CTXHELP_CONTROL_RESOURCE(IDOK, IDS_HELP_IDOK)
		CTXHELP_CONTROL_RESOURCE(IDCANCEL, IDS_HELP_IDCANCEL)
		CTXHELP_CONTROL_RESOURCE(IDHELP, IDS_HELP_IDHELP)
	END_CTXHELP_MAP()

	bool ShouldSaveConfig() const {return m_bSaveConfig;}
	LPCTSTR GetConfigName() const {return m_strName.c_str();}

protected:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		m_hIcon = t_fnGetIcon(GetSystemMetrics(SM_CXSMICON));//DPIUtils::PrepareIconForCaption((HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_uIconID), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
		// set icons
		if (m_hIcon != NULL)
			SetIcon(m_hIcon, FALSE);
		SetWindowText(m_szName);

		RECT rcButton;
		::GetWindowRect(GetDlgItem(IDCANCEL), &rcButton);
		ScreenToClient(&rcButton);
		RECT rcWnd;
		GetClientRect(&rcWnd);
		rcWnd.bottom = rcButton.top-rcWnd.bottom+rcButton.bottom;

		RWCoCreateInstance(m_pConfigWnd, __uuidof(TreeConfigWnd));
		m_pConfigWnd->Create(m_hWnd, &rcWnd, IDC_CONFIGCONTROL, m_tLocaleID, TRUE, ECWBMNothing);
		m_pConfigWnd->ConfigSet(m_pMainConfig, ECPMFull);

		IConfig* pMainConfig = reinterpret_cast<IConfig*>(a_lParam);
		if (pMainConfig)
		{
			CComboBox wndName(GetDlgItem(IDC_LS_NAME));

			CConfigValue cVal;
			pMainConfig->ItemValueGet(CComBSTR(CFGID_VIEWPROFILES), &cVal);
			LONG nLayouts = cVal;

			OLECHAR szNameID[64];
			LONG i;
			for (i = 0; i < nLayouts; i++)
			{
				_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, i);
				CConfigValue cName;
				pMainConfig->ItemValueGet(CComBSTR(szNameID), &cName);
				wndName.AddString(COLE2CT(cName));
			}

			wndName.SetWindowText(m_strName.c_str());
			wndName.ShowWindow(SW_SHOW);

			GetDlgItem(IDC_LS_SAVE).ShowWindow(SW_SHOW);
			CheckDlgButton(IDC_LS_SAVE, BST_CHECKED);
		}

		DlgResize_Init();

		if (m_pGUIConfig != 0)
		{
			CConfigValue cSizeX;
			m_pGUIConfig->ItemValueGet(CComBSTR(t_pszCfgIDDlgSizeX), &cSizeX);
			CConfigValue cSizeY;
			m_pGUIConfig->ItemValueGet(CComBSTR(t_pszCfgIDDlgSizeY), &cSizeY);
			if (cSizeX.operator LONG() != CW_USEDEFAULT && cSizeX.operator LONG() != CW_USEDEFAULT)
			{
				RECT rc;
				GetWindowRect(&rc);
				rc.right = rc.left + cSizeX.operator LONG();
				rc.bottom = rc.top + cSizeY.operator LONG();
				MoveWindow(&rc);
			}
		}

		CenterWindow(GetParent());

		return TRUE;
	}

	LRESULT OnEndDialog(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND UNREF(a_hWndCtl), BOOL& UNREF(bHandled))
	{
		if (m_pGUIConfig != 0)
		{
			RECT rc;
			GetWindowRect(&rc);
			BSTR aIDs[2];
			CComBSTR bstrIDX(t_pszCfgIDDlgSizeX);
			CComBSTR bstrIDY(t_pszCfgIDDlgSizeY);
			aIDs[0] = bstrIDX;
			aIDs[1] = bstrIDY;
			TConfigValue aVals[2];
			aVals[0] = CConfigValue(rc.right-rc.left);
			aVals[1] = CConfigValue(rc.bottom-rc.top);
			m_pGUIConfig->ItemValuesSet(2, aIDs, aVals);
		}
		m_bSaveConfig = IsDlgButtonChecked(IDC_LS_SAVE);
		TCHAR szTmp[256] = _T("");
		GetDlgItemText(IDC_LS_NAME, szTmp, itemsof(szTmp));
		m_strName = szTmp;
		EndDialog(a_wID);
		return 0;
	}
	LRESULT OnClickedLayoutSave(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(bHandled))
	{
		GetDlgItem(IDC_LS_NAME).EnableWindow(IsDlgButtonChecked(IDC_LS_SAVE));
		return 0;
	}

private:
	CComPtr<IConfig> m_pMainConfig;
	CComPtr<IConfig> m_pGUIConfig;
	CComPtr<IConfigWnd> m_pConfigWnd;
	TCHAR m_szName[256];
	std::tstring m_strName;
	bool m_bSaveConfig;
	HICON m_hIcon;
};

extern __declspec(selectany) const TCHAR HELPTOPIC_CONFIGURELAYOUT[] = _T("http://www.rw-designer.com/configure-layout");

#include "ConfigIDsApp.h"

HICON GetIconLayout(ULONG size);

typedef CConfigFrameDlg<CFGID_DLGS_CONFIGURELAYOUT_SIZEX, CFGID_DLGS_CONFIGURELAYOUT_SIZEY, HELPTOPIC_CONFIGURELAYOUT, IDS_CONFIGURELAYOUT, GetIconLayout> CConfigureLayoutDlg;
