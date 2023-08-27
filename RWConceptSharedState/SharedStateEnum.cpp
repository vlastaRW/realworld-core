// SharedStateEnum.cpp : Implementation of CSharedStateEnum

#include "stdafx.h"
#include "SharedStateEnum.h"


// CSharedStateEnum

STDMETHODIMP CSharedStateEnum::CLSIDGet(CLSID* a_pCLSID)
{
	try
	{
		*a_pCLSID = GetObjectCLSID();
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSharedStateEnum::ToText(BSTR* a_pbstrText)
{
	try
	{
		*a_pbstrText = SysAllocString(L"Composed state");
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}


STDMETHODIMP CSharedStateEnum::FromText(BSTR a_bstrText)
{
	return E_NOTIMPL;
}

