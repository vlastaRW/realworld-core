// PixelFormatConverter.cpp : Implementation of CPixelFormatConverter

#include "stdafx.h"
#include "RWImaging.h"
#include "PixelFormatConverter.h"
#include "PixelPackConverter.h"

typedef void (__stdcall ConvertRow)(CPixelPackConverter const* a_pConv, BYTE const* a_pSrc, BYTE* a_pDst, ULONG a_nItems);

void __stdcall ConvertRowSwapR8G8B8(CPixelPackConverter const* UNREF(a_pConv), BYTE const* a_pSrc, BYTE* a_pDst, ULONG a_nItems)
{
	BYTE* const pLastDst = a_pDst+a_nItems+a_nItems+a_nItems;
	while (a_pDst < pLastDst)
	{
		a_pDst[0] = a_pSrc[2];
		a_pDst[1] = a_pSrc[1];
		a_pDst[2] = a_pSrc[0];
		a_pDst += 3;
		a_pSrc += 3;
	}
}

void __stdcall ConvertRowSwapR8G8B8A8(CPixelPackConverter const* UNREF(a_pConv), BYTE const* a_pSrc, BYTE* a_pDst, ULONG a_nItems)
{
	BYTE* const pLastDst = a_pDst+(a_nItems<<2);
	while (a_pDst < pLastDst)
	{
		DWORD const dw = *reinterpret_cast<DWORD const*>(a_pSrc);
		*reinterpret_cast<DWORD*>(a_pDst) = (dw&0xff00ff00)|((dw&0xff)<<16)|((dw>>16)&0xff);
		a_pDst += 4;
		a_pSrc += 4;
	}
}

// CPixelFormatConverter

STDMETHODIMP CPixelFormatConverter::Convert(TPixelFormat const* a_ptInputFormat, ULONG a_nInputPixels, BYTE const* a_aInputPixels, TPixelFormat const* a_ptOutputFormat, ULONG a_nOutputPixels, BYTE* a_aOutputPixels)
{
	try
	{
		if (a_ptInputFormat->nSizeX != a_ptOutputFormat->nSizeX || a_ptInputFormat->nSizeY != a_ptOutputFormat->nSizeY)
			return E_RW_INVALIDPARAM;

		CPixelPackConverter cConv(a_ptInputFormat->nStrideX<<3, a_ptOutputFormat->nStrideX<<3);
		ConvertRow* pfnConvertRow = CPixelPackConverter::DefaultConveter;
		BYTE* pToDelete = NULL;

		if (a_ptInputFormat->nStrideX == a_ptOutputFormat->nStrideX &&
			(a_ptInputFormat->nStrideX == 3 || a_ptInputFormat->nStrideX == 4) && 
			a_ptInputFormat->tRed.nWidth == 8 && a_ptInputFormat->tGreen.nWidth == 8 &&
			a_ptInputFormat->tBlue.nWidth == 8 &&
			a_ptOutputFormat->tRed.nWidth == 8 && a_ptOutputFormat->tGreen.nWidth == 8 &&
			a_ptOutputFormat->tBlue.nWidth == 8 &&
			a_ptInputFormat->tGreen.nOffset == 8 && a_ptOutputFormat->tGreen.nOffset == 8 &&
			a_ptInputFormat->tRed.nOffset == a_ptOutputFormat->tBlue.nOffset &&
			a_ptInputFormat->tBlue.nOffset == a_ptOutputFormat->tRed.nOffset)
		{
			// special case RGB(A)<->BGR(A)
			pfnConvertRow = a_ptInputFormat->nStrideX == 3 ? ConvertRowSwapR8G8B8 : ConvertRowSwapR8G8B8A8;
		}
		else
		{
			if (a_ptOutputFormat->tRed.nWidth && a_ptInputFormat->tRed.nWidth)
				cConv.AddChannel(a_ptInputFormat->tRed, a_ptOutputFormat->tRed);
			if (a_ptOutputFormat->tGreen.nWidth && a_ptInputFormat->tGreen.nWidth)
				cConv.AddChannel(a_ptInputFormat->tGreen, a_ptOutputFormat->tGreen);
			if (a_ptOutputFormat->tBlue.nWidth && a_ptInputFormat->tBlue.nWidth)
				cConv.AddChannel(a_ptInputFormat->tBlue, a_ptOutputFormat->tBlue);
			if (a_ptOutputFormat->tAlpha.nWidth && a_ptInputFormat->tAlpha.nWidth)
				cConv.AddChannel(a_ptInputFormat->tAlpha, a_ptOutputFormat->tAlpha);

			if (a_ptInputFormat->nStrideX > 0 && a_ptOutputFormat->nStrideX > 0 &&
				a_ptInputFormat->nStrideX <= 4 && a_ptOutputFormat->nStrideX <= 4)
			{
#ifndef WIN64
				pToDelete = cConv.CreateConverter();
				pfnConvertRow = (ConvertRow*)pToDelete;
#else
				pfnConvertRow = CPixelPackConverter::OptimizedConveter;
#endif
			}
		}

		for (ULONG y = 0; y < a_ptInputFormat->nSizeY; ++y)
		{
			pfnConvertRow(&cConv, a_aInputPixels+(LONG(y)*a_ptInputFormat->nStrideY), a_aOutputPixels+(LONG(y)*a_ptOutputFormat->nStrideY), a_ptInputFormat->nSizeX);
		}
		CPixelPackConverter::DeleteConverter(pToDelete);

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
