// ConfigValueHelper.cpp : Implementation of CConfigValueHelper

#include "stdafx.h"
#include "ConfigValueHelper.h"


// CConfigValueHelper

STDMETHODIMP CConfigValueHelper::Init(TConfigValue* a_pVal)
{
	ConfigValueInit(*a_pVal);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::Copy(TConfigValue* a_pIn, TConfigValue* a_pOut)
{
	*a_pOut = ConfigValueCopy(*a_pIn);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::Destroy(TConfigValue* a_pVal)
{
	ConfigValueClear(*a_pVal);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::SetInt(TConfigValue* a_pVal, int a_nVal)
{
	ConfigValueClear(*a_pVal);
	*a_pVal = CConfigValue(LONG(a_nVal));
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::SetFloat(TConfigValue* a_pVal, float a_fVal)
{
	ConfigValueClear(*a_pVal);
	*a_pVal = CConfigValue(a_fVal);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::SetBool(TConfigValue* a_pVal, VARIANT_BOOL a_bVal)
{
	ConfigValueClear(*a_pVal);
	*a_pVal = CConfigValue(a_bVal != VARIANT_FALSE);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::SetString(TConfigValue* a_pVal, BSTR a_bstrVal)
{
	ConfigValueClear(*a_pVal);
	*a_pVal = CConfigValue(a_bstrVal);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::SetGUID(TConfigValue* a_pVal, GUID a_guidVal)
{
	ConfigValueClear(*a_pVal);
	*a_pVal = CConfigValue(a_guidVal);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::SetVector2(TConfigValue* a_pVal, float a_fVal1, float a_fVal2)
{
	ConfigValueClear(*a_pVal);
	*a_pVal = CConfigValue(a_fVal1, a_fVal2);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::SetVector3(TConfigValue* a_pVal, float a_fVal1, float a_fVal2, float a_fVal3)
{
	ConfigValueClear(*a_pVal);
	*a_pVal = CConfigValue(a_fVal1, a_fVal2, a_fVal3);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::SetVector4(TConfigValue* a_pVal, float a_fVal1, float a_fVal2, float a_fVal3, float a_fVal4)
{
	ConfigValueClear(*a_pVal);
	*a_pVal = CConfigValue(a_fVal1, a_fVal2, a_fVal3, a_fVal4);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::SetColor(TConfigValue* a_pVal, float a_fValR, float a_fValG, float a_fValB)
{
	ConfigValueClear(*a_pVal);
	*a_pVal = CConfigValue(a_fValR, a_fValG, a_fValB, true);
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::GetInt(TConfigValue* a_pVal, int* a_nVal)
{
	if (a_pVal->eTypeID != ECVTInteger)
		return E_RW_INVALIDPARAM;
	*a_nVal = a_pVal->iVal;
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::GetFloat(TConfigValue* a_pVal, float* a_fVal)
{
	if (a_pVal->eTypeID != ECVTFloat)
		return E_RW_INVALIDPARAM;
	*a_fVal = a_pVal->fVal;
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::GetBool(TConfigValue* a_pVal, VARIANT_BOOL* a_bVal)
{
	if (a_pVal->eTypeID != ECVTBool)
		return E_RW_INVALIDPARAM;
	*a_bVal = a_pVal->bVal ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::GetString(TConfigValue* a_pVal, BSTR* a_bstrVal)
{
	if (a_pVal->eTypeID != ECVTString)
		return E_RW_INVALIDPARAM;
	*a_bstrVal = CComBSTR(a_pVal->bstrVal).Detach();
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::GetGUID(TConfigValue* a_pVal, GUID* a_guidVal)
{
	if (a_pVal->eTypeID != ECVTGUID)
		return E_RW_INVALIDPARAM;
	*a_guidVal = a_pVal->guidVal;
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::GetVector2(TConfigValue* a_pVal, float* a_fVal1, float* a_fVal2)
{
	if (a_pVal->eTypeID != ECVTVector2)
		return E_RW_INVALIDPARAM;
	*a_fVal1 = a_pVal->vecVal[0];
	*a_fVal2 = a_pVal->vecVal[1];
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::GetVector3(TConfigValue* a_pVal, float* a_fVal1, float* a_fVal2, float* a_fVal3)
{
	if (a_pVal->eTypeID != ECVTVector3)
		return E_RW_INVALIDPARAM;
	*a_fVal1 = a_pVal->vecVal[0];
	*a_fVal2 = a_pVal->vecVal[1];
	*a_fVal3 = a_pVal->vecVal[2];
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::GetVector4(TConfigValue* a_pVal, float* a_fVal1, float* a_fVal2, float* a_fVal3, float* a_fVal4)
{
	if (a_pVal->eTypeID != ECVTVector4)
		return E_RW_INVALIDPARAM;
	*a_fVal1 = a_pVal->vecVal[0];
	*a_fVal2 = a_pVal->vecVal[1];
	*a_fVal3 = a_pVal->vecVal[2];
	*a_fVal4 = a_pVal->vecVal[3];
	return S_OK;
}

STDMETHODIMP CConfigValueHelper::GetColor(TConfigValue* a_pVal, float* a_fValR, float* a_fValG, float* a_fValB)
{
	if (a_pVal->eTypeID != ECVTFloatColor)
		return E_RW_INVALIDPARAM;
	*a_fValR = a_pVal->vecVal[0];
	*a_fValG = a_pVal->vecVal[1];
	*a_fValB = a_pVal->vecVal[2];
	return S_OK;
}

