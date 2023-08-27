
#include "stdafx.h"
#include "RWViewAreaLayout.h"
#include "PanelCtrl.h"
#include <MultiLanguageString.h>
#include "DesignerViewRolldown.h"
#include <RWConceptDesignerExtension.h>
#include <XPGUI.h>
#include <Win32LangEx.h>
#include <ContextHelpDlg.h>


void CPanelCtrl::Init(CDesignerViewRolldown* a_pParent, LCID a_tLocaleID, IConfig* a_pItemCfg, BSTR a_bstrName, int a_nLeft, int a_nRight, int a_nTop, float a_fScale)
{
	m_pParent = a_pParent;
	m_pItemCfg = a_pItemCfg;
	m_fScale = a_fScale;
	m_tLocaleID = a_tLocaleID;

	m_hHand = LoadCursor(NULL, IDC_HAND);
	m_hArrow = LoadCursor(NULL, IDC_ARROW);
	m_hSizeNS = LoadCursor(NULL, IDC_SIZENS);
	if (m_hHand == NULL)
		m_hHand = m_hArrow;

	CConfigValue cIconID;
	m_pItemCfg->ItemValueGet(CComBSTR(CFGID_PANELS_ICONID), &cIconID);
	if (!IsEqualGUID(cIconID, GUID_NULL))
	{
		CComPtr<IDesignerFrameIcons> pIconsManager;
		RWCoCreateInstance(pIconsManager, __uuidof(DesignerFrameIconsManager));
		pIconsManager->GetIcon(cIconID, XPGUI::GetSmallIconSize(), &m_hIcon);
	}

	CComBSTR bstrLoc;
	CMultiLanguageString::GetLocalized(a_bstrName, m_pParent->m_tLocaleID, &bstrLoc);
	RECT rc = {a_nLeft, a_nTop, a_nRight, a_nTop+m_pParent->m_nHeaderSizeY+m_pParent->m_nBorderSize};

	Create(*a_pParent, &rc, COLE2CT(bstrLoc.m_str), 0, WS_EX_CONTROLPARENT);

	CConfigValue cCollapsed;
	m_pItemCfg->ItemValueGet(CComBSTR(CFGID_RD_COLLAPSED), &cCollapsed);
	m_bOpen = !cCollapsed.operator bool();
	CConfigValue cCustomHeight;
	m_pItemCfg->ItemValueGet(CComBSTR(CFGID_RD_CUSTOMHEIGHT), &cCustomHeight);
	if (m_bOpen || !cCustomHeight.operator bool())
	{
		CreateView();
	}
}

void CPanelCtrl::CreateView()
{
	CConfigValue cViewID;
	m_pItemCfg->ItemValueGet(CComBSTR(CFGID_RD_VIEWTYPE), &cViewID);
	CComPtr<IConfig> pViewCfg;
	m_pItemCfg->SubConfigGet(CComBSTR(CFGID_RD_VIEWTYPE), &pViewCfg);
	CConfigValue cCustomHeight;
	m_pItemCfg->ItemValueGet(CComBSTR(CFGID_RD_CUSTOMHEIGHT), &cCustomHeight);
	//CConfigValue cAdjustable;
	//m_pItemCfg->ItemValueGet(CComBSTR(CFGID_RD_ADJUSTABLE), &cAdjustable);
	m_bAdjustable = cCustomHeight.operator bool();
	RECT rc;
	GetClientRect(&rc);
	if (m_bAdjustable)
	{
		CConfigValue cHeight;
		m_pItemCfg->ItemValueGet(CComBSTR(CFGID_RD_HEIGHT), &cHeight);
		if (cHeight.operator float() > 0.0f)
			rc.bottom = m_nSizeY = static_cast<int>(cHeight.operator float()*m_fScale+0.5f);
	}
	if (!m_bOpen)
		rc.bottom = 0;
	m_pParent->m_pViewMgr->CreateWnd(m_pParent->m_pViewMgr, cViewID, pViewCfg, m_pParent->m_pStateMgr, m_pParent->m_pStatusBar, m_pParent->m_pDoc, m_hWnd, &rc, EDVWSNoBorder, m_pParent->m_tLocaleID, &m_pView);
	if (m_pView == NULL)
		m_pParent->m_pViewMgr->CreateWnd(m_pParent->m_pViewMgr, CConfigValue(__uuidof(DesignerViewFactoryNULL)), NULL, m_pParent->m_pStateMgr, m_pParent->m_pStatusBar, m_pParent->m_pDoc, m_hWnd, &rc, EDVWSNoBorder, m_pParent->m_tLocaleID, &m_pView);
	if (!m_bAdjustable)
	{
		SIZE szOpt = {rc.right, rc.top+200*m_fScale};
		m_pView->OptimumSize(&szOpt);
		if (szOpt.cy != rc.bottom)
		{
			RECT rcThis;
			GetWindowRect(&rcThis);
			rcThis.bottom += szOpt.cy-rc.bottom;
			m_nSizeY = rc.bottom = szOpt.cy;
			if (m_bOpen)
				SetWindowPos(NULL, 0, 0, rcThis.right-rcThis.left, rcThis.bottom-rcThis.top, SWP_NOMOVE | SWP_NOZORDER);
		}
		ShowWindow(m_nSizeY ? SW_SHOW : SW_HIDE);
	}
	else
	{
		RECT rcThis;
		GetWindowRect(&rcThis);
		rcThis.bottom = rcThis.top + rc.bottom + m_pParent->m_nHeaderSizeY+m_pParent->m_nBorderSize;
		if (m_bAdjustable)
			rcThis.bottom += m_pParent->m_nGripperSize;
		SetWindowPos(NULL, 0, 0, rcThis.right-rcThis.left, rcThis.bottom-rcThis.top, SWP_NOMOVE | SWP_NOZORDER);
		ShowWindow(SW_SHOW);
	}
}

bool CPanelCtrl::UpdateAutoSize()
{
	if (!m_bOpen)
	{
		if (m_pView == NULL)
			return false;
		SIZE szOpt = {-1, 200*m_fScale};
		m_pView->OptimumSize(&szOpt);
		bool bChange = m_nSizeY != szOpt.cy;
		m_nSizeY = szOpt.cy;
		if (bChange)
			ShowWindow(m_nSizeY ? SW_SHOW : SW_HIDE);
		return bChange;
	}

	if (m_bAdjustable)
		return false;
	SIZE szOpt = {-1, 200*m_fScale};
	m_pView->OptimumSize(&szOpt);
	RECT rc;
	GetClientRect(&rc);
	if (rc.bottom == szOpt.cy)
		return false;
	RECT rcThis;
	GetWindowRect(&rcThis);
	rcThis.bottom += szOpt.cy-rc.bottom;
	m_nSizeY = rc.bottom = szOpt.cy;
	SetWindowPos(NULL, 0, 0, rcThis.right-rcThis.left, rcThis.bottom-rcThis.top, SWP_NOMOVE | SWP_NOZORDER);
	ShowWindow(m_nSizeY ? SW_SHOW : SW_HIDE);
	return true;
}

LRESULT CPanelCtrl::OnNCPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	CWindowDC dc(m_hWnd);
	if (dc.IsNull())
		return 0;

	RECT rect;
	GetWindowRect(&rect);
	RECT rc = rect;
	rc.top += m_pParent->m_nHeaderSizeY;
	rc.left += m_pParent->m_nBorderSize;
	rc.right -= m_pParent->m_nBorderSize;
	rc.bottom -= m_pParent->m_nBorderSize;
	if (m_bAdjustable)
		rc.bottom -= m_pParent->m_nGripperSize;
	if (rc.left >= rc.right || rc.top >= rc.bottom)
	{
		rc.left = rc.right = rc.top = rc.bottom = 0;
	}
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rect);
	if (rgn.IsNull())
		return 0;

	if (a_wParam != 1)
		rgn.CombineRgn(reinterpret_cast<HRGN>(a_wParam), rgn, RGN_AND);

	if (rc.left < rc.right || rc.top < rc.bottom)
	{
		::OffsetRect(&rc, -rect.left, -rect.top);
		dc.ExcludeClipRect(&rc);
	}
	::OffsetRect(&rect, -rect.left, -rect.top);

	TCHAR szTmp[256] = _T("");
	GetWindowText(szTmp, itemsof(szTmp));
	RECT rcText = {rect.left+13, rect.top+2, rect.right-25, rect.top+m_pParent->m_nHeaderSizeY};

	if (m_hTheme && !m_pParent->m_bDisableStyle)
	{
		if (IsThemeBackgroundPartiallyTransparent(EBP_NORMALGROUPHEAD, 0))
			DrawThemeParentBackground(dc, &rect);
		DrawThemeBackground(dc, EBP_NORMALGROUPHEAD, 0, &rect, NULL);
		if (m_bAdjustable && m_bOpen)
		{
			RECT rcGrip;
			GetWindowRect(&rcGrip);
			rcGrip.right -= rcGrip.left+1;
			rcGrip.left = 1;
			rcGrip.bottom -= rcGrip.top+m_pParent->m_nBorderSize;
			rcGrip.top = rcGrip.bottom-m_pParent->m_nGripperSize;
			dc.FillRect(&rcGrip, COLOR_3DFACE);
			if (m_pParent->m_hThemeGripper)
			{
				if ((rcGrip.right-rcGrip.left) > 50)
				{
					rcGrip.left = ((rcGrip.left+rcGrip.right)>>1)-25;
					rcGrip.right = rcGrip.left+50;
				}
				RECT rcGrip2 = {rcGrip.left, rcGrip.top-1, rcGrip.right, rcGrip.bottom};
				::DrawThemeBackground(m_pParent->m_hThemeGripper, dc, RP_GRIPPERVERT, 0, &rcGrip2, &rcGrip);
			}
		}
		if (m_hIcon)
		{
			dc.DrawIconEx(rcText.left-5, ((rcText.bottom+rcText.top)>>1)-(XPGUI::GetSmallIconSize()>>1), m_hIcon, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize());
			rcText.left += XPGUI::GetSmallIconSize();
		}
		DrawThemeText(dc, EBP_NORMALGROUPHEAD, m_bHot ? (GetCapture() == m_hWnd ? EBNGC_PRESSED : EBNGC_HOT) : EBNGC_NORMAL, CT2CW(szTmp), -1, DT_END_ELLIPSIS|DT_SINGLELINE|DT_LEFT|DT_VCENTER, NULL, &rcText);
		//COLORREF clr = dc.SetTextColor(m_bHot ? m_pParent->m_clrHeaderHot : m_pParent->m_clrHeader);
		//int bkm = dc.SetBkMode(TRANSPARENT);
		//HFONT hf = dc.SelectFont(m_pParent->m_cHeaderFont);
		//dc.DrawTextEx(szTmp, -1, &rcText, DT_END_ELLIPSIS|DT_SINGLELINE|DT_LEFT|DT_VCENTER, NULL);
		//dc.SelectFont(hf);
		//dc.SetBkMode(bkm);
		//GetThemeColor(EBP_IEBARMENU, EBM_HOT, TMT_TEXTCOLOR, &clr);
		//dc.SetTextColor(clr);
		RECT rcBtn = {rect.right-m_pParent->m_nHeaderSizeY, rect.top, rect.right, rect.top+m_pParent->m_nHeaderSizeY};
		if (m_bOpen)
			DrawThemeBackground(dc, EBP_NORMALGROUPCOLLAPSE, m_bHot ? (GetCapture() == m_hWnd ? EBNGC_PRESSED : EBNGC_HOT) : EBNGC_NORMAL, &rcBtn, NULL);
		else
			DrawThemeBackground(dc, EBP_NORMALGROUPEXPAND, m_bHot ? (GetCapture() == m_hWnd ? EBNGE_PRESSED : EBNGE_HOT) : EBNGE_NORMAL, &rcBtn, NULL);
	}
	else
	{
		dc.FillRect(&rect, (HBRUSH)(COLOR_3DFACE+1));
		if (m_bAdjustable && m_bOpen)
		{
			RECT rcGrip;
			GetWindowRect(&rcGrip);
			rcGrip.right -= rcGrip.left+1;
			rcGrip.left = 1;
			rcGrip.bottom -= rcGrip.top+m_pParent->m_nBorderSize;
			rcGrip.top = rcGrip.bottom-m_pParent->m_nGripperSize;
			if ((rcGrip.right-rcGrip.left) > 80)
			{
				rcGrip.left = ((rcGrip.left+rcGrip.right)>>1)-40;
				rcGrip.right = rcGrip.left+80;
			}
			++rcGrip.left;
			--rcGrip.right;
			++rcGrip.top;
			//--rcGrip.bottom;
			dc.Draw3dRect(&rcGrip, GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_3DSHADOW));
		}
		if (m_hIcon)
		{
			dc.DrawIconEx(rcText.left-5, ((rcText.bottom+rcText.top)>>1)-8, m_hIcon, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize());
			rcText.left += XPGUI::GetSmallIconSize();
		}
		COLORREF clr = dc.SetTextColor(m_bHot ? m_pParent->m_clrHeaderHot : m_pParent->m_clrHeader);
		int bkm = dc.SetBkMode(TRANSPARENT);
		HFONT hf = dc.SelectFont(m_pParent->m_cHeaderFont);
		dc.DrawTextEx(szTmp, -1, &rcText, DT_END_ELLIPSIS|DT_SINGLELINE|DT_LEFT|DT_VCENTER, NULL);
		dc.SelectFont(hf);
		dc.SetBkMode(bkm);
		dc.SetTextColor(clr);
		POINT p1 = {rect.right-18, rect.top+7};
		static POINT const s_aPts[] =
		{
			{3, 0},
			{2, 1}, {3, 1}, {4, 1},
			{1, 2}, {2, 2}, {4, 2}, {5, 2},
			{0, 3}, {1, 3}, {5, 3}, {6, 3},

		};
		for (POINT const* pPt = s_aPts; pPt != s_aPts+itemsof(s_aPts); ++pPt)
			if (m_bOpen)
			{
				dc.SetPixel(p1.x+pPt->x, p1.y+pPt->y, m_pParent->m_clrHeader);
				dc.SetPixel(p1.x+pPt->x, p1.y+pPt->y+4, m_pParent->m_clrHeader);
			}
			else
			{
				dc.SetPixel(p1.x+pPt->x, p1.y-pPt->y+4, m_pParent->m_clrHeader);
				dc.SetPixel(p1.x+pPt->x, p1.y-pPt->y+8, m_pParent->m_clrHeader);
			}

		if (m_bHot)
		{
			dc.FillSolidRect(p1.x-5, p1.y-3, 17, 1, GetSysColor(COLOR_3DHILIGHT));
			dc.FillSolidRect(p1.x-5, p1.y-2, 1, 15, GetSysColor(COLOR_3DHILIGHT));
			dc.FillSolidRect(p1.x-4, p1.y+12, 15, 1, GetSysColor(COLOR_3DSHADOW));
			dc.FillSolidRect(p1.x+11, p1.y-2, 1, 15, GetSysColor(COLOR_3DSHADOW));
		}
	}

	::DefWindowProc(m_hWnd, WM_NCPAINT, (WPARAM)rgn.m_hRgn, 0L);

	return 0;
}

LRESULT CPanelCtrl::OnNCCalcSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	NCCALCSIZE_PARAMS* const pParams = reinterpret_cast<NCCALCSIZE_PARAMS*>(a_lParam);
	pParams->rgrc[0].top += m_pParent->m_nHeaderSizeY;
	pParams->rgrc[0].bottom -= m_pParent->m_nBorderSize;
	pParams->rgrc[0].left += m_pParent->m_nBorderSize;
	pParams->rgrc[0].right -= m_pParent->m_nBorderSize;
	if (m_bAdjustable)
	{
		pParams->rgrc[0].bottom -= m_pParent->m_nGripperSize;
	}
	return 0;
}

LRESULT CPanelCtrl::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_pView)
	{
		RECT rc = {0, int(HIWORD(a_lParam))-m_nSizeY, LOWORD(a_lParam), HIWORD(a_lParam)};
		m_pView->Move(&rc);
	}
	return 0;
}

LRESULT CPanelCtrl::OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (HeaderHitTest(a_lParam))
	{
		m_bResizing = false;
		TRACKMOUSEEVENT tME;
		tME.cbSize = sizeof tME;
		tME.hwndTrack = m_hWnd;
		tME.dwFlags = TME_LEAVE|TME_CANCEL;
		TrackMouseEvent(&tME);
		SetCapture();
		RedrawHeader();
	}
	else if (m_bAdjustable)
	{
		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		ClientToScreen(&tPt);
		RECT rc;
		GetWindowRect(&rc);
		rc.bottom -= m_pParent->m_nBorderSize;
		rc.top = rc.bottom-m_pParent->m_nGripperSize;
		if (::PtInRect(&rc, tPt))
		{
			m_bResizing = true;
			m_nResizeOffset = m_nSizeY-GET_Y_LPARAM(a_lParam)+m_pParent->m_nGripperSize;
			SetCapture();
		}
	}
	return 0;
}

LRESULT CPanelCtrl::OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (GetCapture() == m_hWnd)
	{
		ReleaseCapture();
		if (m_bHot)
		{
			m_bOpen = !m_bOpen;
			TRACKMOUSEEVENT tME;
			tME.cbSize = sizeof tME;
			tME.hwndTrack = m_hWnd;
			tME.dwFlags = TME_LEAVE;
			TrackMouseEvent(&tME);
			RECT rcThis;
			GetWindowRect(&rcThis);
			if (m_bOpen)
			{
				if (m_pView == NULL)
				{
					CreateView();
				}
				else
				{
					UpdateAutoSize();
					SetWindowPos(NULL, 0, 0, rcThis.right-rcThis.left, m_pParent->m_nHeaderSizeY+m_pParent->m_nBorderSize+m_nSizeY+(m_bAdjustable ? m_pParent->m_nGripperSize : 0), SWP_NOMOVE | SWP_NOZORDER);
				}
			}
			else
			{
				SetWindowPos(NULL, 0, 0, rcThis.right-rcThis.left, m_pParent->m_nHeaderSizeY+m_pParent->m_nBorderSize, SWP_NOMOVE | SWP_NOZORDER);
			}
			m_pParent->PanelChanged(this, m_bOpen);
			RedrawHeader();

			CComBSTR cCFGID_RD_COLLAPSED(CFGID_RD_COLLAPSED);
			CConfigValue cVal(!m_bOpen);
			m_pItemCfg->ItemValuesSet(1, &(cCFGID_RD_COLLAPSED.m_str), cVal);
		}
		else if (m_bResizing)
		{
			CComBSTR cCFGID_RD_HEIGHT(CFGID_RD_HEIGHT);
			CConfigValue cVal(m_nSizeY/m_fScale);
			m_pItemCfg->ItemValuesSet(1, &(cCFGID_RD_HEIGHT.m_str), cVal);
			m_bResizing = false;
			m_pParent->PanelChanged(this, true);
		}
	}
	return 0;
}

class CPanelConfigDlg :
	public Win32LangEx::CLangDialogImpl<CPanelConfigDlg>,
	public CDialogResize<CPanelConfigDlg>,
	public CContextHelpDlg<CPanelConfigDlg>
{
public:
	CPanelConfigDlg(IConfig* a_pConfig, LCID a_tLocaleID) :
		Win32LangEx::CLangDialogImpl<CPanelConfigDlg>(a_tLocaleID), m_pMainConfig(a_pConfig),
		CContextHelpDlg<CPanelConfigDlg>(_T("http://www.rw-designer.com/panels-view"))
	{
	}

	enum { IDD = IDD_CONFIGFRAME };

	BEGIN_MSG_MAP(CPanelConfigDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CPanelConfigDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnEndDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnEndDialog)
		CHAIN_MSG_MAP(CDialogResize<CPanelConfigDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CPanelConfigDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CONFIGCONTROL, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CPanelConfigDlg)
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

LRESULT CPanelCtrl::OnContextMenu(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (GetCapture() != m_hWnd && HeaderHitTest(a_lParam, false))
	{
		SetCursor(m_hArrow);

		Reset(m_cImageList);

		CMenu cMenu;
		cMenu.CreatePopupMenu();
		CComBSTR bstrConfigure;
		CMultiLanguageString::GetLocalized(L"[0409]Configure panel[0405]Konfigurovat panel", m_tLocaleID, &bstrConfigure);
		AddItem(cMenu, 1, COLE2CT(bstrConfigure), -1, 0);

		POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		TPMPARAMS tPMParams;
		ZeroMemory(&tPMParams, sizeof tPMParams);
		tPMParams.cbSize = sizeof tPMParams;
		UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD|TPM_VERPOSANIMATION, pt.x, pt.y, m_hWnd);
		if (nSelection != 0)
		{
			CComPtr<IConfig> pDup;
			m_pItemCfg->DuplicateCreate(&pDup);
			CPanelConfigDlg cDlg(pDup, m_tLocaleID);
			if (cDlg.DoModal(m_hWnd) == IDOK)
			{
				if (m_pView)
				{
					m_pView->Destroy();
					m_pView = NULL;
					m_nSizeY = 0;
				}
				CopyConfigValues(m_pItemCfg, pDup);
				CConfigValue cColl;
				m_pItemCfg->ItemValueGet(CComBSTR(CFGID_RD_COLLAPSED), &cColl);
				m_bOpen = !cColl.operator bool();
				if (m_bOpen)
				{
					CreateView();
				}
				else
				{
					RECT rcThis;
					GetWindowRect(&rcThis);
					SetWindowPos(NULL, 0, 0, rcThis.right-rcThis.left, m_pParent->m_nHeaderSizeY+m_pParent->m_nBorderSize, SWP_NOMOVE | SWP_NOZORDER);
				}
				m_pParent->PanelChanged(this, m_bOpen);
			}
		}
	}
	return 0;
}

LRESULT CPanelCtrl::OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_bResizing)
	{
		if (GetCapture() == m_hWnd)
		{
			m_nSizeY = GET_Y_LPARAM(a_lParam)-m_nResizeOffset;
			if (m_nSizeY < 0)
				m_nSizeY = 0;
			RECT rcThis;
			GetWindowRect(&rcThis);
			SetWindowPos(NULL, 0, 0, rcThis.right-rcThis.left, m_pParent->m_nHeaderSizeY+m_pParent->m_nBorderSize+m_nSizeY+m_pParent->m_nGripperSize, SWP_NOMOVE | SWP_NOZORDER);
			m_pParent->RepositionPanels();
		}
	}
	else
	{
		bool bHot = HeaderHitTest(a_lParam);
		if (m_bHot != bHot)
		{
			m_bHot = bHot;
			if (m_bHot && GetCapture() != m_hWnd)
			{
				TRACKMOUSEEVENT tME;
				tME.cbSize = sizeof tME;
				tME.hwndTrack = m_hWnd;
				tME.dwFlags = TME_LEAVE;
				TrackMouseEvent(&tME);
			}
			RedrawHeader();
		}
	}
	return 0;
}

LRESULT CPanelCtrl::OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_bHot)
	{
		m_bHot = false;
		RedrawHeader();
	}
	return 0;
}

LRESULT CPanelCtrl::OnNCHitTest(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	return HTCLIENT;
}

LRESULT CPanelCtrl::OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	DWORD dwPos = GetMessagePos();
	POINT tPt = {GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)};
	RECT rc;
	GetWindowRect(&rc);
	LONG nPrevBottom = rc.bottom;
	rc.bottom = rc.top+m_pParent->m_nHeaderSizeY;
	if (::PtInRect(&rc, tPt))
	{
		SetCursor(m_hHand);
	}
	else if (m_bAdjustable)
	{
		rc.bottom = nPrevBottom-m_pParent->m_nBorderSize;
		rc.top = nPrevBottom-m_pParent->m_nBorderSize-m_pParent->m_nGripperSize;
		if (::PtInRect(&rc, tPt))
		{
			SetCursor(m_hSizeNS);
		}
		else
		{
			SetCursor(m_hArrow);
		}
	}
	return 0;
}

bool CPanelCtrl::HeaderHitTest(LPARAM a_lParam, bool a_bClient)
{
	POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
	if (a_bClient) ClientToScreen(&tPt);
	RECT rc;
	GetWindowRect(&rc);
	rc.bottom = rc.top+m_pParent->m_nHeaderSizeY;
	return ::PtInRect(&rc, tPt);
}

void CPanelCtrl::RedrawHeader()
{
	RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_FRAME);
}

LRESULT CPanelCtrl::OnRWGotFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	if (m_pParent) m_pParent->OnRWGotFocus(this, a_wParam && a_lParam);
	return 0;
}

LRESULT CPanelCtrl::OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	if (m_pParent) return m_pParent->OnRWForward(a_uMsg, a_wParam, a_lParam);
	return 0;
}

LRESULT CPanelCtrl::OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	return m_pParent ? reinterpret_cast<LRESULT>(m_pParent->GetPanelBGBrush()) : NULL;
}

LRESULT CPanelCtrl::OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	if (m_pParent == NULL)
		return 0;
	SetBkColor((HDC)a_wParam, ((GetSysColor(COLOR_3DLIGHT)&0xfefefe) + (GetSysColor(COLOR_3DFACE)&0xfefefe))>>1);
	return reinterpret_cast<LRESULT>(m_pParent->GetPanelBGBrush());
}
