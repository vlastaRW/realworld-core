// ImageMetaData.h : Declaration of the CImageMetaData

#pragma once
#include "resource.h"       // main symbols

#include "RWImaging.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CImageMetaData

class ATL_NO_VTABLE CImageMetaData :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CImageMetaData, &CLSID_ImageMetaData>,
	public IImageMetaData
{
public:
	CImageMetaData()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CImageMetaData)
	COM_INTERFACE_ENTRY(IImageMetaData)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IImageMetaData methods
public:
	STDMETHOD(EnumIDs)(IEnumStrings** a_ppBlockIDs);
	STDMETHOD(SetBlock)(BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData);
	STDMETHOD(GetBlockSize)(BSTR a_bstrID, ULONG* a_pSize);
	STDMETHOD(GetBlock)(BSTR a_bstrID, ULONG a_nSize, BYTE* a_pData);
	STDMETHOD(DeleteBlock)(BSTR a_bstrID);

private:
	typedef std::map<CComBSTR, std::pair<ULONG, CAutoVectorPtr<BYTE> > > CBlocks;

private:
	CBlocks m_cBlocks;
};

OBJECT_ENTRY_AUTO(__uuidof(ImageMetaData), CImageMetaData)
