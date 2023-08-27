// ConfigValueHelper.h : Declaration of the CConfigValueHelper

#pragma once
#include "resource.h"       // main symbols

#include "RWConfig.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CConfigValueHelper

class ATL_NO_VTABLE CConfigValueHelper :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CConfigValueHelper, &CLSID_ConfigValueHelper>,
	public IConfigValueHelper
{
public:
	CConfigValueHelper()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CConfigValueHelper)

BEGIN_COM_MAP(CConfigValueHelper)
	COM_INTERFACE_ENTRY(IConfigValueHelper)
END_COM_MAP()


	// IConfigValueHelper methods
public:
	STDMETHOD(Init)(TConfigValue* a_pVal);
	STDMETHOD(Copy)(TConfigValue* a_pIn, TConfigValue* a_pOut);
	STDMETHOD(Destroy)(TConfigValue* a_pVal);

	STDMETHOD(SetInt)(TConfigValue* a_pVal, int a_nVal);
	STDMETHOD(SetFloat)(TConfigValue* a_pVal, float a_fVal);
	STDMETHOD(SetBool)(TConfigValue* a_pVal, VARIANT_BOOL a_bVal);
	STDMETHOD(SetString)(TConfigValue* a_pVal, BSTR a_bstrVal);
	STDMETHOD(SetGUID)(TConfigValue* a_pVal, GUID a_guidVal);
	STDMETHOD(SetVector2)(TConfigValue* a_pVal, float a_fVal1, float a_fVal2);
	STDMETHOD(SetVector3)(TConfigValue* a_pVal, float a_fVal1, float a_fVal2, float a_fVal3);
	STDMETHOD(SetVector4)(TConfigValue* a_pVal, float a_fVal1, float a_fVal2, float a_fVal3, float a_fVal4);
	STDMETHOD(SetColor)(TConfigValue* a_pVal, float a_fValR, float a_fValG, float a_fValB);

	STDMETHOD(GetInt)(TConfigValue* a_pVal, int* a_nVal);
	STDMETHOD(GetFloat)(TConfigValue* a_pVal, float* a_fVal);
	STDMETHOD(GetBool)(TConfigValue* a_pVal, VARIANT_BOOL* a_bVal);
	STDMETHOD(GetString)(TConfigValue* a_pVal, BSTR* a_bstrVal);
	STDMETHOD(GetGUID)(TConfigValue* a_pVal, GUID* a_guidVal);
	STDMETHOD(GetVector2)(TConfigValue* a_pVal, float* a_fVal1, float* a_fVal2);
	STDMETHOD(GetVector3)(TConfigValue* a_pVal, float* a_fVal1, float* a_fVal2, float* a_fVal3);
	STDMETHOD(GetVector4)(TConfigValue* a_pVal, float* a_fVal1, float* a_fVal2, float* a_fVal3, float* a_fVal4);
	STDMETHOD(GetColor)(TConfigValue* a_pVal, float* a_fValR, float* a_fValG, float* a_fValB);

};

OBJECT_ENTRY_AUTO(__uuidof(ConfigValueHelper), CConfigValueHelper)
