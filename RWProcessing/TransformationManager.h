// TransformationManager.h : Declaration of the CTransformationManager

#pragma once
#include "resource.h"       // main symbols
#include "RWProcessing.h"
#include <WeakSingleton.h>
#include "CooperatingObjectsManager.h"


// CTransformationManager

class ATL_NO_VTABLE CTransformationManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTransformationManager, &CLSID_TransformationManager>,
	public CCooperatingObjectsManagerImpl<ITransformationManager, IDocumentTransformation, &CATID_DocumentTransformation, &__uuidof(DocumentTransformationNULL)>
{
public:
	CTransformationManager()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CTransformationManager)

BEGIN_COM_MAP(CTransformationManager)
	COM_INTERFACE_ENTRY(ITransformationManager)
	COM_INTERFACE_ENTRY(ILateConfigCreator)
END_COM_MAP()


	// ITransformationManager methods (specific only)
public:
	STDMETHOD(CanActivate)(ITransformationManager* a_pOverrideForItem, IDocument* a_pDocument, const TConfigValue* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(ITransformationManager* a_pOverrideForItem, IDocument* a_pDocument, const TConfigValue* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
	STDMETHOD(Visit)(ITransformationManager* a_pOverrideForItem, TConfigValue const* a_ptTransformationID, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor);
};

OBJECT_ENTRY_AUTO(__uuidof(TransformationManager), CTransformationManager)
