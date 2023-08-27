
#pragma once

#include <MultiLanguageString.h>


class CMenuContainer :
	public CWindowImpl<CMenuContainer>,
	public CThemeImpl<CMenuContainer>
{
public:
	CMenuContainer() : m_nSpace(1), m_nEdge(3), m_nCloseID(0), m_nDPI(96),
		m_bInRect(false), m_pwndCmdBar(NULL), m_bInSwitcher(false)
	{
		ZeroMemory(&m_rcClose, sizeof m_rcClose);
		ZeroMemory(&m_rcSwitcher, sizeof m_rcSwitcher);
		SetThemeClassList(L"WINDOW");
	}
	void Init(CCommandBarCtrl& a_wndCmdBar)
	{
		m_pwndCmdBar = &a_wndCmdBar;
		m_pwndCmdBar->SetParent(m_hWnd);
		UpdateLayout();
	}
	void UpdateLayout()
	{
		if (m_hWnd)
		{
			SendMessage(WM_SIZE, 0, 0);
			Invalidate();
		}
	}
	void SetCloseButtonID(UINT a_nID)
	{
		bool const b = (a_nID == 0 && m_nCloseID != 0) || (a_nID != 0 && m_nCloseID == 0);
		m_nCloseID = a_nID;
		if (b)
			UpdateLayout();
	}
	void SetOptions(std::vector<std::pair<CComBSTR, CComBSTR> > const& a_cOptions, BSTR a_bstrActive, LCID a_tLocaleID)
	{
		std::vector<std::pair<CComBSTR, CComBSTR> >::const_iterator iSel = a_cOptions.begin();
		for (std::vector<std::pair<CComBSTR, CComBSTR> >::const_iterator i = a_cOptions.begin(); i != a_cOptions.end(); ++i)
		{
			if (i->first == a_bstrActive)
			{
				if (i+1 != a_cOptions.end())
					iSel = i+1;
				break;
			}
		}
		if (iSel == a_cOptions.end())
		{
			m_bstrSwitch.Empty();
			m_bstrSwitchLoc.Empty();
		}
		else
		{
			m_bstrSwitch = iSel->first;
			m_bstrSwitchLoc.Empty();
			CMultiLanguageString::GetLocalized(iSel->second, a_tLocaleID, &m_bstrSwitchLoc);
		}
		UpdateLayout();
	}

	enum { MCN_LAYOUT_SWITCHED = 1 };

	DECLARE_WND_CLASS_EX(_T("MenuContainer"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_3DFACE);

	BEGIN_MSG_MAP(CMenuContainer)
		CHAIN_MSG_MAP(CThemeImpl<CMenuContainer>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
	END_MSG_MAP()

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		int nX = LOWORD(a_lParam);
		int nY = HIWORD(a_lParam);
		bool bInRect = nX >= m_rcClose.left && nX < m_rcClose.right && nY >= m_rcClose.top && nY < m_rcClose.bottom;
		bool bChange = false;
		if (bInRect != m_bInRect && !m_bInSwitcher)
		{
			m_bInRect = bInRect;
			bChange = true;
			InvalidateRect(&m_rcClose);
		}
		bool bInSwitch = nX >= m_rcSwitcher.left && nX < m_rcSwitcher.right && nY >= m_rcSwitcher.top && nY < m_rcSwitcher.bottom;
		if (bInSwitch != m_bInSwitcher && !m_bInRect)
		{
			m_bInSwitcher = bInSwitch;
			bChange = true;
			InvalidateRect(&m_rcSwitcher);
		}
		if (bChange && (bInSwitch || bInRect))
		{
			TRACKMOUSEEVENT tME;
			tME.cbSize = sizeof tME;
			tME.hwndTrack = m_hWnd;
			tME.dwFlags = TME_LEAVE;
			TrackMouseEvent(&tME);
		}
		return 0;
	}
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		int nX = LOWORD(a_lParam);
		int nY = HIWORD(a_lParam);
		bool bInRect = nX >= m_rcClose.left && nX < m_rcClose.right && nY >= m_rcClose.top && nY < m_rcClose.bottom;
		if (bInRect)
		{
			m_bInRect = true;
			SetCapture();
			InvalidateRect(&m_rcClose);
		}
		bool bInSwitch = nX >= m_rcSwitcher.left && nX < m_rcSwitcher.right && nY >= m_rcSwitcher.top && nY < m_rcSwitcher.bottom;
		if (bInSwitch)
		{
			m_bInSwitcher = true;
			SetCapture();
			InvalidateRect(&m_rcSwitcher);
		}
		return 0;
	}
	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
			if (m_bInRect)
			{
				InvalidateRect(&m_rcClose);
				if (m_nCloseID)
				{
					GetParent().SendMessage(WM_COMMAND, MAKELPARAM(m_nCloseID, BN_CLICKED));
				}
			}
			if (m_bInSwitcher)
			{
				InvalidateRect(&m_rcSwitcher);

				HWND hPar = GetParent();
				if (hPar)
				{
					struct MSMNHDR {NMHDR hdr; LPCOLESTR psz;} nm = {{m_hWnd, GetWindowLong(GWL_ID), MCN_LAYOUT_SWITCHED}, m_bstrSwitch};
					SendMessage(hPar, WM_NOTIFY, nm.hdr.idFrom, (LPARAM)&nm);
				}
			}
		}
		return 0;
	}
	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		if (GetCapture() != m_hWnd)
		{
			if (m_bInRect)
			{
				m_bInRect = false;
				InvalidateRect(&m_rcClose);
			}
			if (m_bInSwitcher)
			{
				m_bInSwitcher = false;
				InvalidateRect(&m_rcSwitcher);
			}
		}
		return 0;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		if (m_pwndCmdBar == NULL || !m_pwndCmdBar->IsWindow())
			return 0;
		int nX = m_hTheme ? GetThemeSysSize(SM_CXSMSIZE) : GetSystemMetrics(SM_CXSMSIZE);
		int nY = m_hTheme ? GetThemeSysSize(SM_CYSMSIZE) : GetSystemMetrics(SM_CYSMSIZE);
		RECT rc = {0, 0, 0, 0};
		m_pwndCmdBar->GetItemRect(0, &rc);
		RECT rcThis;
		GetParent().GetClientRect(&rcThis);
		rcThis.bottom = (m_nSpace<<1) + m_nEdge + max(rc.top+rc.bottom, nY);
		MoveWindow(&rcThis);
		if (m_nCloseID)
		{
			m_rcClose.right = rcThis.right - m_nSpace;
			m_rcClose.left = m_rcClose.right - nX;
		}
		else
		{
			m_rcClose.left = m_rcClose.right = rcThis.right;
		}
		rcThis.bottom -= m_nEdge;
		m_rcClose.top = (rcThis.bottom-nY)>>1;
		m_rcClose.bottom = m_rcClose.top + nY;
		m_rcSwitcher.top = rcThis.top+1;
		m_rcSwitcher.bottom = rcThis.bottom-1;
		if (m_bstrSwitch != NULL && m_bstrSwitch[0])
		{
			CDCHandle cDC(GetDC());
			HFONT hOldFont = cDC.SelectFont(m_fontMenu);
			RECT rc = {0, 0, 0, 0};
			DrawText(cDC, m_bstrSwitchLoc, -1, &rc, DT_CALCRECT);
			cDC.SelectFont(hOldFont);
			ReleaseDC(cDC);

			m_rcSwitcher.right = m_rcClose.left-(m_nSpace<<1);
			m_rcSwitcher.left = m_rcSwitcher.right - rc.right - (m_nDPI*14+48)/96;
		}
		else
		{
			m_rcSwitcher.left = m_rcSwitcher.right = m_rcClose.left;
		}
		rcThis.right = m_rcSwitcher.left - (m_nSpace<<1);
		rcThis.left = m_nSpace;
		rcThis.top += ((rcThis.bottom-rc.bottom-rc.top)>>1);
		rcThis.bottom = rcThis.top + rc.bottom+rc.top;
		m_pwndCmdBar->MoveWindow(&rcThis);
		return 0;
	}
	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		CDCHandle cDC = reinterpret_cast<HDC>(a_wParam);
		RECT rc;
		GetClientRect(&rc);
		rc.bottom -= 2;
		BOOL bFlatMenus = FALSE;
		SystemParametersInfo(SPI_GETFLATMENU, 0, &bFlatMenus, 0);
		cDC.FillRect(&rc, bFlatMenus ? COLOR_MENUBAR : COLOR_MENU);
		rc.top = rc.bottom; rc.bottom += 2;
		cDC.DrawEdge(&rc, EDGE_ETCHED, BF_TOP);
		return 1;
	}
	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		CDCHandle cDC = reinterpret_cast<HDC>(a_wParam);
		PAINTSTRUCT ps;
		if (a_wParam == NULL)
		{
			cDC = BeginPaint(&ps);
		}

		if (m_rcClose.left < m_rcClose.right)
		{
			if (m_hTheme)
			{
				DrawThemeBackground(cDC, WP_MDICLOSEBUTTON,
					m_nCloseID ? (GetCapture() == m_hWnd ? (m_bInRect ? CBS_PUSHED : CBS_NORMAL) : (m_bInRect ? CBS_HOT : CBS_NORMAL)) : CBS_DISABLED, &m_rcClose, NULL);
			}
			else
			{
				DrawFrameControl(cDC, &m_rcClose, DFC_CAPTION, DFCS_CAPTIONCLOSE);
			}
		}

		if (m_rcSwitcher.left < m_rcSwitcher.right)
		{
			HFONT hOldFont = cDC.SelectFont(m_fontMenu);
			COLORREF clrBg = GetSysColor(COLOR_3DFACE/*MENU*/);
			COLORREF clr = GetSysColor(COLOR_WINDOWTEXT/*MENUTEXT*/);
			if (!m_bInSwitcher)
			{
				clr = RGB(
					(int(GetRValue(clrBg))+int(GetRValue(clr)))>>1,
					(int(GetGValue(clrBg))+int(GetGValue(clr)))>>1,
					(int(GetBValue(clrBg))+int(GetBValue(clr)))>>1
					);
			}
			cDC.SetTextColor(clr);
			cDC.SetBkColor(clrBg);
			RECT rc = m_rcSwitcher;
			rc.left += (m_nDPI*10+48)/96;
			DrawText(cDC, m_bstrSwitchLoc, -1, &rc, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
			cDC.SelectFont(hOldFont);

			int nSize = (m_nDPI*3+48)/96;
			POINT ptsArrow[3] =
			{
				{ m_rcSwitcher.left, ((m_rcSwitcher.top+m_rcSwitcher.bottom)>>1)-nSize },
				{ m_rcSwitcher.left, ((m_rcSwitcher.top+m_rcSwitcher.bottom)>>1)+nSize },
				{ m_rcSwitcher.left+nSize, ((m_rcSwitcher.top+m_rcSwitcher.bottom)>>1) }
			};

			CBrush brArrow;
			brArrow.CreateSolidBrush(clr);
			CPen penArrow;
			penArrow.CreatePen(PS_SOLID, 0, clr);

			HBRUSH hbrOld = cDC.SelectBrush(brArrow);
			HPEN hpenOld = cDC.SelectPen(penArrow);

			cDC.SetPolyFillMode(WINDING);
			cDC.Polygon(ptsArrow, 3);

			cDC.SelectBrush(hbrOld);
			cDC.SelectPen(hpenOld);
		}

		if (a_wParam == NULL)
		{
			EndPaint(&ps);
		}
		return 0;
	}
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CreateFont();
		HDC hDC = GetDC();
		m_nDPI = GetDeviceCaps(hDC, LOGPIXELSX);
		ReleaseDC(hDC);

		a_bHandled = FALSE;
		return 0;
	}
	void CreateFont()
	{
		NONCLIENTMETRICS info = { RunTimeHelper::SizeOf_NONCLIENTMETRICS() };
		BOOL bRet = ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		ATLASSERT(bRet);
		if(bRet)
		{
			LOGFONT logfont = { 0 };
			if(m_fontMenu.m_hFont != NULL)
				m_fontMenu.GetLogFont(logfont);
			if(logfont.lfHeight != info.lfMenuFont.lfHeight ||
			   logfont.lfWidth != info.lfMenuFont.lfWidth ||
			   logfont.lfEscapement != info.lfMenuFont.lfEscapement ||
			   logfont.lfOrientation != info.lfMenuFont.lfOrientation ||
			   logfont.lfWeight != info.lfMenuFont.lfWeight ||
			   //logfont.lfItalic != info.lfMenuFont.lfItalic ||
			   logfont.lfUnderline != info.lfMenuFont.lfUnderline ||
			   logfont.lfStrikeOut != info.lfMenuFont.lfStrikeOut ||
			   logfont.lfCharSet != info.lfMenuFont.lfCharSet ||
			   logfont.lfOutPrecision != info.lfMenuFont.lfOutPrecision ||
			   logfont.lfClipPrecision != info.lfMenuFont.lfClipPrecision ||
			   logfont.lfQuality != info.lfMenuFont.lfQuality ||
			   logfont.lfPitchAndFamily != info.lfMenuFont.lfPitchAndFamily ||
			   lstrcmp(logfont.lfFaceName, info.lfMenuFont.lfFaceName) != 0)
			{
				info.lfMenuFont.lfItalic = TRUE;
				HFONT hFontMenu = ::CreateFontIndirect(&info.lfMenuFont);
				ATLASSERT(hFontMenu != NULL);
				if(hFontMenu != NULL)
				{
					if(m_fontMenu.m_hFont != NULL)
						m_fontMenu.DeleteObject();
					m_fontMenu.Attach(hFontMenu);
					//SetFont(m_fontMenu);
				}
			}
		}
	}

private:
	CCommandBarCtrl* m_pwndCmdBar;
	int m_nSpace;
	int m_nEdge;
	UINT m_nCloseID;
	RECT m_rcClose;
	bool m_bInRect;
	RECT m_rcSwitcher;
	bool m_bInSwitcher;

	CFont m_fontMenu;
	CComBSTR m_bstrSwitch;
	CComBSTR m_bstrSwitchLoc;
	int m_nDPI;
};

