
#pragma once

#include "RWDesignerCore.h"
#include "RWProcessing.h"
#include "ThreadCommand.h"


class CTabbedWindowThread : public CMessageLoop
{
public:
	struct CConfigLock
	{
		CConfigLock(CTabbedWindowThread* a_p) : m_p(a_p) { m_p->LockConfig(); }
		~CConfigLock() { m_p->UnlockConfig(); }
	private:
		CTabbedWindowThread* m_p;
	};

	CTabbedWindowThread(CThreadCommand* a_pOrder, LCID a_tLocaleID,
			IConfig* a_pDefConfig, CRITICAL_SECTION* a_pConfigMutex, IApplicationInfo* a_pAppInfo, ULONG a_nStartPages, CLSID* a_aStartPages, 
			IPlugInCache* a_pPlugInCache, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IViewManager* a_pViewManager, IDesignerFrameIcons* a_pIcons, IStorageManager* a_pStorageManager, IConfig* a_pMenuConfig, IConfig* a_pFrameConfig) :
		m_pOrder(a_pOrder), m_tLocaleID(a_tLocaleID), m_nStartPages(a_nStartPages), m_pConfigMutex(a_pConfigMutex),
		m_pDefConfig(a_pDefConfig), m_pAppInfo(a_pAppInfo),
		m_pPlugInCache(a_pPlugInCache), m_pOperationManager(a_pOperationManager), m_pMenuCommandsManager(a_pMenuCommandsManager),
		m_pViewManager(a_pViewManager), m_pIcons(a_pIcons), m_pStorageManager(a_pStorageManager), m_pMenuConfig(a_pMenuConfig), m_pFrameConfig(a_pFrameConfig)
	{
		m_aStartPages.Attach(new CLSID[m_nStartPages]);
		CopyMemory(m_aStartPages.m_p, a_aStartPages, m_nStartPages*sizeof(CLSID));
	}
	~CTabbedWindowThread()
	{
		delete m_pOrder;
	}

	void SetLocaleID(LCID a_tLocaleID)
	{
		SetThreadLocale(a_tLocaleID);
		m_tLocaleID = a_tLocaleID;
	}

	HANDLE Start()
	{
		unsigned uThID;
		return (HANDLE)_beginthreadex(NULL, 0, MessageLoopProc, this, 0, &uThID);
	}
	void CreationFinished()
	{
		delete m_pOrder;
		m_pOrder = NULL;
	}

	IConfig* M_Config() const {return m_pDefConfig;}
	IConfig* M_FrameConfig() const {return m_pFrameConfig;}
	void LockConfig() {EnterCriticalSection(m_pConfigMutex);}
	void UnlockConfig() {LeaveCriticalSection(m_pConfigMutex);}
	IApplicationInfo* M_DesignerAppInfo() const {return m_pAppInfo;}
	LCID GetLocaleID() const { return m_tLocaleID; }
	ULONG M_StartPageCount() const { return m_nStartPages; }
	CLSID const* M_StartPages() const { return m_aStartPages.m_p; }

	IOperationManager* M_OperMgr() const {return m_pOperationManager;}
	IMenuCommandsManager* M_MenuCmdsMgr() const {return m_pMenuCommandsManager;}
	IViewManager* M_ViewMgr() const {return m_pViewManager;}
	IPlugInCache* M_PlugInCache() const {return m_pPlugInCache;}
	IDesignerFrameIcons* M_DesignerFrameIcons() const {return m_pIcons;}
	IStorageManager* M_StorageManager() const {return m_pStorageManager;}
	IConfig* M_MenuConfig() const {return m_pMenuConfig;}

private:
	static unsigned __stdcall MessageLoopProc(void* a_pThis)
	{
		return reinterpret_cast<CTabbedWindowThread*>(a_pThis)->MessageLoop();
	}
	unsigned MessageLoop();

private:
	CThreadCommand* m_pOrder;

	CComPtr<IConfig> m_pDefConfig;
	CComPtr<IApplicationInfo> m_pAppInfo;
	ULONG m_nStartPages;
	CAutoVectorPtr<CLSID> m_aStartPages;

	CComPtr<IPlugInCache> m_pPlugInCache;
	CComPtr<IOperationManager> m_pOperationManager;
	CComPtr<IMenuCommandsManager> m_pMenuCommandsManager;
	CComPtr<IViewManager> m_pViewManager;
	CComPtr<IDesignerFrameIcons> m_pIcons;
	CComPtr<IStorageManager> m_pStorageManager;
	CComPtr<IConfig> m_pMenuConfig;
	CComPtr<IConfig> m_pFrameConfig;
	LCID m_tLocaleID;

	CRITICAL_SECTION* m_pConfigMutex;
};

