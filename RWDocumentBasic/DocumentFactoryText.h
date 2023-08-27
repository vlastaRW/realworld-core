// DocumentFactoryText.h : Declaration of the CDocumentFactoryText

#pragma once
#include "resource.h"       // main symbols
#include "RWDocumentBasic.h"

#include "DocumentText.h"
#include <MultiLanguageString.h>
#include <RWThumbnailProvider.h>


//extern __declspec(selectany) wchar_t const DESWIZ_3D_CAT[] = L"[0409]3D Objects and Scenes[0405]3D objekty a scény";
extern __declspec(selectany) wchar_t const DESWIZ_TEXT_NAME[] = L"[0409]New text document[0405]Nový textový dokument";
extern __declspec(selectany) wchar_t const DESWIZ_TEXT_DESC[] = L"[0409]Create an empty text document.[0405]Vytvořit nový prázdný textový dokument.";

extern __declspec(selectany) IID const g_aTextSupported[] =
{
	__uuidof(IDocumentText)
};
extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsText[] = L"txt";
extern __declspec(selectany) OLECHAR const g_pszShellIconPathText[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszTypeNameText[] = L"[0409]Text[0405]Text";
extern __declspec(selectany) OLECHAR const g_pszFormatNameText[] = L"[0409]Text files[0405]Textové soubory";

typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameText, g_pszTypeNameText, g_pszSupportedExtensionsText, IDI_TEXT_NEWDOC, g_pszShellIconPathText> CDocumentTypeCreatorTXT;


// CDocumentFactoryText

class ATL_NO_VTABLE CDocumentFactoryText : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentFactoryText, &CLSID_DocumentFactoryText>,
	public IDocumentFactoryText,
	public CDocumentDecoderImpl<CDocumentFactoryText, CDocumentTypeCreatorTXT, IDocumentFactoryText, EDPLow, IDocumentCodec>,
	public CDocumentBuilderImpl<CDocumentFactoryText, FEATURES(g_aTextSupported), g_pszTypeNameText, IDI_TEXT_NEWDOC, g_pszFormatNameText, g_pszShellIconPathText, EDPAverage>,
	public CDesignerWizardImpl<DESWIZ_TEXT_NAME, DESWIZ_TEXT_DESC, 0, NULL, IDocumentFactoryText, 255>,
	public IThumbnailProvider
{
public:
	CDocumentFactoryText()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentFactoryText)

BEGIN_CATEGORY_MAP(CDocumentFactoryText)
	IMPLEMENTED_CATEGORY(CATID_DesignerWizard)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
	IMPLEMENTED_CATEGORY(CATID_DocumentBuilder)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
	IMPLEMENTED_CATEGORY(CATID_ThumbnailProvider)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentFactoryText)
	COM_INTERFACE_ENTRY(IDocumentFactoryText)
	COM_INTERFACE_ENTRY(IDesignerWizard)
	COM_INTERFACE_ENTRY(IDocumentDecoder)
	COM_INTERFACE_ENTRY(IDocumentBuilder)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
	COM_INTERFACE_ENTRY_IID(IID_IThumbnailProvider, IThumbnailProvider)
END_COM_MAP()


	// IDocumentBuilder methods
public:

	// IDocumentDecoder methods
public:
	HRESULT Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryText* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);

	// IDocumentEncoder methods
public:
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg);
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	// IDocumentFactoryText methods
public:
	STDMETHOD(Init)(BSTR a_bstrText, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
	{
		return InitEx(a_bstrText, ::SysStringLen(a_bstrText), a_bstrPrefix, a_pBase);
	}
	STDMETHOD(InitEx)(OLECHAR const* a_pText, ULONG a_nLength, BSTR a_bstrPrefix, IDocumentBase* a_pBase);

	// IDesignerWizard methods
public:
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(Category)(ILocalizedString** a_ppCategory);
	STDMETHOD(State)(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText);
	STDMETHOD(Config)(IConfig** a_ppConfig);
	STDMETHOD(Activate)(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryText* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase);

	// IThumbnailProvider methods
public:
	STDMETHOD_(ULONG, IThumbnailProvider::Priority)() { return 100; }
	STDMETHOD(GetThumbnail)(IDocument* a_pDoc, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo, HRESULT(fnRescaleImage)(ULONG a_nSrcSizeX, ULONG a_nSrcSizeY, DWORD const* a_pSrcData, bool a_bSrcAlpha, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds));
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentFactoryText), CDocumentFactoryText)
