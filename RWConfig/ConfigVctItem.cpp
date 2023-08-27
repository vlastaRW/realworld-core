// ConfigVctItem.cpp : Implementation of CConfigVctItem

#include "stdafx.h"
#include "ConfigVctItem.h"


// CConfigVctItem

HRESULT CConfigVctItem::Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, IVectorItemName* a_pCust, IConfig* a_pConfig, ULONG a_nIndex)
{
	m_pName = a_pName;
	m_pDesc = a_pDesc;
	m_pCust = a_pCust;
	m_pConfig = a_pConfig;
	m_nIndex = a_nIndex;
	return S_OK;
}

STDMETHODIMP CConfigVctItem::NameGet(ILocalizedString** a_ppName, ILocalizedString** a_ppHelpText)
{
	if (a_ppName)
	{
		*a_ppName = m_pName;
		if (*a_ppName)
			(*a_ppName)->AddRef();
	}

	if (a_ppHelpText)
	{
		*a_ppHelpText = m_pDesc;
		if (*a_ppHelpText)
			(*a_ppHelpText)->AddRef();
	}

	return S_OK;
}

STDMETHODIMP CConfigVctItem::ValueGetName(const TConfigValue* a_ptValue, ILocalizedString** a_ppName)
{
	CHECKPOINTER(a_ppName);

	if (m_pCust == NULL)
		return E_NOTIMPL;

	// TODO: check value
	return m_pCust->Get(m_nIndex, m_pConfig, a_ppName);
}

STDMETHODIMP CConfigVctItem::ValueIsValid(const TConfigValue* a_ptValue)
{
	if (m_pCust || a_ptValue == NULL || a_ptValue->eTypeID != ECVTString)
	{
		return S_FALSE;
	}
	else
	{
		return S_OK;
	}
}

STDMETHODIMP CConfigVctItem::IsEnabled()
{
	return m_pCust ? S_FALSE : S_OK;
}
