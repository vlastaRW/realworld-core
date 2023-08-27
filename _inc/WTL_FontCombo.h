
#pragma once

#include <map>
#include <string>


namespace WTL
{

class CFontComboBox : public CWindowImpl<CFontComboBox, CComboBox>
{
public:
	// choose how the name and sample are displayed
	enum PreviewStyle
	{
		NAME_ONLY = 0,		// font name, drawn in font
		NAME_GUI_FONT,		// font name, drawn in GUI font
		NAME_THEN_SAMPLE,	// font name in GUI font, then sample text in font
		SAMPLE_THEN_NAME,	// sample text in font, then font name in GUI font
		SAMPLE_ONLY			// sample text in font
	};

	CFontComboBox() : m_iLineHeight(16), m_iMaxNameWidth(100),
		m_style(NAME_THEN_SAMPLE), m_strSample(_T("123-Abc!")), m_bInitializedWidth(false)
	{
	}

	template<typename TIter>
	void Create(TIter a_begin, TIter a_end, HWND a_hParent, RECT const& a_rcWnd, UINT a_nIDC)
	{
		HFONT hFont = (HFONT)::SendMessage(a_hParent, WM_GETFONT, 0, 0);

		HDC hdc = ::GetDC(a_hParent);
		m_fScale = ::GetDeviceCaps(hdc, LOGPIXELSX) / 96.0;
		SIZE sz = {0, 0};
		HFONT hfOld = (HFONT)::SelectObject(hdc, hFont);
		GetTextExtentPoint(hdc, m_strSample.c_str(), m_strSample.length(), &sz);
		::SelectObject(hdc, hfOld);
		::ReleaseDC(a_hParent, hdc);
		m_iMaxNameWidth = 0;
		m_iLineHeight = sz.cy;// + ::GetSystemMetrics(SM_CYBORDER)*2;

		CWindowImpl<CFontComboBox, CComboBox>::Create(a_hParent, const_cast<RECT*>(&a_rcWnd), NULL, WS_VISIBLE|CBS_DROPDOWNLIST|CBS_SORT|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL|WS_TABSTOP|WS_CHILD, 0, a_nIDC);
		while (a_begin != a_end)
		{
			CFonts::key_type cKey(*a_begin);
			CFont& cFont = m_fonts[cKey];
			AddString(cKey.c_str());
			++a_begin;
		}
		SetFont(hFont);
	}
	template<typename TIter>
	void SubclassWindow(TIter a_begin, TIter a_end, HWND a_hOrigCombo)
	{
		HWND hParent = ::GetParent(a_hOrigCombo);
		RECT rcWnd = {0, 0, 0, 0};
		::GetWindowRect(a_hOrigCombo, &rcWnd);
		::ScreenToClient(hParent, (LPPOINT)&rcWnd.left);
		::ScreenToClient(hParent, (LPPOINT)&rcWnd.right);
		HWND hPrev = ::GetNextWindow(a_hOrigCombo, GW_HWNDPREV);
		UINT nID = ::GetWindowLong(a_hOrigCombo, GWLP_ID);
		::DestroyWindow(a_hOrigCombo);
		Create(a_begin, a_end, hParent, rcWnd, nID);
		if (hPrev) SetWindowPos(hPrev, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}

	BEGIN_MSG_MAP(CFontComboBox)    
	    MESSAGE_HANDLER(OCM__BASE + WM_DRAWITEM, OnDrawItem)
	    MESSAGE_HANDLER(OCM__BASE + WM_MEASUREITEM, OnMeasureItem)
	END_MSG_MAP()          	

	LRESULT OnDrawItem(UINT /*a_uMsg*/, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (!m_bInitializedWidth)
		{
			m_bInitializedWidth = true;
			m_iMaxNameWidth = 0;
			CClientDC dc(m_hWnd);
			HFONT hFont = ((HFONT)GetStockObject( DEFAULT_GUI_FONT ));
			HFONT hf = dc.SelectFont(hFont);
			for (CFonts::const_iterator i = m_fonts.begin(); i != m_fonts.end(); ++i)
			{
				SIZE sz = {0, 0};
				dc.GetTextExtent(i->first.c_str(), -1, &sz);
				if (m_iMaxNameWidth < sz.cx) m_iMaxNameWidth = sz.cx;
			}
			dc.SelectFont(hf);
		}

		DRAWITEMSTRUCT* pDIS = reinterpret_cast<DRAWITEMSTRUCT*>(a_lParam);
		if (pDIS->CtlType != ODT_COMBOBOX)
		{
			a_bHandled = FALSE;
			return 0;
		}

		RECT rc = pDIS->rcItem;

		CDCHandle dc(pDIS->hDC);

		if (pDIS->itemState & ODS_FOCUS)
			dc.DrawFocusRect(&rc);

		if (pDIS->itemID == -1)
			return 0;

		int nIndexDC = dc.SaveDC();

		CBrush br;

		COLORREF clrSample;
		{
			COLORREF clrBG = dc.GetBkColor();
			COLORREF clrFG = dc.GetTextColor();
			clrSample = RGB((GetRValue(clrBG)+GetRValue(clrFG)*3)>>2, (GetGValue(clrBG)+GetGValue(clrFG)*3)>>2, (GetBValue(clrBG)+GetBValue(clrFG)*3)>>2);
		}

		if (pDIS->itemState & ODS_SELECTED)
		{
			br.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
			dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			clrSample = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else
		{
			br.CreateSolidBrush(dc.GetBkColor());
		}

		dc.SetBkMode(TRANSPARENT);
		dc.FillRect(&rc, br);

		// which one are we working on?
		int nLen = GetLBTextLen(pDIS->itemID);
		CAutoVectorPtr<TCHAR> pszCurFontName(new TCHAR[nLen+1]);
		GetLBText(pDIS->itemID, pszCurFontName);

		int iOffsetX = SPACING;

		// draw the text
		SIZE sz;
		int iPosY = 0;
		HFONT hf = NULL;
		CFont* cf = NULL;
		BOOL lookupResult = FALSE;
		CFonts::iterator i = m_fonts.find(CFonts::key_type(pszCurFontName));
		if (i != m_fonts.end())
		{
			lookupResult = TRUE;
			cf = &i->second;
			if (m_style != NAME_GUI_FONT && i->second.IsNull())
			{
				i->second.CreateFont(-m_iLineHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 
					FALSE,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
					CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, pszCurFontName);
			}
		}

		switch (lookupResult ? m_style : NAME_GUI_FONT)
		{
		case NAME_GUI_FONT:
			{
				// font name in GUI font
				dc.GetTextExtent(pszCurFontName, -1, &sz);
				iPosY = (rc.bottom-rc.top - sz.cy) >> 1;
				dc.TextOut(rc.left+iOffsetX, rc.top + iPosY, pszCurFontName);
			}
			break;
		case NAME_ONLY:
			{
				// font name in current font
				hf = dc.SelectFont(*cf);
				dc.GetTextExtent(pszCurFontName, -1, &sz);
				iPosY = (rc.bottom-rc.top - sz.cy) >> 1;
				dc.TextOut(rc.left+iOffsetX, rc.top + iPosY, pszCurFontName);
				dc.SelectFont(hf);
			}
			break;
		case NAME_THEN_SAMPLE:
			{
				// font name in GUI font
				dc.GetTextExtent(pszCurFontName, -1, &sz);
				iPosY = (rc.bottom-rc.top - sz.cy) >> 1;
				dc.TextOut(rc.left+iOffsetX, rc.top + iPosY, pszCurFontName);

			 // condense, for edit
			 int iSep = m_iMaxNameWidth;
			 if ((pDIS->itemState & ODS_COMBOBOXEDIT) == ODS_COMBOBOXEDIT)
			 {
				iSep = sz.cx;
			 }

			 // sample in current font
				hf = dc.SelectFont(*cf);
				dc.GetTextExtent(m_strSample.c_str(), -1, &sz);
				iPosY = (rc.bottom-rc.top - sz.cy) >> 1;
				COLORREF clr = dc.SetTextColor(clrSample);
				dc.TextOut(rc.left + iOffsetX + iSep + iOffsetX, rc.top + iPosY, m_strSample.c_str());
				dc.SetTextColor(clr);
				dc.SelectFont(hf);
			}
			break;
		case SAMPLE_THEN_NAME:
			{
			 // sample in current font
				hf = dc.SelectFont(*cf);
				dc.GetTextExtent(m_strSample.c_str(), -1, &sz);
				iPosY = (rc.bottom-rc.top - sz.cy) >> 1;
				COLORREF clr = dc.SetTextColor(clrSample);
				dc.TextOut(rc.left+iOffsetX, rc.top + iPosY, m_strSample.c_str());
				dc.SetTextColor(clr);
				dc.SelectFont(hf);

			 // condense, for edit
			 int iSep = m_iMaxNameWidth;
			 if ((pDIS->itemState & ODS_COMBOBOXEDIT) == ODS_COMBOBOXEDIT)
			 {
				iSep = sz.cx;
			 }

				// font name in GUI font
				dc.GetTextExtent(pszCurFontName, -1, &sz);
				iPosY = (rc.bottom-rc.top - sz.cy) >> 1;
				dc.TextOut(rc.left + iOffsetX + iSep + iOffsetX, rc.top + iPosY, pszCurFontName);
			}
			break;
		case SAMPLE_ONLY:
			{			
				// sample in current font
				hf = dc.SelectFont(*cf);
				dc.GetTextExtent(m_strSample.c_str(), -1, &sz);
				iPosY = (rc.bottom-rc.top - sz.cy) >> 1;
				dc.TextOut(rc.left+iOffsetX, rc.top + iPosY, m_strSample.c_str());
				dc.SelectFont(hf);
			}
			break;
		}

		dc.RestoreDC(nIndexDC);

		dc.Detach();

		return 0;
	}

	LRESULT OnMeasureItem(UINT /*a_uMsg*/, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		MEASUREITEMSTRUCT* pMIS = reinterpret_cast<MEASUREITEMSTRUCT*>(a_lParam);

		pMIS->itemHeight = m_iLineHeight + (4*m_fScale+0.5);
		pMIS->itemWidth = 2*m_iMaxNameWidth;

		return 0;
	}

private:
	enum { SPACING = 10 };
	typedef std::map<std::basic_string<TCHAR>, CFont> CFonts;

private:
	std::basic_string<TCHAR> m_strSample;
	CFonts m_fonts;

	PreviewStyle m_style;
	int m_iLineHeight;
	int m_iMaxNameWidth;
	float m_fScale;
	bool m_bInitializedWidth;
};

}

