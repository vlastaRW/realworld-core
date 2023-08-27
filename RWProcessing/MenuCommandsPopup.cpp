// MenuCommandsPopup.cpp : Implementation of CMenuCommandsPopup

#include "stdafx.h"
#include "MenuCommandsPopup.h"
#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include <RWConceptDesignerExtension.h>

const OLECHAR CFGID_POPUP_SUBCOMMANDS[] = L"Commands";
const OLECHAR CFGID_POPUP_NAME[] = L"Name";
const OLECHAR CFGID_POPUP_DESCRITPION[] = L"Description";
const OLECHAR CFGID_POPUP_ICONID[] = L"IconID";
const OLECHAR CFGID_POPUP_ICONFROMRADIO[] = L"IconFromRadio";
const OLECHAR CFGID_POPUP_TEXTINTOOLBAR[] = L"TextInToolbar";

#include "ConfigGUIPopup.h"


// CMenuCommandsPopup

STDMETHODIMP CMenuCommandsPopup::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENUCOMMANDSPOPUP_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsPopup::ConfigCreate(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_POPUP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_NAME_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_NAME_DESC), CConfigValue(L"SubMenu"), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_POPUP_DESCRITPION), _SharedStringTable.GetStringAuto(IDS_CFGID_DESCRITPION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DESCRITPION_DESC), CConfigValue(L""), NULL, 0, NULL);

		// icon
		CComBSTR cCFGID_ICONID(CFGID_POPUP_ICONID);
		CComPtr<IConfigItemCustomOptions> pCustIconIDs;
		RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
		if (pCustIconIDs != NULL)
			pCfg->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
		else
			pCfg->ItemInsSimple(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_POPUP_ICONFROMRADIO), _SharedStringTable.GetStringAuto(IDS_CFGID_POPUP_ICONFROMRADIO_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_POPUP_ICONFROMRADIO_DESC), CConfigValue(false), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_POPUP_TEXTINTOOLBAR), _SharedStringTable.GetStringAuto(IDS_CFGID_POPUP_TEXTINTOOLBAR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_POPUP_TEXTINTOOLBAR_DESC), CConfigValue(false), NULL, 0, NULL);

		a_pManager->InsertIntoConfigAs(a_pManager, pCfg, CComBSTR(CFGID_POPUP_SUBCOMMANDS), _SharedStringTable.GetStringAuto(IDS_CFGID_POPUP_SUBCOMMANDS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_POPUP_SUBCOMMANDS_DESC), 0, NULL);

		CConfigCustomGUI<&CLSID_MenuCommandsPopup, CConfigGUIPopupDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsPopup::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CConfigValue cName;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_POPUP_NAME), &cName);
		CConfigValue cDesc;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_POPUP_DESCRITPION), &cDesc);
		CConfigValue cIconID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_POPUP_ICONID), &cIconID);
		CComBSTR bstrCFGID_POPUP_SUBCOMMANDS(CFGID_POPUP_SUBCOMMANDS);
		CConfigValue cSubItem;
		a_pConfig->ItemValueGet(bstrCFGID_POPUP_SUBCOMMANDS, &cSubItem);
		CComPtr<IConfig> pSubCfg;
		a_pConfig->SubConfigGet(bstrCFGID_POPUP_SUBCOMMANDS, &pSubCfg);
		CConfigValue cRadioIcon;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_POPUP_ICONFROMRADIO), &cRadioIcon);
		CConfigValue cToolbarText;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_POPUP_TEXTINTOOLBAR), &cToolbarText);

		CComObject<CDocumentMenuCommand>* p = NULL;
		CComObject<CDocumentMenuCommand>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;

		CComPtr<ILocalizedString> pName;
		pName.Attach(new CMultiLanguageString(cName.Detach().bstrVal));
		CComPtr<ILocalizedString> pDesc;
		pDesc.Attach(new CMultiLanguageString(cDesc.Detach().bstrVal));

		CComPtr<IEnumUnknowns> pSubItems;
		a_pManager->CommandsEnum(a_pManager, cSubItem, pSubCfg, a_pStates, a_pView, a_pDocument, &pSubItems);

		p->Init(pName, pDesc, cIconID, pSubItems, cRadioIcon, cToolbarText);

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
		pItems->Insert(p);

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsPopup::CDocumentMenuCommand::Name(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		(*a_ppText = m_pName)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsPopup::CDocumentMenuCommand::Description(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		(*a_ppText = m_pDesc)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsPopup::CDocumentMenuCommand::IconID(GUID* a_pIconID)
{
	try
	{
		m_tLastIconID = m_tIconID;
		HRESULT hRes = S_OK;
		if (m_bRadioIcon && m_pSubItems)
		{
			CComPtr<IDocumentMenuCommand> p;
			for (ULONG i = 0; SUCCEEDED(m_pSubItems->Get(i, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&p))); ++i, p = NULL)
			{
				EMenuCommandState eState = EMCSNormal;
				p->State(&eState);
				if ((eState&EMCSRadioChecked) == EMCSRadioChecked)
				{
					hRes = p->IconID(&m_tLastIconID);
					break;
				}
			}
		}
		*a_pIconID = m_tLastIconID;
		return hRes;
	}
	catch (...)
	{
		return a_pIconID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsPopup::CDocumentMenuCommand::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		if (IsEqualGUID(m_tLastIconID, m_tIconID))
		{
			CComPtr<IDesignerFrameIcons> pIcons;
			RWCoCreateInstance(pIcons, __uuidof(DesignerFrameIconsManager));
			return pIcons->GetIcon(m_tIconID, a_nSize, a_phIcon);
		}
		if (m_bRadioIcon && m_pSubItems)
		{
			CComPtr<IDocumentMenuCommand> p;
			for (ULONG i = 0; SUCCEEDED(m_pSubItems->Get(i, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&p))); ++i, p = NULL)
			{
				EMenuCommandState eState = EMCSNormal;
				p->State(&eState);
				if ((eState&EMCSRadioChecked) == EMCSRadioChecked)
				{
					GUID tID = GUID_NULL;
					p->IconID(&tID);
					return tID == m_tLastIconID ? p->Icon(a_nSize, a_phIcon) : E_FAIL;
				}
			}
		}
		return E_FAIL;
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsPopup::CDocumentMenuCommand::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsPopup::CDocumentMenuCommand::SubCommands(IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;
		if (m_pSubItems == NULL)
			return S_FALSE;

		(*a_ppSubCommands = m_pSubItems)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsPopup::CDocumentMenuCommand::State(EMenuCommandState* a_peState)
{
	try
	{
		*a_peState = m_bToolbarText ? static_cast<EMenuCommandState>(EMCSSubMenu|EMCSShowButtonText) : EMCSSubMenu;
		if (m_bRadioIcon && m_pSubItems)
		{
			CComPtr<IDocumentMenuCommand> p;
			for (ULONG i = 0; SUCCEEDED(m_pSubItems->Get(i, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&p))); ++i, p = NULL)
			{
				EMenuCommandState eState = EMCSNormal;
				p->State(&eState);
				if ((eState&EMCSRadioChecked) == EMCSRadioChecked)
				{
					if (eState&EMCSDisabled)
						*a_peState = static_cast<EMenuCommandState>(EMCSDisabled|*a_peState);
					break;
				}
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsPopup::CDocumentMenuCommand::Execute(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	return E_NOTIMPL;
}

