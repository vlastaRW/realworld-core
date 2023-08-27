
#pragma once 

class ATL_NO_VTABLE CRWActiveScriptSite :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IActiveScriptSite
{
public:
	CRWActiveScriptSite() : m_nErrorCode(S_OK)
	{
	}

	void Init(IActiveScript* a_pActiveScript, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID, IDispatch* a_pXformResult = NULL)
	{
		//m_pManager = a_pManager;
		m_pDocument = a_pDocument;
		m_pConfig = a_pConfig;
		m_pStates = a_pStates;
		m_hParent = a_hParent;
		m_tLocaleID = a_tLocaleID;
		CComPtr<IScriptingInterfaceManager> pScripting;
		RWCoCreateInstance(pScripting, __uuidof(ScriptingInterfaceManager));
		CComObjectStack<CReceiver> cReceiver;
		cReceiver.Init(m_cGlobalObjects);
		pScripting->GetGlobalObjects(&cReceiver, a_pManager, a_pDocument, a_pConfig, a_pStates, a_hParent, a_tLocaleID);
		if (a_pXformResult)
			cReceiver.AddItem(CComBSTR(L"Transformation"), a_pXformResult);

		a_pActiveScript->SetScriptSite(this);
		for (CGlobalObjects::const_iterator i = m_cGlobalObjects.begin(); i != m_cGlobalObjects.end(); ++i)
			a_pActiveScript->AddNamedItem(i->first.c_str(), SCRIPTITEM_ISVISIBLE);
	}

	BSTR M_ErrorSource() const { return m_bstrErrorSource; }
	BSTR M_ErrorDescription() const { return m_bstrErrorDescription; }
	BSTR M_ErrorLineText() const { return m_bstrErrorLine; }
	LONG M_ErrorColumn() const { return m_nErrorColumn; }
	ULONG M_ErrorLine() const { return m_nErrorLine; }
	HRESULT M_ErrorCode() const { return m_nErrorCode; }
	void GetGlobalIConfig(IConfig** a_ppCfg) const
	{
		for (CGlobalObjects::const_iterator i = m_cGlobalObjects.begin(); i != m_cGlobalObjects.end(); ++i)
			if (SUCCEEDED(i->second->QueryInterface(a_ppCfg)))
				return;
	}

BEGIN_COM_MAP(CRWActiveScriptSite)
	COM_INTERFACE_ENTRY(IActiveScriptSite)
END_COM_MAP()


	// IActiveScriptSite methods
public:
	STDMETHOD(GetLCID)(LCID *plcid)
	{
		try
		{
			*plcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);//m_tLocaleID;
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
   	STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti)
	{
		try
		{
			if (ppiunkItem != NULL) *ppiunkItem = NULL;
			if (ppti != NULL) *ppti = NULL;
			if (pstrName == NULL) return E_INVALIDARG;

			CGlobalObjects::const_iterator i = m_cGlobalObjects.find(pstrName);
			if (i == m_cGlobalObjects.end())
				return TYPE_E_ELEMENTNOTFOUND;

			if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
			{
				(*ppiunkItem = i->second)->AddRef();
			}
			if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
			{
				// TODO: implement ITypeInfo
			}

			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
	STDMETHOD(GetDocVersionString)(BSTR *pbstrVersion)
	{
		try
		{
			*pbstrVersion = NULL;
			*pbstrVersion = CComBSTR(L"DocumentOperationJScript 1.0").Detach();
			return S_OK;
		}
		catch (...)
		{
			return pbstrVersion == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(OnScriptTerminate)(const VARIANT* UNREF(pvarResult), const EXCEPINFO* UNREF(pexcepinfo))
	{
		return S_OK;
	}
	STDMETHOD(OnStateChange)(SCRIPTSTATE UNREF(ssScriptState))
	{
		return S_OK;
	}
	STDMETHOD(OnScriptError)(IActiveScriptError* a_pScriptError)
	{
		EXCEPINFO tExcepInfo;
		DWORD context = 0;
		m_bstrErrorSource.Empty();
		m_bstrErrorDescription.Empty();
		m_bstrErrorLine.Empty();
		m_nErrorColumn = 0;
		m_nErrorLine = 0;
		m_nErrorCode = S_OK;
		ZeroMemory(&tExcepInfo, sizeof tExcepInfo);
		a_pScriptError->GetExceptionInfo(&tExcepInfo);
		if (tExcepInfo.pfnDeferredFillIn != NULL)
			tExcepInfo.pfnDeferredFillIn(&tExcepInfo);
		m_nErrorCode = tExcepInfo.scode;
		a_pScriptError->GetSourceLineText(&m_bstrErrorLine);
		a_pScriptError->GetSourcePosition(&context, &m_nErrorLine, &m_nErrorColumn);

		m_bstrErrorDescription.Attach(tExcepInfo.bstrDescription);
		m_bstrErrorSource.Attach(tExcepInfo.bstrSource);
		SysFreeString(tExcepInfo.bstrHelpFile);
		if (m_bstrErrorDescription.Length())
		{
			CComPtr<ILocalizedStringInit> pStr;
			RWCoCreateInstance(pStr, __uuidof(LocalizedString));
			pStr->Insert(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT), m_bstrErrorDescription);
			m_pStates->SetErrorMessage(pStr);
		}

		return /*m_nErrorCode*/S_OK;
	}
	STDMETHOD(OnEnterScript)()
	{
		return S_OK;
	}
	STDMETHOD(OnLeaveScript)()
	{
		return S_OK;
	}

private:
	typedef std::map<std::wstring, CComPtr<IUnknown> > CGlobalObjects;

	class ATL_NO_VTABLE CReceiver :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IScriptingSite
	{
	public:
		void Init(CGlobalObjects& a_cObjects)
		{
			m_pObjects = &a_cObjects;
		}

	BEGIN_COM_MAP(CReceiver)
		COM_INTERFACE_ENTRY(IScriptingSite)
	END_COM_MAP()

		// IScriptingSite methods
	public:
		STDMETHOD(AddItem)(BSTR a_bstrName, IDispatch* a_pItem)
		{
			(*m_pObjects)[std::wstring(a_bstrName)] = a_pItem;
			return S_OK;
		}

	private:
		CGlobalObjects* m_pObjects;
	};


private:
	CGlobalObjects m_cGlobalObjects;

	// global objects
	//CComPtr<IOperationManager> m_pManager;
	CComPtr<IDocument> m_pDocument;
	CComPtr<IConfig> m_pConfig;
	CComPtr<IOperationContext> m_pStates;
	RWHWND m_hParent;
	LCID m_tLocaleID;

	// error reporting
	CComBSTR m_bstrErrorSource;
	CComBSTR m_bstrErrorDescription;
	CComBSTR m_bstrErrorLine;
	LONG m_nErrorColumn;
	ULONG m_nErrorLine;
	HRESULT m_nErrorCode;
};

#include <SharedStateUndo.h>

class ATL_NO_VTABLE CJScriptOperationContext :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationContext
{
public:
	CJScriptOperationContext()
	{
	}
	void Init(IOperationContext* a_pInternal)
	{
		m_pInternal = a_pInternal;
	}
	void Apply(IDocument* a_pDoc = NULL)
	{
		if (m_pInternal == NULL)
			return;
		for (CStates::const_iterator i = m_cStates.begin(); i != m_cStates.end(); ++i)
		{
			CSharedStateUndo<IOperationContext>::SaveState(a_pDoc, m_pInternal, i->first);
			m_pInternal->StateSet(i->first, i->second);
		}
	}


BEGIN_COM_MAP(CJScriptOperationContext)
	COM_INTERFACE_ENTRY(IOperationContext)
END_COM_MAP()

	// IOperationContext methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
		if (i == m_cStates.end())
			return m_pInternal ? m_pInternal->StateGet(a_bstrCategoryName, a_iid, a_ppState) : E_RW_ITEMNOTFOUND;
		return i->second->QueryInterface(a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (a_pState)
			m_cStates[a_bstrCategoryName] = a_pState;
		else
			m_cStates.erase(a_bstrCategoryName);
		return S_OK;
	}
	STDMETHOD(IsCancelled)()
	{
		return m_pInternal ? m_pInternal->IsCancelled() : S_FALSE;
	}
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		return m_pInternal ? m_pInternal->GetOperationInfo(a_pItemIndex, a_pItemsRemaining, a_pStepIndex, a_pStepsRemaining) : E_NOTIMPL;
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		return m_pInternal ? m_pInternal->SetErrorMessage(a_pMessage) : E_NOTIMPL;
	}

private:
	typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

private:
	CStates m_cStates;
	CComPtr<IOperationContext> m_pInternal;
};

