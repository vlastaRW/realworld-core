
#pragma once

#include <TextGUID.h>

#include "RWBase.h"
#include "RWStorage.h"
#include "RWInput.h"


class __declspec(novtable) IThumbnailProvider : public IUnknown
{
public:
    virtual ULONG STDMETHODCALLTYPE Priority() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetThumbnail(IDocument* a_pDoc, ULONG a_nSizeX, ULONG a_nSizeY, DWORD *a_pBGRAData, RECT *a_prcBounds, LCID a_tLocaleID, BSTR *a_pbstrInfo, HRESULT (fnRescaleImage)(ULONG a_nSrcSizeX, ULONG a_nSrcSizeY, DWORD const* a_pSrcData, bool a_bSrcAlpha, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds)) = 0;
};

extern __declspec(selectany) GUID const IID_IThumbnailProvider { GUIDFromText("IThumbProvider") };
extern __declspec(selectany) GUID const CATID_ThumbnailProvider { GUIDFromText("CAT_ThumbProvider") };

template<> inline GUID rwuuidof<IThumbnailProvider>() { return IID_IThumbnailProvider; }
