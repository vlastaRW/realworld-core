// ColorQuantizer.h : Declaration of the CColorQuantizer

#pragma once
#include "resource.h"       // main symbols

#include "RWImaging.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CColorQuantizer

class ATL_NO_VTABLE CColorQuantizer :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CColorQuantizer, &CLSID_ColorQuantizer>,
	public IColorQuantizer
{
public:
	CColorQuantizer() : m_eState(ESRaw)
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CColorQuantizer)
	COM_INTERFACE_ENTRY(IColorQuantizer)
	COM_INTERFACE_ENTRY(IImageVisitor)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IImageVisitor
public:
	STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl* a_pControl);

	// IColorQuantizer methods
public:
	STDMETHOD(Init)(ULONG a_nColors, EColorDithering a_eDither, BYTE a_bAlphaLimit, TPixelChannel const* a_pBackground, TImageSize const* a_pCanvasSize);
	STDMETHOD(Colors)(IEnum2UInts* a_pColors);
	STDMETHOD(Process)(TImagePoint const* a_pOrigin, TImageSize a_tSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl);

private:
	enum EState
	{
		ESRaw = 0,
		ESInitialized,
		ESReady,
	};
	enum { t_nBits = 8 };

	class TRandomMotherOfAll // random number generator
	{
	public:
		TRandomMotherOfAll(unsigned int seed)
		{
			RandomInit(seed);
		}

		void RandomInit(unsigned int seed)
		{
			int i;
			unsigned int s = seed;
			// make random numbers and put them into the buffer
			for (i=0; i<5; i++)
			{
				s = s * 29943829 - 1;
				x[i] = s * (1./(65536.*65536.));
			}
			// randomize some more
			for (i=0; i<19; i++)
				Random();
		}
		int IRandom(int min, int max)       // get integer random number in desired interval
		{
			int iinterval = max - min + 1;
			if (iinterval <= 0) return 0x80000000; // error
			int i = int(iinterval * Random());     // truncate
			if (i >= iinterval) i = iinterval-1;
			return min + i;
		}
		double Random()                     // get floating point random number
		{
			long double c;
			c = (long double)2111111111.0 * x[3] +
				1492.0 * (x[3] = x[2]) +
				1776.0 * (x[2] = x[1]) +
				5115.0 * (x[1] = x[0]) +
				x[4];
			x[4] = floorl(c);
			x[0] = c - x[4];
			x[4] = x[4] * (1./(65536.*65536.));
			return x[0];
		}

	private:
		double x[5];                         // history buffer
	};

	struct SNode
	{
		SNode() : m_nTimeStamp(0), m_iLevel(-1), m_nR(0), m_nG(0), m_nB(0), m_nPixels(0)
		{
			ZeroMemory(m_aSubNodes, sizeof m_aSubNodes);
		}
		SNode(int a_iLevel, TPixelChannel a_tPixel, ULONG a_nCount = 1) : m_nTimeStamp(0), m_iLevel(a_iLevel),
			m_nR(a_tPixel.bR*a_nCount), m_nG(a_tPixel.bG*a_nCount), m_nB(a_tPixel.bB*a_nCount), m_nPixels(a_nCount)
		{
			ZeroMemory(m_aSubNodes, sizeof m_aSubNodes);
		}
		~SNode()
		{
			for (size_t i = 0; i < itemsof(m_aSubNodes); i++)
				delete m_aSubNodes[i];
		}
		void Insert(TPixelChannel a_tPixel, ULONG a_nCount = 1)
		{
			m_nR += a_tPixel.bR*a_nCount;
			m_nG += a_tPixel.bG*a_nCount;
			m_nB += a_tPixel.bB*a_nCount;
			m_nPixels += a_nCount;
		}
		int GetChildCount() const
		{
			int iCount = 0;
			for (int i = 0; i < 8; ++i)
			{
				if (m_aSubNodes[i])
					iCount++;
			}
			return iCount;
		}

		int Weight() const
		{
			return m_fError;
		}
		void PrepareNode()
		{
			for (int j = 0; j < 8; ++j)
			{
				if (m_aSubNodes[j])
					m_aSubNodes[j]->PrepareNode();
			}
			if (m_iLevel == (t_nBits-1))
			{
				m_bFinalR = static_cast<BYTE>(m_nR / m_nPixels);
				m_bFinalG = static_cast<BYTE>(m_nG / m_nPixels);
				m_bFinalB = static_cast<BYTE>(m_nB / m_nPixels);
				m_fError = 0.0;
			}
			else
			{
				m_bFinalR = static_cast<BYTE>((m_nR+(m_nPixels>>1)) / m_nPixels);
				m_bFinalG = static_cast<BYTE>((m_nG+(m_nPixels>>1)) / m_nPixels);
				m_bFinalB = static_cast<BYTE>((m_nB+(m_nPixels>>1)) / m_nPixels);
				m_fError = GetError(m_bFinalR, m_bFinalG, m_bFinalB);
			}
		}
		double GetError(int a_nR, int a_nG, int a_nB) const
		{
			if (m_iLevel == (t_nBits-1))
			{
				return ((a_nR-m_bFinalR)*(a_nR-m_bFinalR) + (a_nG-m_bFinalG)*(a_nG-m_bFinalG) + (a_nB-m_bFinalB)*(a_nB-m_bFinalB)) * m_nPixels;
			}
			else
			{
				double tmp = 0.0;
				for (int j = 0; j < 8; j++)
					if (m_aSubNodes[j])
						tmp += m_aSubNodes[j]->GetError(a_nR, a_nG, a_nB);
				return tmp;
			}
		}

		int m_iLevel;
		ULONG m_nR;
		ULONG m_nG;
		ULONG m_nB;
		ULONG m_nPixels;
		double m_fError;
		BYTE m_bFinalR;
		BYTE m_bFinalG;
		BYTE m_bFinalB;
		SNode* m_aSubNodes[8];

		mutable int m_iIndex;
		mutable ULONG m_nTimeStamp;
	};

private:
	void ComputePalette();
	inline void InsertPixel(TPixelChannel a_tPixel, int a_nCount = 1)
	{
		int nIndR = static_cast<int>(a_tPixel.bR)<<3;
		int nIndG = static_cast<int>(a_tPixel.bG)<<2;
		int nIndB = static_cast<int>(a_tPixel.bB)<<1;
		SNode* pNode = &m_sRoot;
		pNode->Insert(a_tPixel, a_nCount);
		int iLevel;
		for (iLevel = 0; iLevel < t_nBits; iLevel++)
		{
			int iSubNode =
				((nIndR&(1<<(t_nBits+2-iLevel))) |
				 (nIndG&(1<<(t_nBits+1-iLevel))) |
				 (nIndB&(1<<(t_nBits-iLevel))))>>(t_nBits-iLevel);
			if (pNode->m_aSubNodes[iSubNode] == NULL)
			{
				pNode->m_aSubNodes[iSubNode] = new SNode(iLevel, a_tPixel, a_nCount);
			}
			else
			{
				pNode->m_aSubNodes[iSubNode]->Insert(a_tPixel, a_nCount);
			}
			pNode = pNode->m_aSubNodes[iSubNode];
		}
	}
	static inline int BestColor(RGBQUAD const* a_pPalette, int const a_nPalColors, BYTE a_nR, BYTE a_nG, BYTE a_nB)
	{
		int nMinDist = 200000;
		int nBestColor = 0;
		int i;
		for (i = 0; i < a_nPalColors; i++)
		{
			int nRDist = a_pPalette[i].rgbRed - a_nR;
			int nGDist = a_pPalette[i].rgbGreen - a_nG;
			int nBDist = a_pPalette[i].rgbBlue - a_nB;
			int nCurDist = nRDist*nRDist + nGDist*nGDist + nBDist*nBDist;
			if (nCurDist < nMinDist)
			{
				nMinDist = nCurDist;
				nBestColor = i;
			}
		}
		return nBestColor;
	}
	int GetPalette(RGBQUAD* a_pPalEntries, int a_nPalEntries);
	int FindColorIndex(TPixelChannel a_tPixel) const; // may only be called AFTER GetPalette
	void DirectToIndexed(TPixelChannel const* a_pSrc, RGBQUAD const* a_pPalette, RGBQUAD const* a_pPalette2, int a_nPalColors, int a_nSizeX, int a_nSizeY, bool a_bDither, TPixelChannel* a_pDst) const;

private:
	EState m_eState;
	ULONG m_nColors;
	EColorDithering m_eDither;
	BYTE m_bAlphaLimit;
	bool m_bBlend;
	TPixelChannel m_tBackground;

	SNode m_sRoot;
	mutable ULONG m_nLastTimeStamp;
	CAutoVectorPtr<RGBQUAD> m_aPal;
	ULONG m_nPal;
};

OBJECT_ENTRY_AUTO(__uuidof(ColorQuantizer), CColorQuantizer)
