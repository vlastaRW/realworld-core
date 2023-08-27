
#pragma once

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUITreeDlg :
	public CCustomConfigWndImpl<CConfigGUITreeDlg>,
	public CDialogResize<CConfigGUITreeDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_TREE };

	BEGIN_MSG_MAP(CConfigGUITreeDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUITreeDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUITreeDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUITreeDlg)
		DLGRESIZE_CONTROL(IDC_CGTREE_SELECTION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGTREE_ONKEYDOWN, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGTREE_CONTEXTMENU, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGTREE_SUBCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUITreeDlg)
		CONFIGITEM_EDITBOX(IDC_CGTREE_SELECTION, CFGID_TREE_SELSYNCGROUP)
		CONFIGITEM_CHECKBOX(IDC_CGTREE_LINES, CFGID_TREE_VIEWLINES)
		CONFIGITEM_CHECKBOX(IDC_CGTREE_VISONKS, CFGID_TREE_SHOWITEMONFOCUS)
		CONFIGITEM_COMBOBOX(IDC_CGTREE_VIEWMODE, CFGID_TREE_VIEWMODE)
		CONFIGITEM_COMBOBOX(IDC_CGTREE_ONKEYDOWN, CFGID_TREE_KEYDOWNACTION)
		CONFIGITEM_COMBOBOX(IDC_CGTREE_CONTEXTMENU, CFGID_TREE_CONTEXTMENU)
		CONFIGITEM_SUBCONFIG(IDC_CGTREE_SUBCONFIG, CFGID_TREE_CONTEXTMENU)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};
