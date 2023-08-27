
#pragma once

#ifdef __cplusplus

} // pause: extern "C"{


template<wchar_t const* t_pName, wchar_t const* t_pDesc, UINT t_nIDI, wchar_t const* t_pCategory, class TBuilder, ULONG t_nPriority = 256, class TBase = IDesignerWizard>
class CDesignerWizardImpl : public TBase
{
	// IDocumentCreator methods
public:
	STDMETHOD(Priority)(ULONG* a_pPriority)
	{
		try
		{
			*a_pPriority = t_nPriority;
			return S_OK;
		}
		catch (...)
		{
			return a_pPriority ? E_UNEXPECTED : E_POINTER;
		}
	}

	STDMETHOD(Name)(ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			*a_ppName = new CMultiLanguageString(t_pName);
			return S_OK;
		}
		catch (...)
		{
			return a_ppName ? E_UNEXPECTED : E_POINTER;
		}
	}

	STDMETHOD(HelpText)(ILocalizedString** a_ppHelpText)
	{
		try
		{
			*a_ppHelpText = NULL;
			*a_ppHelpText = new CMultiLanguageString(t_pDesc);
			return S_OK;
		}
		catch (...)
		{
			return a_ppHelpText ? E_UNEXPECTED : E_POINTER;
		}
	}

	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			*a_phIcon = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_nIDI), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			return S_OK;
		}
		catch (...)
		{
			return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	STDMETHOD(Category)(ILocalizedString** a_ppCategory)
	{
		try
		{
			*a_ppCategory = NULL;
			*a_ppCategory = new CMultiLanguageString(t_pCategory);
			return S_OK;
		}
		catch (...)
		{
			return a_ppCategory ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IsCompatible)(ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders)
	{
		try
		{
			for (ULONG i = 0; i < a_nBuilders; ++i)
			{
				CComPtr<IUnknown> p;
				a_apBuilders[i]->QueryInterface(__uuidof(TBuilder), reinterpret_cast<void**>(&p));
				if (p)
					return S_OK;
			}
			return S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Activate)(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
	{
		try
		{
			for (ULONG i = 0; i < a_nBuilders; ++i)
			{
				CComPtr<TBuilder> p;
				a_apBuilders[i]->QueryInterface(__uuidof(TBuilder), reinterpret_cast<void**>(&p));
				if (p)
					return Activate(a_hParentWnd, a_tLocaleID, a_pConfig, p, a_bstrPrefix, a_pBase);
			}
			return E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Activate)(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, TBuilder* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
	{
		ATLASSERT(0); // should be implemented in owner
		return E_FAIL;
	}
};

extern "C"{ // continue: extern "C"{

#endif//__cplusplus