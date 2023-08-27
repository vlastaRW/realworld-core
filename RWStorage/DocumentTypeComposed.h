// DocumentTypeComposed.h : Declaration of the CDocumentTypeComposed

#pragma once
#include "RWStorage.h"


// CDocumentTypeComposed

class ATL_NO_VTABLE CDocumentTypeComposed : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentTypeComposed, &CLSID_DocumentTypeComposed>,
	public IDocumentTypeComposed
{
public:
	CDocumentTypeComposed() : m_hIconModule(NULL), m_nIconID(0)
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CDocumentTypeComposed)
	COM_INTERFACE_ENTRY(IDocumentType)
	COM_INTERFACE_ENTRY(IDocumentTypeComposed)
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

	// IDocumentTypeComposed methods
public:
	STDMETHOD(Init)(BSTR a_bstrUniqueID, ILocalizedString* a_pFilterName);
	STDMETHOD(InitEx)(BSTR a_bstrUniqueID, ILocalizedString* a_pFilterName, ILocalizedString* a_pTypeName, BSTR a_bstrIconPath, HMODULE a_hIconModule, UINT a_nIconID);
	STDMETHOD(InitAsAllFiles)();
	STDMETHOD(InitAsAllSupportedFiles)();
	STDMETHOD(DocTypesAddFromList)(ULONG a_nCount, IDocumentType** a_apTypes);
	STDMETHOD(DocTypesAddFromEnum)(IEnumUnknowns* a_pTypes);
	STDMETHOD(DocTypesGet)(IEnumUnknowns** a_ppTypes);

private:
	typedef std::vector<CComPtr<IDocumentType> > CSubTypes;

private:
	CComBSTR m_bstrUniqueID;
	CComPtr<ILocalizedString> m_pFilterName;
	CComPtr<ILocalizedString> m_pTypeName;
	CComBSTR m_bstrIconPath;
	CComBSTR m_bstrFilter;
	CSubTypes m_cAllSubTypes;
	CSubTypes m_cSubTypes;
	HMODULE m_hIconModule;
	UINT m_nIconID;
	CComPtr<IEnumStringsInit> m_pSupportedExtensions;
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentTypeComposed), CDocumentTypeComposed)
