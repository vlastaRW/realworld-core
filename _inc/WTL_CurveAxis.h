
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

void NormalizeRGB(BYTE& a_bR, BYTE& a_bG, BYTE& a_bB);

class CCurveAxisControl :
	public CWindowImpl<CCurveAxisControl>
{
public:
	enum ECurveAxisType
	{
		EATAlpha,
		EATHueDst,
		EATColorDst,
		EATVignetting,
	};

	CCurveAxisControl()
	{
	}
	~CCurveAxisControl()
	{
	}

	DECLARE_WND_CLASS_EX(_T("CurveAxisControlWndClass"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CCurveAxisControl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	// operations
public:
	void SetAxisType(ECurveAxisType a_eAxisType, BYTE a_bR = 0, BYTE a_bG = 0, BYTE a_bB = 0)
	{
		m_eAxisType = a_eAxisType;
		m_bR = a_bR; m_bG = a_bG; m_bB = a_bB;
		if(m_eAxisType == EATHueDst)
			NormalizeRGB(m_bR, m_bG, m_bB);
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
		COLORREF clrBG = GetSysColor(COLOR_WINDOW);
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

		if(m_eAxisType == EATAlpha)
		{
			bool bOdd = true;
			if(height > width)
				for(int i=0; i<height; i+=width/2, bOdd=!bOdd)
				{
					if(bOdd) renb.blend_bar(0, i, width/2-1, i+width/2-1, agg::rgba8(GetRValue(clrDimmed), GetGValue(clrDimmed), GetBValue(clrDimmed), 255), agg::cover_full);
					else renb.blend_bar(width/2, i, width-1, i+width/2-1, agg::rgba8(GetRValue(clrDimmed), GetGValue(clrDimmed), GetBValue(clrDimmed), 255), agg::cover_full);
				}
			else
				for(int i=0; i<width; i+=height/2, bOdd=!bOdd)
				{
					if(bOdd) renb.blend_bar(i, 0, i+height/2-1, height/2-1, agg::rgba8(GetRValue(clrDimmed), GetGValue(clrDimmed), GetBValue(clrDimmed), 255), agg::cover_full);
					else renb.blend_bar(i, height/2, i+height/2-1, height-1, agg::rgba8(GetRValue(clrDimmed), GetGValue(clrDimmed), GetBValue(clrDimmed), 255), agg::cover_full);
				}
			if(height > width)
				for(int i=0; i<height; ++i)
					renb.blend_hline(0, i, width-1, agg::rgba8(255, 255, 255, 255-255*i/height), agg::cover_full);
			else
				for(int i=0; i<width; ++i)
					renb.blend_vline(i, 0, height-1, agg::rgba8(255, 255, 255, 255*i/width), agg::cover_full);
		}
		else if(m_eAxisType == EATHueDst || m_eAxisType == EATColorDst)
		{
			float fR = (m_bR > 127) ? -m_bR : 255-m_bR;
			float fG = (m_bG > 127) ? -m_bG : 255-m_bG;
			float fB = (m_bB > 127) ? -m_bB : 255-m_bB;
			if(height > width)
			{
				fR /= height;
				fG /= height;
				fB /= height;
				for(int i=0; i<height; ++i)
					renb.blend_hline(0, i, width-1, agg::rgba8(255, 255, 255, 255-255*i/height), agg::cover_full);
			}
			else
			{
				fR /= width;
				fG /= width;
				fB /= width;
				for(int i=0; i<width; ++i)
				{
					//renb.copy_pixel(i);
					renb.blend_vline(i, 0, height-1, agg::rgba8(m_bR+i*fR, m_bG+i*fG, m_bB+i*fB, 255), agg::cover_full);
				}
			}
		}
		else if(m_eAxisType == EATVignetting)
		{
			bool bOdd = true;
			if(height > width)
				for(int i=0; i<height; i+=width/2, bOdd=!bOdd)
				{
					if(bOdd) renb.blend_bar(0, i, width/2-1, i+width/2-1, agg::rgba8(GetRValue(clrDimmed), GetGValue(clrDimmed), GetBValue(clrDimmed), 255), agg::cover_full);
					else renb.blend_bar(width/2, i, width-1, i+width/2-1, agg::rgba8(GetRValue(clrDimmed), GetGValue(clrDimmed), GetBValue(clrDimmed), 255), agg::cover_full);
				}
			else
				for(int i=0; i<width; i+=height/2, bOdd=!bOdd)
				{
					if(bOdd) renb.blend_bar(i, 0, i+height/2-1, height/2-1, agg::rgba8(GetRValue(clrDimmed), GetGValue(clrDimmed), GetBValue(clrDimmed), 255), agg::cover_full);
					else renb.blend_bar(i, height/2, i+height/2-1, height-1, agg::rgba8(GetRValue(clrDimmed), GetGValue(clrDimmed), GetBValue(clrDimmed), 255), agg::cover_full);
				}
			if(height > width)
				for(int i=0; i<height; ++i)
				{
					if(i > height / 2)
						renb.blend_hline(0, i, width-1, agg::rgba8(255, 255, 255, 512*(i-height/2)/height), agg::cover_full);
					else
						renb.blend_hline(0, i, width-1, agg::rgba8(m_bR, m_bG, m_bB, 255-510*i/height), agg::cover_full);
				}
			else
				for(int i=0; i<width; ++i)
				{
					if(i > width / 2)
						renb.blend_vline(i, 0, height-1, agg::rgba8(m_bR, m_bG, m_bB, 512*(i-width/2)/width), agg::cover_full);
					else
						renb.blend_hline(0, i, width-1, agg::rgba8(255, 255, 255, 255-510*i/width), agg::cover_full);
				}
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

private:
	ECurveAxisType m_eAxisType;
	BYTE m_bR, m_bG, m_bB;
	bool m_bBSpline;
	TCurvePoints m_aPoints;
	int m_nDragIndex;
	bool m_bNoBackBuffer;
	HCURSOR m_hCursorMove;
	HCURSOR m_hCursorStd;
	HCURSOR m_hCursorCross;
};
