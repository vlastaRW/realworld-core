
#pragma once

#include "oaidl.h"
#include "ocidl.h"
#include "RWBase.h"
#include "RWStorage.h"
#include "RWInput.h"


EXTERN_C const IID IID_IThumbnailRenderer;

MIDL_INTERFACE("4F99C991-9632-4608-9849-CE8B888D494B")
IThumbnailRenderer : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetThumbnail(IStorageFilter *a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD *a_pBGRAData, RECT *a_prcBounds, LCID a_tLocaleID, BSTR *a_pbstrInfo) = 0;
};


EXTERN_C const IID IID_IThumbnailRendererDoc;

MIDL_INTERFACE("48FC90E3-4F50-489E-A33B-BD378D0793CB")
IThumbnailRendererDoc : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetThumbnail(IDocument *a_pDoc, ULONG a_nSizeX, ULONG a_nSizeY, DWORD *a_pBGRAData, RECT *a_prcBounds, LCID a_tLocaleID, BSTR *a_pbstrInfo) = 0;
};
    

EXTERN_C const IID IID_IThumbnailCallback;

MIDL_INTERFACE("78F26E9E-93A9-4DB5-B2AB-ED83F6F1494B")
IThumbnailCallback : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE AdjustSize(ULONG *a_pSizeX, ULONG *a_pSizeY) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetThumbnail(IStorageFilter *a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, const DWORD *a_pBGRAData, const RECT *a_prcBounds, BSTR a_bstrInfo) = 0;
};


EXTERN_C const IID IID_IThumbnailCache;

MIDL_INTERFACE("3C4B8599-2B26-4F88-A1B3-1466E88A947D")
IThumbnailCache : public IThumbnailRenderer
{
public:
    virtual HRESULT STDMETHODCALLTYPE Init(IThumbnailRenderer *a_pInternalRenderer, ULONG a_nMemoryItem, ULONG a_nFilesystemItems, BSTR a_bstrTempPath) = 0;
    virtual HRESULT STDMETHODCALLTYPE Remove(IStorageFilter *a_pFile, ULONG a_nSizeX, ULONG a_nSizeY) = 0;
    virtual HRESULT STDMETHODCALLTYPE Clear() = 0;
};


EXTERN_C const IID IID_IThumbnailDecorator;

MIDL_INTERFACE("2140044E-102B-46AE-8119-BE2B7FAF25EB")
IThumbnailDecorator : public IThumbnailRenderer
{
public:
    virtual HRESULT STDMETHODCALLTYPE Init(IThumbnailRenderer *a_pInternalRenderer, ULONG a_nShadowPixels, BOOL a_bChequeredBackground) = 0;
};


EXTERN_C const IID IID_IAsyncThumbnailRenderer;

MIDL_INTERFACE("D99C7AC8-C381-4D75-B4FE-6B43F872C1A0")
IAsyncThumbnailRenderer : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Init(IThumbnailRenderer *a_pInternalRenderer) = 0;
    virtual HRESULT STDMETHODCALLTYPE PrepareThumbnail(IStorageFilter *a_pFile, LCID a_tInfoLocaleID, IThumbnailCallback *a_pCallback) = 0;
    virtual HRESULT STDMETHODCALLTYPE CancelRequests(IThumbnailCallback *a_pCallback) = 0;
};


EXTERN_C const CLSID CLSID_ThumbnailRenderer;

class DECLSPEC_UUID("20079930-FDE5-4B62-B77E-7F0019B70C2E")
ThumbnailRenderer;

EXTERN_C const CLSID CLSID_ThumbnailCache;

class DECLSPEC_UUID("F8013314-070C-4AD5-8FF4-1CCF4A319720")
ThumbnailCache;

EXTERN_C const CLSID CLSID_ThumbnailDecorator;

class DECLSPEC_UUID("C4C505E0-09F0-4637-8BBF-1D90979CCA69")
ThumbnailDecorator;

EXTERN_C const CLSID CLSID_AsyncThumbnailRenderer;

class DECLSPEC_UUID("529C9409-3639-4597-A84A-280D6B25CDBC")
AsyncThumbnailRenderer;

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        EXTERN_C __declspec(selectany) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

MIDL_DEFINE_GUID(IID, IID_IThumbnailRenderer, 0x4F99C991, 0x9632, 0x4608, 0x98, 0x49, 0xCE, 0x8B, 0x88, 0x8D, 0x49, 0x4B);


MIDL_DEFINE_GUID(IID, IID_IThumbnailRendererDoc, 0x48FC90E3, 0x4F50, 0x489E, 0xA3, 0x3B, 0xBD, 0x37, 0x8D, 0x07, 0x93, 0xCB);


MIDL_DEFINE_GUID(IID, IID_IThumbnailCallback, 0x78F26E9E, 0x93A9, 0x4DB5, 0xB2, 0xAB, 0xED, 0x83, 0xF6, 0xF1, 0x49, 0x4B);


MIDL_DEFINE_GUID(IID, IID_IThumbnailCache, 0x3C4B8599, 0x2B26, 0x4F88, 0xA1, 0xB3, 0x14, 0x66, 0xE8, 0x8A, 0x94, 0x7D);


MIDL_DEFINE_GUID(IID, IID_IThumbnailDecorator, 0x2140044E, 0x102B, 0x46AE, 0x81, 0x19, 0xBE, 0x2B, 0x7F, 0xAF, 0x25, 0xEB);


MIDL_DEFINE_GUID(IID, IID_IAsyncThumbnailRenderer, 0xD99C7AC8, 0xC381, 0x4D75, 0xB4, 0xFE, 0x6B, 0x43, 0xF8, 0x72, 0xC1, 0xA0);


MIDL_DEFINE_GUID(IID, LIBID_RWThumbnailsLib, 0xE85F6A8E, 0x1F69, 0x4290, 0x89, 0xC6, 0x45, 0xFE, 0xC3, 0x18, 0x13, 0x94);


MIDL_DEFINE_GUID(CLSID, CLSID_ThumbnailRenderer, 0x20079930, 0xFDE5, 0x4B62, 0xB7, 0x7E, 0x7F, 0x00, 0x19, 0xB7, 0x0C, 0x2E);


MIDL_DEFINE_GUID(CLSID, CLSID_ThumbnailCache, 0xF8013314, 0x070C, 0x4AD5, 0x8F, 0xF4, 0x1C, 0xCF, 0x4A, 0x31, 0x97, 0x20);


MIDL_DEFINE_GUID(CLSID, CLSID_ThumbnailDecorator, 0xC4C505E0, 0x09F0, 0x4637, 0x8B, 0xBF, 0x1D, 0x90, 0x97, 0x9C, 0xCA, 0x69);


MIDL_DEFINE_GUID(CLSID, CLSID_AsyncThumbnailRenderer, 0x529C9409, 0x3639, 0x4597, 0xA8, 0x4A, 0x28, 0x0D, 0x6B, 0x25, 0xCD, 0xBC);
