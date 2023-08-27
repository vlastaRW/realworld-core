// ScriptingInterfaceBasics.cpp : Implementation of CScriptingInterfaceBasics

#include "stdafx.h"
#include "ScriptingInterfaceBasics.h"

#include "ScriptedOperation.h"
#include "ScriptedApplication.h"
#include "ScriptedConfiguration.h"
#include "ScriptedContext.h"


// CScriptingInterfaceBasics

static OLECHAR g_szAppName[MAX_PATH] = L"";

STDMETHODIMP CScriptingInterfaceBasics::InitGlobals(BSTR a_bstrAppName)
{
	if (a_bstrAppName)
	{
		wcsncpy(g_szAppName, a_bstrAppName, MAX_PATH-1);
		g_szAppName[MAX_PATH-1] = L'\0';
	}
	else
	{
		g_szAppName[0] = L'\0';
	}
	return S_OK;
}

STDMETHODIMP CScriptingInterfaceBasics::GetGlobalObjects(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pContext, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		{
			CComObject<CScriptedApplication>* p = NULL;
			CComObject<CScriptedApplication>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			p->Init(a_pScriptingMgr, a_hParent, a_tLocaleID, g_szAppName);
			a_pSite->AddItem(CComBSTR(L"Application"), pTmp);
		}
		{
			CComPtr<IScriptedDocument> pTmp;
			a_pScriptingMgr->WrapDocument(a_pScriptingMgr, a_pDocument, &pTmp);
			a_pSite->AddItem(CComBSTR(L"Document"), pTmp);
		}
		{
			CComObject<CScriptedOperation>* p = NULL;
			CComObject<CScriptedOperation>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			CComQIPtr<IOperationManager> pOpsMgr(a_pManager);
			if (pOpsMgr == NULL)
				RWCoCreateInstance(pOpsMgr, __uuidof(OperationManager));
			CComQIPtr<ITransformationManager> pTrnMgr(a_pManager);
			if (pTrnMgr == NULL)
				RWCoCreateInstance(pTrnMgr, __uuidof(TransformationManager));
			p->Init(a_pScriptingMgr, pOpsMgr, pTrnMgr, a_tLocaleID, a_hParent);
			a_pSite->AddItem(CComBSTR(L"Operation"), pTmp);
		}
		{
			CComObject<CScriptedConfiguration>* p = NULL;
			CComObject<CScriptedConfiguration>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			p->Init(a_pConfig, a_pScriptingMgr);
			a_pSite->AddItem(CComBSTR(L"Configuration"), pTmp);
		}
		{
			CComObject<CScriptedContext>* p = NULL;
			CComObject<CScriptedContext>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			p->Init(a_pContext);
			a_pSite->AddItem(CComBSTR(L"Context"), pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptingInterfaceBasics::GetInterfaceAdaptors(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument)
{
	// TODO: implement some basic interfaces? - structure, subdocument?
	return S_OK;
}

STDMETHODIMP CScriptingInterfaceBasics::GetKeywords(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary)
{
	try
	{
		a_pPrimary->Insert(CComBSTR(L"Application"));
		a_pPrimary->Insert(CComBSTR(L"Document"));
		a_pPrimary->Insert(CComBSTR(L"Operation"));
		a_pPrimary->Insert(CComBSTR(L"Configuration"));
		a_pPrimary->Insert(CComBSTR(L"Context"));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
