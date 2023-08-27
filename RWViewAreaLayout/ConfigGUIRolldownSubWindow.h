#pragma once

#include "resource.h"
#include "RWViewAreaLayout.h"

#include <Win32LangEx.h>
#include <XPGUI.h>
#include <ConfigCustomGUIImpl.h>
#include <RWConceptDesignerExtension.h>


extern __declspec(selectany) GUID const TROLLDOWNSUBWINDOWCONFIGGUIID = {0x27f35747, 0x7684, 0x46c1, {0x90, 0xd6, 0x51, 0xad, 0x9c, 0x9b, 0x3c, 0xf9}};

class ATL_NO_VTABLE CConfigGUIRolldownSubWindow :
	public CCustomConfigWndImpl<CConfigGUIRolldownSubWindow>,
	public CDialogResize<CConfigGUIRolldownSubWindow>
{
public:
	CConfigGUIRolldownSubWindow()
	{
		RWCoCreateInstance(m_pIconsManager, __uuidof(DesignerFrameIconsManager));
	}
	~CConfigGUIRolldownSubWindow()
	{
		m_cIcons.Destroy();
	}

	enum { IDD = IDD_CONFIGGUI_RDWND };

	BEGIN_MSG_MAP(CConfigGUIRolldownSubWindow)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIRolldownSubWindow>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CGLAYOUT_ICON2, CBN_SELCHANGE, OnLayoutIconChanged)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIRolldownSubWindow>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIRolldownSubWindow)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_VIEW, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_ICON2, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RD_CUSTOMHEIGHT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RD_HEIGHT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RD_COLLAPSED, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_CFG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIRolldownSubWindow)
		CONFIGITEM_COMBOBOX(IDC_CGLAYOUT_VIEW, CFGID_RD_VIEWTYPE)
		CONFIGITEM_CHECKBOX(IDC_RD_CUSTOMHEIGHT, CFGID_RD_CUSTOMHEIGHT)
		CONFIGITEM_EDITBOX(IDC_RD_HEIGHT, CFGID_RD_HEIGHT)
		CONFIGITEM_CHECKBOX(IDC_RD_COLLAPSED, CFGID_RD_COLLAPSED)
	END_CONFIGITEM_MAP()

protected:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
	{
		m_wndOpIcon = GetDlgItem(IDC_CGLAYOUT_ICON2);

		// init icons combo
		CConfigValue cVal;
		CComBSTR cCFGID_ICONID(CFGID_PANELS_ICONID);
		M_Config()->ItemValueGet(cCFGID_ICONID, &cVal);
		CComPtr<IConfigItemOptions> pItem;
		M_Config()->ItemGetUIInfo(cCFGID_ICONID, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
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
		M_Config()->SubConfigGet(CComBSTR(CFGID_RD_VIEWTYPE), &pSubCfg);
		RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
		m_pConfigWnd->Create(m_hWnd, &rcCfg, IDC_CGLAYOUT_CFG, m_tLocaleID, TRUE, ECWBMMarginAndOutline);
		m_pConfigWnd->ConfigSet(pSubCfg, M_Mode());

		DlgResize_Init(false, false, 0);

		return TRUE;
	}
	LRESULT OnLayoutIconChanged(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		CComBSTR cCFGID_ICONID(CFGID_PANELS_ICONID);
		ULONG nIndex = m_wndOpIcon.GetCurSel();
		CConfigValue cVal;
		m_pIconOptions->Get(nIndex, &cVal);
		M_Config()->ItemValuesSet(1, &(cCFGID_ICONID.m_str), cVal);
		return 0;
	}

private:
	CComPtr<IConfigWnd> m_pConfigWnd;
	CComboBoxEx m_wndOpIcon;
	CImageList m_cIcons;
	CComPtr<IEnumConfigItemOptions> m_pIconOptions;
	CComPtr<IDesignerFrameIcons> m_pIconsManager;
};
