// DesignerCore.h : Declaration of the CDesignerCore

#pragma once
#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include "RWProcessing.h"
#include "ThreadCommand.h"
#include <WeakSingleton.h>


// CDesignerCore

class ATL_NO_VTABLE CDesignerCore : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerCore, &CLSID_DesignerCore>,
	public IDesignerCore,
	public IEnumUnknowns
{
public:
	CDesignerCore() : m_hManagerThread(NULL),
#ifdef _DEBUG
		m_bInitialized(FALSE),
#endif// _DEBUG
		m_hWinThreadsFinished(CreateEvent(NULL, TRUE, TRUE, NULL)),
		m_hNewOrder(CreateEvent(NULL, FALSE, FALSE, NULL)),
		m_nStartPages(0), m_tDefaultStartPage(GUID_NULL), m_bTabbed(false)
	{
		InitializeCriticalSection(&m_tManagerCritSec);
		InitializeCriticalSection(&m_tConfigMutex);
	}
	~CDesignerCore()
	{
		StopManager();
		DeleteCriticalSection(&m_tManagerCritSec);
		if (m_hNewOrder)
		{
			CloseHandle(m_hNewOrder);
		}
		if (m_hWinThreadsFinished)
		{
			CloseHandle(m_hWinThreadsFinished);
		}
		while (m_cCommands.size() > 0)
		{
			delete m_cCommands.front();
			m_cCommands.pop();
		}
		DeleteCriticalSection(&m_tConfigMutex);
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CDesignerCore)


BEGIN_COM_MAP(CDesignerCore)
	COM_INTERFACE_ENTRY(IDesignerCore)
	COM_INTERFACE_ENTRY(IEnumUnknowns)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDesignerCore methods
public:
	STDMETHOD(Initialize)(IConfig* a_pConfig, IApplicationInfo* a_pAppInfo, ULONG a_nStartPages, CLSID const* a_aStartPages, REFCLSID a_tDefaultStartPage, IConfig* a_pStartPageMenu, EMDIType a_eMDIType);

	STDMETHOD(Name)(ILocalizedString** a_ppName)
	{
		return m_pAppInfo ? m_pAppInfo->Name(a_ppName) : E_UNEXPECTED;
	}
	STDMETHOD(Version)(ULONG* a_pVersion)
	{
		return m_pAppInfo ? m_pAppInfo->Version(a_pVersion) : E_UNEXPECTED;
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		return m_pAppInfo ? m_pAppInfo->Icon(a_nSize, a_phIcon) : E_UNEXPECTED;
	}

	STDMETHOD(NewWindowPath)(BSTR a_bstrFullPath, RWHWND* a_pCreatedWindow);
	STDMETHOD(NewWindowDocument)(IDocument* a_pDocument, RWHWND* a_pCreatedWindow);
	STDMETHOD(NewWindowPage)(REFCLSID a_tStartPageID, RWHWND* a_pCreatedWindow);

	STDMETHOD(Documents)(IEnumUnknowns** a_ppOpenDocuments)
	{
		try
		{
			*a_ppOpenDocuments = this;
			AddRef();
			return S_OK;
		}
		catch (...)
		{
			return a_ppOpenDocuments ? E_UNEXPECTED : E_POINTER;
		}
	}

	STDMETHOD(MessagePump)(ULONG a_nStopEvents, HANDLE* a_pStopEvents, ULONG* a_pWaitResult);
	//STDMETHOD(Finish)(ULONG a_nMaxWaitTime);
	//STDMETHOD(GetFinishedWaitHandle)(HANDLE* a_phEvent);

	// IEnumUnknowns methods (open documents)
public:
	STDMETHOD(Size)(ULONG* a_pnSize);
	STDMETHOD(Get)(ULONG a_nIndex, REFIID a_iid, void** a_ppItem);
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, REFIID a_iid, void** a_apItems);

private:
	void InternInit();

	static unsigned __stdcall ManagerProc(void* a_pThis);
	unsigned MSDIManager();
	unsigned TabbedManager();
	void StartManager();
	void StopManager();
	bool IsManagerRunning() const;

	void QueueCommand(CThreadCommand* a_pCommand);
	CThreadCommand* PopCommand();

	IConfig* M_DefaultConfig();

private:
	// thread manager data
	HANDLE m_hManagerThread;
	CRITICAL_SECTION m_tManagerCritSec;
	HANDLE m_hNewOrder;
	HANDLE m_hWinThreadsFinished;
	std::queue<CThreadCommand*> m_cCommands;

	// app defaults
	CComPtr<IConfig> m_pDefaultConfig;
	CComPtr<IApplicationInfo> m_pAppInfo;
	ULONG m_nStartPages;
	CAutoVectorPtr<CLSID> m_aStartPages;
	CLSID m_tDefaultStartPage;
	CComPtr<IConfig> m_pStartPageMenu;
	bool m_bTabbed;
	CRITICAL_SECTION m_tConfigMutex;
#ifdef _DEBUG
	BOOL m_bInitialized;
#endif// _DEBUG

	// application data (2nd thread) // TODO: move it elsewhere
	CComPtr<IConfig> m_pInternConfig;
	CComPtr<IPlugInCache> m_pPlugInCache;
	CComPtr<IOperationManager> m_pOperationManager;
	CComPtr<IMenuCommandsManager> m_pMenuCommandsManager;
	CComPtr<IMenuCommandsOperation> m_pMenuCmdsOps;
	CComPtr<IViewManager> m_pViewManager;
	CComPtr<IDesignerFrameIcons> m_pIconsManager;
	CComPtr<IStorageManager> m_pStorageManager;
	CComPtr<IConfigWithDependencies> m_pInternStartPageMenu;
	CComPtr<IGlobalConfigManager> m_pGlobalConfigMgr;

	CComPtr<IAutoUpdater> m_pAutoUpdater;

	// not used, but kept alive for others to prevent creation and destruction
	CComPtr<IThreadPool> m_pThreadPool;
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerCore), CDesignerCore)
