
#include "stdafx.h"
#include "RWDesignerCore.h"
#include <RWProcessing.h>
#include "DesignerFrameMenuCommandsManager.h"


typedef void (FnEnum)(IOperationContext* a_pStates, IConfig* a_pConfig, IDocument* a_pDocument, IDesignerView* a_pView, CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands);
typedef void (FnConfig)(IConfigWithDependencies* a_pConfig);

#include "MenuCommandsFile.h"
#include "MenuCommandsTools.h"
#include "MenuCommandsHelp.h"
#include "MenuCommandsEdit.h"
#include "MenuCommandsView.h"
#include "MenuCommandsMisc.h"


struct SFrameCommandInfo
{
	GUID tID;
	UINT uIDName;
	FnEnum* pfnEnum;
	FnConfig* pfnConfig;
};
static SFrameCommandInfo const s_aFrameCommandInfo[] =
{
	{MenuCommandsNewOpenSaveID, IDS_MENUCOMMANDS_FILEOPS, EnumNewOpenSave, NULL},
	{MenuCommandsSaveAsID, IDS_MENUCOMMANDS_SAVEAS, EnumSaveAs, NULL},
	{MenuCommandsCloseID, IDS_MENUCOMMANDS_CLOSE, EnumClose, NULL},
	{MenuCommandsExitID, IDS_MENUCOMMANDS_EXIT, EnumExit, NULL},
	{MenuCommandsRecentID, IDS_MENUCOMMANDS_RECENT, EnumRecent, NULL},
	{MenuCommandsUndoID, IDS_MENUCOMMANDS_UNDO, EnumUndo, NULL},
	{MenuCommandsRedoID, IDS_MENUCOMMANDS_REDO, EnumRedo, NULL},
	{MenuCommandsClipboardID, IDS_MENUCOMMANDS_CLIPBOARD, EnumClipboard, NULL},
	{MenuCommandsSelectAllID, IDS_MENUCOMMANDS_SELECTALL, EnumSelectAll, NULL},
	{MenuCommandsInvertSelectionID, IDS_MENUCOMMANDS_INVERTSELECTION, EnumInvertSelection, NULL},
	{MenuCommandsDeleteID, IDS_MENUCOMMANDS_DELETE, EnumDelete, NULL},
	{MenuCommandsDuplicateID, IDS_MENUCOMMANDS_DUPLICATE, EnumDuplicate, NULL},
	{MenuCommandsStatusBarID, IDS_MENUCOMMANDS_STATUS, EnumStatusBar, NULL},
	{MenuCommandsLayoutCfgID, IDS_MENUCOMMANDS_LAYOUTCFG, EnumLayoutCfg, NULL},
	{MenuCommandsLayoutsID, IDS_MENUCOMMANDS_LAYOUTS, EnumLayouts, NULL},
	{MenuCommandsManageLayoutsID, IDS_MENUCOMMANDS_LAYOUTSMNG, EnumManageLayouts, NULL},
	{MenuCommandsOptionsID, IDS_MENUCOMMANDS_OPTIONS, EnumOptions, NULL},
	{MenuCommandsDesignerToolsID, IDS_MENUCOMMANDS_FRAMETOOLS, EnumDesignerTools, NULL},
	{MenuCommandsLocalHelpID, IDS_MENUCOMMANDS_HELP, EnumLocalHelp, NULL},
	{MenuCommandsAskOnlineID, IDS_MENUCOMMANDS_ASKONLINE, EnumAskOnline, NULL},
	{MenuCommandsCustomID, IDS_MENUCOMMANDS_CUSTOM, EnumCustom, ConfigCustom},
	{MenuCommandsContextID, IDS_MENUCOMMANDS_CONTEXT, EnumContext, NULL},
	{MenuCommandsAboutID, IDS_MENUCOMMANDS_ABOUT, EnumAbout, NULL},
	{MenuCommandsExecuteCommandID, IDS_MENUCOMMANDS_EXECCMD, EnumExecuteCommand, ConfigExecuteCommand},
};


// CDesignerFrameMenuCommandsManager

STDMETHODIMP CDesignerFrameMenuCommandsManager::CreateConfigEx(IMenuCommandsManager* a_pOverrideForItem, TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		for (SFrameCommandInfo const* p = s_aFrameCommandInfo; p != s_aFrameCommandInfo+itemsof(s_aFrameCommandInfo); ++p)
		{
			if (IsEqualGUID(p->tID, a_ptControllerID->guidVal))
			{
				if (p->pfnConfig)
				{
					CComPtr<IConfigWithDependencies> pTmp;
					RWCoCreateInstance(pTmp, __uuidof(ConfigWithDependencies));
					p->pfnConfig(pTmp);
					pTmp->Finalize(NULL);
					*a_ppConfig = pTmp.Detach();
				}
				return S_OK;
			}
		}
		return m_pManager->CreateConfigEx(a_pOverrideForItem, a_ptControllerID, a_ppConfig);
	}
	catch (...)
	{
		return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameMenuCommandsManager::ItemGetCount(ULONG* a_pnCount)
{
	try
	{
		ULONG nCount = 0;
		HRESULT hRes = m_pManager->ItemGetCount(&nCount);
		*a_pnCount = nCount+itemsof(s_aFrameCommandInfo);
		return hRes;
	}
	catch (...)
	{
		return a_pnCount == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameMenuCommandsManager::ItemIDGetDefault(TConfigValue* a_ptDefaultOpID)
{
	return m_pManager->ItemIDGetDefault(a_ptDefaultOpID);
}

STDMETHODIMP CDesignerFrameMenuCommandsManager::ItemIDGet(IMenuCommandsManager* a_pOverrideForItem, ULONG a_nIndex, TConfigValue* a_ptOperationID, ILocalizedString** a_ppName)
{
	try
	{
		if (a_nIndex < itemsof(s_aFrameCommandInfo))
		{
			*a_ppName = NULL;
			a_ptOperationID->eTypeID = ECVTGUID;
			a_ptOperationID->guidVal = s_aFrameCommandInfo[a_nIndex].tID;
			*a_ppName = _SharedStringTable.GetString(s_aFrameCommandInfo[a_nIndex].uIDName);
			return S_OK;
		}
		else
		{
			return m_pManager->ItemIDGet(a_pOverrideForItem ? a_pOverrideForItem : this, a_nIndex-itemsof(s_aFrameCommandInfo), a_ptOperationID, a_ppName);
		}
	}
	catch (...)
	{
		return a_ppName == NULL || a_ptOperationID == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameMenuCommandsManager::InsertIntoConfigAs(IMenuCommandsManager* a_pOverrideForItem, IConfigWithDependencies* a_pConfig, BSTR a_bstrID, ILocalizedString* a_pItemName, ILocalizedString* a_pItemDesc, ULONG a_nItemConditions, TConfigOptionCondition const* a_aItemConditions)
{
	try
	{
		CComPtr<ISubConfigSwitchLate> pSubCfg;
		RWCoCreateInstance(pSubCfg, __uuidof(SubConfigSwitchLate));
		HRESULT hRes = pSubCfg->Init(a_pOverrideForItem ? a_pOverrideForItem : this);
		if (FAILED(hRes)) return hRes;
		CConfigValue cDefault;
		hRes = m_pManager->ItemIDGetDefault(&cDefault);
		if (FAILED(hRes)) return hRes;
		hRes = a_pConfig->ItemIns1ofN(a_bstrID, a_pItemName, a_pItemDesc, cDefault, pSubCfg);
		if (FAILED(hRes)) return hRes;

		ULONG nCount = 0;
		hRes = ItemGetCount(&nCount);
		if (FAILED(hRes))
			return hRes;

		for (ULONG i = 0; i != nCount; ++i)
		{
			CComPtr<ILocalizedString> pStr;
			CConfigValue cVal;
			ItemIDGet(a_pOverrideForItem, i, &cVal, &pStr);
			a_pConfig->ItemOptionAdd(a_bstrID, cVal, pStr, a_nItemConditions, a_aItemConditions);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameMenuCommandsManager::CommandsEnum(IMenuCommandsManager* a_pOverrideForItem, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		for (SFrameCommandInfo const* p = s_aFrameCommandInfo; p != s_aFrameCommandInfo+itemsof(s_aFrameCommandInfo); ++p)
		{
			if (IsEqualGUID(p->tID, a_ptOperationID->guidVal))
			{
				CComPtr<IEnumUnknownsInit> pInit;
				RWCoCreateInstance(pInit, __uuidof(EnumUnknowns));
				p->pfnEnum(a_pStates, a_pConfig, a_pDocument, a_pView, m_pFrame, pInit);
				*a_ppSubCommands = pInit.Detach();
				return S_OK;
			}
		}
		return m_pManager->CommandsEnum(a_pOverrideForItem ? a_pOverrideForItem : this, a_ptOperationID, a_pConfig, a_pStates, a_pView, a_pDocument, a_ppSubCommands);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

