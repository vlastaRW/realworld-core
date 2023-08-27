// DesignerViewFileBrowser.h : Declaration of the CDesignerViewFileBrowser

#pragma once

#include <htmlhelp.h>
#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>
#include <RWConceptDesignerExtension.h>


// CDesignerViewFileBrowser

class ATL_NO_VTABLE CDesignerViewFileBrowser : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDesignerView,
	public IStorageFilterWindowCallback
{
public:
	CDesignerViewFileBrowser()
	{
	}
	HRESULT Init(IDocument* a_pDocument, IConfig* a_pConfig, BSTR a_bstrOpName, BSTR a_bstrOpDesc, GUID const& a_tIconID, TConfigValue const* a_pOpID, IConfig* a_pOpCfg, IOperationManager* a_pOpMgr, ISharedStateManager* a_pStates, RWHWND a_hWnd, LCID a_tLocaleID, RECT const* a_prc)
	{
		m_pDocument = a_pDocument;
		m_bstrOpName = a_bstrOpName;
		m_bstrOpDesc = a_bstrOpDesc;
		m_tIconID = a_tIconID;
		m_cOpID = *a_pOpID;
		m_pOpCfg = a_pOpCfg;
		m_pOpMgr = a_pOpMgr;
		m_pStates = a_pStates;
		m_tLocaleID = a_tLocaleID;
		m_hParentWnd = a_hWnd;

		CComPtr<IStorageManager> pSM;
		RWCoCreateInstance(pSM, __uuidof(StorageManager));
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComPtr<IEnumUnknowns> pTypes;
		pIM->DocumentTypesEnum(&pTypes);
		if (FAILED(pSM->FilterWindowCreate(NULL, EFTFileBrowser|EFTMultiselection, a_hWnd, pTypes, NULL, a_pConfig, this, NULL, a_tLocaleID, &m_pStWnd)))
			return E_FAIL;
		m_pStWnd->Show(TRUE);
		return m_pStWnd->Move(a_prc);
	}

	BEGIN_COM_MAP(CDesignerViewFileBrowser)
		COM_INTERFACE_ENTRY(IDesignerView)
		COM_INTERFACE_ENTRY(IChildWindow)
	END_COM_MAP()

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
	{
		if (m_pStWnd)
		{
			CComPtr<IUnknown> p;
			m_pStWnd->QueryInterface(a_iid, reinterpret_cast<void**>(&p));
			if (p)
				a_pInterfaces->Insert(p);
		}
		return S_OK;
	}
	STDMETHOD(OnIdle)()
	{
		return S_OK;
	}
	STDMETHOD(OnDeactivate)(BOOL UNREF(a_bCancelChanges))
	{
		return S_OK;
	}
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges)
	{
		return S_FALSE;
	}

	 // IChildWindow methods
public:
	STDMETHOD(Handle)(RWHWND* a_pHandle)
	{
		return m_pStWnd->Handle(a_pHandle);
	}
	STDMETHOD(SendMessage)(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
	{
		return m_pStWnd->SendMessage(a_uMsg, a_wParam, a_lParam);
	}
	STDMETHOD(Show)(BOOL a_bShow)
	{
		return m_pStWnd->Show(a_bShow);
	}
	STDMETHOD(Move)(RECT const* a_prcPosition)
	{
		return m_pStWnd->Move(a_prcPosition);
	}
	STDMETHOD(Destroy)()
	{
		return m_pStWnd->Destroy();
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		return m_pStWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel);
	}

	// IStorageFilterWindowCallback methods
public:
	STDMETHOD(ForwardOK)()
	{
		try
		{
			if (IsEqualGUID(m_cOpID, __uuidof(DocumentOperationNULL)))
				return S_FALSE;
			CComPtr<IStorageFilter> pFlt;
			m_pStWnd->FilterCreate(&pFlt);
			CComBSTR bstrFile;
			if (pFlt) pFlt->ToText(NULL, &bstrFile);
			if (bstrFile == NULL)
				return S_FALSE;
			CComPtr<ISharedState> pSS;
			RWCoCreateInstance(pSS, __uuidof(SharedStateString));
			pSS->FromText(bstrFile);
			CComObject<COperationContextFromStateManager>* p = NULL;
			CComObject<COperationContextFromStateManager>::CreateInstance(&p);
			CComPtr<IOperationContext> pOpCtx = p;
			p->Init(m_pStates, pSS);
			CComPtr<ILocalizedString> pUndoName;
			CComPtr<ILocalizedString> pFileName;
			if (bstrFile)
			{
				LPCOLESTR pszName = wcsrchr(bstrFile, L'\\');
				LPCOLESTR pszName1 = wcsrchr(bstrFile, L'/');
				if (pszName1 > pszName) pszName = pszName1;
				if (pszName)
					pFileName.Attach(new CSimpleLocalizedString(SysAllocString(pszName+1)));
			}
			if (m_bstrOpName.Length())
			{
				if (pFileName)
				{
					CComObject<CPrintfLocalizedString>* pStr = NULL;
					CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
					pUndoName = pStr;
					CComPtr<ILocalizedString> pTempl;
					pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %s")));

					CComPtr<ILocalizedString> pOpName;
					pOpName.Attach(new CMultiLanguageString(SysAllocString(m_bstrOpName)));
					pStr->Init(pTempl, pOpName, pFileName);
				}
				else
				{
					pUndoName.Attach(new CSimpleLocalizedString(SysAllocString(m_bstrOpName)));
				}
			}
			else
			{
				if (pFileName)
					pUndoName.Attach(pFileName.Detach());
			}
			CUndoBlock cBlock(m_pDocument, pUndoName);
			HRESULT hRes = m_pOpMgr->Activate(m_pOpMgr, m_pDocument, m_cOpID, m_pOpCfg, pOpCtx, m_hParentWnd, m_tLocaleID);
			if (FAILED(hRes) && hRes != E_RW_CANCELLEDBYUSER)
			{
				CComBSTR bstr;
				if (p->M_ErrorMessage())
					p->M_ErrorMessage()->GetLocalized(m_tLocaleID, &bstr);
				CComPtr<IApplicationInfo> pAI;
				RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
				CComPtr<ILocalizedString> pCaption;
				if (pAI) pAI->Name(&pCaption);
				CComBSTR bstrCaption;
				if (pCaption) pCaption->GetLocalized(m_tLocaleID, &bstrCaption);
				if (bstrCaption == NULL) bstrCaption = L"Error";
				if (bstr != NULL && bstr[0])
				{
					::MessageBox(m_hParentWnd, CW2T(bstr), CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
				}
				else
				{
					CComBSTR bstrTempl;
					CMultiLanguageString::GetLocalized(L"[0409]The attempted operation failed with error code 0x%08x. Please verify that there is enough free memory and that the configuration of the operation is correct.[0405]Provedení operace se nezdařilo a byl vrácen chybový kód 0x%08x. Prosím ověřte, že je dostatek volné paměti a konfigurace operace je správná.", m_tLocaleID, &bstrTempl);
					wchar_t szMsg[256];
					_swprintf(szMsg, bstrTempl, hRes);
					::MessageBox(m_hParentWnd, szMsg, CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
				}
			}
			return hRes;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ForwardCancel)() { return S_FALSE; }
	STDMETHOD(DefaultCommand)(ILocalizedString** a_ppName, ILocalizedString** a_ppDesc, GUID* a_pIconID)
	{
		try
		{
			if (a_ppName && m_bstrOpName && m_bstrOpName[0])
			{
				*a_ppName = NULL;
				BSTR b = NULL;
				m_bstrOpName.CopyTo(&b);
				*a_ppName = new CMultiLanguageString(b);
			}
			if (a_ppDesc && m_bstrOpDesc && m_bstrOpDesc[0])
			{
				*a_ppDesc = NULL;
				BSTR b = NULL;
				m_bstrOpDesc.CopyTo(&b);
				*a_ppDesc = new CMultiLanguageString(b);
			}
			if (a_pIconID)
			{
				*a_pIconID = m_tIconID;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(DefaultCommandIcon)(ULONG a_nSize, HICON* a_phIcon)
	{
		*a_phIcon = NULL;
		if (!IsEqualGUID(m_tIconID, GUID_NULL))
		{
			CComPtr<IDesignerFrameIcons> pIcons;
			RWCoCreateInstance(pIcons, __uuidof(DesignerFrameIconsManager));
			if (pIcons)
				return pIcons->GetIcon(m_tIconID, a_nSize, a_phIcon);
		}
		return E_NOTIMPL;
	}

private:
	class ATL_NO_VTABLE COperationContextFromStateManager :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IOperationContext
	{
	public:
		void Init(ISharedStateManager* a_pOrig, ISharedState* a_pFileState)
		{
			m_pFileState = a_pFileState;
			m_pOrig = a_pOrig;
		}
		ILocalizedString* M_ErrorMessage()
		{
			return m_pMessage;
		}
		void ResetErrorMessage()
		{
			m_pMessage = NULL;
		}


	BEGIN_COM_MAP(COperationContextFromStateManager)
		COM_INTERFACE_ENTRY(IOperationContext)
	END_COM_MAP()

		// ISharedStateManager methods
	public:
		STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
		{
			if (a_bstrCategoryName && 0 == wcscmp(L"FILE", a_bstrCategoryName))
				return m_pFileState->QueryInterface(a_iid, a_ppState);
			return m_pOrig->StateGet(a_bstrCategoryName, a_iid, a_ppState);
		}
		STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
		{
			return m_pOrig->StateSet(a_bstrCategoryName, a_pState);
		}
		STDMETHOD(IsCancelled)() { return S_FALSE; }
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
		CComPtr<ISharedStateManager> m_pOrig;
		CComPtr<ILocalizedString> m_pMessage;
		CComPtr<ISharedState> m_pFileState;
	};

private:
	CComPtr<IStorageFilterWindow> m_pStWnd;
	CComBSTR m_bstrOpName;
	CComBSTR m_bstrOpDesc;
	GUID m_tIconID;
	CConfigValue m_cOpID;
	CComPtr<IConfig> m_pOpCfg;
	CComPtr<IOperationManager> m_pOpMgr;
	CComPtr<IDocument> m_pDocument;
	CComPtr<ISharedStateManager> m_pStates;
	LCID m_tLocaleID;
	HWND m_hParentWnd;
};

