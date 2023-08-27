// ImageFilterResize.h : Declaration of the CImageFilterResize

#pragma once
#include "resource.h"       // main symbols

#include "RWImaging.h"


// CImageFilterResize

class ATL_NO_VTABLE CImageFilterResize : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CImageFilterResize, &CLSID_ImageFilterResize>,
	public CImageSourceSimple,
	public IImageFilter,
	public IImageMetaData
{
public:
	CImageFilterResize()
	{
	}

DECLARE_NO_REGISTRY()


	static HRESULT WINAPI QIMetaData(void* a_pThis, REFIID a_iid, void** a_ppv, DWORD_PTR a_dw)
	{
		CImageFilterResize* pThis = reinterpret_cast<CImageFilterResize*>(a_pThis);
		if (pThis->m_pMetaData)
		{
			*a_ppv = reinterpret_cast<void*>(static_cast<IImageMetaData*>(pThis));
			pThis->AddRef();
			return S_OK;
		}
		else
		{
			*a_ppv = NULL;
			return E_NOINTERFACE;
		}
	}

BEGIN_COM_MAP(CImageFilterResize)
	COM_INTERFACE_ENTRY(IImageSource)
	COM_INTERFACE_ENTRY(IImageFilter)
	COM_INTERFACE_ENTRY_FUNC(__uuidof(IImageMetaData), 0, QIMetaData)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IImageSource methods
public:
	STDMETHOD(FormatGet)(TImageFormat* a_ptImageFormat);
	STDMETHOD(DataGet)(IImageSink* a_pImageSink);

	// IImageFilter methods
public:
	STDMETHOD(Init)(IImageSource* a_pSource, const TImageFormat* a_tOutputFormat)
	{
		m_pSource = a_pSource;
		m_pMetaData = a_pSource;
		m_cIFOut = *a_tOutputFormat;
		return S_OK;
	}

	// IImageMetaData methods
public:
	STDMETHOD(EnumIDs)(IEnumStrings** a_ppBlockIDs)
	{
		return m_pMetaData.p ? m_pMetaData->EnumIDs(a_ppBlockIDs) : E_NOTIMPL;
	}
	STDMETHOD(SetBlock)(BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData)
	{
		return E_FAIL;
	}
	STDMETHOD(GetBlockSize)(BSTR a_bstrID, ULONG* a_pSize)
	{
		return m_pMetaData.p ? m_pMetaData->GetBlockSize(a_bstrID, a_pSize) : E_NOTIMPL;
	}
	STDMETHOD(GetBlock)(BSTR a_bstrID, ULONG a_nSize, BYTE* a_pData)
	{
		return m_pMetaData.p ? m_pMetaData->GetBlock(a_bstrID, a_nSize, a_pData) : E_NOTIMPL;
	}
	STDMETHOD(DeleteBlock)(BSTR a_bstrID)
	{
		return E_FAIL;
	}

private:
	CComPtr<IImageSource> m_pSource;
	CComQIPtr<IImageMetaData> m_pMetaData;
	CImageFormat m_cIFOut;
};

OBJECT_ENTRY_AUTO(__uuidof(ImageFilterResize), CImageFilterResize)
