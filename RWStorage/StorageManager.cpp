// StorageManager.cpp : Implementation of CStorageManager

#include "stdafx.h"
#include "StorageManager.h"
#include "OpenFilterDlg.h"
#include "OpenFilterFrameDlg.h"
#include <StringParsing.h>

#include "ConfigIDsStorage.h"


// CStorageManager


struct filtercreate
{
	HRESULT& res;
	BSTR location;
	DWORD flags;
	IStorageFilter** output;
	filtercreate(HRESULT& res, BSTR location, DWORD flags, IStorageFilter** output) :
	res(res), location(location), flags(flags), output(output) {}

	void operator()(CLSID const* begin, CLSID const* const end) const
	{
		while (begin != end && !SUCCEEDED(res))
		{
			CComPtr<IStorageFilterFactory> p;
			RWCoCreateInstance(p, *begin);
			++begin;
			if (p == NULL) continue;
			res = p->FilterCreate(location, flags, output);
		}
	}
};

STDMETHODIMP CStorageManager::FilterCreateEx(IStorageFilter* a_pRoot, BSTR a_bstrLocation, DWORD a_dwFlags, IStorageFilter** a_ppFilter)
{
	if (a_ppFilter == NULL)
		return E_POINTER;

	HRESULT hRes = E_RW_ITEMNOTFOUND;
	CPlugInEnumerator::EnumCategoryCLSIDs(CATID_StorageFilterFactory, filtercreate(hRes, a_bstrLocation, a_dwFlags, a_ppFilter));
	//CPlugInEnumerator::EnumCategoryCLSIDs(CATID_StorageFilterFactory,
	//	[=,&hRes](CLSID const* begin, CLSID const* const end)
	//	{
	//		while (begin != end && !SUCCEEDED(res))
	//		{
	//			CComPtr<IStorageFilterFactory> p;
	//			RWCoCreateInstance(p, *begin);
	//			++begin;
	//			if (p == NULL) continue;
	//			hRes = p->FilterCreate(a_bstrLocation, a_dwFlags, a_ppFilter);
	//		}
	//	}
	if (SUCCEEDED(hRes))
		return hRes;

	if (a_pRoot)
		return a_pRoot->SubFilterGet(a_bstrLocation, a_ppFilter);

	return hRes;
}

STDMETHODIMP CStorageManager::FilterCreateInteractivelyCfgHelp(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, IConfig* a_pContextConfig, ILocalizedString* a_pCaption, BSTR a_bstrHelpLink, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilter** a_ppFilter)
{
	try
	{
		CComObject<COpenFilterFrameDlg>* pwndDlg = NULL;
		CComObject<COpenFilterFrameDlg>::CreateInstance(&pwndDlg);
		CComPtr<IUnknown> pTmp = pwndDlg;
		*a_ppFilter = pwndDlg->DoModal(a_hParent, a_bstrInitial, a_dwFlags, a_pFormatFilters, a_pUserConfig, a_pContextConfig, a_pCaption, a_bstrHelpLink, a_pListener, a_tLocaleID);
		return *a_ppFilter == NULL ? E_FAIL : S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageManager::FilterCreateInteractivelyCfg(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, IConfig* a_pContextConfig, ILocalizedString* a_pCaption, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilter** a_ppFilter)
{
	return FilterCreateInteractivelyCfgHelp(a_bstrInitial, a_dwFlags, a_hParent, a_pFormatFilters, a_pUserConfig, a_pContextConfig, a_pCaption, NULL, a_pListener, a_tLocaleID, a_ppFilter);
}

STDMETHODIMP CStorageManager::FilterCreateInteractivelyUID(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, REFGUID a_tContextID, ILocalizedString* a_pCaption, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilter** a_ppFilter)
{
	try
	{
		*a_ppFilter = NULL;

		CComPtr<IConfig> pCfg;
		ConfigGetDefault(&pCfg);

		OLECHAR szGUID[48] = L"";
		StringFromGUID(a_tContextID, szGUID);
		CComPtr<IConfig> pSrcCfg;
		m_pGlobalConfig->SubConfigGet(CComBSTR(szGUID), &pSrcCfg);
		CopyConfigValues(pCfg, pSrcCfg);

		HRESULT hRes = FilterCreateInteractivelyCfgHelp(a_bstrInitial, a_dwFlags, a_hParent, a_pFormatFilters, a_pUserConfig, pCfg, a_pCaption, NULL, a_pListener, a_tLocaleID, a_ppFilter);

		CopyConfigValues(pSrcCfg, pCfg);

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageManager::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = m_pGlobalConfig;
		if (*a_ppConfig) (*a_ppConfig)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageManager::FilterWindowCreate(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilterWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;

		CComObject<COpenFilterDlg>* pwndDlg = NULL;
		CComObject<COpenFilterDlg>::CreateInstance(&pwndDlg);
		CComPtr<IStorageFilterWindow> pTmp = pwndDlg;
		pwndDlg->Create(a_hParent, a_bstrInitial, a_dwFlags, a_pFormatFilters, a_pUserConfig, a_pContextConfig, a_pCallback, a_pListener, a_tLocaleID);
		*a_ppWindow = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

struct initconfig
{
	IConfigWithDependencies* pCfg;
	TConfigValue const& cTrue;
	ILocalizedStringInit* pDummy;
	initconfig(IConfigWithDependencies* pCfg, TConfigValue const& cTrue, ILocalizedStringInit* pDummy) :
	pCfg(pCfg), cTrue(cTrue), pDummy(pDummy) {}

	void operator()(CLSID const* begin, CLSID const* const end) const
	{
		while (begin != end)
		{
			CComPtr<IStorageFilterFactory> p;
			RWCoCreateInstance(p, *begin);
			++begin;
			if (p == NULL) continue;
			CComPtr<IConfig> pPlugInCfg;
			p->ContextConfigGetDefault(&pPlugInCfg);
			CComPtr<ISubConfigSwitch> pSubCfg;
			RWCoCreateInstance(pSubCfg, __uuidof(SubConfigSwitch));
			pSubCfg->ItemInsert(&cTrue, pPlugInCfg);
			TCHAR szCLSID[64];
			StringFromGUID(begin[-1], szCLSID);
			pCfg->ItemInsSimple(CComBSTR(szCLSID), pDummy, pDummy, &cTrue, pSubCfg, 0, NULL);
		}
	}
};

STDMETHODIMP CStorageManager::ConfigGetDefault(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		//return RWCoCreateInstance(__uuidof(ConfigInMemory), NULL, CLSCTX_INPROC_SERVER, __uuidof(IConfig), reinterpret_cast<void**>(a_ppConfig));

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		// TODO: implement IConfig that reacts to plug-in changes during runtime in this class
		CConfigValue cTrue(true);
		CComPtr<ILocalizedStringInit> pDummy;
		RWCoCreateInstance(pDummy, __uuidof(LocalizedString));
		CPlugInEnumerator::EnumCategoryCLSIDs(CATID_StorageFilterFactory, initconfig(pCfg, cTrue, pDummy));
		
		pCfg->ItemInsSimple(CComBSTR(CFGID_POSITIONX), pDummy, pDummy, CConfigValue((LONG)CW_USEDEFAULT), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_POSITIONY), pDummy, pDummy, CConfigValue((LONG)CW_USEDEFAULT), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SIZEX), pDummy, pDummy, CConfigValue((LONG)CW_USEDEFAULT), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SIZEY), pDummy, pDummy, CConfigValue((LONG)CW_USEDEFAULT), NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_ACTIVEPAGE), pDummy, pDummy, CConfigValue(__uuidof(StorageFilterFactoryFileSystem)), NULL, 0, NULL);

		pCfg->Finalize(NULL);
		*a_ppConfig = pCfg.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}
