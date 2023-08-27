// DocumentOperationSequence.h : Declaration of the CDocumentOperationSequence

#pragma once
#include "resource.h"       // main symbols
#include "RWProcessing.h"



// CDocumentOperationSequence

class ATL_NO_VTABLE CDocumentOperationSequence :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationSequence, &CLSID_DocumentOperationSequence>,
	public IDocumentOperation,
	public ICustomOperationVisitor,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationSequence()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationSequence)

BEGIN_CATEGORY_MAP(CDocumentOperationSequence)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationSequence)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(ICustomOperationVisitor)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);

	// ICustomOperationVisitor methods
public:
	STDMETHOD(Visit)(IOperationManager* a_pManager, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor);

private:
	class ATL_NO_VTABLE CCustomNameString :
		public CComObjectRootEx<CComMultiThreadModel>,
		public ILocalizedString
	{
	public:
		void Init(ULONG a_nIndex, IConfig* a_pItemConfig)
		{
			m_nIndex = a_nIndex;
			m_pItemConfig = a_pItemConfig;
		}

	BEGIN_COM_MAP(CCustomNameString)
		COM_INTERFACE_ENTRY(ILocalizedString)
	END_COM_MAP()

		// ILocalizedString methods
	public:
		STDMETHOD(Get)(BSTR *a_pbstrString) { return GetLocalized(GetThreadLocale(), a_pbstrString); }
		STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR *a_pbstrString);

	private:
		ULONG m_nIndex;
		CComPtr<IConfig> m_pItemConfig;
	};

	class ATL_NO_VTABLE CCustomName :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IVectorItemName
	{
	public:

	BEGIN_COM_MAP(CCustomName)
		COM_INTERFACE_ENTRY(IVectorItemName)
	END_COM_MAP()

		// IVectorItemName methods
	public:
		STDMETHOD(Get)(ULONG a_nIndex, IConfig* a_pItemConfig, ILocalizedString** a_ppName)
		{
			try
			{
				*a_ppName = NULL;
				CComObject<CCustomNameString>* p = NULL;
				CComObject<CCustomNameString>::CreateInstance(&p);
				CComPtr<ILocalizedString> pTmp = p;
				p->Init(a_nIndex, a_pItemConfig);
				*a_ppName = pTmp.Detach();
				return S_OK;
			}
			catch (...)
			{
				return a_ppName ? E_UNEXPECTED: E_POINTER;
			}
		}
	};
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationSequence), CDocumentOperationSequence)
