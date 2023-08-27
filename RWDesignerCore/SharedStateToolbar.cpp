// SharedStateToolbar.cpp : Implementation of CSharedStateToolbar

#include "stdafx.h"
#include "SharedStateToolbar.h"


// CSharedStateToolbar

STDMETHODIMP CSharedStateToolbar::CLSIDGet(CLSID* a_pCLSID)
{
	try
	{
		*a_pCLSID = CLSID_SharedStateToolbar;
		return S_OK;
	}
	catch (...)
	{
		return a_pCLSID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CSharedStateToolbar::ToText(BSTR* a_pbstrText)
{
	try
	{
		*a_pbstrText = NULL;
		*a_pbstrText = SysAllocString(m_bVisible ? L"show" : L"hide");
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CSharedStateToolbar::FromText(BSTR a_bstrText)
{
	try
	{
		m_bVisible = a_bstrText == NULL || wcscmp(a_bstrText, L"hide");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateToolbar::IsVisible()
{
	return m_bVisible ? S_OK : S_FALSE;
}

STDMETHODIMP CSharedStateToolbar::SetVisible(BOOL a_bVisible)
{
	m_bVisible = a_bVisible;
	return S_OK;
}

