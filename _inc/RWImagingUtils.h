#pragma once

#ifdef __cplusplus

#include <math.h>
#include <GammaCorrection.h>

struct CImageSize : public TImageSize
{
	CImageSize() { nX = nY = 0; }
	CImageSize(ULONG a_nX, ULONG a_nY) { nX = a_nX; nY = a_nY; }
	operator TImageSize const*() const { return this; }
};

struct CImagePoint : public TImagePoint
{
	CImagePoint() { nX = nY = 0; }
	CImagePoint(LONG a_nX, LONG a_nY) { nX = a_nX; nY = a_nY; }
	operator TImagePoint const*() const { return this; }
};

struct CImageStride : public TImageStride
{
	CImageStride() { nX = nY = 0; }
	CImageStride(ULONG a_nX, ULONG a_nY) { nX = a_nX; nY = a_nY; }
	operator TImageStride const*() const { return this; }
};

struct CPixelChannel : public TPixelChannel
{
	CPixelChannel() { n = 0; }
	CPixelChannel(ULONG a_n) { n = a_n; }
	CPixelChannel(float a_f) { f = a_f; }
	CPixelChannel(BYTE a_bR, BYTE a_bG, BYTE a_bB, BYTE a_bA) { bR = a_bR; bG = a_bG; bB = a_bB; bA = a_bA; }
	operator TPixelChannel const*() const { return this; }
};

struct CChannelDefault : public TChannelDefault
{
	CChannelDefault(EImageChannelID a_eID) { eID = a_eID; tValue.n = 0; }
	CChannelDefault(EImageChannelID a_eID, TPixelChannel a_t) { eID = a_eID; tValue = a_t; }
	CChannelDefault(EImageChannelID a_eID, ULONG a_n) { eID = a_eID; tValue.n = a_n; }
	CChannelDefault(EImageChannelID a_eID, float a_f) { eID = a_eID; tValue.f = a_f; }
	CChannelDefault(EImageChannelID a_eID, BYTE a_bR, BYTE a_bG, BYTE a_bB, BYTE a_bA) { eID = a_eID; tValue.bR = a_bR; tValue.bG = a_bG; tValue.bB = a_bB; tValue.bA = a_bA; }
	operator TChannelDefault const*() const { return this; }
};

struct CImageTile : public TImageTile
{
	CImageTile(ULONG a_nSizeX, ULONG a_nSizeY, TPixelChannel const* a_pData)
	{
		nChannelIDs = EICIRGBA;
		tOrigin.nX = tOrigin.nY = 0;
		tSize.nX = a_nSizeX;
		tSize.nY = a_nSizeY;
		tStride.nX = 1;
		tStride.nY = a_nSizeX;
		nPixels = a_nSizeX*a_nSizeY;
		pData = a_pData;
	}
	operator TImageTile const*() const { return this; }
};

class CNearestImageResizer : public IImageVisitor
{
	CNearestImageResizer(TPixelChannel* a_pData, ULONG a_nDstSizeX, ULONG a_nDstSizeY, ULONG a_nDstStrideX, ULONG a_nDstStrideY, ULONG a_nSrcSizeX, ULONG a_nSrcSizeY) :
		m_pData(a_pData), m_nDstSizeX(a_nDstSizeX), m_nDstSizeY(a_nDstSizeY),
		m_nDstStrideX(a_nDstStrideX), m_nDstStrideY(a_nDstStrideY), m_nSrcSizeX(a_nSrcSizeX), m_nSrcSizeY(a_nSrcSizeY)
	{
		m_nFactorX = (a_nSrcSizeX<<16)/a_nDstSizeX;
		m_nFactorY = (a_nSrcSizeY<<16)/a_nDstSizeY;
	}

public:
	inline static HRESULT GetResizedImage(IDocumentImage* a_pImage, EImageChannelID a_eChannelID, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStrideX, ULONG a_nStrideY, TPixelChannel* a_pPixels)
	{
		TImageSize tImgSize = {1, 1};
		HRESULT hRes = a_pImage->CanvasGet(&tImgSize, NULL, NULL, NULL, NULL);
		if (FAILED(hRes))
			return hRes;
		static TImagePoint const t0 = {0, 0};
		if (a_nSizeX == tImgSize.nX && a_nSizeY == tImgSize.nY)
			return a_pImage->TileGet(a_eChannelID, &t0, &tImgSize, CImageStride(a_nStrideX, a_nStrideY), a_nSizeY*a_nStrideY, a_pPixels, NULL, EIRIPreview);
		if (tImgSize.nX > 0xffff || tImgSize.nY > 0xffff || a_nSizeX > 0xffff || a_nSizeY > 0xffff)
			return E_FAIL;
		CNearestImageResizer cResizer(a_pPixels, a_nSizeX, a_nSizeY, a_nStrideX, a_nStrideY, tImgSize.nX, tImgSize.nY);
		return a_pImage->Inspect(a_eChannelID, &t0, &tImgSize, &cResizer, NULL, EIRIPreview);
	}

private:
	STDMETHOD_(ULONG, AddRef)()
	{
		return 1;
	}
	STDMETHOD_(ULONG, Release)()
	{
		return 1;
	}
	STDMETHOD(QueryInterface)(REFIID a_guidIID, void** a_ppInterface)
	{
		if (IsEqualIID(a_guidIID, IID_IUnknown) || IsEqualIID(a_guidIID, __uuidof(IImageVisitor)))
		{
			*a_ppInterface = static_cast<IImageVisitor*>(this);
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	// IImageVisitor methods
public:
	STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl* /*a_pControl*/)
	{
		for (TImageTile const* const pEnd = a_aTiles+a_nTiles; a_aTiles != pEnd; ++a_aTiles)
		{
			ULONG const nDstY1 = ((a_aTiles->tOrigin.nY<<16)+m_nFactorY-1)/m_nFactorY;
			ULONG const nDstY2 = MyMin((((a_aTiles->tOrigin.nY+a_aTiles->tSize.nY)<<16)+m_nFactorY-1)/m_nFactorY, m_nDstSizeY);
			ULONG const nDstX1 = ((a_aTiles->tOrigin.nX<<16)+m_nFactorX-1)/m_nFactorX;
			ULONG const nDstX2 = MyMin((((a_aTiles->tOrigin.nX+a_aTiles->tSize.nX)<<16)+m_nFactorX-1)/m_nFactorX, m_nDstSizeX);
			for (ULONG y = nDstY1; y < nDstY2; ++y)
			{
				TPixelChannel* pDst = m_pData + y*m_nDstStrideY + nDstX1;
				TPixelChannel const* pSrc = a_aTiles->pData + (((y*m_nFactorY)>>16)-a_aTiles->tOrigin.nY)*a_aTiles->tStride.nY;
				for (ULONG x = nDstX1; x < nDstX2; ++x, ++pDst)
					*pDst = pSrc[(((x*m_nFactorX)>>16)-a_aTiles->tOrigin.nX)*a_aTiles->tStride.nX];
			}
		}
		return S_OK;
	}

private:
	static ULONG MyMin(ULONG a_1, ULONG a_2) { return a_1 < a_2 ? a_1 : a_2; }

private:
	TPixelChannel* m_pData;
	ULONG m_nDstSizeX;
	ULONG m_nDstSizeY;
	ULONG m_nDstStrideX;
	ULONG m_nDstStrideY;
	ULONG m_nSrcSizeX;
	ULONG m_nSrcSizeY;
	ULONG m_nFactorX;
	ULONG m_nFactorY;
};

class CImageChannelDefaultGetter : public IEnumImageChannels
{
public:
	CImageChannelDefaultGetter(EImageChannelID a_eChID, TPixelChannel* a_pDef) : m_eChID(a_eChID), m_pDef(a_pDef) {}

	operator IEnumImageChannels*() { return this; }

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IEnumImageChannels)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IEnumImageChannels methods
public:
	STDMETHOD(Range)(ULONG* /*a_pStart*/, ULONG* /*a_pCount*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Consume)(ULONG /*a_nStart*/, ULONG a_nCount, TChannelDefault const* a_aChannelDefaults)
	{
		for (; a_nCount > 0; ++a_aChannelDefaults, --a_nCount)
			if (a_aChannelDefaults->eID == m_eChID)
			{
				*m_pDef = a_aChannelDefaults->tValue;
				return S_FALSE;
			}
		return S_OK;
	}

private:
	EImageChannelID m_eChID;
	TPixelChannel* m_pDef;
};

struct CBGRABuffer
{
	CAutoVectorPtr<BYTE> aData;
	ULONG nData;
	TImageSize tSize;
	ULONG nStride;
	TImageResolution tResolution;
	bool bAlpha; // image is semitransparent (fully opaque if false)
	bool bColor; // image is colored (grayscale if false)

	CBGRABuffer() : nData(0), bAlpha(true), nStride(0)
	{
		tSize.nX = tSize.nY = 0;
		tResolution.nNumeratorX = tResolution.nDenominatorX = tResolution.nNumeratorY = tResolution.nDenominatorY = 0;
	}

	bool Init(IDocumentImage* a_pI, bool a_bNeedAlphaState = true, bool a_bNeedColorState = false)
	{
		TImagePoint tContentOrig = {0, 0};
		TImageSize tContentSize = {0, 0};
		EImageOpacity eOpacity = EIOUnknown;
		HRESULT hRes = a_pI->CanvasGet(&tSize, &tResolution, &tContentOrig, &tContentSize, &eOpacity);
		if (FAILED(hRes) || tSize.nX*tSize.nY == 0) return false;
		TPixelChannel tDefault;
		tDefault.n = 0;
		a_pI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));
		if (!aData.Allocate(tSize.nX*tSize.nY<<2)) return false;
		hRes = a_pI->TileGet(EICIRGBA, NULL, NULL, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(aData.m_p), NULL, EIRIPreview);
		if (FAILED(hRes)) return false;
		nData = tSize.nX*tSize.nY<<2;
		nStride = tSize.nX<<2;
		if (a_bNeedAlphaState)
			ComputeAlpha(tContentOrig, tContentSize, eOpacity, tDefault);
		if (a_bNeedColorState)
			ComputeColor();
		return true;
	}

	void ToBGR()
	{
		BYTE const* pS = aData.m_p;
		BYTE* pD = aData.m_p;
		for (BYTE const* pEnd = pS+nData; pS != pEnd; pS+=4, pD+=3)
		{
			pD[0] = pS[0];
			pD[1] = pS[1];
			pD[2] = pS[2];
		}
		nStride = tSize.nX*3;
	}

	void ToRGB()
	{
		BYTE const* pS = aData.m_p;
		BYTE* pD = aData.m_p;
		for (BYTE const* pEnd = pS+nData; pS != pEnd; pS+=4, pD+=3)
		{
			BYTE const b = pS[0];
			pD[0] = pS[2];
			pD[1] = pS[1];
			pD[2] = b;
		}
		nStride = tSize.nX*3;
	}

	void ToLA()
	{
		BYTE const* pS = aData.m_p;
		BYTE* pD = aData.m_p;
		for (BYTE const* pEnd = pS+nData; pS != pEnd; pS+=4, pD+=2)
		{
			pD[0] = pS[0];
			pD[1] = pS[3];
		}
		nStride = tSize.nX<<1;
	}

	void ToL()
	{
		BYTE const* pS = aData.m_p;
		BYTE* pD = aData.m_p;
		for (BYTE const* pEnd = pS+nData; pS != pEnd; pS+=4, ++pD)
		{
			pD[0] = pS[0];
		}
		nStride = tSize.nX;
	}

private:
	void ComputeAlpha(TImagePoint const& tContentOrig, TImageSize const& tContentSize, EImageOpacity eOpacity, TPixelChannel tDefault)
	{
		bool const bContentImportant = tContentSize.nX*tContentSize.nY > 0 &&
			tContentOrig.nX < LONG(tSize.nX) && tContentOrig.nY < LONG(tSize.nY) &&
			LONG(tContentOrig.nX+tContentSize.nX) > 0 && LONG(tContentOrig.nY+tContentSize.nY) > 0;
		if (!bContentImportant)
		{
			bAlpha = tDefault.bA != 0xff;
			return;
		}
		bool const bDefaultImportant = tContentOrig.nX > 0 || tContentOrig.nY > 0 ||
			LONG(tContentOrig.nX+tContentSize.nX) < LONG(tSize.nX) || LONG(tContentOrig.nY+tContentSize.nY) < LONG(tSize.nY);
		if (bDefaultImportant && tDefault.bA != 0xff)
		{
			bAlpha = true;
			return;
		}
		bAlpha = false;
		BYTE const* p = aData.m_p+3;
		for (BYTE const* pEnd = p+nData; p != pEnd; p+=4)
			if (*p != 0xff)
			{
				bAlpha = true;
				break;
			}
	}
	void ComputeColor()
	{
		bColor = false;
		BYTE const* p = aData.m_p;
		for (BYTE const* pEnd = p+nData; p != pEnd; p+=4)
			if (*p != p[1] || *p != p[2])
			{
				bColor = true;
				break;
			}
	}
};

const TMatrix3x3f TMATRIX3X3F_IDENTITY =
{
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
};

inline TMatrix3x3f* Matrix3x3fMultiply(TMatrix3x3f const& a_tLeft, TMatrix3x3f const& a_tRight, TMatrix3x3f* a_pOut)
{
	a_pOut->_11 = a_tLeft._11*a_tRight._11 + a_tLeft._12*a_tRight._21 + a_tLeft._13*a_tRight._31;
	a_pOut->_12 = a_tLeft._11*a_tRight._12 + a_tLeft._12*a_tRight._22 + a_tLeft._13*a_tRight._32;
	a_pOut->_13 = a_tLeft._11*a_tRight._13 + a_tLeft._12*a_tRight._23 + a_tLeft._13*a_tRight._33;
	a_pOut->_21 = a_tLeft._21*a_tRight._11 + a_tLeft._22*a_tRight._21 + a_tLeft._23*a_tRight._31;
	a_pOut->_22 = a_tLeft._21*a_tRight._12 + a_tLeft._22*a_tRight._22 + a_tLeft._23*a_tRight._32;
	a_pOut->_23 = a_tLeft._21*a_tRight._13 + a_tLeft._22*a_tRight._23 + a_tLeft._23*a_tRight._33;
	a_pOut->_31 = a_tLeft._31*a_tRight._11 + a_tLeft._32*a_tRight._21 + a_tLeft._33*a_tRight._31;
	a_pOut->_32 = a_tLeft._31*a_tRight._12 + a_tLeft._32*a_tRight._22 + a_tLeft._33*a_tRight._32;
	a_pOut->_33 = a_tLeft._31*a_tRight._13 + a_tLeft._32*a_tRight._23 + a_tLeft._33*a_tRight._33;
	return a_pOut;
}

inline TMatrix3x3f* Matrix3x3fInverse(TMatrix3x3f const& a_tOrig, TMatrix3x3f* a_pOut)
{
	float const f11 = a_tOrig._33*a_tOrig._22-a_tOrig._32*a_tOrig._23;
	float const f21 = a_tOrig._33*a_tOrig._12-a_tOrig._32*a_tOrig._13;
	float const f31 = a_tOrig._23*a_tOrig._12-a_tOrig._22*a_tOrig._13;
	float const f = 1.0f / (a_tOrig._11*f11 - a_tOrig._21*f21 + a_tOrig._31*f31);
	a_pOut->_11 = f * f11;
	a_pOut->_12 = -f * f21;
	a_pOut->_13 = f * f31;
	a_pOut->_21 = -f * (a_tOrig._33*a_tOrig._21-a_tOrig._31*a_tOrig._23);
	a_pOut->_22 = f * (a_tOrig._33*a_tOrig._11-a_tOrig._31*a_tOrig._13);
	a_pOut->_23 = -f * (a_tOrig._23*a_tOrig._11-a_tOrig._21*a_tOrig._13);
	a_pOut->_31 = f * (a_tOrig._32*a_tOrig._21-a_tOrig._31*a_tOrig._22);
	a_pOut->_32 = -f * (a_tOrig._32*a_tOrig._11-a_tOrig._31*a_tOrig._12);
	a_pOut->_33 = f * (a_tOrig._22*a_tOrig._11-a_tOrig._21*a_tOrig._12);
	return a_pOut;
}

inline TVector2f TransformVector2(TMatrix3x3f const& a_tMatrix, TVector2f const& a_tVector)
{
	float const fW = 1.0f/(a_tMatrix._13*a_tVector.x + a_tMatrix._23*a_tVector.y + a_tMatrix._33);
	TVector2f const t =
	{
		fW*(a_tMatrix._11*a_tVector.x + a_tMatrix._21*a_tVector.y + a_tMatrix._31),
		fW*(a_tMatrix._12*a_tVector.x + a_tMatrix._22*a_tVector.y + a_tMatrix._32),
	};
	return t;
}

inline TVector2f TransformDirectionVector2Inv(TMatrix3x3f const& a_tMatrix, TVector2f const& a_tVector)
{
	TVector2f const t =
	{
		a_tMatrix._11*a_tVector.x + a_tMatrix._12*a_tVector.y,
		a_tMatrix._21*a_tVector.x + a_tMatrix._22*a_tVector.y,
	};
	return t;
}

inline TVector2f TransformDirectionVector2(TMatrix3x3f const& a_tMatrix, TVector2f const& a_tVector)
{
	TMatrix3x3f x2;
	Matrix3x3fInverse(a_tMatrix, &x2);
	return TransformDirectionVector2Inv(x2, a_tVector);
}

inline TVector2f operator +(TVector2f const& a_lhs, TVector2f const& a_rhs)
{
	TVector2f t = {a_lhs.x+a_rhs.x, a_lhs.y+a_rhs.y};
	return t;
}

inline TVector2f const& operator +=(TVector2f& a_lhs, TVector2f const& a_rhs)
{
	a_lhs.x += a_rhs.x;
	a_lhs.y += a_rhs.y;
	return a_lhs;
}

inline TVector2f const& VectorNormalize(TVector2f& a)
{
	float const f = 1.0f/sqrtf(a.x*a.x+a.y*a.y);
	a.x *= f;
	a.y *= f;
	return a;
}

inline void Matrix3x3fDecompose(TMatrix3x3f const& a_tMatrix, float* a_pScaleX, float* a_pScaleY, float* a_pRotation, TVector2f* a_pTranslation)
{
	if (a_pScaleX)
		*a_pScaleX = sqrtf(a_tMatrix._11*a_tMatrix._11+a_tMatrix._21*a_tMatrix._21);
	if (a_pScaleY)
		*a_pScaleY = sqrtf(a_tMatrix._12*a_tMatrix._12+a_tMatrix._22*a_tMatrix._22);
	if (a_pTranslation)
	{
		a_pTranslation->x = a_tMatrix._31;
		a_pTranslation->y = a_tMatrix._32;
	}
}

inline float Matrix3x3fDecomposeScale(TMatrix3x3f const& a_tMatrix)
{
	float fScaleX = 1.0f;
	float fScaleY = 1.0f;
	Matrix3x3fDecompose(a_tMatrix, &fScaleX, &fScaleY, NULL, NULL);
	return sqrtf(fScaleX*fScaleY);
}

class CColor : public TColor
{
public:
	CColor()
	{
		fR = fG = fB = fA = 0.0f;
	}
	CColor(TColor const& a_t)
	{
		fR = a_t.fR;
		fG = a_t.fG;
		fB = a_t.fB;
		fA = a_t.fA;
	}
	CColor(COLORREF a_clr)
	{
		fR = CGammaTables::FromSRGB(GetRValue(a_clr));
		fG = CGammaTables::FromSRGB(GetGValue(a_clr));
		fB = CGammaTables::FromSRGB(GetBValue(a_clr));
		fA = 1.0f;
	}
	DWORD GetRGB()
	{
		int const nR = CGammaTables::ToSRGB(fR);
		int const nG = CGammaTables::ToSRGB(fG);
		int const nB = CGammaTables::ToSRGB(fB);
		return RGB(nR < 0 ? 0 : (nR > 255 ? 255 : nR), nG < 0 ? 0 : (nG > 255 ? 255 : nG), nB < 0 ? 0 : (nB > 255 ? 255 : nB));
	}
};

inline bool ColorWindowModal(TColor* a_pColor, LCID a_tLocaleID, HWND a_hParent, bool a_bAlpha)
{
	CComPtr<IColorWindow> pCW;
	RWCoCreateInstance(pCW, __uuidof(ColorWindow));
	if (pCW)
		return S_OK == pCW->DoModal(a_hParent, a_tLocaleID, a_pColor, a_bAlpha);
	return false;
}

struct SLockedImageBuffer
{
	SLockedImageBuffer(IDocumentImage* a_pImage, ITaskControl* a_pControl = NULL, EImageRenderingIntent a_eIntent = EIRIAccurate) : m_pImage(a_pImage)
	{
		m_pImage->BufferLock(EICIRGBA, &tAllocOrigin, &tAllocSize, &tContentOrigin, &tContentSize, &pData, a_pControl, a_eIntent);
	}
	~SLockedImageBuffer()
	{
		m_pImage->BufferUnlock(EICIRGBA, pData);
	}

	TPixelChannel const* Content() const { return pData + tAllocSize.nX*(tContentOrigin.nY-tAllocOrigin.nY) + (tContentOrigin.nX-tAllocOrigin.nX); }

	TImagePoint tAllocOrigin;
	TImageSize tAllocSize;
	TImagePoint tContentOrigin;
	TImageSize tContentSize;
	TPixelChannel const* pData;

private:
	IDocumentImage* m_pImage;
};

#endif//__cplusplus

