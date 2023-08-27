// GlobalOptionsDlg.h : Declaration of the CGlobalOptionsDlg

#pragma once

#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include <atlgdix.h>
#include <CustomTabCtrl.h>
#include <DotNetTabCtrl.h>
#include <IconRenderer.h>


// CGlobalOptionsDlg

class CGlobalOptionsDlg : 
	public Win32LangEx::CLangDialogImpl<CGlobalOptionsDlg>,
	public CDialogResize<CGlobalOptionsDlg>,
	public CContextHelpDlg<CGlobalOptionsDlg>
{
public:
	CGlobalOptionsDlg(LCID a_tLocaleID, HICON a_hIcon, LONG* a_pSizeX, LONG* a_pSizeY, GUID* a_pActivePage) : m_hIcon(a_hIcon),
		m_pSizeX(a_pSizeX), m_pSizeY(a_pSizeY), m_pActivePage(a_pActivePage),
		Win32LangEx::CLangDialogImpl<CGlobalOptionsDlg>(a_tLocaleID), m_hHelpIcon(NULL),
		CContextHelpDlg<CGlobalOptionsDlg>(_T("http://www.rw-designer.com/application-options"))
	{
	}
	~CGlobalOptionsDlg()
	{
		if (m_hHelpIcon) DestroyIcon(m_hHelpIcon);
	}

	bool DoModalPreTranslate(MSG const* a_pMsg)
	{
		return m_pCfgWnd && (S_OK == m_pCfgWnd->PreTranslateMessage(a_pMsg, TRUE) || S_OK == m_pCfgWnd->PreTranslateMessage(a_pMsg, FALSE));
	}

	enum { IDD = IDD_GLOBALOPTIONS };

	BEGIN_MSG_MAP(CGlobalOptionsDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CGlobalOptionsDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		NOTIFY_HANDLER(IDC_GO_TAB, CTCN_SELCHANGE, OnTcnSelchange)
		CHAIN_MSG_MAP(CDialogResize<CGlobalOptionsDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CGlobalOptionsDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_GO_SEP, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_GO_INFO, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_GO_HINT, /*DLSZ_MOVE_Y|*/DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_GO_TAB, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_GO_CONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CGlobalOptionsDlg)
		CTXHELP_CONTROL_RESOURCE(IDOK, IDS_HELP_IDOK)
		CTXHELP_CONTROL_RESOURCE(IDCANCEL, IDS_HELP_IDCANCEL)
		CTXHELP_CONTROL_RESOURCE(IDHELP, IDS_HELP_IDHELP)
	END_CTXHELP_MAP()

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetIcon(m_hIcon, FALSE);

		CStatic cIcon = GetDlgItem(IDC_GO_INFO);
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			RECT r = {0, 0, 0, 0};
			cIcon.GetClientRect(&r);
			CIconRendererReceiver cRenderer(min(r.right, r.bottom));
			pSI->GetLayers(ESIHelp, cRenderer);
			m_hHelpIcon = cRenderer.get();
		}
		cIcon.SetIcon(m_hHelpIcon);

		CWindow wnd = GetDlgItem(IDC_GO_TAB);
		RECT rc;
		wnd.GetWindowRect(&rc);
		ScreenToClient(&rc);
		wnd.DestroyWindow();
		m_wndTab.Create(m_hWnd, rc, _T("Tab"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_NOEDGE|CTCS_SCROLL, 0, IDC_GO_TAB);

		wnd = GetDlgItem(IDC_GO_CONFIG);
		wnd.GetWindowRect(&rc);
		ScreenToClient(&rc);
		wnd.DestroyWindow();
		RWCoCreateInstance(m_pCfgWnd, __uuidof(AutoConfigWnd));
		m_pCfgWnd->Create(m_hWnd, &rc, IDC_GO_CONFIG, m_tLocaleID, TRUE, ECWBMNothing);

		CComPtr<IGlobalConfigManager> m_pMgr;
		RWCoCreateInstance(m_pMgr, __uuidof(GlobalConfigManager));

		CComPtr<IEnumGUIDs> pIDs;
		m_pMgr->EnumIDs(&pIDs);
		ULONG nIDs = 0;
		if (pIDs) pIDs->Size(&nIDs);
		std::map<ULONG, GUID> tIDs;
		for (ULONG i = 0; i < nIDs; ++i)
		{
			GUID tID;
			pIDs->Get(i, &tID);
			BYTE bPrio = 255;
			if (S_OK != m_pMgr->Interactive(tID, &bPrio))
				continue;
			tIDs[(ULONG(bPrio)<<16)|i] = tID;
		}
		std::map<ULONG, GUID>::const_iterator k = tIDs.begin();
		for (std::map<ULONG, GUID>::const_iterator i = tIDs.begin(); i != tIDs.end(); ++i)
		{
			if (m_pActivePage && IsEqualGUID(*m_pActivePage, i->second))
			{
				k = i;
				break;
			}
		}
		int j = 0;
		for (std::map<ULONG, GUID>::const_iterator i = tIDs.begin(); i != tIDs.end(); ++i)
		{
			CComPtr<ILocalizedString> pName;
			m_pMgr->Name(i->second, &pName);
			CComBSTR bstrName;
			if (pName) pName->GetLocalized(m_tLocaleID, &bstrName);
			CComPtr<ILocalizedString> pDesc;
			m_pMgr->Description(i->second, &pDesc);
			CComPtr<IConfig> pConfig;
			m_pMgr->Config(i->second, &pConfig);
			CComPtr<IConfig> pDupl;
			pConfig->DuplicateCreate(&pDupl);
			STabItem* pItem = new STabItem(i->second, pDesc, pDupl, pConfig);
			pItem->SetText(COLE2T(bstrName));
			m_wndTab.InsertItem(j++, pItem, i == k);
		}

		DlgResize_Init();

		if (m_pSizeX && *m_pSizeX != CW_USEDEFAULT && m_pSizeY && *m_pSizeY != CW_USEDEFAULT)
		{
			RECT rc;
			GetWindowRect(&rc);
			rc.right = rc.left + *m_pSizeX;
			rc.bottom = rc.top + *m_pSizeY;
			MoveWindow(&rc);
		}

		CenterWindow(GetParent());

		return 1;  // Let the system set the focus
	}
	LRESULT OnTcnSelchange(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
	{
		int nCount = m_wndTab.GetItemCount();
		int nSel = m_wndTab.GetCurSel();

		if (nSel < 0 || nSel >= nCount)
			return 0;

		STabItem* pItem = m_wndTab.GetItem(nSel);
		pItem->bDisplayed = true;
		m_pCfgWnd->ConfigSet(pItem->pConfig, ECPMFull);
		CComBSTR bstr;
		if (pItem->pDesc) pItem->pDesc->GetLocalized(m_tLocaleID, &bstr);
		COLE2CT str(bstr.m_str ? bstr.m_str : L"");
		SetDlgItemText(IDC_GO_HINT, str);
		return 0;
	}
	LRESULT OnClickedOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int nCount = m_wndTab.GetItemCount();
		for (int i = 0; i < nCount; ++i)
		{
			STabItem* pItem = m_wndTab.GetItem(i);
			if (pItem->bDisplayed)
				CopyConfigValues(pItem->pOriginal, pItem->pConfig);
		}
		OnClickedCancel(a_wNotifyCode, IDOK, a_hWndCtl, a_bHandled);
		return 0;
	}
	LRESULT OnClickedCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		RECT rcWin;
		GetWindowRect(&rcWin);
		if (m_pSizeX) *m_pSizeX = rcWin.right-rcWin.left;
		if (m_pSizeY) *m_pSizeY = rcWin.bottom-rcWin.top;
		if (m_pActivePage) *m_pActivePage = m_wndTab.GetItem(m_wndTab.GetCurSel())->tID;
		EndDialog(a_wID);
		return 0;
	}

private:
	struct STabItem : public CCustomTabItem
	{
		STabItem(GUID a_tID, ILocalizedString* a_pDesc, IConfig* a_pConfig, IConfig* a_pOriginal) :
			tID(a_tID), pDesc(a_pDesc), pConfig(a_pConfig), pOriginal(a_pOriginal), bDisplayed(false)
		{
		}
		STabItem(STabItem const& a_rhs) :
			tID(a_rhs.tID), pDesc(a_rhs.pDesc), pConfig(a_rhs.pConfig), pOriginal(a_rhs.pOriginal), bDisplayed(a_rhs.bDisplayed)
		{
			CCustomTabItem::operator =(a_rhs);
		}
		GUID tID;
		CComPtr<ILocalizedString> pDesc;
		CComPtr<IConfig> pConfig;
		CComPtr<IConfig> pOriginal;
		bool bDisplayed;
	};

	template <class TItem = CCustomTabItem>
	class CContrastTabCtrl : public CDotNetTabCtrlImpl<CContrastTabCtrl<TItem>, TItem>
	{
	public:
		DECLARE_WND_CLASS_EX(_T("WTL_ContrastDotNetTabCtrl"), CS_DBLCLKS, COLOR_WINDOW)

		void InitializeDrawStruct(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
		{
			CDotNetTabCtrlImpl<CContrastTabCtrl, TItem>::InitializeDrawStruct(lpNMCustomDraw);
			lpNMCustomDraw->clrBtnHighlight = ::GetSysColor(COLOR_3DSHADOW);
		}
	};

private:
	CContrastTabCtrl<STabItem> m_wndTab;
	CComPtr<IConfigWnd> m_pCfgWnd;
	HICON m_hIcon;
	HICON m_hHelpIcon;
	LONG* m_pSizeX;
	LONG* m_pSizeY;
	GUID* m_pActivePage;
};


