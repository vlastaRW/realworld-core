// PixelFormatConverter.h : Declaration of the CPixelFormatConverter

#pragma once
#include "resource.h"       // main symbols

#include "RWImaging.h"


// CPixelFormatConverter

class ATL_NO_VTABLE CPixelFormatConverter : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPixelFormatConverter, &CLSID_PixelFormatConverter>,
	public IPixelFormatConverter
{
public:
	CPixelFormatConverter()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CPixelFormatConverter)
	COM_INTERFACE_ENTRY(IPixelFormatConverter)
END_COM_MAP()


	// IPixelFormatConverter methods
public:
	STDMETHOD(Convert)(TPixelFormat const* a_ptInputFormat, ULONG a_nInputPixels, BYTE const* a_aInputPixels, TPixelFormat const* a_ptOutputFormat, ULONG a_nOutputPixels, BYTE* a_aOutputPixels);
};

OBJECT_ENTRY_AUTO(__uuidof(PixelFormatConverter), CPixelFormatConverter)
