
#pragma once

#include <math.h>

#ifndef _2DROT_SIMPLE_
#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_conv_dash.h>
#include <agg_path_storage.h>
#include <agg_ellipse.h>
#include <agg_gamma_lut.h>
#endif

namespace WTL
{

class C2DRotation :
	public CWindowImpl<C2DRotation>,
	public CThemeImpl<C2DRotation>
{
public:
	C2DRotation() : m_fAngle(0.0f), m_bNoBackBuffer(false), m_nFromX(0), m_nFromY(0), m_hCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)))
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~C2DRotation()
	{
	}

	enum {
		C2DR_ANGLE_CHANGED = 1,
		C2DR_DRAG_FINISHED = 2
	};


	DECLARE_WND_CLASS_EX(_T("WTL_2DRotationWndClass"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(C2DRotation)
		CHAIN_MSG_MAP(CThemeImpl<C2DRotation>)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	END_MSG_MAP()

	// operations
public:
	void SetAngle(float a_fDegrees)
	{
		m_fAngle = a_fDegrees;
		Invalidate(FALSE);
	}

	float GetAngle() const
	{
		return m_fAngle;
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hdc = GetDC();
		m_fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		ReleaseDC(hdc);
		return 0;
	}

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (reinterpret_cast<HWND>(a_wParam) == m_hWnd)
		{
			SetCursor(m_hCursor);
			return TRUE;
		}
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
			SendNotification(C2DR_DRAG_FINISHED);
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetCapture();
		m_nFromX = GET_X_LPARAM(a_lParam);
		m_nFromY = GET_Y_LPARAM(a_lParam);
		m_fStart = m_fAngle;
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			int nFromX = GET_X_LPARAM(a_lParam);
			int nFromY = GET_Y_LPARAM(a_lParam);
			RECT rcWin;
			GetClientRect(&rcWin);
			int nCenterX = rcWin.right>>1;
			int nCenterY = rcWin.bottom>>1;
			int nSize = (min(nCenterX, nCenterY))*2/3;
			if ((nFromX == m_nFromX && nFromY == m_nFromY) ||
				(nFromX == nCenterX && nFromY == nCenterY) ||
				(nCenterX == m_nFromX && nCenterY == m_nFromY))
				return 0;
			float fOld = PointToAngle(m_nFromX-nCenterX, nCenterY-m_nFromY);
			float fNew = PointToAngle(nFromX-nCenterX, nCenterY-nFromY);
			m_fAngle = m_fStart+fNew-fOld;
			while (m_fAngle < 0.0f)
				m_fAngle += 360.0f;
			while (m_fAngle >= 360.0f)
				m_fAngle -= 360.0f;
			SendNotification();
			Invalidate(FALSE);
		}
		return 0;
	}

	LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		// get window style
		DWORD dwStyle = GetWindowLong(GWL_STYLE);

		RECT rcWin;
		GetClientRect(&rcWin);

		COLORREF clrPen = GetSysColor(COLOR_WINDOWTEXT);
		COLORREF clrBG = GetSysColor(COLOR_WINDOW);
		if (dwStyle & WS_DISABLED)
		{
			clrPen = (((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
					 (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
					 (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));
		}
		COLORREF clrFill = max(clrPen&0xff, clrBG&0xff) | //(((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
						   (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
						   (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));
		int nCenterX = rcWin.right>>1;
		int nCenterY = rcWin.bottom>>1;
		int nSize = (min(nCenterX, nCenterY))*2/3;
		POINT p[5] =
		{
			{nCenterX-nSize, nCenterY-nSize},
			{nCenterX+nSize, nCenterY-nSize},
			{nCenterX+nSize, nCenterY+nSize},
			{nCenterX-nSize, nCenterY+nSize},
			{nCenterX-nSize, nCenterY-nSize},
		};

		PAINTSTRUCT ps;
		HDC hDC2 = BeginPaint(&ps);
#ifdef _2DROT_SIMPLE_
		HDC hDC;
		HGDIOBJ hBmp;
		if (m_bNoBackBuffer)
		{
			hDC = hDC2;
		}
		else
		{
			hDC = CreateCompatibleDC(hDC2);
			hBmp = SelectObject(hDC, CreateCompatibleBitmap(hDC2, rcWin.right, rcWin.bottom));
		}

		FillRect(hDC, &rcWin, reinterpret_cast<HBRUSH>(COLOR_WINDOW+1));
		HGDIOBJ hSavePen = SelectObject(hDC, CreatePen(PS_DOT, 1, clrPen));
		HGDIOBJ hSaveBrush = SelectObject(hDC, CreateSolidBrush(clrFill));
		Polyline(hDC, p, 5);
		DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 1, clrPen)));
		Ellipse(hDC, p[0].x-2, p[0].y-2, p[0].x+2, p[0].y+2);
		DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 3, clrPen)));
		POINT p2[5];
		for (int i = 0; i < 5; ++i)
		{
			p2[i].x = nCenterX + (p[i].x-nCenterX)*cosf(m_fAngle*3.141528f/180.0f) - (p[i].y-nCenterY)*sinf(m_fAngle*3.141528f/180.0f);
			p2[i].y = nCenterY + (p[i].x-nCenterX)*sinf(m_fAngle*3.141528f/180.0f) + (p[i].y-nCenterY)*cosf(m_fAngle*3.141528f/180.0f);
		};
		Polyline(hDC, p2, 5);
		DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 1, clrPen)));
		Ellipse(hDC, p2[0].x-3, p2[0].y-3, p2[0].x+3, p2[0].y+3);
		DeleteObject(SelectObject(hDC, hSavePen));
		DeleteObject(SelectObject(hDC, hSaveBrush));

		if (!m_bNoBackBuffer)
		{
			BitBlt(hDC2, 0, 0, rcWin.right, rcWin.bottom, hDC, 0, 0, SRCCOPY);
			DeleteObject(SelectObject(hDC, hBmp));
			DeleteDC(hDC);
		}
#else
		int width = rcWin.right - rcWin.left;
		int height = rcWin.bottom - rcWin.top;

		CAutoVectorPtr<BYTE> pData(new BYTE[width*height*4]);

		agg::rendering_buffer rbuf;
		rbuf.attach(pData.m_p, width, height, -width*4); // Use negative stride in order to keep Y-axis consistent with WinGDI, i.e., going down.
		// Pixel format and basic primitives renderer
		agg::pixfmt_bgra32 pixf(rbuf);
		agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		renb.clear(agg::rgba8(GetRValue(clrBG), GetGValue(clrBG), GetBValue(clrBG), 255));
		// Scanline renderer for solid filling.
		agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		// Rasterizer & scanline
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		// draw reference square
        agg::path_storage path1;
		path1.move_to(p[0].x+m_fScale*0.5f, p[0].y+m_fScale*0.5f);
		path1.line_to(p[1].x-m_fScale*0.5f, p[1].y+m_fScale*0.5f);
		path1.line_to(p[2].x-m_fScale*0.5f, p[2].y-m_fScale*0.5f);
		path1.line_to(p[3].x+m_fScale*0.5f, p[3].y-m_fScale*0.5f);
        path1.close_polygon();
		agg::conv_dash<agg::path_storage> dash(path1);
	    agg::conv_stroke<agg::conv_dash<agg::path_storage> > dash_stroke(dash);
		dash.add_dash(2*m_fScale, 2*m_fScale);
		dash_stroke.line_join(agg::miter_join);
		dash_stroke.width(m_fScale);
		ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
		ras.add_path(dash_stroke);
		agg::render_scanlines(ras, sl, ren);

		// ellipse in LU corner
        agg::ellipse e;
        ren.color(agg::rgba8(GetRValue(clrFill), GetGValue(clrFill), GetBValue(clrFill), 255));
        e.init(p[0].x+m_fScale*0.5f, p[0].y+m_fScale*0.5f, m_fScale*2, m_fScale*2, 16);
        ras.add_path(e);
        agg::render_scanlines(ras, sl, ren);
		agg::conv_stroke<agg::ellipse> pge1(e);
		ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
		pge1.width(m_fScale);
		ras.add_path(pge1);
		agg::render_scanlines(ras, sl, ren);

		// draw rotated square
		float const fWidth = 2.0f*m_fScale;
		float const fHalfW = 0.5f*fWidth;
        agg::path_storage path2;
		path2.move_to(
			nCenterX + (p[0].x+fHalfW-nCenterX)*cosf(m_fAngle*3.141528f/180.0f) - (p[0].y+fHalfW-nCenterY)*sinf(m_fAngle*3.141528f/180.0f),
			nCenterY + (p[0].x+fHalfW-nCenterX)*sinf(m_fAngle*3.141528f/180.0f) + (p[0].y+fHalfW-nCenterY)*cosf(m_fAngle*3.141528f/180.0f));
		path2.line_to(
			nCenterX + (p[1].x-fHalfW-nCenterX)*cosf(m_fAngle*3.141528f/180.0f) - (p[1].y+fHalfW-nCenterY)*sinf(m_fAngle*3.141528f/180.0f),
			nCenterY + (p[1].x-fHalfW-nCenterX)*sinf(m_fAngle*3.141528f/180.0f) + (p[1].y+fHalfW-nCenterY)*cosf(m_fAngle*3.141528f/180.0f));
		path2.line_to(
			nCenterX + (p[2].x-fHalfW-nCenterX)*cosf(m_fAngle*3.141528f/180.0f) - (p[2].y-fHalfW-nCenterY)*sinf(m_fAngle*3.141528f/180.0f),
			nCenterY + (p[2].x-fHalfW-nCenterX)*sinf(m_fAngle*3.141528f/180.0f) + (p[2].y-fHalfW-nCenterY)*cosf(m_fAngle*3.141528f/180.0f));
		path2.line_to(
			nCenterX + (p[3].x+fHalfW-nCenterX)*cosf(m_fAngle*3.141528f/180.0f) - (p[3].y-fHalfW-nCenterY)*sinf(m_fAngle*3.141528f/180.0f),
			nCenterY + (p[3].x+fHalfW-nCenterX)*sinf(m_fAngle*3.141528f/180.0f) + (p[3].y-fHalfW-nCenterY)*cosf(m_fAngle*3.141528f/180.0f));
        path2.close_polygon();

		// transparent fill
        ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 64));
        ras.add_path(path2);
        agg::render_scanlines(ras, sl, ren);

		// solid outline (width 3)
		agg::conv_stroke<agg::path_storage> pg(path2);
		pg.line_join(agg::miter_join);
		pg.width(fWidth);
		ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
		ras.add_path(pg);
		agg::render_scanlines(ras, sl, ren);

		// ellipse 2
        ren.color(agg::rgba8(GetRValue(clrFill), GetGValue(clrFill), GetBValue(clrFill), 255));
        e.init(
			nCenterX + (p[0].x+fHalfW-nCenterX)*cosf(m_fAngle*3.141528f/180.0f) - (p[0].y+fHalfW-nCenterY)*sinf(m_fAngle*3.141528f/180.0f),
			nCenterY + (p[0].x+fHalfW-nCenterX)*sinf(m_fAngle*3.141528f/180.0f) + (p[0].y+fHalfW-nCenterY)*cosf(m_fAngle*3.141528f/180.0f),
			m_fScale*3, m_fScale*3, 16);
        ras.add_path(e);
        agg::render_scanlines(ras, sl, ren);
		agg::conv_stroke<agg::ellipse> pge2(e);
		ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
		pge2.width(fWidth);
		ras.add_path(pge2);
		agg::render_scanlines(ras, sl, ren);

		// gamma correction
		agg::gamma_lut<agg::int8u, agg::int8u, 8, 8> gp(2.2);
		pixf.apply_gamma_inv(gp);

		BITMAPINFO bmp_info;
		ZeroMemory(&bmp_info, sizeof bmp_info);
		bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmp_info.bmiHeader.biWidth = width;
		bmp_info.bmiHeader.biHeight = height;
		bmp_info.bmiHeader.biPlanes = 1;
		bmp_info.bmiHeader.biBitCount = 32;
		bmp_info.bmiHeader.biCompression = BI_RGB;

		::SetDIBitsToDevice(hDC2, rcWin.left, rcWin.top, width, height, 0, 0, 0, height, pData.m_p, &bmp_info, 0);
#endif
		EndPaint(&ps);

		return 0;
	}
	LRESULT OnEraseBkgnd(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

private:
	void SendNotification(int a_nCode = C2DR_ANGLE_CHANGED)
	{
		HWND hPar = GetParent();
		if (hPar)
		{
			NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), a_nCode};
			SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}
	float PointToAngle(int a_nX, int a_nY)
	{
		if (a_nX == 0 && a_nY == 0)
			return 0.0f;
		float fDelta = 0.0f;
		if (a_nX < 0)
		{
			fDelta = 180.0f;
			a_nX = -a_nX;
			a_nY = -a_nY;
		}
		if (a_nY < 0)
		{
			fDelta += 90.0f;
			int n = a_nX;
			a_nX = -a_nY;
			a_nY = n;
		}
		return fDelta + atanf(a_nX/float(a_nY))*180.0f/3.141528f;
	}

private:
	float m_fAngle;
	float m_fStart;
	int m_nFromX;
	int m_nFromY;
	bool m_bNoBackBuffer;
	HCURSOR m_hCursor;
	float m_fScale;
};

}; // namespace WTL