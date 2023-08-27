
#pragma once

#include <math.h>

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_conv_dash.h>
#include <agg_path_storage.h>
#include <agg_ellipse.h>
#include <agg_span_allocator.h>
#include <GammaCorrection.h>

namespace WTL
{

class CColorWheel :
	public CWindowImpl<CColorWheel>,
	public CThemeImpl<CColorWheel>
{
public:
	CColorWheel() : m_fH(0.0f), m_fV(0.0f), m_fS(0.0f), m_fA(1.0f), m_bHue(false), m_hCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND))),
		m_nHalfSize(4), m_pHues(NULL), m_pGT(NULL), m_fRoundOutlineWidth(1.0f), m_fTriangleOutlineWidth(1.0f/0.5235987756f)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
		m_pHues = new BYTE[2048];
		int i = 0;
		for (; i < 2048/6; ++i)
			m_pHues[i] = m_pHues[2047-i] = 255;
		for (; i < 2048*2/3; ++i)
			m_pHues[i] = m_pHues[2047-i] = CGammaTables::ToSRGB(float(2048/3-i)/float(2048/6));
		for (; i < 2048/2; ++i)
			m_pHues[i] = m_pHues[2047-i] = 0;
	}
	void InitGamma(CGammaTables const* a_pGT)
	{
		m_pGT = a_pGT;
	}
	~CColorWheel()
	{
		delete[] m_pHues;
	}

	enum {
		CWN_CHANGED = 1,
		CWN_FINISHED = 2
	};


	DECLARE_WND_CLASS_EX(_T("ColorWheelClass"), CS_HREDRAW | CS_VREDRAW | CS_OWNDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CColorWheel)
		CHAIN_MSG_MAP(CThemeImpl<CColorWheel>)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocusChange)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnFocusChange)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	END_MSG_MAP()

	// operations
public:
	void SetColor(TColor const a_tColor)
	{
		float max = a_tColor.fR > a_tColor.fG ? (a_tColor.fR > a_tColor.fB ? a_tColor.fR : a_tColor.fB) : (a_tColor.fG > a_tColor.fB ? a_tColor.fG : a_tColor.fB);
		float min = a_tColor.fR < a_tColor.fG ? (a_tColor.fR < a_tColor.fB ? a_tColor.fR : a_tColor.fB) : (a_tColor.fG < a_tColor.fB ? a_tColor.fG : a_tColor.fB);
		m_fV = max;
		if (min != max)
		{
			float const delta = max - min;
			m_fS = delta / max;
			float rc = (max - a_tColor.fR) / delta;
			float gc = (max - a_tColor.fG) / delta;
			float bc = (max - a_tColor.fB) / delta;

			if (a_tColor.fR == max)
				m_fH = bc - gc;
			else if (a_tColor.fG == max)
				m_fH = 2.0f + rc - bc;
			else
				m_fH = 4.0f + gc - rc;

			m_fH *= 60.0f;
			if (m_fH < 0.0f) m_fH += 360.0f;
		}
		else
		{
			m_fS = 0.0f;
			// keep previous hue
		}

		m_fA = a_tColor.fA;

		Invalidate(FALSE);
	}

	TColor const& GetColor() const
	{
		TColor tColor = {0.0f, 0.0f, 0.0f, m_fA};
		if (m_fV != 0.0)
		{
			int i = floorf(m_fH/60.0f);
			float f = (m_fH/60.0f) - i;
			float p = m_fV * (1 - m_fS);
			float q = m_fV * (1 - (m_fS * f));
			float t = m_fV * (1 - (m_fS * (1 - f)));
			switch (i)
			{
			case 1: tColor.fR = q; tColor.fG = m_fV; tColor.fB = p; break;
			case 2: tColor.fR = p; tColor.fG = m_fV; tColor.fB = t; break;
			case 3: tColor.fR = p; tColor.fG = q; tColor.fB = m_fV; break;
			case 4: tColor.fR = t; tColor.fG = p; tColor.fB = m_fV; break;
			case 5: tColor.fR = m_fV; tColor.fG = p; tColor.fB = q; break;
			case 6: // fall through
			case 0: tColor.fR = m_fV; tColor.fG = t; tColor.fB = p; break;
			}
		}
		return tColor;
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hdc = GetDC();
		m_fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		m_nHalfSize = int(4*m_fScale+0.5f);
		ReleaseDC(hdc);
		return 0;
	}

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnFocusChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT rc = {0, 0, 0, 0};
		GetClientRect(&rc);
		float aTri[6];
		GetTriCoords(rc.right, rc.bottom, aTri);
		InvalidateThumb(aTri);
		SendNotification(a_uMsg == WM_KILLFOCUS ? NM_KILLFOCUS : NM_SETFOCUS);
		return 0;
	}
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_hCursor == NULL)
		{
			a_bHandled = FALSE;
			return 0;
		}

		if (reinterpret_cast<HWND>(a_wParam) == m_hWnd)
		{
			RECT rc = {0, 0, 0, 0};
			GetClientRect(&rc);
			int nSize = rc.right < rc.bottom ? rc.right : rc.bottom;

			POINT tPt = {0, 0};
			GetCursorPos(&tPt);
			ScreenToClient(&tPt);

			float fDist = sqrtf((tPt.x+0.5f-rc.right*0.5)*(tPt.x+0.5f-rc.right*0.5) + (tPt.y+0.5f-rc.bottom*0.5)*(tPt.y+0.5f-rc.bottom*0.5));
			if (fDist < 0.5f*nSize && fDist > 0.37f*nSize)
			{
				SetCursor(m_hCursor);
				return TRUE;
			}
		}
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
			SendNotification(CWN_FINISHED);
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT rc = {0, 0, 0, 0};
		GetClientRect(&rc);
		int nSize = rc.right < rc.bottom ? rc.right : rc.bottom;
		float fCX = 0.5f*rc.right;
		float fCY = 0.5f*rc.bottom;
		int nX = GET_X_LPARAM(a_lParam);
		int nY = GET_Y_LPARAM(a_lParam);

		float aTri[6];
		GetTriCoords(rc.right, rc.bottom, aTri);
		float fX = 0.0f;
		float fY = 0.0f;
		GetThumbPos(aTri, m_fS, m_fV, &fX, &fY);
		POINT tTh = {fX, fY};
		if (nX >= tTh.x-m_nHalfSize-1 && nX <= tTh.x+m_nHalfSize+1 &&
			nY >= tTh.y-m_nHalfSize-1 && nY <= tTh.y+m_nHalfSize+1)
		{
			// thumb hit
			SetCapture();
			m_bHue = false;
			m_fDX = nX-fX;
			m_fDY = nY-fY;
			return 0;
		}

		float fDist = sqrtf((nX+0.5f-fCX)*(nX+0.5f-fCX) + (nY+0.5f-fCY)*(nY+0.5f-fCY));
		if (fDist < 0.5f*nSize && fDist > 0.37f*nSize)
		{
			SetCapture();
			m_bHue = true;
			m_fH = atan2f(nX+0.5f-fCX, fCY-nY-0.5f)*180.0f/3.14159265358979;
			if (m_fH < 0.0f) m_fH += 360.0f;
			InvalidateThumb(aTri);
			if (m_fV <= 0.0f || m_fS <= 0.0f) { m_fV = m_fS = 1.0f; }
			GetTriCoords(rc.right, rc.bottom, aTri);
			InvalidateThumb(aTri);
			InvalidateTriangle(rc.right, rc.bottom);
			UpdateWindow();
			SendNotification();
			return 0;
		}
		if (PointInTriangle(nX, nY, aTri) || PointInTriangle(nX+1, nY+1, aTri) ||
			PointInTriangle(nX+1, nY, aTri) || PointInTriangle(nX, nY+1, aTri))
		{
			SetCapture();
			m_bHue = false;
			m_fDX = 0.0f;
			m_fDY = 0.0f;
			InvalidateThumb(aTri);
			PointToSV(nX+0.5f, nY+0.5f, aTri, &m_fS, &m_fV);
			InvalidateThumb(aTri);
			UpdateWindow();
			SendNotification();
			return 0;
		}
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() != m_hWnd)
			return 0;

		RECT rc = {0, 0, 0, 0};
		GetClientRect(&rc);
		int nSize = rc.right < rc.bottom ? rc.right : rc.bottom;
		float fCX = 0.5f*rc.right;
		float fCY = 0.5f*rc.bottom;
		int nX = GET_X_LPARAM(a_lParam);
		int nY = GET_Y_LPARAM(a_lParam);
		if (m_bHue)
		{
			float aTri[6];
			GetTriCoords(rc.right, rc.bottom, aTri);
			InvalidateThumb(aTri);
			m_fH = atan2f(nX+0.5f-fCX, fCY-nY-0.5f)*180.0f/3.14159265358979;
			if (m_fH < 0.0f) m_fH += 360.0f;
			float fDist = sqrtf((nX+0.5f-fCX)*(nX+0.5f-fCX) + (nY+0.5f-fCY)*(nY+0.5f-fCY));
			if (fDist < 0.34f*nSize) // discretize when inside
			{
				m_fH = int((m_fH+10)/20)*20;
				if (m_fH >= 360.0f) m_fH -= 360.0f;
			}
			GetTriCoords(rc.right, rc.bottom, aTri);
			InvalidateThumb(aTri);
			InvalidateTriangle(rc.right, rc.bottom);
			UpdateWindow();
			SendNotification();
		}
		else
		{
			float aTri[6];
			GetTriCoords(rc.right, rc.bottom, aTri);
			InvalidateThumb(aTri);
			PointToSV(nX-m_fDX+0.5f, nY-m_fDY+0.5f, aTri, &m_fS, &m_fV);
			InvalidateThumb(aTri);
			UpdateWindow();
			SendNotification();
		}

		return 0;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT rc = {0, 0, 0, 0};
		GetClientRect(&rc);
		float aTri[6];
		GetTriCoords(rc.right, rc.bottom, aTri);

		switch (a_wParam)
		{
		case VK_LEFT:
			if (m_fV <= 0.0f)
				return 0;
			InvalidateThumb(aTri);
			m_fV -= 0.01f;
			if (m_fV < 0.0f) m_fV = 0.0f;
			InvalidateThumb(aTri);
			break;
		case VK_RIGHT:
			if (m_fV >= 1.0f)
				return 0;
			InvalidateThumb(aTri);
			m_fV += 0.01f;
			if (m_fV > 1.0f) m_fV = 1.0f;
			InvalidateThumb(aTri);
			break;
		case VK_UP:
			if (m_fS >= 1.0f)
				return 0;
			InvalidateThumb(aTri);
			m_fS += 0.01f;
			if (m_fS > 1.0f) m_fS = 1.0f;
			InvalidateThumb(aTri);
			break;
		case VK_DOWN:
			if (m_fS <= 0.0f)
				return 0;
			InvalidateThumb(aTri);
			m_fS -= 0.01f;
			if (m_fS < 0.0f) m_fS = 0.0f;
			InvalidateThumb(aTri);
			break;
		case VK_NEXT:
		case VK_HOME:
			InvalidateThumb(aTri);
			m_fH -= 3.6f;
			if (m_fH < 0.0f)
				m_fH += 360.0f;
			GetTriCoords(rc.right, rc.bottom, aTri);
			InvalidateThumb(aTri);
			InvalidateTriangle(rc.right, rc.bottom);
			break;
		case VK_PRIOR:
		case VK_END:
			InvalidateThumb(aTri);
			m_fH += 3.6f;
			if (m_fH >= 360.0f)
				m_fH -= 360.0f;
			GetTriCoords(rc.right, rc.bottom, aTri);
			InvalidateThumb(aTri);
			InvalidateTriangle(rc.right, rc.bottom);
			break;
		}
		UpdateWindow();
		SendNotification();
		return 0;
	}

	LRESULT OnMouseWheel(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		int nDelta = GET_WHEEL_DELTA_WPARAM(a_wParam);

		RECT rc = {0, 0, 0, 0};
		GetClientRect(&rc);
		int nSize = rc.right < rc.bottom ? rc.right : rc.bottom;
		float fCX = 0.5f*rc.right;
		float fCY = 0.5f*rc.bottom;
		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		ScreenToClient(&tPt);
		int nX = tPt.x;
		int nY = tPt.y;

		float aTri[6];
		GetTriCoords(rc.right, rc.bottom, aTri);
		float fDist = sqrtf((nX+0.5f-fCX)*(nX+0.5f-fCX) + (nY+0.5f-fCY)*(nY+0.5f-fCY));
		if (fDist < 0.5f*nSize && fDist > 0.37f*nSize)
		{
			m_fH -= nDelta*0.072f/WHEEL_DELTA*180.0f/3.14159265358979;
			if (m_fH < 0.0f) m_fH += 360.0f;
			else if (m_fH > 360.0f) m_fH -= 360.0f;
			GetTriCoords(rc.right, rc.bottom, aTri);
			InvalidateTriangle(rc.right, rc.bottom);
			UpdateWindow();
			SendNotification();
			return 0;
		}
		if (PointInTriangle(nX, nY, aTri) || PointInTriangle(nX+1, nY+1, aTri) ||
			PointInTriangle(nX+1, nY, aTri) || PointInTriangle(nX, nY+1, aTri))
		{
			InvalidateThumb(aTri);
			m_fV += nDelta*0.02f/WHEEL_DELTA;
			if (m_fV < 0.0f) m_fV = 0.0f;
			else if (m_fV > 1.0f) m_fV = 1.0f;
			InvalidateThumb(aTri);
			UpdateWindow();
			SendNotification();
			return 0;
		}

		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnGetDlgCode(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return DLGC_WANTARROWS;
	}

	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRes = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
		if (lRes == MA_NOACTIVATE || lRes == MA_NOACTIVATEANDEAT)
			return lRes;
		SetFocus();
		return MA_ACTIVATE;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		// get window style
		//DWORD dwStyle = GetWindowLong(GWL_STYLE);

		RECT rcWin;
		GetClientRect(&rcWin);

		PAINTSTRUCT ps;
		CDCHandle cDC(BeginPaint(&ps));

		BITMAPINFO tBMI;
		ZeroMemory(&tBMI, sizeof tBMI);
		tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
		tBMI.bmiHeader.biWidth = ps.rcPaint.right-ps.rcPaint.left;
		tBMI.bmiHeader.biHeight = ps.rcPaint.bottom-ps.rcPaint.top;
		tBMI.bmiHeader.biPlanes = 1;
		tBMI.bmiHeader.biBitCount = 32;
		tBMI.bmiHeader.biCompression = BI_RGB;

		BYTE* pData = NULL;
		HBITMAP hBmp = CreateDIBSection(cDC, &tBMI, DIB_RGB_COLORS, reinterpret_cast<void**>(&pData), NULL, 0);
		CDC cDCPaint;
		cDCPaint.CreateCompatibleDC(cDC);
		hBmp = cDCPaint.SelectBitmap(hBmp);

		RECT const rcPaint = {0, 0, ps.rcPaint.right-ps.rcPaint.left, ps.rcPaint.bottom-ps.rcPaint.top};
		if (IsThemingSupported())
		{
			DrawThemeParentBackground(cDCPaint, &rcPaint);
		}
		else
		{
			HBRUSH hBrush = (HBRUSH)GetParent().SendMessage(WM_CTLCOLORDLG, (WPARAM)cDCPaint.m_hDC, 0);
			if (hBrush)
				cDCPaint.FillRect(&rcPaint, hBrush);
			else
				cDCPaint.FillSolidRect(&rcPaint, GetSysColor(COLOR_WINDOW));
		}
		GdiFlush();

		int width = rcWin.right - rcWin.left;
		int height = rcWin.bottom - rcWin.top;
		int size = min(width, height);

		rendering_buffer_sRGB rbuf(pData, ps.rcPaint.right-ps.rcPaint.left, ps.rcPaint.bottom-ps.rcPaint.top, (ps.rcPaint.left-ps.rcPaint.right)*4, m_pGT);
		agg::renderer_base<rendering_buffer_sRGB> renb(rbuf);
		agg::renderer_scanline_aa_solid<agg::renderer_base<rendering_buffer_sRGB> > ren(renb);
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		ras.filling_rule(agg::fill_even_odd);

		// outline - outer
		agg::ellipse eo1(width*0.5f-ps.rcPaint.left, height*0.5f-ps.rcPaint.top, size*0.5f, size*0.5f);
		agg::ellipse eo2(width*0.5f-ps.rcPaint.left, height*0.5f-ps.rcPaint.top, size*0.5f-m_fRoundOutlineWidth*3.0f, size*0.5f-m_fRoundOutlineWidth*3.0f);
		agg::ellipse ei1(width*0.5f-ps.rcPaint.left, height*0.5f-ps.rcPaint.top, size*0.37f+m_fRoundOutlineWidth*3.0f, size*0.37f+m_fRoundOutlineWidth*3.0f);
		agg::ellipse ei2(width*0.5f-ps.rcPaint.left, height*0.5f-ps.rcPaint.top, size*0.37f, size*0.37f);
		ren.color(agg::rgba8(0x82, 0x87, 0x90, 255));
		ras.add_path(eo1);
		ras.add_path(eo2);
		ras.add_path(ei1);
		ras.add_path(ei2);
		ras.move_to_d(width*0.5f+size*0.37f*cosf((m_fH-90.0)*3.14159265358979/180.0f)-ps.rcPaint.left, height*0.5f+size*0.37f*sinf((m_fH-90.0f)*3.14159265358979/180.0f)-ps.rcPaint.top);
		ras.line_to_d(width*0.5f+size*0.37f*cosf((m_fH+30.0f)*3.14159265358979/180.0f)-ps.rcPaint.left, height*0.5f+size*0.37f*sinf((m_fH+30.0f)*3.14159265358979/180.0f)-ps.rcPaint.top);
		ras.line_to_d(width*0.5f+size*0.37f*cosf((m_fH-210.0f)*3.14159265358979/180.0f)-ps.rcPaint.left, height*0.5f+size*0.37f*sinf((m_fH-210.0f)*3.14159265358979/180.0f)-ps.rcPaint.top);
		ras.close_polygon();
		agg::render_scanlines(ras, sl, ren);

		// outline - inner
		agg::ellipse eo3(width*0.5f-ps.rcPaint.left, height*0.5f-ps.rcPaint.top, size*0.5f-m_fRoundOutlineWidth, size*0.5f-m_fRoundOutlineWidth);
		agg::ellipse ei3(width*0.5f-ps.rcPaint.left, height*0.5f-ps.rcPaint.top, size*0.37f+m_fRoundOutlineWidth, size*0.37f+m_fRoundOutlineWidth);
		ren.color(agg::rgba8(255, 255, 255, 255));
		ras.add_path(eo3);
		ras.add_path(eo2);
		ras.add_path(ei1);
		ras.add_path(ei3);
		ras.move_to_d(width*0.5f+(size*0.37f-m_fTriangleOutlineWidth)*cosf((m_fH-90.0)*3.14159265358979/180.0f)-ps.rcPaint.left, height*0.5f+(size*0.37f-m_fTriangleOutlineWidth)*sinf((m_fH-90.0f)*3.14159265358979/180.0f)-ps.rcPaint.top);
		ras.line_to_d(width*0.5f+(size*0.37f-m_fTriangleOutlineWidth)*cosf((m_fH+30.0f)*3.14159265358979/180.0f)-ps.rcPaint.left, height*0.5f+(size*0.37f-m_fTriangleOutlineWidth)*sinf((m_fH+30.0f)*3.14159265358979/180.0f)-ps.rcPaint.top);
		ras.line_to_d(width*0.5f+(size*0.37f-m_fTriangleOutlineWidth)*cosf((m_fH-210.0f)*3.14159265358979/180.0f)-ps.rcPaint.left, height*0.5f+(size*0.37f-m_fTriangleOutlineWidth)*sinf((m_fH-210.0f)*3.14159265358979/180.0f)-ps.rcPaint.top);
		ras.close_polygon();
		agg::render_scanlines(ras, sl, ren);

		// color wheel
		agg::ellipse eco(width*0.5f-ps.rcPaint.left, height*0.5f-ps.rcPaint.top, size*0.5f-m_fRoundOutlineWidth*2.0f, size*0.5f-m_fRoundOutlineWidth*2.0f);
		agg::ellipse eci(width*0.5f-ps.rcPaint.left, height*0.5f-ps.rcPaint.top, size*0.37f+m_fRoundOutlineWidth*2.0f, size*0.37f+m_fRoundOutlineWidth*2.0f);
		ras.add_path(eco);
		ras.add_path(eci);

		agg::span_allocator<agg::rgba8> span_alloc;
		CHueWheel filler(width*0.5f-ps.rcPaint.left, height*0.5f-ps.rcPaint.top, m_pHues);
		agg::render_scanlines_aa(ras, sl, renb, span_alloc, filler);

		float aTri[6];
		GetTriCoords(width, height, aTri);
		POINT tThumb = GetThumbPos(aTri, m_fS, m_fV);
		aTri[0] -= ps.rcPaint.left; aTri[1] -= ps.rcPaint.top;
		aTri[2] -= ps.rcPaint.left; aTri[3] -= ps.rcPaint.top;
		aTri[4] -= ps.rcPaint.left; aTri[5] -= ps.rcPaint.top;
		CLumSatTriangle filler2(aTri[0], aTri[1], aTri[2], aTri[3], aTri[4], aTri[5], m_fH);
		ras.move_to_d(aTri[0], aTri[1]);
		ras.line_to_d(aTri[2], aTri[3]);
		ras.line_to_d(aTri[4], aTri[5]);
		ras.close_polygon();
		agg::render_scanlines_aa(ras, sl, renb, span_alloc, filler2);

		// draw thumb
		tThumb.x -= ps.rcPaint.left;
		tThumb.y -= ps.rcPaint.top;
		TColor tClr = GetColor();
		ren.color(agg::rgba8(CGammaTables::ToSRGB(tClr.fR), CGammaTables::ToSRGB(tClr.fG), CGammaTables::ToSRGB(tClr.fB), 255));
		ras.move_to_d(tThumb.x-m_nHalfSize, tThumb.y-m_nHalfSize);
		ras.line_to_d(tThumb.x+m_nHalfSize+1, tThumb.y-m_nHalfSize);
		ras.line_to_d(tThumb.x+m_nHalfSize+1, tThumb.y+m_nHalfSize+1);
		ras.line_to_d(tThumb.x-m_nHalfSize, tThumb.y+m_nHalfSize+1);
		ras.close_polygon();
		agg::render_scanlines(ras, sl, ren);
		ren.color(agg::rgba8(255, 255, 255, 255));
		ras.move_to_d(tThumb.x-m_nHalfSize-1, tThumb.y-m_nHalfSize-1);
		ras.line_to_d(tThumb.x+m_nHalfSize+2, tThumb.y-m_nHalfSize-1);
		ras.line_to_d(tThumb.x+m_nHalfSize+2, tThumb.y+m_nHalfSize+2);
		ras.line_to_d(tThumb.x-m_nHalfSize-1, tThumb.y+m_nHalfSize+2);
		ras.close_polygon();
		ras.move_to_d(tThumb.x-m_nHalfSize, tThumb.y-m_nHalfSize);
		ras.line_to_d(tThumb.x+m_nHalfSize+1, tThumb.y-m_nHalfSize);
		ras.line_to_d(tThumb.x+m_nHalfSize+1, tThumb.y+m_nHalfSize+1);
		ras.line_to_d(tThumb.x-m_nHalfSize, tThumb.y+m_nHalfSize+1);
		ras.close_polygon();
		agg::render_scanlines(ras, sl, ren);
		if (m_hWnd == GetFocus())
			ren.color(agg::rgba8(0, 0, 0, 255));
		else
			ren.color(agg::rgba8(0x82, 0x87, 0x90, 255));
		ras.move_to_d(tThumb.x-m_nHalfSize-2, tThumb.y-m_nHalfSize-2);
		ras.line_to_d(tThumb.x+m_nHalfSize+3, tThumb.y-m_nHalfSize-2);
		ras.line_to_d(tThumb.x+m_nHalfSize+3, tThumb.y+m_nHalfSize+3);
		ras.line_to_d(tThumb.x-m_nHalfSize-2, tThumb.y+m_nHalfSize+3);
		ras.close_polygon();
		ras.move_to_d(tThumb.x-m_nHalfSize-1, tThumb.y-m_nHalfSize-1);
		ras.line_to_d(tThumb.x+m_nHalfSize+2, tThumb.y-m_nHalfSize-1);
		ras.line_to_d(tThumb.x+m_nHalfSize+2, tThumb.y+m_nHalfSize+2);
		ras.line_to_d(tThumb.x-m_nHalfSize-1, tThumb.y+m_nHalfSize+2);
		ras.close_polygon();
		agg::render_scanlines(ras, sl, ren);

		cDC.BitBlt(ps.rcPaint.left, ps.rcPaint.top, tBMI.bmiHeader.biWidth, tBMI.bmiHeader.biHeight, cDCPaint, 0, 0, SRCCOPY);
		DeleteObject(cDCPaint.SelectBitmap(hBmp));

		EndPaint(&ps);

		return 0;
	}
	LRESULT OnEraseBkgnd(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

private:
	struct CHueWheel
	{
		CHueWheel(float a_fCenterX, float a_fCenterY, BYTE* a_pHues) : fCenterX(a_fCenterX), fCenterY(a_fCenterY), pHues(a_pHues) {}

		bool is_solid(unsigned a_nLayer)
		{
			return false;
		}
		void generate_span(agg::rgba8* span, int x, int y, unsigned len, unsigned style)
		{
			generate(span, x, y, len);
		}
		void generate(agg::rgba8* span, int x, int y, unsigned len)
		{
			float const fY = y-fCenterY+0.5f;
			while (len--)
			{
				float const fA = atan2f(fY, x-fCenterX+0.5f);
				int const nA = fA*(1024/3.14159265358979)+512;
				span->r = pHues[nA&2047];
				span->g = pHues[(nA-2048/3)&2047];
				span->b = pHues[(nA+2048/3)&2047];
				span->a = 0xff;
				++span;
				++x;
			}
		}
		void prepare()
		{
		}

		BYTE const* const pHues;
		float const fCenterX;
		float const fCenterY;
	};
	struct CLumSatTriangle
	{
		CLumSatTriangle(float a_fX1, float a_fY1, float a_fX2, float a_fY2, float a_fX3, float a_fY3, float a_fHue)
		{
			float r = AngleToVal(a_fHue);
			float g = AngleToVal(a_fHue-120.0f);
			float b = AngleToVal(a_fHue+120.0f);
			float const fDI = 1.0f/Determinant(a_fX1, a_fY1, 1.0f, a_fX2, a_fY2, 1.0f, a_fX3, a_fY3, 1.0f);
			fRX = fDI*Determinant(r, a_fY1, 1.0f, 1.0f, a_fY2, 1.0f, 0.0f, a_fY3, 1.0f);
			fRY = fDI*Determinant(a_fX1, r, 1.0f, a_fX2, 1.0f, 1.0f, a_fX3, 0.0f, 1.0f);
			fR = fDI*Determinant(a_fX1, a_fY1, r, a_fX2, a_fY2, 1.0f, a_fX3, a_fY3, 0.0f);
			fGX = fDI*Determinant(g, a_fY1, 1.0f, 1.0f, a_fY2, 1.0f, 0.0f, a_fY3, 1.0f);
			fGY = fDI*Determinant(a_fX1, g, 1.0f, a_fX2, 1.0f, 1.0f, a_fX3, 0.0f, 1.0f);
			fG = fDI*Determinant(a_fX1, a_fY1, g, a_fX2, a_fY2, 1.0f, a_fX3, a_fY3, 0.0f);
			fBX = fDI*Determinant(b, a_fY1, 1.0f, 1.0f, a_fY2, 1.0f, 0.0f, a_fY3, 1.0f);
			fBY = fDI*Determinant(a_fX1, b, 1.0f, a_fX2, 1.0f, 1.0f, a_fX3, 0.0f, 1.0f);
			fB = fDI*Determinant(a_fX1, a_fY1, b, a_fX2, a_fY2, 1.0f, a_fX3, a_fY3, 0.0f);
		}

		bool is_solid(unsigned a_nLayer)
		{
			return false;
		}
		void generate_span(agg::rgba8* span, int x, int y, unsigned len, unsigned style)
		{
			generate(span, x, y, len);
		}
		void generate(agg::rgba8* span, int x, int y, unsigned len)
		{
			float r = (x+0.5f)*fRX + (y+0.5f)*fRY + fR;
			float g = (x+0.5f)*fGX + (y+0.5f)*fGY + fG;
			float b = (x+0.5f)*fBX + (y+0.5f)*fBY + fB;
			while (len--)
			{
				span->r = CGammaTables::ToSRGB(r);
				span->g = CGammaTables::ToSRGB(g);
				span->b = CGammaTables::ToSRGB(b);
				span->a = 0xff;
				++span;
				++x;
				r += fRX;
				g += fGX;
				b += fBX;
			}
		}
		void prepare()
		{
		}

	private:
		inline float Determinant(float f11, float f12, float f13, float f21, float f22, float f23, float f31, float f32, float f33)
		{
			return f11*f22*f33 + f21*f32*f13 + f31*f12*f23 - f13*f22*f31 - f23*f32*f11 - f33*f12*f21;
		}
		float AngleToVal(float fA)
		{
			while (fA < 0.0f) fA += 360.f;
			while (fA > 360.0f) fA -= 360.f;
			if (fA < 60.0f || fA > 300.0f)
				return 1.0f;
			if (fA < 120.0f)
				return (120-fA)/60.0f;
			if (fA < 240.0f)
				return 0.0f;
			return (fA-240.0f)/60.0f;
		}

		float fRX;
		float fRY;
		float fR;
		float fGX;
		float fGY;
		float fG;
		float fBX;
		float fBY;
		float fB;
	};

    struct rendering_buffer_sRGB
	{
        typedef agg::rgba8 color_type;
        typedef agg::int8u row_data;

		rendering_buffer_sRGB(BYTE* a_pData, unsigned a_nSizeX, unsigned a_nSizeY, int a_nStride, CGammaTables const* a_pGT) :
			m_pData(a_nStride < 0 ? a_pData+(-a_nStride)*(a_nSizeY-1) : a_pData),
			m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY), m_nStride(a_nStride), m_pGT(a_pGT)
		{
		}

		unsigned width() const { return m_nSizeX; }
		unsigned height() const { return m_nSizeY; }

		void blend_hline(int x, int y, unsigned len, agg::rgba8 const& c, agg::int8u cover)
		{
			if (cover == 255)
			{
				DWORD const dw = 0xff000000|(DWORD(c.r)<<16)|(DWORD(c.g)<<8)|DWORD(c.b);
				DWORD* p = reinterpret_cast<DWORD*>(m_pData+y*m_nStride+x*4);
				for (DWORD* const pEnd = p+len; p != pEnd; ++p)
					*p = dw;
			}
			else
			{
				ULONG const nA = cover*0x10000/255;
				ULONG const nIA = 0x10000-nA;
				ULONG const r = m_pGT->m_aGamma[c.r]*nA;
				ULONG const g = m_pGT->m_aGamma[c.g]*nA;
				ULONG const b = m_pGT->m_aGamma[c.b]*nA;
				BYTE* p = m_pData+y*m_nStride+x*4;
				for (BYTE* const pEnd = p+(len<<2); p != pEnd; p+=4)
				{
					p[0] = m_pGT->InvGamma((m_pGT->m_aGamma[p[0]]*nIA + b)>>16);
					p[1] = m_pGT->InvGamma((m_pGT->m_aGamma[p[1]]*nIA + g)>>16);
					p[2] = m_pGT->InvGamma((m_pGT->m_aGamma[p[2]]*nIA + r)>>16);
				}
			}
		}
		void blend_solid_hspan(int x, int y, unsigned len, agg::rgba8 const& c, agg::int8u const* covers)
		{
			ULONG const r = m_pGT->m_aGamma[c.r];
			ULONG const g = m_pGT->m_aGamma[c.g];
			ULONG const b = m_pGT->m_aGamma[c.b];
			BYTE* p = m_pData+y*m_nStride+x*4;
			for (BYTE* const pEnd = p+(len<<2); p != pEnd; p+=4, ++covers)
			{
				if (*covers == 255)
				{
					p[0] = c.b;
					p[1] = c.g;
					p[2] = c.r;
				}
				else
				{
					ULONG const nA = *covers*0x101;
					ULONG const nIA = 0x10000-nA;
					p[0] = m_pGT->InvGamma((m_pGT->m_aGamma[p[0]]*nIA + b*nA)>>16);
					p[1] = m_pGT->InvGamma((m_pGT->m_aGamma[p[1]]*nIA + g*nA)>>16);
					p[2] = m_pGT->InvGamma((m_pGT->m_aGamma[p[2]]*nIA + r*nA)>>16);
				}
			}
		}
		void blend_color_hspan(int x, int y, unsigned len, const agg::rgba8* colors, const agg::int8u* covers, agg::int8u cover)
		{
			BYTE* p = m_pData+y*m_nStride+x*4;
			if (covers)
			{
				for (BYTE* const pEnd = p+(len<<2); p != pEnd; p+=4, ++covers, ++colors)
				{
					ULONG const nA = *covers*0x101;
					ULONG const nIA = 0x10000-nA;
					p[0] = m_pGT->InvGamma((m_pGT->m_aGamma[p[0]]*nIA + m_pGT->m_aGamma[colors->b]*nA)>>16);
					p[1] = m_pGT->InvGamma((m_pGT->m_aGamma[p[1]]*nIA + m_pGT->m_aGamma[colors->g]*nA)>>16);
					p[2] = m_pGT->InvGamma((m_pGT->m_aGamma[p[2]]*nIA + m_pGT->m_aGamma[colors->r]*nA)>>16);
				}
				return;
			}
			if (cover == 255)
			{
				for (BYTE* const pEnd = p+(len<<2); p != pEnd; p+=4, ++colors)
				{
					p[0] = colors->b;
					p[1] = colors->g;
					p[2] = colors->r;
				}
				return;
			}
			ULONG const nA = cover*0x10000/255;
			ULONG const nIA = 0x10000-nA;
			for (BYTE* const pEnd = p+(len<<2); p != pEnd; p+=4, ++colors)
			{
				p[0] = m_pGT->InvGamma((m_pGT->m_aGamma[p[0]]*nIA + m_pGT->m_aGamma[colors->b]*nA)>>16);
				p[1] = m_pGT->InvGamma((m_pGT->m_aGamma[p[1]]*nIA + m_pGT->m_aGamma[colors->g]*nA)>>16);
				p[2] = m_pGT->InvGamma((m_pGT->m_aGamma[p[2]]*nIA + m_pGT->m_aGamma[colors->r]*nA)>>16);
			}
		}

	private:
		BYTE* const m_pData;
		unsigned const m_nSizeX;
		unsigned const m_nSizeY;
		int const m_nStride;
		CGammaTables const* m_pGT;
	};

private:
	void SendNotification(int a_nCode = CWN_CHANGED)
	{
		HWND hPar = GetParent();
		if (hPar)
		{
			NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), a_nCode};
			SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}
	void GetTriCoords(int a_nSizeX, int a_nSizeY, float* a_aTri) const
	{
		int const nSize = a_nSizeX < a_nSizeY ? a_nSizeX : a_nSizeY;
		a_aTri[0] = a_nSizeX*0.5f+(nSize*0.37f-m_fTriangleOutlineWidth*2.0f)*cosf((m_fH-90.0)*3.14159265358979/180.0f);
		a_aTri[1] = a_nSizeY*0.5f+(nSize*0.37f-m_fTriangleOutlineWidth*2.0f)*sinf((m_fH-90.0f)*3.14159265358979/180.0f);
		a_aTri[2] = a_nSizeX*0.5f+(nSize*0.37f-m_fTriangleOutlineWidth*2.0f)*cosf((m_fH+30.0)*3.14159265358979/180.0f);
		a_aTri[3] = a_nSizeY*0.5f+(nSize*0.37f-m_fTriangleOutlineWidth*2.0f)*sinf((m_fH+30.0f)*3.14159265358979/180.0f);
		a_aTri[4] = a_nSizeX*0.5f+(nSize*0.37f-m_fTriangleOutlineWidth*2.0f)*cosf((m_fH-210.0)*3.14159265358979/180.0f);
		a_aTri[5] = a_nSizeY*0.5f+(nSize*0.37f-m_fTriangleOutlineWidth*2.0f)*sinf((m_fH-210.0f)*3.14159265358979/180.0f);
	}
	static float sign(float const* a_p1, float const* a_p2, float const* a_p3)
	{
		return (a_p1[0] - a_p3[0]) * (a_p2[1] - a_p3[1]) - (a_p2[0] - a_p3[0]) * (a_p1[1] - a_p3[1]);
	}
	static bool PointInTriangle(float a_fX, float a_fY, float const* a_aTri)
	{
		float const aPt[2] = {a_fX, a_fY};
		bool const b1 = sign(aPt, a_aTri, a_aTri+2) < 0.0f;
		bool const b2 = sign(aPt, a_aTri+2, a_aTri+4) < 0.0f;
		bool const b3 = sign(aPt, a_aTri+4, a_aTri) < 0.0f;
		return b1 == b2 && b2 == b3;
	}
	static float distance(float const* a_pA, float const* a_pB, float const* a_pP)
	{
		float const fNormal = sqrtf((a_pB[0]-a_pA[0])*(a_pB[0]-a_pA[0]) + (a_pB[1]-a_pA[1])*(a_pB[1]-a_pA[1]));
		return ((a_pP[0] - a_pA[0]) * (a_pB[1] - a_pA[1]) - (a_pP[1] - a_pA[1]) * (a_pB[0] - a_pA[0])) / fNormal;
	}
	static void intersection(float const* a_p1, float const* a_p2, float const* a_p3, float const* a_p4, float* a_pI)
	{
		float const f = 1.0f/((a_p1[0]-a_p2[0])*(a_p3[1]-a_p4[1]) - (a_p1[1]-a_p2[1])*(a_p3[0]-a_p4[0]));
		a_pI[0] = f*((a_p1[0]*a_p2[1]-a_p1[1]*a_p2[0])*(a_p3[0]-a_p4[0]) - (a_p1[0]-a_p2[0])*(a_p3[0]*a_p4[1]-a_p3[1]*a_p4[0]));
		a_pI[1] = f*((a_p1[0]*a_p2[1]-a_p1[1]*a_p2[0])*(a_p3[1]-a_p4[1]) - (a_p1[1]-a_p2[1])*(a_p3[0]*a_p4[1]-a_p3[1]*a_p4[0]));
	}
	static void PointToSV(float a_fX, float a_fY, float const* a_aTri, float* a_pS, float* a_pV)
	{
		float aPt[2] = {a_fX, a_fY};
		float const fSide = sqrtf((a_aTri[4]-a_aTri[2])*(a_aTri[4]-a_aTri[2]) + (a_aTri[5]-a_aTri[3])*(a_aTri[5]-a_aTri[3]));
		float const fHeight = fSide*0.866025403784439f;
		float fBlackDist = distance(a_aTri+2, a_aTri, aPt)/fHeight;
		float const fWhiteDist = distance(a_aTri+4, a_aTri+2, aPt)/fHeight;
		float const fHueDist = distance(a_aTri, a_aTri+4, aPt)/fHeight;
		if (fBlackDist <= 0.0f && fWhiteDist <= 0.0f)
		{
			*a_pS = 0.0f;
			*a_pV = 1.0f;
			return;
		}
		if (fBlackDist <= 0.0f && fHueDist <= 0.0f)
		{
			*a_pS = 1.0f;
			*a_pV = 1.0f;
			return;
		}
		if (fWhiteDist <= 0.0f && fHueDist <= 0.0f)
		{
			*a_pV = 0.0f;
			return;
		}
		if (fBlackDist < 0.0f)
		{
			float const aPt2[2] = {aPt[0], aPt[1]};
			intersection(a_aTri+2, a_aTri, a_aTri+4, aPt2, aPt);
			fBlackDist = 0.0f;
		}
		else if (fWhiteDist < 0.0f)
		{
			float const aPt2[2] = {aPt[0], aPt[1]};
			intersection(a_aTri+4, a_aTri+2, a_aTri, aPt2, aPt);
			fBlackDist = distance(a_aTri+2, a_aTri, aPt)/fHeight;
		}
		else if (fHueDist <= 0.0f)
		{
			float const aPt2[2] = {aPt[0], aPt[1]};
			intersection(a_aTri, a_aTri+4, a_aTri+2, aPt2, aPt);
			fBlackDist = distance(a_aTri+2, a_aTri, aPt)/fHeight;
		}
		if (fBlackDist < 0.0f) fBlackDist = 0.0f;
		float const fV = 1.0f-fBlackDist;
		float const aMid[2] = {(a_aTri[0]+a_aTri[2])*0.5f, (a_aTri[1]+a_aTri[3])*0.5f};
		float const fSatRawDist = distance(a_aTri+4, aMid, aPt);
		if (fV > 1.0f)
		{
			*a_pS = 1.0f;
			*a_pV = 1.0f;
		}
		else
		{
			float const fS = fSatRawDist/fSide/fV+0.5f;
			*a_pV = fV < 0.0f ? 0.0f : fV;
			*a_pS = fS < 0.0f ? 0.0f : (fS > 1.0f ? 1.0f : fS);
		}
	}
	static void GetThumbPos(float const* a_aTri, float a_fS, float a_fV, float* a_pX, float* a_pY)
	{
		float const fX1 = a_aTri[0]*a_fS + a_aTri[2]*(1.0f-a_fS);
		float const fY1 = a_aTri[1]*a_fS + a_aTri[3]*(1.0f-a_fS);
		*a_pX = fX1*a_fV + a_aTri[4]*(1.0f-a_fV);
		*a_pY = fY1*a_fV + a_aTri[5]*(1.0f-a_fV);
	}
	static POINT GetThumbPos(float const* a_aTri, float a_fS, float a_fV)
	{
		float fX = 0.0f;
		float fY = 0.0f;
		GetThumbPos(a_aTri, a_fS, a_fV, &fX, &fY);
		POINT tPt = {fX, fY};
		return tPt;
	}
	void InvalidateThumb(float const* a_aTri)
	{
		POINT tPt = GetThumbPos(a_aTri, m_fS, m_fV);
		RECT rc = {tPt.x-m_nHalfSize-2, tPt.y-m_nHalfSize-2, tPt.x+m_nHalfSize+3, tPt.y+m_nHalfSize+3};
		InvalidateRect(&rc, FALSE);
	}
	void InvalidateTriangle(int a_nWndX, int a_nWndY)
	{
		int const nSize = a_nWndX < a_nWndY ? a_nWndX : a_nWndY;
		RECT rc = {floorf(a_nWndX*0.5f-0.37f*nSize), floorf(a_nWndY*0.5f-0.37f*nSize), ceilf(a_nWndX*0.5f+0.37f*nSize), ceilf(a_nWndY*0.5f+0.37f*nSize)};
		InvalidateRect(&rc, FALSE);
	}

private:
	BYTE* m_pHues;
	float m_fH;
	float m_fV;
	float m_fS;
	float m_fA;
	bool m_bHue;
	float m_fDX;
	float m_fDY;
	HCURSOR m_hCursor;
	float m_fScale;
	int m_nHalfSize;
	float m_fRoundOutlineWidth;
	float m_fTriangleOutlineWidth;
	CGammaTables const* m_pGT;
};

}; // namespace WTL