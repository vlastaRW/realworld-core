
#pragma once

#include <Win32LangEx.h>
#include <MultiLanguageString.h>
#include <SubjectImpl.h>
#include <ObserverImpl.h>
#include <RWConceptDesignerExtension.h>
#include <MemoryStorageFilter.h>
#include <ContextHelpDlg.h>
#include <XPGUI.h>
#include <DPIUtils.h>
#include <WTL_PopupButton.h>
#include <ContextMenuWithIcons.h>
#include <math.h>

extern __declspec(selectany) OLECHAR const CFGID_CFG_DLGSIZEX[] = L"DlgSizeX";
extern __declspec(selectany) OLECHAR const CFGID_CFG_DLGSIZEY[] = L"DlgSizeY";
extern __declspec(selectany) OLECHAR const CFGID_CFG_CAPTION[] = L"Caption";
extern __declspec(selectany) OLECHAR const CFGID_CFG_ICONID[] = L"IconID";
extern __declspec(selectany) OLECHAR const CFGID_CFG_PREVIEW[] = L"Preview";
extern __declspec(selectany) OLECHAR const CFGID_CFG_HELPTOPIC[] = L"HelpTopic";
extern __declspec(selectany) OLECHAR const CFGID_CFG_PREVIEWMODE[] = L"DisplayMode";
static LONG const CFGVAL_PM_PROCESSED = 0;
static LONG const CFGVAL_PM_AUTOSELECT = 1;
static LONG const CFGVAL_PM_SPLITTED = 2;
extern __declspec(selectany) OLECHAR const CFGID_CFG_SPLITPOS[] = L"SplitPos";
extern __declspec(selectany) OLECHAR const CFGID_CFG_HISTORY[] = L"History";


struct IPreviewMaker
{
	virtual HRESULT MakePreview(RWHWND a_hParent, LCID a_tLocaleID, IDocument** a_ppPreviewDoc) = 0;
};

template<WORD t_wSkipID = IDCANCEL>
class CShowConfigWithPreviewDlg :
	public Win32LangEx::CLangIndirectDialogImpl<CShowConfigWithPreviewDlg<t_wSkipID> >,
	public CDialogResize<CShowConfigWithPreviewDlg<t_wSkipID> >,
	public CContextHelpDlg<CShowConfigWithPreviewDlg<t_wSkipID> >,
	public CObserverImpl<CShowConfigWithPreviewDlg<t_wSkipID>, IConfigObserver, IUnknown*>,
	public CContextMenuWithIcons<CShowConfigWithPreviewDlg<t_wSkipID> >
{
public:
	enum
	{
		IDC_RT_VIEW_ORIG = 300,
		IDC_RT_VIEW_PROC,
		IDC_RT_HELP,
		IDC_RT_HISTORY,
		IDC_RT_ICON,
		IDC_RT_SEPARATOR_BOT,
		IDC_RT_CONFIG,
		ID_BTN_HISTORY = 1100,
	};

	CShowConfigWithPreviewDlg(LCID a_tLocaleID, IConfig* a_pCfg, IConfig* a_pDlgSizeCfg, IUnknown* a_pMgr, IConfigDescriptor* a_pAssistant, IViewManager* a_pViewManager = NULL, IDocument* a_pDoc = NULL, IPreviewMaker* a_pPreviewMaker = NULL) :
		Win32LangEx::CLangIndirectDialogImpl<CShowConfigWithPreviewDlg<t_wSkipID> >(a_tLocaleID), m_bSaveSize(false), m_bSaveSplit(false),
		m_pConfig(a_pCfg), m_pDlgSizeCfg(a_pDlgSizeCfg), m_bPreview(false), m_bPreviewSplit(false), m_fPreviewLinePos(0.0f),
		m_pViewManager(a_pViewManager), m_pDoc(a_pDoc), m_hIcon(NULL), m_hIconLarge(NULL), m_pPreviewMaker(a_pPreviewMaker), m_bHistory(false),
		m_pMgr(a_pMgr), m_pAssistant(a_pAssistant), m_fScale(1.0f)
	{
		static INITCOMMONCONTROLSEX tICC = {sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES|ICC_LINK_CLASS};
		static BOOL b = InitCommonControlsEx(&tICC);
	}
	~CShowConfigWithPreviewDlg()
	{
		if (m_hIcon) DestroyIcon(m_hIcon);
		if (m_hIconLarge) DestroyIcon(m_hIconLarge);
		m_cImageList.Destroy();
		m_pOriginalWnd = NULL;
		m_pPreviewWnd = NULL;
	}
	void OwnerNotify(TCookie, IUnknown*)
	{
		BOOL b;
		OnUpdatePreview(0, 0, NULL, b);
	}

	bool DoModalPreTranslate(MSG const* a_pMsg)
	{
		if (m_pConfigWnd && m_pConfigWnd->PreTranslateMessage(a_pMsg, TRUE) == S_OK)
			return true;
		if (m_pOriginalWnd && m_pOriginalWnd->PreTranslateMessage(a_pMsg, TRUE) == S_OK)
			return true;
		if (m_pPreviewWnd && m_pPreviewWnd->PreTranslateMessage(a_pMsg, TRUE) == S_OK)
			return true;
		if (m_pConfigWnd && m_pConfigWnd->PreTranslateMessage(a_pMsg, FALSE) == S_OK)
			return true;
		if (m_pOriginalWnd && m_pOriginalWnd->PreTranslateMessage(a_pMsg, FALSE) == S_OK)
			return true;
		if (m_pPreviewWnd && m_pPreviewWnd->PreTranslateMessage(a_pMsg, FALSE) == S_OK)
			return true;
		return false;
	}

	BEGIN_DIALOG_EX(0, 0, 233, 123, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_CLIPCHILDREN|WS_CLIPSIBLINGS)
		DIALOG_EXSTYLE(WS_EX_CONTEXTHELP|WS_EX_CONTROLPARENT)
		DIALOG_CAPTION(_T("[0409]Configure Operation[0405]Konfigurovat operaci"))
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_RT_CONFIG, WC_STATIC, SS_BLACKRECT, 157, 0, 76, 83, 0)
		CONTROL_DEFPUSHBUTTON(_T("[0409]OK[0405]OK"), IDOK, 122, 102, 50, 14, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Cancel[0405]Storno"), IDCANCEL, 176, 102, 50, 14, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_RT_VIEW_PROC, WC_STATIC, SS_BLACKRECT | WS_CLIPCHILDREN | WS_VISIBLE, 7, 7, 150, 60, 0)
		CONTROL_CONTROL(_T(""), IDC_RT_VIEW_ORIG, WC_STATIC, SS_BLACKRECT | WS_CLIPCHILDREN | WS_VISIBLE, 157, 7, 0, 60, 0)
		CONTROL_CONTROL(_T(""), IDC_RT_SEPARATOR_BOT, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 0, 83, 233, 1, 0)
		CONTROL_ICON(_T(""), IDC_RT_ICON, 7, 93, 20, 20, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]After setting the parameters, click OK to activate the operation.\r\n\r\n<a href=\"http://wiki.rw-designer.com/Operation_PlugIns\">Online documentation</a>[0405]Po nastavení parametrů klikněte na OK pro vykonání operace.\r\n\r\n<a href=\"http://wiki.rw-designer.com/Operation_PlugIns\">Online dokumentace</a>"), IDC_RT_HELP, 34, 88, 81, 32, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_RT_HISTORY, TOOLBARCLASSNAME, WS_GROUP | TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER | CCS_NORESIZE | CCS_BOTTOM | CCS_NOMOVEY, 122, 87, 104, 14, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CShowConfigWithPreviewDlg<t_wSkipID>)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		COMMAND_HANDLER(IDHELP, BN_CLICKED, OnClickedHelp)
		CHAIN_MSG_MAP(CContextHelpDlg<CShowConfigWithPreviewDlg<t_wSkipID> >)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOKOrCancel)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnOKOrCancel)
		COMMAND_HANDLER(IDCANCEL, BN_SUBMENUCLICKED, OnSubMenuClicked)
		//COMMAND_HANDLER(ID_BTN_HISTORY, BN_CLICKED, OnHistoryMenu)
		NOTIFY_HANDLER(IDC_RT_HELP, NM_CLICK, OnHelpLinkClick)
		NOTIFY_HANDLER(IDC_RT_HELP, NM_RETURN, OnHelpLinkClick)
		NOTIFY_HANDLER(IDC_RT_HISTORY, TBN_DROPDOWN, OnHistoryMenu)
		CHAIN_MSG_MAP(CDialogResize<CShowConfigWithPreviewDlg<t_wSkipID> >)
		CHAIN_MSG_MAP(CContextMenuWithIcons<CShowConfigWithPreviewDlg<t_wSkipID> >)
		MESSAGE_HANDLER(WM_RW_GETCFGDOC, OnGetCfgDoc)
		MESSAGE_HANDLER(WM_HSCROLL, OnSplitScroll)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CShowConfigWithPreviewDlg<t_wSkipID>)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_RT_HISTORY, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_RT_SEPARATOR_BOT, DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_RT_ICON, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_RT_HELP, DLSZ_SIZE_X | DLSZ_MOVE_Y)
		//DLGRESIZE_CONTROL(IDC_RT_SEPARATOR_VER, DLSZ_SIZE_Y | DLSZ_DIVMOVE_X(2))
		//DLGRESIZE_CONTROL(IDC_RT_SEPARATOR_TOP, DLSZ_SIZE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()
		//DLGRESIZE_CONTROL(IDC_RT_ORIGINAL, DLSZ_DIVSIZE_X(2) | DLSZ_SIZE_Y)
		//DLGRESIZE_CONTROL(IDC_RT_PREVIEW, DLSZ_DIVMOVE_X(2) | DLSZ_DIVSIZE_X(2) | DLSZ_SIZE_Y)

	BEGIN_CTXHELP_MAP(CShowConfigWithPreviewDlg<t_wSkipID>)
		CTXHELP_CONTROL_STRING(IDOK, L"[0409]The OK button closes this dialog and performs the operation.[0405]Tlačítko OK zavře dialog a provede operaci.")
		CTXHELP_CONTROL_STRING(IDCANCEL, t_wSkipID == IDCANCEL ? L"[0409]The Cancel button closes this dialog reverting all changes.[0405]Tlačítko Storno zavře dialog a zruší všechny provedené změny." : L"[0409]The Cancel button closes this dialog reverting all changes. When using the Skip option, the processing will continue with next operation if there is one.[0405]Tlačítko Storno zavře dialog a zruší všechny provedené změny. Použijete-li možnost Přeskočit, zpracování bude pokračovat dalším krokem, pokud je k dispozici.")
		CTXHELP_CONTROL_STRING(IDHELP, L"[0409]The Help button displays more detailed help for this dialog[0405]Tlačítko Nápověda zobrazí detailní nápovědu pro tento dialog.")
	END_CTXHELP_MAP()

	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_bSaveSize = true;
		SIZE sz = {LOWORD(a_lParam), HIWORD(a_lParam)};
		bool bIsVisible = (GetWindowLong(GWL_STYLE) & WS_VISIBLE) != 0;
		if (bIsVisible)
			SetRedraw(FALSE);
		if (m_bPreview)
		{
			RECT rc = { sz.cx-m_szConfig.cx, 0, sz.cx, sz.cy-m_nSeparator };
			m_pConfigWnd->Move(&rc);
			UpdatePreviews(sz.cx, sz.cy);
		}
		else
		{
			RECT rc = { 0, 0, sz.cx, sz.cy-m_nSeparator };
			m_pConfigWnd->Move(&rc);
		}
		if (bIsVisible)
			SetRedraw(TRUE);
		LRESULT lRes = CDialogResize<CShowConfigWithPreviewDlg<t_wSkipID> >::OnSize(a_uMsg, a_wParam, a_lParam, a_bHandled);
		return lRes;
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HWND h = reinterpret_cast<HWND>(a_lParam);
		if (h == m_wndOK || h == m_hWndCancel || h == m_wndGripper || h == m_hWnd)
			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HWND h = reinterpret_cast<HWND>(a_lParam);
		if (h == m_wndIcon || h == m_wndHelp || /*h == m_wndSaveChanges || */h == m_wndOK || h == m_hWndCancel || h == m_wndGripper)
		{
			SetBkColor((HDC)a_wParam, GetSysColor(COLOR_WINDOW));
			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnEraseBkgnd(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hDC = reinterpret_cast<HDC>(a_wParam);
		RECT rc;
		GetClientRect(&rc);
		rc.top = rc.bottom-m_nSeparator;
		FillRect(hDC, &rc, reinterpret_cast<HBRUSH>(COLOR_WINDOW+1));
		rc.bottom = rc.top;
		rc.top = 0;
		FillRect(hDC, &rc, reinterpret_cast<HBRUSH>(COLOR_3DFACE+1));
		return 1;
	}
	LRESULT OnGetCfgDoc(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_lParam)
		{
			*reinterpret_cast<IDocument**>(a_lParam) = m_pDoc;
			if (m_pDoc) m_pDoc.p->AddRef();
		}
		return 0;
	}

	LRESULT OnSplitScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_fPreviewLinePos = m_wndSplitSlider.GetPos();
		RECT rc;
		GetClientRect(&rc);
		bool bIsVisible = (GetWindowLong(GWL_STYLE) & WS_VISIBLE) != 0;
		if (bIsVisible)
			SetRedraw(FALSE);
		UpdatePreviews(rc.right, rc.bottom);
		if (bIsVisible)
			SetRedraw(TRUE);
		RECT rcO;
		m_wndOriginal.GetWindowRect(&rcO);
		m_wndPreview.GetWindowRect(&rc);
		rc.right = rcO.right;
		ScreenToClient(&rc);
		RedrawWindow(&rc, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		m_bSaveSplit = true;
		return 0;
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hDC = GetDC();
		m_fScale = GetDeviceCaps(hDC, LOGPIXELSY)/96.0f;
		ReleaseDC(hDC);

		m_wndIcon = GetDlgItem(IDC_RT_ICON);
		m_wndHelp = GetDlgItem(IDC_RT_HELP);
		m_wndOK = GetDlgItem(IDOK);
		m_wndSepBot = GetDlgItem(IDC_RT_SEPARATOR_BOT);
		m_hWndCancel = GetDlgItem(IDCANCEL);
		if (t_wSkipID != IDCANCEL)
			m_wndCancel.SubclassWindow(m_hWndCancel, NULL);
		m_wndOriginal = GetDlgItem(IDC_RT_VIEW_ORIG);
		m_wndPreview = GetDlgItem(IDC_RT_VIEW_PROC);
		m_wndHistory = GetDlgItem(IDC_RT_HISTORY);

		CConfigValue cCaption;
		m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_CAPTION), &cCaption);
		if (cCaption.TypeGet() == ECVTString)
		{
			CComBSTR bstrLoc;
			CMultiLanguageString::GetLocalized(cCaption, m_tLocaleID, &bstrLoc);
			SetWindowText(COLE2CT(bstrLoc.m_str));
		}

		CConfigValue cIconID;
		m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_ICONID), &cIconID);
		if (cIconID.TypeGet() == ECVTGUID && !IsEqualGUID(cIconID, GUID_NULL))
		{
			CComPtr<IDesignerFrameIcons> pIconsManager;
			RWCoCreateInstance(pIconsManager, __uuidof(DesignerFrameIconsManager));
			pIconsManager->GetIcon(cIconID, XPGUI::GetSmallIconSize(), &m_hIcon);
			if (m_hIcon)
			{
				m_hIcon = DPIUtils::PrepareIconForCaption(m_hIcon);
				SetIcon(m_hIcon, FALSE);
			}
			int nIconSize = XPGUI::GetSmallIconSize()*3;
			pIconsManager->GetIcon(cIconID, nIconSize, &m_hIconLarge);
			if (m_hIconLarge)
			{
				RECT rc;
				m_wndIcon.GetWindowRect(&rc);
				ScreenToClient(&rc);
				rc.top = ((rc.bottom+rc.top)>>1)-(nIconSize>>1);
				rc.bottom = rc.top+nIconSize;
				int nDelta = nIconSize-(rc.right-rc.left);
				rc.right = rc.left+nIconSize;
				m_wndIcon.SetIcon(m_hIconLarge);
				m_wndIcon.MoveWindow(&rc);
				m_wndHelp.GetWindowRect(&rc);
				ScreenToClient(&rc);
				rc.left += nDelta;
				m_wndHelp.MoveWindow(&rc);
			}
		}
		if (m_hIconLarge == NULL)
		{
			RECT rc;
			m_wndIcon.GetWindowRect(&rc);
			ScreenToClient(&rc);
			int nLeft = rc.left;
			m_wndIcon.ShowWindow(SW_HIDE);
			m_wndHelp.GetWindowRect(&rc);
			ScreenToClient(&rc);
			rc.left = nLeft;
			m_wndHelp.MoveWindow(&rc);
		}

		InitHistory();
		if (m_aHistory.size() > 1)
		{
			m_wndHistory.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
			m_wndHistory.SetButtonStructSize(sizeof(TBBUTTON));
			CComBSTR bstrHistory;
			CMultiLanguageString::GetLocalized(L"[0409]Previous settings[0405]Předchozí nastavení", m_tLocaleID, &bstrHistory);
			TBBUTTON tButton = {I_IMAGENONE, ID_BTN_HISTORY, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_WHOLEDROPDOWN|BTNS_AUTOSIZE, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrHistory.m_str)};
			m_wndHistory.AddButtons(1, &tButton);
			m_wndHistory.ShowWindow(TRUE);
		}

		// replace static control with html if supported
		bool bHtml = false;
		{
			RECT rcHelp;
			m_wndHelp.GetWindowRect(&rcHelp);
			ScreenToClient(&rcHelp);
			TCHAR szTmp[512] = _T("");
			m_wndHelp.GetWindowText(szTmp, itemsof(szTmp));
			HWND hLink = ::CreateWindowEx(0, /*WC_LINK*/_T("SysLink"), szTmp, WS_VISIBLE|WS_CHILD|WS_TABSTOP, rcHelp.left, rcHelp.top,
				rcHelp.right-rcHelp.left, rcHelp.bottom-rcHelp.top, m_hWnd, (HMENU)IDC_RT_HELP, _pModule->get_m_hInst(), 0);
			if (hLink)
			{
				::SetWindowPos(hLink, m_wndOriginal, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
				m_wndHelp.DestroyWindow();
				m_wndHelp = hLink;
				bHtml = true;
			}
			LOGFONT lf = {0};
			::GetObject(GetFont(), sizeof(lf), &lf);
			lf.lfHeight = (lf.lfHeight*5)/6;
			m_cHelpFont.CreateFontIndirect(&lf);
			m_wndHelp.SetFont(m_cHelpFont);
		}
		CConfigValue cHelpTopic;
		m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_HELPTOPIC), &cHelpTopic);
		if (cHelpTopic.TypeGet() == ECVTString)
		{
			if (cHelpTopic.operator BSTR()[0])
			{
				CComBSTR bstrLoc;
				CMultiLanguageString::GetLocalized(cHelpTopic, m_tLocaleID, &bstrLoc);
				LPOLESTR pM = bstrLoc;
				while (true)
				{
					LPOLESTR pBR = wcsstr(pM, L"<br>");
					if (pBR)
					{
						pBR[0] = L'\r'; pBR[1] = L'\n';
						wcscpy(pBR+2, pBR+4);
						continue;
					}
					pBR = wcsstr(pM, L"<br/>");
					if (pBR)
					{
						pBR[0] = L'\r'; pBR[1] = L'\n';
						wcscpy(pBR+2, pBR+5);
						continue;
					}
					pBR = wcsstr(pM, L"<br />");
					if (pBR)
					{
						pBR[0] = L'\r'; pBR[1] = L'\n';
						wcscpy(pBR+2, pBR+6);
						continue;
					}
					break;
				}
				m_wndHelp.SetWindowText(COLE2CT(bstrLoc.m_str));
			}
		}
		if (!bHtml)
		{
			int len = m_wndHelp.GetWindowTextLength();
			CAutoVectorPtr<TCHAR> psz(new TCHAR[len+1]);
			m_wndHelp.GetWindowText(psz, len+1);
			psz[len] = _T('\0');
			while (true)
			{
				LPTSTR pA = _tcsstr(psz, _T("<a href=\""));
				if (pA == NULL)
					break;
				LPTSTR pAEnd = _tcsstr(pA, _T("</a>"));
				if (pAEnd == NULL)
					break;
				while ((pA+9) < pAEnd && pA[9] != _T('\"'))
				{
					pA[0] = pA[9];
					++pA;
				}
				_tcscpy(pA, pAEnd+4);
			}
			m_wndHelp.SetWindowText(psz);
		}

		// is preview active?
		CConfigValue cPreview;
		m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_PREVIEW), &cPreview);
		RECT rcPreview = {7, 7, 50, 80};
		MapDialogRect(&rcPreview);
		if (m_pDoc && cPreview.TypeGet() == ECVTGUID)
		{
			if (m_pViewManager == NULL)
				RWCoCreateInstance(m_pViewManager, __uuidof(ViewManager));

			if (m_pViewManager)
			{
				CConfigValue cDefView;
				m_pViewManager->ItemIDGetDefault(&cDefView);
				if (cDefView != cPreview)
					m_bPreview = true;
			}
		}

		RECT rcDlg;
		GetClientRect(&rcDlg);
		RECT rc;
		m_wndSepBot.GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_nSeparator = rcDlg.bottom-rc.top;

		// create config window
		HWND hPlaceHolder = GetDlgItem(IDC_RT_CONFIG);
		::GetWindowRect(hPlaceHolder, &rc);
		ScreenToClient(&rc);
		RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
		CComQIPtr<IConfigCustomGUI> pGUI(m_pConfig);
		RECT rcMargins = {7, 7, 100, 80};
		MapDialogRect(&rcMargins);
		m_szMargins.cx = rcMargins.left;
		m_szMargins.cy = rcMargins.top;
		ULONG nX = rcMargins.right;
		ULONG nY = rcMargins.bottom;
		bool bMargins = false;
		if (pGUI)
		{
			pGUI->MinSizeGet(m_pConfig, m_tLocaleID, ECPMSimplified, &nX, &nY);
			bMargins = pGUI->RequiresMargins() == S_OK;
			nX += 4; // for rounding purposes
		}
		else
		{
			nX += nX>>1;
		}
		nY += 1; // for rounding purposes
		if (bMargins)
		{
			nX += rcMargins.left*2;
			nY += rcMargins.top*2;
		}
		int nDeltaX = 0;
		int nDeltaY = 0;
		if (LONG(nX) > (rc.right-rc.left))
			nDeltaX = nX - (rc.right-rc.left);
		if (!m_bPreview || LONG(nY) > (rc.bottom-rc.top))
			nDeltaY = nY - (rc.bottom-rc.top);
		if (!m_bPreview)
		{
			nDeltaX -= rc.left;
			if (nDeltaX < 0) nDeltaX = 0;
		}
		rc.right += nDeltaX;
		rc.bottom += nDeltaY;
		m_szConfig.cx = nX;
		m_szConfig.cy = nY;
		m_pConfigWnd->Create(m_hWnd, &rc, IDC_RT_CONFIG, m_tLocaleID, TRUE, ECWBMMargin);
		m_pConfigWnd->ConfigSet(m_pConfig, ECPMSimplified);
		RWHWND h = NULL;
		m_pConfigWnd->Handle(&h);
		if (h)
			::SetWindowPos(h, hPlaceHolder, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
		::DestroyWindow(hPlaceHolder);

		if (m_bPreview)
		{
			m_bPreviewSplit = false;
			m_fPreviewLinePos = 0.0f;
			CConfigValue cDisplayMode;
			m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_PREVIEWMODE), &cDisplayMode);
			if (cDisplayMode.TypeGet() == ECVTInteger)
			{
				if (cDisplayMode.operator LONG() == CFGVAL_PM_SPLITTED)
				{
					m_bPreviewSplit = true;
					m_fPreviewLinePos = 0.5f;
				}
				else if (cDisplayMode.operator LONG() == CFGVAL_PM_AUTOSELECT)
				{
					float fSplit = -1.0f;
					m_pConfigWnd->SendMessage(WM_RW_CFGSPLIT, 0, reinterpret_cast<LPARAM>(&fSplit));
					if (fSplit >= 0.0f && fSplit <= 1.0f)
					{
						m_bPreviewSplit = true;
						m_fPreviewLinePos = 1.0f-fSplit;
					}
				}
				if (m_bPreviewSplit)
				{
					CConfigValue cSplitPos;
					m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_SPLITPOS), &cSplitPos);
					if (cSplitPos.TypeGet() == ECVTFloat && cSplitPos.operator float() >= 0.0f && cSplitPos.operator float() <= 1.0f)
						m_fPreviewLinePos = cSplitPos;
				}
			}

			if (m_bPreviewSplit)
			{
				RECT rcSplit = rcPreview;
				rcSplit.top = rcSplit.bottom;
				rcSplit.bottom += m_fScale*12+0.5f;
				m_wndSplitSlider.Create(m_hWnd, &rcSplit);
				m_wndSplitSlider.Init(m_fPreviewLinePos, m_cHelpFont, m_tLocaleID);
			}
		}
		else
		{
			m_wndPreview.ShowWindow(FALSE);
		}

		m_wndOK.GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_wndOK.MoveWindow(rc.left+nDeltaX, rc.top+nDeltaY, rc.right-rc.left, rc.bottom-rc.top, FALSE);
		::GetWindowRect(m_hWndCancel, &rc);
		ScreenToClient(&rc);
		::MoveWindow(m_hWndCancel, rc.left+nDeltaX, rc.top+nDeltaY, rc.right-rc.left, rc.bottom-rc.top, FALSE);
		m_wndHelp.GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_wndHelp.MoveWindow(rc.left, rc.top+nDeltaY, rc.right-rc.left+nDeltaX, rc.bottom-rc.top, FALSE);
		m_wndHistory.GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_wndHistory.MoveWindow(rc.left+nDeltaX, rc.top+nDeltaY, rc.right-rc.left, rc.bottom-rc.top, FALSE);
		m_wndIcon.GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_wndIcon.MoveWindow(rc.left, rc.top+nDeltaY, rc.right-rc.left, rc.bottom-rc.top, FALSE);
		m_wndSepBot.GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_wndSepBot.MoveWindow(rc.left, rc.top+nDeltaY, rc.right-rc.left+nDeltaX, rc.bottom-rc.top, FALSE);
		//m_rcResizePreview.bottom -= rcDlg.bottom;//+nDeltaY;
		//m_rcResizePreview.right -= rcDlg.right+nDeltaX;

		GetWindowRect(&rc);
		LONG nWidth = rc.right-rc.left+nDeltaX;
		LONG nHeight = rc.bottom-rc.top+nDeltaY;
		MoveWindow(rc.left, rc.top, nWidth, nHeight, FALSE);
		GetClientRect(&rc);

		if (m_bPreview)
		{
			UpdatePreviews(rc.right-rc.left, rc.bottom-rc.top);
		}

		DlgResize_Init(false);
		//m_wndGripper = GetDlgItem(ATL_IDW_STATUS_BAR);


		if (m_pDlgSizeCfg)
		{
			CConfigValue cSizeX;
			m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_DLGSIZEX), &cSizeX);
			CConfigValue cSizeY;
			m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_DLGSIZEY), &cSizeY);
			if (cSizeX.TypeGet() == ECVTFloat && cSizeY.TypeGet() == ECVTFloat &&
				cSizeX.operator float() > 0.0f && cSizeY.operator float() > 0.0f)
			{
				nWidth = cSizeX.operator float()*m_fScale+0.5f;
				nHeight = cSizeY.operator float()*m_fScale+0.5f;
				MoveWindow(rc.left, rc.top, nWidth, nHeight, FALSE);
			}
		}

		// center the dialog on the screen
		CenterWindow();

		m_bSaveSize = false;

		m_pConfig->ObserverIns(ObserverGet(), 0);

		BOOL b;
		OnUpdatePreview(0, 0, NULL, b);

		if (h)
			PostMessage(WM_NEXTDLGCTL, (WPARAM)h, 1L);

		return FALSE;
	}
	LRESULT OnOKOrCancel(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		m_pConfig->ObserverDel(ObserverGet(), 0);

		if (m_pDlgSizeCfg && m_bSaveSize)
		{
			RECT rc;
			GetWindowRect(&rc);
			CComBSTR cCFGID_CFG_DLGSIZEX(CFGID_CFG_DLGSIZEX);
			CComBSTR cCFGID_CFG_DLGSIZEY(CFGID_CFG_DLGSIZEY);
			TConfigValue tVals[2];
			tVals[0].eTypeID = tVals[1].eTypeID = ECVTFloat;
			tVals[0].fVal = (rc.right-rc.left)/m_fScale;
			tVals[1].fVal = (rc.bottom-rc.top)/m_fScale;
			BSTR tIDs[2] = {cCFGID_CFG_DLGSIZEX, cCFGID_CFG_DLGSIZEY};
			m_pDlgSizeCfg->ItemValuesSet(2, tIDs, tVals);
		}
		if (m_pDlgSizeCfg && m_bSaveSplit)
		{
			CComBSTR cCFGID_CFG_SPLITPOS(CFGID_CFG_SPLITPOS);
			m_pDlgSizeCfg->ItemValuesSet(1, &(cCFGID_CFG_SPLITPOS.m_str), CConfigValue(m_fPreviewLinePos));
		}
		EndDialog(wID);
		if (wID == IDOK && m_bHistory)
		{
			m_pHistory->DeleteItems(NULL);
			std::vector<CConfigValue> aVals;
			aVals.resize(m_aIDs.size());
			for (size_t j = 0; j != m_aIDs.size(); ++j)
				m_pConfig->ItemValueGet(m_aIDs[j], &aVals[j]);
			LONG n = 0;
			for (std::vector<CComPtr<IConfig> >::const_iterator i = m_aHistory.begin(); i != m_aHistory.end(); ++i)
			{
				bool bChange = false;
				for (size_t j = 0; j != m_aIDs.size(); ++j)
				{
					CConfigValue cVal;
					(*i)->ItemValueGet(m_aIDs[j], &cVal);
					if (cVal != aVals[j])
					{
						if (!bChange)
						{
							bChange = true;
							CComBSTR bstr(1);
							bstr[0] = L'0'+n;
							m_pHistory->ItemValuesSet(1, &(bstr.m_str), CConfigValue(true));
						}
						CComBSTR bstr(1+m_aIDs[j].Length());
						bstr[0] = L'0'+n;
						wcscpy(bstr.m_str+1, m_aIDs[j]);
						m_pHistory->ItemValuesSet(1, &(bstr.m_str), cVal);
					}
				}
				if (bChange)
					++n;
				if (n == 5)
					break;
			}
		}
		return 0;
	}
	void InitHistory()
	{
		if (m_bHistory)
			return;

		CConfigValue cHistory;
		m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_HISTORY), &cHistory);
		if (cHistory.TypeGet() == ECVTBool && cHistory)
		{
			CComPtr<IConfig> pHistoryCfg;
			m_pDlgSizeCfg->SubConfigGet(CComBSTR(CFGID_CFG_HISTORY), &pHistoryCfg);
			m_pHistory = pHistoryCfg;
			if (m_pHistory)
			{
				m_bHistory = true;
				{
					CComPtr<IConfig> pCurrent;
					m_pConfig->DuplicateCreate(&pCurrent);
					m_aHistory.push_back(pCurrent);
				}
				CComPtr<IEnumStrings> pIDs;
				m_pConfig->ItemIDsEnum(&pIDs);
				ULONG nIDs = 0;
				if (pIDs) pIDs->Size(&nIDs);
				m_aIDs.resize(nIDs);
				for (ULONG i = 0; i < nIDs; ++i)
					pIDs->Get(i, &m_aIDs[i]);
				std::vector<CConfigValue> aVals;
				aVals.resize(nIDs);

				if (GetAsyncKeyState(VK_SHIFT)&GetAsyncKeyState(VK_CONTROL)&0x8000)
					return;

				for (LONG i = 0; i < 10; ++i)
				{
					wchar_t szID[10] = L" ";
					szID[0] = L'0'+i;
					CConfigValue cMarker;
					m_pHistory->ItemValueGet(CComBSTR(szID), &cMarker);
					if (cMarker.TypeGet() == ECVTEmpty)
						break;
					std::vector<BSTR> aNewIDs;
					std::vector<TConfigValue> aNewVals;
					for (size_t j = 0; j != nIDs; ++j)
					{
						CComBSTR bstrH(1+m_aIDs[j].Length());
						bstrH[0] = szID[0];
						wcscpy(bstrH.m_str+1, m_aIDs[j]);
						m_pHistory->ItemValueGet(bstrH, &aVals[j]);
						if (aVals[j].TypeGet() != ECVTEmpty)
						{
							aNewIDs.push_back(m_aIDs[j]);
							aNewVals.push_back(aVals[j]);
						}
					}
					CComPtr<IConfig> pOlder;
					m_pConfig->DuplicateCreate(&pOlder);
					if (!aNewIDs.empty())
						pOlder->ItemValuesSet(aNewIDs.size(), &(aNewIDs[0]), &(aNewVals[0]));
					m_aHistory.push_back(pOlder);
				}
			}
		}
	}
	bool GetDefaultConfig(IConfig** a_ppDst)
	{
		std::vector<BSTR> cIDs;
		std::vector<CConfigValue> cVals;
		for (std::vector<CComBSTR>::const_iterator i = m_aIDs.begin(); i != m_aIDs.end(); ++i)
		{
			if (wcsstr(*i, L"SelectionSync") || wcsstr(*i, L"SyncGroup") || wcsstr(*i, L"SyncID") || wcsstr(*i, L"MaskID") || wcsstr(*i, L"SelectionID"))
				continue; // HACK: do not change the values of selection synchronization IDs or things stop working
			CComPtr<IConfigItem> pCI;
			m_pConfig->ItemGetUIInfo(*i, __uuidof(IConfigItem), reinterpret_cast<void**>(&pCI));
			CConfigValue cVal;
			if (pCI) pCI->Default(&cVal);
			if (cVal.TypeGet() != ECVTEmpty)
			{
				cVals.push_back(cVal);
				cIDs.push_back(*i);
			}
		}

		if (cVals.empty())
			return false;

		CComPtr<IConfig> pCfg;
		m_pConfig->DuplicateCreate(&pCfg);
		if (pCfg == NULL) return false;
		pCfg->ItemValuesSet(cVals.size(), &(cIDs[0]), cVals[0]);
		*a_ppDst = pCfg.Detach();
		return true;
	}
	bool IsEqualConfig(IConfig* a_p1, IConfig* a_p2)
	{
		for (size_t j = 0; j != m_aIDs.size(); ++j)
		{
			CConfigValue c1;
			a_p1->ItemValueGet(m_aIDs[j], &c1);
			CConfigValue c2;
			a_p2->ItemValueGet(m_aIDs[j], &c2);
			if (c1 != c2)
				return false;
		}
		return true;
	}
	LRESULT OnHistoryMenu(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		CMenu cMenu;
		cMenu.CreatePopupMenu();
		Reset(NULL);
		CComPtr<IConfig> pDef;
		if (GetDefaultConfig(&pDef) && !IsEqualConfig(pDef, m_pConfig))
		{
			CComBSTR bstrDef;
			CMultiLanguageString::GetLocalized(L"[0409]Default settings[0405]Výchozí nastavení", m_tLocaleID, &bstrDef);
			AddItem(ID_BTN_HISTORY-1, bstrDef, -1);
			cMenu.AppendMenu(MFT_OWNERDRAW, ID_BTN_HISTORY-1, LPCTSTR(NULL));
			cMenu.AppendMenu(MFT_SEPARATOR);
		}
		CComBSTR bstrGen;
		for (std::vector<CComPtr<IConfig> >::const_iterator i = m_aHistory.begin(); i != m_aHistory.end(); ++i)
		{
			CComPtr<ILocalizedString> pName;
			if (m_pAssistant)
				m_pAssistant->Name(m_pMgr, *i, &pName);
			CComBSTR bstrName;
			if (pName)
				pName->GetLocalized(m_tLocaleID, &bstrName);
			UINT const uFlags = IsEqualConfig(m_pConfig, *i) ? MFT_OWNERDRAW|MFT_RADIOCHECK|MFS_CHECKED : MFT_OWNERDRAW;
			if (bstrName.Length())
			{
				AddItem(ID_BTN_HISTORY+(i-m_aHistory.begin()), bstrName, -1);
				cMenu.AppendMenu(uFlags, ID_BTN_HISTORY+(i-m_aHistory.begin()), LPCTSTR(NULL));
			}
			else
			{
				if (bstrGen.m_str == NULL)
					CMultiLanguageString::GetLocalized(L"[0409]Previous settings[0405]Předchozí nastavení", m_tLocaleID, &bstrGen);
				AddItem(ID_BTN_HISTORY+(i-m_aHistory.begin()), bstrGen, -1);
				cMenu.AppendMenu(uFlags, ID_BTN_HISTORY+(i-m_aHistory.begin()), LPCTSTR(NULL));
				break;
			}
		}

		RECT rc;
		m_wndHistory.GetWindowRect(&rc);
		TPMPARAMS tTPMP = { sizeof(TPMPARAMS), rc };
		UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, rc.left, rc.top, m_hWnd, &tTPMP);
		if (nSelection != 0)
		{
			CopyConfigValues(m_pConfig, nSelection == ID_BTN_HISTORY-1 ? pDef.p : m_aHistory[nSelection-ID_BTN_HISTORY]);
		}
		return 0;
	}

	LRESULT OnSubMenuClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CMenu cMenu;
		cMenu.CreatePopupMenu();
		Reset(NULL);
		CComBSTR bstrTmp;
		CMultiLanguageString::GetLocalized(L"[0409]Cancel - stop processing[0405]Storno - ukončit zpracování", m_tLocaleID, &bstrTmp);
		AddItem(10, bstrTmp, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, 10, LPCTSTR(NULL));
		bstrTmp.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Skip - continue with next step[0405]Přeskočit - pokračovat dalším krokem", m_tLocaleID, &bstrTmp);
		AddItem(11, bstrTmp, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, 11, LPCTSTR(NULL));

		RECT rc;
		::GetWindowRect(m_hWndCancel, &rc);
		TPMPARAMS tTPMP = { sizeof(TPMPARAMS), rc };
		UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, rc.left, rc.bottom, m_hWnd, &tTPMP);
		if (nSelection != 0)
		{
			OnOKOrCancel(BN_CLICKED, nSelection == 10 ? IDCANCEL : t_wSkipID, m_hWndCancel, bHandled);
		}
		return 0;
	}
	LRESULT OnClickedHelp(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& a_bHandled)
	{
		if (m_bstrHelpTopic.Length())
		{
			COLE2CT pszHelpTopic(m_bstrHelpTopic);
			if (_tcsncmp(pszHelpTopic, _T("::/"), 3) == 0)
			{
				TCHAR szHelpPath[MAX_PATH*2] = _T("");
				GetHelpFilePath(szHelpPath, itemsof(szHelpPath), m_tLocaleID);
				_tcscat(szHelpPath, pszHelpTopic);
				HtmlHelp(m_hWnd, szHelpPath, HH_DISPLAY_TOPIC, 0);
			}
			else
			{
				HtmlHelp(m_hWnd, pszHelpTopic, HH_DISPLAY_TOPIC, 0);
			}
		}
		else
		{
			a_bHandled = FALSE;
		}
		return 0;
	}
	LRESULT OnUpdatePreview(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_pPreviewMaker && m_bPreview)
		{
			CWaitCursor cWait;
			CComPtr<IDocument> pDoc;
			m_pPreviewMaker->MakePreview(m_hWnd, m_tLocaleID, &pDoc);

			if (pDoc)
			{
				RECT rcO;
				m_wndOriginal.GetWindowRect(&rcO);
				RECT rc;
				m_wndPreview.GetWindowRect(&rc);
				rc.right = rcO.right-rc.left;
				rc.left = 0;
				rc.bottom -= rc.top;
				rc.top = 0;
				CConfigValue cPreview;
				m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_PREVIEW), &cPreview);
				CComPtr<IConfig> pPreview;
				m_pDlgSizeCfg->SubConfigGet(CComBSTR(CFGID_CFG_PREVIEW), &pPreview);
				CComPtr<IDesignerView> pWnd;
				m_pViewManager->CreateWnd(m_pViewManager, cPreview, pPreview, &m_cHelper, &m_cHelper, pDoc, m_wndPreview, &rc, EDVWSBorder, m_tLocaleID, &pWnd);
				if (m_pPreviewWnd)
					m_pPreviewWnd->Destroy();
				m_pPreviewWnd = pWnd;
			}
			else
			{
				// TODO: report error
			}
		}
		return 0;
	}
	template<LONG t_eDisplayMode>
	LRESULT OnDisplayMode(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_eDisplayMode != t_eDisplayMode)
		{
			m_eDisplayMode = t_eDisplayMode;
			RECT rc;
			GetClientRect(&rc);
			UpdatePreviews(rc.right, rc.bottom);
			CComBSTR cCFGID_CFG_DISPLAYMODE(CFGID_CFG_DISPLAYMODE);
			m_pDlgSizeCfg->ItemValuesSet(1, &(cCFGID_CFG_DISPLAYMODE.m_str), CConfigValue(m_eDisplayMode));
			if (m_eDisplayMode == CFGVAL_DM_BOTH)
				Invalidate();
		}
		return 0;
	}
	void UpdatePreviews(LONG a_nSizeX, LONG a_nSizeY)
	{
		if (!m_bPreview)
			return;
		RECT rc = {m_szMargins.cx, m_szMargins.cy, a_nSizeX-m_szConfig.cx, m_bPreviewSplit ? a_nSizeY-m_nSeparator-m_szMargins.cy-12*m_fScale+0.5f : a_nSizeY-m_nSeparator-m_szMargins.cy};
		LONG nSep = m_fPreviewLinePos*rc.left + (1.0f-m_fPreviewLinePos)*rc.right + 0.5f;
		RECT rcP = {rc.left, rc.top, nSep, rc.bottom};
		m_wndPreview.MoveWindow(&rcP, FALSE);
		if (nSep > rc.left)
		{
			if (m_pPreviewWnd)
			{
				RECT const rcInt = {0, 0, rc.right-rc.left, rc.bottom-rc.top};
				m_pPreviewWnd->Move(&rcInt);
			}
		}
		RECT rcO = {nSep, rc.top, rc.right, rc.bottom};
		m_wndOriginal.MoveWindow(&rcO, FALSE);
		if (nSep < rc.right)
		{
			RECT const rcInt = {rc.left-nSep, 0, rc.right-nSep, rc.bottom-rc.top};
			if (m_pOriginalWnd == NULL)
			{
				CConfigValue cPreview;
				m_pDlgSizeCfg->ItemValueGet(CComBSTR(CFGID_CFG_PREVIEW), &cPreview);
				CComPtr<IConfig> pPreview;
				m_pDlgSizeCfg->SubConfigGet(CComBSTR(CFGID_CFG_PREVIEW), &pPreview);

				CComPtr<IDocumentBase> pBase;
				RWCoCreateInstance(pBase, __uuidof(DocumentBase));
				GUID tID = GUID_NULL;
				m_pDoc->DocumentCopy(NULL, pBase, &tID, NULL);
				CComQIPtr<IDocument> pOrigCopy(pBase); // using copy, because it may be smaller than the processed

				m_pViewManager->CreateWnd(m_pViewManager, cPreview, pPreview, &m_cHelper, &m_cHelper, pOrigCopy ? pOrigCopy : m_pDoc, m_wndOriginal, &rcInt, EDVWSBorder, m_tLocaleID, &m_pOriginalWnd);
				m_pOriginalWnd->Show(TRUE);
			}
			else if (m_pOriginalWnd)
				m_pOriginalWnd->Move(&rcInt);
		}
		if (m_bPreviewSplit)
		{
			rc.top = rc.bottom;
			rc.bottom += 12*m_fScale+0.5f;
			m_wndSplitSlider.MoveWindow(&rc);
		}
	}

	LRESULT OnHelpLinkClick(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		PNMLINK pNMLink = reinterpret_cast<PNMLINK>(a_pNMHDR);
		ShellExecute(NULL, _T("open"), COLE2CT(pNMLink->item.szUrl), NULL, NULL, SW_SHOW);
		return 0;
	}

private:
	class ATL_NO_VTABLE CHelper :
		public CComObjectRootEx<CComMultiThreadModel>,
		public CSubjectImpl<ISharedStateManager, ISharedStateObserver, TSharedStateChange>,
		public IStatusBarObserver
	{
	public:
	BEGIN_COM_MAP(CHelper)
		COM_INTERFACE_ENTRY(ISharedStateManager)
		COM_INTERFACE_ENTRY(IStatusBarObserver)
	END_COM_MAP()

		STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
		{
			CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
			if (i == m_cStates.end())
				return E_RW_ITEMNOTFOUND;
			return i->second->QueryInterface(a_iid, a_ppState);
		}
		STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState *a_pState)
		{
			m_cStates[a_bstrCategoryName] = a_pState;
			TSharedStateChange tChange;
			tChange.bstrName = a_bstrCategoryName;
			tChange.pState = a_pState;
			Fire_Notify(tChange);
			return S_OK;
		}

		STDMETHOD(Notify)(TCookie a_tCookie, BYTE a_nDummy)
		{
			return S_FALSE;
		}

	private:
		typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

	private:
		CStates m_cStates;
	};

	class CSplitSlider :
		public CWindowImpl<CSplitSlider>,
		public CThemeImpl<CSplitSlider>
	{
	public:
		CSplitSlider() : m_fPos(0.5f)
		{
			SetThemeClassList(L"TrackBar");
		}
		void Init(float a_fPos, HFONT a_hFont, LCID a_tLocaleID)
		{
			m_fPos = a_fPos;
			m_hFont = a_hFont;
			CMultiLanguageString::GetLocalized(L"[0409]Processed[0405]Výsledek", a_tLocaleID, &m_bstrProcessed);
			CMultiLanguageString::GetLocalized(L"[0409]Original[0405]Originál", a_tLocaleID, &m_bstrOriginal);
		}

		static ATL::CWndClassInfo& GetWndClassInfo()
		{
			static ATL::CWndClassInfo wc =
			{
				{ sizeof(WNDCLASSEX), CS_VREDRAW | CS_HREDRAW | CS_OWNDC, StartWindowProc, 0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_3DFACE + 1), NULL, _T("SplitSlider"), NULL },
				NULL, NULL, IDC_SIZEWE, TRUE, 0, _T("")
			};
			return wc;
		}

		BEGIN_MSG_MAP(CSplitSlider)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
			CHAIN_MSG_MAP(CThemeImpl<CSplitSlider>)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		END_MSG_MAP()

		// operations
	public:
		void SetPos(float a_fPos)
		{
			if (m_fPos != a_fPos)
			{
				m_fPos = a_fPos;
				if (m_hWnd)
					Invalidate(FALSE);
			}
		}
		float GetPos()
		{
			return m_fPos;
		}

		// handlers
	public:
		LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			//HDC hDC = GetDC();
			//m_nHalfSize = (GetDeviceCaps(hDC, LOGPIXELSY)+12)/24;
			//ReleaseDC(hDC);
			return 0;
		}

		LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			return 0;
		}

		LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			if (GetCapture() == m_hWnd)
				ReleaseCapture();
			return 0;
		}

		void Moved(int nX)
		{
			RECT r;
			GetClientRect(&r);
			int nW = r.right-r.left;
			if (nW <= 2)
				return; // control is too narrow
			float fPos = 1.0f-nX/float(nW-1);
			if (fPos < 0.0f) fPos = 0.0f; else if (fPos > 1.0f) fPos = 1.0f;
			if (fPos != m_fPos)
			{
				r.left = (1.0f-m_fPos)*nW-0.5f-2;
				r.right = r.left+2+4;
				m_fPos = fPos;
				LONG nL = (1.0f-m_fPos)*nW-0.5f-2; if (nL < r.left) r.left = nL;
				LONG nR = nL+2+4; if (nR > r.right) r.right = nR;
				InvalidateRect(&r, FALSE);
				UpdateWindow();
				SendNotification();
			}
		}
		LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			Moved(GET_X_LPARAM(a_lParam));
			SetCapture();
			return 0;
		}

		LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			if (GetCapture() == m_hWnd)
				Moved(GET_X_LPARAM(a_lParam));
			return 0;
		}

		LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			return 1;
		}

		LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			RECT r;
			PAINTSTRUCT ps;
			HDC hDC = ::BeginPaint(m_hWnd, &ps);
			GetClientRect(&r);

			COLORREF clr3D = GetSysColor(COLOR_3DFACE);
			float f3DR = powf(GetRValue(clr3D)/255.0f, 2.2f);
			float f3DG = powf(GetGValue(clr3D)/255.0f, 2.2f);
			float f3DB = powf(GetBValue(clr3D)/255.0f, 2.2f);
			COLORREF clr1 = RGB(int(powf(f3DR*0.9f, 1.0f/2.2f)*255.0f+0.5f), int(powf(f3DG*0.9f, 1.0f/2.2f)*255.0f+0.5f), int(powf(f3DB*0.9f, 1.0f/2.2f)*255.0f+0.5f));
			COLORREF clr2 = RGB(int(powf(f3DR*0.9f+0.1f, 1.0f/2.2f)*255.0f+0.5f), int(powf(f3DG*0.9f+0.1f, 1.0f/2.2f)*255.0f+0.5f), int(powf(f3DB*0.9f+0.1f, 1.0f/2.2f)*255.0f+0.5f));

			COLORREF clrText = SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
			int nBkMode = SetBkMode(hDC, TRANSPARENT);
			HGDIOBJ hFont = SelectObject(hDC, m_hFont);

			RECT rc = r;
			rc.right = int((1.0f-m_fPos)*(r.right-r.left)+0.5f);
			HBRUSH hb = CreateSolidBrush(clr1);
			FillRect(hDC, &rc, hb);
			DeleteObject(hb);
			rc.left = rc.right;
			rc.right = r.right;
			hb = CreateSolidBrush(clr2);
			FillRect(hDC, &rc, hb);
			DeleteObject(hb);
			rc.right = (r.left+r.right)>>1;
			DrawText(hDC, m_bstrProcessed, -1, &r, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
			rc.left = rc.right;
			rc.right = r.right;
			DrawText(hDC, m_bstrOriginal, -1, &r, DT_SINGLELINE|DT_RIGHT|DT_VCENTER);

			SetTextColor(hDC, clrText);
			SetBkMode(hDC, nBkMode);
			SelectObject(hDC, hFont);

			rc = r;
			++rc.top;
			--rc.bottom;
			rc.left = int((1.0f-m_fPos)*(r.right-r.left)+0.5f)-1;
			rc.right = rc.left+1;
			--rc.bottom;
			FillRect(hDC, &rc, (HBRUSH)(COLOR_3DHIGHLIGHT+1));
			SetPixel(hDC, rc.left, rc.bottom, GetSysColor(COLOR_3DSHADOW));
			++rc.top;
			++rc.bottom;
			++rc.left;
			++rc.right;
			FillRect(hDC, &rc, (HBRUSH)(COLOR_3DSHADOW+1));
			SetPixel(hDC, rc.left, rc.top-1, GetSysColor(COLOR_3DHIGHLIGHT));

			::EndPaint(m_hWnd, &ps);
			return 0;
		}

		LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			int nDelta = GET_WHEEL_DELTA_WPARAM(wParam);

			return 0;
		}

	private:
		void SendNotification()
		{
			HWND hPar = GetParent();
			SendMessage(hPar, WM_HSCROLL, SB_THUMBTRACK, (LPARAM)&m_hWnd);
		}

	private:
		float m_fPos;
		HFONT m_hFont;
		CComBSTR m_bstrProcessed;
		CComBSTR m_bstrOriginal;
	};

private:
	CComPtr<IConfigWnd> m_pConfigWnd;
	CComPtr<IConfig> m_pConfig;
	CComPtr<IConfig> m_pDlgSizeCfg;
	CComPtr<IViewManager> m_pViewManager;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDesignerView> m_pOriginalWnd;
	CComPtr<IDesignerView> m_pPreviewWnd;
	CComObjectStackEx<CHelper> m_cHelper;
	IPreviewMaker* m_pPreviewMaker;
	bool m_bSaveSize;
	bool m_bSaveSplit;
	bool m_bPreview;
	bool m_bPreviewSplit;
	float m_fPreviewLinePos; // 0 = processed, 1 = preview
	CComBSTR m_bstrHelpTopic;
	HICON m_hIcon;
	HICON m_hIconLarge;
	CButtonWithPopup m_wndCancel;
	HWND m_hWndCancel;
	CStatic m_wndIcon;
	CStatic m_wndHelp;
	CButton m_wndOK;
	CWindow m_wndGripper;
	CImageList m_cImageList;
	CToolBarCtrl m_wndHistory;
	CWindow m_wndSepBot;
	SIZE m_szConfig;
	SIZE m_szMargins;
	ULONG m_nSeparator;
	CWindow m_wndOriginal;
	CWindow m_wndPreview;
	CFont m_cHelpFont;
	CSplitSlider m_wndSplitSlider;
	float m_fScale;

	bool m_bHistory;
	std::vector<CComPtr<IConfig> > m_aHistory;
	CComQIPtr<IConfigInMemory> m_pHistory;
	std::vector<CComBSTR> m_aIDs;
	CComPtr<IUnknown> m_pMgr;
	CComPtr<IConfigDescriptor> m_pAssistant;
};


