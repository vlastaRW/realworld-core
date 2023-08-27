
#pragma once


inline HRESULT RGBAGetTileImpl(TImageSize const& a_tSrcCanvasSize, TImagePoint const& a_tSrcOrigin, TImageSize const& a_tSrcSize, TPixelChannel const* a_pSrc, ULONG a_nSrcStrideY, TPixelChannel const a_tDefault, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pPixels)
{
	if (a_pOrigin && a_pSize == NULL)
		return E_RW_INVALIDPARAM; // if Origin is set, so must be Size
	TImagePoint tOrigin = {0, 0};
	if (a_pOrigin) tOrigin = *a_pOrigin;
	TImagePoint const tEnd =
	{
		a_pSize ? tOrigin.nX+a_pSize->nX : tOrigin.nX+a_tSrcCanvasSize.nX,
		a_pSize ? tOrigin.nY+a_pSize->nY : tOrigin.nY+a_tSrcCanvasSize.nY
	};
	TImageSize const tSize = {tEnd.nX-tOrigin.nX, tEnd.nY-tOrigin.nY};

	TImageSize const tStride = {a_pStride ? a_pStride->nX : 1, a_pStride ? a_pStride->nY : tEnd.nX-tOrigin.nX};
	ULONG const nLineDiff = tStride.nY-tStride.nX*tSize.nX;
	if (a_nPixels < tStride.nY*tSize.nY)
	{
		ATLASSERT(0);
		return E_FAIL; // buffer not big enough
	}

	TImagePoint const tSrcEnd = {a_tSrcOrigin.nX+a_tSrcSize.nX, a_tSrcOrigin.nY+a_tSrcSize.nY};

	if (tOrigin.nX >= tSrcEnd.nX || tOrigin.nY >= tSrcEnd.nY || tEnd.nX <= a_tSrcOrigin.nX || tEnd.nY <= a_tSrcOrigin.nY)
	{
		// empty tile
		if (tStride.nX == 1)
		{
			if (tSize.nX == tStride.nY)
			{
				std::fill_n(a_pPixels, tSize.nX*tSize.nY, a_tDefault);
			}
			else
			{
				for (; tOrigin.nY < tEnd.nY; ++tOrigin.nY)
				{
					std::fill_n(a_pPixels, tSize.nX, a_tDefault);
					a_pPixels += tStride.nY;
				}
			}
		}
		else
		{
			for (; tOrigin.nY < tEnd.nY; ++tOrigin.nY)
			{
				for (TPixelChannel* const pEnd = a_pPixels+tStride.nX*tSize.nX; a_pPixels != pEnd; a_pPixels += tStride.nX)
					*a_pPixels = a_tDefault;
				a_pPixels += nLineDiff;
			}
		}
		return S_OK;
	}

	if (tStride.nX == 1)
	{
		for (LONG const nYEnd = tEnd.nY < a_tSrcOrigin.nY ? tEnd.nY : a_tSrcOrigin.nY; tOrigin.nY < nYEnd; ++tOrigin.nY)
		{
			std::fill_n(a_pPixels, tSize.nX, a_tDefault);
			a_pPixels += tStride.nY;
		}
		LONG const nXLen1 = tEnd.nX < a_tSrcOrigin.nX ? tEnd.nX-tOrigin.nX : (tOrigin.nX < a_tSrcOrigin.nX ? a_tSrcOrigin.nX-tOrigin.nX : 0);
		LONG const nXLen3 = tEnd.nX > tSrcEnd.nX ? tEnd.nX-tSrcEnd.nX : 0;
		LONG const nXLen2 = tSize.nX-nXLen1-nXLen3;
		TPixelChannel const* pSrc = a_pSrc+(tOrigin.nY-a_tSrcOrigin.nY)*a_nSrcStrideY+(tOrigin.nX > a_tSrcOrigin.nX ? tOrigin.nX-a_tSrcOrigin.nX : 0);
		ULONG const tSrcLineDiff = a_nSrcStrideY-nXLen2;
		for (LONG const nYEnd = tEnd.nY < tSrcEnd.nY ? tEnd.nY : tSrcEnd.nY; tOrigin.nY < nYEnd; ++tOrigin.nY)
		{
			std::fill_n(a_pPixels, nXLen1, a_tDefault);
			a_pPixels += nXLen1;
			std::copy(pSrc, pSrc+nXLen2, a_pPixels);
			a_pPixels += nXLen2;
			std::fill_n(a_pPixels, nXLen3, a_tDefault);
			a_pPixels += nXLen3+nLineDiff;
			pSrc += a_nSrcStrideY;
		}
		for (; tOrigin.nY < tEnd.nY; ++tOrigin.nY)
		{
			std::fill_n(a_pPixels, tSize.nX, a_tDefault);
			a_pPixels += tStride.nY;
		}
		return S_OK;
	}

	for (LONG const nYEnd = tEnd.nY < a_tSrcOrigin.nY ? tEnd.nY : a_tSrcOrigin.nY; tOrigin.nY < nYEnd; ++tOrigin.nY)
	{
		for (TPixelChannel* const pEnd = a_pPixels+tStride.nX*tSize.nX; a_pPixels != pEnd; a_pPixels += tStride.nX)
			*a_pPixels = a_tDefault;
		a_pPixels += nLineDiff;
	}
	LONG const nXLen1 = tEnd.nX < a_tSrcOrigin.nX ? tEnd.nX-tOrigin.nX : (tOrigin.nX < a_tSrcOrigin.nX ? a_tSrcOrigin.nX-tOrigin.nX : 0);
	LONG const nXLen3 = tEnd.nX > tSrcEnd.nX ? tEnd.nX-tSrcEnd.nX : 0;
	LONG const nXLen2 = tSize.nX-nXLen1-nXLen3;
	TPixelChannel const* pSrc = a_pSrc+(tOrigin.nY-a_tSrcOrigin.nY)*a_nSrcStrideY+(tOrigin.nX > a_tSrcOrigin.nX ? tOrigin.nX-a_tSrcOrigin.nX : 0);
	ULONG const tSrcLineDiff = a_nSrcStrideY-nXLen2;
	for (LONG const nYEnd = tEnd.nY < tSrcEnd.nY ? tEnd.nY : tSrcEnd.nY; tOrigin.nY < nYEnd; ++tOrigin.nY)
	{
		for (TPixelChannel* const pEnd = a_pPixels+tStride.nX*nXLen1; a_pPixels != pEnd; a_pPixels += tStride.nX)
			*a_pPixels = a_tDefault;
		for (TPixelChannel* const pEnd = a_pPixels+tStride.nX*nXLen2; a_pPixels != pEnd; a_pPixels += tStride.nX, ++pSrc)
			*a_pPixels = *pSrc;
		for (TPixelChannel* const pEnd = a_pPixels+tStride.nX*nXLen3; a_pPixels != pEnd; a_pPixels += tStride.nX)
			*a_pPixels = a_tDefault;
		a_pPixels += nLineDiff;
		pSrc += tSrcLineDiff;
	}
	for (; tOrigin.nY < tEnd.nY; ++tOrigin.nY)
	{
		for (TPixelChannel* const pEnd = a_pPixels+tStride.nX*tSize.nX; a_pPixels != pEnd; a_pPixels += tStride.nX)
			*a_pPixels = a_tDefault;
		a_pPixels += nLineDiff;
	}
	return S_OK;
}

inline HRESULT RGBAInspectImpl(TImagePoint const& a_tSrcOrigin, TImageSize const& a_tSrcSize, TPixelChannel const* a_pSrc, ULONG a_nSrcStrideY, TPixelChannel const a_tDefault, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl)
{
	if (a_pOrigin == NULL || a_pSize == NULL)
	{
		a_pOrigin = &a_tSrcOrigin;
		a_pSize = &a_tSrcSize;
	}
	TImageTile aTiles[5];
	ULONG nTiles = 0;
	TImagePoint const tSrc1 = {a_pOrigin->nX < a_tSrcOrigin.nX ? a_tSrcOrigin.nX : a_pOrigin->nX, a_pOrigin->nY < a_tSrcOrigin.nY ? a_tSrcOrigin.nY : a_pOrigin->nY};
	TImagePoint const tSrc2 = {LONG(a_pOrigin->nX+a_pSize->nX) > LONG(a_tSrcOrigin.nX+a_tSrcSize.nX) ? a_tSrcOrigin.nX+a_tSrcSize.nX : a_pOrigin->nX+a_pSize->nX, LONG(a_pOrigin->nY+a_pSize->nY) > LONG(a_tSrcOrigin.nY+a_tSrcSize.nY) ? a_tSrcOrigin.nY+a_tSrcSize.nY : a_pOrigin->nY+a_pSize->nY};
	if (tSrc1.nX < tSrc2.nX && tSrc1.nY < tSrc2.nY)
	{
		if (a_pOrigin->nY < tSrc1.nY)
		{
			// top
			aTiles[nTiles].nChannelIDs = EICIRGBA;
			aTiles[nTiles].tOrigin = *a_pOrigin;
			aTiles[nTiles].tSize.nX = a_pSize->nX;
			aTiles[nTiles].tSize.nY = tSrc1.nY-a_pOrigin->nY;
			aTiles[nTiles].tStride.nX = aTiles[nTiles].tStride.nY = 0;
			aTiles[nTiles].nPixels = 1;
			aTiles[nTiles].pData = &a_tDefault;
			++nTiles;
		}
		if (a_pOrigin->nX < tSrc1.nX)
		{
			// left
			aTiles[nTiles].nChannelIDs = EICIRGBA;
			aTiles[nTiles].tOrigin.nX = a_pOrigin->nX;
			aTiles[nTiles].tOrigin.nY = tSrc1.nY;
			aTiles[nTiles].tSize.nX = tSrc1.nX-a_pOrigin->nX;
			aTiles[nTiles].tSize.nY = tSrc2.nY-tSrc1.nY;
			aTiles[nTiles].tStride.nX = aTiles[nTiles].tStride.nY = 0;
			aTiles[nTiles].nPixels = 1;
			aTiles[nTiles].pData = &a_tDefault;
			++nTiles;
		}
		{
			// center = content
			aTiles[nTiles].nChannelIDs = EICIRGBA;
			aTiles[nTiles].tOrigin = tSrc1;
			aTiles[nTiles].tSize.nX = tSrc2.nX-tSrc1.nX;
			aTiles[nTiles].tSize.nY = tSrc2.nY-tSrc1.nY;
			aTiles[nTiles].tStride.nX = 1;
			aTiles[nTiles].tStride.nY = a_nSrcStrideY;
			aTiles[nTiles].nPixels = aTiles[nTiles].tSize.nX*aTiles[nTiles].tSize.nY;
			aTiles[nTiles].pData = a_pSrc+(tSrc1.nY-a_tSrcOrigin.nY)*a_nSrcStrideY+tSrc1.nX-a_tSrcOrigin.nX;
			++nTiles;
		}
		if (LONG(a_pOrigin->nX+a_pSize->nX) > tSrc2.nX)
		{
			// right
			aTiles[nTiles].nChannelIDs = EICIRGBA;
			aTiles[nTiles].tOrigin.nX = tSrc2.nX;
			aTiles[nTiles].tOrigin.nY = tSrc1.nY;
			aTiles[nTiles].tSize.nX = a_pOrigin->nX+a_pSize->nX-tSrc2.nX;
			aTiles[nTiles].tSize.nY = tSrc2.nY-tSrc1.nY;
			aTiles[nTiles].tStride.nX = aTiles[nTiles].tStride.nY = 0;
			aTiles[nTiles].nPixels = 1;
			aTiles[nTiles].pData = &a_tDefault;
			++nTiles;
		}
		if (LONG(a_pOrigin->nY+a_pSize->nY) > tSrc2.nY)
		{
			// bottom
			aTiles[nTiles].nChannelIDs = EICIRGBA;
			aTiles[nTiles].tOrigin.nX = a_pOrigin->nX;
			aTiles[nTiles].tOrigin.nY = tSrc2.nY;
			aTiles[nTiles].tSize.nX = a_pSize->nX;
			aTiles[nTiles].tSize.nY = a_pOrigin->nY+a_pSize->nY-tSrc2.nY;
			aTiles[nTiles].tStride.nX = aTiles[nTiles].tStride.nY = 0;
			aTiles[nTiles].nPixels = 1;
			aTiles[nTiles].pData = &a_tDefault;
			++nTiles;
		}
	}
	else
	{
		// out of image
		aTiles[nTiles].nChannelIDs = EICIRGBA;
		aTiles[nTiles].tOrigin = *a_pOrigin;
		aTiles[nTiles].tSize = *a_pSize;
		aTiles[nTiles].tStride.nX = aTiles[nTiles].tStride.nY = 0;
		aTiles[nTiles].nPixels = 1;
		aTiles[nTiles].pData = &a_tDefault;
		++nTiles;
	}
	return a_pVisitor->Visit(nTiles, aTiles, a_pControl);
}

class ATL_NO_VTABLE CTileCopyTask :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IThreadedTask
{
public:
	void Init(TImageSize a_tSrcCanvas, TImagePoint a_tSrcOrigin, TImageSize a_tSrcSize, TPixelChannel const* a_pSrc, ULONG a_nSrcStrideY, TPixelChannel const a_tDefault, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, TPixelChannel* a_pData)
	{
		tSrcCanvas = a_tSrcCanvas;
		tSrcOrigin = a_tSrcOrigin;
		tSrcSize = a_tSrcSize;
		pSrc = a_pSrc;
		nSrcStrideY = a_nSrcStrideY;
		tDefault = a_tDefault;

		tOrigin = *a_pOrigin;
		tSize = *a_pSize;
		if (a_pStride)
		{
			tStride = *a_pStride;
		}
		else
		{
			tStride.nX = 1;
			tStride.nY = tSize.nX;
		}
		pData = a_pData;
	}

BEGIN_COM_MAP(CTileCopyTask)
	COM_INTERFACE_ENTRY(IThreadedTask)
END_COM_MAP()

	STDMETHOD(Execute)(ULONG a_nIndex, ULONG a_nTotal)
	{
		if (a_nIndex > tSize.nY)
			return S_FALSE;
		if (a_nTotal > tSize.nY)
			a_nTotal = tSize.nY;
		ULONG nY0 = a_nIndex*tSize.nY/a_nTotal;
		ULONG nY1 = (a_nIndex+1)*tSize.nY/a_nTotal;
		TImageSize tSubSize = tSize;
		tSubSize.nY = nY1-nY0;
		TImagePoint tSubOrigin = tOrigin;
		tSubOrigin.nY += nY0;
		TPixelChannel* pSubData = pData+tStride.nY*nY0;
		ULONG nPixels = tSize.nY*tStride.nY;
		return RGBAGetTileImpl(tSrcCanvas, tSrcOrigin, tSrcSize, pSrc, nSrcStrideY, tDefault, &tSubOrigin, &tSubSize, &tStride, nPixels, pSubData);
	}

private:
	TImageSize tSrcCanvas;
	TImagePoint tSrcOrigin;
	TImageSize tSrcSize;
	TPixelChannel const* pSrc;
	ULONG nSrcStrideY;
	TPixelChannel tDefault;

	TImagePoint tOrigin;
	TImageSize tSize;
	TImageStride tStride;
	TPixelChannel* pData;
};

inline HRESULT RGBAGetTileImpl(IThreadPool* a_pThPool, TImageSize const a_tSrcCanvasSize, TImagePoint const a_tSrcOrigin, TImageSize const a_tSrcSize, TPixelChannel const* a_pSrc, ULONG a_nSrcStrideY, TPixelChannel const a_tDefault, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pPixels)
{
	if (a_pThPool && a_nPixels > 1024*1024 && a_pOrigin && a_pSize && a_pSize->nY >= 16) // only use threads for larger images
	{
		CComObjectStackEx<CTileCopyTask> cCopyTile;
		ATLASSERT(a_nPixels >= a_pSize->nY*(a_pStride ? a_pStride->nY : a_pSize->nX));
		cCopyTile.Init(a_tSrcCanvasSize, a_tSrcOrigin, a_tSrcSize, a_pSrc, a_nSrcStrideY, a_tDefault, a_pOrigin, a_pSize, a_pStride, a_pPixels);
		return a_pThPool->Execute(0, &cCopyTile);
	}
	return RGBAGetTileImpl(a_tSrcCanvasSize, a_tSrcOrigin, a_tSrcSize, a_pSrc, a_nSrcStrideY, a_tDefault, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels);
}