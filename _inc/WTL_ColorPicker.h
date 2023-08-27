
#pragma once

#include <SubjectImpl.h>
#include <MultiLanguageString.h>
#include <math.h>
#include <RWImaging.h>

#include <GammaCorrection.h>


namespace WTL
{

// simple localization
#define CLRPICK_BUTTON_DEFAULT L"[0409]Default color[0405]Výchozí barva"
#define CLRPICK_BUTTON_CUSTOM L"[0409]More colors...[0405]Další barvy..."
#define CLRPICK_BUTTON_PIXEL L"[0409]From screen pixel[0405]Podle bodu obrazovky"


#ifndef COLORPICKER_NOBUTTON

#ifdef NOICONRENDERER
extern __declspec(selectany) BYTE const g_aPattern[] = // the image is upside down
{
	0x20, 0x00, 
	0x20, 0x00, 
	0x20, 0x00, 
	0x20, 0x00, 
	0x20, 0x00, 
	0x20, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x07, 0x11, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x06, 0x12, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x06, 0x12, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x05, 0x13, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x18, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x18, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x08, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x07, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x0b, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x0a, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x09, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x08, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x07, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x06, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x05, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x04, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x03, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x02, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x01, 0xff, 0x00, 0x00, 
};
extern __declspec(selectany) POINT const g_tHotSpot = {0, 0};
#else
#include <IconRenderer.h>
#endif


class CPixelColorPicker : public CWindowImpl<CPixelColorPicker>
{
public:
	DECLARE_WND_CLASS_EX(_T("PixelColorPicker"), 0, -1) // no background brush

	// returns true if color was picked
	static bool PickColor(COLORREF* a_pColor)
	{
		CPixelColorPicker wnd;
		wnd.Create(NULL, 0, _T("Pixel Color Picker"), WS_POPUP|WS_VISIBLE|WS_MAXIMIZE, WS_EX_TRANSPARENT|WS_EX_TOPMOST);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) // not entirely correct, but...
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!wnd.m_bValid)
			return false;
		*a_pColor = wnd.m_clrPicked;
		return true;
	}

private:
	// private constructor - do not create instances of this window - just use CPixelColorPicker::PickColor
	CPixelColorPicker() : m_bValid(false), m_hLastCursor(NULL)
	{
	}
	~CPixelColorPicker()
	{
		if (m_hLastCursor) DestroyCursor(m_hLastCursor);
	}


	BEGIN_MSG_MAP(CPixelColorPicker)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnCancel)
		MESSAGE_HANDLER(WM_KEYDOWN, OnCancel)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnCancel)
		MESSAGE_HANDLER(WM_MBUTTONDOWN, OnCancel)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnPick)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	END_MSG_MAP()

	LRESULT OnCancel(UINT /*a_uMsg*/, WPARAM /*a_wParam*/, LPARAM /*a_lParam*/, BOOL& /*a_bHandled*/)
	{
		DestroyWindow();
		PostQuitMessage(0); // break the message pump
		return 0;
	}
	LRESULT OnPick(UINT /*a_uMsg*/, WPARAM /*a_wParam*/, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hDC = GetDC();
		m_clrPicked = GetPixel(hDC, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
		ReleaseDC(hDC);
		m_bValid = true;
		return OnCancel(0, 0, 0, a_bHandled);
	}
	LRESULT OnSetCursor(UINT /*a_uMsg*/, WPARAM /*a_wParam*/, LPARAM /*a_lParam*/, BOOL& a_bHandled)
	{
		static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		if (tVersion.dwMajorVersion == 0)
			GetVersionEx(&tVersion);
		if (tVersion.dwMajorVersion < 5)
		{
			a_bHandled = FALSE;
			return 0; // custom cursors will not be used on old systems
		}

		HDC hDC = GetDC();
		POINT tPt;
		GetCursorPos(&tPt);
		COLORREF clr = GetPixel(hDC, tPt.x, tPt.y);
		//DWORD dwPos = GetMessagePos();
		//COLORREF clr = GetPixel(hDC, GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos));
		ReleaseDC(hDC);
		HCURSOR hCur = CreateDropperCursor(clr, clr);
		SetCursor(hCur);
		DestroyCursor(m_hLastCursor);
		m_hLastCursor = hCur;
		return 0;
	}

#ifdef NOICONRENDERER
	// Creates cursor from currently picked color.
	// The cursor must be destroyed using DestroyCursor.
	// The created cursor includes alpha channel and
	// therefore it is NOT compatible with WinNT4 and Win98.
	static HCURSOR CreateDropperCursor(COLORREF a_clr, BYTE const* a_aPattern = g_aPattern, POINT const& a_tHotSpot = g_tHotSpot)
	{
		#pragma pack( push )
		#pragma pack( 2 )
		struct GRPCURSORDIRENTRY
		{
		   BYTE   bWidth;               // Width, in pixels, of the image
		   BYTE   bHeight;              // Height, in pixels, of the image
		   BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
		   BYTE   bReserved;            // Reserved
		   WORD   wX;
		   WORD   wY;
		   DWORD  dwBytesInRes;         // how many bytes in this resource?
		   WORD   nID;                  // the ID
		};

		struct GRPCURSORDIR
		{
		   WORD            idReserved;   // Reserved (must be 0)
		   WORD            idType;       // Resource type (1 for icons)
		   WORD            idCount;      // How many images?
		   GRPCURSORDIRENTRY idEntries[1]; // The entries for each image
		};
		#pragma pack( pop )

		BYTE pData[4 + sizeof(BITMAPINFOHEADER) + 32*32*4 + 4*32]; // hot spot, bitmap header, color data, mask data
		pData[0] = static_cast<BYTE>(a_tHotSpot.x&0xff);
		pData[1] = static_cast<BYTE>((a_tHotSpot.x>>8)&0xff);
		pData[2] = static_cast<BYTE>(a_tHotSpot.y&0xff);
		pData[3] = static_cast<BYTE>((a_tHotSpot.y>>8)&0xff);
		BITMAPINFOHEADER* pHeader = reinterpret_cast<BITMAPINFOHEADER*>(pData+4);
		ZeroMemory(pHeader, sizeof *pHeader);
		pHeader->biSize = sizeof *pHeader;
		pHeader->biWidth = 32;
		pHeader->biHeight = 64; // + mask height
		pHeader->biPlanes = 1;
		pHeader->biBitCount = 32;
		pHeader->biClrUsed = 0;
		pHeader->biClrImportant = 0;
		pHeader->biSizeImage = 32*32*4 + 4*32;
		BYTE* pXOR = reinterpret_cast<BYTE*>(pHeader+1);
		ZeroMemory(pXOR, 32*32*4); // clear the pixels
		int const nR = GetRValue(a_clr);
		int const nG = GetGValue(a_clr);
		int const nB = GetBValue(a_clr);
		BYTE const* pSrc = a_aPattern;
		for (int y = 0; y < 32; ++y)
		{
			BYTE const bToSkip = 32-pSrc[0]-pSrc[1];
			pXOR += 4*pSrc[0];
			pSrc += 2;
			for (BYTE x = pSrc[-1]; x > 0; --x)
			{
				if (*pSrc == 0)
				{
					// transparent pixel
					pXOR[0] = pXOR[1] = pXOR[2] = pXOR[3] = 0;
				}
				else
				{
					pXOR[3] = *pSrc;
					if (pSrc[1] == pSrc[2])
					{
						// grey pixel stays grey
						pXOR[0] = pXOR[1] = pXOR[2] = pSrc[1];
					}
					else
					{
						// colorize the pixel
						//ATLASSERT(pSrc[1] > pSrc[2]);
						BYTE bGrey = pSrc[2];
						BYTE bColor = pSrc[1] - pSrc[2];
						pXOR[0] = static_cast<BYTE>((nB*bColor+127)/255 + bGrey);
						pXOR[1] = static_cast<BYTE>((nG*bColor+127)/255 + bGrey);
						pXOR[2] = static_cast<BYTE>((nR*bColor+127)/255 + bGrey);
					}
				}
				pSrc += 3;
				pXOR += 4;
			}
			pXOR += 4*bToSkip;
		}
		BYTE* pAND = pXOR;
		pXOR -= 4*32*32;
		// create AND mask (TODO: optimize - create the mask only once)
		for (int y = 0; y < 32; ++y)
		{
			for (int x1 = 0; x1 < 4; ++x1)
			{
				BYTE bMask = 0;
				for (int x2 = 0; x2 < 8; ++x2)
				{
					if (pXOR[3] == 0)
					{
						bMask |= 0x80 >> x2;
					}
					pXOR += 4;
				}
				*pAND = bMask;
				++pAND;
			}
		}
		return reinterpret_cast<HCURSOR>(CreateIconFromResourceEx(pData, 4 + sizeof(BITMAPINFOHEADER) + 32*32*4 + 4*32, FALSE, 0x00030000, 32, 32, LR_DEFAULTCOLOR));
	}
#else
	static HCURSOR CreateDropperCursor(COLORREF a_clr1, COLORREF a_clr2)
	{
		ULONG nSX = GetSystemMetrics(SM_CXCURSOR);
		ULONG nSY = GetSystemMetrics(SM_CYCURSOR);
		HKEY hKey = NULL;
		if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, _T("Control Panel\\Cursors"), &hKey))
		{
			DWORD dwValue = 0;
			DWORD dwSize = sizeof dwValue;
			RegQueryValueEx(hKey, _T("CursorBaseSize"), NULL, NULL, reinterpret_cast<BYTE*>(&dwValue), &dwSize);
			if (dwValue > nSX && dwValue <= 256) nSX = dwValue;
			if (dwValue > nSY && dwValue <= 256) nSY = dwValue;
			RegCloseKey(hKey);
		}
		ULONG nHX = 9.0f*nSX/32-0.5f;//((nSX+3)>>2);
		ULONG nHY = 9.0f*nSY/32-0.5f;//((nSY+3)>>2);
		CIconRendererReceiver cRenderer(nSX, nSY);
		IRCanvas canvas = {0, 0, nSX, nSY, 0, 0, NULL, NULL};
		IRFill white(0xffffffff);
		IRFill black(0xff000000);
		float r10 = min(nSX, nSY)/32.0f;
		float r15 = r10*1.5f;
		float r05 = r10*0.5f;
		float len = r10*8.5f;
		float cx = nHX+0.5f;
		float cy = nHY+0.5f;
		IRPolyPoint const topo[] = { {cx, cy-r10}, {cx-r15, cy-r10-r15}, {cx-r15, cy-len+r05}, {cx-r10, cy-len}, {cx+r10, cy-len}, {cx+r15, cy-len+r05}, {cx+r15, cy-r10-r15} };
		IRPolyPoint const topi[] = { {cx, cy-r10*2}, {cx-r10, cy-r10*3}, {cx, cy-r10*4}, {cx-r10, cy-r10*5}, {cx, cy-r10*6}, {cx-r10, cy-r10*7}, {cx, cy-r10*8}, {cx+r10, cy-r10*7}, {cx, cy-r10*6}, {cx+r10, cy-r10*5}, {cx, cy-r10*4}, {cx+r10, cy-r10*3} };
		cRenderer(&canvas, itemsof(topo), topo, &black);
		cRenderer(&canvas, itemsof(topi), topi, &white);
		IRPolyPoint const boto[] = { {cx, cy+r10}, {cx+r15, cy+r10+r15}, {cx+r15, cy+len-r05}, {cx+r10, cy+len}, {cx-r10, cy+len}, {cx-r15, cy+len-r05}, {cx-r15, cy+r10+r15} };
		IRPolyPoint const boti[] = { {cx, cy+r10*2}, {cx+r10, cy+r10*3}, {cx, cy+r10*4}, {cx+r10, cy+r10*5}, {cx, cy+r10*6}, {cx+r10, cy+r10*7}, {cx, cy+r10*8}, {cx-r10, cy+r10*7}, {cx, cy+r10*6}, {cx-r10, cy+r10*5}, {cx, cy+r10*4}, {cx-r10, cy+r10*3} };
		cRenderer(&canvas, itemsof(boto), boto, &black);
		cRenderer(&canvas, itemsof(boti), boti, &white);
		IRPolyPoint const lfto[] = { {cx-r10, cy}, {cx-r10-r15, cy-r15}, {cx-len+r05, cy-r15}, {cx-len, cy-r10}, {cx-len, cy+r10}, {cx-len+r05, cy+r15}, {cx-r10-r15, cy+r15} };
		IRPolyPoint const lfti[] = { {cx-r10*2, cy}, {cx-r10*3, cy-r10}, {cx-r10*4, cy}, {cx-r10*5, cy-r10}, {cx-r10*6, cy}, {cx-r10*7, cy-r10}, {cx-r10*8, cy}, {cx-r10*7, cy+r10}, {cx-r10*6, cy}, {cx-r10*5, cy+r10}, {cx-r10*4, cy}, {cx-r10*3, cy+r10} };
		cRenderer(&canvas, itemsof(lfto), lfto, &black);
		cRenderer(&canvas, itemsof(lfti), lfti, &white);
		IRPolyPoint const rgto[] = { {cx+r10, cy}, {cx+r10+r15, cy-r15}, {cx+len-r05, cy-r15}, {cx+len, cy-r10}, {cx+len, cy+r10}, {cx+len-r05, cy+r15}, {cx+r10+r15, cy+r15} };
		IRPolyPoint const rgti[] = { {cx+r10*2, cy}, {cx+r10*3, cy-r10}, {cx+r10*4, cy}, {cx+r10*5, cy-r10}, {cx+r10*6, cy}, {cx+r10*7, cy-r10}, {cx+r10*8, cy}, {cx+r10*7, cy+r10}, {cx+r10*6, cy}, {cx+r10*5, cy+r10}, {cx+r10*4, cy}, {cx+r10*3, cy+r10} };
		cRenderer(&canvas, itemsof(rgto), rgto, &black);
		cRenderer(&canvas, itemsof(rgti), rgti, &white);

		cRenderer.pixelRow(nHY)[nHX] = 0x80000000;

		IRTarget scale(min(nSX, nSY)/256.0f, -1, -1);

		IRPathPoint const border[] =
		{
			{240, 176, 0, -35.3462, 0, 35.3462},
			{176, 112, -35.3462, 0, 35.3462, 0},
			{112, 176, 0, 35.3462, 0, -35.3462},
			{176, 240, 35.3462, 0, -35.3462, 0},
		};
		IROutlinedFill borderMat(&white, &black, 0, r10);
		cRenderer(&canvas, itemsof(border), border, &borderMat, scale);

		if (a_clr1 == a_clr2)
		{
			IRPathPoint const color[] =
			{
				{224, 176, 0, -26.5097, 0, 26.5097},
				{176, 128, -26.5097, 0, 26.5097, 0},
				{128, 176, 0, 26.5097, 0, -26.5097},
				{176, 224, 26.5097, 0, -26.5097, 0},
			};
			IRFill colorMat(0xff000000|((a_clr1&0xff)<<16)|(a_clr1&0xff00)|((a_clr1&0xff0000)>>16));
			cRenderer(&canvas, itemsof(color), color, &colorMat, scale);
		}
		else
		{
			IRPathPoint const color1[] =
			{
				{224, 176, 0, -26.5097, 0, 0}, 
				{176, 128, 0, 0, 26.5097, 0},
				{128, 176, 0, 26.5097, 0, 0},
				{176, 224, 0, 0, -26.5097, 0},
			};
			IRPathPoint const color2[] =
			{
				{176, 128, -26.5097, 0, 0, 0},
				{128, 176, 0, 0, 0, -26.5097},
				{176, 176, 0, 0, 0, 0},
				{176, 224, 26.5097, 0, 0, 0},
				{224, 176, 0, 0, 0, 26.5097},
				{176, 176, 0, 0, 0, 0},
			};
			IRFill color1Mat(0xff000000|((a_clr1&0xff)<<16)|(a_clr1&0xff00)|((a_clr1&0xff0000)>>16));
			IRFill color2Mat(0xff000000|((a_clr2&0xff)<<16)|(a_clr2&0xff00)|((a_clr2&0xff0000)>>16));
			cRenderer(&canvas, itemsof(color1), color1, &color1Mat, scale);
			cRenderer(&canvas, itemsof(color2), color2, &color2Mat, scale);
		}
		return cRenderer.getCursor(nHX, nHY);
	}
	static HCURSOR CreateDropperCursor(DWORD abgr)
	{
		if ((abgr&0xff000000) == 0xff000000)
			return CreateDropperCursor(abgr&0xffffff, abgr&0xffffff);
		return CreateDropperCursor(CGammaTables::BlendSRGB(abgr, GetSysColor(COLOR_3DLIGHT), (abgr>>24)/255.0f), CGammaTables::BlendSRGB(abgr, GetSysColor(COLOR_3DSHADOW), (abgr>>24)/255.0f));
	}
#endif

private:
	bool m_bValid;
	COLORREF m_clrPicked;
	HCURSOR m_hLastCursor;
};


class CButtonColorPicker :
	public CWindowImpl<CButtonColorPicker>,
	public CThemeImpl<CButtonColorPicker>
{
public:

	enum
	{
		BCPN_SELCHANGE      = 0x8000,	// Colour Picker Selection change
		BCPN_DROPDOWN       = 0x8001,	// Colour Picker drop down
		BCPN_CLOSEUP        = 0x8002,	// Colour Picker close up
		BCPN_SELENDOK       = 0x8003,	// Colour Picker end OK
		BCPN_SELENDCANCEL   = 0x8004,	// Colour Picker end (cancelled)
		FROMPIXEL_BOX_VALUE	= -4,
		DEFAULT_BOX_VALUE	= -3,
		CUSTOM_BOX_VALUE	= -2,
		INVALID_COLOR		= -1,
		OFFSET_AUTOCOLORS	= 0x10000,
		OFFSET_LASTCOLORS	= 0x20000,
		COLORINDEX_MASK		= 0xffff,
	};
	enum EOverride
	{
		ForceDisabled = 0,
		NoOverride = 1,
		ForceEnabled = 2,
	};

	enum EColorType
	{
		ECTNormal = 0,
		ECTDefault,
		ECTCustom,
		ECTFromPixel,
	};

	struct SColor
	{
		SColor() : eCT(ECTNormal), fR(0.0f), fG(0.0f), fB(0.0f), fA(0.0f) {}
		SColor(float a_fR, float a_fG, float a_fB, float a_fA) : eCT(ECTNormal), fR(a_fR), fG(a_fG), fB(a_fB), fA(a_fA) {}
		SColor(DWORD a_clr, bool a_bAlpha = false, float a_fGamma = 2.2f) : eCT(ECTNormal),
			fR(a_fGamma == 2.2f ? CGammaTables::FromSRGB(GetRValue(a_clr)) : powf(GetRValue(a_clr)/255.0f, a_fGamma)),
			fG(a_fGamma == 2.2f ? CGammaTables::FromSRGB(GetGValue(a_clr)) : powf(GetGValue(a_clr)/255.0f, a_fGamma)),
			fB(a_fGamma == 2.2f ? CGammaTables::FromSRGB(GetBValue(a_clr)) : powf(GetBValue(a_clr)/255.0f, a_fGamma)),
			fA(a_bAlpha ? ((a_clr>>24)&0xff)/255.0f : 1.0f) {}
		SColor(EColorType a_eCT) : eCT(a_eCT), fR(0.0f), fG(0.0f), fB(0.0f), fA(0.0f) {}

		EColorType eCT;
		float fR;
		float fG;
		float fB;
		float fA;
		DWORD ToCOLORREF() const
		{
			DWORD const nR = CGammaTables::ToSRGB(fR);
			DWORD const nG = CGammaTables::ToSRGB(fG);
			DWORD const nB = CGammaTables::ToSRGB(fB);
			return (eCT == ECTDefault ? 0xff000000 : 0) | RGB(nR, nG, nB);
		}
		DWORD ToRGBA(float a_fGamma = 2.2f) const
		{
			if (a_fGamma == 2.2f)
			{
				DWORD const nR = CGammaTables::ToSRGB(fR);
				DWORD const nG = CGammaTables::ToSRGB(fG);
				DWORD const nB = CGammaTables::ToSRGB(fB);
				DWORD const nA = fA <= 0.0f ? 0 : (fA >= 1.0f ? 255 : (fA*255.0f+0.5f));
				return (nA<<24)|(nB<<16)|(nG<<8)|nR;
			}
			else
			{
				a_fGamma = 1.0f/a_fGamma;
				DWORD const nR = fR <= 0.0f ? 0 : (fR >= 1.0f ? 255 : powf(fR, a_fGamma)*255.0f+0.5f);
				DWORD const nG = fG <= 0.0f ? 0 : (fG >= 1.0f ? 255 : powf(fG, a_fGamma)*255.0f+0.5f);
				DWORD const nB = fB <= 0.0f ? 0 : (fB >= 1.0f ? 255 : powf(fB, a_fGamma)*255.0f+0.5f);
				DWORD const nA = fA <= 0.0f ? 0 : (fA >= 1.0f ? 255 : (fA*255.0f+0.5f));
				return (nA<<24)|(nB<<16)|(nG<<8)|nR;
			}
		}
		DWORD ToBGRA(float a_fGamma = 2.2f) const
		{
			if (a_fGamma == 2.2f)
			{
				DWORD const nR = CGammaTables::ToSRGB(fR);
				DWORD const nG = CGammaTables::ToSRGB(fG);
				DWORD const nB = CGammaTables::ToSRGB(fB);
				DWORD const nA = fA <= 0.0f ? 0 : (fA >= 1.0f ? 255 : (fA*255.0f+0.5f));
				return (nA<<24)|(nR<<16)|(nG<<8)|nB;
			}
			else
			{
				a_fGamma = 1.0f/a_fGamma;
				DWORD const nR = fR <= 0.0f ? 0 : (fR >= 1.0f ? 255 : powf(fR, a_fGamma)*255.0f+0.5f);
				DWORD const nG = fG <= 0.0f ? 0 : (fG >= 1.0f ? 255 : powf(fG, a_fGamma)*255.0f+0.5f);
				DWORD const nB = fB <= 0.0f ? 0 : (fB >= 1.0f ? 255 : powf(fB, a_fGamma)*255.0f+0.5f);
				DWORD const nA = fA <= 0.0f ? 0 : (fA >= 1.0f ? 255 : (fA*255.0f+0.5f));
				return (nA<<24)|(nR<<16)|(nG<<8)|nB;
			}
		}
		bool operator==(SColor const& a_rhs) const
		{
			return eCT == a_rhs.eCT && fR == a_rhs.fR && fG == a_rhs.fG && fB == a_rhs.fB && fA == a_rhs.fA;
		}
		bool operator!=(SColor const& a_rhs) const
		{
			return !operator==(a_rhs);
		}
	};

	struct NMCOLORBUTTON
	{
		NMHDR	hdr;
		BOOL	fColorValid;
		SColor	clr;
	};

public:

	CButtonColorPicker(bool a_bAlphaChannel = true, LCID a_tLocaleID = GetThreadLocale()) :
		m_wndPicker(this, 1), m_fDPIScale(1.0f), m_bAlphaChannel(a_bAlphaChannel),
		m_fPopupActive(false), m_fTrackSelection(false), m_fMouseOver(false), m_tLocaleID(a_tLocaleID),
		m_clrCurrent(ECTDefault), m_clrDefault(::GetSysColor(COLOR_APPWORKSPACE)),
		m_strDefaultText(CLRPICK_BUTTON_DEFAULT, itemsof(CLRPICK_BUTTON_DEFAULT)-1),
		m_strCustomText(CLRPICK_BUTTON_CUSTOM, itemsof(CLRPICK_BUTTON_CUSTOM)-1),
		m_strFromPixelText(CLRPICK_BUTTON_PIXEL, itemsof(CLRPICK_BUTTON_PIXEL)-1),
		s_sizeTextHiBorder(3, 3), s_sizeTextMargin(2, 2), s_sizeBoxHiBorder(2, 2), s_sizeBoxMargin(0, 0), s_sizeBoxCore(14, 14),
		m_eLastColorsOverride(NoOverride), m_eAutoColorsOverride(NoOverride)
	{
		m_aColors.push_back(m_bAlphaChannel ? SColor(0.0f, 0.0f, 0.0f, 0.0f) : SColor(RGB(0x00, 0x00, 0x00))); //_T("Black")
		m_aColors.push_back(SColor(RGB(0xA5, 0x2A, 0x00))); //_T("Brown")
		m_aColors.push_back(SColor(RGB(0x00, 0x40, 0x40))); //_T("Dark Olive Green")
		m_aColors.push_back(SColor(RGB(0x00, 0x55, 0x00))); //_T("Dark Green")
		m_aColors.push_back(SColor(RGB(0x00, 0x00, 0x5E))); //_T("Dark Teal")
		m_aColors.push_back(SColor(RGB(0x00, 0x00, 0x8B))); //_T("Dark blue")
		m_aColors.push_back(SColor(RGB(0x4B, 0x00, 0x82))); //_T("Indigo")
		m_aColors.push_back(m_bAlphaChannel ? SColor(RGB(0x00, 0x00, 0x00)) : SColor(RGB(0x28, 0x28, 0x28))); //_T("Dark grey")
		m_aColors.push_back(SColor(RGB(0x8B, 0x00, 0x00))); //_T("Dark red")
		m_aColors.push_back(SColor(RGB(0xFF, 0x68, 0x20))); //_T("Orange")
		m_aColors.push_back(SColor(RGB(0x8B, 0x8B, 0x00))); //_T("Dark yellow")
		m_aColors.push_back(SColor(RGB(0x00, 0x93, 0x00))); //_T("Green")
		m_aColors.push_back(SColor(RGB(0x38, 0x8E, 0x8E))); //_T("Teal")
		m_aColors.push_back(SColor(RGB(0x00, 0x00, 0xFF))); //_T("Blue")
		m_aColors.push_back(SColor(RGB(0x7B, 0x7B, 0xC0))); //_T("Blue-grey")
		m_aColors.push_back(m_bAlphaChannel ? SColor(RGB(0x40, 0x40, 0x40)) : SColor(RGB(0x66, 0x66, 0x66))); //_T("Grey - 40")
		m_aColors.push_back(SColor(RGB(0xFF, 0x00, 0x00))); //_T("Red")
		m_aColors.push_back(SColor(RGB(0xFF, 0xAD, 0x5B))); //_T("Light orange")
		m_aColors.push_back(SColor(RGB(0x32, 0xCD, 0x32))); //_T("Lime")
		m_aColors.push_back(SColor(RGB(0x3C, 0xB3, 0x71))); //_T("Sea green")
		m_aColors.push_back(SColor(RGB(0x7F, 0xFF, 0xD4))); //_T("Aqua")
		m_aColors.push_back(SColor(RGB(0x7D, 0x9E, 0xC0))); //_T("Light blue")
		m_aColors.push_back(SColor(RGB(0x80, 0x00, 0x80))); //_T("Violet")
		m_aColors.push_back(SColor(RGB(0x7F, 0x7F, 0x7F))); //_T("Grey - 50")
		m_aColors.push_back(SColor(RGB(0xFF, 0xC0, 0xCB))); //_T("Pink")
		m_aColors.push_back(SColor(RGB(0xFF, 0xD7, 0x00))); //_T("Gold")
		m_aColors.push_back(SColor(RGB(0xFF, 0xFF, 0x00))); //_T("Yellow")
		m_aColors.push_back(SColor(RGB(0x00, 0xFF, 0x00))); //_T("Bright green")
		m_aColors.push_back(SColor(RGB(0x40, 0xE0, 0xD0))); //_T("Turquoise")
		m_aColors.push_back(SColor(RGB(0xC0, 0xFF, 0xFF))); //_T("Skyblue")
		m_aColors.push_back(SColor(RGB(0x48, 0x00, 0x48))); //_T("Plum")
		m_aColors.push_back(SColor(RGB(0xC0, 0xC0, 0xC0))); //_T("Light grey")
		m_aColors.push_back(SColor(RGB(0xFF, 0xE4, 0xE1))); //_T("Rose")
		m_aColors.push_back(SColor(RGB(0xD2, 0xB4, 0x8C))); //_T("Tan")
		m_aColors.push_back(SColor(RGB(0xFF, 0xFF, 0xE0))); //_T("Light yellow")
		m_aColors.push_back(SColor(RGB(0x98, 0xFB, 0x98))); //_T("Pale green ")
		m_aColors.push_back(SColor(RGB(0xAF, 0xEE, 0xEE))); //_T("Pale turquoise")
		m_aColors.push_back(SColor(RGB(0x68, 0x83, 0x8B))); //_T("Pale blue")
		m_aColors.push_back(SColor(RGB(0xE6, 0xE6, 0xFA))); //_T("Lavender")
		m_aColors.push_back(SColor(RGB(0xFF, 0xFF, 0xFF))); //_T("White")

		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		if (pMgr)
		{
			static GUID const tColorGCID = {0x2CBE06C7, 0x4847, 0x4766, {0xAA, 0x01, 0x22, 0x6A, 0xF5, 0x2D, 0x54, 0x88}}; // CLSID_DesignerViewFactoryColorSwatch
			pMgr->Config(tColorGCID, &m_pColorsCfg);
		}
	}

public:
	BOOL SubclassWindow(HWND hWnd)
	{
		CWindowImpl<CButtonColorPicker>::SubclassWindow(hWnd);
		ModifyStyle(0, BS_OWNERDRAW);
		HDC hdc = GetDC();
		m_fDPIScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		ReleaseDC(hdc);
		OpenThemeData(L"Button");
		return TRUE;
	}
	HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		HWND h = CWindowImpl<CButtonColorPicker>::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
		if (h != NULL)
		{
			HDC hdc = GetDC();
			m_fDPIScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
			ReleaseDC(hdc);
			OpenThemeData(L"Button");
		}
		return h;
	}

	void OverrideAutoColors(EOverride a_eOverride)
	{
		m_eAutoColorsOverride = a_eOverride;
	}
	void OverrideLastColors(EOverride a_eOverride)
	{
		m_eLastColorsOverride = a_eOverride;
	}

	std::vector<SColor> const& GetColorTable() const
	{
		return m_aColors;
	}

	void SetColorTable(std::vector<SColor> const& a_aValues)
	{
		m_aColors = a_aValues;
	}

	SColor const& GetColor() const
	{
		return m_clrCurrent;
	}

	void SetColor(SColor clrCurrent, bool a_bAutoSetDefault = false)
	{
		if (a_bAutoSetDefault && !m_strDefaultText.empty() && clrCurrent.eCT == ECTNormal && clrCurrent == m_clrDefault)
			clrCurrent = SColor(ECTDefault);
		if (m_clrCurrent != clrCurrent)
		{
			m_clrCurrent = clrCurrent;
			if (IsWindow())
				InvalidateRect(NULL);
		}
	}

	SColor const& GetDefaultColor() const
	{
		return m_clrDefault;
	}

	void SetDefaultColor(SColor const& clrDefault)
	{
		m_clrDefault = clrDefault;
	}

	void SetCustomText(LPCTSTR pszText)
	{
		m_strCustomText = pszText ? pszText : _T("");
	}

	void SetDefaultText (LPCTSTR pszText)
	{
		m_strDefaultText = pszText ? pszText : _T("");
	}

	void SetFromPixelText (LPCTSTR pszText)
	{
		m_strFromPixelText = pszText ? pszText : _T("");
	}

	bool GetTrackSelection() const
	{
		return m_fTrackSelection;
	}

	void SetTrackSelection(bool fTrack)
	{
		m_fTrackSelection = fTrack;
	}

	bool HasCustomText() const
	{
		return !m_strCustomText.empty();
	}

	bool HasDefaultText() const
	{
		return !m_strDefaultText.empty();
	}

	bool HasFromPixelText() const
	{
		return !m_strFromPixelText.empty();
	}

public:
	BEGIN_MSG_MAP (CButtonColorPicker)
		CHAIN_MSG_MAP(CThemeImpl<CButtonColorPicker>)

		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove);
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave);

	    MESSAGE_HANDLER(OCM__BASE + WM_DRAWITEM, OnDrawItem)

	    REFLECTED_COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)

		ALT_MSG_MAP(1)

		MESSAGE_HANDLER(WM_PAINT, OnPickerPaint);
		MESSAGE_HANDLER(WM_QUERYNEWPALETTE, OnPickerQueryNewPalette);
		MESSAGE_HANDLER(WM_PALETTECHANGED, OnPickerPaletteChanged);
	END_MSG_MAP ()

protected:
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		LPDRAWITEMSTRUCT lpItem = (LPDRAWITEMSTRUCT)lParam;
		CDC dc(lpItem->hDC);

		//
		// Get data about the request
		//

		UINT uState = lpItem->itemState;
		CRect rcDraw = lpItem->rcItem;

		//
		// If we have a theme
		//

		m_fPopupActive = false;
		if (m_hTheme != NULL)
		{

			//
			// Draw the outer edge - with theme
			//

			UINT uFrameState = 0;
			if ((uState & ODS_SELECTED) != 0 || m_fPopupActive)
				uFrameState |= PBS_PRESSED;
			if ((uState & ODS_DISABLED) != 0)
				uFrameState |= PBS_DISABLED;
			if ((uState & ODS_HOTLIGHT) != 0 || m_fMouseOver)
				uFrameState |= PBS_HOT;
			else if ((uState & ODS_DEFAULT) != 0)
				uFrameState |= PBS_DEFAULTED;
			DrawThemeBackground(dc, BP_PUSHBUTTON, uFrameState, &rcDraw, NULL);
			GetThemeBackgroundContentRect(dc, BP_PUSHBUTTON, uFrameState, &rcDraw, &rcDraw);
		}
		else
		{
			//
			// Draw the outer edge - no theme
			//

			UINT uFrameState = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
			if ((uState & ODS_SELECTED) != 0 || m_fPopupActive)
				uFrameState |= DFCS_PUSHED;
			if ((uState & ODS_DISABLED) != 0)
				uFrameState |= DFCS_INACTIVE;
			dc.DrawFrameControl(&rcDraw, DFC_BUTTON, uFrameState);

			//
			// Adjust the position if we are selected (gives a 3d look)
			//
			
			if ((uState & ODS_SELECTED) != 0 || m_fPopupActive)
				rcDraw.OffsetRect(1, 1);
		}

		//
		// Draw focus
		//

		if ((uState & ODS_FOCUS) != 0 || m_fPopupActive) 
		{
			CRect rcFocus(rcDraw.left, rcDraw.top, rcDraw.right, rcDraw.bottom);
			dc.DrawFocusRect(&rcFocus);
		}
		rcDraw.InflateRect(-::GetSystemMetrics(SM_CXEDGE), -::GetSystemMetrics(SM_CYEDGE));

		//
		// Draw the arrow
		//

		{
			CRect rcArrow;
			RECT rc;
			GetClientRect(&rc);
			int ciArrowSizeY = int(rc.bottom*0.15+0.5f);
			int ciArrowSizeX = ciArrowSizeY+ciArrowSizeY;
			rcArrow.left   = rcDraw. right - ciArrowSizeX - ::GetSystemMetrics(SM_CXEDGE);
			rcArrow.top    = (rcDraw.bottom + rcDraw.top)/2 - ciArrowSizeY/2;
			rcArrow.right  = rcArrow.left + ciArrowSizeX;
			rcArrow.bottom = rcArrow.top + ciArrowSizeY;

			DrawArrow(dc, rcArrow, 0, (uState & ODS_DISABLED) ? ::GetSysColor (COLOR_GRAYTEXT) : RGB (0,0,0));

			rcDraw.right = rcArrow.left - ::GetSystemMetrics(SM_CXEDGE)*2;
		}

		//
		// Draw separator
		//

		//dc.DrawEdge(&rcDraw, EDGE_ETCHED, BF_RIGHT);
		//rcDraw.right -= (::GetSystemMetrics(SM_CXEDGE) * 2) + 1 ;

		//
		// Draw color
		//

		if ((uState & ODS_DISABLED) == 0)
		{
			SColor clr = m_clrCurrent.eCT == ECTDefault ? m_clrDefault : m_clrCurrent;
			CRgn cOldRgn;
			int const nCut = min(rcDraw.right-rcDraw.left, rcDraw.bottom-rcDraw.top)>>1;
			bool const bDefault = m_clrCurrent.eCT == ECTDefault || (m_clrCurrent == m_clrDefault && !m_strDefaultText.empty());
			if (bDefault)
			{
				// TODO: default color -> draw rectangle without corner to indicate the default state
				POINT aPts[5] =
				{
					{rcDraw.left, rcDraw.top},
					{rcDraw.right, rcDraw.top},
					{rcDraw.right, rcDraw.bottom-nCut},
					{rcDraw.right-nCut, rcDraw.bottom},
					{rcDraw.left, rcDraw.bottom},
				};
				CRgn cRgn;
				cRgn.CreatePolygonRgn(aPts, 5, WINDING);
				dc.GetClipRgn(cOldRgn);
				dc.SelectClipRgn(cRgn, RGN_AND);
				//	dc.SetBkColor((m_clrCurrent.eCT == CLR_DEFAULT) ? m_clrDefault : m_clrCurrent);
			}
			dc.FrameRect(&rcDraw, (HBRUSH)::GetStockObject(BLACK_BRUSH));
			RECT rcBox = {rcDraw.left+1, rcDraw.top+1, rcDraw.right-1, rcDraw.bottom-1};
			if (clr.fA >= 1.0f)
			{
				dc.SetBkColor(clr.ToRGBA()&0xffffff);
				dc.ExtTextOut(0, 0, ETO_OPAQUE, &rcBox, NULL, 0, NULL);
			}
			else
			{
				int const nSquare = 4*m_fDPIScale+0.5f;
				COLORREF clrHiLight = GetSysColor(COLOR_3DLIGHT);
				COLORREF clrLoLight = GetSysColor(COLOR_3DSHADOW);
				float fA = clr.fA <= 0.0f ? 0.0f : clr.fA;
				float fIA = 1.0f-fA;
				float fR = clr.fR <= 0.0f ? 0.0f : (clr.fR >= 1.0f ? 1.0f : clr.fR);
				float fG = clr.fG <= 0.0f ? 0.0f : (clr.fG >= 1.0f ? 1.0f : clr.fG);
				float fB = clr.fB <= 0.0f ? 0.0f : (clr.fB >= 1.0f ? 1.0f : clr.fB);
				COLORREF clr1 = RGB(CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetRValue(clrHiLight))*fIA + fR*fA), CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetGValue(clrHiLight))*fIA + fG*fA), CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetBValue(clrHiLight))*fIA + fB*fA));
				COLORREF clr2 = RGB(CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetRValue(clrLoLight))*fIA + fR*fA), CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetGValue(clrLoLight))*fIA + fG*fA), CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetBValue(clrLoLight))*fIA + fB*fA));
				for (LONG y = rcBox.top; y < rcBox.bottom; y += nSquare)
				{
					for (LONG x = rcBox.left; x < rcBox.right; x += nSquare)
					{
						dc.SetBkColor(((((y-rcBox.top)/nSquare)&1) == (((x-rcBox.left)/nSquare)&1)) ? clr2 : clr1);
						RECT rc = {x, y, min(x+nSquare, rcBox.right), min(y+nSquare, rcBox.bottom)};
						dc.ExtTextOut(0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
					}
				}
			}
			if (bDefault)
			{
				for (int i = 1; i < nCut-1; ++i)
					dc.SetPixel(rcDraw.right-1-i, rcDraw.bottom-nCut+i, 0);
				dc.SelectClipRgn(cOldRgn);
			}
		}
		return 1;
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if (!m_fMouseOver)
		{
			m_fMouseOver = TRUE;
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof tme;
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = m_hWnd;
			_TrackMouseEvent(&tme);
			InvalidateRect(NULL);
		}
		bHandled = FALSE;
		return FALSE;
	}

	// @cmember Handle mouse leave

	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if (m_fMouseOver)
		{
			m_fMouseOver = FALSE;
			InvalidateRect(NULL);
		}
		bHandled = FALSE;
		return FALSE;
	}

	// @cmember Handle key down for picker

	LRESULT OnPickerKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{

		//
		// Get the key data
		//
		
		UINT nChar = wParam;

		//
		// Get the offset for movement
		//

		int nOffset = 0;
		switch (nChar)
		{
			case VK_DOWN:
				nOffset = m_nNumColumns;
				break;

			case VK_UP:
				nOffset = -m_nNumColumns;
				break;

			case VK_RIGHT:
				nOffset = 1;
				break;

			case VK_LEFT:
				nOffset = -1;
				break;

			case VK_ESCAPE:
				m_clrPicker = m_clrCurrent;
				EndPickerSelection(FALSE);
				break;

			case VK_RETURN:
			case VK_SPACE:
				if (m_nCurrentSel == INVALID_COLOR)
					m_clrPicker = m_clrCurrent;
				EndPickerSelection(m_nCurrentSel != INVALID_COLOR);
				break;
		}

		//
		// If we have an offset
		//

		if (nOffset != 0)
		{

			//
			// Based on our current position, compute a new position
			//

			int nNewSel;
			if (m_nCurrentSel == INVALID_COLOR)
				nNewSel = nOffset > 0 ? DEFAULT_BOX_VALUE : CUSTOM_BOX_VALUE;
			else if (m_nCurrentSel == DEFAULT_BOX_VALUE)
				nNewSel = nOffset > 0 ? FROMPIXEL_BOX_VALUE : CUSTOM_BOX_VALUE;
			else if (m_nCurrentSel == FROMPIXEL_BOX_VALUE)
				nNewSel = nOffset > 0 ? 0 : DEFAULT_BOX_VALUE;
			else if (m_nCurrentSel == CUSTOM_BOX_VALUE)
				nNewSel = nOffset > 0 ? DEFAULT_BOX_VALUE : m_aColors.size() - 1;
			else
			{
				nNewSel = m_nCurrentSel + nOffset;
				if (nNewSel < 0)
					nNewSel = FROMPIXEL_BOX_VALUE;
				else if (nNewSel >= int(m_aColors.size()))
					nNewSel = CUSTOM_BOX_VALUE;
			}

			//
			// Now, for simplicity, the previous code blindly set new 
			// DEFAUT/CUSTOM indexes without caring if we really have those boxes.
			// The following code makes sure we actually map those values into
			// their proper locations.  This loop will run AT the most three times.
			//

			while (true)
			{
				if (nNewSel == DEFAULT_BOX_VALUE && !HasDefaultText ())
					nNewSel = nOffset > 0 ? FROMPIXEL_BOX_VALUE : CUSTOM_BOX_VALUE;
				else if (nNewSel == CUSTOM_BOX_VALUE && !HasCustomText ())
					nNewSel = nOffset > 0 ? DEFAULT_BOX_VALUE : m_aColors.size() - 1;
				else if (nNewSel == FROMPIXEL_BOX_VALUE && !HasFromPixelText ())
					nNewSel = nOffset > 0 ? 0 : DEFAULT_BOX_VALUE;
				else
					break;
			}

			//
			// Set the new location
			//

			ChangePickerSelection(nNewSel);
		}
		bHandled = FALSE;
		return FALSE;
	}

	LRESULT OnPickerLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{

		//
		// Where did the button come up at?
		//

		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		int nNewSelection = PickerHitTest(pt);

		//
		// If valid, then change selection and end
		//

		if (nNewSelection != m_nCurrentSel)
			ChangePickerSelection(nNewSelection);
		EndPickerSelection(nNewSelection != INVALID_COLOR);
		return 0;
	}

	LRESULT OnPickerMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{

		//
		// Do a hit test
		//

		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		int nNewSelection = PickerHitTest(pt);

		//
		// OK - we have the row and column of the current selection 
		// (may be CUSTOM_BOX_VALUE) Has the row/col selection changed? 
		// If yes, then redraw old and new cells.
		//

		if (nNewSelection != m_nCurrentSel)
			ChangePickerSelection(nNewSelection);
		return 0;
	}

	LRESULT OnPickerPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		CPaintDC dc (m_wndPicker);

		//
		// Draw raised window edge (ex-window style WS_EX_WINDOWEDGE is sposed to do this,
		// but for some reason isn't
		//

		CRect rect;
		m_wndPicker.GetClientRect(&rect);
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

		//
		// Draw the Default Area text
		// 
		if (HasDefaultText())
			DrawPickerCell(dc, DEFAULT_BOX_VALUE);

		//
		// Draw the Default Area text
		// 
		if (HasFromPixelText())
			DrawPickerCell(dc, FROMPIXEL_BOX_VALUE);
	 
		//
		// Draw colour cells
		// 

		for (size_t i = 0; i < m_aColors.size(); ++i)
			DrawPickerCell(dc, i);

		for (size_t i = 0; i < m_aAutoColors.size(); ++i)
			DrawPickerCell(dc, i | OFFSET_AUTOCOLORS);

		size_t i;
		for (i = 0; i < m_aLastColors.size(); ++i)
			DrawPickerCell(dc, i | OFFSET_LASTCOLORS);
		for (; i%m_nNumColumns; ++i)
		{
			CRect rect;
			if (GetPickerCellRect(i | OFFSET_LASTCOLORS, &rect)) 
				dc.FillRect(rect, COLOR_MENU);
		}

		// draw empty space between color cell groups
		if (m_aAutoColors.size())
		{
			RECT rc = {m_rectBoxes.left, m_rectAuto.bottom, m_rectBoxes.right, m_rectBoxes.top};
			dc.FillRect(&rc, COLOR_MENU);
		}
		if (m_aLastColors.size())
		{
			RECT rc = {m_rectBoxes.left, m_rectBoxes.bottom, m_rectBoxes.right, m_rectLast.top};
			dc.FillRect(&rc, COLOR_MENU);
		}

		//
		// Draw custom text
		//

		if (HasCustomText())
			DrawPickerCell(dc, CUSTOM_BOX_VALUE);
		return 0;
	}

	LRESULT OnPickerQueryNewPalette(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		Invalidate();
		return DefWindowProc(uMsg, wParam, lParam);
	}

	LRESULT OnPickerPaletteChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		LRESULT lResult = DefWindowProc(uMsg, wParam, lParam);
		if ((HWND) wParam != m_hWnd)
			Invalidate();
		return lResult;
	}

public:
	LRESULT OnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		//
		// Mark button as active and invalidate button to force a redraw
		//

		m_fPopupActive = TRUE;
		InvalidateRect(NULL);

		//
		// Get the parent window
		//

		HWND hWndParent = GetParent();

		//
		// Send the drop down notification to the parent
		//

		SendNotification(BCPN_DROPDOWN, m_clrCurrent, TRUE); 

		//
		// Save the current color for future reference
		//

		SColor clrOldColor = m_clrCurrent;

		//
		// Refresh auto and last colors
		//

		if (m_pColorsCfg)
		{
			CConfigValue cBP;
			m_pColorsCfg->ItemValueGet(CComBSTR(L"ButtonPal"), &cBP);
			bool bAutoColors = m_eAutoColorsOverride == ForceEnabled || (m_eAutoColorsOverride == NoOverride && cBP.TypeGet() == ECVTInteger && (cBP.operator LONG()&2) == 2);
			bool bLastColors = m_eLastColorsOverride == ForceEnabled || (m_eLastColorsOverride == NoOverride && cBP.TypeGet() == ECVTInteger && (cBP.operator LONG()&1) == 1);

			SColor clrCurrent = m_clrCurrent.eCT == ECTDefault ? m_clrDefault : m_clrCurrent;
			m_aAutoColors.clear();
			if (bAutoColors)
			{
				if (clrCurrent.fA >= 7.0f/255.0f)
				{
					if (clrCurrent.fR == clrCurrent.fG && clrCurrent.fR == clrCurrent.fB)
					{
						SColor tColor = clrCurrent;
						tColor.eCT = ECTNormal;
						// shades of grey
						for (int i = 0; i < 8; ++i)
						{
							tColor.fR = tColor.fG = tColor.fB = i/7.0f;
							m_aAutoColors.push_back(tColor);
						}
					}
					else
					{
						float h, l, s;
						RGB2HLS(clrCurrent.fR, clrCurrent.fG, clrCurrent.fB, h, l, s);
						SColor tColor = clrCurrent;
						tColor.eCT = ECTNormal;
						// luminance
						for (int i = 0; i < 8; ++i)
						{
							HLS2RGB(h, i/7.0f, s, tColor.fR, tColor.fG, tColor.fB);
							m_aAutoColors.push_back(tColor);
						}
						// saturation
						for (int i = 0; i < 8; ++i)
						{
							HLS2RGB(h, (l<0.2f || l>0.8f) ? 0.5f : l, i/7.0f, tColor.fR, tColor.fG, tColor.fB);
							m_aAutoColors.push_back(tColor);
						}
					}
				}
				if (m_bAlphaChannel)
				{
					SColor tColor = clrCurrent;
					tColor.eCT = ECTNormal;
					// add colors with different level of transparency
					for (int i = 0; i < 8; ++i)
					{
						tColor.fA = i/7.0f;
						m_aAutoColors.push_back(tColor);
					}
				}
			}

			m_aLastColors.clear();
			if (bLastColors)
			{
				CConfigValue cClrs;
				m_pColorsCfg->ItemValueGet(CComBSTR(L"LastColors"), &cClrs);
				if (cClrs.TypeGet() == ECVTString && cClrs.operator BSTR()[0])
				{
					wchar_t const* psz = cClrs.operator BSTR();
					wchar_t const* pszEnd = psz+SysStringLen(cClrs.operator BSTR());
					SColor tColor;
					tColor.fR = tColor.fG = tColor.fB = 0.0f;
					tColor.fA = 1.0f;
					tColor.eCT = ECTNormal;
					float* aDsts[10];
					int nDsts = 0;
					bool bSkip = false;
					while (psz < pszEnd)
					{
						switch (*psz)
						{
						case L'R':
							aDsts[nDsts++] = &tColor.fR;
							break;
						case L'G':
							aDsts[nDsts++] = &tColor.fG;
							break;
						case L'B':
							aDsts[nDsts++] = &tColor.fB;
							break;
						case L'A':
							aDsts[nDsts++] = &tColor.fA;
							break;
						case L'H':
						case L'L':
						case L'S':
							bSkip = true;
							break;
						case L',':
							break;
						case L'|':
							{
								wchar_t const* psz2 = psz+1;
								while (psz2 < pszEnd && *psz != L'\r' && *psz != L'\n')
									++psz2;
								if (psz2 > psz)
								{
									psz = psz2;
								}
								else
								{
									break;
								}
							}
						case L'\r':
						case L'\n':
							if (psz+1 < pszEnd && psz[1] == ((*psz)^('\r'^'\n')))
								++psz;
							if (!bSkip)
								m_aLastColors.push_back(tColor);
							bSkip = false;
							nDsts = 0;
							tColor.fR = tColor.fG = tColor.fB = 0.0f;
							tColor.fA = 1.0f;
							break;
						case L':':
							if (psz+1 < pszEnd)
								++psz;
						default:
							{
								wchar_t* pszNext = const_cast<wchar_t*>(psz);
								float f = wcstod(psz, &pszNext);
								if (pszNext > psz)
								{
									for (int i = 0; i < nDsts; ++i)
										*(aDsts[i]) = f;
									psz = pszNext-1;
								}
								nDsts = 0;
							}
							break;
						}
						++psz;
						if (m_aLastColors.size() == 8)
							break;
					}
					if (m_aLastColors.size() < 8)
						m_aLastColors.push_back(tColor);
				}
			}
		}

		//
		// Display the popup
		//

		BOOL fOked = Picker();

		//
		// Cancel the popup
		//

		m_fPopupActive = FALSE;

		//
		// If the popup was canceled without a selection
		//

		if (!fOked)
		{

			//
			// If we are tracking, restore the old selection
			//

			if (m_fTrackSelection)
			{
				if (clrOldColor != m_clrCurrent)
				{
					m_clrCurrent = clrOldColor;
					SendNotification(BCPN_SELCHANGE, m_clrCurrent, TRUE); 
				}
			}
			SendNotification(BCPN_CLOSEUP, m_clrCurrent, TRUE); 
			SendNotification(BCPN_SELENDCANCEL, m_clrCurrent, TRUE); 
		}
		else
		{
			if (clrOldColor != m_clrCurrent)
			{
				SendNotification(BCPN_SELCHANGE, m_clrCurrent, TRUE); 
			}
			SendNotification(BCPN_CLOSEUP, m_clrCurrent, TRUE); 
			SendNotification(BCPN_SELENDOK, m_clrCurrent, TRUE); 
		}

		//
		// Invalidate button to force repaint
		//

		InvalidateRect(NULL);
		return TRUE;
	}

// @access Protected methods
protected:

	// @cmember Display the picker popup

	BOOL Picker ()
	{
		BOOL fOked = FALSE;

		//
		// See what version we are using
		//

		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof (osvi);
		::GetVersionEx(&osvi);
		bool fIsXP = osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && (osvi.dwMajorVersion > 5 || (osvi.dwMajorVersion == 5 && osvi .dwMinorVersion >= 1));

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
	   
		//
		// Get the margins
		//

		m_rectMargins.left = ::GetSystemMetrics(SM_CXEDGE);
		m_rectMargins.top = ::GetSystemMetrics(SM_CYEDGE);
		m_rectMargins.right = ::GetSystemMetrics(SM_CXEDGE);
		m_rectMargins.bottom = ::GetSystemMetrics(SM_CYEDGE);

		//
		// Initialize our state
		// 

		m_nCurrentSel       = INVALID_COLOR;
		m_nChosenColorSel	= INVALID_COLOR;
		m_clrPicker			= m_clrCurrent;

		//
		// Create the font
		//

		NONCLIENTMETRICS ncm;
		ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof ncm, &ncm, 0);
		m_font.CreateFontIndirect(&ncm.lfMessageFont);

		//
		// Create the palette
		//

		struct 
		{
			LOGPALETTE    LogPalette;
			PALETTEENTRY  PalEntry[192];
		} pal;

		LOGPALETTE *pLogPalette = (LOGPALETTE *) &pal;
		pLogPalette->palVersion    = 0x300;
		pLogPalette->palNumEntries = (WORD) min(192, m_aColors.size()); 

		for (size_t i = 0; i < m_aColors.size() && i < 192; i++)
		{
			pLogPalette->palPalEntry[i].peRed   = GetRValue(m_aColors[i].ToRGBA());
			pLogPalette->palPalEntry[i].peGreen = GetGValue(m_aColors[i].ToRGBA());
			pLogPalette->palPalEntry[i].peBlue  = GetBValue(m_aColors[i].ToRGBA());
			pLogPalette->palPalEntry[i].peFlags = 0;
		}
		m_palette.CreatePalette(pLogPalette);

		//
		// Register the window class used for the picker
		//

		WNDCLASSEX wc;
		wc.cbSize = sizeof (WNDCLASSEX);
		wc.style  = CS_CLASSDC | CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = CContainedWindow::StartWindowProc;
		wc.cbClsExtra  = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = _pModule->get_m_hInst();
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor (NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = _T("ColorPickerRGBA");
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

		//
		// Create the window
		//

		CRect rcButton;
		GetWindowRect (&rcButton);
		_pModule->AddCreateWndData(&m_wndPicker .m_thunk .cd, &m_wndPicker);
		m_wndPicker.m_hWnd = ::CreateWindowEx (0, (LPCTSTR) MAKELONG (atom, 0), _T(""), WS_POPUP, rcButton .left, rcButton .bottom, 100, 100, GetParent(), NULL, _pModule->get_m_hInst(), NULL);

		//
		// If we created the window
		//

		if (m_wndPicker.m_hWnd != NULL)
		{
	        
			//
			// Set the window size
			//

			SetPickerWindowSize();

			//
			// Create the tooltips
			//

			CToolTipCtrl sToolTip;
			CreatePickerToolTips(sToolTip);

			//
			// Find which cell (if any) corresponds to the initial color
			//

			FindPickerCellFromColor(m_clrCurrent);

			//
			// Make visible
			//
		
			m_wndPicker.ShowWindow(SW_SHOWNA);

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

			m_wndPicker.SetCapture();
			_ASSERTE(m_wndPicker.m_hWnd == ::GetCapture());

			//
			// Get messages until capture lost or cancelled/accepted
			//

			while (m_wndPicker.m_hWnd == ::GetCapture())
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

					// just dispatch rest of the messages
					default:
						DispatchMessage(&msg);
						break;
				}
			}
			::ReleaseCapture();
			fOked = m_fOked;

			//
			// Destroy the window
			//

			sToolTip.DestroyWindow();
			m_wndPicker.DestroyWindow();

			//
			// If needed, show custom
			//

			if (fOked)
			{
				if (m_nCurrentSel == CUSTOM_BOX_VALUE)
				{
					SColor s = m_clrCurrent.eCT == ECTDefault ? m_clrDefault : m_clrCurrent;
					TColor tClr = {s.fR, s.fG, s.fB, s.fA};
					if (ColorWindowModal(&tClr, m_tLocaleID, m_hWnd, m_bAlphaChannel))
						m_clrCurrent = SColor(tClr.fR, tClr.fG, tClr.fB, tClr.fA);
					else
						fOked = FALSE;
				}
				else if (m_nCurrentSel == FROMPIXEL_BOX_VALUE)
				{
					COLORREF clr;
					if (!CPixelColorPicker::PickColor(&clr))
						fOked = FALSE;
					else
						m_clrCurrent = SColor(clr);
				}
				else
					m_clrCurrent = m_clrPicker;
			}

			//
			// Clean up GDI objects
			//

			m_font.DeleteObject();
			m_palette.DeleteObject();
		}

		//
		// Unregister our class
		//

		::UnregisterClass((LPCTSTR) MAKELONG (atom, 0), _pModule->get_m_hInst());
		return fOked;
	}

	void SetPickerWindowSize()
	{
		SIZE szText = {0, 0};

		//
		// If we are showing a custom or default text area, get the font and text size.
		//

		if (HasCustomText() || HasDefaultText() || HasFromPixelText())
		{
			CClientDC dc(m_wndPicker);
			HFONT hfontOld = dc.SelectFont(m_font);

			//
			// Get the size of the custom text (if there IS custom text)
			//

			if (HasCustomText())
			{
				CComBSTR bstrTr;
				CMultiLanguageString::GetLocalized(m_strCustomText.c_str(), m_tLocaleID, &bstrTr);
				COLE2CT strTr(bstrTr.m_str);
				dc.GetTextExtent(strTr, _tcslen(strTr), &szText);
			}

			//
			// Get the size of the default text (if there IS default text)
			//

			if (HasDefaultText())
			{
				SIZE szDefault;
				CComBSTR bstrTr;
				CMultiLanguageString::GetLocalized(m_strDefaultText.c_str(), m_tLocaleID, &bstrTr);
				COLE2CT strTr(bstrTr.m_str);
				dc.GetTextExtent(strTr, _tcslen(strTr), &szDefault);
				if (szDefault.cx > szText.cx)
					szText.cx = szDefault.cx;
				if (szDefault.cy > szText.cy)
					szText.cy = szDefault.cy;
			}

			//
			// Get the size of the default text (if there IS default text)
			//

			if (HasFromPixelText())
			{
				SIZE szFromPixel;
				CComBSTR bstrTr;
				CMultiLanguageString::GetLocalized(m_strFromPixelText.c_str(), m_tLocaleID, &bstrTr);
				COLE2CT strTr(bstrTr.m_str);
				dc.GetTextExtent(strTr, _tcslen(strTr), &szFromPixel);
				if (szFromPixel.cx > szText.cx)
					szText.cx = szFromPixel.cx;
				if (szFromPixel.cy > szText.cy)
					szText.cy = szFromPixel.cy;
			}
			dc.SelectFont(hfontOld);

			//
			// Commpute the final size
			//

			szText.cx += 2 * (s_sizeTextMargin.cx + s_sizeTextHiBorder.cx);
			szText.cy += 2 * (s_sizeTextMargin.cy + s_sizeTextHiBorder.cy);
		}

		//
		// Initiailize our box size
		//

		_ASSERTE(s_sizeBoxHiBorder.cx == s_sizeBoxHiBorder.cy);
		_ASSERTE(s_sizeBoxMargin.cx == s_sizeBoxMargin.cy);
		m_sizeBox.cx = static_cast<int>(s_sizeBoxCore.cx*m_fDPIScale+0.5f) + (s_sizeBoxHiBorder.cx + s_sizeBoxMargin.cx) * 2;
		m_sizeBox.cy = static_cast<int>(s_sizeBoxCore.cy*m_fDPIScale+0.5f) + (s_sizeBoxHiBorder.cy + s_sizeBoxMargin.cy) * 2;

		//
		// Get the number of columns and rows
		//

		m_nNumColumns = 8;
		m_nNumRows = (m_aColors.size()+m_nNumColumns-1) / m_nNumColumns;

		int nLastColorRows = (m_aLastColors.size()+m_nNumColumns-1) / m_nNumColumns;
		int nAutoColorRows = (m_aAutoColors.size()+m_nNumColumns-1) / m_nNumColumns;

		//
		// Compute the min width
		//

		int nBoxTotalWidth = m_nNumColumns * m_sizeBox.cx;
		int nMinWidth = nBoxTotalWidth;
		if (nMinWidth < szText.cx)
			nMinWidth = szText.cx;

		//
		// Create the rectangle for the default text
		//

		m_rectDefaultText = CRect(CPoint(0, 0), CSize(nMinWidth, HasDefaultText() ? szText .cy : 0));

		//
		// Create the rectangle for the default text
		//

		m_rectFromPixelText = CRect(CPoint(0, m_rectDefaultText.bottom), CSize(nMinWidth, (HasFromPixelText() ? szText .cy : 0)));

		//
		// Initialize the color box rectangles
		//

		m_rectAuto = CRect(CPoint((nMinWidth - nBoxTotalWidth) / 2, m_rectFromPixelText.bottom), CSize(nBoxTotalWidth, nAutoColorRows * m_sizeBox.cy));
		m_rectBoxes = CRect(CPoint((nMinWidth - nBoxTotalWidth) / 2, nAutoColorRows ? m_rectAuto.bottom+s_sizeTextMargin.cy*2 : m_rectAuto.bottom), CSize(nBoxTotalWidth, m_nNumRows * m_sizeBox.cy));
		m_rectLast = CRect(CPoint((nMinWidth - nBoxTotalWidth) / 2, nLastColorRows && m_nNumRows ? m_rectBoxes.bottom+s_sizeTextMargin.cy*2 : m_rectBoxes.bottom), CSize(nBoxTotalWidth, nLastColorRows * m_sizeBox.cy));

		//
		// Create the rectangle for the custom text
		//

		m_rectCustomText = CRect(CPoint(0, m_rectLast.bottom), CSize(nMinWidth, HasCustomText() ? szText .cy : 0));

		//
		// Get the current window position, and set the new size
		//

		CRect rectWindow(m_rectDefaultText.TopLeft(), m_rectCustomText.BottomRight());
		CRect rect;
		m_wndPicker.GetWindowRect(&rect);
		rectWindow.OffsetRect(rect.TopLeft());

		//
		// Adjust the rects for the border
		//

		rectWindow.right += m_rectMargins.left + m_rectMargins.right;
		rectWindow.bottom += m_rectMargins.top + m_rectMargins.bottom;
		::OffsetRect(&m_rectDefaultText, m_rectMargins.left, m_rectMargins.top);
		::OffsetRect(&m_rectFromPixelText, m_rectMargins.left, m_rectMargins.top);
		::OffsetRect(&m_rectAuto, m_rectMargins.left, m_rectMargins.top);
		::OffsetRect(&m_rectBoxes, m_rectMargins.left, m_rectMargins.top);
		::OffsetRect(&m_rectLast, m_rectMargins.left, m_rectMargins.top);
		::OffsetRect(&m_rectCustomText, m_rectMargins.left, m_rectMargins.top);

		//
		// Get the screen rectangle
		//

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
				HMONITOR hMonitor = pfnMonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
				mi.cbSize = sizeof mi;
				pfnGetMonitorInfo(hMonitor, &mi);
				rectScreen = mi.rcWork;
			}
		}
	#endif

		//
		// Need to check it'll fit on screen: Too far right?
		//

		if (rectWindow .right > rectScreen .right)
			::OffsetRect (&rectWindow, rectScreen .right - rectWindow .right, 0);

		//
		// Too far left?
		//

		if (rectWindow.left < rectScreen.left)
			::OffsetRect (&rectWindow, rectScreen.left - rectWindow.left, 0);

		//
		// Bottom falling out of screen?  If so, the move
		// the whole popup above the parents window
		//

		if (rectWindow.bottom > rectScreen.bottom)
		{
			CRect rcParent;
			GetWindowRect(&rcParent);
			::OffsetRect(&rectWindow, 0, - ((rcParent.bottom - rcParent.top) + (rectWindow.bottom - rectWindow.top)));
		}

		//
		// Set the window size and position
		//

		m_wndPicker.MoveWindow(&rectWindow, TRUE);
	}

	// @cmember Create the picker tooltips

	void CreatePickerToolTips(CToolTipCtrl &sToolTip)
	{
		//
		// Create the tool tip
		//

		if (!sToolTip.Create(m_wndPicker .m_hWnd)) 
			return;

		CConfigValue cRng(100.0f);
		CConfigValue cDec(1L);
		if (m_pColorsCfg)
		{
			m_pColorsCfg->ItemValueGet(CComBSTR(L"Factor"), &cRng);
			m_pColorsCfg->ItemValueGet(CComBSTR(L"Decimal"), &cDec);
		}
		float const fMul1 = cRng.operator float()*powf(10, cDec.operator LONG());
		float const fMul2 = powf(10, -cDec.operator LONG());

		//
		// Add a tool for each cell
		// 
		for (std::vector<SColor>::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
		{
			CRect rect;
			if (!GetPickerCellRect(i-m_aColors.begin(), &rect)) 
				continue;
			TCHAR szTmp[256];
			_stprintf(szTmp, _T("R: %g\r\nG: %g\r\nB: %g\r\nA: %g"), int(i->fR*fMul1)*fMul2, int(i->fG*fMul1)*fMul2, int(i->fB*fMul1)*fMul2, int(i->fA*fMul1)*fMul2);
			sToolTip.AddTool(m_wndPicker.m_hWnd, szTmp, &rect, 1);
		}
		for (std::vector<SColor>::const_iterator i = m_aLastColors.begin(); i != m_aLastColors.end(); ++i)
		{
			CRect rect;
			if (!GetPickerCellRect(i-m_aLastColors.begin()|OFFSET_LASTCOLORS, &rect)) 
				continue;
			TCHAR szTmp[256];
			_stprintf(szTmp, _T("R: %g\r\nG: %g\r\nB: %g\r\nA: %g"), int(i->fR*fMul1)*fMul2, int(i->fG*fMul1)*fMul2, int(i->fB*fMul1)*fMul2, int(i->fA*fMul1)*fMul2);
			sToolTip.AddTool(m_wndPicker.m_hWnd, szTmp, &rect, 1);
		}
		for (std::vector<SColor>::const_iterator i = m_aAutoColors.begin(); i != m_aAutoColors.end(); ++i)
		{
			CRect rect;
			if (!GetPickerCellRect(i-m_aAutoColors.begin()|OFFSET_AUTOCOLORS, &rect))
				continue;
			TCHAR szTmp[256];
			_stprintf(szTmp, _T("R: %g\r\nG: %g\r\nB: %g\r\nA: %g"), int(i->fR*fMul1)*fMul2, int(i->fG*fMul1)*fMul2, int(i->fB*fMul1)*fMul2, int(i->fA*fMul1)*fMul2);
			sToolTip.AddTool(m_wndPicker.m_hWnd, szTmp, &rect, 1);
		}

		sToolTip.SetMaxTipWidth(800);
	}

	// @cmember Get the rect of a given cell

	BOOL GetPickerCellRect(int nIndex, RECT *pRect) const
	{

		//
		// If the custom box
		//

		if (nIndex == CUSTOM_BOX_VALUE)
		{
			*pRect = m_rectCustomText;
			return TRUE;
		}

		//
		// If the default box
		//

		else if (nIndex == DEFAULT_BOX_VALUE)
		{
			*pRect = m_rectDefaultText;
			return TRUE;
		}

		//
		// If the from pixel box
		//

		else if (nIndex == FROMPIXEL_BOX_VALUE)
		{
			*pRect = m_rectFromPixelText;
			return TRUE;
		}

		//
		// Compute the value of the boxes
		//

		else if (nIndex >= OFFSET_LASTCOLORS && nIndex < int(m_aLastColors.size()+OFFSET_LASTCOLORS))
		{
			pRect->left = ((nIndex&COLORINDEX_MASK) % m_nNumColumns) * m_sizeBox.cx + m_rectLast.left;
			pRect->top  = ((nIndex&COLORINDEX_MASK) / m_nNumColumns) * m_sizeBox.cy + m_rectLast.top;
			pRect->right = pRect->left + m_sizeBox.cx;
			pRect->bottom = pRect->top + m_sizeBox.cy;
			return TRUE;
		}

		else if (nIndex >= OFFSET_AUTOCOLORS && nIndex < int(m_aAutoColors.size()+OFFSET_AUTOCOLORS))
		{
			pRect->left = ((nIndex&COLORINDEX_MASK) % m_nNumColumns) * m_sizeBox.cx + m_rectAuto.left;
			pRect->top  = ((nIndex&COLORINDEX_MASK) / m_nNumColumns) * m_sizeBox.cy + m_rectAuto.top;
			pRect->right = pRect->left + m_sizeBox.cx;
			pRect->bottom = pRect->top + m_sizeBox.cy;
			return TRUE;
		}

		else if (nIndex >= 0 && nIndex < int(m_aColors.size()))
		{
			pRect->left = (nIndex % m_nNumColumns) * m_sizeBox.cx + m_rectBoxes.left;
			pRect->top  = (nIndex / m_nNumColumns) * m_sizeBox.cy + m_rectBoxes.top;
			pRect->right = pRect->left + m_sizeBox.cx;
			pRect->bottom = pRect->top + m_sizeBox.cy;
			return TRUE;
		}
		return FALSE;
	}

	void FindPickerCellFromColor(SColor const& clr)
	{
		if (clr.eCT == ECTDefault && HasDefaultText())
		{
			m_nChosenColorSel = DEFAULT_BOX_VALUE;
			return;
		}

		for (size_t i = 0; i < m_aColors.size(); ++i)
		{
			if (m_aColors[i] == clr)
			{
				m_nChosenColorSel = i;
				return;
			}
		}

		for (size_t i = 0; i < m_aLastColors.size(); ++i)
		{
			if (m_aLastColors[i] == clr)
			{
				m_nChosenColorSel = i | OFFSET_LASTCOLORS;
				return;
			}
		}

		if (HasCustomText())
			m_nChosenColorSel = CUSTOM_BOX_VALUE;
		else
			m_nChosenColorSel = INVALID_COLOR;
	}

	void ChangePickerSelection(int nIndex)
	{
		CClientDC dc(m_wndPicker);

		//
		// Clamp the index
		//

		if (nIndex >= 0 && (((nIndex&OFFSET_AUTOCOLORS) && (nIndex&COLORINDEX_MASK) > int(m_aAutoColors.size())) ||
			((nIndex&OFFSET_LASTCOLORS) && (nIndex&COLORINDEX_MASK) > int(m_aLastColors.size())) ||
			(nIndex == (nIndex&COLORINDEX_MASK) && nIndex > int(m_aColors.size()))))
			nIndex = CUSTOM_BOX_VALUE; 

		//
		// If the current selection is valid, redraw old selection with out
		// it being selected
		//

		if (m_nCurrentSel != INVALID_COLOR)
		{
			int nOldSel = m_nCurrentSel;
			m_nCurrentSel = INVALID_COLOR;
			DrawPickerCell(dc, nOldSel);
		}

		//
		// Set the current selection as row/col and draw (it will be drawn selected)
		//

		m_nCurrentSel = nIndex;
		DrawPickerCell(dc, m_nCurrentSel);

		//
		// Store the current colour
		//

		BOOL fValid = TRUE;
		SColor clr;
		if (m_nCurrentSel == CUSTOM_BOX_VALUE || m_nCurrentSel == FROMPIXEL_BOX_VALUE)
			clr = m_clrDefault;
		else if (m_nCurrentSel == DEFAULT_BOX_VALUE)
			clr = m_clrPicker = SColor(ECTDefault);
		else if (m_nCurrentSel == INVALID_COLOR)
		{
			clr = SColor(0);
			fValid = FALSE;
		}
		else if (m_nCurrentSel&OFFSET_AUTOCOLORS)
			clr = m_clrPicker = m_aAutoColors[m_nCurrentSel&COLORINDEX_MASK];
		else if (m_nCurrentSel&OFFSET_LASTCOLORS)
			clr = m_clrPicker = m_aLastColors[m_nCurrentSel&COLORINDEX_MASK];
		else
			clr = m_clrPicker = m_aColors[m_nCurrentSel];

		//
		// Send the message
		//

		if (m_fTrackSelection)
		{
			if (fValid)
				m_clrCurrent = clr;
			InvalidateRect(NULL);
			SendNotification(BCPN_SELCHANGE, m_clrCurrent, fValid); 
		}
	}

	// @cmember End the picker selection process

	void EndPickerSelection(BOOL fOked)
	{
		::ReleaseCapture();
		m_fOked = fOked;
	}

	// @cmember Draw a cell

	void DrawPickerCell(CDC &dc, int nIndex)
	{

		//
		// Get the drawing rect
		//

		CRect rect;
		if (!GetPickerCellRect(nIndex, &rect)) 
			return;

		//
		// Get the text pointer and colors
		//

		CComBSTR bstrText;
		SColor clrBox;
		SIZE sizeMargin;
		SIZE sizeHiBorder;
		if (nIndex == CUSTOM_BOX_VALUE)
		{
			CMultiLanguageString::GetLocalized(m_strCustomText.c_str(), m_tLocaleID, &bstrText);
			sizeMargin = s_sizeTextMargin;
			sizeHiBorder = s_sizeTextHiBorder;
		}
		else if (nIndex == DEFAULT_BOX_VALUE)
		{
			CMultiLanguageString::GetLocalized(m_strDefaultText.c_str(), m_tLocaleID, &bstrText);
			sizeMargin = s_sizeTextMargin;
			sizeHiBorder = s_sizeTextHiBorder;
		}
		else if (nIndex == FROMPIXEL_BOX_VALUE)
		{
			CMultiLanguageString::GetLocalized(m_strFromPixelText.c_str(), m_tLocaleID, &bstrText);
			sizeMargin = s_sizeTextMargin;
			sizeHiBorder = s_sizeTextHiBorder;
		}
		else
		{
			clrBox = (nIndex&OFFSET_AUTOCOLORS) ? m_aAutoColors[nIndex&COLORINDEX_MASK] : ((nIndex&OFFSET_LASTCOLORS) ? m_aLastColors[nIndex&COLORINDEX_MASK] : m_aColors[nIndex]);
			sizeMargin = s_sizeBoxMargin;
			sizeHiBorder = s_sizeBoxHiBorder;
		}
		COLE2CT strText(bstrText.m_str);
		LPCTSTR pszText = strText;

		//
		// Based on the selections, get our colors
		//

		COLORREF clrHiLight;
		COLORREF clrText;
		bool fSelected;
		if (m_nCurrentSel == nIndex)
		{
			fSelected = true;
			clrHiLight = m_clrHiLight;
			clrText = m_clrHiLightText;
		}
		else if (m_nChosenColorSel == nIndex)
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

		//
		// Select and realize the palette
		//

		HPALETTE hpalOld = NULL;
		if (pszText == NULL)
		{
			if (m_palette.m_hPalette != NULL && (dc.GetDeviceCaps (RASTERCAPS) & RC_PALETTE) != 0)
			{
				hpalOld = dc.SelectPalette(m_palette, FALSE);
				dc.RealizePalette();
			}
		}

		//
		// If we are currently selected
		//

		if (fSelected)
		{

			//
			// If we have a background margin, then draw that
			//

			if (sizeMargin.cx > 0 || sizeMargin.cy > 0)
			{
				dc.SetBkColor(m_clrBackground);
				dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
				rect.InflateRect(-sizeMargin.cx, -sizeMargin.cy);
			}

			//
			// Draw the selection rectagle
			//

			dc.SetBkColor(m_clrHiLightBorder);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			rect.InflateRect(-1, -1);

			//
			// Draw the inner coloring
			//

			dc.SetBkColor(clrHiLight);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			rect.InflateRect(- (sizeHiBorder .cx - 1), - (sizeHiBorder .cy - 1));
		}

		//
		// Otherwise, we are not selected
		//

		else
		{
			
			//
			// Draw the background
			//

			dc.SetBkColor(m_clrBackground);
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			rect.InflateRect (-(sizeMargin .cx + sizeHiBorder .cx), -(sizeMargin .cy + sizeHiBorder .cy));
		}

		//
		// Draw custom text
		//

		if (pszText)
		{
			HFONT hfontOld = dc.SelectFont (m_font);
			dc.SetTextColor(clrText);
			dc.SetBkMode(TRANSPARENT);
			dc.DrawText(pszText, _tcslen (pszText), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			dc.SelectFont(hfontOld);
		}        

		//
		// Otherwise, draw color
		//

		else
		{

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
				float fR = clrBox.fR <= 0.0f ? 0.0f : (clrBox.fR >= 1.0f ? 1.0f : clrBox.fR);
				float fG = clrBox.fG <= 0.0f ? 0.0f : (clrBox.fG >= 1.0f ? 1.0f : clrBox.fG);
				float fB = clrBox.fB <= 0.0f ? 0.0f : (clrBox.fB >= 1.0f ? 1.0f : clrBox.fB);
				//COLORREF clr1 = RGB(int(0.5f + 255.0f*powf(powf(GetRValue(clrHiLight)/255.0f, 2.2f)*fIA + fR*fA, 1.0f/2.2f)), int(0.5f + 255.0f*powf(powf(GetGValue(clrHiLight)/255.0f, 2.2f)*fIA + fG*fA, 1.0f/2.2f)), int(0.5f + 255.0f*powf(powf(GetBValue(clrHiLight)/255.0f, 2.2f)*fIA + fB*fA, 1.0f/2.2f)));
				//COLORREF clr2 = RGB(int(0.5f + 255.0f*powf(powf(GetRValue(clrLoLight)/255.0f, 2.2f)*fIA + fR*fA, 1.0f/2.2f)), int(0.5f + 255.0f*powf(powf(GetGValue(clrLoLight)/255.0f, 2.2f)*fIA + fG*fA, 1.0f/2.2f)), int(0.5f + 255.0f*powf(powf(GetBValue(clrLoLight)/255.0f, 2.2f)*fIA + fB*fA, 1.0f/2.2f)));
				COLORREF clr1 = RGB(CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetRValue(clrHiLight))*fIA + fR*fA), CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetGValue(clrHiLight))*fIA + fG*fA), CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetBValue(clrHiLight))*fIA + fB*fA));
				COLORREF clr2 = RGB(CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetRValue(clrLoLight))*fIA + fR*fA), CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetGValue(clrLoLight))*fIA + fG*fA), CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetBValue(clrLoLight))*fIA + fB*fA));
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

		//
		// Restore the pallete
		//

		if (hpalOld && (dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE) != 0)
			dc.SelectPalette(hpalOld, FALSE);
	}

	// @cmember Send notification message

	void SendNotification(UINT nCode, SColor const& clr, BOOL fColorValid)
	{
		NMCOLORBUTTON nmclr;

		nmclr.hdr.code = nCode;
		nmclr.hdr.hwndFrom = m_hWnd;
		nmclr.hdr.idFrom = GetDlgCtrlID();
		nmclr.fColorValid = fColorValid;
		nmclr.clr = clr;
		if (nmclr.clr.eCT == ECTDefault)
		{
			nmclr.clr.fR = m_clrDefault.fR;
			nmclr.clr.fG = m_clrDefault.fG;
			nmclr.clr.fB = m_clrDefault.fB;
			nmclr.clr.fA = m_clrDefault.fA;
		}

		GetParent().SendMessage(WM_NOTIFY, (WPARAM)GetDlgCtrlID(), (LPARAM)&nmclr);
	}

	// @cmember Do a hit test

	int PickerHitTest(const POINT &pt)
	{

		//
		// If we are in the custom text
		//

		if (m_rectCustomText.PtInRect(pt))
			return CUSTOM_BOX_VALUE;

		//
		// If we are in the default text
		//

		if (m_rectDefaultText.PtInRect(pt))
			return DEFAULT_BOX_VALUE;

		//
		// If we are in the default text
		//

		if (m_rectFromPixelText.PtInRect(pt))
			return FROMPIXEL_BOX_VALUE;

		//
		// If the point isn't in the boxes, return invalid color
		//

		if (m_rectBoxes.PtInRect(pt))
		{
			//
			// Convert the point to an index
			//
			int nRow = (pt.y - m_rectBoxes.top) / m_sizeBox.cy;
			int nCol = (pt.x - m_rectBoxes.left) / m_sizeBox.cx;
			if (nRow < 0 || nCol < 0 || nCol >= m_nNumColumns)
				return INVALID_COLOR;
			int nIndex = nRow * m_nNumColumns + nCol;
			if (nIndex >= int(m_aColors.size()))
				return INVALID_COLOR;
			return nIndex;
		}

		if (m_rectLast.PtInRect(pt))
		{
			int nRow = (pt.y - m_rectLast.top) / m_sizeBox.cy;
			int nCol = (pt.x - m_rectLast.left) / m_sizeBox.cx;
			if (nRow < 0 || nCol < 0 || nCol >= m_nNumColumns)
				return INVALID_COLOR;
			int nIndex = nRow * m_nNumColumns + nCol;
			if (nIndex >= int(m_aLastColors.size()))
				return INVALID_COLOR;
			return nIndex | OFFSET_LASTCOLORS;
		}

		if (m_rectAuto.PtInRect(pt))
		{
			int nRow = (pt.y - m_rectAuto.top) / m_sizeBox.cy;
			int nCol = (pt.x - m_rectAuto.left) / m_sizeBox.cx;
			if (nRow < 0 || nCol < 0 || nCol >= m_nNumColumns)
				return INVALID_COLOR;
			int nIndex = nRow * m_nNumColumns + nCol;
			if (nIndex >= int(m_aAutoColors.size()))
				return INVALID_COLOR;
			return nIndex | OFFSET_AUTOCOLORS;
		}

		return INVALID_COLOR;
	}


protected:
	static void DrawArrow(CDC &dc, const RECT &rect, int iDirection = 0, COLORREF clrArrow = RGB(0, 0, 0))
	{
		POINT ptsArrow[3];
		
		switch (iDirection)
		{
			case 0 : // Down
				{
					ptsArrow[0].x = rect.left;
					ptsArrow[0].y = rect.top;
					ptsArrow[1].x = rect.right;
					ptsArrow[1].y = rect.top;
					ptsArrow[2].x = (rect.left + rect.right) / 2;
					ptsArrow[2].y = rect.bottom;
					break;
				}
				
			case 1 : // Up
				{
					ptsArrow[0].x = rect.left;
					ptsArrow[0].y = rect.bottom;
					ptsArrow[1].x = rect.right;
					ptsArrow[1].y = rect.bottom;
					ptsArrow[2].x = (rect.left + rect.right) / 2;
					ptsArrow[2].y = rect.top;
					break;
				}
				
			case 2 : // Left
				{
					ptsArrow[0].x = rect.right;
					ptsArrow[0].y = rect.top;
					ptsArrow[1].x = rect.right;
					ptsArrow[1].y = rect.bottom;
					ptsArrow[2].x = rect.left;
					ptsArrow[2].y = (rect.top + rect.bottom) / 2;
					break;
				}
				
			case 3 : // Right
				{
					ptsArrow[0].x = rect.left;
					ptsArrow[0].y = rect.top;
					ptsArrow[1].x = rect.left;
					ptsArrow[1].y = rect.bottom;
					ptsArrow[2].x = rect.right;
					ptsArrow[2].y = (rect.top + rect.bottom) / 2;
					break;
				}
		}
		
		CBrush brArrow;
		brArrow.CreateSolidBrush(clrArrow);
		CPen penArrow;
		penArrow.CreatePen(PS_SOLID, 0, clrArrow);

		HBRUSH hbrOld = dc.SelectBrush(brArrow);
		HPEN hpenOld = dc.SelectPen(penArrow);

		dc.SetPolyFillMode(WINDING);
		dc.Polygon(ptsArrow, 3);

		dc.SelectBrush(hbrOld);
		dc.SelectPen(hpenOld);
		return;
	}

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

public:
	LCID m_tLocaleID;

private:

	CComPtr<IConfig> m_pColorsCfg;
	EOverride m_eAutoColorsOverride;
	EOverride m_eLastColorsOverride;

	//
	// THE FOLLOWING variables control the actual button
	//
	float m_fDPIScale;
	bool m_bAlphaChannel;
	SColor		 m_clrCurrent;
	SColor		 m_clrDefault;
	std::wstring m_strDefaultText;
	std::wstring m_strCustomText;
	std::wstring m_strFromPixelText;
	bool		 m_fPopupActive;
	bool		 m_fTrackSelection;
	bool		 m_fMouseOver;

	//
	// THE FOLLOWING variables control the popup
	//
	CContainedWindow		m_wndPicker;
	std::vector<SColor>		m_aColors;
	std::vector<SColor>		m_aAutoColors;
	std::vector<SColor>		m_aLastColors;
	int						m_nNumColumns;
    int						m_nNumRows;
    CFont					m_font;
	CPalette				m_palette;
	SColor					m_clrPicker;
	CRect					m_rectMargins;
    CRect					m_rectCustomText;
	CRect					m_rectDefaultText;
	CRect					m_rectFromPixelText;
	CRect					m_rectBoxes;
	CRect					m_rectLast;
	CRect					m_rectAuto;
	BOOL					m_fPickerFlat;
	CSize					m_sizeBox;
    int						m_nCurrentSel;
    int						m_nChosenColorSel;
	BOOL					m_fOked;
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
};

#endif //COLORPICKER_NOBUTTON

}; // namespace WTL


#ifndef COLORPICKER_NOGRADIENT
#include <WTL_ColorGradient.h>
#endif //COLORPICKER_NOGRADIENT

