
#pragma once

inline int GetChannelProps(DWORD dwSourceMask, int nTargetShift, int& nShift, DWORD& dwMask, DWORD& dwOr)
{
	if (dwSourceMask == 0)
	{
		nShift = 0;
		dwMask = 0;
		dwOr |= 255<<nTargetShift;
		return 0;
	}
	int n1 = 0;
	while ((dwSourceMask&1) == 0)
	{
		++n1;
		dwSourceMask >>= 1;
	}
	int n2 = 0;
	while (dwSourceMask)
	{
		++n2;
		dwSourceMask >>= 1;
	}
	nShift = 8+nTargetShift-n1-n2;
	dwMask = ((1<<(min(8, n2)))-1)<<(n2 < 8 ? nTargetShift+8-n2 : nTargetShift);
	return 1;
}

class CClipboardHandler
{
public:
	CClipboardHandler(HWND a_hWnd)
	{
		if (!::OpenClipboard(a_hWnd))
			throw E_FAIL;
	}
	~CClipboardHandler()
	{
		::CloseClipboard();
	}
};

inline bool GetClipboardImage(TImageSize& tSize, CAutoVectorPtr<TPixelChannel>& pPixels)
{
	// try internal format first
	HANDLE hMem = GetClipboardData(CF_DIBV5);
	if (hMem != NULL)
	{
		ULONG const nDataSize = GlobalSize(hMem);
		BITMAPV5HEADER const* pHeader = reinterpret_cast<BITMAPV5HEADER const*>(GlobalLock(hMem));
		if (pHeader->bV5Planes == 1 && pHeader->bV5BitCount == 32 && pHeader->bV5Compression == BI_BITFIELDS)
		{
			ULONG nSizeX = pHeader->bV5Width;
			ULONG nSizeY = abs(pHeader->bV5Height);
			int iShifts[4];
			DWORD dwMasks[4];
			DWORD dwOr = 0;
			int nRecords = 0;
			nRecords += GetChannelProps(pHeader->bV5RedMask, 16, iShifts[nRecords], dwMasks[nRecords], dwOr);
			nRecords += GetChannelProps(pHeader->bV5GreenMask, 8, iShifts[nRecords], dwMasks[nRecords], dwOr);
			nRecords += GetChannelProps(pHeader->bV5BlueMask, 0, iShifts[nRecords], dwMasks[nRecords], dwOr);
			nRecords += GetChannelProps(pHeader->bV5AlphaMask, 24, iShifts[nRecords], dwMasks[nRecords], dwOr);
			bool bChange = true;
			while (bChange)
			{
				bChange = false;
				for (int i = 0; i < nRecords && !bChange; ++i)
				{
					for (int ii = i+1; ii < nRecords && !bChange; ++ii)
					{
						if (iShifts[i] == iShifts[ii])
						{
							bChange = true;
							dwMasks[i] |= dwMasks[ii];
							for (++ii; ii < nRecords; ++ii)
							{
								iShifts[ii-1] = iShifts[ii];
								dwMasks[ii-1] = dwMasks[ii];
							}
							--nRecords;
						}
					}
				}
			}
			tSize.nX = nSizeX;
			tSize.nY = nSizeY;
			pPixels.Attach(new TPixelChannel[nSizeX*nSizeY]);
			ATLASSERT(sizeof(TPixelChannel) == sizeof(DWORD));
			DWORD* pD = reinterpret_cast<DWORD*>(pPixels.m_p);
			DWORD const* pS = reinterpret_cast<DWORD const*>(reinterpret_cast<BYTE const*>(pHeader)+pHeader->bV5Size+sizeof(RGBQUAD)*(pHeader->bV5ClrUsed)+pHeader->bV5ProfileSize);
			if (reinterpret_cast<BYTE const*>(pHeader)+nDataSize >= reinterpret_cast<BYTE const*>(pS+nSizeX*nSizeY+3) &&
				pHeader->bV5RedMask == pS[0] && pHeader->bV5GreenMask == pS[1] && pHeader->bV5BlueMask == pS[2])
				pS += 3;
			for (ULONG y = 0; y < nSizeY; ++y)
			{
				DWORD const* pSL = pS + nSizeX*(pHeader->bV5Height > 0 ? nSizeY-y-1 : y);
				if (nRecords == 1 && dwOr == 0)
				{
					CopyMemory(pD, pSL, sizeof(DWORD)*nSizeX);
					pD += nSizeX;
				}
				else
				{
					DWORD const* pEnd = pSL+nSizeX;
					while (pSL != pEnd)
					{
						DWORD dw = dwOr;
						for (int i = 0; i < nRecords; ++i)
						{
							if (iShifts[i] < 0)
								dw |= ((*pSL)>>iShifts[i])&dwMasks[i];
							else
								dw |= ((*pSL)<<-iShifts[i])&dwMasks[i];
						}
						*pD = dw;
						++pSL;
						++pD;
					}
				}
			}
		}
		else if (pHeader->bV5Planes == 1 && (pHeader->bV5BitCount == 32 || pHeader->bV5BitCount == 24) && pHeader->bV5Compression == BI_RGB)
		{
			ULONG nSizeX = pHeader->bV5Width;
			ULONG nSizeY = abs(pHeader->bV5Height);
			tSize.nX = nSizeX;
			tSize.nY = nSizeY;
			size_t nRowLen = pHeader->bV5BitCount == 24 ? (nSizeX*3+3)&0xfffffffc : nSizeX<<2;
			pPixels.Attach(new TPixelChannel[tSize.nX*tSize.nY]);
			TPixelChannel* pD = pPixels.m_p;
			BYTE const* pS = reinterpret_cast<BYTE const*>(pHeader)+pHeader->bV5Size+sizeof(RGBQUAD)*(/*3+*/pHeader->bV5ClrUsed)+pHeader->bV5ProfileSize;
			for (ULONG y = 0; y < nSizeY; ++y)
			{
				BYTE const* pSL = pS + nRowLen*(pHeader->bV5Height > 0 ? nSizeY-y-1 : y);
				BYTE const* pEnd = pSL+(pHeader->bV5BitCount>>3)*nSizeX;
				if (pHeader->bV5BitCount == 32)
				{
					while (pSL != pEnd)
					{
						pD->bB = pSL[0];
						pD->bG = pSL[1];
						pD->bR = pSL[2];
						pD->bA = pSL[3];
						pSL += 4;
						++pD;
					}
				}
				else
				{
					while (pSL != pEnd)
					{
						pD->bB = pSL[0];
						pD->bG = pSL[1];
						pD->bR = pSL[2];
						pD->bA = 0xff;
						pSL += 3;
						++pD;
					}
				}
			}
		}
		GlobalUnlock(hMem);
	}

	if (pPixels.m_p == NULL)
	{
		// try bitmap
		HBITMAP hBmp = reinterpret_cast<HBITMAP>(GetClipboardData(CF_BITMAP));
		if (hBmp == NULL)
			return false;

		HDC hDisplayDC = ::GetDC(NULL);
		BITMAPINFO tBI;
		ZeroMemory(&tBI, sizeof(tBI));
		tBI.bmiHeader.biSize = sizeof(tBI.bmiHeader);
		if (0 == GetDIBits(hDisplayDC, hBmp, 0, 0, NULL, &tBI, DIB_RGB_COLORS))
		{
			::ReleaseDC(NULL, hDisplayDC);
			return false;
		}
		tBI.bmiHeader.biPlanes = 1;
		tBI.bmiHeader.biCompression = BI_RGB;
		tBI.bmiHeader.biBitCount = 24;
		size_t nRowLen = (tBI.bmiHeader.biWidth*3+3)&0xfffffffc;
		CAutoVectorPtr<BYTE> pMem(new BYTE[nRowLen*tBI.bmiHeader.biHeight]);
		tBI.bmiHeader.biHeight = -tBI.bmiHeader.biHeight;
		GetDIBits(hDisplayDC, hBmp, 0, -tBI.bmiHeader.biHeight, pMem, &tBI, DIB_RGB_COLORS);
		tBI.bmiHeader.biHeight = -tBI.bmiHeader.biHeight;
		::ReleaseDC(NULL, hDisplayDC);

		tSize.nX = tBI.bmiHeader.biWidth;
		tSize.nY = tBI.bmiHeader.biHeight;
		pPixels.Attach(new TPixelChannel[tSize.nX*tSize.nY]);

		TPixelChannel* pDst = pPixels;
		BYTE* pSrc = pMem;
		size_t nRowDiff = nRowLen-3*tSize.nX;
		for (ULONG iY = 0; iY < tSize.nY; ++iY)
		{
			for (ULONG iX = 0; iX < tSize.nX; ++iX)
			{
				pDst->bR = pSrc[2];
				pDst->bG = pSrc[1];
				pDst->bB = pSrc[0];
				pDst->bA = 0xff;
				++pDst;
				pSrc += 3;
			}
			pSrc += nRowDiff;
		}
		pMem.Free();
	}
	return true;
}

inline bool GetClipboardImage(HWND a_hParentWnd, TImageSize& tSize, CAutoVectorPtr<TPixelChannel>& pPixels)
{
	CClipboardHandler cClipboard(a_hParentWnd);

	bool bBitmap = false;
	bool bInternal = false;
	ULONG iFmt = 0;
	while (true)
	{
		iFmt = EnumClipboardFormats(iFmt);
		if (iFmt == 0)
			break;
		if (iFmt == CF_DIBV5)
			bInternal = true;
		if (iFmt == CF_BITMAP)
			bBitmap = true;
	}
	if (!bInternal && !bBitmap)
		return false;

	return GetClipboardImage(tSize, pPixels);
}
