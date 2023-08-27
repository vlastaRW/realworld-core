// ScriptingInterfaceManager.cpp : Implementation of CScriptingInterfaceManager

#include "stdafx.h"
#include "ScriptingInterfaceManager.h"


// CScriptingInterfaceManager

STDMETHODIMP CScriptingInterfaceManager::GetGlobalObjects(IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CheckState();

		ULONG nFactories = 0;
		if (m_pPlugIns) m_pPlugIns->Size(&nFactories);
		for (ULONG i = 0; i < nFactories; ++i)
		{
			CComPtr<IScriptingInterface> pInterface;
			m_pPlugIns->Get(i, __uuidof(IScriptingInterface), reinterpret_cast<void**>(&pInterface));
			if (pInterface)
				pInterface->GetGlobalObjects(this, a_pSite, a_pManager, a_pDocument, a_pConfig, a_pStates, a_hParent, a_tLocaleID);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptingInterfaceManager::GetInterfaceAdaptors(IScriptingSite* a_pSite, IDocument* a_pDocument)
{
	try
	{
		CheckState();

		ULONG nFactories = 0;
		if (m_pPlugIns) m_pPlugIns->Size(&nFactories);
		for (ULONG i = 0; i < nFactories; ++i)
		{
			CComPtr<IScriptingInterface> pInterface;
			m_pPlugIns->Get(i, __uuidof(IScriptingInterface), reinterpret_cast<void**>(&pInterface));
			if (pInterface)
				pInterface->GetInterfaceAdaptors(this, a_pSite, a_pDocument);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptingInterfaceManager::GetKeywords(IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary)
{
	try
	{
		CheckState();

		ULONG nFactories = 0;
		if (m_pPlugIns) m_pPlugIns->Size(&nFactories);
		for (ULONG i = 0; i < nFactories; ++i)
		{
			CComPtr<IScriptingInterface> pInterface;
			m_pPlugIns->Get(i, __uuidof(IScriptingInterface), reinterpret_cast<void**>(&pInterface));
			if (pInterface)
				pInterface->GetKeywords(this, a_pPrimary, a_pSecondary);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "ScriptedDocument.h"

STDMETHODIMP CScriptingInterfaceManager::WrapDocument(IScriptingInterfaceManager* a_pOverride, IDocument* a_pDocument, IScriptedDocument** a_ppWrapped)
{
	try
	{
		*a_ppWrapped = NULL;
		CComObject<CScriptedDocument>* p = NULL;
		CComObject<CScriptedDocument>::CreateInstance(&p);
		CComPtr<IScriptedDocument> pTmp = p;
		p->Init(a_pOverride ? a_pOverride : this, a_pDocument);
		*a_ppWrapped = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppWrapped ? E_UNEXPECTED : E_POINTER;
	}
}

class ATL_NO_VTABLE CJScriptArray : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IJScriptArrayInit, &IID_IJScriptArrayInit, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
	typedef IDispatchImpl<IJScriptArrayInit, &IID_IJScriptArrayInit, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff> CDispatchBase;

public:

	CJScriptArray()
	{
	}

DECLARE_NOT_AGGREGATABLE(CJScriptArray)

BEGIN_COM_MAP(CJScriptArray)
	COM_INTERFACE_ENTRY(IJScriptArrayInit)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	// IDispatch methods
public:
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
	{
		try
		{
			// TODO: locks?
			if (cNames == 1)
			{
				if (_wcsicmp(L"length", *rgszNames) == 0)
				{
					*rgDispId = 1;
					return S_OK;
				}
				size_t i = 0;
				LPCOLESTR p = *rgszNames;
				while (*p)
				{
					if (*p < L'0' || *p > L'9')
						break;
					i = i*10 + *p-L'0';
					++p;
				}
				if (*p == L'\0' && i < m_aItems.size())
				{
					*rgDispId = i+2;
					return S_OK;
				}
			}
			return CDispatchBase::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
	{
		try
		{
			// TODO: locks?
			if (dispIdMember == 1)
			{
				if (wFlags == DISPATCH_PROPERTYGET)
				{
					pVarResult->vt = VT_I4;
					pVarResult->intVal = m_aItems.size();
					return S_OK;
				}
			}
			else if (dispIdMember >= 2 && dispIdMember < LONG(m_aItems.size()+2))
			{
				if (wFlags == DISPATCH_PROPERTYGET)
				{
					return ::VariantCopy(pVarResult, &(m_aItems[dispIdMember-2]));
				}
			}
			return CDispatchBase::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IJScriptArrayInit methods
public:
	STDMETHOD(Add)(VARIANT a_tVal)
	{
		try
		{
			ObjectLock cLock(this);
			m_aItems.resize(m_aItems.size()+1);
			m_aItems[m_aItems.size()-1] = a_tVal;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddNumber)(LONG a_nVal)
	{
		try
		{
			ObjectLock cLock(this);
			m_aItems.resize(m_aItems.size()+1);
			m_aItems[m_aItems.size()-1] = a_nVal;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddString)(BSTR a_bstrVal)
	{
		try
		{
			ObjectLock cLock(this);
			m_aItems.resize(m_aItems.size()+1);
			m_aItems[m_aItems.size()-1] = a_bstrVal;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddObject)(IDispatch* a_pVal)
	{
		try
		{
			ObjectLock cLock(this);
			m_aItems.resize(m_aItems.size()+1);
			m_aItems[m_aItems.size()-1] = a_pVal;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddFloat)(float a_fVal)
	{
		try
		{
			ObjectLock cLock(this);
			m_aItems.resize(m_aItems.size()+1);
			m_aItems[m_aItems.size()-1] = a_fVal;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	std::vector<CComVariant> m_aItems;
};


STDMETHODIMP CScriptingInterfaceManager::CreateJScriptArray(IJScriptArrayInit** a_ppArray)
{
	try
	{
		*a_ppArray = NULL;
		CComObject<CJScriptArray>* p = NULL;
		CComObject<CJScriptArray>::CreateInstance(&p);
		CComPtr<IJScriptArrayInit> pTmp = p;
		*a_ppArray = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppArray ? E_UNEXPECTED : E_POINTER;
	}
}

void CScriptingInterfaceManager::CheckState()
{
	if (m_bInitiated)
		return;
	ObjectLock cLock(this);
	if (m_bInitiated)
		return;
	CComPtr<IPlugInCache> pPIC;
	RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
	CComPtr<IEnumGUIDs> pGUIDs;
	pPIC->InterfacesEnum(CATID_ScriptingInterface, __uuidof(IScriptingInterface), 0xffffffff, &m_pPlugIns, &pGUIDs);
	m_bInitiated = true;
}
