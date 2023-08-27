// MenuCommandsAutoOperation.h : Declaration of the CMenuCommandsAutoOperation

#pragma once
#include "resource.h"       // main symbols

#include "RWProcessing.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CMenuCommandsAutoOperation

class ATL_NO_VTABLE CMenuCommandsAutoOperation :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsAutoOperation, &CLSID_MenuCommandsAutoOperation>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsAutoOperation()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsAutoOperation)

BEGIN_CATEGORY_MAP(CMenuCommandsAutoOperation)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsAutoOperation)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CAutoCommands :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IEnumUnknowns
	{
	public:
		CAutoCommands()
		{
		}
		void Init(IConfig* a_pConfig, IOperationManager* a_pOpsMgr, IOperationContext* a_pStates, IDocument* a_pDocument, IDesignerView* a_pView);

	BEGIN_COM_MAP(CAutoCommands)
		COM_INTERFACE_ENTRY(IEnumUnknowns)
	END_COM_MAP()

		// IEnumUnknowns methods
	public:
		STDMETHOD(Size)(ULONG* a_pnSize);
		STDMETHOD(Get)(ULONG a_nIndex, REFIID a_iid, void** a_ppItem);
		STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, REFIID a_iid, void** a_apItems);

	private:
		CComPtr<IConfig> m_pConfig;
		CComPtr<IEnumGUIDs> m_pCLSIDs;
		CComPtr<IOperationManager> m_pOpsMgr;
		CComPtr<IOperationContext> m_pStates;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDesignerView> m_pView;
	};

	class ATL_NO_VTABLE CCommand :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		CCommand()
		{
		}
		void Init(CLSID a_tCLSID, IConfig* a_pItemCfg, IOperationManager* a_pOpsMgr, IOperationContext* a_pStates, IDocument* a_pDocument, IDesignerView* a_pView)
		{
			RWCoCreateInstance(m_pAO, a_tCLSID);
			m_pItemCfg = a_pItemCfg;
			m_pOpsMgr = a_pOpsMgr;
			m_pStates = a_pStates;
			m_pDocument = a_pDocument;
			m_pView = a_pView;
		}

	BEGIN_COM_MAP(CCommand)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		STDMETHOD(Description)(ILocalizedString** a_ppText);
		STDMETHOD(IconID)(GUID* a_pIconID);
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel);
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands) { return E_NOTIMPL; }
		STDMETHOD(State)(EMenuCommandState* a_peState);
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IAutoOperation> m_pAO;
		CComPtr<IConfig> m_pItemCfg;
		CComPtr<IOperationManager> m_pOpsMgr;
		CComPtr<IOperationContext> m_pStates;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDesignerView> m_pView;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsAutoOperation), CMenuCommandsAutoOperation)
