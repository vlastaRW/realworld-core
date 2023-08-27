// CustomNewFileTemplates.h : Declaration of the CCustomNewFileTemplates

#pragma once
#include "resource.h"       // main symbols
#include <RWConceptDesignerExtension.h>
#include <WeakSingleton.h>


// 4314C4C2-AF13-49F8-989A-33CBF497B891
extern __declspec(selectany) CLSID const CLSID_CustomNewFileTemplates = { 0x4314c4c2, 0xaf13, 0xba02, { 0x98, 0x9a, 0x33, 0xcb, 0xf4, 0x97, 0xb8, 0x91 } };

// CCustomNewFileTemplates

class ATL_NO_VTABLE CCustomNewFileTemplates : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CCustomNewFileTemplates, &CLSID_CustomNewFileTemplates>,
	public IDesignerFrameTools,
	public IDocumentCreator
{
public:
	CCustomNewFileTemplates();
	~CCustomNewFileTemplates();

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CCustomNewFileTemplates)

BEGIN_CATEGORY_MAP(CCustomNewFileTemplates)
	IMPLEMENTED_CATEGORY(CATID_DocumentCreator)
//	IMPLEMENTED_CATEGORY(CATID_DesignerFrameTools)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CCustomNewFileTemplates)
	COM_INTERFACE_ENTRY(IDesignerFrameTools)
	COM_INTERFACE_ENTRY(IDocumentCreator)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDocumentCreator methods
public:
	STDMETHOD(DefaultConfig)(IConfig** UNREF(a_ppConfig))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IDocumentCreator::Size)(ULONG* a_pnCount) {return TemplatesSize(a_pnCount);}
	STDMETHOD(IDocumentCreator::Name)(ULONG a_nIndex, ILocalizedString** a_ppName) {return TemplatesName(a_nIndex, a_ppName);}
	STDMETHOD(IDocumentCreator::HelpText)(ULONG a_nIndex, ILocalizedString** a_ppHelpText) {return TemplatesHelpText(a_nIndex, a_ppHelpText);}
	STDMETHOD(IDocumentCreator::Icon)(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon) {return TemplatesIcon(a_nIndex, a_nSize, a_phIcon);}
	STDMETHOD(Category)(ULONG a_nIndex, ILocalizedString** a_ppCategory);
	STDMETHOD(CheckFeatures)(ULONG a_nIndex, ULONG a_nCount, const IID* a_aiidRequired);
	STDMETHOD(DocumentCreate)(ULONG a_nIndex, RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, BSTR a_bstrPrefix, IDocumentBase* a_pBase);

	// IDesignerFrameTools methods
public:
	STDMETHOD(IDesignerFrameTools::Size)(ULONG* a_pnCount) {return ToolsSize(a_pnCount);}
	STDMETHOD(IDesignerFrameTools::Name)(ULONG a_nIndex, ILocalizedString** a_ppName) {return ToolsName(a_nIndex, a_ppName);}
	STDMETHOD(IDesignerFrameTools::HelpText)(ULONG a_nIndex, ILocalizedString** a_ppHelpText) {return ToolsHelpText(a_nIndex, a_ppHelpText);}
	STDMETHOD(IDesignerFrameTools::Icon)(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon) {return ToolsIcon(a_nIndex, a_nSize, a_phIcon);}
	STDMETHOD(Activate)(ULONG a_nIndex, RWHWND a_hFrameWnd, LCID a_tLocaleID, IDocument* a_pDocument);

private:
	HRESULT TemplatesSize(ULONG* a_pnCount);
	HRESULT TemplatesName(ULONG a_nIndex, ILocalizedString** a_ppName);
	HRESULT TemplatesHelpText(ULONG a_nIndex, ILocalizedString** a_ppHelpText);
	HRESULT TemplatesIcon(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon);
	HRESULT ToolsSize(ULONG* a_pnCount);
	HRESULT ToolsName(ULONG a_nIndex, ILocalizedString** a_ppName);
	HRESULT ToolsHelpText(ULONG a_nIndex, ILocalizedString** a_ppHelpText);
	HRESULT ToolsIcon(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon);

	void InternInit();

private:
	struct SItem
	{
		CComBSTR bstrPath;
		CComPtr<ILocalizedString> pName;
		CComPtr<ILocalizedString> pDesc;
	};
	typedef vector<SItem> CItems;

private:
	bool m_bInitialized;
	CItems m_cItems;
};

OBJECT_ENTRY_AUTO(CLSID_CustomNewFileTemplates, CCustomNewFileTemplates)
