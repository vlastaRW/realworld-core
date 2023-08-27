// MenuCommandsVector.cpp : Implementation of CMenuCommandsVector

#include "stdafx.h"
#include "MenuCommandsVector.h"
#include <SharedStringTable.h>

const OLECHAR CFGID_VECTOR_ITEMS[] = L"Items";
const OLECHAR CFGID_VECTOR_SUBCOMMANDS[] = L"SubItem";

#include "ConfigGUIVector.h"
#include "ConfigGUIVectorItem.h"


// CMenuCommandsVector

STDMETHODIMP CMenuCommandsVector::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENUCOMMANDSVECTOR_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsVector::ConfigCreate(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		CComPtr<ISubConfigVector> pItems;
		RWCoCreateInstance(pItems, __uuidof(SubConfigVector));

		CComObject<CConfigPattern>* p = NULL;
		CComObject<CConfigPattern>::CreateInstance(&p);
		CComPtr<IConfig> pPattern = p;
		p->Init(a_pManager);

		CComObject<CCustomName>* p2 = NULL;
		CComObject<CCustomName>::CreateInstance(&p2);
		CComPtr<IVectorItemName> pName = p2;

		if (FAILED(pItems->InitName(pName, pPattern)))
			return E_FAIL;

		if (FAILED(pCfg->ItemInsSimple(CComBSTR(CFGID_VECTOR_ITEMS), _SharedStringTable.GetStringAuto(IDS_CFGID_VECTOR_ITEMS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VECTOR_ITEMS_DESC), CConfigValue(0L), pItems, 0, NULL)))
			return E_FAIL;

		CConfigCustomGUI<&CLSID_MenuCommandsVector, CConfigGUIVectorDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsVector::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;
		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_VECTOR_ITEMS), &cVal);
		for (LONG i = 0; i < cVal.operator LONG(); ++i)
		{
			OLECHAR szTmp[64] = L"";
			swprintf(szTmp, itemsof(szTmp), L"%s\\%08x\\%s", CFGID_VECTOR_ITEMS, i, CFGID_VECTOR_SUBCOMMANDS);
			CComBSTR bstrItem(szTmp);
			CConfigValue cID;
			a_pConfig->ItemValueGet(bstrItem, &cID);
			CComPtr<IConfig> pCfg;
			a_pConfig->SubConfigGet(bstrItem, &pCfg);
			CComPtr<IEnumUnknowns> pSubItems;
			a_pManager->CommandsEnum(a_pManager, cID, pCfg, a_pStates, a_pView, a_pDocument, &pSubItems);
			pItems->InsertFromEnum(pSubItems);
		}
		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

extern const GUID tVectorItemID = {0x377494cb, 0x3b3e, 0x40f1, {0x94, 0xb4, 0x95, 0xcc, 0x3f, 0x5c, 0xd3, 0x81}};

STDMETHODIMP CMenuCommandsVector::CConfigPattern::DuplicateCreate(IConfig** a_ppCopiedConfig)
{
	try
	{
		*a_ppCopiedConfig = NULL;
		if (m_pPattern)
			return m_pPattern->DuplicateCreate(a_ppCopiedConfig);

		ObjectLock cLock(this);
		RWCoCreateInstance(m_pPattern, __uuidof(ConfigWithDependencies));
		m_pManager->InsertIntoConfigAs(m_pManager, m_pPattern, CComBSTR(CFGID_VECTOR_SUBCOMMANDS), _SharedStringTable.GetStringAuto(IDS_CFGID_VECTOR_SUBCOMMANDS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VECTOR_SUBCOMMANDS_DESC), 0, NULL);
		CConfigCustomGUI<&tVectorItemID, CConfigGUIVectorItemDlg>::FinalizeConfig(m_pPattern);
		return m_pPattern->DuplicateCreate(a_ppCopiedConfig);
	}
	catch (...)
	{
		return a_ppCopiedConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsVector::CCustomNameString::GetLocalized(LCID a_tLCID, BSTR *a_pbstrString)
{
	try
	{
		*a_pbstrString = NULL;
		CComBSTR bstr;
		if (m_pItemConfig)
		{
			CConfigValue cVal;
			m_pItemConfig->ItemValueGet(CComBSTR(CFGID_VECTOR_SUBCOMMANDS), &cVal);
			CComPtr<IConfigItem> pItem;
			m_pItemConfig->ItemGetUIInfo(CComBSTR(CFGID_VECTOR_SUBCOMMANDS), __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
			if (pItem)
			{
				CComPtr<ILocalizedString> pStr;
				pItem->ValueGetName(cVal, &pStr);
				if (pStr)
				{
					pStr->GetLocalized(a_tLCID, &bstr);
				}
			}
		}
		OLECHAR sz[256] = L"";
		swprintf(sz, bstr == NULL ? L"#%i" : L"#%i - %s", m_nIndex+1, bstr);
		*a_pbstrString = CComBSTR(sz).Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrString ? E_UNEXPECTED : E_POINTER;
	}
}

