// DesignerFrameToolsShellAssociations.h : Declaration of the CDesignerFrameToolsShellAssociations

#pragma once
#include "resource.h"       // main symbols
#include <RWConceptDesignerExtension.h>


// DE631A13-02F1-4947-A8B1-157E58E3DDB5
extern __declspec(selectany) CLSID const CLSID_DesignerFrameToolsShellAssociations = { 0xde631a13, 0x02f1, 0x4947, { 0xa8, 0xb1, 0x15, 0x7e, 0x58, 0xe3, 0xdd, 0xb5 } };

// CDesignerFrameToolsShellAssociations

class ATL_NO_VTABLE CDesignerFrameToolsShellAssociations : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerFrameToolsShellAssociations, &CLSID_DesignerFrameToolsShellAssociations>,
	public IDesignerFrameTools
{
public:
	CDesignerFrameToolsShellAssociations()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerFrameToolsShellAssociations)

BEGIN_CATEGORY_MAP(CDesignerFrameToolsShellAssociations)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameTools)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerFrameToolsShellAssociations)
	COM_INTERFACE_ENTRY(IDesignerFrameTools)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDesignerFrameTools methods
public:
	STDMETHOD(Size)(ULONG* a_pnCount);
	STDMETHOD(Name)(ULONG a_nIndex, ILocalizedString** a_ppName);
	STDMETHOD(HelpText)(ULONG a_nIndex, ILocalizedString** a_ppHelpText);
	STDMETHOD(Icon)(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(Activate)(ULONG a_nIndex, RWHWND a_hFrameWnd, LCID a_tLocaleID, IDocument* a_pDocument);
};

OBJECT_ENTRY_AUTO(CLSID_DesignerFrameToolsShellAssociations, CDesignerFrameToolsShellAssociations)
