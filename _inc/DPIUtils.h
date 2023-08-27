
#pragma once

namespace DPIUtils
{
	//inline HICON LoadCaptionIcon(HMODULE a_hModule, LPCTSTR a_nResourceID)
	//{
	//	return NULL;
	//}

	inline HICON PrepareIconForCaption(HICON a_hSource)
	{
		if (a_hSource == NULL)
			return NULL;
		ICONINFO tInfo;
		ZeroMemory(&tInfo, sizeof tInfo);
		GetIconInfo(a_hSource, &tInfo);
		if (tInfo.hbmMask)
			DeleteObject(tInfo.hbmMask);
		if (tInfo.hbmColor == NULL)
			return a_hSource;
		BITMAP tBmp;
		ZeroMemory(&tBmp, sizeof tBmp);
		GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
		if (tBmp.bmBitsPixel != 32)
		{
			DeleteObject(tInfo.hbmColor);
			return a_hSource; // do not bother low quality icons
		}
		static int nSmSizeX = GetSystemMetrics(SM_CXSMICON);
		static int nSmSizeY = GetSystemMetrics(SM_CYSMICON);
		if ((tBmp.bmWidth >= nSmSizeX || tBmp.bmHeight > nSmSizeY) &&
			(tBmp.bmWidth > nSmSizeX || tBmp.bmHeight >= nSmSizeY))
		{
			DeleteObject(tInfo.hbmColor);
			return a_hSource; // only fix smaller icons
		}

		CAutoVectorPtr<DWORD> pSource(new DWORD[tBmp.bmWidth*tBmp.bmHeight]);
		if (0 == GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pSource.m_p))
		{
			DeleteObject(tInfo.hbmColor);
			return a_hSource; // failure
		}

		DeleteObject(tInfo.hbmColor);

		DWORD nXOR = nSmSizeY*nSmSizeX<<2;
		DWORD nAND = nSmSizeY*((((nSmSizeX+7)>>3)+3)&0xfffffffc);
		CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
		ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);
		BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
		pHead->biSize = sizeof(BITMAPINFOHEADER);
		pHead->biWidth = nSmSizeX;
		pHead->biHeight = nSmSizeY<<1;
		pHead->biPlanes = 1;
		pHead->biBitCount = 32;
		pHead->biCompression = BI_RGB;
		pHead->biSizeImage = nXOR+nAND;
		pHead->biXPelsPerMeter = 0;
		pHead->biYPelsPerMeter = 0;
		DWORD *pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);
		ULONG nRowDelta = nSmSizeX-tBmp.bmWidth;
		pXOR += ((nSmSizeX-tBmp.bmWidth)>>1) + ((nSmSizeY-tBmp.bmHeight)>>1)*nSmSizeX;
		DWORD* pXORSrc = pSource+tBmp.bmWidth*(tBmp.bmHeight-1);
		for (int y = 0; y < tBmp.bmHeight; ++y)
		{
			for (int x = 0; x < tBmp.bmWidth; ++x)
			{
				*pXOR = *pXORSrc;
				++pXORSrc;
				++pXOR;
			}
			pXOR += nRowDelta;
			pXORSrc -= tBmp.bmWidth<<1;
		}
		pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);
		BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(nSmSizeY*nSmSizeX));
		int nANDLine = ((((nSmSizeX+7)>>3)+3)&0xfffffffc);
		for (int y = 0; y < nSmSizeY; ++y)
		{
			for (int x = 0; x < nSmSizeX; ++x)
			{
				if (0 == (0xff000000&*pXOR))
				{
					pAND[x>>3] |= 0x80 >> (x&7);
				}
				++pXOR;
			}
			pAND += nANDLine;
		}
		HICON hTmp = CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, nSmSizeX, nSmSizeY, LR_DEFAULTCOLOR);
		DestroyIcon(a_hSource);
		return hTmp;
	}
};