
#include "stdafx.h"
#include "StartPageBatchImageProcessor.h"
#include <RWScripting.h>
#include "BatchOperationContext.h"
#include <math.h>
#include <IconRenderer.h>


class ATL_NO_VTABLE CStartPageFactoryBatchImageProcessor :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStartPageFactoryBatchImageProcessor, &CLSID_StartPageBatchImageProcessor>,
	public IStartViewPageFactory,
	public IAnimatedIcon,
	public ICommandLineProcessor,
	public IDesignerBatchOpManager
{
public:
	CStartPageFactoryBatchImageProcessor()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CStartPageFactoryBatchImageProcessor)
	COM_INTERFACE_ENTRY(IStartViewPageFactory)
	COM_INTERFACE_ENTRY(IAnimatedIcon)
	COM_INTERFACE_ENTRY(ICommandLineProcessor)
	COM_INTERFACE_ENTRY(IDesignerBatchOpManager)
END_COM_MAP()


BEGIN_CATEGORY_MAP(CStartPageBatchImageProcessor)
	IMPLEMENTED_CATEGORY(CATID_StartViewPage)
END_CATEGORY_MAP()


	// IStartViewPageFactory methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppName)
	{
		if (a_ppName == nullptr)
			return E_POINTER;
		try
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Batch[0405]Dávka");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(HelpText)(ILocalizedString** a_ppHelpText)
	{
		if (a_ppHelpText == nullptr)
			return E_POINTER;
		try
		{
			*a_ppHelpText = new CMultiLanguageString(L"[0409]Apply filter or resize multiple images at once.[0405]Spustit filtr nebo změnit velikost více obrázků najednou.");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		*a_phIcon = Icon(0, a_nSize);
		return S_OK;
	}

	STDMETHOD(InitConfig)(IConfigWithDependencies* a_pMainConfig)
	{
		try
		{
			return CStartPageBatchImageProcessor::InitConfig(a_pMainConfig, M_OpMgr());
		}
		catch (...)
		{
			return a_pMainConfig ? E_UNEXPECTED : E_POINTER;
		}
	}

	STDMETHOD(Create)(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig, IStartViewPage** a_ppPage)
	{
		try
		{
			*a_ppPage = NULL;
			CComObject<CStartPageBatchImageProcessor>* p = NULL;
			CComObject<CStartPageBatchImageProcessor>::CreateInstance(&p);
			CComPtr<IStartViewPage> pTmp = p;
			p->WindowCreate(a_hParent, a_prc, a_tLocaleID, a_pCallback, a_pAppConfig, M_OpMgr());
			*a_ppPage = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppPage == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	// IAnimatedIcon methods
public:
	enum
	{
		DURATION = 128,
		PHASES = 16,
	};
	STDMETHOD_(ULONG, Phase)(ULONG a_nMSElapsed, ULONG* a_pNextPhase)
	{
		ULONG phase = a_nMSElapsed/DURATION;
		if (a_pNextPhase)
		{
			ULONG delta = a_nMSElapsed-phase*DURATION;
			*a_pNextPhase = DURATION-delta+(DURATION/4); // extra (DURATION/4) is a margin to fall within the interval more reliably
		}
		return phase%PHASES;
	}
	STDMETHOD_(HICON, Icon)(ULONG a_nPhase, ULONG a_nSize)
	{
		try
		{
			CIconRendererReceiver cRenderer(a_nSize);
			// cogwheel 1
			{
				static int const extra = 7;
				static float const r1 = 0.085f;
				static float const r2 = 0.265f;
				static float const r3 = 0.355f;
				static float const cx = 0.355f;
				static float const cy = 0.645f;
				float const rot = 0.15f-a_nPhase*2.0f*3.14159265359f/(extra*PHASES);
				IRPolyPoint aInner[extra*4];
				IRPolyPoint aOuter[10];
				IRPolyPoint* p = aInner;
				for (int i = 0; i < extra; ++i)
				{
					float const a1 = ((i+0.135f)/extra)*2.0f*3.14159265359f+rot;
					float const a2 = ((i+0.315f)/extra)*2.0f*3.14159265359f+rot;
					float const a3 = ((i+0.685f)/extra)*2.0f*3.14159265359f+rot;
					float const a4 = ((i+0.865f)/extra)*2.0f*3.14159265359f+rot;
					p->x = cx + cosf(a1)*r2;
					p->y = cy + sinf(a1)*r2;
					++p;
					p->x = cx + cosf(a2)*r3;
					p->y = cy + sinf(a2)*r3;
					++p;
					p->x = cx + cosf(a3)*r3;
					p->y = cy + sinf(a3)*r3;
					++p;
					p->x = cx + cosf(a4)*r2;
					p->y = cy + sinf(a4)*r2;
					++p;
				}
				p = aOuter;
				for (int i = 0; i < 10; ++i)
				{
					float const a1 = (i*0.2f+0.05f)*3.14159265359f;
					p->x = cx + cosf(a1)*r1;
					p->y = cy + sinf(a1)*r1;
					++p;
				}
				IRPolygon const aCogwheel[] = { {itemsof(aInner), aInner}, {itemsof(aOuter), aOuter} };
				IRFill matCogwheelFill(0xffa38863);
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				IROutlinedFill matCogwheel(&matCogwheelFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
				IRCanvas canvas = {0, 0, 1, 1, 0, 0, NULL, NULL};

				cRenderer(&canvas, itemsof(aCogwheel), aCogwheel, &matCogwheel);
			}
			{
				static int const extra = 5;
				static float const r1 = 0.055f;
				static float const r2 = 0.18f;
				static float const r3 = 0.26f;
				static float const cx = 0.74f;
				static float const cy = 0.26f;
				float const rot = 0.405f+a_nPhase*2.0f*3.14159265359f/(extra*PHASES);
				IRPolyPoint aInner[extra*4];
				IRPolyPoint aOuter[10];
				IRPolyPoint* p = aInner;
				for (int i = 0; i < extra; ++i)
				{
					float const a1 = ((i+0.125f)/extra)*2.0f*3.14159265359f+rot;
					float const a2 = ((i+0.325f)/extra)*2.0f*3.14159265359f+rot;
					float const a3 = ((i+0.675f)/extra)*2.0f*3.14159265359f+rot;
					float const a4 = ((i+0.875f)/extra)*2.0f*3.14159265359f+rot;
					p->x = cx + cosf(a1)*r2;
					p->y = cy + sinf(a1)*r2;
					++p;
					p->x = cx + cosf(a2)*r3;
					p->y = cy + sinf(a2)*r3;
					++p;
					p->x = cx + cosf(a3)*r3;
					p->y = cy + sinf(a3)*r3;
					++p;
					p->x = cx + cosf(a4)*r2;
					p->y = cy + sinf(a4)*r2;
					++p;
				}
				p = aOuter;
				for (int i = 0; i < 10; ++i)
				{
					float const a1 = (i*0.2f+0.05f)*3.14159265359f;
					p->x = cx + cosf(a1)*r1;
					p->y = cy + sinf(a1)*r1;
					++p;
				}
				IRPolygon const aCogwheel[] = { {itemsof(aInner), aInner}, {itemsof(aOuter), aOuter} };
				IRFill matCogwheelFill(0xffc8bc69);
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				IROutlinedFill matCogwheel(&matCogwheelFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
				IRCanvas canvas = {0, 0, 1, 1, 0, 0, NULL, NULL};

				cRenderer(&canvas, itemsof(aCogwheel), aCogwheel, &matCogwheel);
			}
			return cRenderer.get();
		}
		catch (...)
		{
			return NULL;
		}
	}

	// ICommandLineProcessor methods
public:
	STDMETHOD(TargetFolder)(BSTR* a_pbstrPath)
	{
		try
		{
			*a_pbstrPath = NULL;
			TCHAR szBatch[MAX_PATH+4] = _T("");
			GetTempPath(MAX_PATH, szBatch);
			_tcscat(szBatch, _T("RWP"));
			*a_pbstrPath = CComBSTR(szBatch).Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrPath ? E_UNEXPECTED : E_POINTER;
		}
	}

	STDMETHOD(Run)(IConfig* a_pOperation, IEnumStrings* a_pPaths, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrApplication, ICommandLineProcessorFeedback* a_pFeedback)
	{
		try
		{
			CComBSTR bstrAppFolder;
			LPCOLESTR pszAppName = a_bstrApplication ? wcsrchr(a_bstrApplication, L'\\') : NULL;
			if (pszAppName)
			{
				CComPtr<IScriptingInterfaceBasics> pScripting;
				RWCoCreateInstance(pScripting, __uuidof(ScriptingInterfaceBasics));
				pScripting->InitGlobals(CComBSTR(pszAppName+1));
				bstrAppFolder.Attach(::SysAllocStringLen(a_bstrApplication, pszAppName-a_bstrApplication));
			}

			CComPtr<IConfigWithDependencies> p1Op;
			RWCoCreateInstance(p1Op, __uuidof(ConfigWithDependencies));
			CComPtr<ILocalizedString> pStr;
			RWCoCreateInstance(pStr, __uuidof(LocalizedString));
			p1Op->ItemInsSimple(CComBSTR(CFGID_NAME), pStr, pStr, CConfigValue(L"[0409]New operation[0405]Nová operace"), NULL, 0, NULL);
			p1Op->ItemInsSimple(CComBSTR(CFGID_DESCRIPTION), pStr, pStr, CConfigValue(L""), NULL, 0, NULL);
			p1Op->ItemInsSimple(CComBSTR(CFGID_ICONID), pStr, pStr, CConfigValue(GUID_NULL), NULL, 0, NULL);
			p1Op->ItemInsSimple(CComBSTR(CFGID_OUTPUT), pStr, pStr, CConfigValue(L""), NULL, 0, NULL);
			p1Op->ItemInsSimple(CComBSTR(CFGID_FACTORY), pStr, pStr, CConfigValue(GUID_NULL), NULL, 0, NULL);
			CComBSTR cCFGID_OPERATION(CFGID_OPERATION);
			IOperationManager* pOpMgr = M_OpMgr(bstrAppFolder);
			pOpMgr->InsertIntoConfigAs(pOpMgr, p1Op, cCFGID_OPERATION, pStr, pStr, 0, NULL);
			p1Op->Finalize(NULL);
			CopyConfigValues(p1Op, a_pOperation);
			return RunEx(p1Op, a_pPaths, a_hParent, a_tLocaleID, a_bstrApplication, NULL, a_pFeedback, NULL);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(RunEx)(IConfig* p1Op, IEnumStrings* a_pPaths, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrApplication, BSTR a_bstrOutputFolder, ICommandLineProcessorFeedback* a_pFeedback, ICommandLineProcessorProgress* a_pProgress)
	{
		try
		{
			CComBSTR cCFGID_OPERATION(CFGID_OPERATION);
			CConfigValue cOpID;
			p1Op->ItemValueGet(cCFGID_OPERATION, &cOpID);
			if (IsEqualGUID(cOpID, GUID_NULL))
			{
				AddMessage(a_pFeedback, L"Invalid operation");
				return E_FAIL;
			}
			CComPtr<IConfig> pOpCfg;
			p1Op->SubConfigGet(cCFGID_OPERATION, &pOpCfg);
			std::tstring strOutput;
			if (a_bstrOutputFolder)
			{
				strOutput = a_bstrOutputFolder;
			}
			else
			{
				CConfigValue val;
				p1Op->ItemValueGet(CComBSTR(CFGID_OUTPUT), &val);
				strOutput = val.operator BSTR();
			}
			CComPtr<IDocumentBuilder> builder;
			{
				CConfigValue val;
				p1Op->ItemValueGet(CComBSTR(CFGID_FACTORY), &val);
				if (!IsEqualGUID(val, GUID_NULL))
					RWCoCreateInstance(builder, val);
			}
			CConfigValue cName;
			p1Op->ItemValueGet(CComBSTR(CFGID_NAME), &cName);
			CComBSTR bstrLoc;
			CMultiLanguageString::GetLocalized(cName, a_tLocaleID, &bstrLoc);
			if (bstrLoc.Length())
			{
				CComBSTR bstrMsg;
				CMultiLanguageString::GetLocalized(L"[0409]Performing operation[0405]Provádí se operace", a_tLocaleID, &bstrMsg);
				wchar_t szFinal[1024] = L"";
				swprintf(szFinal, 1024, L"\n>>> %s: %s\n", bstrMsg.m_str, bstrLoc.m_str);
				AddMessage(a_pFeedback, szFinal);
			}

			CComPtr<IInputManager> pInMgr;
			RWCoCreateInstance(pInMgr, __uuidof(InputManager));
			CComObject<CBatchOperationContext>* pBOC = NULL;
			CComObject<CBatchOperationContext>::CreateInstance(&pBOC);
			CComPtr<IOperationContext> pBOCTmp = pBOC;

			std::deque<std::pair<std::wstring, size_t> > cPaths;
			ULONG nPaths = 0;
			a_pPaths->Size(&nPaths);
			for (ULONG i = 0; i < nPaths; ++i)
			{
				CComBSTR bstrPath;
				a_pPaths->Get(i, &bstrPath);
				if (bstrPath.Length())
				{
					LPCOLESTR psz1 = wcsrchr(bstrPath, L'\\');
					LPCOLESTR psz2 = wcsrchr(bstrPath, L'/');
					if (psz1 < psz2) psz1 = psz2;
					std::pair<std::wstring, size_t> tItem;
					tItem.first = bstrPath.m_str;
					tItem.second = psz1 ? psz1-bstrPath.m_str : 0;
					cPaths.push_back(tItem);
				}
			}
			ULONG nIndex = 0;
			HRESULT hFinalRes = S_OK;
			while (!cPaths.empty())
			{
				std::pair<std::wstring, size_t> tItem = cPaths.front();
				cPaths.pop_front();
				DWORD dw = GetFileAttributes(tItem.first.c_str());
				if (dw != INVALID_FILE_ATTRIBUTES && (dw&FILE_ATTRIBUTE_DIRECTORY))
				{
					// enum files in directory
					WCHAR szCurrMask[MAX_PATH+16];
					wcscpy(szCurrMask, tItem.first.c_str());
					tItem.first.clear();
					size_t i = wcslen(szCurrMask);
					wcscat(szCurrMask, szCurrMask[i-1] != L'\\' ? L"\\*.*" : L"*.*");
					std::vector<std::tstring> cFiles;
					std::vector<std::tstring> cDirs;
					WIN32_FIND_DATA w32fd;
					CW2CT strCurrMask(szCurrMask);
					HANDLE hFindData = FindFirstFile(strCurrMask, &w32fd);
					if (hFindData != INVALID_HANDLE_VALUE)
					{
						do
						{
							TCHAR szSub[MAX_PATH];
							_tcscpy(szSub, strCurrMask);
							size_t i =_tcslen(szSub);
							if ((i+_tcslen(w32fd.cFileName)-3) < MAX_PATH)
							{
								_tcscpy(szSub+i-3, w32fd.cFileName);
								if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
								{
									if (_tcscmp(w32fd.cFileName, _T(".")) != 0 && _tcscmp(w32fd.cFileName, _T("..")) != 0)
										cDirs.push_back(szSub);
								}
								else
								{
									cFiles.push_back(szSub);
								}
							}
						} while (FindNextFile(hFindData, &w32fd));

						FindClose(hFindData);
					}
					for (std::vector<std::tstring>::const_reverse_iterator i = cFiles.rbegin(); i != cFiles.rend(); ++i)
					{
						tItem.first = static_cast<LPCWSTR>(CT2CW(i->c_str()));
						cPaths.push_front(tItem);
					}
					for (std::vector<std::tstring>::const_reverse_iterator i = cDirs.rbegin(); i != cDirs.rend(); ++i)
					{
						tItem.first = static_cast<LPCWSTR>(CT2CW(i->c_str()));
						cPaths.push_front(tItem);
					}
					continue;
				}
				CComBSTR bstrPath(tItem.first.c_str());
				HRESULT hRes = S_OK;
				try
				{
					CStorageFilter cInPath(bstrPath);
					CComPtr<IDocument> pDoc;
					hRes = pInMgr->DocumentCreateEx(builder, cInPath, NULL, &pDoc);
					CComQIPtr<IDocumentUndo> pUndo(pDoc);
					if (pUndo) pUndo->UndoModeSet(EUMDisabled);
					if (a_pProgress)
					{
						if (S_FALSE == a_pProgress->Tick(nIndex, cPaths.size()+1))
						{
							AddMessage(a_pFeedback, L"Stopped by user.");
							return E_RW_CANCELLEDBYUSER;
						}
					}
					if (FAILED(hRes))
					{
						AddMessage(a_pFeedback, L"Stopped by user.");
						hFinalRes = hRes;
						continue;
					}
					pBOC->Step(nIndex++, cPaths.size(), tItem.second, strOutput);
					IOperationManager* pOpMgr = M_OpMgr();
					if (SUCCEEDED(hRes = pOpMgr->Activate(pOpMgr, pDoc, cOpID, pOpCfg, pBOCTmp, a_hParent, a_tLocaleID)))
					{
						CComBSTR bstrMsg;
						CMultiLanguageString::GetLocalized(L"[0409]Processed - [0405]Zpracováno - ", a_tLocaleID, &bstrMsg);
						bstrMsg += bstrPath;
						AddMessage(a_pFeedback, bstrMsg);
						continue;
					}
				}
				catch (...)
				{
				}
				if (FAILED(hRes))
				{
					if (hRes == E_RW_CANCELLEDBYUSER)
					{
						AddMessage(a_pFeedback, L"Stopped by user.");
						return E_RW_CANCELLEDBYUSER;
					}
					hFinalRes = hRes;
				}
				wchar_t szError[16] = _T("");
				CComBSTR bstrError;
				if (pBOC->M_ErrorMessage())
					pBOC->M_ErrorMessage()->GetLocalized(a_tLocaleID, &bstrError);
				if (bstrError == NULL || bstrError[0] == L'\0')
					swprintf(szError, L"0x%08x", hRes);
				CComBSTR bstrMsg;
				CMultiLanguageString::GetLocalized(L"[0409]Error: %s - %s[0405]Chyba: %s - %s", a_tLocaleID, &bstrMsg);
				wchar_t szFinal[1024] = L"";
				swprintf(szFinal, bstrMsg.m_str, szError[0] ? szError : bstrError.m_str, bstrPath.m_str/*strPath.c_str()*/);
				AddMessage(a_pFeedback, szFinal);
			}
			if (a_pProgress) a_pProgress->Tick(nIndex, 0);
			return hFinalRes;
		}
		catch (...)
		{
			AddMessage(a_pFeedback, L"Unexpected error.");
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(InitOpConfig)(BSTR a_bstrApplication, IConfig** a_ppOperation)
	{
		if (a_ppOperation == NULL)
			return E_POINTER;

		CComBSTR bstrAppFolder;
		LPCOLESTR pszAppName = a_bstrApplication ? wcsrchr(a_bstrApplication, L'\\') : NULL;
		if (pszAppName)
		{
			CComPtr<IScriptingInterfaceBasics> pScripting;
			RWCoCreateInstance(pScripting, __uuidof(ScriptingInterfaceBasics));
			pScripting->InitGlobals(CComBSTR(pszAppName+1));
			bstrAppFolder.Attach(::SysAllocStringLen(a_bstrApplication, pszAppName-a_bstrApplication));
		}

		return CStartPageBatchImageProcessor::Init1OpConfig(M_OpMgr(bstrAppFolder), a_ppOperation);
		//CComPtr<IConfigWithDependencies> p1Op;
		//RWCoCreateInstance(p1Op, __uuidof(ConfigWithDependencies));
		//CComPtr<ILocalizedString> pStr;
		//RWCoCreateInstance(pStr, __uuidof(LocalizedString));
		//p1Op->ItemInsSimple(CComBSTR(CFGID_NAME), pStr, pStr, CConfigValue(L"[0409]New operation[0405]Nová operace"), NULL, 0, NULL);
		//p1Op->ItemInsSimple(CComBSTR(CFGID_DESCRIPTION), pStr, pStr, CConfigValue(L""), NULL, 0, NULL);
		//p1Op->ItemInsSimple(CComBSTR(CFGID_ICONID), pStr, pStr, CConfigValue(GUID_NULL), NULL, 0, NULL);
		//p1Op->ItemInsSimple(CComBSTR(CFGID_OUTPUT), pStr, pStr, CConfigValue(L""), NULL, 0, NULL);
		//p1Op->ItemInsSimple(CComBSTR(CFGID_FACTORY), pStr, pStr, CConfigValue(GUID_NULL), NULL, 0, NULL);
		//CComBSTR cCFGID_OPERATION(CFGID_OPERATION);
		//IOperationManager* pOpMgr = M_OpMgr();
		//pOpMgr->InsertIntoConfigAs(pOpMgr, p1Op, cCFGID_OPERATION, pStr, pStr, 0, NULL);
		//p1Op->Finalize(NULL);
		//*a_ppOperation = p1Op.Detach();
		//return S_OK;
	}
	STDMETHOD(GetOpProps)(IConfig* a_pOperation, BSTR* a_pName, BSTR* a_pDesc, GUID* a_pIconID, BSTR* a_pOutput, GUID* a_pFactory, BSTR* a_pOrigName, BSTR* a_pCustomFilter)
	{
		if (a_pOperation == NULL)
			return E_POINTER;

		if (a_pName)
		{
			CConfigValue val;
			a_pOperation->ItemValueGet(CComBSTR(CFGID_NAME), &val);
			*a_pName = val.TypeGet() == ECVTString ? val.Detach().bstrVal : NULL;
		}

		if (a_pDesc)
		{
			CConfigValue val;
			a_pOperation->ItemValueGet(CComBSTR(CFGID_DESCRIPTION), &val);
			*a_pDesc = val.TypeGet() == ECVTString ? val.Detach().bstrVal : NULL;
		}

		if (a_pIconID)
		{
			CConfigValue val;
			a_pOperation->ItemValueGet(CComBSTR(CFGID_ICONID), &val);
			*a_pIconID = val.TypeGet() == ECVTGUID ? val.Detach().guidVal : GUID_NULL;
		}

		if (a_pOutput)
		{
			CConfigValue val;
			a_pOperation->ItemValueGet(CComBSTR(CFGID_OUTPUT), &val);
			*a_pOutput = val.TypeGet() == ECVTString ? val.Detach().bstrVal : NULL;
		}

		if (a_pFactory)
		{
			CConfigValue val;
			a_pOperation->ItemValueGet(CComBSTR(CFGID_FACTORY), &val);
			*a_pFactory = val.TypeGet() == ECVTGUID ? val.Detach().guidVal : GUID_NULL;
		}

		if (a_pOrigName)
		{
			CConfigValue val;
			a_pOperation->ItemValueGet(CComBSTR(CFGID_PREVNAME), &val);
			*a_pOrigName = val.TypeGet() == ECVTString ? val.Detach().bstrVal : NULL;
		}

		if (a_pCustomFilter)
		{
			CConfigValue val;
			a_pOperation->ItemValueGet(CComBSTR(CFGID_CUSTOMFILTER), &val);
			*a_pCustomFilter = val.TypeGet() == ECVTString ? val.Detach().bstrVal : NULL;
		}

		return S_OK;
	}
	STDMETHOD(GetSubOps)(IConfig* a_pOperation, LCID a_tLocaleID, IEnumUnknowns** a_pSubOps)
	{
		if (a_pSubOps == NULL)
			return E_POINTER;
		try
		{
			*a_pSubOps = NULL;
			CComPtr<IEnumUnknownsInit> p;
			RWCoCreateInstance(p, __uuidof(EnumUnknowns));
			CStartPageBatchImageProcessor::CSubOps subOps;
			CStartPageBatchImageProcessor::GetSubOps(a_pOperation, subOps);
			for (CStartPageBatchImageProcessor::CSubOps::const_iterator i = subOps.begin(); i != subOps.end(); ++i)
			{
				CComBSTR bstrID(CFGID_OPERATION);
				CComPtr<IConfig> pOp;
				a_pOperation->DuplicateCreate(&pOp);
				CComPtr<IConfig> pSubOp;
				pOp->SubConfigGet(bstrID, &pSubOp);
				CopyConfigValues(pSubOp, i->config);
				CComBSTR b(L"[0000]");
				CComBSTR b2;
				i->name->GetLocalized(a_tLocaleID, &b2);
				b += b2;
				CComBSTR bstrName(CFGID_NAME);
				CConfigValue prevName;
				pOp->ItemValueGet(bstrName, &prevName);
				CComBSTR bstrIcon(CFGID_ICONID);
				CComBSTR bstrPrev(CFGID_PREVNAME);
				BSTR aIDs[3] = {bstrName, bstrIcon, bstrPrev};
				TConfigValue aVals[3];
				ZeroMemory(aVals, sizeof aVals);
				aVals[0].eTypeID = ECVTString;
				aVals[0].bstrVal = b;
				aVals[1].eTypeID = ECVTGUID;
				aVals[1].guidVal = GUID_NULL;
				aVals[2] = prevName;
				pOp->ItemValuesSet(3, aIDs, aVals);
				p->Insert(pOp);
			}
			*a_pSubOps = p.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}


	// IDesignerBatchOpManager methods
public:
	STDMETHOD(HasOperation)(IConfig* a_pMainConfig, BSTR a_bstrOpName)
	{
		try
		{
			CComPtr<IConfig> pRootCfg;
			a_pMainConfig->SubConfigGet(CComBSTR(CFGID_ROOT), &pRootCfg);
			CConfigValue cOps;
			pRootCfg->ItemValueGet(CComBSTR(CFGID_OPERATIONS), &cOps);
			for (LONG i = 0; i < cOps.operator LONG(); ++i)
			{
				OLECHAR szTmp[128];
				swprintf(szTmp, L"%s\\%08x\\%s", CFGID_OPERATIONS, i, CFGID_NAME);
				CConfigValue cName;
				pRootCfg->ItemValueGet(CComBSTR(szTmp), &cName);
				CComBSTR bstrLoc;
				CMultiLanguageString::GetLocalized(cName, 0x0409, &bstrLoc);
				if (bstrLoc == a_bstrOpName)
					return S_OK;
			}
			return S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(AddOperation)(IConfig* a_pMainConfig, ULONG a_nLen, BYTE const* a_pData)
	{
		try
		{
			CComPtr<IConfig> pRootCfg;
			a_pMainConfig->SubConfigGet(CComBSTR(CFGID_ROOT), &pRootCfg);

			CComPtr<IConfigInMemory> pMemCfg;
			RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
			pMemCfg->DataBlockSet(a_nLen, a_pData);

			CConfigValue cOps;
			CComBSTR cCFGID_OPERATIONS(CFGID_OPERATIONS);
			pRootCfg->ItemValueGet(cCFGID_OPERATIONS, &cOps);
			OLECHAR szTmp[64];
			swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, cOps.operator LONG());
			CConfigValue cOps2(cOps.operator LONG()+1L);
			BSTR bstrs[1];
			bstrs[0] = cCFGID_OPERATIONS;
			TConfigValue vals[1];
			vals[0] = cOps2;
			pRootCfg->ItemValuesSet(1, bstrs, vals);
			CComPtr<IConfig> p1Op;
			if (FAILED(pRootCfg->SubConfigGet(CComBSTR(szTmp), &p1Op)))
				return E_FAIL;
			return CopyConfigValues(p1Op, pMemCfg);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}


private:
	IOperationManager* M_OpMgr(BSTR a_bstrAppFolder = NULL)
	{
		if (m_pOpMgr)
			return m_pOpMgr;
		ObjectLock cLock(this);
		if (m_pOpMgr == NULL)
		{
			CComPtr<IOperationManager> pOpMgr;
			RWCoCreateInstance(pOpMgr, __uuidof(OperationManager));
			CComObject<CBatchOperationManager>* p = NULL;
			CComObject<CBatchOperationManager>::CreateInstance(&p);
			m_pOpMgr = p;
			p->Init(pOpMgr, a_bstrAppFolder);
		}
		return m_pOpMgr;
	}
	static void AddMessage(ICommandLineProcessorFeedback* a_pFeedback, LPCWSTR a_psz)
	{
		if (a_pFeedback && a_psz) a_pFeedback->Message(CComBSTR(a_psz));
	}


private:
	CComPtr<IOperationManager> m_pOpMgr;
};

OBJECT_ENTRY_AUTO(__uuidof(StartPageBatchImageProcessor), CStartPageFactoryBatchImageProcessor)
