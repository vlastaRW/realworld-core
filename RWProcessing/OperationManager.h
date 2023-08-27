// OperationManager.h : Declaration of the COperationManager

#pragma once
#include "resource.h"       // main symbols
#include "RWProcessing.h"
#include <WeakSingleton.h>
#include "CooperatingObjectsManager.h"


// COperationManager

class ATL_NO_VTABLE COperationManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<COperationManager, &CLSID_OperationManager>,
	public CCooperatingObjectsManagerImpl<IOperationManager, IDocumentOperation, &CATID_DocumentOperation, &__uuidof(DocumentOperationNULL)>
{
public:
	COperationManager()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(COperationManager)

BEGIN_COM_MAP(COperationManager)
	COM_INTERFACE_ENTRY(IOperationManager)
	COM_INTERFACE_ENTRY(ILateConfigCreator)
END_COM_MAP()


	// IOperationManager methods (specific only)
public:
	STDMETHOD(CanActivate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, const TConfigValue* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, const TConfigValue* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
	STDMETHOD(Visit)(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor);
};

OBJECT_ENTRY_AUTO(__uuidof(OperationManager), COperationManager)
