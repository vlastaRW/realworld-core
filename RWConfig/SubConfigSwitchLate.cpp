// SubConfigSwitchLate.cpp : Implementation of CSubConfigSwitchLate

#include "stdafx.h"
#include "SubConfigSwitchLate.h"


// CSubConfigSwitchLate

STDMETHODIMP CSubConfigSwitchLate::Init(ILateConfigCreator* a_pCreator)
{
	CHECKPOINTER(a_pCreator);

	m_pCreator = a_pCreator;

	return S_OK;
}

STDMETHODIMP CSubConfigSwitchLate::ItemIDsEnum(IEnumStrings** a_ppIDs)
{
	if (m_pActiveConfig)
	{
		return m_pActiveConfig->ItemIDsEnum(a_ppIDs);
	}
	else
	{
		CHECKPOINTER(a_ppIDs);
		return RWCoCreateInstance(__uuidof(EnumStrings), NULL, CLSCTX_ALL, __uuidof(IEnumStrings), (void**)a_ppIDs);
	}
}

STDMETHODIMP CSubConfigSwitchLate::ItemValueGet(BSTR a_bstrID, TConfigValue* a_ptValue)
{
	if (m_pActiveConfig)
	{
		return m_pActiveConfig->ItemValueGet(a_bstrID, a_ptValue);
	}
	else
	{
		CHECKPOINTER(a_ptValue);
		ConfigValueInit(*a_ptValue);
		return E_RW_ITEMNOTFOUND;
	}
}

STDMETHODIMP CSubConfigSwitchLate::ItemValuesSet(ULONG a_nCount, BSTR* a_aIDs, const TConfigValue* a_atValues)
{
	if (m_pActiveConfig)
	{
		return m_pActiveConfig->ItemValuesSet(a_nCount, a_aIDs, a_atValues);
	}
	else
	{
		return E_RW_ITEMNOTFOUND;
	}
}

STDMETHODIMP CSubConfigSwitchLate::ItemGetUIInfo(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
{
	if (m_pActiveConfig)
	{
		return m_pActiveConfig->ItemGetUIInfo(a_bstrID, a_iidInfo, a_ppItemInfo);
	}
	else
	{
		CHECKPOINTER(a_ppItemInfo);
		*a_ppItemInfo = 0;
		return E_RW_ITEMNOTFOUND;
	}
}

STDMETHODIMP CSubConfigSwitchLate::SubConfigGet(BSTR a_bstrID, IConfig** a_ppSubConfig)
{
	if (m_pActiveConfig)
	{
		return m_pActiveConfig->SubConfigGet(a_bstrID, a_ppSubConfig);
	}
	else
	{
		CHECKPOINTER(a_ppSubConfig);
		*a_ppSubConfig = NULL;
		return E_RW_ITEMNOTFOUND;
	}
}

STDMETHODIMP CSubConfigSwitchLate::DuplicateCreate(IConfig** a_ppCopiedConfig)
{
	CHECKPOINTER(a_ppCopiedConfig);
	*a_ppCopiedConfig = NULL;

	HRESULT hRes;
	CComObject<CSubConfigSwitchLate> *pCopy;
	hRes = CComObject<CSubConfigSwitchLate>::CreateInstance(&pCopy);
	if (FAILED(hRes))
		return hRes;

	CSubConfigSwitchLate* pAccess = pCopy;

	pAccess->m_pCreator = m_pCreator;

	XSubConfigs::const_iterator i;
	for (i = m_xConfigCache.begin(); i != m_xConfigCache.end(); i++)
	{
		CComPtr<IConfig> pCfgCopy;
		i->second->DuplicateCreate(&pCfgCopy);
		pAccess->m_xConfigCache[i->first] = pCfgCopy;
		if (m_pActiveConfig == i->second.p)
		{
			pAccess->m_pActiveConfig = pCfgCopy;
			pAccess->m_pActiveConfig->ObserverIns(pAccess->ObserverGet(), 0);
		}
	}

	(*a_ppCopiedConfig = pCopy)->AddRef();

	return S_OK;
}

STDMETHODIMP CSubConfigSwitchLate::CopyFrom(IConfig* a_pSource, BSTR a_bstrIDPrefix)
{
	if (m_pActiveConfig)
	{
		return m_pActiveConfig->CopyFrom(a_pSource, a_bstrIDPrefix);
	}
	else
	{
		return E_RW_ITEMNOTFOUND;
	}
}

STDMETHODIMP CSubConfigSwitchLate::ControllerSet(const TConfigValue* a_ptValue)
{
	CHECKPOINTER(a_ptValue);

	XSubConfigs::const_iterator iVal = m_xConfigCache.find(*a_ptValue);
	IConfig* pNewCfg = NULL;
	if (iVal != m_xConfigCache.end())
	{
		// already cached
		pNewCfg = iVal->second;
	}
	else
	{
		// not in cache
		m_pCreator->CreateConfig(a_ptValue, &pNewCfg);
		if (pNewCfg)
		{
			// insert into cache
			m_xConfigCache[*a_ptValue] = pNewCfg;
			pNewCfg->Release(); // AddRef in creator
		}
		else
		{
			CComPtr<IConfig>& c = m_xConfigCache[*a_ptValue];
			RWCoCreateInstance(c, __uuidof(ConfigInMemory));
			pNewCfg = c;
		}
	}

	if (m_pActiveConfig == pNewCfg)
	{
		return S_OK; // no change
	}

	CComPtr<IEnumStringsInit> pChanges;
	RWCoCreateInstance(pChanges, __uuidof(EnumStrings));

	if (m_pActiveConfig)
	{
		CComPtr<IEnumStrings> pTmp;
		m_pActiveConfig->ItemIDsEnum(&pTmp);
		pChanges->InsertFromEnum(pTmp);

		m_pActiveConfig->ObserverDel(ObserverGet(), 0);
	}

	m_pActiveConfig = pNewCfg;

	if (m_pActiveConfig)
	{
		m_pActiveConfig->ObserverIns(ObserverGet(), 0);

		CComPtr<IEnumStrings> pTmp;
		m_pActiveConfig->ItemIDsEnum(&pTmp);
		pChanges->InsertFromEnum(pTmp);
	}

	ULONG nChanges = 0;
	pChanges->Size(&nChanges);
	if (nChanges > 0)
	{
		Fire_Notify(pChanges.p);
	}

	return S_OK;
}

HRESULT CSubConfigSwitchLate::OwnerNotify(ULONG UNREF(a_nCookie), IUnknown* a_pChangedParam)
{
	Fire_Notify(a_pChangedParam);
	return S_OK;
}

STDMETHODIMP CSubConfigSwitchLate::UIDGet(GUID* a_pguid)
{
	CComQIPtr<IConfigCustomGUI> pCust(m_pActiveConfig);
	if (pCust)
	{
		return pCust->UIDGet(a_pguid);
	}
	else
	{
		return E_NOTIMPL;
	}
}

STDMETHODIMP CSubConfigSwitchLate::RequiresMargins()
{
	CComQIPtr<IConfigCustomGUI> pCust(m_pActiveConfig);
	if (pCust)
	{
		return pCust->RequiresMargins();
	}
	else
	{
		return E_NOTIMPL;
	}
}

STDMETHODIMP CSubConfigSwitchLate::MinSizeGet(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY)
{
	CComQIPtr<IConfigCustomGUI> pCust(m_pActiveConfig);
	if (pCust)
	{
		return pCust->MinSizeGet(a_pConfig, a_tLocaleID, a_eMode, a_nSizeX, a_nSizeY);
	}
	else
	{
		return E_NOTIMPL;
	}
}

STDMETHODIMP CSubConfigSwitchLate::WindowCreate(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;
		CComQIPtr<IConfigCustomGUI> pCust(m_pActiveConfig);
		if (pCust == NULL)
			return E_NOTIMPL;
		return pCust->WindowCreate(a_hParent, a_prcPositon, a_nCtlID, a_tLocaleID, a_bVisible, a_bParentBorder, a_pConfig, a_eMode, a_ppWindow);
	}
	catch (...)
	{
		return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
	}
}


