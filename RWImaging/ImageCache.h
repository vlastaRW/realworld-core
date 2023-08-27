// ImageCache.h : Declaration of the CImageCache

#pragma once
#include "resource.h"       // main symbols

#include "RWImaging.h"


// CImageCache

class ATL_NO_VTABLE CImageCache : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CImageCache, &CLSID_ImageCache>,
	public IDocumentFactoryRasterImage,
	public IDocumentBuilder,
	public IDocumentImage
{
public:
	CImageCache() : m_pData(NULL), m_fGamma(2.2f)
	{
		m_tCanvasSize.nX = m_tCanvasSize.nY = 0;
		m_tContentSize.nX = m_tContentSize.nY = 0;
		m_tContentOrigin.nX = m_tContentOrigin.nY = 0;
		m_tDefault.n = 0;
		m_tResolution.nNumeratorX = m_tResolution.nNumeratorY = 100;
		m_tResolution.nDenominatorX = m_tResolution.nDenominatorY = 254;
	}
	~CImageCache()
	{
		delete[] m_pData;
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CImageCache)
	COM_INTERFACE_ENTRY(IDocumentImage)
	COM_INTERFACE_ENTRY(IDocumentFactoryRasterImage)
	COM_INTERFACE_ENTRY(IDocumentBuilder)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDocumentImage methods
public:
	STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity);
	STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults);
	STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
	STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
	STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer);

	STDMETHOD(ObserverIns)(IImageObserver* UNREF(a_pObserver), TCookie UNREF(a_tCookie)) { return S_FALSE; }
	STDMETHOD(ObserverDel)(IImageObserver* UNREF(a_pObserver), TCookie UNREF(a_tCookie)) { return S_FALSE; }

	// IDocumentFactoryRasterImage methods
public:
	STDMETHOD(Create)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile);

	// IDocumentBuilder methods
public:
	STDMETHOD(Priority)(ULONG* a_pnPriority) { *a_pnPriority = EDPAverage; return S_OK; }
	STDMETHOD(TypeName)(ILocalizedString** a_ppType) { return E_NOTIMPL; }
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon) { return E_NOTIMPL; }
	STDMETHOD(FormatInfo)(ILocalizedString** a_ppFormat, BSTR* a_pbstrShellIcon) { return E_NOTIMPL; }
	STDMETHOD(HasFeatures)(ULONG a_nCount, IID const* a_aiidRequired) { return E_NOTIMPL; }

private:
	TImageSize m_tCanvasSize;
	TImagePoint m_tContentOrigin;
	TImageSize m_tContentSize;
	TPixelChannel m_tDefault;
	TImageResolution m_tResolution;
	float m_fGamma;
	TPixelChannel* m_pData;
};

OBJECT_ENTRY_AUTO(__uuidof(ImageCache), CImageCache)
