
#pragma once

#ifdef SHAREDSTRINGTABLE_STRING_INCLUDED
template<class T, UINT t_uIDName, UINT t_uIDDesc, GUID const* t_pIconID, UINT t_uIDIcon>
class ATL_NO_VTABLE CDocumentMenuCommandImpl : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand
{
public:
	typedef CDocumentMenuCommandImpl<T, t_uIDName, t_uIDDesc, t_pIconID, t_uIDIcon> thisClass;

BEGIN_COM_MAP(thisClass)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()

	// overrides
	EMenuCommandState IntState() { return EMCSNormal; }


	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			if (t_uIDName == 0)
				return E_NOTIMPL;
			*a_ppText = _SharedStringTable.GetString(t_uIDName);
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			if (t_uIDDesc == 0)
				return E_NOTIMPL;
			*a_ppText = _SharedStringTable.GetString(t_uIDDesc);
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		try
		{
			if (t_pIconID == 0)
				return E_NOTIMPL;
			*a_pIconID = *t_pIconID;
			return S_OK;
		}
		catch (...)
		{
			return a_pIconID ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			if (t_uIDIcon == 0)
				return E_NOTIMPL;

			*a_phIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_uIDIcon), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(State)(EMenuCommandState* a_peState)
	{
		try
		{
			*a_peState = static_cast<T*>(this)->IntState();
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		return E_NOTIMPL;
	}
};
#endif

#ifdef MULTIMANGUAGE_STRING_INCLUDED
template<class T, OLECHAR const* t_pszName, OLECHAR const* t_pszDesc, GUID const* t_pIconID, UINT t_uIDIcon>
class ATL_NO_VTABLE CDocumentMenuCommandMLImpl : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand
{
public:
	typedef CDocumentMenuCommandMLImpl<T, t_pszName, t_pszDesc, t_pIconID, t_uIDIcon> thisClass;

BEGIN_COM_MAP(thisClass)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()

	// overrides
	EMenuCommandState IntState() { return EMCSNormal; }


	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			if (t_pszName == NULL)
				return E_NOTIMPL;
			*a_ppText = new CMultiLanguageString(t_pszName);
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			if (t_pszDesc == 0)
				return E_NOTIMPL;
			*a_ppText = new CMultiLanguageString(t_pszDesc);
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		try
		{
			if (t_pIconID == 0)
				return E_NOTIMPL;
			*a_pIconID = *t_pIconID;
			return S_OK;
		}
		catch (...)
		{
			return a_pIconID ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			if (t_uIDIcon == 0)
				return E_NOTIMPL;

			*a_phIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_uIDIcon), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(State)(EMenuCommandState* a_peState)
	{
		try
		{
			*a_peState = static_cast<T*>(this)->IntState();
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		return E_NOTIMPL;
	}
};

class ATL_NO_VTABLE CDocumentMenuCommand : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand
{
public:
	CDocumentMenuCommand(OLECHAR const* a_pszName, OLECHAR const* a_pszDesc, GUID const* a_pIconID, UINT a_uIDIcon) :
		m_pszName(a_pszName), m_pszDesc(a_pszDesc), m_pIconID(a_pIconID), m_uIDIcon(a_uIDIcon)
	{
	}

BEGIN_COM_MAP(CDocumentMenuCommand)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()

	// overrides
	EMenuCommandState IntState() { return EMCSNormal; }


	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			if (m_pszName == NULL)
				return E_NOTIMPL;
			*a_ppText = new CMultiLanguageString(m_pszName);
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			if (m_pszDesc == 0)
				return E_NOTIMPL;
			*a_ppText = new CMultiLanguageString(m_pszDesc);
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		try
		{
			if (m_pIconID == 0)
				return E_NOTIMPL;
			*a_pIconID = *m_pIconID;
			return S_OK;
		}
		catch (...)
		{
			return a_pIconID ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			if (m_uIDIcon == 0)
				return E_NOTIMPL;

			*a_phIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(m_uIDIcon), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands))
	{
		return E_NOTIMPL;
	}
	//STDMETHOD(State)(EMenuCommandState* UNREF(a_peState))
	//STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))

protected:
	wchar_t const* m_pszName;
	wchar_t const* m_pszDesc;
	GUID const* m_pIconID;
	UINT m_uIDIcon;
};

#endif
