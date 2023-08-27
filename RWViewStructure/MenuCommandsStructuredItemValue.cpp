// MenuCommandsStructuredItemValue.cpp : Implementation of CMenuCommandsStructuredItemValue

#include "stdafx.h"
#include "MenuCommandsStructuredItemValue.h"
#include <SharedStringTable.h>


// CMenuCommandsStructuredItemValue

STDMETHODIMP CMenuCommandsStructuredItemValue::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_STRUCTUREDITEMVALUE);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsStructuredItemValue::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
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

STDMETHODIMP CMenuCommandsStructuredItemValue::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IEnumUnknownsInit> pViews;
		RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
		a_pView->QueryInterfaces(__uuidof(IDesignerViewStructure), EQIFVisible, pViews);
		CComPtr<IDesignerViewStructure> pView;
		pViews->Get(0, __uuidof(IDesignerViewStructure), reinterpret_cast<void**>(&pView));
		if (pView == NULL)
			return S_FALSE;

		CComPtr<IEnumUnknowns> pSel;
		pView->SelectionGet(&pSel);
		if (pSel == NULL)
			return S_FALSE;
		ULONG nSelSize = 0;
		pSel->Size(&nSelSize);
		if (nSelSize == 0)
			return S_FALSE;

		IID iidStructure = __uuidof(IStructuredRoot);
		pView->RootIDGet(&iidStructure);

		CComPtr<IStructuredRoot> pStrRoot;
		a_pDocument->QueryFeatureInterface(iidStructure, reinterpret_cast<void**>(&pStrRoot));
		if (pStrRoot == NULL)
			return S_FALSE;

		std::vector<CComPtr<IItemBool> > cEnable;
		std::vector<CComPtr<IItemBool> > cDisable;

		for (ULONG i = 0; i < nSelSize; ++i)
		{
			CComPtr<IComparable> pItem;
			pSel->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			CComPtr<IItemBool> pBool;
			pStrRoot->ItemFeatureGet(pItem, __uuidof(IItemBool), reinterpret_cast<void**>(&pBool));
			if (pBool == NULL)
				continue;
			boolean bVal = false;
			if (FAILED(pBool->ValueGet(&bVal)))
				continue;
			if (bVal)
				cDisable.push_back(pBool);
			else
				cEnable.push_back(pBool);
		}

		if (cEnable.empty() && cDisable.empty())
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		if (!cEnable.empty())
		{
			CComObject<CBoolItem>* p = NULL;
			CComObject<CBoolItem>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pDocument, cEnable, true);
			pItems->Insert(pTmp);
		}

		if (!cDisable.empty())
		{
			CComObject<CBoolItem>* p = NULL;
			CComObject<CBoolItem>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pDocument, cDisable, false);
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



STDMETHODIMP CMenuCommandsStructuredItemValue::CBoolItem::Name(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		*a_ppText = _SharedStringTable.GetString(m_bVal ? IDS_ENABLE_NAME : IDS_DISABLE_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsStructuredItemValue::CBoolItem::Description(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		*a_ppText = _SharedStringTable.GetString(m_bVal ? IDS_ENABLE_DESC : IDS_DISABLE_DESC);
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsStructuredItemValue::CBoolItem::IconID(GUID* a_pIconID)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsStructuredItemValue::CBoolItem::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsStructuredItemValue::CBoolItem::State(EMenuCommandState* a_peState)
{
	try
	{
		*a_peState = EMCSNormal;
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsStructuredItemValue::CBoolItem::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CWriteLock<IBlockOperations> cLock(m_pDoc);
		for (std::vector<CComPtr<IItemBool> >::const_iterator i = m_cItems.begin(); i != m_cItems.end(); ++i)
		{
			(*i)->ValueSet(m_bVal);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

