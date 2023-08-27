// DesignerCore.cpp : Implementation of CDesignerCore

#include "stdafx.h"
#include "DesignerCore.h"

#include "WindowThread.h"
//#include "TabbedWindowThread.h"
#include "MainFrame.h"
//#include "TabbedMainFrame.h"
#include "RecentFiles.h"
#include "ConfigIDsApp.h"
#include <StringParsing.h>
#include "GlobalConfigMainFrame.h"
#include <RWLocalization.h>

LCID GetLocaleID(IGlobalConfigManager* a_pGlobalConfigMgr)
{
	CComPtr<IConfig> pLangCfg;
	if (a_pGlobalConfigMgr)
		a_pGlobalConfigMgr->Config(__uuidof(Translator), &pLangCfg);
	if (pLangCfg)
	{
		CConfigValue cLCID;
		pLangCfg->ItemValueGet(CComBSTR(CFGID_LANGUAGECODE), &cLCID);
		return cLCID.operator LONG();
	}
	LCID tLCID = GetThreadLocale();
	if (LANGIDFROMLCID(tLCID) == MAKELANGID(LANG_SLOVAK, SUBLANG_DEFAULT) || LANGIDFROMLCID(tLCID) == MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT))
		return MAKELCID(MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT), SORT_DEFAULT);
	else
		return MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT);
}

// CDesignerCore

STDMETHODIMP CDesignerCore::Initialize(IConfig* a_pConfig, IApplicationInfo* a_pAppInfo, ULONG a_nStartPages, CLSID const* a_aStartPages, REFCLSID a_tDefaultStartPage, IConfig* a_pStartPageMenu, EMDIType a_eMDIType)
{
	try
	{
		ATLASSERT(m_bInitialized == FALSE);

		ObjectLock cLock(this);

		RWCoCreateInstance(m_pThreadPool, __uuidof(ThreadPool)); // keep thread pool alive always

		m_pDefaultConfig = a_pConfig;
		m_pAppInfo = a_pAppInfo;
		if (a_nStartPages)
		{
			m_aStartPages.Allocate(a_nStartPages);
			CopyMemory(m_aStartPages.m_p, a_aStartPages, a_nStartPages*sizeof(CLSID));
			m_nStartPages = a_nStartPages;
		}
		else
		{
			m_nStartPages = 0;
		}
		if (IsEqualGUID(a_tDefaultStartPage, GUID_NULL))
			m_tDefaultStartPage = __uuidof(StartPageOnline);
		else
			m_tDefaultStartPage = a_tDefaultStartPage;
		m_pStartPageMenu = a_pStartPageMenu;
		if (a_eMDIType == EMTMSDI)
		{
			m_bTabbed = false;
		}
		else if (a_eMDIType == EMTTabs)
		{
			m_bTabbed = true;
		}
		else if (a_eMDIType == EMTDefault)
		{
			// TODO: read document interface flag from configuration
		}
#ifdef _DEBUG
		m_bInitialized = TRUE;
#endif//_DEBUG
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP CDesignerCore::NewWindowPath(BSTR a_bstrFullPath, RWHWND* a_pCreatedWindow)
{
	try
	{
		HANDLE hWndCreated = a_pCreatedWindow ? CreateEvent(NULL, FALSE, FALSE, NULL) : NULL;
		QueueCommand(a_bstrFullPath == NULL ?
			new CThreadCommand(hWndCreated, a_pCreatedWindow, __uuidof(StartViewPageFactoryOpenFile)) :
			new CThreadCommand(hWndCreated, a_pCreatedWindow, a_bstrFullPath));

		StartManager();
		if (!IsManagerRunning())
			return E_FAIL;

		if (a_pCreatedWindow)
		{
			WaitForSingleObject(hWndCreated, INFINITE);
			//AtlWaitWithMessageLoop(hWndCreated);
			CloseHandle(hWndCreated);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP CDesignerCore::NewWindowDocument(IDocument* a_pDocument, RWHWND* a_pCreatedWindow)
{
	try
	{
		HANDLE hWndCreated = a_pCreatedWindow ? CreateEvent(NULL, FALSE, FALSE, NULL) : NULL;
		QueueCommand(a_pDocument == NULL ?
			new CThreadCommand(hWndCreated, a_pCreatedWindow, __uuidof(StartViewPageFactoryNewDocument)) :
			new CThreadCommand(hWndCreated, a_pCreatedWindow, a_pDocument));

		StartManager();
		if (!IsManagerRunning())
			return E_FAIL;

		if (a_pCreatedWindow)
		{
			WaitForSingleObject(hWndCreated, INFINITE);
			//AtlWaitWithMessageLoop(hWndCreated);
			CloseHandle(hWndCreated);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP CDesignerCore::NewWindowPage(REFCLSID a_tStartPageID, RWHWND* a_pCreatedWindow)
{
	try
	{
		HANDLE hWndCreated = a_pCreatedWindow ? CreateEvent(NULL, FALSE, FALSE, NULL) : NULL;
		QueueCommand(new CThreadCommand(hWndCreated, a_pCreatedWindow, a_tStartPageID));

		StartManager();
		if (!IsManagerRunning())
			return E_FAIL;

		if (a_pCreatedWindow)
		{
			WaitForSingleObject(hWndCreated, INFINITE);
			//AtlWaitWithMessageLoop(hWndCreated);
			CloseHandle(hWndCreated);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP CDesignerCore::MessagePump(ULONG a_nStopEvents, HANDLE* a_pStopEvents, ULONG* a_pWaitResult)
{
	try
	{
		if (a_nStopEvents >= MAXIMUM_WAIT_OBJECTS)
			return E_RW_INVALIDPARAM;

		HANDLE aHandles[MAXIMUM_WAIT_OBJECTS];
		for (ULONG i = 0; i < a_nStopEvents; ++i)
			aHandles[i] = a_pStopEvents[i];
		aHandles[a_nStopEvents] = m_hWinThreadsFinished;
		DWORD dwRes = WaitForMultipleObjects(a_nStopEvents+1, aHandles, FALSE, INFINITE);
		if (a_pWaitResult)
			*a_pWaitResult = dwRes;
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

//STDMETHODIMP CDesignerCore::Finish(ULONG a_nMaxWaitTime)
//{
//	if (a_nMaxWaitTime == INFINITE)
//	{
//		AtlWaitWithMessageLoop(m_hWinThreadsFinished);
//		return S_OK;
//	}
//	else
//	{
//		return WaitForSingleObject(m_hWinThreadsFinished, a_nMaxWaitTime) == WAIT_OBJECT_0 ? S_OK : S_FALSE;
//	}
//}
//
//STDMETHODIMP CDesignerCore::GetFinishedWaitHandle(HANDLE* a_phEvent)
//{
//	try
//	{
//		*a_phEvent = m_hWinThreadsFinished;
//		return S_OK;
//	}
//	catch (...)
//	{
//		return E_POINTER;
//	}
//}

void CDesignerCore::QueueCommand(CThreadCommand* a_pCommand)
{
	EnterCriticalSection(&m_tManagerCritSec);
	m_cCommands.push(a_pCommand);
	SetEvent(m_hNewOrder);
	if (!a_pCommand->IsQuitRequest())
	{
		ResetEvent(m_hWinThreadsFinished);
	}
	LeaveCriticalSection(&m_tManagerCritSec);
}

CThreadCommand* CDesignerCore::PopCommand()
{
	CThreadCommand* pTop = NULL;
	EnterCriticalSection(&m_tManagerCritSec);
	if (m_cCommands.size() > 0)
	{
		pTop = m_cCommands.front();
		m_cCommands.pop();
	}
	LeaveCriticalSection(&m_tManagerCritSec);
	return pTop;
}

void CDesignerCore::StartManager()
{
	if (m_hManagerThread == NULL)
	{
		unsigned uThID;
		m_hManagerThread = (HANDLE)_beginthreadex(NULL, 0, ManagerProc, this, 0, &uThID);
	}
}

void CDesignerCore::StopManager()
{
	if (IsManagerRunning())
	{
		QueueCommand(new CThreadCommand());
		AtlWaitWithMessageLoop(m_hManagerThread);
		CloseHandle(m_hManagerThread);
		m_hManagerThread = NULL;
	}
}

bool CDesignerCore::IsManagerRunning() const
{
	return m_hManagerThread != NULL;
}


unsigned __stdcall CDesignerCore::ManagerProc(void* a_pThis)
{
	return reinterpret_cast<CDesignerCore*>(a_pThis)->m_bTabbed ?
		reinterpret_cast<CDesignerCore*>(a_pThis)->TabbedManager() :
		reinterpret_cast<CDesignerCore*>(a_pThis)->MSDIManager();
}

unsigned CDesignerCore::MSDIManager()
{
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return 0;

	DWORD nThreads = 0;
	HANDLE aWaitHandles[MAXIMUM_WAIT_OBJECTS];
	HANDLE* pThreadHandles = &aWaitHandles[1];
	aWaitHandles[0] = m_hNewOrder;

	try
	{
		InternInit();

		DWORD nTimeout = 0;
		bool bQuitRequested = false;
		do
		{
			DWORD dwRet;
			do
			{
				dwRet = ::MsgWaitForMultipleObjects(nThreads+1, aWaitHandles, FALSE, nTimeout, QS_ALLINPUT);

				if (dwRet == 0xFFFFFFFF)
				{
					return 0;
				}
				else if (dwRet == WAIT_OBJECT_0)
				{
					// new order
					CThreadCommand* pOrder = NULL;
					while (!bQuitRequested && (pOrder = PopCommand()) != NULL)
					{
						if (pOrder->IsQuitRequest())
						{
							bQuitRequested = true;
							delete pOrder;
						}
						else
						{
							if (nThreads < (MAXIMUM_WAIT_OBJECTS-1))
							{
								CComPtr<IConfig> pFrameCfg;
								m_pGlobalConfigMgr->Config(CLSID_GlobalConfigMainFrame, &pFrameCfg);
								CWindowThread* pWinTh = new CWindowThread(pOrder, GetLocaleID(m_pGlobalConfigMgr),
									m_pInternConfig, &m_tConfigMutex, m_pAppInfo,
									m_nStartPages, m_aStartPages,
									m_pPlugInCache, m_pOperationManager, m_pMenuCommandsManager, m_pViewManager, m_pIconsManager, m_pStorageManager, m_pInternStartPageMenu, pFrameCfg);
								pThreadHandles[nThreads] = pWinTh->Start();
								nThreads++;
							}
							else
							{
								delete pOrder;
								// TODO: error message
							}
						}
					}
				}
				else if (dwRet > WAIT_OBJECT_0 && dwRet < (WAIT_OBJECT_0 + nThreads + 1))
				{
					// remove the finished thread
					DWORD iThread = dwRet - WAIT_OBJECT_0 - 1;
					CloseHandle(pThreadHandles[iThread]);
					nThreads--;
					if (iThread != nThreads)
					{
						pThreadHandles[iThread] = pThreadHandles[nThreads];
					}
				}
				else if (dwRet == (WAIT_OBJECT_0 + nThreads + 1))
				{
					MSG msg;
					::GetMessage(&msg, NULL, 0, 0);
					DispatchMessage(&msg);
				}

				if (nThreads == 0)
				{
					SetEvent(m_hWinThreadsFinished);
				}
				else
				{
					ResetEvent(m_hWinThreadsFinished);
				}
			} while (dwRet != WAIT_TIMEOUT && nTimeout != INFINITE);

			nTimeout = INFINITE;
		} while (nThreads > 0 || !bQuitRequested);

		// save config to registry
		if (M_DefaultConfig() && m_pInternConfig != NULL)
		{
			CopyConfigValues(M_DefaultConfig(), m_pInternConfig);
		}
	}
	catch (...)
	{
	}

	SetEvent(m_hWinThreadsFinished);

	CoUninitialize();
	return 0;
}

unsigned CDesignerCore::TabbedManager()
{
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return 0;

	try
	{
		InternInit();

		DWORD nTimeout = 0;
		bool bQuitRequested = false;
		do
		{
			DWORD dwRet;
			do
			{
				dwRet = ::MsgWaitForMultipleObjects(1, &m_hNewOrder, FALSE, nTimeout, QS_ALLINPUT);

				if (dwRet == 0xFFFFFFFF)
				{
					return 0;
				}
				else if (dwRet == WAIT_OBJECT_0)
				{
					// new order
					CThreadCommand* pOrder = NULL;
					while (!bQuitRequested && (pOrder = PopCommand()) != NULL)
					{
						if (pOrder->IsQuitRequest())
						{
							bQuitRequested = true;
							delete pOrder;
						}
						//else
						//{
						//	CComPtr<IConfig> pFrameCfg;
						//	m_pGlobalConfigMgr->Config(CLSID_GlobalConfigMainFrame, &pFrameCfg);
						//	CTabbedWindowThread* pWinTh = new CTabbedWindowThread(pOrder, GetLocaleID(m_pGlobalConfigMgr),
						//		m_pInternConfig, &m_tConfigMutex, m_pAppInfo,
						//		m_nStartPages, m_aStartPages,
						//		m_pPlugInCache, m_pOperationManager, m_pMenuCommandsManager, m_pViewManager, m_pIconsManager, m_pStorageManager, m_pInternStartPageMenu, pFrameCfg);
						//	/*pThreadHandles[nThreads] =*/ pWinTh->Start();
						//	//nThreads++;
						//}
					}
				}
				else if (dwRet == (WAIT_OBJECT_0 + 1))
				{
					MSG msg;
					::GetMessage(&msg, NULL, 0, 0);
					DispatchMessage(&msg);
				}
			}
			while (dwRet != WAIT_TIMEOUT && nTimeout != INFINITE);

			nTimeout = INFINITE;
		} while (!bQuitRequested);

		// save config to registry
		if (M_DefaultConfig() && m_pInternConfig != NULL)
		{
			CopyConfigValues(M_DefaultConfig(), m_pInternConfig);
		}
	}
	catch (...)
	{
	}

	SetEvent(m_hWinThreadsFinished);

	CoUninitialize();
	return 0;
}

void CDesignerCore::InternInit()
{
	// prepare plug-ins
	RWCoCreateInstance(m_pPlugInCache, __uuidof(PlugInCache));

	CGlobalConfigMainFrame::InitStartPages(m_nStartPages, m_aStartPages, m_tDefaultStartPage);
	RWCoCreateInstance(m_pGlobalConfigMgr, __uuidof(GlobalConfigManager));

	RWCoCreateInstance(m_pViewManager, __uuidof(ViewManager));

	RWCoCreateInstance(m_pOperationManager, __uuidof(OperationManager));

	RWCoCreateInstance(m_pIconsManager, __uuidof(DesignerFrameIconsManager));

	RWCoCreateInstance(m_pStorageManager, __uuidof(StorageManager));

	CComObject<CDesignerFrameOperationManager>* pOperMgr;
	CComObject<CDesignerFrameOperationManager>::CreateInstance(&pOperMgr);
	CComPtr<IOperationManager> pOMTmp = pOperMgr;
	pOperMgr->Init(m_pOperationManager);

	RWCoCreateInstance(m_pMenuCmdsOps, __uuidof(MenuCommandsOperation));
	m_pMenuCmdsOps->OperationManagerSet(pOMTmp);

	RWCoCreateInstance(m_pMenuCommandsManager, __uuidof(MenuCommandsManager));

	CComObject<CDesignerFrameMenuCommandsManager>* pCmdsMgr;
	CComObject<CDesignerFrameMenuCommandsManager>::CreateInstance(&pCmdsMgr);
	CComPtr<IMenuCommandsManager> pCMTmp = pCmdsMgr;
	pCmdsMgr->Init(m_pMenuCommandsManager, pOMTmp);

	CComObject<CDesignerFrameViewManager>* pViewMgr;
	CComObject<CDesignerFrameViewManager>::CreateInstance(&pViewMgr);
	CComPtr<IViewManager> pVMTmp = pViewMgr;
	pViewMgr->Init(m_pViewManager, pCMTmp);

	// create and init global config
	CComPtr<IConfigWithDependencies> pCfgInit;
	RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));
	// add globals to the main config
	pCfgInit->ItemInsSimple(CComBSTR(L"Globals"), NULL, NULL, CConfigValue(true), CComQIPtr<ISubConfig>(m_pGlobalConfigMgr), 0, NULL); 
	//{
	//	CComPtr<IConfigWithDependencies> pGlbs;
	//	RWCoCreateInstance(pGlbs, __uuidof(ConfigWithDependencies));
	//	CConfigValue cTrue(true);
	//	CComPtr<ILocalizedString> pDummy;
	//	RWCoCreateInstance(pDummy, __uuidof(LocalizedString));
	//	CComPtr<IEnumGUIDs> pIDs;
	//	m_pGlobalConfigMgr->EnumIDs(&pIDs);
	//	ULONG nIDs = 0;
	//	if (pIDs) pIDs->Size(&nIDs);
	//	int j = 0;
	//	for (ULONG i = 0; i < nIDs; ++i)
	//	{
	//		GUID tID;
	//		pIDs->Get(i, &tID);
	//		CComPtr<IConfig> pGlbCfg;
	//		m_pGlobalConfigMgr->Config(tID, &pGlbCfg);
	//		CComQIPtr<ISubConfig> pSubCfg(pGlbCfg);
	//		if (pSubCfg == NULL)
	//		{
	//			CComPtr<ISubConfigSwitch> pSubSw;
	//			RWCoCreateInstance(pSubSw, __uuidof(SubConfigSwitch));
	//			pSubSw->ItemInsert(cTrue, pGlbCfg);
	//			pSubCfg = pSubSw;
	//		}
	//		OLECHAR szID[64];
	//		StringFromGUID(tID, szID);
	//		pGlbs->ItemInsSimple(CComBSTR(szID), pDummy, pDummy, cTrue, pSubCfg, 0, NULL); 
	//	}
	//	pGlbs->Finalize(NULL);
	//	pCfgInit->ItemInsSimple(CComBSTR(L"Globals"), pDummy, pDummy, cTrue, CComQIPtr<ISubConfig>(pGlbs), 0, NULL); 
	//}
	CMainFrame::InitConfig(pCfgInit, pVMTmp, pOMTmp, pCMTmp, m_pIconsManager, m_pStorageManager);
	if (m_nStartPages && m_aStartPages)
	{
		for (ULONG i = 0; i < m_nStartPages; ++i)
		{
			CComPtr<IStartViewPageFactory> p;
			if (!IsEqualCLSID(GUID_NULL, m_aStartPages[i]))
			RWCoCreateInstance(p, m_aStartPages[i]);
			if (p)
				p->InitConfig(pCfgInit);
		}
	}
	else
	{
		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumGUIDs> pGUIDs;
		if (pPIC) pPIC->CLSIDsEnum(CATID_StartViewPage, 0xffffffff, &pGUIDs);
		ULONG nGUIDs = 0;
		if (pGUIDs) pGUIDs->Size(&nGUIDs);
		CAutoVectorPtr<GUID> aStartPages(new GUID[nGUIDs+4]);
		aStartPages[0] = __uuidof(StartViewPageFactoryNewDocument);
		aStartPages[1] = __uuidof(StartViewPageFactoryOpenFile);
		aStartPages[2] = __uuidof(StartViewPageFactoryRecentFiles);
		pGUIDs->GetMultiple(0, nGUIDs, aStartPages.m_p+3);
		aStartPages[3+nGUIDs] = __uuidof(StartPageOnline);
		for (ULONG i = 0; i < nGUIDs+4; ++i)
		{
			CComPtr<IStartViewPageFactory> p;
			if (!IsEqualCLSID(GUID_NULL, aStartPages[i]))
			RWCoCreateInstance(p, aStartPages[i]);
			if (p)
				p->InitConfig(pCfgInit);
		}
	}
	pCfgInit->Finalize(NULL);
	m_pInternConfig = pCfgInit.p;

	// load config from registry
	if (M_DefaultConfig())
		CopyConfigValues(m_pInternConfig, M_DefaultConfig());
	SetThreadLocale(GetLocaleID(m_pGlobalConfigMgr));

	RWCoCreateInstance(m_pInternStartPageMenu, __uuidof(ConfigWithDependencies));
	CComBSTR cCFGID_MENUCOMMANDS(CFGID_MENUCOMMANDS);
	pCMTmp->InsertIntoConfigAs(pCMTmp, m_pInternStartPageMenu, cCFGID_MENUCOMMANDS, _SharedStringTable.GetStringAuto(IDS_CFGID_COMMANDS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_COMMANDS_DESC), 0, NULL);
	m_pInternStartPageMenu->Finalize(NULL);
	if (m_pStartPageMenu)
	{
		CopyConfigValues(m_pInternStartPageMenu, m_pStartPageMenu);
	}
	else
	{
		HRSRC hLayouts = FindResource(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDR_DEFAULTMENU), _T("RT_CONFIG"));
		HGLOBAL hMem = LoadResource(_pModule->get_m_hInst(), hLayouts);
		DWORD nMemSize = SizeofResource(_pModule->get_m_hInst(), hLayouts);
		CComPtr<IConfigInMemory> pMemCfg;
		RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
		pMemCfg->DataBlockSet(nMemSize, static_cast<BYTE const*>(LockResource(hMem)));
		CConfigValue cItemID(__uuidof(MenuCommandsVector));
		m_pInternStartPageMenu->ItemValuesSet(1, &(cCFGID_MENUCOMMANDS.m_str), cItemID);
		CComPtr<IConfig> pItemCfg;
		m_pInternStartPageMenu->SubConfigGet(cCFGID_MENUCOMMANDS, &pItemCfg);
		CopyConfigValues(pItemCfg, pMemCfg);
	}

	RWCoCreateInstance(m_pAutoUpdater, __uuidof(AutoUpdater));
}

IConfig* CDesignerCore::M_DefaultConfig()
{
	return m_pDefaultConfig;
}

STDMETHODIMP CDesignerCore::Size(ULONG* a_pnSize)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDesignerCore::Get(ULONG a_nIndex, REFIID a_iid, void** a_ppItem)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDesignerCore::GetMultiple(ULONG a_nIndexFirst, ULONG a_nCount, REFIID a_iid, void** a_apItems)
{
	return E_NOTIMPL;
}


