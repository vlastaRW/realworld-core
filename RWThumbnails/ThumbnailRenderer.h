// ThumbnailRenderer.h : Declaration of the CThumbnailRenderer

#pragma once
#include "resource.h"       // main symbols
#include "RWThumbnails.h"
#include <WeakSingleton.h>



// CThumbnailRenderer

class ATL_NO_VTABLE CThumbnailRenderer :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CThumbnailRenderer, &CLSID_ThumbnailRenderer>,
	public IThumbnailRenderer,
	public IThumbnailRendererDoc
{
public:
	CThumbnailRenderer()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CThumbnailRenderer)


BEGIN_COM_MAP(CThumbnailRenderer)
	COM_INTERFACE_ENTRY(IThumbnailRenderer)
	COM_INTERFACE_ENTRY(IThumbnailRendererDoc)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IThumbnailRenderer methods
public:
	STDMETHOD(GetThumbnail)(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo);

	// IThumbnailRendererDoc methods
public:
	STDMETHOD(GetThumbnail)(IDocument* a_pDoc, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo);

private:
	HRESULT GetLayerEffectThumbnail(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo);
	HRESULT GetBatchOperationThumbnail(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo);

private:
	CComPtr<IInputManager> m_pIM;
};

OBJECT_ENTRY_AUTO(__uuidof(ThumbnailRenderer), CThumbnailRenderer)
