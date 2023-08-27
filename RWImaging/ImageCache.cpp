// ImageCache.cpp : Implementation of CImageCache

#include "stdafx.h"
#include "ImageCache.h"
#include "RWImagingDocumentUtils.h"


// CImageCache

STDMETHODIMP CImageCache::Create(BSTR UNREF(a_bstrPrefix), IDocumentBase* UNREF(a_pBase), TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile)
{
	try
	{
		if (a_nChannels != 1 && a_aChannelDefaults[0].eID != EICIRGBA)
			return E_INVALIDARG;
		m_tDefault = a_aChannelDefaults[0].tValue;
		m_tCanvasSize = *a_pSize;
		if (a_pResolution)
			m_tResolution = *a_pResolution;
		ULONG const nSize = m_tCanvasSize.nX*m_tCanvasSize.nY;
		if (m_tCanvasSize.nX*m_tCanvasSize.nY == 0)
			return E_INVALIDARG;
		if (a_fGamma != 0.0f) m_fGamma = a_fGamma;
		delete[] m_pData;
		if (a_pTile && a_pTile->nChannelIDs == EICIRGBA)
		{
			m_pData = new TPixelChannel[a_pTile->tSize.nX*a_pTile->tSize.nY];
			m_tContentOrigin = a_pTile->tOrigin;
			m_tContentSize = a_pTile->tSize;
			if (a_pTile->tStride.nX == 1)
			{
				if (a_pTile->tStride.nY == a_pTile->tSize.nX)
					CopyMemory(m_pData, a_pTile->pData, a_pTile->tSize.nX*a_pTile->tSize.nY*sizeof*a_pTile->pData);
				else
					for (ULONG y = 0; y < a_pTile->tSize.nY; ++y)
						CopyMemory(m_pData+y*m_tContentSize.nX, a_pTile->pData+y*a_pTile->tStride.nY, a_pTile->tSize.nX*sizeof*a_pTile->pData);
			}
			else
			{
				TPixelChannel* pD = m_pData;
				for (ULONG y = 0; y < a_pTile->tSize.nY; ++y)
				{
					TPixelChannel const* pS = a_pTile->pData+y*a_pTile->tStride.nY;
					for (TPixelChannel* const pEnd = pD+a_pTile->tSize.nX; pD < pEnd; ++pD, pS+=a_pTile->tStride.nX)
						*pD = *pS;
				}
			}
		}
		else
		{
			m_pData = NULL;
			m_tContentOrigin.nX = m_tContentOrigin.nY = 0;
			m_tContentSize.nX = m_tContentSize.nY = 0;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CImageCache::CanvasGet(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
{
	try
	{
		if (a_pCanvasSize) *a_pCanvasSize = m_tCanvasSize;
		if (a_pResolution) *a_pResolution = m_tResolution;
		if (a_pContentOrigin) *a_pContentOrigin = m_tContentOrigin;
		if (a_pContentSize) *a_pContentSize = m_tContentSize;
		if (a_pContentOpacity) *a_pContentOpacity = EIOUnknown;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CImageCache::ChannelsGet(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
{
	try
	{
		if (a_pChannelIDs) *a_pChannelIDs = EICIRGBA;
		if (a_pGamma) *a_pGamma = m_fGamma;
		if (a_pChannelDefaults) a_pChannelDefaults->Consume(0, 1, &CChannelDefault(EICIRGBA, m_tDefault));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CImageCache::TileGet(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* UNREF(a_pControl), EImageRenderingIntent UNREF(a_eIntent))
{
	try
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_RW_INVALIDPARAM;
		return RGBAGetTileImpl(m_tCanvasSize, m_tContentOrigin, m_tContentSize, m_pData, m_tContentSize.nX, m_tDefault, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CImageCache::Inspect(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent UNREF(a_eIntent))
{
	try
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_RW_INVALIDPARAM;
		return RGBAInspectImpl(m_tContentOrigin, m_tContentSize, m_pData, m_tContentSize.nX, m_tDefault, a_pOrigin, a_pSize, a_pVisitor, a_pControl);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CImageCache::BufferLock(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
{
	if (a_nChannelID != EICIRGBA)
		return E_RW_INVALIDPARAM;
	try
	{
		if (a_pAllocOrigin) *a_pAllocOrigin = m_tContentOrigin;
		if (a_pAllocSize) *a_pAllocSize = m_tContentSize;
		if (a_pContentOrigin) *a_pContentOrigin = m_tContentOrigin;
		if (a_pContentSize) *a_pContentSize = m_tContentSize;
		if (a_ppBuffer)
			*a_ppBuffer = m_pData;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CImageCache::BufferUnlock(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
{
	if (a_nChannelID != EICIRGBA || a_pBuffer != m_pData)
		return E_RW_INVALIDPARAM;
	return S_OK;
}

