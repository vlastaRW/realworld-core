// DocumentTransformationJScript.cpp : Implementation of CDocumentTransformationJScript

#include "stdafx.h"
#include "DocumentTransformationJScript.h"
#include <MultiLanguageString.h>
#include <activscp.h>


// CScriptedTransformation

class ATL_NO_VTABLE CScriptedTransformation : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedTransformation, &IID_IScriptedTransformation, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
public:
	CScriptedTransformation()
	{
	}

	void GetResult(IDocument** a_ppDoc)
	{
		if (m_pResult)
			m_pResult->QueryInterface(a_ppDoc);
	}

DECLARE_NOT_AGGREGATABLE(CScriptedTransformation)

BEGIN_COM_MAP(CScriptedTransformation)
	COM_INTERFACE_ENTRY(IScriptedTransformation)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	// IScriptedTransformation methods
public:
	STDMETHOD(put_Result)(IDispatch* pDocument)
	{
		m_pResult = pDocument;
		return S_OK;
	}

private:
	CComPtr<IDispatch> m_pResult;
};


#include "RWActiveScriptingSite.h"


static OLECHAR const CFGID_JS_SCRIPT[] = L"Script";
static OLECHAR const CFGID_JS_CFGSCRIPT[] = L"CfgScript";

#include "../RWProcessing/ShowConfigWithPreviewDlg.h"
#include "ConfigGUIJScript.h"


// CDocumentTransformationJScript

STDMETHODIMP CDocumentTransformationJScript::NameGet(ITransformationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
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

STDMETHODIMP CDocumentTransformationJScript::ConfigCreate(ITransformationManager* a_pManager, IConfig** a_ppDefaultConfig)
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
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CFG_CAPTION), CMultiLanguageString::GetAuto(L"[0409]Window caption[0405]Titulek okna"), CMultiLanguageString::GetAuto(L"[0409]Text in the caption of the window displayed during execution.[0405]Text v titulku onka konfiguračního dialogu."), CConfigValue(L"[0409]Configure Transformation[0405]Konfigurovat transformaci"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CFG_HELPTOPIC), CMultiLanguageString::GetAuto(L"[0409]Help topic[0405]Téma nápovědy"), CMultiLanguageString::GetAuto(L"[0409]Path to a html help topic that will be displayed if user clicks on a Help button. If this field is left blank, the Help button will be hidden.[0405]Cesta k tématu v html nápovědě, který bude zobrazen po kliknutí na tlačítko Nápověda. Pokud je toto pole prázdné, tlačítko bude skryté."), CConfigValue(L""), NULL, 0, NULL);

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

		CConfigCustomGUI<&CLSID_DocumentOperationJScript, CConfigGUIJScriptDlg, false>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTransformationJScript::CanActivate(ITransformationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
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

struct CJScriptPreviewMakerX : public IPreviewMaker
{
	CJScriptPreviewMakerX(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, BSTR a_bstrScriptText) :
		m_pManager(a_pManager), m_pDocument(a_pDocument), m_bstrScriptText(a_bstrScriptText), m_pConfig(a_pConfig), m_pStates(a_pStates)
	{
	}

	HRESULT MakePreview(RWHWND a_hParent, LCID a_tLocaleID, IDocument** a_ppPreviewDoc)
	{
		CComPtr<IDocumentBase> pBase;
		RWCoCreateInstance(pBase, __uuidof(DocumentBase));
		if (FAILED(m_pDocument->DocumentCopy(NULL, pBase, NULL, NULL)))
			return E_NOTIMPL;
		{
			 // copy location
			CComPtr<IStorageFilter> pLoc;
			m_pDocument->LocationGet(&pLoc);
			if (pLoc)
			{
				CComQIPtr<IDocument> pNew(pBase);
				if (pNew)
					pNew->LocationSet(pLoc);
			}
		}
		CComPtr<IDocument> pSrc;
		pBase->QueryInterface(&pSrc);
		try
		{
			static CLSID const CLSID_JScriptEngine = {0xf414c260, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};
			CComPtr<IActiveScript> pScripEngine;
			pScripEngine.CoCreateInstance(CLSID_JScriptEngine);
			CComQIPtr<IActiveScriptParse> pScriptParse(pScripEngine);
			if (pScripEngine == NULL || pScriptParse == NULL)
				return E_FAIL;

			CComObject<CScriptedTransformation>* pTrans = NULL;
			CComObject<CScriptedTransformation>::CreateInstance(&pTrans);
			CComPtr<IDispatch> pTr = pTrans;

			CComObject<CJScriptOperationContext>* pJSCtx = NULL;
			CComObject<CJScriptOperationContext>::CreateInstance(&pJSCtx);
			CComPtr<IOperationContext> pJSCtxTmp = pJSCtx;
			pJSCtx->Init(m_pStates);

			CComObject<CRWActiveScriptSite>* pSite = NULL;
			CComObject<CRWActiveScriptSite>::CreateInstance(&pSite);
			CComPtr<IActiveScriptSite> pSitePtr = pSite;
			pSite->Init(pScripEngine, m_pManager, pSrc, m_pConfig, pJSCtx, a_hParent, a_tLocaleID, pTr);

			if (FAILED(pScriptParse->InitNew()))
				return E_FAIL;

			if (FAILED(pScriptParse->ParseScriptText(m_bstrScriptText, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISVISIBLE, NULL, NULL)) ||
				FAILED(pScripEngine->SetScriptState(SCRIPTSTATE_STARTED)) ||
				FAILED(pScripEngine->SetScriptState(SCRIPTSTATE_CONNECTED)))
				return E_FAIL;
			pTrans->GetResult(a_ppPreviewDoc);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
		return S_OK;
	}

private:
	ITransformationManager* m_pManager;
	IDocument* m_pDocument;
	IConfig* m_pConfig;
	IOperationContext* m_pStates;
	BSTR m_bstrScriptText;
};


STDMETHODIMP CDocumentTransformationJScript::Activate(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
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

		CComObject<CScriptedTransformation>* pTrans = NULL;
		CComObject<CScriptedTransformation>::CreateInstance(&pTrans);
		CComPtr<IDispatch> pTr = pTrans;

		CComObject<CRWActiveScriptSite>* pSite = NULL;
		CComObject<CRWActiveScriptSite>::CreateInstance(&pSite);
		CComPtr<IActiveScriptSite> pSitePtr = pSite;
		CComPtr<IConfig> pConfig;
		a_pConfig->SubConfigGet(CComBSTR(CFGID_JS_SCRIPT), &pConfig);
		CComPtr<IConfig> pLiveConfig;
		pConfig->DuplicateCreate(&pLiveConfig);
		bool bUpdateConfig = true;
		pSite->Init(pScripEngine, a_pManager, a_pDocument, pLiveConfig, pJSCtx, a_hParent, a_tLocaleID, pTr);

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
				CJScriptPreviewMakerX cPreviewMaker(a_pManager, a_pDocument, pOpCfg, a_pStates, cScriptText);
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
			}

			pJSCtx->Apply();
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

		if (FAILED(hRes))
			return hRes;

		CComPtr<IDocument> pResDoc;
		pTrans->GetResult(&pResDoc);
		if (pResDoc == NULL)
			return E_FAIL;

		HRESULT hRes2 = pResDoc->DocumentCopy(a_bstrPrefix, a_pBase, NULL, NULL);
		if (FAILED(hRes2))
			return hRes2;

		if (a_bstrPrefix == NULL || a_bstrPrefix[0] == L'\0')
		{
			CComQIPtr<IDocument> pNew(a_pBase);
			if (pNew)
			{
				pNew->ClearDirty();
				 // copy location
				CComPtr<IStorageFilter> pLoc;
				pResDoc->LocationGet(&pLoc);
				if (pLoc)
				{
					if (pNew)
						pNew->LocationSet(pLoc);
				}
			}
		}

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

