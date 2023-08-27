
#pragma once

#include <math.h>
#include <agg_bspline.h>

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_conv_dash.h>
#include <agg_path_storage.h>
#include <agg_ellipse.h>
#include <agg_gamma_lut.h>
#include <agg_curves.h>

#include <WTL_ColorArea.h>

float GetIntensity(int a_nHue, int a_nArea1, int a_nArea2, int a_nArea3, int a_nArea4)
{
	if(a_nArea4 < a_nArea1) a_nArea4 += 360;
	if(a_nArea3 < a_nArea1) a_nArea3 += 360;
	if(a_nArea2 < a_nArea1) a_nArea2 += 360;
	if(a_nHue < a_nArea1) a_nHue += 360;
	if(a_nHue >= a_nArea4) return 0.0f;
	if(a_nHue >= a_nArea2 && a_nHue <= a_nArea3) return 1.0f;
	if(a_nHue > a_nArea3) return static_cast<float>((a_nArea4-a_nArea3)-(a_nHue-a_nArea3))/(a_nArea4-a_nArea3);
	return static_cast<float>(a_nHue-a_nArea1)/(a_nArea2-a_nArea1);
}

bool IsBetween(int a_nDir, int a_nNumber, int a_nFrom, int a_nTo)
{
	if(a_nDir > 0 && a_nTo > a_nFrom)
		return a_nNumber < a_nTo && a_nNumber >= a_nFrom;
	if(a_nDir > 0 && a_nTo < a_nFrom)
		return a_nNumber < a_nTo || a_nNumber >= a_nFrom;
	if(a_nDir < 0 && a_nTo > a_nFrom)
		return a_nNumber > a_nTo || a_nNumber <= a_nFrom;
	return a_nNumber > a_nTo && a_nNumber <= a_nFrom;
}

int IndexDistance(int a_nDir, int a_nFrom, int a_nTo)
{
	if(a_nDir > 0 && a_nTo > a_nFrom)
		return a_nTo - a_nFrom;
	if(a_nDir > 0 && a_nTo < a_nFrom)
		return 4 + a_nTo - a_nFrom;
	if(a_nDir < 0 && a_nTo > a_nFrom)
		return 4 + a_nTo - a_nFrom;
	return a_nFrom - a_nTo;
}
int HueDistance(int a_nHue1, int a_nHue2)
{
	int d = abs(a_nHue2 - a_nHue1);
	if(d<180)
		return d;
	return 360-d;
}

void LinearizeArea(int* a_paArea)
{
	while(a_paArea[3] < a_paArea[0]) a_paArea[3] += 360;
	while(a_paArea[2] < a_paArea[0]) a_paArea[2] += 360;
	while(a_paArea[1] < a_paArea[0]) a_paArea[1] += 360;
}
void NormalizeArea(int* a_paArea)
{
	for(int i=0; i<4; ++i)
	{
		a_paArea[i] %= 360;
		if(a_paArea[i] < 0) a_paArea[i] = 360 + a_paArea[i];
	}
}

class CHueBarControl :
	public CWindowImpl<CHueBarControl>
{
public:
	CHueBarControl() :
		m_nDragIndex(-1), m_nSelectedIndex(-1),
		m_hCursorMove(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE))),
		m_hCursorStd(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW))),
		m_hCursorCross(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS))),
		m_bArea(false), m_nHue(0), m_nLight(0), m_nSat(0)
	{
		m_aArea[0] = 150; m_aArea[1] = 160; m_aArea[2] = 200; m_aArea[3] = 210;
	}
	~CHueBarControl()
	{
	}

	enum {
		CHB_AREA_MOVED = 1,
		CHB_DRAG_START,
		CHB_DRAG_FINISHED
	};

	DECLARE_WND_CLASS_EX(_T("HueBarControlWndClass"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CHueBarControl)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
	END_MSG_MAP()

	// operations
public:
	void SetHLS(bool a_bArea, int a_nHue, int a_nLight, int a_nSat)
	{
		m_bArea = a_bArea;
		m_nHue = a_nHue;
		m_nLight = a_nLight;
		m_nSat = a_nSat;
		if(IsWindow())
			Invalidate(FALSE);
	}
	void SetArea(int a_nArea1, int a_nArea2, int a_nArea3, int a_nArea4)
	{
		m_aArea[0] = a_nArea1;
		m_aArea[1] = a_nArea2;
		m_aArea[2] = a_nArea3;
		m_aArea[3] = a_nArea4;
		NormalizeArea(m_aArea);
		if(IsWindow())
			Invalidate(FALSE);
	}
	void GetArea(int& a_nArea1, int& a_nArea2, int& a_nArea3, int& a_nArea4)
	{
		a_nArea1 = m_aArea[0];
		a_nArea2 = m_aArea[1];
		a_nArea3 = m_aArea[2];
		a_nArea4 = m_aArea[3];
	}

	// handlers
public:
	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		DWORD dwStyle = GetWindowLong(GWL_STYLE);

		PAINTSTRUCT ps;
		HDC hDC2 = BeginPaint(&ps);
		RECT rcWin;
		GetClientRect(&rcWin);

		COLORREF clrPen = GetSysColor(COLOR_WINDOWTEXT);
		COLORREF clrBG = GetSysColor(COLOR_BTNFACE);
		COLORREF clrDimmed = 0xff000000 + ((clrPen&0xff) + (clrBG&0xff)) / 2;
		clrDimmed += ((((clrPen&0xff00)>>8) + ((clrBG&0xff00)>>8)) / 2) << 8;
		clrDimmed += ((((clrPen&0xff0000)>>16) + ((clrBG&0xff0000)>>16)) / 2) << 16;

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

		CColorBaseHLSA cColor;
		for(int i=0; i<width; ++i)
		{
			float r,g,b, r2,g2,b2;
			cColor.SetL(0.5f);
			cColor.SetS(1.0f);
			cColor.SetH(360.0f*i/width);
			cColor.GetRGB(&r, &g, &b);
			renb.blend_vline(i, 0, height/3-1, agg::rgba8(255*r, 255*g, 255*b, 255), agg::cover_full);
			float rI = 1.0f;
			if(m_bArea)
			{
				rI = GetIntensity(360.0f*i/width, m_aArea[0], m_aArea[1], m_aArea[2], m_aArea[3]);
				renb.blend_vline(i, height/3, 2*height/3-1, agg::rgba8(GetRValue(clrBG)*(1-rI), GetGValue(clrBG)*(1-rI), GetBValue(clrBG)*(1-rI), 255), agg::cover_full);
			}else
				renb.blend_vline(i, height/3, 2*height/3-1, agg::rgba8(GetRValue(clrBG), GetGValue(clrBG), GetBValue(clrBG), 255), agg::cover_full);
			cColor.SetH(cColor.GetH() + m_nHue);
			float rLight = cColor.GetL() + m_nLight/100.0f;
			if(rLight < 0.0f) rLight = 0.0f;
			if(rLight > 1.0f) rLight = 1.0f;
			cColor.SetL(rLight);
			float rSat = cColor.GetS() + m_nSat/100.0f;
			if(rSat < 0.0f) rSat = 0.0f;
			if(rSat > 1.0f) rSat = 1.0f;
			cColor.SetS(rSat);
			cColor.GetRGB(&r2, &g2, &b2);
			renb.blend_vline(i, 2*height/3, height-1, agg::rgba8(255*(r2*rI + r*(1-rI)), 255*(g2*rI + g*(1-rI)), 255*(b2*rI + b*(1-rI)), 255), agg::cover_full);
		}

		if(m_bArea)
		{
			agg::path_storage path;
			path.move_to(width * m_aArea[0] / 360.0, height/3.0);
			path.line_to(width * m_aArea[0] / 360.0, 2*height/3.0-1);
			agg::conv_stroke<agg::path_storage> dash_stroke(path);
			dash_stroke.line_join(agg::miter_join);
			dash_stroke.width(1.0);
			if(m_nSelectedIndex == 0)
				ren.color(agg::rgba8(255, 0, 0, 255));
			else
				ren.color(agg::rgba8(0, 0, 0, 255));
			ras.add_path(dash_stroke);
			agg::render_scanlines(ras, sl, ren);

			path.remove_all();
			path.move_to(width * m_aArea[1] / 360.0, height/3.0);
			path.line_to(width * m_aArea[1] / 360.0, 2*height/3.0-1);
			if(m_nSelectedIndex == 1)
				ren.color(agg::rgba8(255, 0, 0, 255));
			else
				ren.color(agg::rgba8(GetRValue(clrBG), GetGValue(clrBG), GetBValue(clrBG), 255));
			ras.add_path(dash_stroke);
			agg::render_scanlines(ras, sl, ren);

			path.remove_all();
			path.move_to(width * m_aArea[2] / 360.0, height/3.0);
			path.line_to(width * m_aArea[2] / 360.0, 2*height/3.0-1);
			if(m_nSelectedIndex == 2)
				ren.color(agg::rgba8(255, 0, 0, 255));
			else
				ren.color(agg::rgba8(GetRValue(clrBG), GetGValue(clrBG), GetBValue(clrBG), 255));
			ras.add_path(dash_stroke);
			agg::render_scanlines(ras, sl, ren);				

			path.remove_all();
			path.move_to(width * m_aArea[3] / 360.0, height/3.0);
			path.line_to(width * m_aArea[3] / 360.0, 2*height/3.0-1);
			if(m_nSelectedIndex == 3)
				ren.color(agg::rgba8(255, 0, 0, 255));
			else
				ren.color(agg::rgba8(0, 0, 0, 255));
			ras.add_path(dash_stroke);
			agg::render_scanlines(ras, sl, ren);
		}

		BITMAPINFO bmp_info;
		ZeroMemory(&bmp_info, sizeof bmp_info);
		bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmp_info.bmiHeader.biWidth = width;
		bmp_info.bmiHeader.biHeight = height;
		bmp_info.bmiHeader.biPlanes = 1;
		bmp_info.bmiHeader.biBitCount = 32;
		bmp_info.bmiHeader.biCompression = BI_RGB;

		::SetDIBitsToDevice(hDC2, rcWin.left, rcWin.top, width, height, 0, 0, 0, height, pData.m_p, &bmp_info, 0);
		EndPaint(&ps);
		return 0;
	}
	LRESULT OnEraseBkgnd(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

	LRESULT OnKillFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_nSelectedIndex = -1;
		m_nDragIndex = -1;
		a_bHandled = FALSE;
		Invalidate(FALSE);
		return 1;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			NormalizeArea(m_aArea);
			m_nSelectedIndex = m_nDragIndex;
			m_nDragIndex = -1;
			ReleaseCapture();
			SendNotification(CHB_DRAG_FINISHED);
			Invalidate(FALSE);
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetCapture();

		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		RECT rcWin;
		GetClientRect(&rcWin);
		int width = rcWin.right - rcWin.left;
		int height = rcWin.bottom - rcWin.top;
		int x = 360 * tPt.x / width;
		m_nLastMouse = x;

		if(tPt.y < height / 3 || tPt.y > 2 * height / 3)
			return 0;

		if(m_aArea[2] < x+2 && m_aArea[2] > x-2) m_nDragIndex = 2;
		else if(m_aArea[1] < x+2 && m_aArea[1] > x-2) m_nDragIndex = 1;
		else if(m_aArea[0] < x+2 && m_aArea[0] > x-2) m_nDragIndex = 0;
		else if(m_aArea[3] < x+2 && m_aArea[3] > x-2) m_nDragIndex = 3;
		else
		{
			LinearizeArea(m_aArea);
			int d = x - (m_aArea[1] + m_aArea[2]) / 2;
			m_aArea[0] += d;
			m_aArea[1] += d;
			m_aArea[2] += d;
			m_aArea[3] += d;
			NormalizeArea(m_aArea);
			SendNotification();
			Invalidate(FALSE);
			m_nDragIndex = -2;
		}
		m_nSelectedIndex = m_nDragIndex;

		SendNotification(CHB_DRAG_START);

		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		RECT rcWin;
		GetClientRect(&rcWin);
		int width = rcWin.right - rcWin.left;
		int height = rcWin.bottom - rcWin.top;
		int x = 360.0f * tPt.x / width;

		if (GetCapture() == m_hWnd && m_nDragIndex >= 0)
		{
			int dir = (x - m_nLastMouse) < 0 ? -1 : 1;

			int nOldArea = m_aArea[m_nDragIndex];
			m_aArea[m_nDragIndex] = x;
			m_aArea[m_nDragIndex] %= 360;
			if(m_aArea[m_nDragIndex] == nOldArea) return 0;

			if(m_aArea[m_nDragIndex] < 0) m_aArea[m_nDragIndex] = 360 + m_aArea[m_nDragIndex];
			for(int i=0; i<4; ++i)
			{
				if(i != m_nDragIndex)
				{
					int d = dir * IndexDistance(dir, m_nDragIndex, i);
					if(IsBetween(dir, m_aArea[i], nOldArea, m_aArea[m_nDragIndex] + d))
						m_aArea[i] = m_aArea[m_nDragIndex] + d;
				}
			}
			NormalizeArea(m_aArea);

			m_nLastMouse = x;
			SendNotification();
			Invalidate(FALSE);
		}
		else if (GetCapture() == m_hWnd && m_nDragIndex == -2)
		{
			LinearizeArea(m_aArea);
			int d = x - (m_aArea[1] + m_aArea[2]) / 2;
			m_aArea[0] += d;
			m_aArea[1] += d;
			m_aArea[2] += d;
			m_aArea[3] += d;
			NormalizeArea(m_aArea);
			SendNotification();
			Invalidate(FALSE);
		}
		else
		{
			if(tPt.y < height / 3 || tPt.y > 2 * height / 3)
			{
				SetCursor(m_hCursorStd);
			}else
			{
				if(m_aArea[2] < x+2 && m_aArea[2] > x-2 ||
				   m_aArea[1] < x+2 && m_aArea[1] > x-2 ||
				   m_aArea[0] < x+2 && m_aArea[0] > x-2 ||
				   m_aArea[3] < x+2 && m_aArea[3] > x-2)
					SetCursor(m_hCursorMove);
				else
					SetCursor(m_hCursorCross);
			}
		}
		return 0;
	}

	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRet = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
		if (lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
			::SetFocus(m_hWnd);
		return lRet;
	}
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		int dir = 0;
		switch(wParam)
		{
		case VK_LEFT:
			dir = -1;
			break;
		case VK_RIGHT:
			dir = 1;
			break;
		}
		if(dir != 0)
		{
			int nOldArea = m_aArea[m_nDragIndex];
			m_aArea[m_nDragIndex] += dir;
			m_aArea[m_nDragIndex] %= 360;
			if(m_aArea[m_nDragIndex] < 0) m_aArea[m_nDragIndex] = 360 + m_aArea[m_nDragIndex];
			for(int i=0; i<4; ++i)
			{
				if(i != m_nDragIndex)
				{
					int d = dir * IndexDistance(dir, m_nDragIndex, i);
					if(IsBetween(dir, m_aArea[i], nOldArea, m_aArea[m_nDragIndex] + d))
						m_aArea[i] = m_aArea[m_nDragIndex] + d;
				}
			}
			NormalizeArea(m_aArea);
			bHandled = TRUE;
			SendNotification(CHB_AREA_MOVED);
			return 0;
		}

		bHandled = FALSE;
		return 0;
	}
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		int nX, nY;
		switch(wParam)
		{
		case VK_LEFT:
		case VK_RIGHT:
			bHandled = TRUE;
			SendNotification(CHB_DRAG_FINISHED);
			return 0;
		}
		bHandled = FALSE;
		return 0;
	}

private:
	void SendNotification(int a_nCode = CHB_AREA_MOVED)
	{
		HWND hPar = GetParent();
		if (hPar)
		{
			NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), a_nCode};
			SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}

private:
	bool m_bNoBackBuffer;
	HCURSOR m_hCursorMove;
	HCURSOR m_hCursorCross;
	HCURSOR m_hCursorStd;
	int m_nHue, m_nLight, m_nSat;
	bool m_bArea;
	int m_aArea[4];
	int m_nDragIndex, m_nSelectedIndex;
	int m_nLastMouse;
};
