
#pragma once

#include <DocumentName.h>


class ATL_NO_VTABLE CScriptedDocument : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedDocument, &IID_IScriptedDocument, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
	typedef IDispatchImpl<IScriptedDocument, &IID_IScriptedDocument, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff> CDispatchBase;

public:

	CScriptedDocument()
	{
	}
	void Init(IScriptingInterfaceManager* a_pScriptingMgr, IDocument* a_pDocument)
	{
		m_pScriptingMgr = a_pScriptingMgr;
		CComObjectStackEx<CReceiver> cReceiver;
		cReceiver.Init(m_cInterfaces);
		a_pScriptingMgr->GetInterfaceAdaptors(&cReceiver, a_pDocument);
		m_pDoc = a_pDocument;
	}

DECLARE_NOT_AGGREGATABLE(CScriptedDocument)

BEGIN_COM_MAP(CScriptedDocument)
	COM_INTERFACE_ENTRY(IScriptedDocument)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IDocument), m_pDoc.p)
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
			if (cNames == 1) for (CInterfaces::const_iterator i = m_cInterfaces.begin(); i != m_cInterfaces.end(); ++i)
			{
				if (i->first == *rgszNames)
				{
					*rgDispId = 100+(i-m_cInterfaces.begin());
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
			if (dispIdMember >= 100 && dispIdMember < LONG(100+m_cInterfaces.size()))
			{
				pVarResult->vt = VT_DISPATCH;
				(pVarResult->pdispVal = m_cInterfaces[dispIdMember-100].second)->AddRef();
				return S_OK;
			}
			return CDispatchBase::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IScriptedDocument methods
public:
	STDMETHOD(get_Location)(BSTR* pVal)
	{
		try
		{
			*pVal = NULL;
			CComPtr<IStorageFilter> pLoc;
			m_pDoc->LocationGet(&pLoc);
			return pLoc ? pLoc->ToText(NULL, pVal) : S_FALSE;
		}
		catch (...)
		{
			return pVal ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(put_Location)(BSTR newVal)
	{
		try
		{
			if (newVal == NULL)
				return S_FALSE;
			CStorageFilter cSF(newVal);
			if (cSF.operator IStorageFilter *() == NULL)
				return E_INVALIDARG;
			return m_pDoc->LocationSet(cSF);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(get_Name)(BSTR* pVal)
	{
		try
		{
			*pVal = NULL;
			CComPtr<IStorageFilter> pLoc;
			m_pDoc->LocationGet(&pLoc);
			OLECHAR szPath[MAX_PATH] = L"";
			CDocumentName::GetDocNameW(pLoc, szPath, itemsof(szPath));
			if (szPath[0])
				*pVal = SysAllocString(szPath);
			return S_OK;
		}
		catch (...)
		{
			return pVal ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(put_Name)(BSTR newVal)
	{
		try
		{
			if (newVal == NULL)
				return S_FALSE;
			if (CDocumentName::IsValidFilter(m_pDoc))
			{
				CComPtr<IStorageFilter> pLoc;
				m_pDoc->LocationGet(&pLoc);
				CComPtr<IStorageFilter> pNew;
				pLoc->SubFilterGet(newVal, &pNew);
				if (pNew)
					return m_pDoc->LocationSet(pNew);
			}
			CComObject<CDocumentName>* pName = NULL;
			CComObject<CDocumentName>::CreateInstance(&pName);
			CComPtr<IStorageFilter> pTmp = pName;
			CComBSTR bstrDefExt;
			if (NULL == wcschr(newVal, L'.'))
			{
				CComPtr<IConfig> pCfg;
				GUID tEncID = GUID_NULL;
				m_pDoc->EncoderGet(&tEncID, &pCfg);
				CComPtr<IDocumentEncoder> pEnc;
				if (!IsEqualGUID(tEncID, GUID_NULL)) RWCoCreateInstance(pEnc, tEncID);
				CComPtr<IDocumentType> pType;
				if (pEnc) pEnc->DocumentType(&pType);
				if (pType) pType->DefaultExtensionGet(&bstrDefExt);
			}
			if (bstrDefExt.Length())
			{
				CComBSTR bstr(newVal);
				bstr += L".";
				bstr += bstrDefExt;
				pName->Init(bstr);
			}
			else
			{
				pName->Init(newVal);
			}
			return m_pDoc->LocationSet(pTmp);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Save)()
	{
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComPtr<IConfig> pCfg;
		pIM->SaveOptionsGet(m_pDoc, &pCfg, NULL, NULL);
		return pIM->Save(m_pDoc, pCfg, NULL);
	}
	STDMETHOD(SaveCopyAs)(BSTR location)
	{
		CStorageFilter cSF(location);
		if (cSF.operator IStorageFilter *() == NULL)
			return E_INVALIDARG;
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));

		CComPtr<IConfig> pCfg;
		pIM->SaveOptionsGet(m_pDoc, &pCfg, NULL, NULL);

		//CComPtr<IEnumUnknowns> pTypes;
		//pIM->DocumentTypesEnum(&pTypes);
		//ULONG nTypes = 0;
		//if (pTypes) pTypes->Size(&nTypes);
		//for (ULONG i = 0; i < nTypes; ++i)
		//{
		//	CComPtr<IDocumentType> pDT;
		//	pTypes->Get(i, &pDT);
		//	if (S_OK == pDT->MatchFilename(location))
		//	{
		//		pDT->
		//		CComBSTR cCFGID_FFSWITCH(L"FFEncoder");
		//		pCfg->ItemValuesSet(1, &(cCFGID_FFSWITCH.m_str), 
		//		pIM->SaveEncoder
		//		break;
		//	}
		//}

		return pIM->Save(m_pDoc, pCfg, cSF);
	}
	STDMETHOD(Duplicate)(IScriptedDocument** ppCopy)
	{
		try
		{
			*ppCopy = NULL;
			CComPtr<IDocumentBase> pBase;
			RWCoCreateInstance(pBase, __uuidof(DocumentBase));
			HRESULT hRes = m_pDoc->DocumentCopy(NULL, pBase, NULL, NULL);
			if (FAILED(hRes))
				return hRes;
			CComQIPtr<IDocument> pCopy(pBase);
			CComObject<CScriptedDocument>* p = NULL;
			CComObject<CScriptedDocument>::CreateInstance(&p);
			CComPtr<IScriptedDocument> pDoc = p;
			p->Init(m_pScriptingMgr, pCopy);
			*ppCopy = pDoc.Detach();
			return S_OK;
		}
		catch (...)
		{
			return ppCopy ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SupportsFeature)(BSTR a_bstrFeature, VARIANT_BOOL* pSupport)
	{
		try
		{
			*pSupport = VARIANT_FALSE;
			for (CInterfaces::const_iterator i = m_cInterfaces.begin(); i != m_cInterfaces.end(); ++i)
			{
				if (i->first == a_bstrFeature)
				{
					*pSupport = VARIANT_TRUE;
					break;
				}
			}
			return S_OK;
		}
		catch (...)
		{
			return pSupport ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(get_DefaultExtension)(BSTR* pVal)
	{
		try
		{
			*pVal = NULL;
			CComPtr<IConfig> pCfg;
			GUID tID = GUID_NULL;
			m_pDoc->EncoderGet(&tID, &pCfg);
			CComPtr<IInputManager> pIM;
			RWCoCreateInstance(pIM, __uuidof(InputManager));
			CComPtr<IDocumentEncoder> pDE;
			RWCoCreateInstance(pDE, tID);
			CComPtr<IDocumentType> pDT;
			pDE->DocumentType(&pDT);
			return pDT->DefaultExtensionGet(pVal);
		}
		catch (...)
		{
			return pVal ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	typedef std::vector<std::pair<CComBSTR, CComPtr<IDispatch> > > CInterfaces;

	class ATL_NO_VTABLE CReceiver :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IScriptingSite
	{
	public:
		void Init(CInterfaces& a_cObjects)
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
			size_t n = m_pObjects->size();
			m_pObjects->resize(n+1);
			(*m_pObjects)[n].first = a_bstrName;
			(*m_pObjects)[n].second = a_pItem;
			return S_OK;
		}

	private:
		CInterfaces* m_pObjects;
	};

private:
	CInterfaces m_cInterfaces;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IScriptingInterfaceManager> m_pScriptingMgr;
};

