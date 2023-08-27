// DesignerViewTab.cpp : Implementation of CDesignerViewTab

#include "stdafx.h"
#include "DesignerViewTab.h"

#include "ConfigIDsTab.h"
#include <XPGUI.h>
#include <RWConceptDesignerExtension.h>
#include <MultiLanguageString.h>
#include <Win32LangEx.h>
#include <ContextHelpDlg.h>
#include <StringParsing.h>


// CDesignerViewTab

typedef void (__stdcall fnEnumModuleCallback) (void* a_pContext, LPCWSTR a_pszName, ULONG a_nVer1, ULONG a_nVer2);
typedef void (STDAPICALLTYPE fnEnumModules)(void* a_pContext, fnEnumModuleCallback* a_pfnCallback);

extern __declspec(selectany) fnEnumModules* s_pfnEnumModules = NULL;

void EnumModules(void* a_pContext, fnEnumModuleCallback* a_pfnCallback)
{
	if (s_pfnEnumModules == NULL)
	{
		HMODULE hMod = GetModuleHandle(NULL);
#ifdef WIN64
		s_pfnEnumModules = (fnEnumModules*) GetProcAddress(hMod, "RWEnumModules");
#else
		s_pfnEnumModules = (fnEnumModules*) GetProcAddress(hMod, "_RWEnumModules@8");
#endif
		if (s_pfnEnumModules == NULL)
			return;
	}
	return (*s_pfnEnumModules)(a_pContext, a_pfnCallback);
}

void __stdcall CheckModule(void* a_pContext, LPCWSTR a_pszName, ULONG a_nVer1, ULONG a_nVer2)
{
	std::pair<LPCOLESTR, bool>& t = *reinterpret_cast<std::pair<LPCOLESTR, bool>*>(a_pContext);
	if (!t.second)
	{
		int n1 = wcslen(t.first);
		int n2 = wcslen(a_pszName);
		if (n1+4 == n2 && 0 == _wcsnicmp(a_pszName, t.first, n1))
			t.second = true;
	}
}

void CDesignerViewTab::Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IViewManager *a_pViewManager, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID)
{
	m_pConfig = a_pConfig;
	m_pFrame = a_pFrame;
	m_pStatusBar = a_pStatusBar;
	m_pViewManager = a_pViewManager;
	m_pDocument = a_pDoc;
	m_tLocaleID = a_tLocaleID;

	DWORD dwExBorderStyle = (a_nStyle&EDVWSBorderMask) == EDVWSBorder ? WS_EX_CLIENTEDGE : ((a_nStyle&EDVWSBorderMask) == EDVWSNoBorder ? 0 : WS_EX_STATICEDGE);

	// create self
	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("Tab View"), WS_CHILDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, dwExBorderStyle|WS_EX_CONTROLPARENT) == NULL)
	{
		// creation failed
		throw E_FAIL;
	}

	COLORREF clrBtnFace = GetSysColor(COLOR_3DFACE);
	int nRed = GetRValue(clrBtnFace);
	int nGreen = GetGValue(clrBtnFace);
	int nBlue = GetBValue(clrBtnFace);

	int nMax = (nRed > nGreen) ? ((nRed > nBlue) ? nRed : nBlue) : ((nGreen > nBlue) ? nGreen : nBlue);
	const BYTE nMagicBackgroundOffset = (nMax > (0xFF - 35)) ? (BYTE)(0xFF - nMax) : (BYTE)35;
	if(nMax == 0)
	{
		nRed = (BYTE)(nRed + nMagicBackgroundOffset);
		nGreen = (BYTE)(nGreen + nMagicBackgroundOffset);
		nBlue = (BYTE)(nBlue + nMagicBackgroundOffset);
	}
	else
	{
		nRed = (BYTE)(nRed + (nMagicBackgroundOffset*(nRed/(double)nMax) + 0.5));
		nGreen = (BYTE)(nGreen + (nMagicBackgroundOffset*(nGreen/(double)nMax) + 0.5));
		nBlue = (BYTE)(nBlue + (nMagicBackgroundOffset*(nBlue/(double)nMax) + 0.5));
	}

	m_hClientBG.CreateSolidBrush(m_clrClientBG = RGB(nRed, nGreen, nBlue));

	CConfigValue cTabPos;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_TABS_HEADERPOS), &cTabPos);
	m_dwTabFlags = cTabPos.operator LONG();

	CConfigValue cInvisible;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_TABS_INVISIBLE), &cInvisible);
	m_bInvisibleTab = cInvisible;

	m_wndTab.Create(m_hWnd, rcDefault, _T("Tab"), (m_bInvisibleTab ? 0 : WS_VISIBLE)|WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_NOEDGE|m_dwTabFlags, 0, IDC_TAB);
	RECT rcTab;
	RECT rcWindow;
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		SplitArea(rcClient, &rcTab, &rcWindow, m_wndTab);
	}
	m_wndTab.MoveWindow(&rcTab);

	m_wndReactivate.Create(m_hWnd, NULL, NULL, WS_CHILD|BS_OWNERDRAW, 0, IDC_BUTTON);

	m_nIconSize = XPGUI::GetSmallIconSize();
	m_cIcons.Create(m_nIconSize, m_nIconSize, XPGUI::GetImageListColorFlags(), 4, 4);

	CComPtr<IDesignerFrameIcons> pIconsManager;
	RWCoCreateInstance(pIconsManager, __uuidof(DesignerFrameIconsManager));

	m_wndTab.SetImageList(m_cIcons);

	CConfigValue cTabCount;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_TABS_ITEMS), &cTabCount);

	m_vpWnd.resize(cTabCount.operator LONG());

	LONG i;
	for (i = 0; i < cTabCount.operator LONG(); ++i)
	{
		OLECHAR szID[64];
		swprintf(szID, L"%s\\%08x", CFGID_TABS_ITEMS, i);
		CConfigValue cViewName;
		m_pConfig->ItemValueGet(CComBSTR(szID), &cViewName);
		LPOLESTR pszSub = szID+wcslen(szID);
		wcscpy(pszSub, L"\\Condition");
		CConfigValue cCondition;
		m_pConfig->ItemValueGet(CComBSTR(szID), &cCondition);
		if (cCondition.operator BSTR()[0])
		{
			ULONG n = SysStringLen(cCondition);
			bool bGUID = false;
			GUID tID = GUID_NULL;
			if (n == 36)
				bGUID = GUIDFromString(cCondition.operator BSTR(), &tID);
			else if (n == 38 && cCondition.operator BSTR()[0] == L'{' && cCondition.operator BSTR()[37] == L'}')
				bGUID = GUIDFromString(cCondition.operator BSTR()+1, &tID);
			if (bGUID)
			{
				CComPtr<IUnknown> p;
				RWCoCreateInstance(p, tID);
				if (p == NULL)
					continue;
			}
			else
			{
				std::pair<LPCOLESTR, bool> t;
				t.first = cCondition;
				t.second = false;
				EnumModules(&t, CheckModule);
				if (!t.second)
					continue;
			}
		}
		wcscpy(pszSub, L"\\IconID");
		CConfigValue cViewIcon;
		m_pConfig->ItemValueGet(CComBSTR(szID), &cViewIcon);
		HICON hIcon = NULL;
		int iIcon = -1;
		if (!IsEqualGUID(cViewIcon, GUID_NULL) && SUCCEEDED(pIconsManager->GetIcon(cViewIcon, m_nIconSize, &hIcon)) && hIcon != NULL)
		{
			m_cIcons.AddIcon(hIcon);
			iIcon = m_cIcons.GetImageCount()-1;
			DestroyIcon(hIcon);
		}
		CComBSTR bstrLoc;
		CMultiLanguageString::GetLocalized(cViewName.operator BSTR(), m_tLocaleID, &bstrLoc);
		int iTab = m_wndTab.InsertItem(m_wndTab.GetItemCount(), COLE2CT(bstrLoc.m_str), iIcon);
		m_cCfgToTab[i] = iTab;
		m_cTabToCfg[iTab] = i;
	}
	m_cCfgToTab[-1] = -1;
	m_cTabToCfg[-1] = -1;
	m_bTabActive = true;
	CConfigValue cActiveIndex;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_TABS_ACTIVEINDEX), &cActiveIndex);
	CIIMap::const_iterator iSel = m_cCfgToTab.find(cActiveIndex);
	if (cTabCount.operator LONG() > 0)
	{
		if (iSel == m_cCfgToTab.end())
			m_wndTab.SetCurSel(0);
		else
		{
			m_wndTab.SetCurSel(iSel->second);
			if (iSel->second == -1)
			{
				m_wndReactivate.ShowWindow(SW_SHOW);
				m_wndTab.ShowWindow(SW_HIDE);
			}
		}
	}
	if (!m_bInvisibleTab && m_cCfgToTab.size() == 2)
	{
		CConfigValue cVal;
		m_pConfig->ItemValueGet(CComBSTR(CFGID_TABS_TABID), &cVal);
		if (cVal.operator BSTR() && *cVal.operator BSTR())
			m_wndTab.ModifyStyle(0, CTCS_CLOSEBUTTON);
	}

	ShowWindow(SW_SHOW);
}

STDMETHODIMP CDesignerViewTab::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	int nPos = m_wndTab.GetCurSel();
	if (nPos >= 0 && ULONG(nPos) < m_vpWnd.size())
	{
		if (S_OK == m_vpWnd[nPos]->PreTranslateMessage(a_pMsg, a_bBeforeAccel))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CDesignerViewTab::OnIdle()
{
	int nPos = m_wndTab.GetCurSel();
	if (nPos >= 0 && ULONG(nPos) < m_vpWnd.size())
	{
		return m_vpWnd[nPos]->OnIdle();
	}
	else
	{
		return S_FALSE;
	}
}

STDMETHODIMP CDesignerViewTab::OnDeactivate(BOOL a_bCancelChanges)
{
	int nPos = m_wndTab.GetCurSel();
	if (nPos >= 0 && ULONG(nPos) < m_vpWnd.size())
	{
		return m_vpWnd[nPos]->OnDeactivate(a_bCancelChanges);
	}
	else
	{
		return S_FALSE;
	}
}

STDMETHODIMP CDesignerViewTab::QueryInterfaces(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
{
	try
	{
		CComPtr<IUnknown> pThis;
		QueryInterface(a_iid, reinterpret_cast<void**>(&pThis));
		if (pThis)
			a_pInterfaces->Insert(pThis);

		int nPos = m_wndTab.GetCurSel();
		if (nPos >= 0 && ULONG(nPos) < m_vpWnd.size())
		{
			m_vpWnd[nPos]->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
		}
		if (a_eFilter == EQIFAll)
		{
			for (ULONG i = 0; i < m_vpWnd.size(); ++i)
				if (i != nPos && m_vpWnd[i])
					m_vpWnd[i]->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewTab::OptimumSize(SIZE* a_pSize)
{
	int nPos = m_wndTab.GetCurSel();
	if (nPos >= 0 && ULONG(nPos) < m_vpWnd.size())
	{
		if (m_vpWnd.size() > 1)
			return E_NOTIMPL;
		if (m_bInvisibleTab)
			return m_vpWnd[nPos]->OptimumSize(a_pSize);
		SIZE sz = {a_pSize->cx, a_pSize->cy-m_tabHeight};
		HRESULT hRes = m_vpWnd[nPos]->OptimumSize(&sz);
		if (sz.cy > 0) sz.cy += m_tabHeight;
		*a_pSize = sz;
		return hRes;
	}
	a_pSize->cx = a_pSize->cy = m_bInvisibleTab ? 0 : m_buttonWidth;
	return S_OK;
}

//LRESULT CDesignerViewTab::OnPaint(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
//{
//	int nPos = m_wndTab.GetCurSel();
//	if (nPos < 0)
//	{
//		a_bHandled = FALSE;
//		return 0;
//	}
//	PAINTSTRUCT ps;
//	CDCHandle cDC(BeginPaint(&ps));
//
//	RECT rc = {0, 0, 100, 100};
//	cDC.FillRect(&rc, COLOR_3DSHADOW);
//
//	EndPaint(&ps);
//	return 0;

		//void DrawLabel(CDC& dc,const CRect& rc,const CSide& side) const
		//{
		//	CRect rcOutput(rc);
		//	rcOutput.DeflateRect(captionPadding,captionPadding);
		//	if(m_icon!=NULL)
		//	{
		//		CDWSettings settings;
		//		CSize  szIcon(settings.CXMinIcon(),settings.CYMinIcon());
		//		if(side.IsHorizontal())
		//		{
		//			if(rc.Width()>szIcon.cx+2*captionPadding)
		//			{
		//				POINT pt={rcOutput.left,rc.top+(rc.Height()-szIcon.cx)/2};
		//				rcOutput.left+=szIcon.cx+captionPadding;
		//				dc.DrawIconEx(pt,m_icon,szIcon);
		//			}
		//		}
		//		else
		//		{
		//			if(rc.Height()>szIcon.cy+2*captionPadding)
		//			{
		//				POINT pt={rc.left+(rc.Width()-szIcon.cy)/2,rcOutput.top};
		//				rcOutput.top+=szIcon.cy+captionPadding;
		//				dc.DrawIconEx(pt,m_icon,szIcon);
		//			}
		//		}
		//	}
		//	DrawEllipsisText(dc,m_txt,_tcslen(m_txt),&rcOutput,side.IsHorizontal());
		//}
//}

LRESULT CDesignerViewTab::OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (m_wndTab.m_hWnd)
	{
		RECT rcTab;
		RECT rcWindow;
		{
			RECT rcClient = {0, 0, LOWORD(a_lParam), HIWORD(a_lParam)};
			SplitArea(rcClient, &rcTab, &rcWindow, m_wndTab);
		}
		m_wndTab.MoveWindow(&rcTab);

		std::vector<CComPtr<IDesignerView> >::const_iterator i;
		for (i = m_vpWnd.begin(); i != m_vpWnd.end(); ++i)
		{
			if (*i != NULL)
			{
				(*i)->Move(&rcWindow);
			}
		}
	}

	if (m_wndReactivate.m_hWnd)
	{
		RECT rc = {0, 0, LOWORD(a_lParam), HIWORD(a_lParam)};
		m_wndReactivate.MoveWindow(&rc);
	}

	return 0;
}

LRESULT CDesignerViewTab::OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	int nPos = m_wndTab.GetCurSel();
	if (nPos >= 0 && ULONG(nPos) < m_vpWnd.size())
	{
		RWHWND rwhWnd = NULL;
		if (m_vpWnd[nPos] != NULL && SUCCEEDED(m_vpWnd[nPos]->Handle(&rwhWnd)) && rwhWnd != NULL)
		{
			::SetFocus(rwhWnd);
		}
	}

	return 0;
}

LRESULT CDesignerViewTab::OnTabChange(int UNREF(a_idCtrl), LPNMHDR a_pnmh, BOOL& UNREF(a_bHandled))
{
	try
	{
		if (!m_bTabActive)
			return 0;
		size_t iNew = m_wndTab.GetCurSel();
		if (m_iActiveView == iNew)
		{
			if (iNew < m_vpWnd.size() && m_vpWnd[iNew] != NULL && GetWindowLong(GWL_STYLE)&WS_VISIBLE)
			{
				RWHWND rwhWnd = NULL;
				m_vpWnd[iNew]->Handle(&rwhWnd);
				::SetFocus(rwhWnd);
			}
			return 0;
		}

		if (m_iActiveView >= 0)
		{
			m_vpWnd[m_iActiveView]->OnDeactivate(FALSE);
			m_vpWnd[m_iActiveView]->Show(FALSE);
		}

		CComBSTR cActiveIndex(CFGID_TABS_ACTIVEINDEX);
		m_pConfig->ItemValuesSet(1, &(cActiveIndex.m_str), CConfigValue(m_cTabToCfg[iNew]));

		if (iNew >= m_vpWnd.size())
		{
			m_iActiveView = -1;
			return 0;
		}

		if (m_vpWnd[iNew] == NULL)
		{
			RECT rcClient;
			GetClientRect(&rcClient);
			RECT rcTab;
			RECT rcWindow;
			SplitArea(rcClient, &rcTab, &rcWindow, m_wndTab);

			OLECHAR szID[64];
			swprintf(szID, L"%s\\%08x\\%s", CFGID_TABS_ITEMS, int(m_cTabToCfg[iNew]), CFGID_TABS_VIEW);
			CComPtr<IConfig> pSubCfg;
			m_pConfig->SubConfigGet(CComBSTR(szID), &pSubCfg);
			CConfigValue cViewType;
			m_pConfig->ItemValueGet(CComBSTR(szID), &cViewType);
			m_pViewManager->CreateWnd(m_pViewManager, cViewType, pSubCfg, m_pFrame, m_pStatusBar, m_pDocument, m_hWnd, &rcWindow, EDVWSNoBorder, m_tLocaleID, &(m_vpWnd[iNew]));
		}
		else
		{
			m_vpWnd[iNew]->Show(TRUE);
		}

		if (m_vpWnd[iNew] != NULL)
		{
			if (GetWindowLong(GWL_STYLE)&WS_VISIBLE)
			{
				RWHWND rwhWnd = NULL;
				m_vpWnd[iNew]->Handle(&rwhWnd);
				::SetFocus(rwhWnd);
			}
			m_iActiveView = iNew;
		}
	}
	catch (...)
	{
	}

	return 0;
}

LRESULT CDesignerViewTab::OnTabClose(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled)
{
	ActiveIndexSet(-1);
	return 0;
}

class CTabConfigDlg :
	public Win32LangEx::CLangDialogImpl<CTabConfigDlg>,
	public CDialogResize<CTabConfigDlg>,
	public CContextHelpDlg<CTabConfigDlg>
{
public:
	CTabConfigDlg(IConfig* a_pConfig, LCID a_tLocaleID) :
		Win32LangEx::CLangDialogImpl<CTabConfigDlg>(a_tLocaleID), m_pMainConfig(a_pConfig),
		CContextHelpDlg<CTabConfigDlg>(_T("http://www.rw-designer.com/tabs-view"))
	{
	}

	enum { IDD = IDD_CONFIGFRAME };

	BEGIN_MSG_MAP(CTabConfigDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CTabConfigDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnEndDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnEndDialog)
		CHAIN_MSG_MAP(CDialogResize<CTabConfigDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CTabConfigDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CONFIGCONTROL, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CTabConfigDlg)
		//CTXHELP_CONTROL_RESOURCE(IDOK, IDS_HELP_IDOK)
		//CTXHELP_CONTROL_RESOURCE(IDCANCEL, IDS_HELP_IDCANCEL)
		//CTXHELP_CONTROL_RESOURCE(IDHELP, IDS_HELP_IDHELP)
	END_CTXHELP_MAP()

protected:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		CWindow wnd = GetDlgItem(IDC_CONFIGCONTROL);
		RECT rcWnd;
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);

		RWCoCreateInstance(m_pConfigWnd, __uuidof(TreeConfigWnd));
		m_pConfigWnd->Create(m_hWnd, &rcWnd, IDC_CONFIGCONTROL, m_tLocaleID, TRUE, ECWBMNothing);
		m_pConfigWnd->ConfigSet(m_pMainConfig, ECPMFull);

		wnd.DestroyWindow();

		DlgResize_Init();

		CenterWindow(GetParent());

		return TRUE;
	}

	LRESULT OnEndDialog(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND UNREF(a_hWndCtl), BOOL& UNREF(bHandled))
	{
		EndDialog(a_wID);
		return 0;
	}

private:
	CComPtr<IConfig> m_pMainConfig;
	CComPtr<IConfigWnd> m_pConfigWnd;
};

LRESULT CDesignerViewTab::OnTabRClick(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled)
{
	try
	{
		NMCTCITEM* pData = reinterpret_cast<NMCTCITEM*>(a_pnmh);
		if (pData->iItem != m_iActiveView)
			return FALSE;

		Reset(m_cImageList);

		CMenu cMenu;
		cMenu.CreatePopupMenu();
		CComBSTR bstrConfigure;
		CMultiLanguageString::GetLocalized(L"[0409]Configure panel[0405]Konfigurovat panel", m_tLocaleID, &bstrConfigure);
		AddItem(cMenu, 1, COLE2CT(bstrConfigure), -1, 0);

		POINT pt = pData->pt;
		m_wndTab.ClientToScreen(&pt);
		TPMPARAMS tPMParams;
		ZeroMemory(&tPMParams, sizeof tPMParams);
		tPMParams.cbSize = sizeof tPMParams;
		UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD|TPM_VERPOSANIMATION, pt.x, pt.y, m_hWnd);
		if (nSelection != 0)
		{
			OLECHAR szID[64];
			swprintf(szID, L"%s\\%08x", CFGID_TABS_ITEMS, int(m_cTabToCfg[m_iActiveView]));
			CComPtr<IConfig> pSub;
			m_pConfig->SubConfigGet(CComBSTR(szID), &pSub);

			CComPtr<IConfig> pDup;
			pSub->DuplicateCreate(&pDup);
			CTabConfigDlg cDlg(pDup, m_tLocaleID);
			if (cDlg.DoModal(m_hWnd) == IDOK)
			{
				m_vpWnd[m_iActiveView]->Destroy();
				m_vpWnd[m_iActiveView] = NULL;
				CopyConfigValues(pSub, pDup);

				RECT rcClient;
				GetClientRect(&rcClient);
				RECT rcTab;
				RECT rcWindow;
				SplitArea(rcClient, &rcTab, &rcWindow, m_wndTab);

				CComPtr<IConfig> pSubCfg;
				pSub->SubConfigGet(CComBSTR(CFGID_TABS_VIEW), &pSubCfg);
				CConfigValue cViewType;
				pSub->ItemValueGet(CComBSTR(CFGID_TABS_VIEW), &cViewType);
				m_pViewManager->CreateWnd(m_pViewManager, cViewType, pSubCfg, m_pFrame, m_pStatusBar, m_pDocument, m_hWnd, &rcWindow, EDVWSNoBorder, m_tLocaleID, &(m_vpWnd[m_iActiveView]));
			}
		}
	}
	catch (...)
	{
	}
	return TRUE;
}

LRESULT CDesignerViewTab::OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	for (ULONG i = 0; i < m_vpWnd.size(); ++i)
	{
		RWHWND h = NULL;
		if (m_vpWnd[i]) m_vpWnd[i]->Handle(&h);
		if (h) ::SendMessage(h, a_uMsg, a_wParam, a_lParam);
	}
	m_wndTab.SendMessage(a_uMsg, a_wParam, a_lParam);
	return 0;
}

LRESULT CDesignerViewTab::OnRWGotFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	if (a_wParam != 0)
	{
		for (ULONG i = 0; i < m_vpWnd.size(); ++i)
		{
			RWHWND h = NULL;
			if (m_vpWnd[i]) m_vpWnd[i]->Handle(&h);
			if (h == reinterpret_cast<RWHWND>(a_wParam))
			{
				if (m_iActiveView != i)
					ActiveIndexSet(i);
				break;
			}
		}
	}
	GetParent().SendMessage(WM_RW_GOTFOCUS, a_wParam ? reinterpret_cast<WPARAM>(m_hWnd) : 0, a_lParam ? reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)) : 0);
	return 0;
}

void CDesignerViewTab::SplitArea(RECT const& a_rcClient, RECT* a_prcTab, RECT* a_prcWindow, CWindow& a_wndTab)
{
	HDC hDC = GetWindowDC();
	float fScale = GetDeviceCaps(hDC, LOGPIXELSX)/96.0f;
	ReleaseDC(hDC);

	m_tabHeight = fScale*26;
	m_buttonWidth = fScale*22;

	a_prcTab->left = a_prcWindow->left = a_rcClient.left;
	a_prcTab->right = a_prcWindow->right = a_rcClient.right;

	if (m_bInvisibleTab)
	{
		a_prcWindow->top = a_prcTab->top = a_prcTab->bottom = a_rcClient.top;
		a_prcWindow->bottom = a_rcClient.bottom;
	}
	else
	{
		if (m_dwTabFlags&CTCS_BOTTOM)
		{
			a_prcWindow->top = a_rcClient.top;
			a_prcTab->top = a_prcWindow->bottom = max(0, a_rcClient.bottom-m_tabHeight);
			a_prcTab->bottom = a_rcClient.bottom;
		}
		else
		{
			a_prcTab->top = a_rcClient.top;
			a_prcWindow->top = a_prcTab->bottom = a_rcClient.top+m_tabHeight;
			a_prcWindow->bottom = max(a_prcWindow->top, a_rcClient.bottom);
		}
	}
}

STDMETHODIMP CDesignerViewTab::TabID(BSTR* a_pbstrID)
{
	try
	{
		*a_pbstrID = NULL;
		CConfigValue cVal;
		m_pConfig->ItemValueGet(CComBSTR(CFGID_TABS_TABID), &cVal);
		*a_pbstrID = cVal.Detach().bstrVal;
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewTab::ActiveIndexGet(ULONG* a_pnIndex)
{
	try
	{
		*a_pnIndex = m_iActiveView;
		return S_OK;
	}
	catch (...)
	{
		return a_pnIndex ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewTab::ActiveIndexSet(ULONG a_nIndex)
{
	try
	{
		if (!m_wndTab.IsWindow())
			return E_FAIL;
		if (a_nIndex > ULONG(m_wndTab.GetItemCount()))
		{
			m_wndTab.ShowWindow(SW_HIDE);
			m_wndReactivate.ShowWindow(SW_SHOW);
		}
		else
		{
			m_wndReactivate.ShowWindow(SW_HIDE);
			m_wndTab.ShowWindow(SW_SHOW);
		}
		m_wndTab.SetCurSel(a_nIndex);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewTab::ItemCount(ULONG* a_pnCount)
{
	try
	{
		*a_pnCount = m_vpWnd.size();
		return S_OK;
	}
	catch (...)
	{
		return a_pnCount ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewTab::NameGet(ULONG a_nIndex, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		OLECHAR szID[64];
		swprintf(szID, L"%s\\%08x", CFGID_TABS_ITEMS, m_cTabToCfg[a_nIndex]);
		CConfigValue cViewName;
		m_pConfig->ItemValueGet(CComBSTR(szID), &cViewName);
		if (cViewName.TypeGet() != ECVTString)
			return E_FAIL;
		*a_ppName = new CMultiLanguageString(cViewName.Detach().bstrVal);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewTab::IconIDGet(ULONG a_nIndex, GUID* a_ptIconID)
{
	try
	{
		*a_ptIconID = GUID_NULL;
		OLECHAR szID[64];
		swprintf(szID, L"%s\\%08x\\IconID", CFGID_TABS_ITEMS, m_cTabToCfg[a_nIndex]);
		CConfigValue cViewIconID;
		m_pConfig->ItemValueGet(CComBSTR(szID), &cViewIconID);
		if (cViewIconID.TypeGet() != ECVTGUID)
			return E_FAIL;
		*a_ptIconID = cViewIconID;
		return S_OK;
	}
	catch (...)
	{
		return a_ptIconID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewTab::IconGet(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		OLECHAR szID[64];
		swprintf(szID, L"%s\\%08x\\IconID", CFGID_TABS_ITEMS, m_cTabToCfg[a_nIndex]);
		CConfigValue cViewIconID;
		m_pConfig->ItemValueGet(CComBSTR(szID), &cViewIconID);
		if (cViewIconID.TypeGet() != ECVTGUID)
			return E_FAIL;
		CComPtr<IDesignerFrameIcons> pIconsManager;
		RWCoCreateInstance(pIconsManager, __uuidof(DesignerFrameIconsManager));
		return pIconsManager->GetIcon(cViewIconID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

LRESULT CDesignerViewTab::OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
}

LRESULT CDesignerViewTab::OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	return reinterpret_cast<LRESULT>(m_hClientBG.m_hBrush);
}

LRESULT CDesignerViewTab::OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	if (a_wParam) SetBkColor((HDC)a_wParam, m_clrClientBG);
	return reinterpret_cast<LRESULT>(m_hClientBG.m_hBrush);
}

STDMETHODIMP CDesignerViewTab::Drag(IDataObject* UNREF(a_pDataObj), IEnumStrings* UNREF(a_pFileNames), DWORD UNREF(a_grfKeyState), POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** UNREF(a_ppFeedback))
{
	if (m_bInvisibleTab || m_wndTab.m_hWnd == NULL)
	{
		if (m_iWaitItem != -1)
			KillTimer(DRAGTIMERID);
		m_iWaitItem = -1;
		return E_FAIL;
	}
	CTCHITTESTINFO tCHT = {a_pt, CTCHT_NOWHERE};
	m_wndTab.ScreenToClient(&tCHT.pt);
	int iHit = m_wndTab.HitTest(&tCHT);
	if (tCHT.flags&CTCHT_ONITEM && iHit != m_wndTab.GetCurSel())
	{
		if (m_iWaitItem != iHit)
		{
			m_iWaitItem = iHit;
			m_dwWaitStart = GetTickCount();
			m_ptWaitPos = a_pt;
			SetTimer(DRAGTIMERID, 500);
		}
		else
		{
			if (abs(a_pt.x-m_ptWaitPos.x) > abs(GetSystemMetrics(SM_CXDRAG)) ||
				abs(a_pt.y-m_ptWaitPos.y) > abs(GetSystemMetrics(SM_CYDRAG)))
			{
				m_dwWaitStart = GetTickCount();
				m_ptWaitPos = a_pt;
				SetTimer(DRAGTIMERID, 500);
			}
		}
		return E_FAIL;
	}
	if (m_iWaitItem != -1)
		KillTimer(DRAGTIMERID);
	m_iWaitItem = -1;
	return E_FAIL;
}

LRESULT CDesignerViewTab::OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	KillTimer(DRAGTIMERID);
	CTCHITTESTINFO tCHT = {{0, 0}, CTCHT_NOWHERE};
	GetCursorPos(&tCHT.pt);
	m_wndTab.ScreenToClient(&tCHT.pt);
	int iHit = m_wndTab.HitTest(&tCHT);
	if (tCHT.flags&CTCHT_ONITEM && iHit == m_iWaitItem)
	{
		m_wndTab.SetCurSel(m_iWaitItem);
	}
	m_iWaitItem = -1;
	return 0;
}

LRESULT CDesignerViewTab::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	LPDRAWITEMSTRUCT lpItem = (LPDRAWITEMSTRUCT) lParam;
	CDC dc (lpItem->hDC);

	UINT uState = lpItem->itemState;
	CRect rcDraw = lpItem->rcItem;

	dc.FillRect(rcDraw, COLOR_3DFACE);
	UINT uFrameState = 0;

	// Draw text
	TCHAR const* szText = m_wndTab.GetItem(0)->GetText();
	int icon = m_wndTab.GetItem(0)->GetImageIndex();
	//GetWindowText(szText, 256);

	dc.SetBkColor(GetSysColor(COLOR_3DFACE));
	dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));

	LOGFONT lfIcon = { 0 };
	::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfIcon), &lfIcon, 0);
	HFONT hf;
	lfIcon.lfWeight = FW_BOLD;

	bool const vertical = rcDraw.Height() > rcDraw.Width();

	if (vertical)
	{
		SetGraphicsMode(dc, GM_ADVANCED);
		XFORM xform = {0, 1, -1, 0, rcDraw.Width(), 0};
		SetWorldTransform(dc, &xform);
		std::swap(rcDraw.top, rcDraw.left);
		std::swap(rcDraw.bottom, rcDraw.right);
	}

	HFONT newFont = CreateFontIndirect(&lfIcon);
	hf = dc.SelectFont(newFont);
	//dc.ExtTextOut(4+20+4, rcDraw.top, ETO_OPAQUE|ETO_CLIPPED, rcDraw, szText);
	CRect rcText = rcDraw;
	rcText.left += (m_nIconSize>>1)+m_nIconSize;
	rcText.right -= m_nIconSize;
	rcText.top = rcText.bottom-m_buttonWidth;
	dc.DrawText(szText, -1, rcText, DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
	DeleteObject(dc.SelectFont(hf));

	CPen penArrow;
	penArrow.CreatePen(PS_SOLID, 1, COLORREF(0));
	CPenHandle penOld = dc.SelectPen(penArrow);

	ULONG arrowSize = ((m_nIconSize>>1)-2)&~1; // make it odd
	RECT rcX = rcDraw;
	rcX.right -= (m_nIconSize>>1);
	rcX.left = rcX.right - arrowSize;
	rcX.top = rcDraw.bottom-m_buttonWidth+((m_buttonWidth-arrowSize)>>1);
	rcX.bottom = rcX.top + arrowSize;

	POINT mid = {(rcX.left+rcX.right)>>1, (rcX.top+rcX.bottom)>>1};
	dc.MoveTo(rcX.left, mid.y);
	dc.LineTo(mid.x, rcX.top);
	dc.LineTo(rcX.right, mid.y);
	dc.LineTo(rcX.right, mid.y+1);
	dc.LineTo(mid.x, rcX.top+1);
	dc.LineTo(rcX.left, mid.y+1);
	dc.LineTo(rcX.left, mid.y);

	dc.MoveTo(rcX.left, rcX.bottom);
	dc.LineTo(mid.x, mid.y);
	dc.LineTo(rcX.right, rcX.bottom);
	dc.LineTo(rcX.right, rcX.bottom+1);
	dc.LineTo(mid.x, mid.y+1);
	dc.LineTo(rcX.left, rcX.bottom+1);
	dc.LineTo(rcX.left, rcX.bottom);

	dc.SelectPen(penOld);

	m_cIcons.Draw(dc, icon, m_nIconSize>>2, rcDraw.bottom-m_buttonWidth+((m_buttonWidth-m_nIconSize)>>1), ILD_NORMAL);

	if (vertical)
	{
		XFORM xform = {1, 0, 0, 1, 0, 0};
		SetWorldTransform(dc, &xform);
		SetGraphicsMode(dc, GM_COMPATIBLE);
	}

	return 1;
}
