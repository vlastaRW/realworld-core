
#pragma once

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIVectorItemDlg :
	public CCustomConfigWndImpl<CConfigGUIVectorItemDlg>,
	public CDialogResize<CConfigGUIVectorItemDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_VECTORITEM };

	BEGIN_MSG_MAP(CConfigGUIVectorItemDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIVectorItemDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIVectorItemDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIVectorItemDlg)
		DLGRESIZE_CONTROL(IDC_CG_OPERATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIVectorItemDlg)
		CONFIGITEM_COMBOBOX(IDC_CG_OPERATION, CFGID_VECTOR_SUBCOMMANDS)
		CONFIGITEM_SUBCONFIG(IDC_CG_OPCONFIG, CFGID_VECTOR_SUBCOMMANDS)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};
