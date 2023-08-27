// DocumentOperationJScript.cpp : Implementation of CDocumentOperationJScript

#include "stdafx.h"
#include "DocumentOperationJScript.h"
#include <MultiLanguageString.h>
#include <activscp.h>

#include "RWActiveScriptingSite.h"


static OLECHAR const CFGID_JS_SCRIPT[] = L"Script";
static OLECHAR const CFGID_JS_CFGSCRIPT[] = L"CfgScript";

#include "../RWProcessing/ShowConfigWithPreviewDlg.h"
#include "ConfigGUIJScript.h"


// CDocumentOperationJScript

STDMETHODIMP CDocumentOperationJScript::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]» JavaScript «[0405]» JavaScript «");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationJScript::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComPtr<ISubConfig> pSubCfg;
		RWCoCreateInstance(pSubCfg, __uuidof(ConfigInMemory));
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_JS_SCRIPT), CMultiLanguageString::GetAuto(L"[0409]Exe script[0405]Výkonný skript"), CMultiLanguageString::GetAuto(L"[0409]JavaScript that will be executed. Refer to the manual (http://wiki.rw-designer.com/Operation_JScript) to learn about scripting.[0405]JavaScript, který bude vykonán při spuštění. Více o skriptování se dozvíte v manuálu (http://wiki.rw-designer.com/Operation_JScript)."), CConfigValue(L"// place your custom JavaScript code here"), pSubCfg, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_JS_CFGSCRIPT), CMultiLanguageString::GetAuto(L"[0409]GUI Script[0405]GUI skript"), CMultiLanguageString::GetAuto(L"[0409]This JavaScript code should use the Configuration object to define a configuration dialog. http://wiki.rw-designer.com/Operation_JScript[0405]Tento JavaScript má za úkol nadefinovat konfigurační dialog pomocí objektu Configuration. http://wiki.rw-designer.com/Operation_JScript"), CConfigValue(L""), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CFG_DLGSIZEX), CMultiLanguageString::GetAuto(L"[0409]Window width[0405]Šířka okna"), nullptr, CConfigValue(-1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CFG_DLGSIZEY), CMultiLanguageString::GetAuto(L"[0409]Window height[0405]Výška okna"), nullptr, CConfigValue(-1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CFG_CAPTION), CMultiLanguageString::GetAuto(L"[0409]Window caption[0405]Titulek okna"), CMultiLanguageString::GetAuto(L"[0409]Text in the caption of the window displayed during execution.[0405]Text v titulku onka konfiguračního dialogu."), CConfigValue(L"[0409]Configure Operation[0405]Konfigurovat operaci"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CFG_HELPTOPIC), CMultiLanguageString::GetAuto(L"[0409]Help topic[0405]Téma nápovědy"), CMultiLanguageString::GetAuto(L"[0409]Path to a html help topic that will be displayed if user clicks on a Help button. If this field is left blank, the Help button will be hidden.[0405]Cesta k tématu v html nápovědě, který bude zobrazen po kliknutí na tlačítko Nápověda. Pokud je toto pole prázdné, tlačítko bude skryté."), CConfigValue(L""), NULL, 0, NULL);
		CComBSTR cCFGID_CFG_PREVIEWMODE(CFGID_CFG_PREVIEWMODE);
		pCfgInit->ItemIns1ofN(cCFGID_CFG_PREVIEWMODE, CMultiLanguageString::GetAuto(L"[0409]Display mode[0405]Způsob zobrazení"), CMultiLanguageString::GetAuto(L"[0409]Controls how is the original and the preview of the processed image displayed.[0405]Určuje, jak je zobrazen originál a náhled na zpracovný obrázek."), CConfigValue(CFGVAL_PM_AUTOSELECT), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_CFG_PREVIEWMODE, CConfigValue(CFGVAL_PM_PROCESSED), CMultiLanguageString::GetAuto(L"[0409]Processed[0405]Výsledek"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_CFG_PREVIEWMODE, CConfigValue(CFGVAL_PM_AUTOSELECT), CMultiLanguageString::GetAuto(L"[0409]Default[0405]Výchozí"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_CFG_PREVIEWMODE, CConfigValue(CFGVAL_PM_SPLITTED), CMultiLanguageString::GetAuto(L"[0409]Both[0405]Oba"), 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CFG_SPLITPOS), NULL, NULL, CConfigValue(-1.0f), NULL, 0, NULL);

		CComBSTR cCFGID_ICONID(CFGID_CFG_ICONID);
		CComPtr<IConfigItemCustomOptions> pCustIconIDs;
		RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
		if (pCustIconIDs != NULL)
			pCfgInit->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, CMultiLanguageString::GetAuto(L"[0409]Icon[0405]Ikona"), CMultiLanguageString::GetAuto(L"[0409]Icon assigned to the configuration window.[0405]Ikona zobrazená v konfiuračním dialogu."), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
		else
			pCfgInit->ItemInsSimple(cCFGID_ICONID, CMultiLanguageString::GetAuto(L"[0409]Icon[0405]Ikona"), CMultiLanguageString::GetAuto(L"[0409]Icon assigned to the configuration window.[0405]Ikona zobrazená v konfiuračním dialogu."), CConfigValue(GUID_NULL), NULL, 0, NULL);

		CComQIPtr<IViewManager> pViewMgr(a_pManager);
		if (pViewMgr == NULL)
			RWCoCreateInstance(pViewMgr, __uuidof(ViewManager));
		pViewMgr->InsertIntoConfigAs(pViewMgr, pCfgInit, CComBSTR(CFGID_CFG_PREVIEW), CMultiLanguageString::GetAuto(L"[0409]Preview[0405]Náhled"), CMultiLanguageString::GetAuto(L"[0409]Identifier of the view used for operation result preview.[0405]Pohled použitý pro náhled na výsledek operace."), 0, NULL);

		// TODO: scripted config GUI is not updated when config changes
		//CComPtr<ISubConfig> pHistory;
		//RWCoCreateInstance(pHistory, __uuidof(ConfigInMemory));
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_CFG_HISTORY), CMultiLanguageString::GetAuto(L"[0409]History[0405]Historie"), CMultiLanguageString::GetAuto(L"[0409]If enabled, previous selected values will be remembered.[0405]Je-li povoleno, budou se zaznamenávat dříve vybraná nastavení."), CConfigValue(true), pHistory, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationJScript, CConfigGUIJScriptDlg, false>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationJScript::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		if (a_pConfig == NULL)
			return S_OK;
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_JS_SCRIPT), &cVal);
		if (cVal.operator BSTR() == NULL || cVal.operator BSTR()[0] == L'\0')
			return S_FALSE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

struct CJScriptPreviewMaker : public IPreviewMaker
{
	CJScriptPreviewMaker(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, BSTR a_bstrScriptText) :
		m_pManager(a_pManager), m_pDocument(a_pDocument), m_bstrScriptText(a_bstrScriptText), m_pConfig(a_pConfig), m_pStates(a_pStates)
	{
	}

	HRESULT MakePreview(RWHWND a_hParent, LCID a_tLocaleID, IDocument** a_ppPreviewDoc)
	{
		CComPtr<IDocumentBase> pBase;
		RWCoCreateInstance(pBase, __uuidof(DocumentBase));
		if (FAILED(m_pDocument->DocumentCopy(NULL, pBase, NULL, NULL)))
			return E_NOTIMPL;
		pBase->QueryInterface(a_ppPreviewDoc);
		if (NULL == *a_ppPreviewDoc)
			return E_NOTIMPL;
		try
		{
			static CLSID const CLSID_JScriptEngine = {0xf414c260, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};
			CComPtr<IActiveScript> pScripEngine;
			pScripEngine.CoCreateInstance(CLSID_JScriptEngine);
			CComQIPtr<IActiveScriptParse> pScriptParse(pScripEngine);
			if (pScripEngine == NULL || pScriptParse == NULL)
				return E_FAIL;

			CComObject<CJScriptOperationContext>* pJSCtx = NULL;
			CComObject<CJScriptOperationContext>::CreateInstance(&pJSCtx);
			CComPtr<IOperationContext> pJSCtxTmp = pJSCtx;
			pJSCtx->Init(m_pStates);

			CComObject<CRWActiveScriptSite>* pSite = NULL;
			CComObject<CRWActiveScriptSite>::CreateInstance(&pSite);
			CComPtr<IActiveScriptSite> pSitePtr = pSite;
			pSite->Init(pScripEngine, m_pManager, *a_ppPreviewDoc, m_pConfig, pJSCtx, a_hParent, a_tLocaleID);

			if (FAILED(pScriptParse->InitNew()))
				return E_FAIL;

			CWriteLock<IDocument> cLock(*a_ppPreviewDoc);
			if (FAILED(pScriptParse->ParseScriptText(m_bstrScriptText, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISVISIBLE, NULL, NULL)) ||
				FAILED(pScripEngine->SetScriptState(SCRIPTSTATE_STARTED)) ||
				FAILED(pScripEngine->SetScriptState(SCRIPTSTATE_CONNECTED)))
				return E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
		return S_OK;
	}

private:
	IOperationManager* m_pManager;
	IDocument* m_pDocument;
	IConfig* m_pConfig;
	IOperationContext* m_pStates;
	BSTR m_bstrScriptText;
};


STDMETHODIMP CDocumentOperationJScript::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		ULONG nItemIndex = 0;
		ULONG nStepsRemaining = 1;
		if (a_pStates) a_pStates->GetOperationInfo(&nItemIndex, NULL, NULL, &nStepsRemaining);
		bool bInSequence = nItemIndex != 0;

		CConfigValue cScriptText;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_JS_SCRIPT), &cScriptText);
		CConfigValue cScriptGUI;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_JS_CFGSCRIPT), &cScriptGUI);

		// {f414c260-6ac0-11cf-b6d1-00aa00bbbb58}
		static CLSID const CLSID_JScriptEngine = {0xf414c260, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};
		CComPtr<IActiveScript> pScripEngine;
		pScripEngine.CoCreateInstance(CLSID_JScriptEngine);
		CComQIPtr<IActiveScriptParse> pScriptParse(pScripEngine);
		if (pScripEngine == NULL || pScriptParse == NULL)
			return E_FAIL;

		CComObject<CJScriptOperationContext>* pJSCtx = NULL;
		CComObject<CJScriptOperationContext>::CreateInstance(&pJSCtx);
		CComPtr<IOperationContext> pJSCtxTmp = pJSCtx;
		pJSCtx->Init(a_pStates);

		CComObject<CRWActiveScriptSite>* pSite = NULL;
		CComObject<CRWActiveScriptSite>::CreateInstance(&pSite);
		CComPtr<IActiveScriptSite> pSitePtr = pSite;
		CComPtr<IConfig> pConfig;
		a_pConfig->SubConfigGet(CComBSTR(CFGID_JS_SCRIPT), &pConfig);
		CComPtr<IConfig> pLiveConfig;
		pConfig->DuplicateCreate(&pLiveConfig);
		bool bUpdateConfig = true;
		pSite->Init(pScripEngine, a_pManager, a_pDocument, pLiveConfig, pJSCtx, a_hParent, a_tLocaleID);

		if (FAILED(pScriptParse->InitNew()))
			return E_FAIL;

		if (cScriptGUI.operator BSTR()[0] != L'\0' && (FAILED(pScriptParse->ParseScriptText(cScriptGUI, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISVISIBLE, NULL, NULL)) ||
			FAILED(pScripEngine->SetScriptState(SCRIPTSTATE_STARTED)) ||
			FAILED(pScripEngine->SetScriptState(SCRIPTSTATE_CONNECTED))))
			return E_FAIL;

		CComPtr<IConfig> pOpCfg;
		pSite->GetGlobalIConfig(&pOpCfg);
		ULONG nIDs = 0;
		if (pOpCfg)
		{
			CComPtr<IEnumStrings> pIDs;
			pOpCfg->ItemIDsEnum(&pIDs);
			if (pIDs) pIDs->Size(&nIDs);
		}
		if (nIDs)
		{
			if (!bInSequence && a_hParent)
			{
				CJScriptPreviewMaker cPreviewMaker(a_pManager, a_pDocument, pOpCfg, a_pStates, cScriptText);
				CConfigValue cRuntimeConfig;
				int nDlgRes = nStepsRemaining ?
					CShowConfigWithPreviewDlg<IDCONTINUE>(a_tLocaleID, pOpCfg, /*&bUpdateConfig, */a_pConfig, NULL, NULL, CComQIPtr<IViewManager>(a_pManager), a_pDocument, &cPreviewMaker).DoModal(a_hParent) :
					CShowConfigWithPreviewDlg<>(a_tLocaleID, pOpCfg, /*&bUpdateConfig, */a_pConfig, NULL, NULL, CComQIPtr<IViewManager>(a_pManager), a_pDocument, &cPreviewMaker).DoModal(a_hParent);
				switch (nDlgRes)
				{
				case IDOK:
					break;
				case IDCONTINUE:
					return S_FALSE;
				default:
					return E_RW_CANCELLEDBYUSER;
				}
			}
		}

		HRESULT hRes = S_OK;
		try
		{
			{
				CWriteLock<IBlockOperations> cLock(a_pDocument);

				EXCEPINFO tExceptInfo;
				ZeroMemory(&tExceptInfo, sizeof tExceptInfo);
				if (FAILED(pScriptParse->ParseScriptText(cScriptText, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISVISIBLE, NULL, &tExceptInfo)))
				{
					// TODO: free tExceptInfo
					throw E_FAIL;
				}

				if (FAILED(pScripEngine->SetScriptState(SCRIPTSTATE_STARTED)))
					throw E_FAIL;

				if (FAILED(pScripEngine->SetScriptState(SCRIPTSTATE_CONNECTED)))
					throw E_FAIL;

				hRes = pSite->M_ErrorCode();

				pJSCtx->Apply(a_pDocument);
			}

			//if (bCancelled)
			//	hRes = E_RW_CANCELLEDBYUSER;
		}
		catch (HRESULT e_hRes)
		{
			bUpdateConfig = false;
			hRes = e_hRes;
		}
		catch (...)
		{
			bUpdateConfig = false;
			hRes = E_FAIL;
		}
		pScripEngine->Close();
		if (bUpdateConfig)
			CopyConfigValues(pConfig, pLiveConfig);

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

HICON GetIconScript(ULONG a_nSize)
{
	static IRPathPoint const right[] =
	{
		{178.75, 50.75, 0, 0, -9.11273, -9.11272},
		{256, 128, 0, 0, 0, 0},
		{178.75, 205.25, -8.76668, 8.76668, 0, 0},
		{145.75, 205.25, -9.45878, -8.76668, 9.45878, 8.76668},
		{145.75, 172.25, 0, 0, -9.45878, 9.45877},
		{190, 128, 0, 0, 0, 0},
		{145.75, 83.75, -9.11272, -9.11272, 0, 0},
		{145.75, 50.75, 9.11272, -9.11272, -9.11272, 9.11272},
	};
	static IRPathPoint const left[] =
	{
		{77, 205, 0, 0, 9.11273, 9.11272},
		{0, 128, 0, 0, 0, 0},
		{77, 51, 8.76668, -8.76668, 0, 0},
		{110, 51, 9.45879, 8.76668, -9.45879, -8.76668},
		{110, 84, 0, 0, 9.45879, -9.45877},
		{66, 128, 0, 0, 0, 0},
		{110, 172, 9.11272, 9.11272, 0, 0},
		{110, 205, -9.11272, 9.11272, 9.11272, -9.11272},
	};
	static IRCanvas canvas2 = { 0, 0, 256, 256, 0, 0, NULL, NULL };
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas2, itemsof(left), left, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&canvas2, itemsof(right), right, pSI->GetMaterial(ESMScheme1Color1));
	return cRenderer.get();
}
