// MenuCommandsStructureRoot.cpp : Implementation of CMenuCommandsStructureRoot

#include "stdafx.h"
#include "MenuCommandsStructureRoot.h"
#include <SharedStringTable.h>


// CMenuCommandsStructureRoot

STDMETHODIMP CMenuCommandsStructureRoot::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_STRUCTUREROOT);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsStructureRoot::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsStructureRoot::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IStructuredDocument> pStrRoot;
		a_pDocument->QueryFeatureInterface(__uuidof(IStructuredDocument), reinterpret_cast<void**>(&pStrRoot));
		if (pStrRoot == NULL)
			return S_FALSE;
		CComPtr<IEnumGUIDs> pRoots;
		pStrRoot->StructuredRootsEnum(&pRoots);
		if (pRoots == NULL)
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pViews;
		RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
		a_pView->QueryInterfaces(__uuidof(IDesignerViewStructure), EQIFVisible, pViews);
		CComPtr<IDesignerViewStructure> pView;
		pViews->Get(0, __uuidof(IDesignerViewStructure), reinterpret_cast<void**>(&pView));
		if (pView == NULL)
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		GUID tID;
		for (ULONG i = 0; SUCCEEDED(pRoots->Get(i, &tID)); ++i)
		{
			CComObject<CDocumentMenuRootID>* p = NULL;
			CComObject<CDocumentMenuRootID>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(pView, pStrRoot, tID);
			pItems->Insert(pTmp);
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsStructureRoot::CDocumentMenuRootID::Name(ILocalizedString** a_ppText)
{
	return m_pDoc->NameGet(m_iid, a_ppText);
}

STDMETHODIMP CMenuCommandsStructureRoot::CDocumentMenuRootID::State(EMenuCommandState* a_peState)
{
	try
	{
		GUID tID = GUID_NULL;
		m_pView->RootIDGet(&tID);
		*a_peState = IsEqualIID(m_iid, tID) ? EMCSRadioChecked : EMCSRadio;
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsStructureRoot::CDocumentMenuRootID::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	return m_pView->RootIDSet(m_iid);
}
