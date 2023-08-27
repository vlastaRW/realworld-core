// StorageFilterFactoryResource.cpp : Implementation of CStorageFilterFactoryResource

#include "stdafx.h"
#include "StorageFilterFactoryResource.h"

#include "StorageFilterResource.h"
#include "StorageFilterWindowResource.h"
#include <MultiLanguageString.h>


// CStorageFilterFactoryResource

STDMETHODIMP CStorageFilterFactoryResource::NameGet(ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Resources[0405]Resource");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFactoryResource::IconGet(ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		return E_NOTIMPL;
		//*a_phIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_PRINTER), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
		//return (*a_phIcon) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFactoryResource::SupportsGUI(DWORD UNREF(a_dwFlags))
{
	return S_FALSE;
}

STDMETHODIMP CStorageFilterFactoryResource::FilterCreate(BSTR a_bstrFilter, DWORD a_dwFlags, IStorageFilter** a_ppFilter)
{
	try
	{
		*a_ppFilter = NULL;
		CHECKPOINTER(a_bstrFilter);

		USES_CONVERSION;
		LPTSTR pszTmp = OLE2T(a_bstrFilter);
		if (_tcsnicmp(pszTmp, _T("res://"), 6) == 0)
		{
			pszTmp += 6;
		}
		else
		{
			return E_FAIL; // TODO: error code
		}
		CComObject<CStorageFilterResource>* pObj = NULL;
		CComObject<CStorageFilterResource>::CreateInstance(&pObj);
		CComPtr<IStorageFilter> pTmp = pObj;
		pObj->Init(pszTmp);

		*a_ppFilter= pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFilter == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFactoryResource::WindowCreate(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilterWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;

		USES_CONVERSION;
		CComObject<CStorageFilterWindowResource>* pWnd = NULL;
		CComObject<CStorageFilterWindowResource>::CreateInstance(&pWnd);
		CComPtr<IStorageFilterWindow> pTmp = pWnd;
		HRESULT hRes = pWnd->Init(a_bstrInitial, reinterpret_cast<HWND>(a_hParent), a_tLocaleID);
		if (FAILED(hRes)) return hRes;

		*a_ppWindow = pTmp.Detach();

		return hRes;
	}
	catch (...)
	{
		return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterFactoryResource::ContextConfigGetDefault(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->Finalize(NULL);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

