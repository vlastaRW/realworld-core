// DocumentFactoryBinary.h : Declaration of the CDocumentFactoryBinary

#pragma once
#include "resource.h"       // main symbols
#include "RWDocumentBasic.h"

#include "DocumentBinary.h"

extern __declspec(selectany) IID const g_aSupportedBin[] =
{
	__uuidof(IDocumentBinary),
};
extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsBin[] = L"bin";
extern __declspec(selectany) OLECHAR const g_pszShellIconPathBin[] = L"%MODULE%,1";
extern __declspec(selectany) OLECHAR const g_pszTypeNameBin[] = L"[0409]Binary Data[0405]Binární data";
extern __declspec(selectany) OLECHAR const g_pszFormatNameBin[] = L"[0409]Binary files[0405]Binární soubory";

typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameBin, g_pszTypeNameBin, g_pszSupportedExtensionsBin, 0, g_pszShellIconPathBin> CDocumentTypeCreatorBIN;


// CDocumentFactoryBinary

class ATL_NO_VTABLE CDocumentFactoryBinary : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentFactoryBinary, &CLSID_DocumentFactoryBinary>,
	public IDocumentFactoryBinary,
	public CDocumentDecoderImpl<CDocumentFactoryBinary, CDocumentTypeCreatorBIN, IDocumentFactoryBinary, EDPMinimum, IDocumentCodec>,
	public CDocumentBuilderImpl<CDocumentFactoryBinary, FEATURES(g_aSupportedBin), g_pszTypeNameBin, 0, g_pszFormatNameBin, g_pszShellIconPathBin, EDPAverage>
{
public:
	CDocumentFactoryBinary()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentFactoryBinary)

BEGIN_CATEGORY_MAP(CDocumentFactoryBinary)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
	IMPLEMENTED_CATEGORY(CATID_DocumentBuilder)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentFactoryBinary)
	COM_INTERFACE_ENTRY(IDocumentFactoryBinary)
	COM_INTERFACE_ENTRY(IDocumentDecoder)
	COM_INTERFACE_ENTRY(IDocumentBuilder)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDocumentBuilder methods
public:

	// IDocumentDecoder methods
public:
	HRESULT Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryBinary* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);

	// IDocumentEncoder methods
public:
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg);
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	// IDocumentFactoryBinary methods
public:
	STDMETHOD(Init)(ULONG a_nLength, BYTE const* a_pData, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentFactoryBinary), CDocumentFactoryBinary)
