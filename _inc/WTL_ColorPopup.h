
#pragma once


namespace WTL
{

class CColorPopup :
	public CWindowImpl<CColorPopup>
{
public:
	enum
	{
		FROMPIXEL_BOX_VALUE	= -4,
		DEFAULT_BOX_VALUE	= -3,
		CUSTOM_BOX_VALUE	= -2,
		INVALID_COLOR		= -1,
		OFFSET_AUTOCOLORS	= 0x10000,
		OFFSET_LASTCOLORS	= 0x20000,
		COLORINDEX_MASK		= 0xffff,
	};

	struct SColor
	{
		SColor() : fR(0.0f), fG(0.0f), fB(0.0f), fA(0.0f) {}
		SColor(float a_fR, float a_fG, float a_fB, float a_fA) : fR(a_fR), fG(a_fG), fB(a_fB), fA(a_fA) {}
		SColor(DWORD a_clr, bool a_bAlpha = false) : fR(CGammaTables::FromSRGB(GetRValue(a_clr))), fG(CGammaTables::FromSRGB(GetGValue(a_clr))), fB(CGammaTables::FromSRGB(GetBValue(a_clr))), fA(a_bAlpha ? ((a_clr>>24)&0xff)/255.0f : 1.0f) {}

		float fR;
		float fG;
		float fB;
		float fA;
		DWORD ToCOLORREF() const
		{
			DWORD const nR = CGammaTables::ToSRGB(fR);
			DWORD const nG = CGammaTables::ToSRGB(fG);
			DWORD const nB = CGammaTables::ToSRGB(fB);
			return RGB(nR, nG, nB);
		}
		DWORD ToRGBA() const
		{
			DWORD const nR = CGammaTables::ToSRGB(fR);
			DWORD const nG = CGammaTables::ToSRGB(fG);
			DWORD const nB = CGammaTables::ToSRGB(fB);
			DWORD const nA = fA <= 0.0f ? 0 : (fA >= 1.0f ? 255 : (fA*255.0f+0.5f));
			return (nA<<24)|(nB<<16)|(nG<<8)|nR;
		}
		DWORD ToBGRA() const
		{
			DWORD const nR = CGammaTables::ToSRGB(fR);
			DWORD const nG = CGammaTables::ToSRGB(fG);
			DWORD const nB = CGammaTables::ToSRGB(fB);
			DWORD const nA = fA <= 0.0f ? 0 : (fA >= 1.0f ? 255 : (fA*255.0f+0.5f));
			return (nA<<24)|(nR<<16)|(nG<<8)|nB;
		}
		bool operator==(SColor const& a_rhs) const
		{
			return fabsf(fR-a_rhs.fR) < 1e-4f && fabsf(fG-a_rhs.fG) < 1e-4f && fabsf(fB-a_rhs.fB) < 1e-4f && fabsf(fA-a_rhs.fA) < 1e-4f;
		}
		bool operator!=(SColor const& a_rhs) const
		{
			return !operator==(a_rhs);
		}
	};

public:
	CColorPopup() :
		m_nColumns(8), m_nHotColumn(0), m_nGap(2),
		s_sizeTextHiBorder(3, 3), s_sizeTextMargin(2, 2), s_sizeBoxHiBorder(2, 2), s_sizeBoxMargin(0, 0), s_sizeBoxCore(14, 14)
	{
	}

	void SetTextItem(int a_iID, wchar_t const* a_psz, SColor const* a_pDef = NULL)
	{
		SEntry& s = m_cEntries[a_iID];
		s.strText = a_psz;
		if (a_pDef)
		{
			s.aColors.resize(1);
			s.aColors[0] = *a_pDef;
		}
		else
		{
			s.aColors.clear();
		}
	}
	template<typename TIterator>
	void SetColorsItem(int a_iID, TIterator a_begin, TIterator a_end)
	{
		SEntry& s = m_cEntries[a_iID];
		s.strText.clear();
		s.aColors.assign(a_begin, a_end);
	}
	void SetGapItem(int a_iID)
	{
		SEntry& s = m_cEntries[a_iID];
		s.aColors.clear();
		s.strText.clear();
	}
	void RemoveItem(int a_iID)
	{
		m_cEntries.erase(a_iID);
	}
	void RemoveAllItems()
	{
		m_cEntries.clear();
	}

	bool ShowPopup(HWND a_hParent, RECT const& a_rcButton, SColor const& a_tInitialColor, int* a_pSelectedEntry, SColor* a_pSelectedColor)
	{
		if (m_hWnd)
			return false; // another already popup visible

		HDC hDC = ::GetDC(a_hParent);
		float a_fScale = GetDeviceCaps(hDC, LOGPIXELSX)/96.0f;
		::ReleaseDC(a_hParent, hDC);

		_ASSERTE(s_sizeBoxHiBorder.cx == s_sizeBoxHiBorder.cy);
		_ASSERTE(s_sizeBoxMargin.cx == s_sizeBoxMargin.cy);
		m_sizeBox.cx = static_cast<int>(s_sizeBoxCore.cx*a_fScale+0.5f) + (s_sizeBoxHiBorder.cx + s_sizeBoxMargin.cx) * 2;
		m_sizeBox.cy = static_cast<int>(s_sizeBoxCore.cy*a_fScale+0.5f) + (s_sizeBoxHiBorder.cy + s_sizeBoxMargin.cy) * 2;

		m_fOked = FALSE;
		m_bMouseDown = false;

		//
		// See what version we are using
		//

		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof (osvi);
		::GetVersionEx(&osvi);
		bool fIsXP = osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && (osvi.dwMajorVersion > 5 || (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1));

		//
		// Get the flat flag
		//

		m_fPickerFlat = FALSE;
#if (_WIN32_WINNT >= 0x0501)
 		if (fIsXP)
			::SystemParametersInfo(SPI_GETFLATMENU, 0, &m_fPickerFlat, FALSE);
#endif

		//
		// Get all the colors I need
		//

		int nAlpha = 48;
		m_clrBackground = ::GetSysColor(COLOR_MENU);
		m_clrHiLightBorder = ::GetSysColor(COLOR_HIGHLIGHT);
		m_clrHiLight = m_clrHiLightBorder;
#if (WINVER >= 0x0501)
		if (fIsXP)
			m_clrHiLight = ::GetSysColor (COLOR_MENUHILIGHT);
#endif
		m_clrHiLightText = ::GetSysColor (COLOR_HIGHLIGHTTEXT);
		m_clrText = ::GetSysColor (COLOR_MENUTEXT);
		m_clrLoLight = RGB (
			(GetRValue (m_clrBackground) * (255 - nAlpha) + GetRValue (m_clrHiLightBorder) * nAlpha) >> 8,
			(GetGValue (m_clrBackground) * (255 - nAlpha) + GetGValue (m_clrHiLightBorder) * nAlpha) >> 8,
			(GetBValue (m_clrBackground) * (255 - nAlpha) + GetBValue (m_clrHiLightBorder) * nAlpha) >> 8);
	   
		// get the margins
		m_rectMargins.left = ::GetSystemMetrics(SM_CXEDGE);
		m_rectMargins.top = ::GetSystemMetrics(SM_CYEDGE);
		m_rectMargins.right = ::GetSystemMetrics(SM_CXEDGE);
		m_rectMargins.bottom = ::GetSystemMetrics(SM_CYEDGE);

		// create the font
		NONCLIENTMETRICS ncm;
		ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof ncm, &ncm, 0);
		m_font.CreateFontIndirect(&ncm.lfMessageFont);

		// compute dimensions
		int nTextHeight = 2*(s_sizeTextMargin.cy + s_sizeTextHiBorder.cy);
		{
			CClientDC dc(a_hParent);
			HFONT hfontOld = dc.SelectFont(m_font);
			SIZE szText = {0, 0};
			dc.GetTextExtent(_T("A1X"), 3, &szText);
			dc.SelectFont(hfontOld);
			nTextHeight += szText.cy;
		}

		m_nSelEntry = -1;
		m_nSelColor = 0;
		int nTotalWidth = m_nColumns*m_sizeBox.cx + m_rectMargins.left+m_rectMargins.right;
		int nTotalHeight = m_rectMargins.top;
		for (CEntries::iterator i = m_cEntries.begin(); i != m_cEntries.end(); ++i)
		{
			i->second.rc.left = m_rectMargins.left;
			i->second.rc.right = nTotalWidth-m_rectMargins.right;
			i->second.rc.top = nTotalHeight;
			if (!i->second.strText.empty())
			{
				i->second.rc.bottom = i->second.rc.top+nTextHeight;
			}
			else if (!i->second.aColors.empty())
			{
				i->second.rc.bottom = i->second.rc.top+m_sizeBox.cy*((i->second.aColors.size()+m_nColumns-1)/m_nColumns);
			}
			else
			{
				i->second.rc.bottom = i->second.rc.top+m_nGap*a_fScale+0.5f;
			}
			if (m_nSelEntry == -1 && !i->second.aColors.empty())
			{
				for (size_t j = 0; j < i->second.aColors.size(); ++j)
				{
					if (a_tInitialColor == i->second.aColors[j])
					{
						m_nSelEntry = i->first;
						m_nSelColor = j;
						break;
					}
				}
			}
			nTotalHeight = i->second.rc.bottom;
		}
		nTotalHeight += m_rectMargins.bottom;

		CRect rectWindow(a_rcButton.left, a_rcButton.bottom, a_rcButton.left+nTotalWidth, a_rcButton.bottom+nTotalHeight);

		// get the screen rectangle
		CRect rectScreen(CPoint(0, 0), CSize(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN)));
#if (WINVER >= 0x0500)
		HMODULE hUser32 = ::GetModuleHandleA("USER32.DLL");
		if (hUser32 != NULL)
		{
			typedef HMONITOR (WINAPI *FN_MonitorFromWindow) (HWND hWnd, DWORD dwFlags);
			typedef BOOL (WINAPI *FN_GetMonitorInfo) (HMONITOR hMonitor, LPMONITORINFO lpmi);
			FN_MonitorFromWindow pfnMonitorFromWindow = (FN_MonitorFromWindow)::GetProcAddress(hUser32, "MonitorFromWindow");
			FN_GetMonitorInfo pfnGetMonitorInfo = (FN_GetMonitorInfo)::GetProcAddress(hUser32, "GetMonitorInfoA");
			if (pfnMonitorFromWindow != NULL && pfnGetMonitorInfo != NULL)
			{
				MONITORINFO mi;
				HMONITOR hMonitor = pfnMonitorFromWindow(a_hParent, MONITOR_DEFAULTTONEAREST);
				mi.cbSize = sizeof mi;
				pfnGetMonitorInfo(hMonitor, &mi);
				rectScreen = mi.rcWork;
			}
		}
#endif

		// need to check it'll fit on screen: too far right?
		if (rectWindow.right > rectScreen.right)
			::OffsetRect(&rectWindow, rectScreen.right - rectWindow.right, 0);

		// too far left?
		if (rectWindow.left < rectScreen.left)
			::OffsetRect(&rectWindow, rectScreen.left - rectWindow.left, 0);

		// Bottom falling out of screen?  If so, the move
		// the whole popup above the parent's window
		if (rectWindow.bottom > rectScreen.bottom)
		{
			::OffsetRect(&rectWindow, 0, - ((a_rcButton.bottom - a_rcButton.top) + (rectWindow.bottom - rectWindow.top)));
		}

		//
		// Initialize our state
		// 

		m_clrPicker			= a_tInitialColor;

		// register the window class used for the picker
		WNDCLASSEX wc;
		wc.cbSize = sizeof (WNDCLASSEX);
		wc.style  = CS_CLASSDC | CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = StartWindowProc;
		wc.cbClsExtra  = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = _pModule->get_m_hInst();
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor (NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = _T("ColorPopup");
		wc.hIconSm = NULL;
#if (_WIN32_WINNT >= 0x0501)
		if (fIsXP)
		{
			BOOL fDropShadow;
			::SystemParametersInfo (SPI_GETDROPSHADOW, 0, &fDropShadow, FALSE);
			if (fDropShadow)
				wc.style |= CS_DROPSHADOW;
		}
#endif
		ATOM atom = ::RegisterClassEx(&wc);

		// create the window
		_pModule->AddCreateWndData(&m_thunk.cd, this);
		m_hWnd = ::CreateWindowEx(0, (LPCTSTR) MAKELONG (atom, 0), _T(""), WS_POPUP, rectWindow.left, rectWindow.top, rectWindow.Width(), rectWindow.Height(), a_hParent, NULL, _pModule->get_m_hInst(), NULL);
		if (m_hWnd == NULL)
		{
			::UnregisterClass((LPCTSTR) MAKELONG (atom, 0), _pModule->get_m_hInst());
			return false;
		}
		{
	        
			//
			// Create the tooltips
			//

			CToolTipCtrl sToolTip;
			CreatePickerToolTips(sToolTip);

			////
			//// Find which cell (if any) corresponds to the initial color
			////
			//FindPickerCellFromColor(a_tInitialColor);

			//
			// Make visible
			//
		
			ShowWindow(SW_SHOWNA);

			//
			// Purge the message queue of paints
			//

			MSG msg;
			while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
			{
				if (!GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
					return FALSE;
				DispatchMessage(&msg);
			}

			// 
			// Set capture to the window which received this message
			//

			SetCapture();
			_ASSERTE(m_hWnd == ::GetCapture());

			//
			// Get messages until capture lost or cancelled/accepted
			//

			while (m_hWnd == ::GetCapture())
			{
				MSG msg;
				if (!::GetMessage(&msg, NULL, 0, 0))
				{
					::PostQuitMessage(msg.wParam);
					break;
				}

				sToolTip.RelayEvent(&msg);

				switch (msg.message)
				{
					case WM_LBUTTONUP:
						{
							BOOL bHandled = TRUE;
							OnPickerLButtonUp(msg.message, msg.wParam, msg.lParam, bHandled);
						}
						break;

					case WM_MOUSEMOVE:
						{
							BOOL bHandled = TRUE;
							OnPickerMouseMove(msg.message, msg.wParam, msg.lParam, bHandled);
						}
						break;

					case WM_KEYUP:
						break;

					case WM_KEYDOWN:
						{
							BOOL bHandled = TRUE;
							OnPickerKeyDown(msg.message, msg.wParam, msg.lParam, bHandled);
						}
						break;

					case WM_RBUTTONDOWN:
						::ReleaseCapture();
						m_fOked = FALSE;
						break;

					case WM_LBUTTONDOWN:
						m_bMouseDown = true;

					// just dispatch rest of the messages
					default:
						DispatchMessage(&msg);
						break;
				}
			}
			::ReleaseCapture();

			//
			// Destroy the window
			//

			sToolTip.DestroyWindow();
			DestroyWindow();

			//
			// If needed, show custom
			//

			//if (fOked)
			//{
			//	m_clrCurrent = m_clrPicker;
			//}

			//
			// Clean up GDI objects
			//

			m_font.DeleteObject();
		}

		//
		// Unregister our class
		//

		::UnregisterClass((LPCTSTR) MAKELONG (atom, 0), _pModule->get_m_hInst());

		if (!m_fOked)
			return false;
		CEntries::const_iterator iSel = m_cEntries.find(m_nHotEntry);
		if (iSel == m_cEntries.end())
			return false;
		*a_pSelectedEntry = m_nHotEntry;
		if (iSel->second.aColors.empty())
			return true;
		if (m_nHotColor >= iSel->second.aColors.size())
			return false;
		*a_pSelectedColor = iSel->second.aColors[m_nHotColor];
		return true;
	}

	void SetDefaultColorsItem(int a_iID, bool a_bAlpha = true)
	{
		SEntry& s = m_cEntries[a_iID];
		s.strText.clear();
		s.aColors.clear();
		s.aColors.push_back(a_bAlpha ? SColor(0.0f, 0.0f, 0.0f, 0.0f) : SColor(RGB(0x00, 0x00, 0x00))); //_T("Black")
		s.aColors.push_back(SColor(RGB(0xA5, 0x2A, 0x00))); //_T("Brown")
		s.aColors.push_back(SColor(RGB(0x00, 0x40, 0x40))); //_T("Dark Olive Green")
		s.aColors.push_back(SColor(RGB(0x00, 0x55, 0x00))); //_T("Dark Green")
		s.aColors.push_back(SColor(RGB(0x00, 0x00, 0x5E))); //_T("Dark Teal")
		s.aColors.push_back(SColor(RGB(0x00, 0x00, 0x8B))); //_T("Dark blue")
		s.aColors.push_back(SColor(RGB(0x4B, 0x00, 0x82))); //_T("Indigo")
		s.aColors.push_back(a_bAlpha ? SColor(RGB(0x00, 0x00, 0x00)) : SColor(RGB(0x28, 0x28, 0x28))); //_T("Dark grey")
		s.aColors.push_back(SColor(RGB(0x8B, 0x00, 0x00))); //_T("Dark red")
		s.aColors.push_back(SColor(RGB(0xFF, 0x68, 0x20))); //_T("Orange")
		s.aColors.push_back(SColor(RGB(0x8B, 0x8B, 0x00))); //_T("Dark yellow")
		s.aColors.push_back(SColor(RGB(0x00, 0x93, 0x00))); //_T("Green")
		s.aColors.push_back(SColor(RGB(0x38, 0x8E, 0x8E))); //_T("Teal")
		s.aColors.push_back(SColor(RGB(0x00, 0x00, 0xFF))); //_T("Blue")
		s.aColors.push_back(SColor(RGB(0x7B, 0x7B, 0xC0))); //_T("Blue-grey")
		s.aColors.push_back(a_bAlpha ? SColor(RGB(0x40, 0x40, 0x40)) : SColor(RGB(0x66, 0x66, 0x66))); //_T("Grey - 40")
		s.aColors.push_back(SColor(RGB(0xFF, 0x00, 0x00))); //_T("Red")
		s.aColors.push_back(SColor(RGB(0xFF, 0xAD, 0x5B))); //_T("Light orange")
		s.aColors.push_back(SColor(RGB(0x32, 0xCD, 0x32))); //_T("Lime")
		s.aColors.push_back(SColor(RGB(0x3C, 0xB3, 0x71))); //_T("Sea green")
		s.aColors.push_back(SColor(RGB(0x7F, 0xFF, 0xD4))); //_T("Aqua")
		s.aColors.push_back(SColor(RGB(0x7D, 0x9E, 0xC0))); //_T("Light blue")
		s.aColors.push_back(SColor(RGB(0x80, 0x00, 0x80))); //_T("Violet")
		s.aColors.push_back(SColor(RGB(0x7F, 0x7F, 0x7F))); //_T("Grey - 50")
		s.aColors.push_back(SColor(RGB(0xFF, 0xC0, 0xCB))); //_T("Pink")
		s.aColors.push_back(SColor(RGB(0xFF, 0xD7, 0x00))); //_T("Gold")
		s.aColors.push_back(SColor(RGB(0xFF, 0xFF, 0x00))); //_T("Yellow")
		s.aColors.push_back(SColor(RGB(0x00, 0xFF, 0x00))); //_T("Bright green")
		s.aColors.push_back(SColor(RGB(0x40, 0xE0, 0xD0))); //_T("Turquoise")
		s.aColors.push_back(SColor(RGB(0xC0, 0xFF, 0xFF))); //_T("Skyblue")
		s.aColors.push_back(SColor(RGB(0x48, 0x00, 0x48))); //_T("Plum")
		s.aColors.push_back(SColor(RGB(0xC0, 0xC0, 0xC0))); //_T("Light grey")
		s.aColors.push_back(SColor(RGB(0xFF, 0xE4, 0xE1))); //_T("Rose")
		s.aColors.push_back(SColor(RGB(0xD2, 0xB4, 0x8C))); //_T("Tan")
		s.aColors.push_back(SColor(RGB(0xFF, 0xFF, 0xE0))); //_T("Light yellow")
		s.aColors.push_back(SColor(RGB(0x98, 0xFB, 0x98))); //_T("Pale green ")
		s.aColors.push_back(SColor(RGB(0xAF, 0xEE, 0xEE))); //_T("Pale turquoise")
		s.aColors.push_back(SColor(RGB(0x68, 0x83, 0x8B))); //_T("Pale blue")
		s.aColors.push_back(SColor(RGB(0xE6, 0xE6, 0xFA))); //_T("Lavender")
		s.aColors.push_back(SColor(RGB(0xFF, 0xFF, 0xFF))); //_T("White")
	}
	void SetSimilarColorsItem(int a_iID, SColor const& clrCurrent, bool a_bAlpha = true)
	{
		SEntry& s = m_cEntries[a_iID];
		s.strText.clear();
		s.aColors.clear();

		if (clrCurrent.fA >= 7.0f/255.0f)
		{
			if (clrCurrent.fR == clrCurrent.fG && clrCurrent.fR == clrCurrent.fB)
			{
				SColor tColor = clrCurrent;
				// shades of grey
				for (int i = 0; i < 8; ++i)
				{
					tColor.fR = tColor.fG = tColor.fB = i/7.0f;
					s.aColors.push_back(tColor);
				}
			}
			else
			{
				float h, l, fS;
				RGB2HLS(clrCurrent.fR, clrCurrent.fG, clrCurrent.fB, h, l, fS);
				SColor tColor = clrCurrent;
				// luminance
				for (int i = 0; i < 8; ++i)
				{
					HLS2RGB(h, i/7.0f, fS, tColor.fR, tColor.fG, tColor.fB);
					s.aColors.push_back(tColor);
				}
				// saturation
				for (int i = 0; i < 8; ++i)
				{
					HLS2RGB(h, (l<0.2f || l>0.8f) ? 0.5f : l, i/7.0f, tColor.fR, tColor.fG, tColor.fB);
					s.aColors.push_back(tColor);
				}
			}
		}
		if (a_bAlpha)
		{
			SColor tColor = clrCurrent;
			// add colors with different level of transparency
			for (int i = 0; i < 8; ++i)
			{
				tColor.fA = i/7.0f;
				s.aColors.push_back(tColor);
			}
		}
		if (s.aColors.size() == 0)
			m_cEntries.erase(a_iID);
	}


public:
	BEGIN_MSG_MAP(CColorPopup)
		MESSAGE_HANDLER(WM_PAINT, OnPickerPaint);
	END_MSG_MAP()

protected:

	// @cmember Handle key down for picker

	LRESULT OnPickerKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		UINT nChar = wParam;

		int nOffsetX = 0;
		int nOffsetY = 0;
		switch (nChar)
		{
			case VK_DOWN:
				nOffsetY = 1;
				break;

			case VK_UP:
				nOffsetY = -1;
				break;

			case VK_RIGHT:
				nOffsetX = 1;
				break;

			case VK_LEFT:
				nOffsetX = -1;
				break;

			case VK_ESCAPE:
				EndPickerSelection(FALSE);
				break;

			case VK_RETURN:
			case VK_SPACE:
				EndPickerSelection(m_cEntries.find(m_nHotEntry) != m_cEntries.end());
				break;
		}

		int nPrevEntry = m_nHotEntry;
		int nPrevColor = m_nHotColor;
		CEntries::const_iterator i = m_cEntries.find(m_nHotEntry);
		if (i == m_cEntries.end())
		{
			if (nOffsetX > 0 || nOffsetY > 0)
			{
				m_nHotEntry = m_cEntries.begin()->first;
			}
			else
			{
				m_nHotEntry = m_cEntries.rbegin()->first;
			}
		}
		else
		{
			if (nOffsetX && !i->second.aColors.empty())
			{
				m_nHotColor = ((m_nHotColor/m_nColumns)*m_nColumns)+((m_nHotColor+m_nColumns+nOffsetX)%m_nColumns);
				m_nHotColumn = m_nHotColor%m_nColumns;
			}
			else if (nOffsetY)
			{
				if (!i->second.aColors.empty())
				{
					size_t nNew = m_nHotColor+m_nColumns*nOffsetY;
					if (nNew < i->second.aColors.size())
					{
						m_nHotColor = nNew;
						nOffsetY = 0;
					}
				}
				if (nOffsetY > 0)
				{
					do
					{
						++i;
						if (i == m_cEntries.end())
							i = m_cEntries.begin();
					}
					while (i->second.strText.empty() && i->second.aColors.empty());
					m_nHotEntry = i->first;
					if (!i->second.aColors.empty())
					{
						m_nHotColor = m_nHotColumn;
					}
				}
				else if (nOffsetY < 0)
				{
					do
					{
						if (i == m_cEntries.begin())
							i = m_cEntries.end();
						--i;
					}
					while (i->second.strText.empty() && i->second.aColors.empty());
					m_nHotEntry = i->first;
					if (!i->second.aColors.empty())
					{
						m_nHotColor = ((i->second.aColors.size()-1)/m_nColumns)*m_nColumns + m_nHotColumn;
					}
				}
			}

			if (!i->second.aColors.empty() && m_nHotColor >= i->second.aColors.size())
			{
				m_nHotColor = i->second.aColors.size()-1;
			}
		}

		if (nPrevEntry != m_nHotEntry || nPrevColor != m_nHotColor)
		{
			RedrawPickerSelection(nPrevEntry, nPrevColor);
		}

		bHandled = FALSE;
		return FALSE;
	}

	LRESULT OnPickerLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		int nPrevEntry = m_nHotEntry;
		size_t nPrevColor = m_nHotColor;
		bool bHit = PickerHitTest(pt, &m_nHotEntry, &m_nHotColor);

		if (nPrevEntry != m_nHotEntry || nPrevColor != m_nHotColor)
		{
			RedrawPickerSelection(nPrevEntry, nPrevColor);
		}
		if (bHit || m_bMouseDown)
			EndPickerSelection(bHit);
		return 0;
	}

	LRESULT OnPickerMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		int nPrevEntry = m_nHotEntry;
		size_t nPrevColor = m_nHotColor;
		if (!PickerHitTest(pt, &m_nHotEntry, &m_nHotColor))
		{
			m_nHotEntry = -1;
			m_nHotColor = 0;
		}
		else
		{
			CEntries::const_iterator i = m_cEntries.find(m_nHotEntry);
			if (i != m_cEntries.end())
				m_nHotColumn = m_nHotColor%m_nColumns;
		}

		if (nPrevEntry != m_nHotEntry || nPrevColor != m_nHotColor)
		{
			RedrawPickerSelection(nPrevEntry, nPrevColor);
		}
		return 0;
	}

	LRESULT OnPickerPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		CPaintDC dc(m_hWnd);

		CRect rect;
		GetClientRect(&rect);
		if (m_fPickerFlat)
		{
			CPen pen;
			pen.CreatePen(PS_SOLID, 0, ::GetSysColor (COLOR_GRAYTEXT));
			HPEN hpenOld = dc.SelectPen(pen);
			dc.Rectangle(rect.left, rect.top, rect.Width(), rect.Height());
			dc.SelectPen(hpenOld);
		}
		else
		{
			dc.DrawEdge(&rect, EDGE_RAISED, BF_RECT);
		}

		for (CEntries::const_iterator i = m_cEntries.begin(); i != m_cEntries.end(); ++i)
		{
			if (!i->second.strText.empty())
			{
				DrawPickerTextCell(dc, i->second.rc, i->second.strText.c_str(), i->first);
			}
			else if (!i->second.aColors.empty())
			{
				for (size_t j = 0; j != i->second.aColors.size(); ++j)
				{
					RECT rc = i->second.rc;
					rc.left += (j%m_nColumns)*m_sizeBox.cx;
					rc.right = rc.left+m_sizeBox.cx;
					rc.top += (j/m_nColumns)*m_sizeBox.cy;
					rc.bottom = rc.top+m_sizeBox.cy;
					DrawPickerColorCell(dc, rc, i->second.aColors[j], i->first, j);
				}
			}
			else
			{
				dc.FillRect(&i->second.rc, COLOR_MENU);
			}
		}

		return 0;
	}

	// @cmember Create the picker tooltips

	void CreatePickerToolTips(CToolTipCtrl &sToolTip)
	{
		//
		// Create the tool tip
		//

		if (!sToolTip.Create(m_hWnd)) 
			return;

		CConfigValue cRng(100.0f);
		CConfigValue cDec(1L);
		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		CComPtr<IConfig> pColorsCfg;
		if (pMgr)
		{
			static GUID const tColorGCID = {0x2CBE06C7, 0x4847, 0x4766, {0xAA, 0x01, 0x22, 0x6A, 0xF5, 0x2D, 0x54, 0x88}}; // CLSID_DesignerViewFactoryColorSwatch
			pMgr->Config(tColorGCID, &pColorsCfg);
			pMgr = NULL;
		}
		if (pColorsCfg)
		{
			pColorsCfg->ItemValueGet(CComBSTR(L"Factor"), &cRng);
			pColorsCfg->ItemValueGet(CComBSTR(L"Decimal"), &cDec);
			pColorsCfg = NULL;
		}
		float const fMul1 = cRng.operator float()*powf(10, cDec.operator LONG());
		float const fMul2 = powf(10, -cDec.operator LONG());

		//
		// Add a tool for each cell
		// 
		for (CEntries::const_iterator j = m_cEntries.begin(); j != m_cEntries.end(); ++j)
		{
			for (size_t i = 0; i < j->second.aColors.size(); ++i)
			{
				RECT rc = j->second.rc;
				rc.left += (i%m_nColumns)*m_sizeBox.cx;
				rc.right = rc.left+m_sizeBox.cx;
				rc.top += (i/m_nColumns)*m_sizeBox.cy;
				rc.bottom = rc.top+m_sizeBox.cy;
				TCHAR szTmp[256];
				_stprintf(szTmp, _T("R: %g\r\nG: %g\r\nB: %g\r\nA: %g"), int(j->second.aColors[i].fR*fMul1)*fMul2, int(j->second.aColors[i].fG*fMul1)*fMul2, int(j->second.aColors[i].fB*fMul1)*fMul2, int(j->second.aColors[i].fA*fMul1)*fMul2);
				sToolTip.AddTool(m_hWnd, szTmp, &rc, 1);
			}
		}

		sToolTip.SetMaxTipWidth(800);
	}

	void RedrawPickerSelection(int a_nPrevEntry, size_t a_nPrevColor)
	{
		CClientDC dc(m_hWnd);

		int aE[2] = {a_nPrevEntry, m_nHotEntry};
		size_t aC[2] = {a_nPrevColor, m_nHotColor};
		for (int j = 0; j < 2; ++j)
		{
			CEntries::const_iterator i = m_cEntries.find(aE[j]);
			if (i == m_cEntries.end())
				continue;
			if (!i->second.strText.empty())
			{
				DrawPickerTextCell(dc, i->second.rc, i->second.strText.c_str(), i->first);
			}
			else if (!i->second.aColors.empty() && aC[j] < i->second.aColors.size())
			{
				RECT rc = i->second.rc;
				rc.left += (aC[j]%m_nColumns)*m_sizeBox.cx;
				rc.right = rc.left+m_sizeBox.cx;
				rc.top += (aC[j]/m_nColumns)*m_sizeBox.cy;
				rc.bottom = rc.top+m_sizeBox.cy;
				DrawPickerColorCell(dc, rc, i->second.aColors[aC[j]], i->first, aC[j]);
			}
		}
	}

	// @cmember End the picker selection process

	void EndPickerSelection(BOOL fOked)
	{
		::ReleaseCapture();
		m_fOked = fOked;
	}

	void DrawPickerTextCell(CDC &dc, CRect rect, wchar_t const* a_psz, int iItem)
	{
		COLORREF clrHiLight;
		COLORREF clrText;
		bool fSelected;
		if (m_nHotEntry == iItem)
		{
			fSelected = true;
			clrHiLight = m_clrHiLight;
			clrText = m_clrHiLightText;
		}
		else if (m_nSelEntry == iItem)
		{
			fSelected = true;
			clrHiLight = m_clrLoLight;
			clrText = m_clrText;
		}
		else
		{
			fSelected = false;
			clrHiLight = m_clrLoLight;
			clrText = m_clrText;
		}

		if (fSelected)
		{
			// if we have a background margin, then draw that
			if (s_sizeTextMargin.cx > 0 || s_sizeTextMargin.cy > 0)
			{
				dc.SetBkColor(m_clrBackground);
				dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
				rect.InflateRect(-s_sizeTextMargin.cx, -s_sizeTextMargin.cy);
			}

			// draw the selection rectagle
			dc.SetBkColor(m_clrHiLightBorder);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			rect.InflateRect(-1, -1);

			// draw the inner coloring
			dc.SetBkColor(clrHiLight);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			rect.InflateRect(-(s_sizeTextHiBorder.cx - 1), -(s_sizeTextHiBorder.cy - 1));
		}
		else
		{
			// draw unselected background
			dc.SetBkColor(m_clrBackground);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			rect.InflateRect(-(s_sizeTextMargin.cx + s_sizeTextHiBorder.cx), -(s_sizeTextMargin.cy + s_sizeTextHiBorder.cy));
		}

		// draw custom text
		HFONT hfontOld = dc.SelectFont(m_font);
		dc.SetTextColor(clrText);
		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(a_psz, wcslen(a_psz), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		dc.SelectFont(hfontOld);
	}
	void DrawPickerColorCell(CDC &dc, CRect rect, SColor const& clrBox, int iItem, size_t iColor)
	{
		// based on the selections, get our colors
		COLORREF clrHiLight;
		COLORREF clrText;
		bool fSelected;
		if (m_nHotEntry == iItem && m_nHotColor == iColor)
		{
			fSelected = true;
			clrHiLight = m_clrHiLight;
			clrText = m_clrHiLightText;
		}
		else if (m_nSelEntry == iItem && m_nSelColor == iColor)
		{
			fSelected = true;
			clrHiLight = m_clrLoLight;
			clrText = m_clrText;
		}
		else
		{
			fSelected = false;
			clrHiLight = m_clrLoLight;
			clrText = m_clrText;
		}

		if (fSelected)
		{
			// if we have a background margin, then draw that
			if (s_sizeBoxMargin.cx > 0 || s_sizeBoxMargin.cy > 0)
			{
				dc.SetBkColor(m_clrBackground);
				dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
				rect.InflateRect(-s_sizeBoxMargin.cx, -s_sizeBoxMargin.cy);
			}

			// Draw the selection rectagle
			dc.SetBkColor(m_clrHiLightBorder);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			rect.InflateRect(-1, -1);

			// Draw the inner coloring
			dc.SetBkColor(clrHiLight);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			rect.InflateRect(-(s_sizeBoxHiBorder.cx - 1), -(s_sizeBoxHiBorder.cy - 1));
		}
		else
		{
			// Draw the background
			dc.SetBkColor(m_clrBackground);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			rect.InflateRect(-(s_sizeBoxMargin.cx + s_sizeBoxHiBorder.cx), -(s_sizeBoxMargin.cy + s_sizeBoxHiBorder.cy));
		}

		//
		// Draw color (ok, this code is bit sleeeeeezy.  But the
		// area's that are being drawn are SO small, that nobody
		// will notice.)
		//

		dc.SetBkColor(::GetSysColor(COLOR_3DSHADOW));
		dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
		rect.InflateRect(-1, -1);
		if (clrBox.fA >= 1)
		{
			dc.SetBkColor(clrBox.ToCOLORREF());
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
		}
		else
		{
			COLORREF clrHiLight = GetSysColor(COLOR_3DLIGHT);
			COLORREF clrLoLight = GetSysColor(COLOR_3DSHADOW);
			float fA = clrBox.fA <= 0.0f ? 0.0f : clrBox.fA;
			float fIA = 1.0f-fA;
			float fR = clrBox.fR <= 0.0f ? 0.0f : (clrBox.fR >= 1.0f ? 255.0f : 255.0f*clrBox.fR);
			float fG = clrBox.fG <= 0.0f ? 0.0f : (clrBox.fG >= 1.0f ? 255.0f : 255.0f*clrBox.fG);
			float fB = clrBox.fB <= 0.0f ? 0.0f : (clrBox.fB >= 1.0f ? 255.0f : 255.0f*clrBox.fB);
			COLORREF clr1 = RGB(int(0.5f + GetRValue(clrHiLight)*fIA + fR*fA), int(0.5f + GetGValue(clrHiLight)*fIA + fG*fA), int(0.5f + GetBValue(clrHiLight)*fIA + fB*fA));
			COLORREF clr2 = RGB(int(0.5f + GetRValue(clrLoLight)*fIA + fR*fA), int(0.5f + GetGValue(clrLoLight)*fIA + fG*fA), int(0.5f + GetBValue(clrLoLight)*fIA + fB*fA));
			RECT rc1 = rect;
			RECT rc2 = rect;
			RECT rc3 = rect;
			RECT rc4 = rect;
			rc1.bottom = rc2.bottom = rc3.top = rc4.top = (rect.bottom+rect.top)>>1;
			rc1.right = rc2.left = rc3.right = rc4.left = (rect.right+rect.left)>>1;
			dc.SetBkColor(clr1);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rc1, NULL, 0, NULL);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rc4, NULL, 0, NULL);
			dc.SetBkColor(clr2);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rc2, NULL, 0, NULL);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rc3, NULL, 0, NULL);
		}
	}

	// @cmember Do a hit test

	bool PickerHitTest(const POINT &pt, int* a_pEntry, size_t* a_pColor)
	{
		for (CEntries::const_iterator j = m_cEntries.begin(); j != m_cEntries.end(); ++j)
		{
			if (pt.x >= j->second.rc.left && pt.x < j->second.rc.right &&
				pt.y >= j->second.rc.top && pt.y < j->second.rc.bottom)
			{
				if (!j->second.strText.empty())
				{
					*a_pEntry = j->first;
					*a_pColor = 0;
					return true;
				}
				for (size_t i = 0; i < j->second.aColors.size(); ++i)
				{
					int nX = m_sizeBox.cx*(i%m_nColumns);
					int nY = m_sizeBox.cy*(i/m_nColumns);
					if (pt.x >= j->second.rc.left+nX && pt.x < j->second.rc.left+nX+m_sizeBox.cx &&
						pt.y >= j->second.rc.top+nY && pt.y < j->second.rc.top+nY+m_sizeBox.cy)
					{
						*a_pEntry = j->first;
						*a_pColor = i;
						return true;
					}
				}
				return false;
			}
		}

		return false;
	}


private:
	struct SEntry
	{
		RECT rc;
		std::wstring strText;
		std::vector<SColor> aColors;
	};
	typedef std::map<int, SEntry> CEntries;

private:
	static float hls_value(float n1, float n2, float h)
	{
		h += 360.0f;
		float hue = h - 360.0f*(int)(h/360.0f);

		if (hue < 60.0f)
			return n1 + ( n2 - n1 ) * hue / 60.0f;
		else if (hue < 180.0f)
			return n2;
		else if (hue < 240.0f)
			return n1 + ( n2 - n1 ) * ( 240.0f - hue ) / 60.0f;
		else
			return n1;
	}

	static void HLS2RGB(float h, float l, float s, float& r, float& g, float& b)
	{ // h from <0, 360)
		float m1, m2;
		m2 = l + (l <= 0.5f ? l*s : s - l*s);
		m1 = 2.0f * l - m2;
		if (s == 0.0f)
			r = g = b = l;
		else
		{
			r = hls_value(m1, m2, h+120.0f);
			g = hls_value(m1, m2, h);
			b = hls_value(m1, m2, h-120.0f);
		}
		if (r > 1.0f) r = 1.0f;
		if (g > 1.0f) g = 1.0f;
		if (b > 1.0f) b = 1.0f;
		if (r < 0.0f) r = 0.0f;
		if (g < 0.0f) g = 0.0f;
		if (b < 0.0f) b = 0.0f;
	}

	static void RGB2HLS(float r, float g, float b, float& h, float& l, float& s)
	{
		float bc, gc, rc, rgbmax, rgbmin;

		// Compute lightness.
		rgbmax = r>g ? (r>b ? r : b) : (g>b ? g : b);
		rgbmin = r<g ? (r<b ? r : b) : (g<b ? g : b);
		l = (rgbmax + rgbmin) * 0.5f;

		// Compute saturation.
		if (rgbmax == rgbmin)
			s = 0.0f;
		else if (l <= 0.5f)
			s = (rgbmax - rgbmin) / (rgbmax + rgbmin);
		else
			s = (rgbmax - rgbmin) / (2.0f - rgbmax - rgbmin);

		// Compute the hue.
		if (rgbmax == rgbmin)
			h = 0.0f;
		else
		{
			rc = (rgbmax - r) / (rgbmax - rgbmin);
			gc = (rgbmax - g) / (rgbmax - rgbmin);
			bc = (rgbmax - b) / (rgbmax - rgbmin);

			if (r == rgbmax)
				h = bc - gc;
			else if (g == rgbmax)
				h = 2.0f + rc - bc;
			else
				h = 4.0f + gc - rc;

			h *= 60.0f;
			h = h - 360.0f*(int)(h/360.0f);
		}
	}

private:
	CEntries m_cEntries;

    CFont					m_font;
    int						m_nSelEntry;
    size_t					m_nSelColor;
    int						m_nHotEntry;
    size_t					m_nHotColor;
	int						m_nHotColumn;
	BOOL m_fOked;
	bool m_bMouseDown;

	SColor					m_clrPicker;
	CRect					m_rectMargins;
    CSize					m_sizeText;
	BOOL					m_fPickerFlat;
	CSize					m_sizeBox;
	COLORREF				m_clrBackground;
	COLORREF				m_clrHiLightBorder;
	COLORREF				m_clrHiLight;
	COLORREF				m_clrLoLight;
	COLORREF				m_clrHiLightText;
	COLORREF				m_clrText;

	// constants
	CSize const s_sizeTextHiBorder;
	CSize const s_sizeTextMargin;
	CSize const s_sizeBoxHiBorder;
	CSize const s_sizeBoxMargin;
	CSize const s_sizeBoxCore;
	int const m_nGap;
	int const m_nColumns;
};

}; // namespace WTL
