// DesignerViewFactoryFileBrowser.h : Declaration of the CDesignerViewFactoryFileBrowser

#pragma once
#include "resource.h"       // main symbols
#include <RWConceptDesignerView.h>
#include <RWProcessing.h>
#include <WeakSingleton.h>


// D408ED4E-447B-43EF-B050-3A150D92D863
extern __declspec(selectany) CLSID const CLSID_DesignerViewFactoryFileBrowser = { 0xd408ed4e, 0x447b, 0x43ef, { 0xb0, 0x50, 0x3a, 0x15, 0x0d, 0x92, 0xd8, 0x63 } };

// CDesignerViewFactoryFileBrowser

class ATL_NO_VTABLE CDesignerViewFactoryFileBrowser :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryFileBrowser, &CLSID_DesignerViewFactoryFileBrowser>,
	public IDesignerViewFactory,
	public IDocumentMenuCommands
{
public:
	CDesignerViewFactoryFileBrowser()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CDesignerViewFactoryFileBrowser)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryFileBrowser)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryFileBrowser)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);

	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pContext, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

	GUID GetIconID(CComBSTR const& a_bstrFilterID)
	{
		ObjectLock cLock(this);
		std::map<CComBSTR, GUID>::const_iterator i = m_cIconIDs.find(a_bstrFilterID);
		if (i == m_cIconIDs.end())
		{
			GUID t;
			CoCreateGuid(&t);
			m_cIconIDs[a_bstrFilterID] = t;
			return t;
		}
		return i->second;
	}

private:
	std::map<CComBSTR, GUID> m_cIconIDs;
};

OBJECT_ENTRY_AUTO(CLSID_DesignerViewFactoryFileBrowser, CDesignerViewFactoryFileBrowser)
