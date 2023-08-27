// MouseGesturesHelper.h : Declaration of the CMouseGesturesHelper

#pragma once
#include "resource.h"       // main symbols

#include "RWProcessing.h"
#include "MLNet.h"
#include <WeakSingleton.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CMouseGesturesHelper

class ATL_NO_VTABLE CMouseGesturesHelper :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMouseGesturesHelper, &CLSID_MouseGesturesHelper>,
	public IMouseGesturesHelper
{
public:
	CMouseGesturesHelper()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMouseGesturesHelper)


BEGIN_COM_MAP(CMouseGesturesHelper)
	COM_INTERFACE_ENTRY(IMouseGesturesHelper)
END_COM_MAP()


public:
	STDMETHOD(InitConfig)(IOperationManager* a_pOpMgr, IConfigWithDependencies* a_pMainCfg);
	STDMETHOD(Configure)(RWHWND a_hParent, LCID a_tLCID, IConfig* a_pMainCfg);
	STDMETHOD(Recognize)(ULONG a_nPoints, POINT const* a_pPoints, IConfig* a_pMainCfg, TConfigValue* a_pOpID, IConfig** a_ppOpCfg);

private:
	CAutoPtr<MLNet> m_pNet;
};

OBJECT_ENTRY_AUTO(__uuidof(MouseGesturesHelper), CMouseGesturesHelper)
