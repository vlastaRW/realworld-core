// DocumentTypeWildcards.h : Declaration of the CDocumentTypeWildcards

#pragma once
#include "RWStorage.h"


// CDocumentTypeWildcards

class ATL_NO_VTABLE CDocumentTypeWildcards : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentTypeWildcards, &CLSID_DocumentTypeWildcards>,
	public IDocumentTypeWildcards
{
public:
	CDocumentTypeWildcards() : m_hIconModule(NULL), m_nIconID(0), m_fnGetIcon(NULL)
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CDocumentTypeWildcards)
	COM_INTERFACE_ENTRY(IDocumentType)
	COM_INTERFACE_ENTRY(IDocumentTypeWildcards)
END_COM_MAP()


	// IDocumentType methods
public:
	STDMETHOD(UniqueIDGet)(BSTR* a_pbstrUniqueID);
	STDMETHOD(FilterNameGet)(ILocalizedString** a_ppFilterName);
	STDMETHOD(TypeNameGet)(BSTR a_bstrExt, ILocalizedString** a_ppTypeName);
	STDMETHOD(IconGet)(BSTR a_bstrExt, ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(IconPathGet)(BSTR a_bstrExt, BSTR* a_pbstrIconPath);
	STDMETHOD(DefaultExtensionGet)(BSTR* a_pbstrExtension);
	STDMETHOD(SupportedExtensionsGet)(IEnumStrings** a_ppSupportedExtensions);
	STDMETHOD(MatchFilename)(BSTR a_bstrFilename);

	// IDocumentTypeWildcards methods
public:
	STDMETHOD(Init)(ILocalizedString* a_pFormatName, BSTR a_bstrFilter);
	STDMETHOD(InitEx)(ILocalizedString* a_pFormatName, ILocalizedString* a_pTypeName, ULONG a_nSupportedExtensions, BSTR const* a_aSupportedExtensions, BSTR a_bstrIconPath, HMODULE a_hIconModule, UINT a_nIconID, BSTR a_bstrFilter);
	STDMETHOD(FilterGet)(BSTR* a_pbstrFilter);
	STDMETHOD(InitEx2)(ILocalizedString* a_pFormatName, ILocalizedString* a_pTypeName, ULONG a_nSupportedExtensions, BSTR const* a_aSupportedExtensions, BSTR a_bstrIconPath, HICON (a_fnGetIcon)(ULONG), BSTR a_bstrFilter);

	// internal methods
public:
	static int wildcmp(wchar_t const* wild, wchar_t const* string);
	static int wildcmp1(wchar_t const* wild, wchar_t const* string);

private:
	CComPtr<ILocalizedString> m_pFilterName;
	CComPtr<ILocalizedString> m_pTypeName;
	CComPtr<IEnumStrings> m_pSupportedExtensions;
	CComBSTR m_bstrIconPath;
	HMODULE m_hIconModule;
	UINT m_nIconID;
	HICON (*m_fnGetIcon)(ULONG);
	CComBSTR m_bstrFilter;
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentTypeWildcards), CDocumentTypeWildcards)
