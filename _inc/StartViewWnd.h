
#pragma once

#include <SharedStringTable.h>


template<class T>
class CStartViewPageImpl :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CChildWindowImpl<T, IStartViewPage>
{
public:
	STDMETHOD(OnIdle)() { return E_NOTIMPL; }

BEGIN_COM_MAP(T)
	COM_INTERFACE_ENTRY(IStartViewPage)
	COM_INTERFACE_ENTRY(IChildWindow)
END_COM_MAP()
};

typedef HRESULT (fnInitConfig)(IConfigWithDependencies* a_pCfg);
HRESULT inline InitConfigNotImpl(IConfigWithDependencies*) { return E_NOTIMPL; }

template<class TStartViewPage, CLSID const* t_pClsID, UINT t_nNameID, UINT t_nDescID, UINT t_nIconID, fnInitConfig* t_pfnInitConfig = InitConfigNotImpl>
class CStartViewPageFactory :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStartViewPageFactory<TStartViewPage, t_pClsID, t_nNameID, t_nDescID, t_nIconID, t_pfnInitConfig>, t_pClsID>,
	public IStartViewPageFactory
{
public:
	typedef CStartViewPageFactory<TStartViewPage, t_pClsID, t_nNameID, t_nDescID, t_nIconID, t_pfnInitConfig> thisClass;

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(thisClass)

BEGIN_COM_MAP(thisClass)
	COM_INTERFACE_ENTRY(IStartViewPageFactory)
END_COM_MAP()

	// IStartViewPageFactory methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			*a_ppName = _SharedStringTable.GetString(t_nNameID);
			return S_OK;
		}
		catch (...)
		{
			return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(HelpText)(ILocalizedString** a_ppHelpText)
	{
		try
		{
			*a_ppHelpText = NULL;
			*a_ppHelpText = _SharedStringTable.GetString(t_nDescID);
			return S_OK;
		}
		catch (...)
		{
			return a_ppHelpText == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = 0;
			*a_phIcon = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_nIconID), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			return S_OK;
		}
		catch (...)
		{
			return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(InitConfig)(IConfigWithDependencies* a_pMainConfig)
	{
		try
		{
			return t_pfnInitConfig(a_pMainConfig);
		}
		catch (...)
		{
			return a_pMainConfig ? E_UNEXPECTED : E_POINTER;
		}
	}

	STDMETHOD(Create)(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig, IStartViewPage** a_ppPage)
	{
		try
		{
			*a_ppPage = NULL;
			CComObject<TStartViewPage>* p = NULL;
			CComObject<TStartViewPage>::CreateInstance(&p);
			CComPtr<IStartViewPage> pTmp = p;
			p->WindowCreate(a_hParent, a_prc, a_tLocaleID, a_pCallback, a_pAppConfig);
			*a_ppPage = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppPage == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
};

template<class TStartViewPage, CLSID const* t_pClsID, wchar_t const* t_pszName, wchar_t const* t_pszDesc, HICON (*t_pfnGetIcon)(ULONG) = NULL, fnInitConfig* t_pfnInitConfig = InitConfigNotImpl>
class CStartViewPageFactoryML :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStartViewPageFactoryML<TStartViewPage, t_pClsID, t_pszName, t_pszDesc, t_pfnGetIcon, t_pfnInitConfig>, t_pClsID>,
	public IStartViewPageFactory
{
public:
	typedef CStartViewPageFactoryML<TStartViewPage, t_pClsID, t_pszName, t_pszDesc, t_pfnGetIcon, t_pfnInitConfig> thisClass;

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(thisClass)

BEGIN_COM_MAP(thisClass)
	COM_INTERFACE_ENTRY(IStartViewPageFactory)
END_COM_MAP()

	// IStartViewPageFactory methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppName)
	{
		if (a_ppName == NULL)
			return E_POINTER;
		try
		{
			*a_ppName = new CMultiLanguageString(t_pszName);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(HelpText)(ILocalizedString** a_ppHelpText)
	{
		if (a_ppHelpText == NULL)
			return E_POINTER;
		try
		{
			*a_ppHelpText = new CMultiLanguageString(t_pszDesc);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			*a_phIcon = t_pfnGetIcon ? t_pfnGetIcon(a_nSize) : 0;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(InitConfig)(IConfigWithDependencies* a_pMainConfig)
	{
		try
		{
			return t_pfnInitConfig(a_pMainConfig);
		}
		catch (...)
		{
			return a_pMainConfig ? E_UNEXPECTED : E_POINTER;
		}
	}

	STDMETHOD(Create)(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig, IStartViewPage** a_ppPage)
	{
		try
		{
			*a_ppPage = NULL;
			CComObject<TStartViewPage>* p = NULL;
			CComObject<TStartViewPage>::CreateInstance(&p);
			CComPtr<IStartViewPage> pTmp = p;
			p->WindowCreate(a_hParent, a_prc, a_tLocaleID, a_pCallback, a_pAppConfig);
			*a_ppPage = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppPage == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
};