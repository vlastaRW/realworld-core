// ContextMenuWithIcons.h : Declaration of the CContextMenuWithIcons

#pragma once


// CContextMenuWithIcons

template<typename T>
class CContextMenuWithIcons
{
public:
	CContextMenuWithIcons() : m_bFlatMenus(false), m_hImageList(0), m_nIconX(16), m_nIconY(16)
	{
	}
	void Reset(HIMAGELIST a_hImageList)
	{
		m_hImageList = a_hImageList;
		if (m_hImageList)
			ImageList_GetIconSize(m_hImageList, &m_nIconX, &m_nIconY);
		m_cItems.clear();
	}
	void AddItem(UINT a_nID, LPCTSTR a_pszName, int a_nIconIndex)
	{
		SItem sItem;
		if (a_pszName != NULL)
			sItem.strText = a_pszName;
		sItem.nIconIndex = a_nIconIndex;
		m_cItems[a_nID] = sItem;
	}
	void AddItem(HMENU a_hMenu, UINT a_nID, LPCTSTR a_pszName, int a_nIconIndex, UINT a_nExtraFlags = 0)
	{
		AddItem(a_nID, a_pszName, a_nIconIndex);
		AppendMenu(a_hMenu, a_nExtraFlags|MFT_OWNERDRAW, a_nID, LPCTSTR(NULL)/*a_pszName*/);
	}
	void AddItem(HMENU a_hMenu, HMENU a_hSubMenu, LPCTSTR a_pszName, int a_nIconIndex)
	{
		AddItem(reinterpret_cast<UINT>(a_hSubMenu), a_pszName, a_nIconIndex);
		AppendMenu(a_hMenu, MF_POPUP|MFT_OWNERDRAW, reinterpret_cast<UINT>(a_hSubMenu), a_pszName);
	}
	void RemoveItem(UINT a_nID)
	{
		m_cItems.erase(a_nID);
	}

	LPCTSTR GetMenuItemText(UINT a_nID)
	{
		CItems::const_iterator i = m_cItems.find(a_nID);
		if (i == m_cItems.end())
			return NULL;
		return i->second.strText.c_str();
	}

BEGIN_MSG_MAP(CContextMenuWithIcons)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
	//MESSAGE_HANDLER(WM_MENUCHAR, OnMenuChar)
	if (uMsg == WM_SETTINGCHANGE && !m_cFontMenu.IsNull())
		m_cFontMenu.DeleteObject();
END_MSG_MAP()

	// handlers
public:
	LRESULT OnDrawItem(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LPDRAWITEMSTRUCT pDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(a_lParam);

		if (pDrawItemStruct->CtlType != ODT_MENU)
		{
			a_bHandled = FALSE;
			return 0;
		}

		CItems::const_iterator i = m_cItems.find(pDrawItemStruct->itemID);
		if (i == m_cItems.end())
		{
			a_bHandled = FALSE;
			return 0;
		}

		if (m_cFontMenu == NULL)
		{
			GetSystemSettings();
		}
		if (m_bFlatMenus)
		{
			DrawItemFlat(pDrawItemStruct, i->second.strText.c_str(), i->second.nIconIndex);
		}
		else
		{
			DrawItem3D(pDrawItemStruct, i->second.strText.c_str(), i->second.nIconIndex);
		}

		return TRUE;
	}

	LRESULT OnMeasureItem(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LPMEASUREITEMSTRUCT pMeasureItemStruct = reinterpret_cast<LPMEASUREITEMSTRUCT>(a_lParam);
				
		if (pMeasureItemStruct->CtlType != ODT_MENU)
		{
			a_bHandled = FALSE;
			return 0;
		}
		CItems::const_iterator i = m_cItems.find(pMeasureItemStruct->itemID);
		if (i == m_cItems.end())
		{
			a_bHandled = FALSE;
			return 0;
		}

		if (m_cFontMenu == NULL)
		{
			GetSystemSettings();
		}

		if (i->second.strText.empty())
		{
			// no text
			LOGFONT lf;
			m_cFontMenu.GetLogFont(lf);
			pMeasureItemStruct->itemHeight = max(abs(lf.lfHeight)+2*int(4*m_fScaling+0.5f), m_nIconY+2*int(s_kcyButtonMargin*m_fScaling+0.5f));
			pMeasureItemStruct->itemWidth = pMeasureItemStruct->itemHeight-::GetSystemMetrics(SM_CXMENUCHECK)+1;
		}
		else
		{
			// compute size of text - use DrawText with DT_CALCRECT
			CWindowDC dc(NULL);
			HFONT hOldFont;
			hOldFont = dc.SelectFont(m_cFontMenu);

			RECT rcText = { 0, 0, 0, 0 };
			USES_CONVERSION;
			dc.DrawText(i->second.strText.c_str(), -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);
			int cx = rcText.right - rcText.left;
			dc.SelectFont(hOldFont);

			LOGFONT lf;
			m_cFontMenu.GetLogFont(lf);
			pMeasureItemStruct->itemHeight = max(abs(lf.lfHeight)+2*int(4*m_fScaling+0.5f), m_nIconY+2*int(s_kcyButtonMargin*m_fScaling+0.5f));

			// width is width of text plus a bunch of stuff
			cx += 2 * int(s_kcxTextMargin*m_fScaling+0.5f);	// L/R margin for readability
			cx += s_kcxGap*m_fScaling+0.5f;			// space between button and menu text
			cx += 2 * (m_nIconY+2*int(s_kcxButtonMargin*m_fScaling+0.5f));	// button width (L=button; R=empty margin)
			cx += m_cxExtraSpacing;		// extra between item text and accelerator keys

			// Windows adds 1 to returned value
			cx -= ::GetSystemMetrics(SM_CXMENUCHECK) - 1;
			pMeasureItemStruct->itemWidth = cx;		// done deal
		}

		return TRUE;
	}

	//LRESULT OnMenuChar(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	//{
	//	bHandled = FALSE;
	//	T* pT = static_cast<T*>(this);

	//	LRESULT lRet;
	//	if (m_bMenuActive && LOWORD(wParam) != 0x0D)
	//		lRet = 0;
	//	else
	//		lRet = MAKELRESULT(1, 1);

	//	if (m_bMenuActive && HIWORD(wParam) == MF_POPUP)
	//	{
	//		// Convert character to lower/uppercase and possibly Unicode, using current keyboard layout
	//		TCHAR ch = (TCHAR)LOWORD(wParam);
	//		CMenuHandle menu = (HMENU)lParam;
	//		int nCount = ::GetMenuItemCount(menu);
	//		int nRetCode = MNC_EXECUTE;
	//		BOOL bRet = FALSE;
	//		TCHAR szString[pT->_nMaxMenuItemTextLength];
	//		WORD wMnem = 0;
	//		bool bFound = false;
	//		for(int i = 0; i < nCount; i++)
	//		{
	//			CMenuItemInfo mii;
	//			mii.cch = pT->_nMaxMenuItemTextLength;
	//			mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	//			mii.dwTypeData = szString;
	//			bRet = menu.GetMenuItemInfo(i, TRUE, &mii);
	//			if(!bRet || (mii.fType & MFT_SEPARATOR))
	//				continue;
	//			_MenuItemData* pmd = (_MenuItemData*)mii.dwItemData;
	//			if(pmd != NULL && pmd->IsCmdBarMenuItem())
	//			{
	//				LPTSTR p = pmd->lpstrText;

	//				if(p != NULL)
	//				{
	//					while(*p && *p != _T('&'))
	//						p = ::CharNext(p);
	//					if(p != NULL && *p)
	//					{
	//						DWORD dwP = MAKELONG(*(++p), 0);
	//						DWORD dwC = MAKELONG(ch, 0);
	//						if(::CharLower((LPTSTR)ULongToPtr(dwP)) == ::CharLower((LPTSTR)ULongToPtr(dwC)))
	//						{
	//							if(!bFound)
	//							{
	//								wMnem = (WORD)i;
	//								bFound = true;
	//							}
	//							else
	//							{
	//								nRetCode = MNC_SELECT;
	//								break;
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//		if(bFound)
	//		{
	//			if(nRetCode == MNC_EXECUTE)
	//			{
	//				PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
	//				pT->GiveFocusBack();
	//			}
	//			bHandled = TRUE;
	//			lRet = MAKELRESULT(wMnem, nRetCode);
	//		}
	//	} 

	//	return lRet;
	//}

private:
	enum _CmdBarDrawConstants
	{
		s_kcxGap = 1,
		s_kcxTextMargin = 2,
		s_kcxButtonMargin = 3,
		s_kcyButtonMargin = 3
	};

	void GetSystemSettings()
	{
		// refresh our font
		NONCLIENTMETRICS info = { RunTimeHelper::SizeOf_NONCLIENTMETRICS() };
		BOOL bRet = ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, info.cbSize, &info, 0);
		ATLASSERT(bRet);
		if(bRet)
		{
			LOGFONT logfont;
			memset(&logfont, 0, sizeof(LOGFONT));
			if(m_cFontMenu.m_hFont != NULL)
				m_cFontMenu.GetLogFont(logfont);
			if(logfont.lfHeight != info.lfMenuFont.lfHeight ||
				logfont.lfWidth != info.lfMenuFont.lfWidth ||
				logfont.lfEscapement != info.lfMenuFont.lfEscapement ||
				logfont.lfOrientation != info.lfMenuFont.lfOrientation ||
				logfont.lfWeight != info.lfMenuFont.lfWeight ||
				logfont.lfItalic != info.lfMenuFont.lfItalic ||
				logfont.lfUnderline != info.lfMenuFont.lfUnderline ||
				logfont.lfStrikeOut != info.lfMenuFont.lfStrikeOut ||
				logfont.lfCharSet != info.lfMenuFont.lfCharSet ||
				logfont.lfOutPrecision != info.lfMenuFont.lfOutPrecision ||
				logfont.lfClipPrecision != info.lfMenuFont.lfClipPrecision ||
				logfont.lfQuality != info.lfMenuFont.lfQuality ||
				logfont.lfPitchAndFamily != info.lfMenuFont.lfPitchAndFamily ||
				lstrcmp(logfont.lfFaceName, info.lfMenuFont.lfFaceName) != 0)
			{
				HFONT hFontMenu = ::CreateFontIndirect(&info.lfMenuFont);
				ATLASSERT(hFontMenu != NULL);
				if(hFontMenu != NULL)
				{
					if(m_cFontMenu.m_hFont != NULL)
						m_cFontMenu.DeleteObject();
					m_cFontMenu.Attach(hFontMenu);
				}
			}
		}

		// check if we need extra spacing for menu item text
		CWindowDC dc(static_cast<T*>(this)->m_hWnd);
		m_fScaling = GetDeviceCaps(dc, LOGPIXELSX)/96.0f;
		if (m_fScaling < 1.0f) m_fScaling = 1.0f;
		HFONT hFontOld = dc.SelectFont(m_cFontMenu);
		RECT rcText = { 0, 0, 0, 0 };
		dc.DrawText(_T("\t"), -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);
		if((rcText.right - rcText.left) < 4)
		{
			::SetRectEmpty(&rcText);
			dc.DrawText(_T("x"), -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);
			m_cxExtraSpacing = rcText.right - rcText.left;
		}
		else
		{
			m_cxExtraSpacing = 0;
		}
		dc.SelectFont(hFontOld);

		// get Windows version
		OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
		::GetVersionEx(&ovi);

		// query flat menu mode (Windows XP or later)
		if((ovi.dwMajorVersion == 5 && ovi.dwMinorVersion >= 1) || (ovi.dwMajorVersion > 5))
		{
	#ifndef SPI_GETFLATMENU
			const UINT SPI_GETFLATMENU = 0x1022;
	#endif //!SPI_GETFLATMENU
			BOOL bRetVal = FALSE;
			bRet = ::SystemParametersInfo(SPI_GETFLATMENU, 0, &bRetVal, 0);
			m_bFlatMenus = (bRet && bRetVal);
		}
	}

	void DrawItem3D(LPDRAWITEMSTRUCT a_pDrawItemStruct, LPCTSTR a_pszText, int a_nIconIndex)
	{
		USES_CONVERSION;

		CDCHandle dc = a_pDrawItemStruct->hDC;
		const RECT& rcItem = a_pDrawItemStruct->rcItem;

		BOOL bDisabled = a_pDrawItemStruct->itemState & ODS_GRAYED;
		BOOL bSelected = a_pDrawItemStruct->itemState & ODS_SELECTED;
		BOOL bChecked = a_pDrawItemStruct->itemState & ODS_CHECKED;
		BOOL bHasImage = FALSE;

		SIZE szButton = {m_nIconY+2*int(s_kcyButtonMargin*m_fScaling+0.5f), m_nIconY+2*int(s_kcxButtonMargin*m_fScaling+0.5f)};
		SIZE szBitmap = {m_nIconX, m_nIconY};
		RECT rcButn = { rcItem.left, rcItem.top, rcItem.left + szButton.cx, rcItem.top + szButton.cy };			// button rect
		::OffsetRect(&rcButn, 0, ((rcItem.bottom - rcItem.top) - (rcButn.bottom - rcButn.top)) / 2);	// center vertically

		if (a_nIconIndex >= 0)
		{
			bHasImage = TRUE;

			// calc drawing point
			SIZE sz = { rcButn.right - rcButn.left - szBitmap.cx, rcButn.bottom - rcButn.top - szBitmap.cy };
			sz.cx /= 2;
			sz.cy /= 2;
			POINT point = { rcButn.left + sz.cx, rcButn.top + sz.cy };

			// draw disabled or normal
			if(!bDisabled)
			{
				// normal - fill background depending on state
				if(!bChecked || bSelected)
				{
					dc.FillRect(&rcButn, (bChecked && !bSelected) ? COLOR_3DLIGHT : COLOR_MENU);
				}
				else
				{
					COLORREF crTxt = dc.SetTextColor(::GetSysColor(COLOR_BTNFACE));
					COLORREF crBk = dc.SetBkColor(::GetSysColor(COLOR_BTNHILIGHT));
					CBrush hbr(CDCHandle::GetHalftoneBrush());
					dc.SetBrushOrg(rcButn.left, rcButn.top);
					dc.FillRect(&rcButn, hbr);
					dc.SetTextColor(crTxt);
					dc.SetBkColor(crBk);
				}

				// draw pushed-in or popped-out edge
				if(bSelected || bChecked)
				{
					RECT rc2 = rcButn;
					dc.DrawEdge(&rc2, bChecked ? BDR_SUNKENOUTER : BDR_RAISEDINNER, BF_RECT);
				}
				// draw the image
				::ImageList_Draw(m_hImageList, a_nIconIndex, dc, point.x, point.y, ILD_TRANSPARENT);
			}
			else
			{
				HBRUSH hBrushBackground = ::GetSysColorBrush(COLOR_MENU);
				DrawBitmapDisabled(dc, a_nIconIndex, point, hBrushBackground);
			}
		}
		else
		{
			// no image - look for custom checked/unchecked bitmaps
			CMenuItemInfo info;
			info.fMask = MIIM_CHECKMARKS | MIIM_TYPE;
			::GetMenuItemInfo((HMENU)a_pDrawItemStruct->hwndItem, a_pDrawItemStruct->itemID, MF_BYCOMMAND, &info);
			if(bChecked || info.hbmpUnchecked != NULL)
			{
				BOOL bRadio = ((info.fType & MFT_RADIOCHECK) != 0);
				bHasImage = DrawCheckmark(dc, rcButn, bSelected, bDisabled, bRadio, bChecked ? info.hbmpChecked : info.hbmpUnchecked);
			}
		}

		// draw item text
		int cxButn = szButton.cx;
		COLORREF colorBG = ::GetSysColor(bSelected ? COLOR_HIGHLIGHT : COLOR_MENU);
		if(bSelected || a_pDrawItemStruct->itemAction == ODA_SELECT)
		{
			RECT rcBG = rcItem;
			if(bHasImage)
				rcBG.left += cxButn + int(s_kcxGap*m_fScaling+0.5f);
			dc.FillRect(&rcBG, bSelected && (!bDisabled || a_pszText && *a_pszText) ? COLOR_HIGHLIGHT : COLOR_MENU);
		}

		// calc text rectangle and colors
		RECT rcText = rcItem;
		rcText.left += cxButn + int(s_kcxGap*m_fScaling+0.5f) + int(s_kcxTextMargin*m_fScaling+0.5f);
		rcText.right -= cxButn;
		dc.SetBkMode(TRANSPARENT);
		COLORREF colorText = ::GetSysColor(bDisabled ?  (bSelected ? COLOR_GRAYTEXT : COLOR_3DSHADOW) : (bSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));

		// font already selected by Windows
		if(bDisabled && (!bSelected || colorText == colorBG))
		{
			// disabled - draw shadow text shifted down and right 1 pixel (unles selected)
			RECT rcDisabled = rcText;
			::OffsetRect(&rcDisabled, 1, 1);
			DrawMenuText(dc, rcDisabled, a_pszText, ::GetSysColor(COLOR_3DHILIGHT));
		}
		DrawMenuText(dc, rcText, a_pszText, colorText); // finally!
	}

	void DrawItemFlat(LPDRAWITEMSTRUCT a_pDrawItemStruct, LPCTSTR a_pszText, int a_nIconIndex)
	{
		USES_CONVERSION;

		CDCHandle dc = a_pDrawItemStruct->hDC;
		const RECT& rcItem = a_pDrawItemStruct->rcItem;

		BOOL bDisabled = a_pDrawItemStruct->itemState & ODS_GRAYED;
		BOOL bSelected = a_pDrawItemStruct->itemState & ODS_SELECTED;
		BOOL bChecked = a_pDrawItemStruct->itemState & ODS_CHECKED;

		// paint background
		if(bSelected || a_pDrawItemStruct->itemAction == ODA_SELECT)
		{
			if(bSelected && (!bDisabled || a_pszText && *a_pszText))
			{
				dc.FillRect(&rcItem, ::GetSysColorBrush(COLOR_MENUHILIGHT));
				dc.FrameRect(&rcItem, ::GetSysColorBrush(COLOR_HIGHLIGHT));
			}
			else
			{
				dc.FillRect(&rcItem, ::GetSysColorBrush(COLOR_MENU));
			}
		}

		SIZE szButton = {m_nIconY+2*int(s_kcyButtonMargin*m_fScaling+0.5f), m_nIconY+2*int(s_kcxButtonMargin*m_fScaling+0.5f)};
		SIZE szBitmap = {m_nIconX, m_nIconY};
		RECT rcButn = { rcItem.left, rcItem.top, rcItem.left + szButton.cx, rcItem.top + szButton.cy };			// button rect
		::OffsetRect(&rcButn, 0, ((rcItem.bottom - rcItem.top) - (rcButn.bottom - rcButn.top)) / 2);	// center vertically

		// draw background and border for checked items
		if(bChecked)
		{
			RECT rcCheck = rcButn;
			::InflateRect(&rcCheck, -1, -1);
			if(bSelected)
				dc.FillRect(&rcCheck, ::GetSysColorBrush(COLOR_MENU));
			dc.FrameRect(&rcCheck, ::GetSysColorBrush(COLOR_HIGHLIGHT));
		}

		if (a_nIconIndex >= 0)
		{
			// calc drawing point
			SIZE sz = { rcButn.right - rcButn.left - szBitmap.cx, rcButn.bottom - rcButn.top - szBitmap.cy };
			sz.cx /= 2;
			sz.cy /= 2;
			POINT point = { rcButn.left + sz.cx, rcButn.top + sz.cy };

			// draw disabled or normal
			if(!bDisabled)
			{
				::ImageList_Draw(m_hImageList, a_nIconIndex, dc, point.x, point.y, ILD_TRANSPARENT);
			}
			else
			{
				HBRUSH hBrushBackground = ::GetSysColorBrush(bSelected ? COLOR_MENUHILIGHT : COLOR_MENU);
				HBRUSH hBrushDisabledImage = ::GetSysColorBrush(COLOR_3DSHADOW);
				DrawBitmapDisabled(dc, a_nIconIndex, point, hBrushBackground, hBrushBackground, hBrushDisabledImage);
			}
		}
		else
		{
			// no image - look for custom checked/unchecked bitmaps
			CMenuItemInfo info;
			info.fMask = MIIM_CHECKMARKS | MIIM_TYPE;
			::GetMenuItemInfo((HMENU)a_pDrawItemStruct->hwndItem, a_pDrawItemStruct->itemID, MF_BYCOMMAND, &info);
			if(bChecked || info.hbmpUnchecked != NULL)
			{
				BOOL bRadio = ((info.fType & MFT_RADIOCHECK) != 0);
				DrawCheckmark(dc, rcButn, bSelected, bDisabled, bRadio, bChecked ? info.hbmpChecked : info.hbmpUnchecked);
			}
		}

		// draw item text
		int cxButn = szButton.cx;
		// calc text rectangle and colors
		RECT rcText = rcItem;
		rcText.left += cxButn + int(s_kcxGap*m_fScaling+0.5f) + int(s_kcxTextMargin*m_fScaling+0.5f);
		rcText.right -= cxButn;
		dc.SetBkMode(TRANSPARENT);
		COLORREF colorText = ::GetSysColor(bDisabled ?  (bSelected ? COLOR_GRAYTEXT : COLOR_3DSHADOW) : (bSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));

		DrawMenuText(dc, rcText, a_pszText, colorText); // finally!
	}

	void DrawMenuText(CDCHandle& dc, RECT& rc, LPCTSTR lpstrText, COLORREF color)
	{
		int nTab = -1;
		for(int i = 0; i < lstrlen(lpstrText); i++)
		{
			if(lpstrText[i] == '\t')
			{
				nTab = i;
				break;
			}
		}
		dc.SetTextColor(color);
		dc.DrawText(lpstrText, nTab, &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER /*| (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX)*/);
		if(nTab != -1)
			dc.DrawText(&lpstrText[nTab + 1], -1, &rc, DT_SINGLELINE | DT_RIGHT | DT_VCENTER /*| (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX)*/);
	}

	void DrawBitmapDisabled(CDCHandle& dc, int nImage, POINT point,
		HBRUSH hBrushBackground = ::GetSysColorBrush(COLOR_3DFACE),
		HBRUSH hBrush3DEffect = ::GetSysColorBrush(COLOR_3DHILIGHT),
		HBRUSH hBrushDisabledImage = ::GetSysColorBrush(COLOR_3DSHADOW))
	{
		IMAGELISTDRAWPARAMS ildp;
		memset(&ildp, 0, sizeof(ildp));
		ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
		ildp.himl = m_hImageList;
		ildp.i = nImage;
		ildp.hdcDst = dc;
		ildp.x = point.x;
		ildp.y = point.y;
		ildp.cx = 0;
		ildp.cy = 0;
		ildp.xBitmap = 0;
		ildp.yBitmap = 0;
		ildp.fStyle = ILD_TRANSPARENT;
		ildp.fState = ILS_SATURATE;
		ildp.Frame = (DWORD)-255;
		::ImageList_DrawIndirect(&ildp);
	}

	BOOL DrawCheckmark(CDCHandle& dc, const RECT& rc, BOOL bSelected, BOOL bDisabled, BOOL bRadio, HBITMAP hBmpCheck)
	{
		// get checkmark bitmap, if none, use Windows standard
		SIZE size = { 0, 0 };
		CBitmapHandle bmp = hBmpCheck;
		if(hBmpCheck != NULL)
		{
			bmp.GetSize(size);
		}
		else
		{
			size.cx = ::GetSystemMetrics(SM_CXMENUCHECK); 
			size.cy = ::GetSystemMetrics(SM_CYMENUCHECK); 
			bmp.CreateCompatibleBitmap(dc, size.cx, size.cy);
			ATLASSERT(bmp.m_hBitmap != NULL);
		}
		// center bitmap in caller's rectangle
		RECT rcDest = rc;
		if((rc.right - rc.left) > size.cx)
		{
			rcDest.left = rc.left + (rc.right - rc.left - size.cx) / 2;
			rcDest.right = rcDest.left + size.cx;
		}
		if((rc.bottom - rc.top) > size.cy)
		{
			rcDest.top = rc.top + (rc.bottom - rc.top - size.cy) / 2;
			rcDest.bottom = rcDest.top + size.cy;
		}
		// paint background
		if(!m_bFlatMenus && !bDisabled)
		{
			if(bSelected)
			{
				dc.FillRect(&rcDest, COLOR_MENU);
			}
			else
			{
				COLORREF clrTextOld = dc.SetTextColor(::GetSysColor(COLOR_BTNFACE));
				COLORREF clrBkOld = dc.SetBkColor(::GetSysColor(COLOR_BTNHILIGHT));
				CBrush hbr(CDCHandle::GetHalftoneBrush());
				dc.SetBrushOrg(rcDest.left, rcDest.top);
				dc.FillRect(&rcDest, hbr);
				dc.SetTextColor(clrTextOld);
				dc.SetBkColor(clrBkOld);
			}
		}

		// create source image
		CDC dcSource;
		dcSource.CreateCompatibleDC(dc);
		HBITMAP hBmpOld = dcSource.SelectBitmap(bmp);
		// set colors
		const COLORREF clrBlack = RGB(0, 0, 0);
		const COLORREF clrWhite = RGB(255, 255, 255);
		COLORREF clrTextOld = dc.SetTextColor(clrBlack);
		COLORREF clrBkOld = dc.SetBkColor(clrWhite);
		// create mask
		CDC dcMask;
		dcMask.CreateCompatibleDC(dc);
		CBitmap bmpMask;
		bmpMask.CreateBitmap(size.cx, size.cy, 1, 1, NULL);
		HBITMAP hBmpOld1 = dcMask.SelectBitmap(bmpMask);

		// draw the checkmark transparently
		int cx = rcDest.right - rcDest.left;
		int cy = rcDest.bottom - rcDest.top;	
		if(hBmpCheck != NULL)
		{
			// build mask based on transparent color	
			dcSource.SetBkColor(RGB(192, 192, 192));
			dcMask.SetBkColor(clrBlack);
			dcMask.SetTextColor(clrWhite);
			dcMask.BitBlt(0, 0, size.cx, size.cy, dcSource, 0, 0, SRCCOPY);
			// draw bitmap using the mask
			dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, SRCINVERT);
			dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcMask, 0, 0, SRCAND);
			dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, SRCINVERT);
		}
		else
		{
			const DWORD ROP_DSno = 0x00BB0226L;
			const DWORD ROP_DSa = 0x008800C6L;
			const DWORD ROP_DSo = 0x00EE0086L;
			const DWORD ROP_DSna = 0x00220326L;

			// draw mask
			RECT rcSource = { 0, 0, min(size.cx, rc.right - rc.left), min(size.cy, rc.bottom - rc.top) };
			dcMask.DrawFrameControl(&rcSource, DFC_MENU, bRadio ? DFCS_MENUBULLET : DFCS_MENUCHECK);

			// draw shadow if disabled
			if(!m_bFlatMenus && bDisabled)
			{
				// offset by one pixel
				int x = rcDest.left + 1;
				int y = rcDest.top + 1;
				// paint source bitmap
				const int nColor = COLOR_3DHILIGHT;
				dcSource.FillRect(&rcSource, nColor);
				// draw checkmark - special case black and white colors
				COLORREF clrCheck = ::GetSysColor(nColor);
				if(clrCheck == clrWhite)
				{
					dc.BitBlt(x, y, cx, cy, dcMask,  0, 0,   ROP_DSno);
					dc.BitBlt(x, y, cx, cy, dcSource, 0, 0, ROP_DSa);
				}
				else
				{
					if(clrCheck != clrBlack)
					{
						ATLASSERT(dcSource.GetTextColor() == clrBlack);
						ATLASSERT(dcSource.GetBkColor() == clrWhite);
						dcSource.BitBlt(0, 0, size.cx, size.cy, dcMask, 0, 0, ROP_DSna);
					}
					dc.BitBlt(x, y, cx, cy, dcMask,  0,  0,  ROP_DSa);
					dc.BitBlt(x, y, cx, cy, dcSource, 0, 0, ROP_DSo);
				}
			}

			// paint source bitmap
			const int nColor = bDisabled ? COLOR_BTNSHADOW : COLOR_MENUTEXT;
			dcSource.FillRect(&rcSource, nColor);
			// draw checkmark - special case black and white colors
			COLORREF clrCheck = ::GetSysColor(nColor);
			if(clrCheck == clrWhite)
			{
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcMask,  0, 0,   ROP_DSno);
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, ROP_DSa);
			}
			else
			{
				if(clrCheck != clrBlack)
				{
					ATLASSERT(dcSource.GetTextColor() == clrBlack);
					ATLASSERT(dcSource.GetBkColor() == clrWhite);
					dcSource.BitBlt(0, 0, size.cx, size.cy, dcMask, 0, 0, ROP_DSna);
				}
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcMask,  0,  0,  ROP_DSa);
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, ROP_DSo);
			}
		}
		// restore all
		dc.SetTextColor(clrTextOld);			
		dc.SetBkColor(clrBkOld);
		dcSource.SelectBitmap(hBmpOld);
		dcMask.SelectBitmap(hBmpOld1);
		if(hBmpCheck == NULL)
			bmp.DeleteObject();
		// draw pushed-in hilight
		if(!m_bFlatMenus && !bDisabled)
		{
			if(rc.right - rc.left > size.cx)
				::InflateRect(&rcDest, 1,1);	// inflate checkmark by one pixel all around
			dc.DrawEdge(&rcDest, BDR_SUNKENOUTER, BF_RECT);
		}

		return TRUE;
	}


private:
	struct SItem
	{
		std::basic_string<TCHAR> strText;
		int nIconIndex;
	};
	typedef std::map<UINT, SItem> CItems;

private:
	CFont m_cFontMenu;
	bool m_bFlatMenus;
	HIMAGELIST m_hImageList;
	CItems m_cItems;
	int m_nIconX;
	int m_nIconY;
	int m_cxExtraSpacing;
	float m_fScaling;
};
