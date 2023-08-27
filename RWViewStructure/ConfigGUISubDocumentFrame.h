
#pragma once

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUISubDocumentFrameDlg :
	public CCustomConfigWndImpl<CConfigGUISubDocumentFrameDlg>,
	public CDialogResize<CConfigGUISubDocumentFrameDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_SUBDOCFRAME };

	BEGIN_MSG_MAP(CConfigGUISubDocumentFrameDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISubDocumentFrameDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUISubDocumentFrameDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISubDocumentFrameDlg)
		DLGRESIZE_CONTROL(IDC_CGSUBDOCFRM_VIEWTYPE, DLSZ_DIVSIZE_X(2))
		//DLGRESIZE_CONTROL(IDC_CGSUBDOCFRM_CACHEVIEWS, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGSUBDOCFRM_SYNCGROUP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGSUBDOCFRM_EMPTYMSG, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGSUBDOCFRM_SUBCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUISubDocumentFrameDlg)
		CONFIGITEM_EDITBOX(IDC_CGSUBDOCFRM_SYNCGROUP, CFGID_FRM_SELSYNCGROUP)
		CONFIGITEM_COMBOBOX(IDC_CGSUBDOCFRM_VIEWTYPE, CFGID_FRM_SUBVIEW)
		//CONFIGITEM_CHECKBOX(IDC_CGSUBDOCFRM_CACHEVIEWS, CFGID_FRM_CACHEVIEWS)
		CONFIGITEM_EDITBOX(IDC_CGSUBDOCFRM_EMPTYMSG, CFGID_FRM_NOVIEWMSG)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CWindow wnd(GetDlgItem(IDC_CGSUBDOCFRM_SUBCONFIG));
		RECT rc;
		wnd.GetWindowRect(&rc);
		ScreenToClient(&rc);
		wnd.DestroyWindow();

		RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
		m_pConfigWnd->Create(m_hWnd, &rc, IDC_CGSUBDOCFRM_SUBCONFIG, m_tLocaleID, TRUE, ECWBMMarginAndOutline);
		CComPtr<IConfig> pDeviceCfg;
		M_Config()->SubConfigGet(CComBSTR(CFGID_FRM_SUBVIEW), &pDeviceCfg);
		m_pConfigWnd->ConfigSet(pDeviceCfg, M_Mode());

		DlgResize_Init(false, false, 0);

		return 1;
	}

private:
	CComPtr<IConfigWnd> m_pConfigWnd;
};
