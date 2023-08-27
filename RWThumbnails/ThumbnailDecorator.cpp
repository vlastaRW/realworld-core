// ThumbnailDecorator.cpp : Implementation of CThumbnailDecorator

#include "stdafx.h"
#include "ThumbnailDecorator.h"


// CThumbnailDecorator

STDMETHODIMP CThumbnailDecorator::Init(IThumbnailRenderer* a_pInternalRenderer, ULONG a_nShadowPixels, BOOL a_bChequeredBackground)
{
	m_pInternal = a_pInternalRenderer;
	m_nShadowPixels = a_nShadowPixels;
	m_bChequeredBackground = a_bChequeredBackground;
	return S_OK;
}

void BlendWithBackground(ULONG a_nSrcX, ULONG a_nSrcY, ULONG a_nSrcStride, DWORD const* a_pSrc, ULONG a_nDstX, ULONG a_nDstY, ULONG a_nDstStride, DWORD* a_pDst)
{
}

STDMETHODIMP CThumbnailDecorator::GetThumbnail(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo)
{
	try
	{
		ULONG nShadowPixels = (a_nSizeX <= m_nShadowPixels*2 || a_nSizeY <= m_nShadowPixels*2) ? 0 : m_nShadowPixels;
		if (nShadowPixels == 0 && !m_bChequeredBackground)
			return m_pInternal->GetThumbnail(a_pFile, a_nSizeX, a_nSizeY, a_pBGRAData, a_prcBounds, a_tLocaleID, a_pbstrInfo);
		if (a_nSizeX == 0 || a_nSizeY == 0)
			return E_RW_INVALIDPARAM;
		if (nShadowPixels == 0)
		{
			RECT rcBounds;
			HRESULT hRes = m_pInternal->GetThumbnail(a_pFile, a_nSizeX, a_nSizeY, a_pBGRAData, &rcBounds, a_tLocaleID, a_pbstrInfo);
			if (a_prcBounds) *a_prcBounds = rcBounds;
			if (FAILED(hRes))
				return hRes;
			if (m_bChequeredBackground)
				BlendWithBackground(a_nSizeX, a_nSizeY, a_nSizeX, a_pBGRAData, a_nSizeX, a_nSizeY, a_nSizeX, a_pBGRAData);
			return hRes;
		}
		else
		{
			ULONG nSizeX = a_nSizeX-m_nShadowPixels*2;
			ULONG nSizeY = a_nSizeY-m_nShadowPixels*2;
			CAutoVectorPtr<DWORD> cData(new DWORD[nSizeX*nSizeY]);
			RECT rcBounds;
			HRESULT hRes = m_pInternal->GetThumbnail(a_pFile, nSizeX, nSizeY, cData, &rcBounds, a_tLocaleID, a_pbstrInfo);
			if (a_prcBounds)
			{
				a_prcBounds->left = rcBounds.left;
				a_prcBounds->top = rcBounds.top;
				a_prcBounds->right = rcBounds.right+m_nShadowPixels*2;
				a_prcBounds->bottom = rcBounds.bottom+m_nShadowPixels*2;
			}
			if (FAILED(hRes))
				return hRes;
			if (m_bChequeredBackground)
				BlendWithBackground(a_nSizeX, a_nSizeY, a_nSizeX, a_pBGRAData, a_nSizeX, a_nSizeY, a_nSizeX, a_pBGRAData);
			else
			{
				// TODO: copy rectangle
			}
			// TODO: render shadow
			return hRes;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

