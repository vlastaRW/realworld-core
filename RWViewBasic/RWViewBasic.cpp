// RWViewBasic.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include <CommCtrlDependency.h>

#pragma comment(lib, "imm32.lib")

class CRWViewBasicModule : public CAtlDllModuleT< CRWViewBasicModule >
{
    virtual HRESULT GetGITPtr(IGlobalInterfaceTable** ppGIT) throw()
    {
        return E_NOTIMPL;
    }
};

CRWViewBasicModule _AtlModule;


// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}


// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


#include <RWEnumClasses.inl>
