// StorageFilterWindowResource.cpp : Implementation of CStorageFilterWindowResource

#include "stdafx.h"
#include "StorageFilterWindowResource.h"

#include "StorageFilterResource.h"

// CStorageFilterWindowResource

HRESULT CStorageFilterWindowResource::Init(LPCOLESTR a_pszInitial, HWND a_hWnd, LCID a_tLocaleID)
{
	m_tLocaleID = a_tLocaleID;
	Create(a_hWnd);

	return a_pszInitial == NULL || _wcsnicmp(L"res://", a_pszInitial, 6) != 0 ? S_FALSE : S_OK;
}

STDMETHODIMP CStorageFilterWindowResource::FilterCreate(IStorageFilter** a_ppFilter)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterWindowResource::FiltersCreate(IEnumUnknowns** a_ppFilters)
{
	try
	{
		*a_ppFilters = NULL;
		CComPtr<IStorageFilter> pFlt;
		HRESULT hRes = FilterCreate(&pFlt);
		if (pFlt == NULL)
			return hRes;
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		p->Insert(pFlt);
		*a_ppFilters = p.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFilters ? E_UNEXPECTED : E_NOTIMPL;
	}
}

LRESULT CStorageFilterWindowResource::OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	AddRef();

	DlgResize_Init(false, false, 0);

	return TRUE;
}

void CStorageFilterWindowResource::OnFinalMessage(HWND a_hWnd)
{
	Release();
}

LRESULT CStorageFilterWindowResource::OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	return 0;
}

LRESULT CStorageFilterWindowResource::OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	return 0;
}

