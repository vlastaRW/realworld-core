#pragma once

#include "resource.h"
#include "RWViewAreaLayout.h"

#include <Win32LangEx.h>
#include <ContextHelpDlg.h>
#include "ConfigIDsSplitter.h"


class ATL_NO_VTABLE CConfigGUISplitter :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigCustomGUI
{
public:
	BEGIN_COM_MAP(CConfigGUISplitter)
		COM_INTERFACE_ENTRY(IConfigCustomGUI)
	END_COM_MAP()

	// IConfigCustomGUI methods
public:
	STDMETHOD(UIDGet)(GUID* a_pguid);
	STDMETHOD(RequiresMargins)();
	STDMETHOD(MinSizeGet)(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY);
	STDMETHOD(WindowCreate)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow);

private:
	class ATL_NO_VTABLE CConfigWnd :
		public CComObjectRootEx<CComMultiThreadModel>,
		public CChildWindowImpl<CConfigWnd, IChildWindow>,
		public Win32LangEx::CLangDialogImpl<CConfigWnd>,
		public CDialogResize<CConfigWnd>,
		public CContextHelpDlg<CConfigWnd>
	{
	public:
		CConfigWnd() : m_eSplitType(-1), m_eMode(ECPMFull)
		{
		}
		void Create(HWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, IConfig* a_pConfig, EConfigPanelMode a_eMode);

		BEGIN_COM_MAP(CConfigWnd)
			COM_INTERFACE_ENTRY(IChildWindow)
		END_COM_MAP()

		enum { IDD = IDD_CONFIGGUI_SPLITTER };

		BEGIN_MSG_MAP(CConfigWnd)
			CHAIN_MSG_MAP(CContextHelpDlg<CConfigWnd>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_HSCROLL, OnVerRelativeChanged)
			MESSAGE_HANDLER(WM_VSCROLL, OnHorRelativeChanged)
			COMMAND_HANDLER(IDC_CGSPLITTER_SPLITDIRECTION, CBN_SELCHANGE, OnSplitDirectionChanged)
			COMMAND_HANDLER(IDC_CGSPLITTER_VERRELATIVE, BN_CLICKED, OnVerTypeClicked)
			COMMAND_HANDLER(IDC_CGSPLITTER_HORRELATIVE, BN_CLICKED, OnHorTypeClicked)
			COMMAND_HANDLER(IDC_CGSPLITTER_VERADJUSTABLE, BN_CLICKED, OnVerTypeClicked)
			COMMAND_HANDLER(IDC_CGSPLITTER_HORADJUSTABLE, BN_CLICKED, OnHorTypeClicked)
			COMMAND_HANDLER(IDC_CGSPLITTER_VERABSOLUTEBACK, BN_CLICKED, OnVerTypeClicked)
			COMMAND_HANDLER(IDC_CGSPLITTER_HORABSOLUTEBACK, BN_CLICKED, OnHorTypeClicked)
			COMMAND_HANDLER(IDC_CGSPLITTER_VERABSOLUTEVAL, EN_CHANGE, OnVerAbsoluteChanged)
			COMMAND_HANDLER(IDC_CGSPLITTER_HORABSOLUTEVAL, EN_CHANGE, OnHorAbsoluteChanged)
			COMMAND_HANDLER(IDC_CGSPLITTER_LT, CBN_SELCHANGE, OnComboChanged<CFGID_SUBVIEWLT>)
			COMMAND_HANDLER(IDC_CGSPLITTER_LB, CBN_SELCHANGE, OnComboChanged<CFGID_SUBVIEWLB>)
			COMMAND_HANDLER(IDC_CGSPLITTER_RT, CBN_SELCHANGE, OnComboChanged<CFGID_SUBVIEWRT>)
			COMMAND_HANDLER(IDC_CGSPLITTER_RB, CBN_SELCHANGE, OnComboChanged<CFGID_SUBVIEWRB>)
			CHAIN_MSG_MAP(CDialogResize<CConfigWnd>)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CConfigWnd)
			DLGRESIZE_CONTROL(IDC_CGSPLITTER_VERRELATIVEVAL, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_CGSPLITTER_HORRELATIVEVAL, DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDC_CGSPLITTER_VERGROUP, DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDC_CGSPLITTER_HORGROUP, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_CGSPLITTER_SUBVIEWS, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

		bool DlgResize_PositionControl(int cxWidth, int cyHeight, RECT& rectGroup, _AtlDlgResizeData& data, bool bGroup, _AtlDlgResizeData* pDataPrev = NULL)
		{
			bool bRet = CDialogResize<CConfigWnd>::DlgResize_PositionControl(cxWidth, cyHeight, rectGroup, data, bGroup, pDataPrev);
			if (data.m_nCtlID == IDC_CGSPLITTER_SUBVIEWS)
				UpdateSubViewPositions();
			return bRet;
		}

		BEGIN_CTXHELP_MAP(CConfigWnd)
			CTXHELP_CONTROL_RESOURCE(IDC_CGSPLITTER_SPLITDIRECTION, IDS_CFGID_SPLITTYPE_HELP)
			CTXHELP_CONTROL_RESOURCE(IDC_CGSPLITTER_HORRELATIVEVAL, IDS_CFGID_HORRELATIVE_HELP)
			CTXHELP_CONTROL_RESOURCE(IDC_CGSPLITTER_VERRELATIVEVAL, IDS_CFGID_VERRELATIVE_HELP)
			CTXHELP_CONTROL_RESOURCE(IDC_CGSPLITTER_HORABSOLUTEVAL, IDS_CFGID_HORABSOLUTE_HELP)
			CTXHELP_CONTROL_RESOURCE(IDC_CGSPLITTER_VERABSOLUTEVAL, IDS_CFGID_VERABSOLUTE_HELP)
			CTXHELP_CONTROL_RESOURCE(IDC_CGSPLITTER_LT, IDS_CFGID_SUBVIEWLT_HELP)
			CTXHELP_CONTROL_RESOURCE(IDC_CGSPLITTER_LB, IDS_CFGID_SUBVIEWLB_HELP)
			CTXHELP_CONTROL_RESOURCE(IDC_CGSPLITTER_RT, IDS_CFGID_SUBVIEWRT_HELP)
			CTXHELP_CONTROL_RESOURCE(IDC_CGSPLITTER_RB, IDS_CFGID_SUBVIEWRB_HELP)
//CTXHELP_CONTROL_RESOURCE(, IDS_CFGID_HORDIVTYPE_HELP)
//CTXHELP_CONTROL_RESOURCE(, IDS_CFGID_VERDIVTYPE_HELP)
//IDC_CGSPLITTER_VERRELATIVE
//IDC_CGSPLITTER_VERADJUSTABLE
//IDC_CGSPLITTER_VERABSOLUTEBACK
//IDC_CGSPLITTER_HORRELATIVE
//IDC_CGSPLITTER_HORADJUSTABLE
//IDC_CGSPLITTER_HORABSOLUTEBACK
		END_CTXHELP_MAP()

	protected:
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnVerRelativeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnHorRelativeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSplitDirectionChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
		LRESULT OnVerTypeClicked(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
		LRESULT OnHorTypeClicked(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
		LRESULT OnVerAbsoluteChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
		LRESULT OnHorAbsoluteChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
		template<LPCOLESTR t_pszID>
		LRESULT OnComboChanged(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND a_hWndCtl, BOOL& UNREF(a_bHandled))
		{
			if (!m_bSetting)
			{
				try
				{
					CComboBox wnd = a_hWndCtl;
					CComBSTR cCFGID_SUBVIEW(t_pszID);
					CComPtr<IConfigItemOptions> pItem;
					m_pConfig->ItemGetUIInfo(cCFGID_SUBVIEW, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
					CComPtr<IEnumConfigItemOptions> pOptions;
					pItem->OptionsEnum(&pOptions);
					ULONG nIndex = static_cast<ULONG>(wnd.GetItemData(wnd.GetCurSel()));
					CConfigValue cVal;
					pOptions->Get(nIndex, &cVal);
					m_pConfig->ItemValuesSet(1, &(cCFGID_SUBVIEW.m_str), cVal);
				}
				catch (...)
				{
				}
			}

			return 0;
		}

		void ValuesToGUI();
		void UpdateSubViewPositions();

	private:
		CComPtr<IConfig> m_pConfig;
		EConfigPanelMode m_eMode;

		CComboBox m_wndSplitDirection;

		CButton m_wndVerRelative;
		CButton m_wndVerAdjustable;
		CEdit m_wndVerEdit;
		CStatic m_wndVerStatic;
		CWindow m_wndVerLine;
		CWindow m_wndVerLabel;
		CButton m_wndVerFromBack;
		CTrackBarCtrl m_wndVerSlider;

		CButton m_wndHorRelative;
		CButton m_wndHorAdjustable;
		CEdit m_wndHorEdit;
		CStatic m_wndHorStatic;
		CWindow m_wndHorLine;
		CWindow m_wndHorLabel;
		CButton m_wndHorFromBack;
		CTrackBarCtrl m_wndHorSlider;

		CComboBox m_wndViewTypeLT;
		CComPtr<IConfigWnd> m_pConfigWndLT;
		CComboBox m_wndViewTypeLB;
		CComPtr<IConfigWnd> m_pConfigWndLB;
		CComboBox m_wndViewTypeRT;
		CComPtr<IConfigWnd> m_pConfigWndRT;
		CComboBox m_wndViewTypeRB;
		CComPtr<IConfigWnd> m_pConfigWndRB;

		CStatic m_wndSubViews;

		LONG m_eSplitType; // cached value

		bool m_bSetting;
	};
};
