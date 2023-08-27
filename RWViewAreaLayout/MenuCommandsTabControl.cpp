// MenuCommandsTabControl.cpp : Implementation of CMenuCommandsTabControl

#include "stdafx.h"
#include "MenuCommandsTabControl.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <Win32LangEx.h>


// CMenuCommandsTabControl

STDMETHODIMP CMenuCommandsTabControl::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_TABCONTROL_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_TABSMENU_SHOWTEXT[] = L"ShowTabName";
static OLECHAR const CFGID_TABSMENU_TABID[] = L"TabID";

STDMETHODIMP CMenuCommandsTabControl::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TABSMENU_SHOWTEXT), _SharedStringTable.GetStringAuto(IDS_CFGID_TABSMENU_SHOWTEXT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TABSMENU_SHOWTEXT_DESC), CConfigValue(false), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TABSMENU_TABID), CMultiLanguageString::GetAuto(L"[0409]Tab ID[0405]ID záložky"), CMultiLanguageString::GetAuto(L"[0409]Identifier of the tabs controlled.[0405]Identifikátor záložek, které budou ovládány."), CConfigValue(L""), NULL, 0, NULL);

		// finalize the initialization of the config
		pCfgInit->Finalize(NULL);
		//CConfigCustomGUI<&CLSID_DesignerViewFactoryTab, CConfigGUITabDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsTabControl::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), IDesignerView* a_pView, IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CConfigValue cTabID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TABSMENU_TABID), &cTabID);

		CComPtr<IEnumUnknownsInit> pViews;
		RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
		a_pView->QueryInterfaces(__uuidof(IDesignerViewTabControl), EQIFVisible, pViews);
		CComPtr<IDesignerViewTabControl> pView;
		ULONG i = 0;
		while (true)
		{
			pViews->Get(i, __uuidof(IDesignerViewTabControl), reinterpret_cast<void**>(&pView));
			if (pView == NULL)
				return S_FALSE;
			CComBSTR bstrID;
			pView->TabID(&bstrID);
			if (bstrID == cTabID.operator BSTR())
				break;
			++i;
			pView = NULL;
		}

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		CConfigValue cShowName;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TABSMENU_SHOWTEXT), &cShowName);

		ULONG nItems = 0;
		pView->ItemCount(&nItems);
		for (ULONG i = 0; i < nItems; ++i)
		{
			CComObject<CDocumentMenuItem>* p = NULL;
			CComObject<CDocumentMenuItem>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(pView, i, cShowName, nItems == 1);
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


// CMenuCommandsTabControl::CDocumentMenuItem

STDMETHODIMP CMenuCommandsTabControl::CDocumentMenuItem::Name(ILocalizedString** a_ppText)
{
	return m_pDVTC->NameGet(m_nIndex, a_ppText);
}

STDMETHODIMP CMenuCommandsTabControl::CDocumentMenuItem::Description(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		(*a_ppText = this)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsTabControl::CDocumentMenuItem::GetLocalized(LCID a_tLCID, BSTR *a_pbstrString)
{
	try
	{
		*a_pbstrString = NULL;
		CComPtr<ILocalizedString> pName;
		m_pDVTC->NameGet(m_nIndex, &pName);
		CComBSTR bstrName;
		pName->GetLocalized(a_tLCID, &bstrName);
		OLECHAR szText[256] = L"";
		OLECHAR szTempl[128] = L"";
		Win32LangEx::LoadStringW(_pModule->get_m_hInst(), m_bSingleItem ? IDS_TABCONTROL_ONOFF : IDS_TABCONTROL_DESC, szTempl, itemsof(szTempl), LANGIDFROMLCID(a_tLCID));
		_swprintf(szText, szTempl, bstrName);
		CComBSTR bstrText(szText);
		*a_pbstrString = bstrText.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrString ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsTabControl::CDocumentMenuItem::IconID(GUID* a_pIconID)
{
	return m_pDVTC->IconIDGet(m_nIndex, a_pIconID);
}

STDMETHODIMP CMenuCommandsTabControl::CDocumentMenuItem::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	return m_pDVTC->IconGet(m_nIndex, a_nSize, a_phIcon);
}

STDMETHODIMP CMenuCommandsTabControl::CDocumentMenuItem::State(EMenuCommandState* a_peState)
{
	try
	{
		ULONG nActive = 0xffffffff;
		m_pDVTC->ActiveIndexGet(&nActive);
		*a_peState = m_bSingleItem ? (nActive == m_nIndex ? EMCSChecked : EMCSNormal) : (nActive == m_nIndex ? EMCSRadioChecked : EMCSRadio);
		if (m_bTextInToolbar)
			*a_peState = static_cast<EMenuCommandState>(EMCSShowButtonText | *a_peState);
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsTabControl::CDocumentMenuItem::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	if (m_bSingleItem)
	{
		ULONG n = 0xffffffff;
		m_pDVTC->ActiveIndexGet(&n);
		if (n == m_nIndex)
			return m_pDVTC->ActiveIndexSet(0xffffffff);
	}
	return m_pDVTC->ActiveIndexSet(m_nIndex);
}
