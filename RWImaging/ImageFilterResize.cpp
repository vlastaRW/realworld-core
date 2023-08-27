// ImageFilterResize.cpp : Implementation of CImageFilterResize

#include "stdafx.h"
/*
#include "RWImaging.h"

#include "ImageFilterResize.h"


// CImageFilterResize

STDMETHODIMP CImageFilterResize::FormatGet(TImageFormat* a_ptImageFormat)
{
	CHECKPOINTER(a_ptImageFormat);

	*a_ptImageFormat = m_cIFOut;

	return S_OK;
}

STDMETHODIMP CImageFilterResize::DataGet(IImageSink* a_pImageSink)
{
	ATLASSERT(m_pSource.p);
	CHECKPOINTER(a_pImageSink);

	HRESULT hRes;

	BYTE* pOut;
	if (FAILED(hRes = a_pImageSink->BufferLock(0, m_cIFOut.GetDimSize(4), &pOut)))
		return hRes;

	CImageFormat cIFIn;
	m_pSource->FormatGet(cIFIn);
	cIFIn.Normalize();
	CComPtr<IImageSourcePull> pSource;
	if (FAILED(hRes = m_pSource->PullISGet(&pSource)))
	{
		a_pImageSink->BufferUnlock(m_cIFOut.GetDimSize(4), pOut);
		return hRes;
	}

	BYTE const* pIn = NULL;
	if (FAILED(hRes = pSource->BufferLock(0, cIFIn.GetDimSize(4), &pIn)))
	{
		a_pImageSink->BufferUnlock(m_cIFOut.GetDimSize(4), pOut);
		return hRes;
	}

	// simple "nearest pixel" resizing
	ULONG d3;
	for (d3 = 0; d3 < m_cIFOut->atDims[3].nItems; d3++)
	{
		BYTE* const pOut3 = pOut + d3*m_cIFOut.GetDimSize(3);
		BYTE const* const pIn3 = pIn + (d3*cIFIn->atDims[3].nItems/m_cIFOut->atDims[3].nItems)*cIFIn.GetDimSize(3);

		ULONG d2;
		for (d2 = 0; d2 < m_cIFOut->atDims[2].nItems; d2++)
		{
			BYTE* const pOut2 = pOut3 + d2*m_cIFOut.GetDimSize(2);
			BYTE const* const pIn2 = pIn3 + (d2*cIFIn->atDims[2].nItems/m_cIFOut->atDims[2].nItems)*cIFIn.GetDimSize(2);

			ULONG d1;
			for (d1 = 0; d1 < m_cIFOut->atDims[1].nItems; d1++)
			{
				BYTE* const pOut1 = pOut2 + d1*m_cIFOut.GetDimSize(1);
				BYTE const* const pIn1 = pIn2 + (d1*cIFIn->atDims[1].nItems/m_cIFOut->atDims[1].nItems)*cIFIn.GetDimSize(1);

				ULONG d0;
				for (d0 = 0; d0 < m_cIFOut->atDims[0].nItems; d0++)
				{
					BYTE* const pOut0 = pOut1 + d0*m_cIFOut.GetDimSize(0);
					BYTE const* const pIn0 = pIn1 + (d0*cIFIn->atDims[0].nItems/m_cIFOut->atDims[0].nItems)*cIFIn.GetDimSize(0);
					CopyMemory(pOut0, pIn0, m_cIFOut.GetDimSize(0));
				}
			}
		}
	}

	pSource->BufferUnlock(cIFIn.GetDimSize(4), pIn);
	a_pImageSink->BufferUnlock(m_cIFOut.GetDimSize(4), pOut);

	return S_OK;
}


*/