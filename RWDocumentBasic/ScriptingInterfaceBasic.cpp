// ScriptingInterfaceBasic.cpp : Implementation of CScriptingInterfaceBasic

#include "stdafx.h"
#include "ScriptingInterfaceBasic.h"


// CScriptingInterfaceBasic

class ATL_NO_VTABLE CScriptedText : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedText, &IID_IScriptedText, &LIBID_RWDocumentBasicLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
public:
	CScriptedText()
	{
	}
	void Init(IDocument* a_pDoc, IDocumentText* a_pText)
	{
		m_pDoc = a_pDoc;
		m_pText = a_pText;
	}

DECLARE_NOT_AGGREGATABLE(CScriptedText)

BEGIN_COM_MAP(CScriptedText)
	COM_INTERFACE_ENTRY(IScriptedText)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IDocument), m_pDoc.p)
END_COM_MAP()


// IScriptedText methods
public:
	STDMETHOD(get_All)(BSTR* pVal)
	{
		return m_pText->TextGet(pVal);
	}
	STDMETHOD(put_All)(BSTR newVal)
	{
		return m_pText->TextSet(newVal);
	}
	STDMETHOD(get_LineCount)(ULONG* pCount)
	{
		return m_pText->LinesGetCount(pCount);
	}
	STDMETHOD(GetLine)(ULONG a_nIndex, BSTR* a_pbstrLine)
	{
		return m_pText->LineGet(a_nIndex, a_pbstrLine);
	}
	STDMETHOD(ReplaceLine)(ULONG a_nIndex, BSTR a_bstrLine)
	{
		return m_pText->LineSet(a_nIndex, a_bstrLine);
	}
	STDMETHOD(InsertLine)(ULONG a_nIndex, BSTR a_bstrLine)
	{
		return m_pText->LineIns(a_nIndex, a_bstrLine);
	}
	STDMETHOD(DeleteLine)(ULONG a_nIndex)
	{
		return m_pText->LineDel(a_nIndex);
	}

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentText> m_pText;
};


// CScriptingInterfaceBasic

STDMETHODIMP CScriptingInterfaceBasic::GetGlobalObjects(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	return S_FALSE;
}

STDMETHODIMP CScriptingInterfaceBasic::GetInterfaceAdaptors(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument)
{
	try
	{
		CComPtr<IDocumentText> pDT;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentText), reinterpret_cast<void**>(&pDT));
		if (pDT)
		{
			CComObject<CScriptedText>* p = NULL;
			CComObject<CScriptedText>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			p->Init(a_pDocument, pDT);
			a_pSite->AddItem(CComBSTR(L"Text"), pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptingInterfaceBasic::GetKeywords(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary)
{
	try
	{
		a_pSecondary->Insert(CComBSTR(L"Text"));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

