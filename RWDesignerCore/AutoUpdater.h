// AutoUpdater.h : Declaration of the CAutoUpdater

#pragma once
#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include <WeakSingleton.h>
#include <atltime.h>
#include "ConfigIDsApp.h"
#include <ObserverImpl.h>


// CAutoUpdater

class ATL_NO_VTABLE CAutoUpdater : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAutoUpdater, &CLSID_AutoUpdater>,
	public IAutoUpdater,
	public CObserverImpl<CAutoUpdater, IConfigObserver, IUnknown*>
{
public:
	CAutoUpdater() : m_hThread(0), m_uThreadID(0), m_eUpdateStatus(EUSChecking)
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CAutoUpdater)

BEGIN_COM_MAP(CAutoUpdater)
	COM_INTERFACE_ENTRY(IAutoUpdater)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		CComPtr<IGlobalConfigManager> pGCM;
		RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
		pGCM->Config(__uuidof(GlobalConfigMainFrame), &m_pConfig);
		m_hChange = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_eCommand = ETCStart;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, AutoUpdateProc, this, 0, &m_uThreadID);
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_hThread)
		{
			m_eCommand = ETCExit;
			SetEvent(m_hChange);
			WaitForSingleObject(m_hThread, 60000);
			CloseHandle(m_hThread);
			CloseHandle(m_hChange);
		}
	}

	void OwnerNotify(TCookie, IUnknown*)
	{
		if (UpdatesEnabled())
			SetEvent(m_hChange);
	}

	bool UpdatesEnabled() const
	{
		CConfigValue cVal;
		if (m_pConfig) m_pConfig->ItemValueGet(CComBSTR(CFGID_AUTOUPDATE), &cVal);
		return cVal.TypeGet() == ECVTInteger && cVal.operator LONG();
	}
	bool PeriodEllapsed() const;
	int DownloadFile(LPCTSTR a_pszAgent, LPCTSTR a_pszServer, LPCTSTR a_pszPath, CAutoVectorPtr<BYTE>& szResponse);

	// IAutoUpdater methods
public:
	STDMETHOD(CheckNow)()
	{
		if (m_eCommand == ETCExit)
			return E_FAIL;
		m_eCommand = ETCCheckNow;
		SetEvent(m_hChange);
		return S_OK;
	}

private:
	enum EUpdateStatus
	{
		EUSWaiting,
		EUSChecking,
		EUSUpdating,
		EUSFinished,
	};
	enum EThreadCommand
	{
		ETCStart = 0,
		ETCCheckNow,
		ETCExit
	};

private:
	static unsigned __stdcall AutoUpdateProc(void* a_pThis);
	unsigned AutoUpdate();

private:
	CComPtr<IConfig> m_pConfig;
	unsigned m_uThreadID;
	HANDLE m_hThread;
	HANDLE m_hChange;
	EThreadCommand m_eCommand;
	EUpdateStatus m_eUpdateStatus;
};

OBJECT_ENTRY_AUTO(__uuidof(AutoUpdater), CAutoUpdater)
