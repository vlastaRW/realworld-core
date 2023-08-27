
#pragma once

#include <ConfigCustomGUIIcons.h>
#include <ConfigCustomGUIML.h>

extern __declspec(selectany) GUID const ConfigIDLayout = {0x940e4b2f, 0x1c5c, 0x4972, {0xbe, 0xc1, 0x8d, 0xda, 0xbb, 0xca, 0x1d, 0x32}};

class ATL_NO_VTABLE CConfigGUILayoutDlg :
	public CCustomConfigWndMultiLang<CConfigGUILayoutDlg, CCustomConfigWndWithIcons<CConfigGUILayoutDlg> >,
	public CDialogResize<CConfigGUILayoutDlg>
{
public:
	CConfigGUILayoutDlg() :
		CCustomConfigWndMultiLang<CConfigGUILayoutDlg, CCustomConfigWndWithIcons<CConfigGUILayoutDlg> >(CFGID_LAYOUTNAME, CFGID_LAYOUTVERB)
	{
	}

	enum { IDD = IDD_CONFIGGUI_LAYOUT, IDC_CGLAYOUT_TAB = 99 };

	BEGIN_MSG_MAP(CConfigGUILayoutDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUILayoutDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUILayoutDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGLAYOUT_TAB, CTCN_SELCHANGE, OnTabChange)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUILayoutDlg)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_TAB, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_VIEWCFG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_CMDSCFG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_ICONLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_ICON, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_VIEW, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_COMMANDS, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_NAME, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGLAYOUT_VERB, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUILayoutDlg)
		CONFIGITEM_COMBOBOX(IDC_CGLAYOUT_VIEW, CFGID_DESIGNERVIEW)
		CONFIGITEM_COMBOBOX(IDC_CGLAYOUT_COMMANDS, CFGID_MENUCOMMANDS)
		CONFIGITEM_ICONCOMBO(IDC_CGLAYOUT_ICON, CFGID_ICONID)
		CONFIGITEM_SUBCONFIG(IDC_CGLAYOUT_VIEWCFG, CFGID_DESIGNERVIEW)
		CONFIGITEM_SUBCONFIG(IDC_CGLAYOUT_CMDSCFG, CFGID_MENUCOMMANDS)
		CONFIGITEM_EDITBOX(IDC_CGLAYOUT_NAME, CFGID_LAYOUTNAME)
		CONFIGITEM_EDITBOX(IDC_CGLAYOUT_VERB, CFGID_LAYOUTVERB)
		CONFIGITEM_COMBOBOX(IDC_CGLAYOUT_DOCTYPE, CFGID_DOCBUILDER)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		RECT rcTab = {0, 0, 281, 12};
		MapDialogRect(&rcTab);
		m_wndTab.Create(m_hWnd, rcTab, _T("Tab"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_NOEDGE, 0, IDC_CGLAYOUT_TAB);

		TCHAR szColumn[64] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_LAYOUTCFG_COLVIEW, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
		m_wndTab.InsertItem(0, szColumn);
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_LAYOUTCFG_COLOPS, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
		m_wndTab.InsertItem(1, szColumn);
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_LAYOUTCFG_COLDESC, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
		m_wndTab.InsertItem(2, szColumn);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnTabChange(int UNREF(a_idCtrl), LPNMHDR a_pnmh, BOOL& UNREF(a_bHandled))
	{
		BOOL b1 = m_wndTab.GetCurSel() == 0;
		BOOL b2 = m_wndTab.GetCurSel() == 1;
		BOOL b3 = m_wndTab.GetCurSel() == 2;
		GetDlgItem(IDC_CGLAYOUT_VIEWCFG).ShowWindow(b1);
		GetDlgItem(IDC_CGLAYOUT_VIEWLABEL).ShowWindow(b1);
		GetDlgItem(IDC_CGLAYOUT_VIEW).ShowWindow(b1);
		GetDlgItem(IDC_CGLAYOUT_COMMANDSLABEL).ShowWindow(b2);
		GetDlgItem(IDC_CGLAYOUT_COMMANDS).ShowWindow(b2);
		GetDlgItem(IDC_CGLAYOUT_CMDSCFG).ShowWindow(b2);
		GetDlgItem(IDC_CGLAYOUT_ICONLABEL).ShowWindow(b3);
		GetDlgItem(IDC_CGLAYOUT_ICON).ShowWindow(b3);
		GetDlgItem(IDC_CGLAYOUT_NAME_LABEL).ShowWindow(b3);
		GetDlgItem(IDC_CGLAYOUT_NAME).ShowWindow(b3);
		GetDlgItem(IDC_CGLAYOUT_VERB_LABEL).ShowWindow(b3);
		GetDlgItem(IDC_CGLAYOUT_VERB).ShowWindow(b3);
		GetDlgItem(IDC_CGLAYOUT_DOCTYPE_LABEL).ShowWindow(b3);
		GetDlgItem(IDC_CGLAYOUT_DOCTYPE).ShowWindow(b3);

		return 0;
	}

private:
	CDotNetTabCtrl<CCustomTabItem> m_wndTab;
};

/*
#pragma once

#include "resource.h"
#include "RWDesignerCore.h"

#include <Win32LangEx.h>
#include "ConfigGUILayoutView.h"
#include "ConfigGUILayoutOps.h"
#include "ConfigGUIMenuCommands.h"


class ATL_NO_VTABLE CConfigGUILayout :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigCustomGUI
{
public:
	void Init(IDesignerFrameIcons* a_pIconsManager)
	{
		m_pIconsManager = a_pIconsManager;
	}

	BEGIN_COM_MAP(CConfigGUILayout)
		COM_INTERFACE_ENTRY(IConfigCustomGUI)
	END_COM_MAP()

	// IConfigCustomGUI methods
public:
	STDMETHOD(UIDGet)(GUID* a_pguid);
	STDMETHOD(RequiresMargins)();
	STDMETHOD(MinSizeGet)(LCID a_tLocaleID, ULONG* a_nSizeX, ULONG* a_nSizeY);
	STDMETHOD(WindowCreate)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorders, IConfig* a_pConfig, IChildWindow** a_ppWindow);

private:
	class ATL_NO_VTABLE CConfigWnd :
		public CComObjectRootEx<CComMultiThreadModel>,
		public CChildWindowImpl<CConfigWnd, IChildWindow>,
		public Win32LangEx::CLangDialogImpl<CConfigWnd>,
		public CDialogResize<CConfigWnd>
	{
	public:
		void Create(HWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, IConfig* a_pConfig, IDesignerFrameIcons* a_pIconsManager);

		BEGIN_COM_MAP(CConfigWnd)
			COM_INTERFACE_ENTRY(IChildWindow)
		END_COM_MAP()

		enum
		{
			IDD = IDD_CONFIGGUI_LAYOUT,
			IDC_SUBWND_VIEW = 0,
			IDC_SUBWND_OPS,
			IDC_SUBWND_CMDS,
			IDC_CGLAYOUT_TAB = 213
		};

		BEGIN_MSG_MAP(CConfigWnd)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			NOTIFY_HANDLER(IDC_CGLAYOUT_TAB, CTCN_SELCHANGE, OnTabChange)
			CHAIN_MSG_MAP(CDialogResize<CConfigWnd>)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CConfigWnd)
			DLGRESIZE_CONTROL(IDC_CGLAYOUT_TAB, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_SUBWND_VIEW, DLSZ_SIZE_X | DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDC_SUBWND_OPS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDC_SUBWND_CMDS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

	protected:
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTabChange(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled);

	private:
		CComPtr<IConfig> m_pConfig;
		CDotNetTabCtrl<CCustomTabItem> m_wndTab;
		CConfigWndLayoutView m_wndView;
		CConfigWndLayoutOps m_wndOps;
		CConfigWndMenuCommands m_wndCmds;
	};

private:
	CComPtr<IDesignerFrameIcons> m_pIconsManager;
};
*/