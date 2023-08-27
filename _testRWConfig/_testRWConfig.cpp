// RWRendererSetup.cpp : main source file for RWRendererSetup.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>


#include "resource.h"

// Note: Proxy/Stub Information
//		To build a separate proxy/stub DLL, 
//		run nmake -f RWRendererSetupps.mk in the project directory.
#include "initguid.h"

#include "ConfigControlTestDlg.h"

#include <CommCtrlDependency.h>


CServerAppModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

#include <RWCOMImpl.inl>


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));
#if (_WIN32_IE >= 0x0300)
	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof(iccx);
	iccx.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
	BOOL bRet = ::InitCommonControlsEx(&iccx);
	bRet;
	ATLASSERT(bRet);
#else
	::InitCommonControls();
#endif

	{
		TCHAR szMaskApp[MAX_PATH];
		GetModuleFileName(NULL, szMaskApp, MAX_PATH);
		_tcscpy(_tcsrchr(szMaskApp, _T('\\')), _T("\\RW*.dll"));
		OLECHAR const* aSearchMasks[] = {szMaskApp, NULL};
		InitializePlugIns(aSearchMasks);
	}

	int nRet;
	{
		hRes = _Module.Init(ObjectMap, hInstance);
		AtlAxWinInit();

		CConfigControlTestDlg dlgMain;
		nRet = dlgMain.DoModal();

		AtlAxWinTerm();

		_Module.Term();
	}
	::CoUninitialize();

	return nRet;
}
