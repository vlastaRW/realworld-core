
#pragma once

#include <SimpleLocalizedString.h>
#include <StringParsing.h>
#include <RWConceptDesignerExtension.h>


// CScriptedContext

class ATL_NO_VTABLE CScriptedContext : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedContext, &IID_IScriptedContext, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
	typedef IDispatchImpl<IScriptedContext, &IID_IScriptedContext, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff> CDispatchBase;

public:
	CScriptedContext()
	{
	}
	void Init(IOperationContext* a_pContext)
	{
		m_pContext = a_pContext;
	}

DECLARE_NOT_AGGREGATABLE(CScriptedContext)

BEGIN_COM_MAP(CScriptedContext)
	COM_INTERFACE_ENTRY(IScriptedContext)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IOperationContext), m_pContext.p)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IScriptedContext methods
public:
	STDMETHOD(GetState)(BSTR stateID, BSTR* pVal)
	{
		*pVal = NULL;
		CComPtr<ISharedState> pState;
		m_pContext->StateGet(stateID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		if (pState)
			pState->ToText(pVal);
		return S_OK;
	}
	STDMETHOD(SetState)(BSTR stateID, BSTR val)
	{
		CComPtr<ISharedState> pState;
		m_pContext->StateGet(stateID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		if (pState)
		{
			if (SUCCEEDED(pState->FromText(val)))
				m_pContext->StateSet(stateID, pState);
		}
		return S_OK;
	}
	STDMETHOD(get_Canceled)(VARIANT_BOOL* pVal)
	{
		*pVal = m_pContext->IsCancelled() == S_OK;
		return S_OK;
	}
	STDMETHOD(put_ErrorMessage)(BSTR message)
	{
		return m_pContext->SetErrorMessage(new CSimpleLocalizedString(CComBSTR(message).Detach()));
	}
	STDMETHOD(get_ItemIndex)(ULONG* pVal)
	{
		*pVal = 0;
		m_pContext->GetOperationInfo(pVal, NULL, NULL, NULL);
		return S_OK;
	}
	STDMETHOD(get_ItemsRemaining)(ULONG* pVal)
	{
		*pVal = 0;
		m_pContext->GetOperationInfo(NULL, pVal, NULL, NULL);
		return S_OK;
	}
	STDMETHOD(get_StepIndex)(ULONG* pVal)
	{
		*pVal = 0;
		m_pContext->GetOperationInfo(NULL, NULL, pVal, NULL);
		return S_OK;
	}
	STDMETHOD(get_StepsRemaining)(ULONG* pVal)
	{
		*pVal = 0;
		m_pContext->GetOperationInfo(NULL, NULL, NULL, pVal);
		return S_OK;
	}
	STDMETHOD(StopProcessing)(VARIANT_BOOL showErrorBox)
	{
		return showErrorBox != VARIANT_FALSE ? E_FAIL : E_RW_CANCELLEDBYUSER;
	}
	STDMETHOD(GetValue)(BSTR name, VARIANT* pVal)
	{
		try
		{
			CContextObjects::const_iterator i = m_cObjects.find(name);
			if (i == m_cObjects.end())
				return E_INVALIDARG;
			return ::VariantCopy(pVal, &i->second);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SetValue)(BSTR name, VARIANT val)
	{
		try
		{
			if (val.vt == VT_EMPTY || val.vt == VT_NULL)
				m_cObjects.erase(name);
			else
				m_cObjects[name] = val;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ClearValues)()
	{
		m_cObjects.clear();
		return S_OK;
	}

private:
	typedef std::map<CComBSTR, CComVariant> CContextObjects;

private:
	CComPtr<IOperationContext> m_pContext;
	CContextObjects m_cObjects;
};


