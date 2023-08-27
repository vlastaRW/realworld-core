#pragma once

#include "resource.h"
#include "RWViewAreaLayout.h"

#include <Win32LangEx.h>
#include <XPGUI.h>
#include <ConfigCustomGUIImpl.h>
#include <RWConceptDesignerExtension.h>


extern __declspec(selectany) GUID const TTABSUBWINDOWCONFIGGUIID = {0x1f7d9bfc, 0xaecc, 0x4e37, {0x96, 0x3d, 0xb1, 0xad, 0xc9, 0x48, 0x21, 0xd5}};

class ATL_NO_VTABLE CConfigGUITabSubWindow :
	public CCustomConfigWndImpl<CConfigGUITabSubWindow>,
	public CDialogResize<CConfigGUITabSubWindow>
{
public:
	CConfigGUITabSubWindow()
	{
		RWCoCreateInstance(m_pIconsManager, __uuidof(DesignerFrameIconsManager));
	}
	~CConfigGUITabSubWindow()
	{
		m_cIcons.Destroy();
	}

	enum { IDD = IDD_CONFIGGUI_TABWND };

	BEGIN_MSG_MAP(CConfigGUITabSubWindow)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUITabSubWindow>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CGLAYOUT_ICON, CBN_SELCHANGE, OnLayoutIconChanged)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUITabSubWindow>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUITabSubWindow)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_VIEW, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_CONDITION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_ICON, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_ICONLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_CFG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUITabSubWindow)
		CONFIGITEM_EDITBOX(IDC_CGLAYOUT_CONDITION, CFGID_TABS_CONDITION)
		CONFIGITEM_COMBOBOX(IDC_CGLAYOUT_VIEW, CFGID_TABS_VIEW)
	END_CONFIGITEM_MAP()

protected:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
	{
		m_wndOpIcon = GetDlgItem(IDC_CGLAYOUT_ICON);

		// init icons combo
		CConfigValue cVal;
		CComBSTR cCFGID_TABS_ICONID(CFGID_TABS_ICONID);
		M_Config()->ItemValueGet(cCFGID_TABS_ICONID, &cVal);
		CComPtr<IConfigItemOptions> pItem;
		M_Config()->ItemGetUIInfo(cCFGID_TABS_ICONID, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
		if (pItem != NULL)
		{
			pItem->OptionsEnum(&m_pIconOptions);

			ULONG nIcons = 0;
			m_cIcons.Create(16, 16, XPGUI::GetImageListColorFlags(), 4, 4);
			m_wndOpIcon.SetImageList(m_cIcons);

			CConfigValue cOption;
			for (ULONG i = 0; SUCCEEDED(m_pIconOptions->Get(i, &cOption)); i++)
			{
				HICON hIcon = NULL;
				m_pIconsManager->GetIcon(cOption, 16, &hIcon);
				if (hIcon != NULL)
				{
					m_cIcons.AddIcon(hIcon);
					DestroyIcon(hIcon);
					nIcons++;
				}
				COMBOBOXEXITEM tItem;
				ZeroMemory(&tItem, sizeof tItem);
				tItem.mask = CBEIF_IMAGE|CBEIF_SELECTEDIMAGE|CBEIF_TEXT;
				tItem.iItem = i;
				tItem.pszText = _T("                                     ");
				tItem.iImage = tItem.iSelectedImage = hIcon ? nIcons-1 : -1;
				m_wndOpIcon.InsertItem(&tItem);
				if (cVal == cOption)
				{
					m_wndOpIcon.SetCurSel(i);
				}
			}
		}
		else
		{
			m_wndOpIcon.EnableWindow(FALSE);
		}

		// create subwindow
		RECT rcCfg;
		HWND hCfg = GetDlgItem(IDC_CGLAYOUT_CFG);
		::GetWindowRect(hCfg, &rcCfg);
		::DestroyWindow(hCfg);
		ScreenToClient(&rcCfg);
		CComPtr<IConfig> pSubCfg;
		M_Config()->SubConfigGet(CComBSTR(CFGID_TABS_VIEW), &pSubCfg);
		RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
		m_pConfigWnd->Create(m_hWnd, &rcCfg, IDC_CGLAYOUT_CFG, m_tLocaleID, TRUE, ECWBMMarginAndOutline);
		m_pConfigWnd->ConfigSet(pSubCfg, M_Mode());

		DlgResize_Init(false, false, 0);

		return TRUE;
	}
	LRESULT OnLayoutIconChanged(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		CComBSTR cCFGID_TABS_ICONID(CFGID_TABS_ICONID);
		ULONG nIndex = m_wndOpIcon.GetCurSel();
		CConfigValue cVal;
		m_pIconOptions->Get(nIndex, &cVal);
		M_Config()->ItemValuesSet(1, &(cCFGID_TABS_ICONID.m_str), cVal);
		return 0;
	}

private:
	CComPtr<IConfigWnd> m_pConfigWnd;
	CComboBoxEx m_wndOpIcon;
	CImageList m_cIcons;
	CComPtr<IEnumConfigItemOptions> m_pIconOptions;
	CComPtr<IDesignerFrameIcons> m_pIconsManager;
};
