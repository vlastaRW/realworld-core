// ColorQuantizer.cpp : Implementation of CColorQuantizer

#include "stdafx.h"
#include "ColorQuantizer.h"


// CColorQuantizer

STDMETHODIMP CColorQuantizer::Init(ULONG a_nColors, EColorDithering a_eDither, BYTE a_bAlphaLimit, TPixelChannel const* a_pBackground, TImageSize const* a_pCanvasSize)
{
	m_nColors = a_nColors;
	m_eDither = a_eDither;
	m_bAlphaLimit = a_bAlphaLimit;
	if (a_pBackground)
	{
		m_bBlend = true;
		m_tBackground = *a_pBackground;
	}
	else
	{
		m_bBlend = false;
	}
	m_eState = ESInitialized;
	m_aPal.Free();
	m_nPal = 0;
	return S_OK;
}

STDMETHODIMP CColorQuantizer::Visit(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl* a_pControl)
{
	if (m_eState != ESInitialized)
		return E_FAIL;

	for (TImageTile const* const pEnd = a_aTiles+a_nTiles; a_aTiles != pEnd; ++a_aTiles)
	{
		if (a_aTiles->tStride.nX == 0 && a_aTiles->tStride.nY == 0)
		{
			InsertPixel(*a_aTiles->pData, a_aTiles->tSize.nX*a_aTiles->tSize.nY);
			continue;
		}
		for (ULONG y = 0; y < a_aTiles->tSize.nY; ++y)
		{
			TPixelChannel const* p = a_aTiles->pData+y*a_aTiles->tStride.nY;
			for (TPixelChannel const* pEnd = p+a_aTiles->tSize.nX*a_aTiles->tStride.nX; p != pEnd; p += a_aTiles->tStride.nX)
			{
				InsertPixel(*p);
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CColorQuantizer::Colors(IEnum2UInts* a_pColors)
{
	if (m_eState == ESRaw)
		return E_FAIL;
	if (m_eState == ESInitialized)
		ComputePalette();
	if (m_nPal < 2)
		return E_FAIL;
	return a_pColors->Consume(0, m_nPal, reinterpret_cast<ULONG*>(m_aPal.m_p));
}

STDMETHODIMP CColorQuantizer::Process(TImagePoint const* a_pOrigin, TImageSize a_tSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl)
{
	if (m_eState == ESRaw)
		return E_FAIL;
	if (m_eState == ESInitialized)
		ComputePalette();
	if (m_nPal < 2)
		return E_FAIL;
	if (a_pStride && (a_pStride->nX != 1 || a_pStride->nY != a_tSize.nX))
		return E_RW_INVALIDPARAM;
	DirectToIndexed(a_pData, m_aPal, m_aPal, m_nPal, a_tSize.nX, a_tSize.nY, m_eDither != ECDNoDithering, a_pData);
	return S_OK;
}

#include <GammaCorrection.h>

void CColorQuantizer::ComputePalette()
{
	if (!m_aPal.Allocate(m_nColors))
		return;
	m_nPal = GetPalette(m_aPal, m_nColors);
	m_eState = ESReady;
}

int CColorQuantizer::GetPalette(RGBQUAD* a_pPalEntries, int a_nPalEntries)
{
	if (m_sRoot.m_nPixels == 0 || a_nPalEntries <= 0)
		return 0;

	// compute errors
	m_sRoot.PrepareNode();

	CAutoVectorPtr<SNode const*> aBuffer(new SNode const*[a_nPalEntries]);
	aBuffer[0] = &m_sRoot;
	int nOccupied = 1;
	while (true)
	{
		int i;
		for (i = 0; i < nOccupied; i++)
		{
			int nChildren = aBuffer[i]->GetChildCount();
			if (aBuffer[i]->m_nPixels > 1 && nChildren > 0 && nChildren <= (a_nPalEntries-nOccupied+1))
				break;
		}
		if (i < nOccupied)
		{
			SNode const* pNode = aBuffer[i];
			// delete the node from buffer
			memcpy(aBuffer+i, aBuffer+i+1, sizeof(SNode const*)*(nOccupied-i-1));
			// generate sequence of new nodes
			SNode const* aSubNodes[8];
			int nSubNodes = 0;
			for (int j = 0; j < 8; j++)
			{
				if (pNode->m_aSubNodes[j])
				{
					aSubNodes[nSubNodes] = pNode->m_aSubNodes[j];
					int nChildren = aSubNodes[nSubNodes]->GetChildCount();
					while (nChildren == 1 && aSubNodes[nSubNodes]->m_nPixels)
					{
						for (int k = 0; k < 8; k++)
						{
							if (aSubNodes[nSubNodes]->m_aSubNodes[k])
							{
								aSubNodes[nSubNodes] = aSubNodes[nSubNodes]->m_aSubNodes[k];
								break;
							}
						}
						nChildren = aSubNodes[nSubNodes]->GetChildCount();
					}
					nSubNodes++;
				}
			}
			// short bubble-sort
			for (int i1 = 1; i1 < nSubNodes; i1++)
			{
				for (int i2 = nSubNodes-1; i2 >= i1; i2--)
				{
					if (aSubNodes[i2]->Weight() > aSubNodes[i2-1]->Weight())
					{
						SNode const* pTmp = aSubNodes[i2];
						aSubNodes[i2] = aSubNodes[i2-1];
						aSubNodes[i2-1] = pTmp;
					}
				}
			}
			// merge the sequences
			int iOldLast = nOccupied-2;
			int iNewLast = nSubNodes-1;
			nOccupied += nSubNodes-1;
			for (int iDstLast = nOccupied-1; iDstLast >= 0; iDstLast--) // the condition is acually redundant (one of the breaks will do the job)
			{
				if (iOldLast < 0)
				{
					// copy rest of new sequence to the destination
					memcpy(aBuffer, aSubNodes, sizeof(SNode const*)*(iNewLast+1));
					break;
				}
				else if (iNewLast < 0)
				{
					// nothing to do (old sequence is already in place)
					break;
				}
				else if (aBuffer[iOldLast]->Weight() <= aSubNodes[iNewLast]->Weight())
				{
					aBuffer[iDstLast] = aBuffer[iOldLast--];
				}
				else
				{
					aBuffer[iDstLast] = aSubNodes[iNewLast--];
				}
			}
		}
		else
		{
			break;
		}
	}

	m_nLastTimeStamp++;

	// generate the palette from the buffer
	for (int iEntry = 0; iEntry < nOccupied; iEntry++)
	{
		if (aBuffer[iEntry]->m_nPixels)
		{
			a_pPalEntries[iEntry].rgbRed = static_cast<BYTE>(aBuffer[iEntry]->m_nR / aBuffer[iEntry]->m_nPixels);
			a_pPalEntries[iEntry].rgbGreen = static_cast<BYTE>(aBuffer[iEntry]->m_nG / aBuffer[iEntry]->m_nPixels);
			a_pPalEntries[iEntry].rgbBlue = static_cast<BYTE>(aBuffer[iEntry]->m_nB / aBuffer[iEntry]->m_nPixels);
		}
		else
		{
			a_pPalEntries[iEntry].rgbRed = a_pPalEntries[iEntry].rgbGreen = a_pPalEntries[iEntry].rgbBlue = 0;
		}
		a_pPalEntries[iEntry].rgbReserved = 0;
		aBuffer[iEntry]->m_iIndex = iEntry;
		aBuffer[iEntry]->m_nTimeStamp = m_nLastTimeStamp;
	}

	return nOccupied;
}

int CColorQuantizer::FindColorIndex(TPixelChannel a_tPixel) const // may only be called AFTER GetPalette
{
	int nIndR = static_cast<int>(a_tPixel.bR)<<3;
	int nIndG = static_cast<int>(a_tPixel.bG)<<2;
	int nIndB = static_cast<int>(a_tPixel.bB)<<1;
	SNode const* pNode = &m_sRoot;
	int iLevel;
	for (iLevel = 0; pNode->m_nTimeStamp != m_nLastTimeStamp; iLevel++)
	{
		int iSubNode =
			((nIndR&(1<<(t_nBits+2-iLevel))) |
			 (nIndG&(1<<(t_nBits+1-iLevel))) |
			 (nIndB&(1<<(t_nBits-iLevel))))>>(t_nBits-iLevel);
		if (pNode->m_aSubNodes[iSubNode] == NULL)
		{
			ATLASSERT(0); // incorrect palette???
			return -1;
		}
		pNode = pNode->m_aSubNodes[iSubNode];
	}
	return pNode->m_iIndex;
}

void CColorQuantizer::DirectToIndexed(TPixelChannel const* a_pSrc, RGBQUAD const* a_pPalette, RGBQUAD const* a_pPalette2, int a_nPalColors, int a_nSizeX, int a_nSizeY, bool a_bDither, TPixelChannel* a_pDst) const
{
	if (!a_bDither)
	{
		int i;
		for (i = 0; i < (a_nSizeX * a_nSizeY); ++i, ++a_pDst)
		{
			if (a_pSrc[i].bA)
			{
				TPixelChannel tP = a_pSrc[i];
				int iC = BestColor(a_pPalette, a_nPalColors, tP.bR, tP.bG, tP.bB);
				a_pDst->bR = a_pPalette2[iC].rgbRed;
				a_pDst->bG = a_pPalette2[iC].rgbGreen;
				a_pDst->bB = a_pPalette2[iC].rgbBlue;
			}
			else
			{
				a_pDst->n = 0;
			}
		}
	}
	else
	{
		static const int FS_SHIFT = 10;
		a_pDst += -a_nSizeX-1;

		CAutoVectorPtr<int> aErrs(new int[(2+a_nSizeX)*6]);
		int* thisrerr = aErrs;
		int* nextrerr = thisrerr+2+a_nSizeX;
		int* thisgerr = nextrerr+2+a_nSizeX;
		int* nextgerr = thisgerr+2+a_nSizeX;
		int* thisberr = nextgerr+2+a_nSizeX;
		int* nextberr = thisberr+2+a_nSizeX;
		TRandomMotherOfAll cRnd(42);
		for (int x = 0; x < 2+a_nSizeX; ++x)
		{
			// (random errors in [-1 .. 1])
			thisrerr[x] = cRnd.IRandom(-(1<<FS_SHIFT), 1<<FS_SHIFT);
			thisgerr[x] = cRnd.IRandom(-(1<<FS_SHIFT), 1<<FS_SHIFT);
			thisberr[x] = cRnd.IRandom(-(1<<FS_SHIFT), 1<<FS_SHIFT);
		}
		bool fs_direction = true;

		for (int y = 0; y < a_nSizeY; ++y)
		{
			for (int x = 0; x < 2+a_nSizeX; ++x)
				nextrerr[x] = nextgerr[x] = nextberr[x] = 0;

			TPixelChannel const* pP;
			int actX;
			int lastX;
			if (fs_direction)
			{
				actX = 0;
				lastX = a_nSizeX;
				pP = a_pSrc + y*a_nSizeX;
				a_pDst += a_nSizeX+1;
			}
			else
			{
				actX = a_nSizeX - 1;
				lastX = -1;
				pP = a_pSrc + y*a_nSizeX+actX;
				a_pDst += actX;
			}
			do
			{
				if (pP->bA == 0)
				{
					a_pDst->n = 0;
				}
				else
				{
					int ind;
					TPixelChannel tP = *pP;
					/* Use Floyd-Steinberg errors to adjust actual color. */
					int sr = tP.bR + ((thisrerr[actX + 1] + (1<<(FS_SHIFT-1))) >> FS_SHIFT);
					int sg = tP.bG + ((thisgerr[actX + 1] + (1<<(FS_SHIFT-1))) >> FS_SHIFT);
					int sb = tP.bB + ((thisberr[actX + 1] + (1<<(FS_SHIFT-1))) >> FS_SHIFT);
					//if (sr < 0) sr = 0;
					//else if (sr > 255) sr = 255;
					//if (sg < 0) sg = 0;
					//else if (sg > 255) sg = 255;
					//if (sb < 0) sb = 0;
					//else if (sb > 255) sb = 255;

					int dist = (sr-a_pPalette[0].rgbRed)*(sr-a_pPalette[0].rgbRed) + (sg-a_pPalette[0].rgbGreen)*(sg-a_pPalette[0].rgbGreen) + (sb-a_pPalette[0].rgbBlue)*(sb-a_pPalette[0].rgbBlue);
					ind = 0;
					for (int i = 1; i < a_nPalColors; ++i)
					{
						int r2 = a_pPalette[i].rgbRed;
						int g2 = a_pPalette[i].rgbGreen;
						int b2 = a_pPalette[i].rgbBlue;
						int newdist = (sr-r2)*(sr-r2) + (sg-g2)*(sg-g2) + (sb-b2)*(sb-b2);
						if (newdist < dist)
						{
							ind = i;
							dist = newdist;
						}
					}

					/* Propagate Floyd-Steinberg error terms. */
					if (fs_direction)
					{
						int err = thisrerr[actX + 1] + ((tP.bR - (int)a_pPalette[ind].rgbRed)<<FS_SHIFT);
						thisrerr[actX + 2] += ( err * 7 ) >> 4;
						nextrerr[actX    ] += ( err * 3 ) >> 4;
						nextrerr[actX + 1] += ( err * 5 ) >> 4;
						nextrerr[actX + 2] += ( err     ) >> 4;
						err = thisgerr[actX + 1] + ((tP.bG - (int)a_pPalette[ind].rgbGreen)<<FS_SHIFT);
						thisgerr[actX + 2] += ( err * 7 ) >> 4;
						nextgerr[actX    ] += ( err * 3 ) >> 4;
						nextgerr[actX + 1] += ( err * 5 ) >> 4;
						nextgerr[actX + 2] += ( err     ) >> 4;
						err = thisberr[actX + 1] + ((tP.bB - (int)a_pPalette[ind].rgbBlue)<<FS_SHIFT);
						thisberr[actX + 2] += ( err * 7 ) >> 4;
						nextberr[actX    ] += ( err * 3 ) >> 4;
						nextberr[actX + 1] += ( err * 5 ) >> 4;
						nextberr[actX + 2] += ( err     ) >> 4;
					}
					else
					{
						int err = thisrerr[actX + 1] + ((tP.bR - (int)a_pPalette[ind].rgbRed)<<FS_SHIFT);
						thisrerr[actX    ] += ( err * 7 ) >> 4;
						nextrerr[actX + 2] += ( err * 3 ) >> 4;
						nextrerr[actX + 1] += ( err * 5 ) >> 4;
						nextrerr[actX    ] += ( err     ) >> 4;
						err = thisgerr[actX + 1] + ((tP.bG - (int)a_pPalette[ind].rgbGreen)<<FS_SHIFT);
						thisgerr[actX    ] += ( err * 7 ) >> 4;
						nextgerr[actX + 2] += ( err * 3 ) >> 4;
						nextgerr[actX + 1] += ( err * 5 ) >> 4;
						nextgerr[actX    ] += ( err     ) >> 4;
						err = thisberr[actX + 1] + ((tP.bB - (int)a_pPalette[ind].rgbBlue)<<FS_SHIFT);
						thisberr[actX    ] += ( err * 7 ) >> 4;
						nextberr[actX + 2] += ( err * 3 ) >> 4;
						nextberr[actX + 1] += ( err * 5 ) >> 4;
						nextberr[actX    ] += ( err     ) >> 4;
					}

					int const nMaxErr = 384<<FS_SHIFT;
					// prevent excessive color bleeding
					if (thisrerr[actX    ] > nMaxErr)
						thisrerr[actX    ] = nMaxErr;
					else if (thisrerr[actX    ] < -nMaxErr)
						thisrerr[actX    ] = -nMaxErr;
					if (thisgerr[actX    ] > nMaxErr)
						thisgerr[actX    ] = nMaxErr;
					else if (thisgerr[actX    ] < -nMaxErr)
						thisgerr[actX    ] = -nMaxErr;
					if (thisberr[actX    ] > nMaxErr)
						thisberr[actX    ] = nMaxErr;
					else if (thisberr[actX    ] < -nMaxErr)
						thisberr[actX    ] = -nMaxErr;

					if (nextrerr[actX + 2] > nMaxErr)
						nextrerr[actX + 2] = nMaxErr;
					else if (nextrerr[actX + 2] < -nMaxErr)
						nextrerr[actX + 2] = -nMaxErr;
					if (nextgerr[actX + 2] > nMaxErr)
						nextgerr[actX + 2] = nMaxErr;
					else if (nextgerr[actX + 2] < -nMaxErr)
						nextgerr[actX + 2] = -nMaxErr;
					if (nextberr[actX + 2] > nMaxErr)
						nextberr[actX + 2] = nMaxErr;
					else if (nextberr[actX + 2] < -nMaxErr)
						nextberr[actX + 2] = -nMaxErr;

					if (nextrerr[actX + 1] > nMaxErr)
						nextrerr[actX + 1] = nMaxErr;
					else if (nextrerr[actX + 1] < -nMaxErr)
						nextrerr[actX + 1] = -nMaxErr;
					if (nextgerr[actX + 1] > nMaxErr)
						nextgerr[actX + 1] = nMaxErr;
					else if (nextgerr[actX + 1] < -nMaxErr)
						nextgerr[actX + 1] = -nMaxErr;
					if (nextberr[actX + 1] > nMaxErr)
						nextberr[actX + 1] = nMaxErr;
					else if (nextberr[actX + 1] < -nMaxErr)
						nextberr[actX + 1] = -nMaxErr;

					if (nextrerr[actX    ] > nMaxErr)
						nextrerr[actX    ] = nMaxErr;
					else if (nextrerr[actX    ] < -nMaxErr)
						nextrerr[actX    ] = -nMaxErr;
					if (nextgerr[actX    ] > nMaxErr)
						nextgerr[actX    ] = nMaxErr;
					else if (nextgerr[actX    ] < -nMaxErr)
						nextgerr[actX    ] = -nMaxErr;
					if (nextberr[actX    ] > nMaxErr)
						nextberr[actX    ] = nMaxErr;
					else if (nextberr[actX    ] < -nMaxErr)
						nextberr[actX    ] = -nMaxErr;

					a_pDst->bR = a_pPalette2[ind].rgbRed;
					a_pDst->bG = a_pPalette2[ind].rgbGreen;
					a_pDst->bB = a_pPalette2[ind].rgbBlue;
				}

				if (fs_direction)
				{
					++actX;
					++pP;
					++a_pDst;
				}
				else
				{
					--actX;
					--pP;
					--a_pDst;
				}
			}
			while (actX != lastX);

			int* temperr = thisrerr;
			thisrerr = nextrerr;
			nextrerr = temperr;
			temperr = thisgerr;
			thisgerr = nextgerr;
			nextgerr = temperr;
			temperr = thisberr;
			thisberr = nextberr;
			nextberr = temperr;
			fs_direction = ! fs_direction;

		}
	}
}

