// InputManager.h : Declaration of the CInputManager

#pragma once
#include "resource.h"       // main symbols
#include <PlugInCache.h>
#include <WeakSingleton.h>


// CInputManager

class ATL_NO_VTABLE CInputManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CInputManager, &CLSID_InputManager>,
	public IInputManager,
	public IGlobalConfigFactory
{
public:
	CInputManager()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CInputManager)

BEGIN_CATEGORY_MAP(CInputManager)
	IMPLEMENTED_CATEGORY(CATID_GlobalConfigFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CInputManager)
	COM_INTERFACE_ENTRY(IInputManager)
	COM_INTERFACE_ENTRY(IGlobalConfigFactory)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IInputManager methods
public:
	STDMETHOD(DocumentTypesEnum)(IEnumUnknowns** a_ppDocumentTypes);
	STDMETHOD(DocumentTypesEnumEx)(IUnknown* a_pBuilderSpec, IEnumUnknowns** a_ppDocumentTypes);
	STDMETHOD(DocumentCreate)(IStorageFilter* a_pSource, IBlockOperations* a_pOwner, IDocument** a_ppDocument);
	STDMETHOD(DocumentCreateEx)(IUnknown* a_pBuilderSpec, IStorageFilter* a_pSource, IBlockOperations* a_pOwner, IDocument** a_ppDocument);
	STDMETHOD(DocumentCreateData)(IUnknown* a_pBuilderSpec, IStorageFilter* a_pSource, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
	STDMETHOD(DocumentCreateDataEx)(IUnknown* a_pBuilderSpec, ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pSource, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);
	STDMETHOD(GetCompatibleBuilders)(ULONG a_nCount, IID const* a_aiidRequired, IEnumUnknowns** a_ppBuilders);

	STDMETHOD(SaveOptionsGet)(IDocument* a_pDocument, IConfig** a_ppSaveOptions, IEnumUnknowns** a_ppFormatFilters, IStorageFilterWindowListener** a_ppWindowListener);
	STDMETHOD(SaveEncoder)(IConfig* a_pSaveOptions, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg);
	STDMETHOD(Save)(IDocument* a_pDocument, IConfig* a_pSaveOptions, IStorageFilter* a_pSaveCopyAsDestination);

	STDMETHOD(FindBestEncoder)(IDocument* a_pDocument, GUID* a_pEncID, IConfig** a_ppEncCfg) { return FindBestEncoderEx(a_pDocument, 0, NULL, NULL, a_pEncID, a_ppEncCfg); }
	STDMETHOD(FindBestEncoderEx)(IDocument* a_pDocument, ULONG a_nExtra, BSTR const* a_abstrIDs, float const* a_afWeights, GUID* a_pEncID, IConfig** a_ppEncCfg);

	// IGlobalConfigFactory methods
public:
	STDMETHOD(Interactive)(BYTE* a_pPriority) { if (a_pPriority) *a_pPriority = 80; return S_OK; }
	STDMETHOD(Name)(ILocalizedString** a_ppName);
	STDMETHOD(Description)(ILocalizedString** a_ppDesc);
	STDMETHOD(Config)(IConfig** a_ppConfig);

private:
	void InitConfig();

private:
	CComPtr<IConfig> m_pConfig;
};

OBJECT_ENTRY_AUTO(__uuidof(InputManager), CInputManager)
