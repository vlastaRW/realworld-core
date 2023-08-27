// FullConfigWnd.cpp : Implementation of CFullConfigWnd

#include "stdafx.h"
#include "FullConfigWnd.h"


// CFullConfigWnd

STDMETHODIMP CFullConfigWnd::TopWindowSet(BOOL a_bIsTopWindow, DWORD a_clrBackground)
{
	return S_FALSE;
}

STDMETHODIMP CFullConfigWnd::Create(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, EConfigWindowBorderMode a_eBorderMode)
{
	m_tLocaleID = a_tLocaleID;
	IDD = a_eBorderMode == ECWBMMarginAndOutline ? IDD_FULLCONFIG_BORDER : IDD_FULLCONFIG_NOBORDER;
	static INITCOMMONCONTROLSEX tICCEx = {sizeof tICCEx, ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES};
	static BOOL bDummy = InitCommonControlsEx(&tICCEx);
	Win32LangEx::CLangDialogImpl<CFullConfigWnd>::Create(a_hParent);
	SetWindowLong(GWL_ID, a_nCtlID);
	MoveWindow(a_prcPositon);
	ShowWindow(a_bVisible ? SW_SHOW : SW_HIDE);
	return S_OK;
}

STDMETHODIMP CFullConfigWnd::ConfigSet(IConfig* a_pConfig, EConfigPanelMode a_eMode)
{
	m_eMode = a_eMode;
	if (a_pConfig == m_pConfig.p)
		return S_OK;

	if (m_pConfig.p)
	{
		m_pConfig->ObserverDel(ObserverGet(), 0);
	}
	m_pConfig = a_pConfig;
	if (m_pConfig.p)
	{
		m_pConfig->ObserverIns(ObserverGet(), 0);
	}

	m_cClient.ConfigSet(m_pConfig);

	return S_OK;
}

void CFullConfigWnd::OwnerNotify(TCookie a_tCookie, IUnknown* a_pChangedParam)
{
	m_cClient.ItemsChanged();
}

LRESULT CFullConfigWnd::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_cClient.Initialize(*this, m_tLocaleID);
	BOOL b;
	CThemeImpl<CFullConfigWnd>::OnCreate(0, 0, 0, b);
	AddRef();

	return TRUE;
}

LRESULT CFullConfigWnd::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TScrollInfo tScroll = ValueGet();
	switch (LOWORD(wParam))
	{
	case SB_BOTTOM:		tScroll.nActPosV = tScroll.nRangeV-tScroll.nPageV; break;
	case SB_TOP:		tScroll.nActPosV = 0; break;
	case SB_LINEUP:		tScroll.nActPosV--; break;
	case SB_LINEDOWN:	tScroll.nActPosV++; break;
	case SB_PAGEUP:		tScroll.nActPosV -= tScroll.nPageV; break;
	case SB_PAGEDOWN:	tScroll.nActPosV += tScroll.nPageV; break;
	case SB_THUMBTRACK:	tScroll.nActPosV = HIWORD(wParam); break;
	}
	if (tScroll.nActPosV < 0)
		tScroll.nActPosV = 0;
	if (tScroll.nActPosV > (tScroll.nRangeV-tScroll.nPageV))
		tScroll.nActPosV = tScroll.nRangeV-tScroll.nPageV;
	ValueSet(tScroll);
	return TRUE;
}

LRESULT CFullConfigWnd::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TScrollInfo tScroll = ValueGet();
	switch (LOWORD(wParam))
	{
	case SB_BOTTOM:		tScroll.nActPosH = tScroll.nRangeH-tScroll.nPageH; break;
	case SB_TOP:		tScroll.nActPosH = 0; break;
	case SB_LINEUP:		tScroll.nActPosH--; break;
	case SB_LINEDOWN:	tScroll.nActPosH++; break;
	case SB_PAGEUP:		tScroll.nActPosH -= tScroll.nPageH; break;
	case SB_PAGEDOWN:	tScroll.nActPosH += tScroll.nPageH; break;
	case SB_THUMBTRACK:	tScroll.nActPosH = HIWORD(wParam); break;
	}
	if (tScroll.nActPosH < 0)
		tScroll.nActPosH = 0;
	if (tScroll.nActPosH > (tScroll.nRangeH-tScroll.nPageH))
		tScroll.nActPosH = tScroll.nRangeH-tScroll.nPageH;
	ValueSet(tScroll);
	return TRUE;
}

void CFullConfigWnd::Notify(const TScrollInfo& a_tScrollInfo)
{
	SCROLLINFO tScrollInfo;
	tScrollInfo.cbSize = sizeof(tScrollInfo);
	tScrollInfo.fMask = SIF_ALL;// | SIF_DISABLENOSCROLL;
	tScrollInfo.nMin = 0;
	tScrollInfo.nMax = a_tScrollInfo.nRangeV;
	tScrollInfo.nPage = a_tScrollInfo.nPageV;
	tScrollInfo.nPos = a_tScrollInfo.nActPosV;
	SetScrollInfo(SB_VERT, &tScrollInfo);
	tScrollInfo.nMin = 0;
	tScrollInfo.nMax = a_tScrollInfo.nRangeH;
	tScrollInfo.nPage = a_tScrollInfo.nPageH;
	tScrollInfo.nPos = a_tScrollInfo.nActPosH;
	SetScrollInfo(SB_HORZ, &tScrollInfo);
}

LRESULT CFullConfigWnd::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	SIZE tTmp = {LOWORD(lParam), HIWORD(lParam)};
	m_cClient.SetAreaSize(tTmp);

	return 0;
}

LRESULT CFullConfigWnd::OnHeaderTrack(int wParam, LPNMHDR pNMHdr, BOOL& bHandled)
{
	m_cClient.HeaderTrack();
	return FALSE;
}

LRESULT CFullConfigWnd::OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	return 0;
}

LRESULT CFullConfigWnd::OnHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	return 0;
}

LRESULT CFullConfigWnd::OnCtlColorDlg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
}

