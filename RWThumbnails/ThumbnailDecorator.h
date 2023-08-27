// ThumbnailDecorator.h : Declaration of the CThumbnailDecorator

#pragma once
#include "resource.h"       // main symbols
#include "RWThumbnails.h"



// CThumbnailDecorator

class ATL_NO_VTABLE CThumbnailDecorator :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CThumbnailDecorator, &CLSID_ThumbnailDecorator>,
	public IThumbnailDecorator
{
public:
	CThumbnailDecorator()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CThumbnailDecorator)
	COM_INTERFACE_ENTRY(IThumbnailDecorator)
	COM_INTERFACE_ENTRY(IThumbnailRenderer)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IThumbnailDecorator methods
public:
	STDMETHOD(Init)(IThumbnailRenderer* a_pInternalRenderer, ULONG a_nShadowPixels, BOOL a_bChequeredBackground);

	// IThumbnailRenderer methods
public:
	STDMETHOD(GetThumbnail)(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo);

private:
	CComPtr<IThumbnailRenderer> m_pInternal;
	ULONG m_nShadowPixels;
	BOOL m_bChequeredBackground;
};

OBJECT_ENTRY_AUTO(__uuidof(ThumbnailDecorator), CThumbnailDecorator)
