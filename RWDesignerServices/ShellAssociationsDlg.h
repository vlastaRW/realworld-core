#pragma once

#include "resource.h"
#include <RWInput.h>
#include <MultiLanguageString.h>
#include <Win32LangEx.h>
#include <ContextHelpDlg.h>


class CShellAssociationsDlg :
	public Win32LangEx::CLangIndirectDialogImpl<CShellAssociationsDlg>,
	public CDialogResize<CShellAssociationsDlg>,
	public CContextHelpDlg<CShellAssociationsDlg>
{
public:
	CShellAssociationsDlg(LCID a_tLocaleID, HICON a_hIcon);
	~CShellAssociationsDlg();

	enum
	{
		IDC_SHLASSOC_LIST = 120,
	};

	BEGIN_DIALOG_EX(0, 0, 266, 242, 0)
		DIALOG_CAPTION(_T("[0409]Manage File Associations[0405]Správa asociovaných typů souborů"))
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)
		DIALOG_EXSTYLE(WS_EX_CONTEXTHELP | WS_EX_NOPARENTNOTIFY)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Handled file types:[0405]Asociované typy souborů:"), IDC_STATIC, 7, 7, 100, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_SHLASSOC_LIST, WC_LISTVIEW, LVS_REPORT | LVS_SINGLESEL | LVS_ALIGNLEFT | LVS_NOSORTHEADER | WS_BORDER | WS_TABSTOP | WS_VISIBLE, 7, 19, 252, 195, 0)
		CONTROL_DEFPUSHBUTTON(_T("[0409]OK[0405]OK"), IDOK, 155, 221, 50, 14, WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Cancel[0405]Storno"), IDCANCEL, 209, 221, 50, 14, WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Help[0405]Nápověda"), IDHELP, 7, 221, 50, 14, WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CShellAssociationsDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CShellAssociationsDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CDialogResize<CShellAssociationsDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CShellAssociationsDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_SHLASSOC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CAppOptionsDlg)
		CTXHELP_CONTROL_STRING(IDOK, L"[0409]The OK button closes this dialog accepting the changes made[0405]Tlačítko OK zavře dialog a potvrdí všechny provedené změny.")
		CTXHELP_CONTROL_STRING(IDCANCEL, L"[0409]The Cancel button closes this dialog reverting the changes made[0405]Tlačítko Storno zavře dialog a zruší všechny provedené změny.")
		CTXHELP_CONTROL_STRING(IDHELP, L"[0409]The Help button displays more detailed help for this dialog[0405]Tlačítko Nápověda zobrazí detailní nápovědu pro tento dialog.")
		CTXHELP_CONTROL_STRING(IDC_SHLASSOC_LIST, L"[0409]This table shows file types supported by this application. Set a check mark to the left of a file type that should be associated in shell.[0405]Tato tabulka obsahuje typy souboru podporované touto aplikací. Zaškrtněte typy, které mají být asociovány v uživatelském rozhraní Windows.")
	END_CTXHELP_MAP()

protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
	HRESULT DeleteSubTree(HKEY a_hRootKey);

private:
	HICON m_hIcon;
	CComPtr<IInputManager> m_pInMgr;
	CListViewCtrl m_wndList;
};
