// SubConfigSwitch.cpp : Implementation of CSubConfigSwitch

#include "stdafx.h"
#include "SubConfigSwitch.h"


// CSubConfigSwitch

STDMETHODIMP CSubConfigSwitch::ItemIDsEnum(IEnumStrings** a_ppIDs)
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

STDMETHODIMP CSubConfigSwitch::ItemValueGet(BSTR a_bstrID, TConfigValue* a_ptValue)
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

STDMETHODIMP CSubConfigSwitch::ItemValuesSet(ULONG a_nCount, BSTR* a_aIDs, const TConfigValue* a_atValues)
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

STDMETHODIMP CSubConfigSwitch::ItemGetUIInfo(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
{
	if (m_pActiveConfig)
	{
		return m_pActiveConfig->ItemGetUIInfo(a_bstrID, a_iidInfo, a_ppItemInfo);
	}
	else
	{
		CHECKPOINTER(a_ppItemInfo);
		*a_ppItemInfo = NULL;
		return E_RW_ITEMNOTFOUND;
	}
}

STDMETHODIMP CSubConfigSwitch::DuplicateCreate(IConfig** a_ppCopiedConfig)
{
	CHECKPOINTER(a_ppCopiedConfig);
	*a_ppCopiedConfig = NULL;

	CComObject<CSubConfigSwitch>* pCopy = NULL;
	CComObject<CSubConfigSwitch>::CreateInstance(&pCopy);

	CSubConfigSwitch* pAccess = pCopy;

	XSubConfigs::const_iterator i;
	for (i = m_xSubConfigs.begin(); i != m_xSubConfigs.end(); i++)
	{
		CComPtr<IConfig> pCfgCopy;
		i->second->DuplicateCreate(&pCfgCopy);
		pAccess->m_xSubConfigs[i->first] = pCfgCopy;
		if (m_pActiveConfig == i->second.p)
		{
			pAccess->m_pActiveConfig = pCfgCopy;
			pAccess->m_pActiveConfig->ObserverIns(pAccess->ObserverGet(), 0);
		}
	}

	pCopy->QueryInterface<IConfig>(a_ppCopiedConfig);

	return S_OK;
}

STDMETHODIMP CSubConfigSwitch::CopyFrom(IConfig* a_pSource, BSTR a_bstrIDPrefix)
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

STDMETHODIMP CSubConfigSwitch::SubConfigGet(BSTR a_bstrID, IConfig** a_ppSubConfig)
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

STDMETHODIMP CSubConfigSwitch::ControllerSet(const TConfigValue* a_ptValue)
{
	CHECKPOINTER(a_ptValue);

	XSubConfigs::const_iterator iVal = m_xSubConfigs.find(*a_ptValue);
	IConfig* pNewCfg = NULL;
	if (iVal != m_xSubConfigs.end())
	{
		pNewCfg = iVal->second;
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

STDMETHODIMP CSubConfigSwitch::ItemInsert(const TConfigValue* a_ptValue, IConfig* a_pConfig)
{
	CHECKPOINTER(a_ptValue);

	if (FAILED(a_pConfig->DuplicateCreate(&m_xSubConfigs[*a_ptValue])))
	{
		m_xSubConfigs.erase(*a_ptValue);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CSubConfigSwitch::OwnerNotify(ULONG UNREF(a_nCookie), IUnknown* a_pChangedParam)
{
	Fire_Notify(a_pChangedParam);
	return S_OK;
}

STDMETHODIMP CSubConfigSwitch::UIDGet(GUID* a_pguid)
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

STDMETHODIMP CSubConfigSwitch::RequiresMargins()
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

STDMETHODIMP CSubConfigSwitch::MinSizeGet(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY)
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

STDMETHODIMP CSubConfigSwitch::WindowCreate(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow)
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

