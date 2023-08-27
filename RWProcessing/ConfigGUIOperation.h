
#pragma once

#include <ConfigCustomGUIIcons.h>

class ATL_NO_VTABLE CConfigGUIOperationDlg :
	public CCustomConfigWndWithIcons<CConfigGUIOperationDlg>,
	public CDialogResize<CConfigGUIOperationDlg>
{
public:
	CConfigGUIOperationDlg() : m_wndShortcut(this, 1)
	{
	}

	enum { IDD = IDD_CONFIGGUI_OPERATION };

	BEGIN_MSG_MAP(CConfigGUIOperationDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIOperationDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIOperationDlg>)
	ALT_MSG_MAP(1)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnKeyDown)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIOperationDlg)
		DLGRESIZE_CONTROL(IDC_CG_NAME, DLSZ_MULDIVSIZE_X(2, 3))
		DLGRESIZE_CONTROL(IDC_CG_SHORTCUTLABEL, DLSZ_MULDIVMOVE_X(2, 3))
		DLGRESIZE_CONTROL(IDC_CG_SHORTCUT, DLSZ_MULDIVMOVE_X(2, 3)|DLSZ_MULDIVSIZE_X(1, 3))
		DLGRESIZE_CONTROL(IDC_CG_DESCRIPTION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_ICONLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_ICON, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPERATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIOperationDlg)
		CONFIGITEM_CONTEXTHELP(IDC_CG_SHORTCUT, CFGID_OPERATION_SHORTCUT)
		CONFIGITEM_EDITBOX(IDC_CG_NAME, CFGID_OPERATION_NAME)
		CONFIGITEM_EDITBOX(IDC_CG_DESCRIPTION, CFGID_OPERATION_DESCRITPION)
		CONFIGITEM_COMBOBOX(IDC_CG_OPERATION, CFGID_OPERATION_ID)
		CONFIGITEM_ICONCOMBO(IDC_CG_ICON, CFGID_OPERATION_ICONID)
		CONFIGITEM_SUBCONFIG(IDC_CG_OPCONFIG, CFGID_OPERATION_ID)
	END_CONFIGITEM_MAP()

	void ExtraConfigNotify()
	{
		CConfigValue cShortcut;
		M_Config()->ItemValueGet(CComBSTR(CFGID_OPERATION_SHORTCUT), &cShortcut);
		if (cShortcut.TypeGet() == ECVTInteger && m_wndShortcut.m_hWnd)
		{
			TCHAR szTmp[128] = _T("");
			if (cShortcut.operator LONG() == 0)
			{
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_NOSHORTCUT, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
			}
			else
			{
				TCHAR* p = szTmp;
				if (cShortcut.operator LONG()&(FALT<<16))
				{
					p += GetKeyNameText((MapVirtualKey(VK_MENU, 0)<<16)|0x2000000, p, 31);
					_tcscpy(p, _T("+"));
					p += 1;
				}
				if (cShortcut.operator LONG()&(FCONTROL<<16))
				{
					p += GetKeyNameText((MapVirtualKey(VK_CONTROL, 0)<<16)|0x2000000, p, 31);
					_tcscpy(p, _T("+"));
					p += 1;
				}
				if (cShortcut.operator LONG()&(FSHIFT<<16))
				{
					p += GetKeyNameText((MapVirtualKey(VK_SHIFT, 0)<<16)|0x2000000, p, 31);
					_tcscpy(p, _T("+"));
					p += 1;
				}
				UINT scancode = MapVirtualKey(cShortcut.operator LONG()&0xffff, 0);
				switch(cShortcut.operator LONG()&0xffff)
				{
				case VK_INSERT:
				case VK_DELETE:
				case VK_HOME:
				case VK_END:
				case VK_NEXT:  // Page down
				case VK_PRIOR: // Page up
				case VK_LEFT:
				case VK_RIGHT:
				case VK_UP:
				case VK_DOWN:
					scancode |= 0x100; // Add extended bit
				}
				GetKeyNameText((scancode<<16)|0x2000000, p, 31);
				szTmp[itemsof(szTmp)-1] = _T('\0');
			}
			m_wndShortcut.SetWindowText(szTmp);
		}
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndShortcut.SubclassWindow(GetDlgItem(IDC_CG_SHORTCUT));

		BOOL b;
		CCustomConfigWndImpl<CConfigGUIOperationDlg>::OnInitDialog(a_uMsg, a_wParam, a_lParam, b);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		try
		{
			CComBSTR bstr(CFGID_OPERATION_SHORTCUT);
			if (a_wParam != VK_MENU && a_wParam != VK_SHIFT && a_wParam != VK_CONTROL &&
				a_wParam != VK_LMENU && a_wParam != VK_LSHIFT && a_wParam != VK_LCONTROL &&
				a_wParam != VK_RMENU && a_wParam != VK_RSHIFT && a_wParam != VK_RCONTROL)
				M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(LONG(a_wParam|
					((GetKeyState(VK_MENU)&0x8000 ? FALT : 0) | (GetKeyState(VK_CONTROL)&0x8000 ? FCONTROL : 0) | (GetKeyState(VK_SHIFT)&0x8000 ? FSHIFT : 0))<<16)));
			else
				M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(0L));
		}
		catch (...)
		{
		}
		a_bHandled = FALSE;
		return 0;
	}

private:
	CContainedWindowT<CEdit> m_wndShortcut;
};
