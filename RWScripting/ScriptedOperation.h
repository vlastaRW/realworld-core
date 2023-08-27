
#pragma once

#include <StringParsing.h>
#include <PlugInCache.h>
#include <DispEx.h>


// CScriptedOpConfig

class ATL_NO_VTABLE CScriptedOpConfig : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedOpConfig, &IID_IScriptedOpConfig, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
	typedef IDispatchImpl<IScriptedOpConfig, &IID_IScriptedOpConfig, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff> CDispatchBase;

public:
	CScriptedOpConfig()
	{
	}
	void Init(TConfigValue const& a_tID, IConfig* a_pConfig, IScriptedOperation* a_pOperation)
	{
		m_tID = a_tID;
		m_pConfig = a_pConfig;
		m_pOperation = a_pOperation; // Operation is a global singleton, so its lifetime is not an issue
	}

DECLARE_NOT_AGGREGATABLE(CScriptedOpConfig)

BEGIN_COM_MAP(CScriptedOpConfig)
	COM_INTERFACE_ENTRY(IScriptedOpConfig)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDispatch methods
public:
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
	{
		try
		{
			CConfigValue cVal;
			CComBSTR bstr(*rgszNames);
			if (SUCCEEDED(m_pConfig->ItemValueGet(bstr, &cVal)))
			{
				for (CDispIDs::const_iterator i = m_cDispIDs.begin(); i != m_cDispIDs.end(); ++i)
				{
					if (_wcsicmp(*i, bstr) == 0)
					{
						*rgDispId = 100+(i-m_cDispIDs.begin());
						return S_OK;
					}
				}
				m_cDispIDs.push_back(bstr);
				*rgDispId = 99+m_cDispIDs.size();
				return S_OK;
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
			if (dispIdMember >= 100 && dispIdMember < LONG(100+m_cDispIDs.size()))
			{
				return SetParameter(m_cDispIDs[dispIdMember-100], pDispParams->rgvarg[0]);
				//pVarResult->vt = VT_DISPATCH;
				//(pVarResult->pdispVal = m_cInterfaces[dispIdMember-100].second)->AddRef();
				//return S_OK;
			}
			return CDispatchBase::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IScriptedOpConfig methods
public:
	STDMETHOD(get_ParameterNames)(VARIANT* pNames)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetParameter)(BSTR parameterName, VARIANT parameterValue)
	{
		try
		{
			CConfigValue cVal;
			if (FAILED(m_pConfig->ItemValueGet(parameterName, &cVal)))
				return E_RW_ITEMNOTFOUND;
			CComVariant var;
			switch (cVal.TypeGet())
			{
			case ECVTInteger:
				if (parameterValue.vt == VT_BSTR)
				{
					CComPtr<IConfigItemOptions> pOptions;
					m_pConfig->ItemGetUIInfo(parameterName, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pOptions));
					CComPtr<IEnumConfigItemOptions> pOpts;
					if (pOptions) pOptions->OptionsEnum(&pOpts);
					ULONG nOpts = 0;
					if (pOpts) pOpts->Size(&nOpts);
					ULONG i;
					for (i = 0; i < nOpts; ++i)
					{
						CConfigValue cOpt;
						CComPtr<ILocalizedString> pName;
						CComBSTR bstrOpt;
						if (SUCCEEDED(pOpts->Get(i, &cOpt)) &&
							SUCCEEDED(pOptions->ValueGetName(cOpt, &pName)) && pName &&
							SUCCEEDED(pName->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT), &bstrOpt)) &&
							bstrOpt == parameterValue.bstrVal)
						{
							cVal = cOpt;
							break;
						}
					}
					if (i < nOpts)
						break;
				}
				if (FAILED(var.ChangeType(VT_I4, &parameterValue)))
					return E_FAIL;
				cVal = var.lVal;
				break;
			case ECVTString:
				if (FAILED(var.ChangeType(VT_BSTR, &parameterValue)))
					return E_FAIL;
				cVal = var.bstrVal;
				break;
			case ECVTBool:
				if (FAILED(var.ChangeType(VT_BOOL, &parameterValue)))
					return E_FAIL;
				cVal = var.boolVal != VARIANT_FALSE;
				break;
			case ECVTFloat:
				if (FAILED(var.ChangeType(VT_R4, &parameterValue)))
					return E_FAIL;
				cVal = var.fltVal;
				break;
			case ECVTGUID:
				{
					GUID tID;
					if (FAILED(var.ChangeType(VT_BSTR, &parameterValue)) ||
						!GUIDFromString(var.bstrVal, &tID))
						return E_FAIL;
					cVal = tID;
				}
				break;
			case ECVTFloatColor:
			case ECVTVector2:
			case ECVTVector3:
			case ECVTVector4:
				{
					ULONG const nExpected = cVal.TypeGet() == ECVTVector2 ? 2 : (cVal.TypeGet() == ECVTVector4 ? 4 : 3);
					if (parameterValue.vt == VT_BSTR)
					{
						TConfigValue t;
						ZeroMemory(&t, sizeof t);
						t.eTypeID = cVal.TypeGet();
						int nNmbrs = swscanf(var.bstrVal, L"%f,%f,%f,%f", &t.vecVal[0], &t.vecVal[1], &t.vecVal[2], &t.vecVal[3]);
						if (nNmbrs != nExpected)
							return E_FAIL;
						cVal = t;
					}
					else if ((parameterValue.vt&VT_TYPEMASK) == VT_DISPATCH)
					{
						IDispatch* p = (parameterValue.vt&VT_BYREF) ? *parameterValue.ppdispVal : parameterValue.pdispVal;

						// get array length
						DISPPARAMS params;
						FillMemory(&params, sizeof(DISPPARAMS), 0);

						LPOLESTR ln = L"length";

						DISPID dl;
						if (FAILED(p->GetIDsOfNames(IID_NULL, &ln, 1, LOCALE_USER_DEFAULT, &dl)))
							return E_FAIL;

						VARIANT res;
						VariantInit(&res);
						if (FAILED(p->Invoke(dl, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
							return E_FAIL;

						CComVariant len;
						len.ChangeType(VT_I4, &res);
						VariantClear(&res);

						if (len.iVal != nExpected)
							return E_FAIL;

						// get individual values
						TConfigValue t;
						ZeroMemory(&t, sizeof t);
						t.eTypeID = cVal.TypeGet();

						for (ULONG i = 0; i < nExpected; ++i)
						{
							OLECHAR szInd[16];
							_itow(i, szInd, 10);
							LPOLESTR pszInd = szInd;
							if (FAILED(p->GetIDsOfNames(IID_NULL, &pszInd, 1, LOCALE_USER_DEFAULT, &dl)))
								return E_FAIL;
							if (FAILED(p->Invoke(dl, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
								return E_FAIL;
							CComVariant val;
							HRESULT hr = val.ChangeType(VT_R4, &res);
							VariantClear(&res);
							if (FAILED(hr))
								return hr;
							t.vecVal[i] = val.fltVal;
						}
						cVal = t;
					}
				}
				break;
			default:
				return E_FAIL;
			}
			return m_pConfig->ItemValuesSet(1, &parameterName, cVal);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetParameter)(BSTR parameterName, VARIANT* parameterValue)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Execute)(IUnknown* document, VARIANT context, IScriptedDocument** ppDocument)
	{
		return m_pOperation ? m_pOperation->Execute(this, document, context, ppDocument) : E_FAIL;
	}
	STDMETHOD(GetOpIDAndConfig)(TConfigValue* a_pOpID, IConfig** a_ppOpCfg)
	{
		*a_pOpID = m_tID;
		(*a_ppOpCfg = m_pConfig)->AddRef();
		return S_OK;
	}

private:
	typedef std::vector<CComBSTR> CDispIDs;

private:
	TConfigValue m_tID;
	CComPtr<IConfig> m_pConfig;
	CDispIDs m_cDispIDs;
	IScriptedOperation* m_pOperation;
};


// CScriptedOperation

class ATL_NO_VTABLE CScriptedOperation : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedOperation, &IID_IScriptedOperation, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
	typedef IDispatchImpl<IScriptedOperation, &IID_IScriptedOperation, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff> CDispatchBase;
public:
	CScriptedOperation() : m_initialized(false)
	{
	}
	void Init(IScriptingInterfaceManager* a_pScriptingMgr, IOperationManager* a_pOpMan, ITransformationManager* a_pTrMan, LCID a_tLocaleID, HWND a_hWnd)
	{
		m_pScriptingMgr = a_pScriptingMgr;
		m_pManager = a_pOpMan;
		m_pTrMan = a_pTrMan;
		m_tLocaleID = a_tLocaleID;
		m_hWnd = a_hWnd;
	}

DECLARE_NOT_AGGREGATABLE(CScriptedOperation)

BEGIN_COM_MAP(CScriptedOperation)
	COM_INTERFACE_ENTRY(IScriptedOperation)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDispatch methods
public:
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
	{
		HRESULT hRes = CDispatchBase::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
		if (SUCCEEDED(hRes)) return hRes;
		try
		{
			ObjectLock lock(this);
			UpdateOpMap();

			CComBSTR bstr(*rgszNames);
			COpList::const_iterator best = m_opList.end();
			LONG score = LONG_MIN;
			for (COpList::const_iterator i = m_opList.begin(); i != m_opList.end(); ++i)
			{
				LONG const cur = FuzzyCompare(i->first, bstr);
				if (best == m_opList.end() || cur > score)
				{
					best = i;
					score = cur;
				}
			}
			if (best != m_opList.end())
			{
				*rgDispId = 100+(best-m_opList.begin());
				return S_OK;
			}
			return hRes;
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
			if (dispIdMember >= 100 && dispIdMember < LONG(100+m_opList.size()))
			{
				IUnknown* document = NULL;
				IDispatch* config = NULL;
				VARIANT context;
				context.vt = VT_EMPTY;
				UINT nArgs = pDispParams->cArgs;
				VARIANT* pArgs = pDispParams->rgvarg;
				if (nArgs == 3) // with Context
				{
					context = *pArgs;
					pArgs++;
					nArgs--;
				}
				if (nArgs == 2) // with configuration
				{
					if (pArgs->vt == VT_UNKNOWN || pArgs->vt == VT_DISPATCH)
						config = pArgs->pdispVal;
					pArgs++;
					nArgs--;
				}
				if (nArgs != 1 || (pArgs->vt != VT_UNKNOWN && pArgs->vt != VT_DISPATCH)) // Document is mandatory
					return E_INVALIDARG;
				document = pArgs->punkVal;
				CComPtr<IScriptedDocument> pOut;
				HRESULT hRes = ExecuteInPlace(m_opList[dispIdMember-100].second, document, config, context, &pOut);
				if (pVarResult)
				{
					if (pOut)
					{
						pVarResult->vt = VT_DISPATCH;
						pVarResult->pdispVal = pOut.Detach();
					}
					else
					{
						pVarResult->vt = VT_EMPTY;
					}
				}
				return hRes;
			}
			return CDispatchBase::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IScriptedOperation methods
public:
	STDMETHOD(Create)(BSTR guidOrName, IScriptedOpConfig** ppConfig)
	{
		try
		{
			*ppConfig = NULL;
			if (guidOrName == NULL)
				return E_INVALIDARG;
			TConfigValue tID;
			tID.eTypeID = ECVTGUID;
			tID.guidVal = GUID_NULL;
			if (!GUIDFromString(guidOrName, &tID.guidVal))
			{
				// try to convert name to id
				ULONG nCount = 0;
				m_pManager->ItemGetCount(&nCount);
				ULONG i;
				for (i = 0; i < nCount; ++i)
				{
					CComPtr<ILocalizedString> pName;
					m_pManager->ItemIDGet(m_pManager, i, &tID, &pName);
					if (pName)
					{
						CComBSTR bstr;
						pName->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), &bstr);
						if (bstr != NULL && _wcsicmp(bstr, guidOrName) == 0)
							break;
					}
				}
				if (i == nCount)
				{
					nCount = 0;
					m_pTrMan->ItemGetCount(&nCount);
					for (i = 0; i < nCount; ++i)
					{
						CComPtr<ILocalizedString> pName;
						m_pTrMan->ItemIDGet(m_pTrMan, i, &tID, &pName);
						if (pName)
						{
							CComBSTR bstr;
							pName->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), &bstr);
							if (bstr != NULL && _wcsicmp(bstr, guidOrName) == 0)
								break;
						}
					}
					if (i == nCount)
						return E_FAIL;
				}
			}
			CComPtr<IConfig> pCfg;
			if (FAILED(m_pManager->CreateConfig(&tID, &pCfg)) && FAILED(m_pTrMan->CreateConfig(&tID, &pCfg)))
				return E_FAIL;
			CComObject<CScriptedOpConfig>* p = NULL;
			CComObject<CScriptedOpConfig>::CreateInstance(&p);
			CComPtr<IScriptedOpConfig> pTmp = p;
			p->Init(tID, pCfg, this);
			*ppConfig = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	class ATL_NO_VTABLE CSimpleOperationContext :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IOperationContext
	{
	public:
		ILocalizedString* M_ErrorMessage()
		{
			return m_pMessage;
		}


	BEGIN_COM_MAP(CSimpleOperationContext)
		COM_INTERFACE_ENTRY(IOperationContext)
	END_COM_MAP()

		// IOperationContext methods
	public:
		STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
		{
			CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
			if (i == m_cStates.end())
				return E_RW_ITEMNOTFOUND;
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
			return S_FALSE;
		}
		STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
		{
			if (a_pItemIndex) *a_pItemIndex = 0;
			if (a_pItemsRemaining) *a_pItemsRemaining = 0;
			if (a_pStepIndex) *a_pStepIndex = 0;
			if (a_pStepsRemaining) *a_pStepsRemaining = 0;
			return S_OK;
		}
		STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
		{
			m_pMessage = a_pMessage;
			return S_OK;
		}

	private:
		typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

	private:
		CStates m_cStates;
		CComPtr<ILocalizedString> m_pMessage;
	};



	STDMETHOD(Execute)(IScriptedOpConfig* config, IUnknown* document, VARIANT context, IScriptedDocument** ppDocument)
	{
		try
		{
			*ppDocument = NULL;
			if (config == NULL)
				return E_POINTER;
			CConfigValue cID;
			CComPtr<IConfig> pCfg;
			if (FAILED(config->GetOpIDAndConfig(&cID, &pCfg)))
				return E_UNEXPECTED;
			CComQIPtr<IDocument> pDoc(document);
			if (pDoc == NULL)
				return E_POINTER;
			CComPtr<IOperationContext> pStates;
			if (context.vt == VT_UNKNOWN)
			{
				context.punkVal->QueryInterface(__uuidof(IOperationContext), reinterpret_cast<void**>(&pStates));
			}
			else if (context.vt == VT_DISPATCH)
			{
				context.pdispVal->QueryInterface(__uuidof(IOperationContext), reinterpret_cast<void**>(&pStates));
			}
			if (pStates == NULL)
			{
				CComObject<CSimpleOperationContext>* p = NULL;
				CComObject<CSimpleOperationContext>::CreateInstance(&p);
				pStates = p;
			}
			CComPtr<IConfig> pDummy;
			HRESULT hRes = m_pManager->CreateConfig(cID, &pDummy);
			if (SUCCEEDED(hRes) || hRes == E_NOTIMPL)
				return m_pManager->Activate(m_pManager, pDoc, cID, pCfg, pStates, m_hWnd, m_tLocaleID);
			CComPtr<IDocumentBase> pNewDoc;
			RWCoCreateInstance(pNewDoc, __uuidof(DocumentBase));
			hRes = m_pTrMan->Activate(m_pTrMan, pDoc, cID, pCfg, pStates, m_hWnd, m_tLocaleID, NULL, pNewDoc);
			if (FAILED(hRes))
				return hRes;
			return m_pScriptingMgr->WrapDocument(m_pScriptingMgr, CComQIPtr<IDocument>(pNewDoc), ppDocument);
		}
		catch (...)
		{
			return ppDocument ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Process)(BSTR guidOrName, IUnknown* document, IDispatch* config, VARIANT context, IScriptedDocument** ppDocument)
	{
		try
		{
			TConfigValue tID;
			tID.eTypeID = ECVTGUID;
			tID.guidVal = GUID_NULL;
			if (!GUIDFromString(guidOrName, &tID.guidVal))
			{
				// try to convert name to id
				ULONG nCount = 0;
				m_pManager->ItemGetCount(&nCount);
				ULONG i;
				for (i = 0; i < nCount; ++i)
				{
					CComPtr<ILocalizedString> pName;
					m_pManager->ItemIDGet(m_pManager, i, &tID, &pName);
					if (pName)
					{
						CComBSTR bstr;
						pName->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), &bstr);
						if (bstr != NULL && _wcsicmp(bstr, guidOrName) == 0)
							break;
					}
				}
				if (i == nCount)
					return E_FAIL;
			}
			return ExecuteInPlace(tID.guidVal, document, config, context, ppDocument);
		}
		catch (...)
		{
			return ppDocument ? E_UNEXPECTED : E_POINTER;
		}
	}
	HRESULT ExecuteInPlace(GUID const& opID, IUnknown* document, IDispatch* config, VARIANT context, IScriptedDocument** ppDocument)
	{
		TConfigValue tID;
		tID.eTypeID = ECVTGUID;
		tID.guidVal = opID;
		CComPtr<IConfig> pCfg;
		if (FAILED(m_pManager->CreateConfig(CConfigValue(opID), &pCfg)) && FAILED(m_pTrMan->CreateConfig(&tID, &pCfg)))
			return E_FAIL;
		CComObject<CScriptedOpConfig>* p = NULL;
		CComObject<CScriptedOpConfig>::CreateInstance(&p);
		CComPtr<IScriptedOpConfig> pTmp = p;
		p->Init(tID, pCfg, this);

		CComQIPtr<IDispatchEx> dispex(config);
		if (dispex)
		{
			DISPID did = DISPID_STARTENUM;
			while (NOERROR == dispex->GetNextDispID(fdexEnumAll, did, &did) && did != DISPID_STARTENUM)
			{
				DISPPARAMS params;
				ZeroMemory(&params, sizeof params);
				CComVariant res;
				CComBSTR bstrKey;
				if (SUCCEEDED(dispex->GetMemberName(did, &bstrKey)) &&
					SUCCEEDED(dispex->InvokeEx(did, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
				{
					p->SetParameter(bstrKey, res);
				}
			}
		}

		return Execute(p, document, context, ppDocument);
	}

private:
	typedef std::vector<std::pair<CComBSTR, GUID> > COpList;

	void UpdateOpMap()
	{
		ULONG const timestamp = CPlugInEnumerator::GetCategoryTimestamp(CATID_DocumentOperation);
		if (m_initialized && timestamp == m_timestamp)
			return;
		m_timestamp = timestamp;
		m_initialized = true;
		ULONG nCount = 0;
		m_pManager->ItemGetCount(&nCount);
		for (ULONG i = 0; i < nCount; ++i)
		{
			CComPtr<ILocalizedString> pName;
			CConfigValue cID;
			m_pManager->ItemIDGet(m_pManager, i, &cID, &pName);
			if (pName)
			{
				std::pair<CComBSTR, GUID> item;
				pName->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), &item.first);
				if (item.first != NULL)
				{
					item.second = cID.operator const GUID &();
					// check if the item is already in the list (in case we are updating)
					COpList::const_iterator j = m_opList.begin();
					while (j != m_opList.end() && !IsEqualGUID(j->second, item.second)) ++j;
					if (j == m_opList.end())
						m_opList.push_back(item);
				}
			}
		}
	}
	LONG FuzzyCompare(LPCWSTR haystack, LPCWSTR needle)
	{
		wchar_t tmp[128];
		for (int n = 0; n < 128 && (tmp[n] = *haystack); ++haystack)
			if ((*haystack >= L'A' && *haystack <= L'Z') ||
				(*haystack >= L'a' && *haystack >= L'z') ||
				(*haystack >= L'0' && *haystack >= L'9'))
				++n;
		tmp[127] = L'\0';
		if (wcsstr(tmp, needle))
			return -LONG(wcslen(haystack));
		return LONG_MIN;
	}

private:
	CComPtr<IScriptingInterfaceManager> m_pScriptingMgr;
	LCID m_tLocaleID;
	HWND m_hWnd;

	CComPtr<IOperationManager> m_pManager;
	CComPtr<ITransformationManager> m_pTrMan;
	COpList m_opList;
	ULONG m_timestamp;
	bool m_initialized;
};

