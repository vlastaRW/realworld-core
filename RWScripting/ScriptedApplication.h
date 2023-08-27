
#pragma once

#define AFX_RESOURCE_DLL
#include <RWVersionInfo.rc2>
#include <SimpleLocalizedString.h>
#include <StringParsing.h>
#include <RWConceptDesignerExtension.h>
#include <math.h>

// copied from RWDesignerCore.h - cyclic dependency
	MIDL_INTERFACE("11AB0A89-2016-4D26-B8CD-A08219D2A025")
	IDesignerCore : public IUnknown
	{
	public:
		virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Initialize(/* removed */) = 0;
	    
		virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Name( 
			/* [out] */ ILocalizedString **a_ppName) = 0;
	    
		virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Version( 
			/* [out] */ ULONG *a_pVersion) = 0;
	    
		virtual /* [local][helpstring] */ HRESULT STDMETHODCALLTYPE Icon( 
			/* [in] */ ULONG a_nSize,
			/* [out] */ HICON *a_phIcon) = 0;
	    
		virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NewWindowPath( 
			/* [in] */ BSTR a_bstrFullPath,
			/* [out] */ RWHWND* a_pCreatedWindow) = 0;
	    
		virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NewWindowDocument( 
			/* [in] */ IDocument *a_pDocument,
			/* [out] */ RWHWND* a_pCreatedWindow) = 0;
	    
		virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NewWindowPage( 
			/* [in] */ REFCLSID a_tStartPageID,
			/* [out] */ RWHWND* a_pCreatedWindow) = 0;
	    
		virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Documents( 
			/* [out] */ IEnumUnknowns **a_ppOpenDocuments) = 0;
	    
		virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MessagePump( 
			/* [in] */ ULONG a_nStopEvents,
			/* [size_is][in] */ HANDLE *a_pStopEvents,
			/* [out] */ ULONG *a_pWaitResult) = 0;
	    
	};
	class DECLSPEC_UUID("B274F86D-6584-4B8B-9C11-D8247BB532DF")
	DesignerCore;


// CScriptedRNG

class ATL_NO_VTABLE CScriptedRNG : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedRNG, &IID_IScriptedRNG, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
public:
	void Init(double seed)
	{
		ULONG s = seed == 0.0 ? GetTickCount() : ULONG(seed);
		// make random numbers and put them into the buffer
		for (int i = 0; i < 5; ++i)
		{
			s = s * 29943829 - 1;
			x[i] = s * (1.0/(65536.0*65536.0));
		}
		// randomize some more
		for (int i = 0; i < 19; ++i)
		{
			double d;
			NextVal(&d);
		}
	}

DECLARE_NOT_AGGREGATABLE(CScriptedRNG)

BEGIN_COM_MAP(CScriptedRNG)
	COM_INTERFACE_ENTRY(IScriptedRNG)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	// IScriptedRNG methods
public:
	STDMETHOD(NextVal)(double* val)
	{
		long double c;
		c = (long double)2111111111.0 * x[3] +
			1492.0 * (x[3] = x[2]) +
			1776.0 * (x[2] = x[1]) +
			5115.0 * (x[1] = x[0]) +
			x[4];
		x[4] = floorl(c);
		x[0] = c - x[4];
		x[4] = x[4] * (1./(65536.*65536.));
		*val = x[0];
		return S_OK;
	}
	STDMETHOD(NextInt)(ULONG limit, ULONG* val)
	{
		double d;
		NextVal(&d);
		*val = limit*d;
		if (*val >= limit) *val = limit-1;
		return S_OK;
	}

private:
	double x[5];
};


// CScriptedApplication

class ATL_NO_VTABLE CScriptedApplication : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedApplication, &IID_IScriptedApplication, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
	typedef IDispatchImpl<IScriptedApplication, &IID_IScriptedApplication, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff> CDispatchBase;

public:
	CScriptedApplication() : m_bTranslatorTried(false)
	{
	}
	void Init(IScriptingInterfaceManager* a_pScriptingMgr, RWHWND a_hParent, LCID a_tLocaleID, LPCWSTR a_pszAppName)
	{
		m_pScriptingMgr = a_pScriptingMgr;
		m_hWnd = a_hParent;
		m_tLocaleID = a_tLocaleID;
		m_pszAppName = a_pszAppName;
	}

DECLARE_NOT_AGGREGATABLE(CScriptedApplication)

BEGIN_COM_MAP(CScriptedApplication)
	COM_INTERFACE_ENTRY(IScriptedApplication)
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

	// IScriptedApplication methods
public:
	STDMETHOD(get_Name)(BSTR* name)
	{
		if (m_pszAppName && *m_pszAppName)
		{
			*name = SysAllocString(m_pszAppName);
			return S_OK;
		}

		CComPtr<IDesignerCore> pDC;
		RWCoCreateInstance(pDC, __uuidof(DesignerCore));

		CComPtr<ILocalizedString> pStr;
		if (pDC) pDC->Name(&pStr);
		if (pStr) pStr->GetLocalized(m_tLocaleID, name);
		if (*name == NULL)
			*name = SysAllocString(L"RealWorld Designer");
		return S_OK;
	}
	STDMETHOD(get_Version)(BSTR* version)
	{
		CComBSTR bstr(RWVER0409_PRODUCTVERSION);
		*version = bstr.Detach();
		return S_OK;
	}
	STDMETHOD(get_LocaleID)(ULONG* localeID)
	{
		*localeID = m_tLocaleID;
		return S_OK;
	}

	class CClipboardHandler
	{
	public:
		CClipboardHandler(HWND a_hWnd)
		{
			if (!::OpenClipboard(a_hWnd))
				throw E_FAIL;
		}
		~CClipboardHandler()
		{
			::CloseClipboard();
		}
	};

	STDMETHOD(get_Clipboard)(BSTR* text)
	{
		try
		{
			*text = NULL;
			CClipboardHandler cHandler(m_hWnd);
#ifdef _UNICODE
			if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
				return S_FALSE;
			HANDLE hData = GetClipboardData(CF_UNICODETEXT);
			if (hData == NULL)
				return S_FALSE;
			LPVOID pData = GlobalLock(hData);
			if (pData == NULL)
				return S_FALSE;
			*text = SysAllocString(reinterpret_cast<OLECHAR const*>(pData));
#else
			if (!IsClipboardFormatAvailable(CF_TEXT))
				return S_FALSE;
			HANDLE hData = GetClipboardData(CF_TEXT);
			if (hData == NULL)
				return S_FALSE;
			LPVOID pData = GlobalLock(hData);
			if (pData == NULL)
				return S_FALSE;
			*text = SysAllocString(CA2W(reinterpret_cast<char const*>(pData)));
#endif
			GlobalUnlock(hData);
			return S_OK;
		}
		catch (...)
		{
			return text == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(put_Clipboard)(BSTR text)
	{
		try
		{
			CClipboardHandler cHandler(m_hWnd);
#ifdef _UNICODE
			if (text == NULL || text[0] == L'\0')
			{
				SetClipboardData(CF_UNICODETEXT, NULL);
				return S_OK;
			}

			HANDLE hMem = GlobalAlloc(GHND, sizeof(OLECHAR)*SysStringLen(text));
			if (hMem == NULL)
				return E_FAIL;
			LONG* pMem = reinterpret_cast<LONG*>(GlobalLock(hMem));
			CopyMemory(pMem, text, sizeof(OLECHAR)*SysStringLen(text));
			GlobalUnlock(hMem);

			SetClipboardData(CF_UNICODETEXT, hMem);
#else
			if (text == NULL || text[0] == L'\0')
			{
				SetClipboardData(CF_TEXT, NULL);
				return S_OK;
			}

			CW2A str(text);
			ULONG nLen = strlen(str);
			HANDLE hMem = GlobalAlloc(GHND, 1+nLen);
			if (hMem == NULL)
				return E_FAIL;
			LONG* pMem = reinterpret_cast<LONG*>(GlobalLock(hMem));
			CopyMemory(pMem, (char*)str, 1+nLen);
			GlobalUnlock(hMem);

			SetClipboardData(CF_UNICODETEXT, hMem);
#endif
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(MessageBox)(BSTR text, BSTR caption, VARIANT_BOOL question, VARIANT_BOOL* answer)
	{
		int i = ::MessageBox(m_hWnd, COLE2T(text), COLE2T(caption), question ? MB_YESNO|MB_ICONQUESTION : MB_OK);
		if (answer)
			*answer = i == IDYES ? VARIANT_TRUE : VARIANT_FALSE;
		return S_OK;
	}
	STDMETHOD(FileDialog)(BSTR initialPath, BSTR caption, VARIANT_BOOL fileMustExist, BSTR* pPath)
	{
		try
		{
			*pPath = NULL;
			CComPtr<ILocalizedString> pCaption;
			pCaption.Attach(new CSimpleLocalizedString(CComBSTR(caption).Detach()));
			CComPtr<IStorageFilter> pOut;
			static const GUID tID = {0xdd82c3e7, 0x8a7e, 0x446b, {0xa1, 0x7c, 0x0, 0xc1, 0xa1, 0xc9, 0x25, 0x26}};
			M_StorageManager()->FilterCreateInteractivelyUID(initialPath, fileMustExist ? EFTOpenExisting : EFTCreateNew, m_hWnd, NULL, NULL, tID, pCaption, NULL, m_tLocaleID, &pOut);
			if (pOut == NULL)
				return S_FALSE;
			return pOut->ToText(NULL, pPath);
		}
		catch (...)
		{
			return pPath ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(OpenDocument)(BSTR path, IScriptedDocument** ppDocument)
	{
		try
		{
			*ppDocument = NULL;
			CComPtr<IStorageFilter> pLoc;
			M_StorageManager()->FilterCreate(path, 0, &pLoc);
			if (pLoc == NULL)
				return S_FALSE;
			CComPtr<IDocument> pDoc;
			M_InputManager()->DocumentCreate(pLoc, NULL, &pDoc);
			if (pDoc == NULL)
				return S_FALSE;
			return m_pScriptingMgr->WrapDocument(m_pScriptingMgr, pDoc, ppDocument);
		}
		catch (...)
		{
			return ppDocument ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(OpenDocumentAs)(BSTR path, BSTR builderID, IScriptedDocument** ppDocument)
	{
		try
		{
			*ppDocument = NULL;
			if (builderID == NULL)
				return E_INVALIDARG;
			GUID tID = GUID_NULL;
			if (!GUIDFromString(builderID, &tID))
				return S_FALSE;
			CComPtr<IDocumentBuilder> pBuilder;
			RWCoCreateInstance(pBuilder, tID);
			if (pBuilder == NULL)
				return S_FALSE;
			CComPtr<IStorageFilter> pLoc;
			M_StorageManager()->FilterCreate(path, 0, &pLoc);
			if (pLoc == NULL)
				return S_FALSE;
			CComPtr<IInputManager> pIM;
			RWCoCreateInstance(pIM, __uuidof(InputManager));
			CComPtr<IDocument> pDoc;
			if (FAILED(pIM->DocumentCreateEx(pBuilder, pLoc, NULL, &pDoc)))
				return S_FALSE;
			return m_pScriptingMgr->WrapDocument(m_pScriptingMgr, pDoc, ppDocument);
		}
		catch (...)
		{
			return ppDocument ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(CreateWizard)(BSTR wizardID, IScriptedOpConfig** ppConfig)
	{
		try
		{
			*ppConfig = NULL;
			if (wizardID == NULL)
				return E_INVALIDARG;
			TConfigValue tID;
			tID.eTypeID = ECVTGUID;
			tID.guidVal = GUID_NULL;
			if (!GUIDFromString(wizardID, &tID.guidVal))
			{
				return S_FALSE; // no name->guid translation yet
				//// try to convert name to id
				//ULONG nCount = 0;
				//m_pManager->ItemGetCount(&nCount);
				//ULONG i = 0;
				//for (i = 0; i < nCount; ++i)
				//{
				//	CComPtr<ILocalizedString> pName;
				//	m_pManager->ItemIDGet(m_pManager, i, &tID, &pName);
				//	if (pName)
				//	{
				//		CComBSTR bstr;
				//		pName->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), &bstr);
				//		if (bstr != NULL && _wcsicmp(bstr, guidOrName) == 0)
				//			break;
				//	}
				//}
				//if (i == nCount)
				//	return E_FAIL;
			}
			CComPtr<IDesignerWizard> pFact;
			RWCoCreateInstance(pFact, tID.guidVal);
			if (pFact == NULL)
				return S_FALSE;
			CComPtr<IConfig> pCfg;
			if (FAILED(pFact->Config(&pCfg)))
				return E_FAIL;
			CComObject<CScriptedOpConfig>* p = NULL;
			CComObject<CScriptedOpConfig>::CreateInstance(&p);
			CComPtr<IScriptedOpConfig> pTmp = p;
			p->Init(tID, pCfg, NULL);
			*ppConfig = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return ppConfig ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(CreateDocument)(IScriptedOpConfig* config, VARIANT builder, IScriptedDocument** ppDocument)
	{
		try
		{
			if (config == NULL)
				return E_POINTER;
			CConfigValue cID;
			CComPtr<IConfig> pCfg;
			if (FAILED(config->GetOpIDAndConfig(&cID, &pCfg)))
				return E_UNEXPECTED;

			CComPtr<IDesignerWizard> pFact;
			RWCoCreateInstance(pFact, cID.operator const GUID &());
			if (pFact == NULL)
				return E_FAIL;

			std::vector<CComPtr<IDocumentBuilder> > cBuilders;
			if (builder.vt == VT_BSTR)
			{
				CComPtr<IDocumentBuilder> pFact;
				ULONG const n = ::SysStringLen(builder.bstrVal);
				bool bGUID = false;
				GUID tBuilder = GUID_NULL;
				if (n == 36)
					bGUID = GUIDFromString(builder.bstrVal, &tBuilder);
				else if (n == 38 && builder.bstrVal[0] == L'{' && builder.bstrVal[37] == L'}')
					bGUID = GUIDFromString(builder.bstrVal+1, &tBuilder);
				if (bGUID)
				{
					if (FAILED(RWCoCreateInstance(pFact, tBuilder)))
						return E_FAIL;
					cBuilders.push_back(pFact);
				}
				else if (n > 0)
				{
					CComPtr<IPlugInCache> pPIC;
					RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
					CComPtr<IEnumUnknowns> pBuilders;
					if (pPIC) pPIC->InterfacesEnum(CATID_DocumentBuilder, __uuidof(IDocumentBuilder), 0, &pBuilders, NULL);
					ULONG nBuilders = 0;
					pBuilders->Size(&nBuilders);
					ULONG nBest = 0;
					for (ULONG i = 0; i < nBuilders; ++i)
					{
						CComPtr<IDocumentBuilder> pDB;
						pBuilders->Get(i, &pDB);
						CComPtr<ILocalizedString> pName;
						if (pDB == NULL) continue;
						pDB->TypeName(&pName);
						if (pName == NULL) continue;
						CComBSTR bstrName;
						pName->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT), &bstrName);
						if (bstrName == builder.bstrVal)
						{
							cBuilders.push_back(pDB);
							break;
						}
					}
					if (cBuilders.empty())
						return E_FAIL;
				}
			}
			if (cBuilders.empty())
			{
				CComPtr<IPlugInCache> pPIC;
				RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
				CComPtr<IEnumUnknowns> pBuilders;
				if (pPIC) pPIC->InterfacesEnum(CATID_DocumentBuilder, __uuidof(IDocumentBuilder), 0, &pBuilders, NULL);
				ULONG nBuilders = 0;
				pBuilders->Size(&nBuilders);
				ULONG nBest = 0;
				for (ULONG i = 0; i < nBuilders; ++i)
				{
					CComPtr<IDocumentBuilder> pDB;
					pBuilders->Get(i, &pDB);
					ULONG nPriority = EDPAverage;
					pDB->Priority(&nPriority);
					size_t j = 0;
					for (; j < cBuilders.size(); ++j)
					{
						ULONG nPriority2 = EDPAverage;
						cBuilders[j]->Priority(&nPriority2);
						if (nPriority2 < nPriority)
						{
							cBuilders.insert(cBuilders.begin()+j, pDB);
							break;
						}
					}
					if (j == cBuilders.size())
						cBuilders.push_back(pDB);
				}
			}

			CComPtr<IDocumentBase> pDoc;
			RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
			if (FAILED(pFact->Activate(m_hWnd, m_tLocaleID, pCfg, cBuilders.size(), !cBuilders.empty() ? &(cBuilders[0].p) : NULL, NULL, pDoc)))
				return S_FALSE;
			return m_pScriptingMgr->WrapDocument(m_pScriptingMgr, CComQIPtr<IDocument>(pDoc), ppDocument);
		}
		catch (...)
		{
			return ppDocument ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(OpenWindow)(IDispatch* pDocument)
	{
		try
		{
			CComQIPtr<IDocument> pDoc(pDocument);
			if (pDoc == NULL)
				return E_INVALIDARG;
			CComPtr<IDesignerCore> pDC;
			RWCoCreateInstance(pDC, __uuidof(DesignerCore));
			return pDC->NewWindowDocument(pDoc, NULL);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetCreatedTime)(BSTR path, double* pMSFrom1970)
	{
		try
		{
			*pMSFrom1970 = 0.0;
			CComPtr<IStorageFilter> pLoc;
			M_StorageManager()->FilterCreate(path, EFTHintNoStream|EFTShareRead|EFTShareWrite|EFTOpenExisting, &pLoc);
			CComQIPtr<IStorageLocatorAttrs> pAttrs(pLoc);
			if (pAttrs == NULL)
				return S_FALSE;
			ULONGLONG tTime;
			if (FAILED(pAttrs->GetTime(ESTTCreation, &tTime)))
				return S_FALSE;
			*pMSFrom1970 = (tTime-116444736000000000ULL)/10000;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Translate)(BSTR eng, BSTR* pTranslated)
	{
		try
		{
			*pTranslated = NULL;
			if (eng == NULL || *eng == L'\0')
				return S_FALSE;
			ITranslator* pTr = M_Translator();
			if (pTr)
				pTr->Translate(eng, m_tLocaleID, pTranslated);
			if (*pTranslated == NULL)
				*pTranslated = SysAllocString(eng);
			return S_OK;
		}
		catch (...)
		{
			return pTranslated ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(GetFileSize)(BSTR path, ULONG* pBytes)
	{
		try
		{
			*pBytes = 0;
			CComPtr<IStorageFilter> pLoc;
			M_StorageManager()->FilterCreate(path, EFTHintNoStream|EFTShareRead|EFTShareWrite|EFTOpenExisting, &pLoc);
			if (pLoc == NULL) return S_FALSE;
			CComPtr<IDataSrcDirect> pSrc;
			pLoc->SrcOpen(&pSrc);
			if (pSrc == NULL) return S_FALSE;
			return pSrc->SizeGet(pBytes);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CreateRNG)(double seed, IScriptedRNG** rng)
	{
		try
		{
			*rng = 0;
			CComObject<CScriptedRNG>* p = NULL;
			CComObject<CScriptedRNG>::CreateInstance(&p);
			CComPtr<IScriptedRNG> pTmp = p;
			p->Init(seed);
			*rng = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	IStorageManager* M_StorageManager()
	{
		if (m_pStMgr)
			return m_pStMgr;
		ObjectLock cLock(this);
		if (m_pStMgr == NULL)
			RWCoCreateInstance(m_pStMgr, __uuidof(StorageManager));
		return m_pStMgr;
	}
	IInputManager* M_InputManager()
	{
		if (m_pInMgr)
			return m_pInMgr;
		ObjectLock cLock(this);
		if (m_pInMgr == NULL)
			RWCoCreateInstance(m_pInMgr, __uuidof(InputManager));
		return m_pInMgr;
	}
	ITranslator* M_Translator()
	{
		if (m_pTranslator || m_bTranslatorTried)
			return m_pTranslator;
		ObjectLock cLock(this);
		if (m_pTranslator == NULL)
			RWCoCreateInstance(m_pTranslator, __uuidof(Translator));
		m_bTranslatorTried = true;
		return m_pTranslator;
	}

private:
	CComPtr<IScriptingInterfaceManager> m_pScriptingMgr;
	RWHWND m_hWnd;
	LCID m_tLocaleID;
	LPCWSTR m_pszAppName;
	CComPtr<IStorageManager> m_pStMgr;
	CComPtr<IInputManager> m_pInMgr;
	CComPtr<ITranslator> m_pTranslator;
	bool m_bTranslatorTried;
};


