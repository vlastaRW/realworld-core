
#pragma once

#include <DocumentName.h>
#include <ConfigCustomGUIImpl.h>
#include <MultiLanguageString.h>

extern __declspec(selectany) OLECHAR const CFGID_OUTPUTPATH[] = L"OutputPath";
extern __declspec(selectany) OLECHAR const STATE_DROPROOT[] = L"BatchDropRoot";
extern __declspec(selectany) OLECHAR const CFGID_FILETIME[] = L"FileTime";


class ATL_NO_VTABLE CConfigGUIBatchOutputDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIBatchOutputDlg>,
	public CDialogResize<CConfigGUIBatchOutputDlg>
{
public:
	enum { IDC_OUTPUTPATH = 100, IDC_KEEPMODIFYTIME };

	BEGIN_DIALOG_EX(0, 0, 150, 26, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Output path:[0405]Cílová cesta:"), IDC_STATIC, 0, 2, 48, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_OUTPUTPATH, 52, 0, 97, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CHECKBOX(L"[0409]Preserve file modification date[0405]Zachovat datum úpravy souboru", IDC_KEEPMODIFYTIME, 0, 16, 150, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIBatchOutputDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIBatchOutputDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIBatchOutputDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIBatchOutputDlg)
		DLGRESIZE_CONTROL(IDC_OUTPUTPATH, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_KEEPMODIFYTIME, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIBatchOutputDlg)
		CONFIGITEM_EDITBOX(IDC_OUTPUTPATH, CFGID_OUTPUTPATH)
		CONFIGITEM_CHECKBOX_FLAG(IDC_KEEPMODIFYTIME, CFGID_FILETIME, 1<<ESTTModification)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

extern __declspec(selectany) GUID const BatchOperationManagerID = {0xeb5f49b5, 0xb6a6, 0x488d, {0xbd, 0x22, 0x82, 0x78, 0x55, 0x4d, 0x84, 0x67}};

class ATL_NO_VTABLE CBatchOperationManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationManager
{
public:
	CBatchOperationManager()
	{
	}
	void Init(IOperationManager* a_pManager, BSTR a_bstrAppFolder)
	{
		if (a_pManager == NULL)
			throw E_POINTER;
		m_pManager = a_pManager;
		RWCoCreateInstance(m_pIM, __uuidof(InputManager));
		m_bstrAppFolder = a_bstrAppFolder;
		if (m_bstrAppFolder.Length() == 0)
		{
			TCHAR sz[MAX_PATH];
			GetModuleFileName(NULL, sz, MAX_PATH);
			*wcsrchr(sz, _T('\\')) = L'\0';
			m_bstrAppFolder = sz;
		}
	}

BEGIN_COM_MAP(CBatchOperationManager)
	COM_INTERFACE_ENTRY(IOperationManager)
	COM_INTERFACE_ENTRY(ILateConfigCreator)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// ILateConfigCreator methods
public:
	STDMETHOD(CreateConfig)(TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
	{
		return CreateConfigEx(this, a_ptControllerID, a_ppConfig);
	}

	// IOperationManager methods
public:
	STDMETHOD(CreateConfigEx)(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
	{
		try
		{
			*a_ppConfig = NULL;
			if (IsEqualGUID(BatchOperationManagerID, a_ptControllerID->guidVal))
			{
				*a_ppConfig = NULL;
				CComPtr<IConfigWithDependencies> pCfg;
				RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
				pCfg->ItemInsSimple(CComBSTR(CFGID_OUTPUTPATH), CMultiLanguageString::GetAuto(L"[0409]Output path[0405]Cílová cesta"), CMultiLanguageString::GetAuto(L"[0409]Path, where to save the processed file. If the file exists, it will be overwritten unless it has a read-only flag. Following placeholders can be used: %BATCH% - temporary folder, %FOLDER% - original folder, %NAME% - original name, %EXT% - original extension, %INDEX% - batch counter.[0405]Cesta, kam bude uložen zpracovaný soubor. Pokud soubor existuje, bude přepsán. Je možno použít následující zástupce: %BATCH% - dočasná složka, %FOLDER% - původní složka, %NAME% - původní jméno, %EXT% - původní přípona, %INDEX% - dávkové počítadlo."), CConfigValue(L"%BATCH%%NAME%.%EXT%"), NULL, 0, NULL);
				pCfg->ItemInsSimple(CComBSTR(CFGID_FILETIME), CMultiLanguageString::GetAuto(L"[0409]Timestamps[0405]Časové značky"), CMultiLanguageString::GetAuto(L"[0409]Preserve file timestamps if possible.[0405]Zachovat časové značky souboru, je-li to možné."), CConfigValue(1L<<ESTTCreation), NULL, 0, NULL);
				CConfigCustomGUI<&BatchOperationManagerID, CConfigGUIBatchOutputDlg>::FinalizeConfig(pCfg);
				*a_ppConfig = pCfg.Detach();
				return S_OK;
			}
			return m_pManager->CreateConfigEx(a_pOverrideForItem, a_ptControllerID, a_ppConfig);
		}
		catch (...)
		{
			return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	STDMETHOD(ItemGetCount)(ULONG* a_pnCount)
	{
		try
		{
			ULONG nCount = 0;
			HRESULT hRes = m_pManager->ItemGetCount(&nCount);
			*a_pnCount = nCount+1;
			return hRes;
		}
		catch (...)
		{
			return a_pnCount == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ItemIDGetDefault)(TConfigValue* a_ptDefaultOpID)
	{
		return m_pManager->ItemIDGetDefault(a_ptDefaultOpID);
	}
	STDMETHOD(ItemIDGet)(IOperationManager* a_pOverrideForItem, ULONG a_nIndex, TConfigValue* a_ptOperationID, ILocalizedString** a_ppName)
	{
		if (a_ppName == nullptr || a_ptOperationID == nullptr)
			return E_POINTER;
		try
		{
			if (a_nIndex == 0)
			{
				a_ptOperationID->eTypeID = ECVTGUID;
				a_ptOperationID->guidVal = BatchOperationManagerID;
				*a_ppName = new CMultiLanguageString(L"[0409]Batch - Save Result[0405]Dávka - uložit výsledek");
				return S_OK;
			}
			else
			{
				return m_pManager->ItemIDGet(a_pOverrideForItem ? a_pOverrideForItem : this, a_nIndex-1, a_ptOperationID, a_ppName);
			}
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(InsertIntoConfigAs)(IOperationManager* a_pOverrideForItem, IConfigWithDependencies* a_pConfig, BSTR a_bstrID, ILocalizedString* a_pItemName, ILocalizedString* a_pItemDesc, ULONG a_nItemConditions, TConfigOptionCondition const* a_aItemConditions)
	{
		try
		{
			CComPtr<ISubConfigSwitchLate> pSubCfg;
			RWCoCreateInstance(pSubCfg, __uuidof(SubConfigSwitchLate));
			HRESULT hRes = pSubCfg->Init(a_pOverrideForItem ? a_pOverrideForItem : this);
			if (FAILED(hRes)) return hRes;
			CConfigValue cDefault;
			hRes = m_pManager->ItemIDGetDefault(&cDefault);
			if (FAILED(hRes)) return hRes;
			hRes = a_pConfig->ItemIns1ofN(a_bstrID, a_pItemName, a_pItemDesc, cDefault, pSubCfg);
			if (FAILED(hRes)) return hRes;

			ULONG nCount = 0;
			hRes = ItemGetCount(&nCount);
			if (FAILED(hRes))
				return hRes;

			for (ULONG i = 0; i != nCount; ++i)
			{
				CComPtr<ILocalizedString> pStr;
				CConfigValue cVal;
				ItemIDGet(a_pOverrideForItem, i, &cVal, &pStr);
				a_pConfig->ItemOptionAdd(a_bstrID, cVal, pStr, a_nItemConditions, a_aItemConditions);
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CanActivate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates)
	{
		try
		{
			if (IsEqualGUID(BatchOperationManagerID, a_ptOperationID->guidVal))
			{
				return a_pDocument != NULL ? S_OK : S_FALSE;
			}
			return m_pManager->CanActivate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_ptOperationID, a_pConfig, a_pStates);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Activate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			if (IsEqualGUID(BatchOperationManagerID, a_ptOperationID->guidVal))
			{
				CComPtr<IConfig> pConfig;
				m_pIM->SaveOptionsGet(a_pDocument, &pConfig, NULL, NULL);

				CConfigValue cTemplate;
				a_pConfig->ItemValueGet(CComBSTR(CFGID_OUTPUTPATH), &cTemplate);
				if (cTemplate.operator BSTR()[0] == L'\0')
				{
					return m_pIM->Save(a_pDocument, pConfig, NULL);
				}

				TCHAR szBatch[MAX_PATH+4] = _T("");
				CComPtr<ISharedState> pBatchRoot;
				if (a_pStates) a_pStates->StateGet(CComBSTR(L"OutputFolder"), __uuidof(ISharedState), reinterpret_cast<void**>(&pBatchRoot));
				if (pBatchRoot)
				{
					CComBSTR bstrBatchRoot;
					pBatchRoot->ToText(&bstrBatchRoot);
					if (bstrBatchRoot.Length())
						_tcscpy(szBatch, bstrBatchRoot);
				}
				if (szBatch[0] == _T('\0'))
				{
					GetTempPath(MAX_PATH, szBatch);
					_tcscat(szBatch, _T("RWBatch\\"));
				}

				ULONG nCounter = 0;
				if (a_pStates) a_pStates->GetOperationInfo(&nCounter, NULL, NULL, NULL);
				OLECHAR szCounter[16] = _T("");
				swprintf(szCounter, L"%i", nCounter);
				CComPtr<ISharedState> pDropRoot;
				if (a_pStates) a_pStates->StateGet(CComBSTR(STATE_DROPROOT), __uuidof(ISharedState), reinterpret_cast<void**>(&pDropRoot));
				ULONG nDropRootLen = 0;
				if (pDropRoot)
				{
					CComBSTR bstrDropRoot;
					pDropRoot->ToText(&bstrDropRoot);
					if (bstrDropRoot.Length())
						nDropRootLen = _wtoi(bstrDropRoot.m_str);
				}

				OLECHAR szRoot[MAX_PATH] = _T("");
				OLECHAR szName[MAX_PATH] = _T("");
				OLECHAR szExt[MAX_PATH] = _T("");
				ULONG nRootLen = 0;
				CComPtr<IStorageFilter> pFlt;
				a_pDocument->LocationGet(&pFlt);
				if (pFlt)
				{
					CComQIPtr<IDocumentName> pDN(pFlt);
					if (pDN != NULL)
					{
						CComBSTR bstrName;
						pDN->GetName(&bstrName);
						if (bstrName != NULL)
							wcsncpy(szName, bstrName, MAX_PATH);
					}
					else
					{
						CComBSTR bstrFileName;
						pFlt->ToText(NULL, &bstrFileName);
						if (bstrFileName != NULL)
						{
							wcsncpy(szRoot, bstrFileName, MAX_PATH);
							LPOLESTR pBack = wcsrchr(szRoot, L'\\');
							LPOLESTR pForw = wcsrchr(szRoot, L'/');
							LPOLESTR pName = max(pBack, pForw);
							if (pName)
							{
								wcscpy(szName, pName+1);
								pName[1] = L'\0';
							}
						}
					}
					LPOLESTR pDot = wcsrchr(szName, L'.');
					if (pDot)
					{
						wcscpy(szExt, pDot+1);
						*pDot = L'\0';
					}
				}
				nRootLen = wcslen(szRoot);
				if (nDropRootLen == 0 || nDropRootLen > nRootLen)
					nDropRootLen = nRootLen;
				OLECHAR szOutPath[MAX_PATH+MAX_PATH] = L"";
				LPOLESTR pszOut = szOutPath;
				LPOLESTR pszIn = cTemplate;
				while (*pszIn)
				{
					if (_wcsnicmp(pszIn, L"%ROOT%", 6) == 0)
					{
						if (nRootLen == 0)
							return E_FAIL; // TODO: error message
						pszIn += 6;
						wcsncpy(pszOut, szRoot, nDropRootLen);
						pszOut += nDropRootLen;
					}
					else if (_wcsnicmp(pszIn, L"%PATH%", 6) == 0)
					{
						if (nRootLen == 0)
							return E_FAIL; // TODO: error message
						pszIn += 6;
						wcscpy(pszOut, szRoot+nDropRootLen);
						pszOut += nRootLen-nDropRootLen;
					}
					else if (_wcsnicmp(pszIn, L"%FOLDER%", 8) == 0)
					{
						if (nRootLen == 0)
							return E_FAIL; // TODO: error message
						pszIn += 8;
						wcscpy(pszOut, szRoot);
						pszOut += nRootLen;
					}
					else if (_wcsnicmp(pszIn, L"%BATCH%", 7) == 0)
					{
						pszIn += 7;
						wcscpy(pszOut, szBatch);
						pszOut += wcslen(szBatch);
					}
					else if (_wcsnicmp(pszIn, L"%NAME%", 6) == 0)
					{
						if (szName[0] == L'\0')
							return E_FAIL; // TODO: error message
						pszIn += 6;
						wcscpy(pszOut, szName);
						pszOut += wcslen(szName);
					}
					else if (_wcsnicmp(pszIn, L"%EXT%", 5) == 0)
					{
						pszIn += 5;
						wcscpy(pszOut, szExt);
						pszOut += wcslen(szExt);
					}
					else if (_wcsnicmp(pszIn, L"%INDEX%", 7) == 0)
					{
						pszIn += 7;
						wcscpy(pszOut, szCounter);
						pszOut += wcslen(szCounter);
					}
					else if (_wcsnicmp(pszIn, L"%APPFOLDER%", 11) == 0)
					{
						pszIn += 11;
						wcscpy(pszOut, m_bstrAppFolder);
						pszOut += m_bstrAppFolder.Length();
					}
					else
					{
						// prevent double slashes
						if (pszOut == szOutPath || (pszOut[-1] != L'/' && pszOut[-1] != L'\\') || (*pszIn != L'/' && *pszIn != L'\\'))
							*(pszOut++) = *pszIn;
						++pszIn;
					}
				}
				*pszOut = L'\0';

				CComPtr<IApplicationInfo> pAI;
				RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
				ULONG nDays = 0;
				ELicensingMode eMode = ELMDonate;
				pAI->LicensingMode(&eMode);
				if (eMode != ELMEnterSerial || S_OK == pAI->License(NULL, NULL, NULL, NULL, &nDays) || nDays <= 30)
				{
					CStorageFilter pDst(szOutPath);
					HRESULT hr = m_pIM->Save(a_pDocument, pConfig, pDst);
					if (FAILED(hr)) return hr;
					CComQIPtr<IStorageLocatorAttrs> pSrcAttrs(pFlt);
					CComQIPtr<IStorageLocatorAttrs> pDstAttrs(pDst);
					if (pSrcAttrs && pDstAttrs)
					{
						CConfigValue cStamps;
						a_pConfig->ItemValueGet(CComBSTR(CFGID_FILETIME), &cStamps);
						LONG stamps = cStamps;
						ULONGLONG t = 0;
						if (stamps&(1<<ESTTCreation))
							if (SUCCEEDED(pSrcAttrs->GetTime(ESTTCreation, &t)))
								pDstAttrs->SetTime(ESTTCreation, t);
						if (stamps&(1<<ESTTModification))
							if (SUCCEEDED(pSrcAttrs->GetTime(ESTTModification, &t)))
								pDstAttrs->SetTime(ESTTModification, t);
						if (stamps&(1<<ESTTAccess))
							if (SUCCEEDED(pSrcAttrs->GetTime(ESTTAccess, &t)))
								pDstAttrs->SetTime(ESTTAccess, t);
					}
					return hr;
				}
				a_pStates->SetErrorMessage(CMultiLanguageString::GetAuto(L"[0409]Evaluation period expired.\n\nSaving functionality will be restored after valid license is entered.[0405]Zkušební doba vypršela.\n\nUkládání souborů bude umožněno po vložení platné licence."));
				return E_FAIL;
			}
			return m_pManager->Activate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_ptOperationID, a_pConfig, a_pStates, a_hParent, a_tLocaleID);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Visit)(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor)
	{
		return m_pManager->Visit(a_pOverrideForItem ? a_pOverrideForItem : this, a_ptOperationID, a_pConfig, a_pVisitor);
	}

private:
	CComPtr<IOperationManager> m_pManager;
	CComPtr<IInputManager> m_pIM;
	CComBSTR m_bstrAppFolder;
};
