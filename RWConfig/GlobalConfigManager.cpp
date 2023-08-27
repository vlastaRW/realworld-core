// GlobalConfigManager.cpp : Implementation of CGlobalConfigManager

#include "stdafx.h"
#include "GlobalConfigManager.h"


// CGlobalConfigManager

STDMETHODIMP CGlobalConfigManager::EnumIDs(IEnumGUIDs** a_ppIDs)
{
	try
	{
		*a_ppIDs = NULL;
		ObjectLock lock(this);
		Validate();
		CComPtr<IEnumGUIDsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumGUIDs));
		for (CObjects::const_iterator i = m_cObjects.begin(); i != m_cObjects.end(); ++i)
			pTmp->Insert(i->first);
		*a_ppIDs = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppIDs ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CGlobalConfigManager::Interactive(REFGUID a_tID, BYTE* a_pPriority)
{
	try
	{
		ObjectLock lock(this);
		Validate();
		CObjects::const_iterator i = m_cObjects.find(a_tID);
		if (i == m_cObjects.end())
			return E_RW_ITEMNOTFOUND;
		return i->second.first->Interactive(a_pPriority);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CGlobalConfigManager::Name(REFGUID a_tID, ILocalizedString** a_ppName)
{
	try
	{
		ObjectLock lock(this);
		Validate();
		CObjects::const_iterator i = m_cObjects.find(a_tID);
		if (i == m_cObjects.end())
			return E_RW_ITEMNOTFOUND;
		return i->second.first->Name(a_ppName);
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CGlobalConfigManager::Description(REFGUID a_tID, ILocalizedString** a_ppDesc)
{
	try
	{
		ObjectLock lock(this);
		Validate();
		CObjects::const_iterator i = m_cObjects.find(a_tID);
		if (i == m_cObjects.end())
			return E_RW_ITEMNOTFOUND;
		return i->second.first->Description(a_ppDesc);
	}
	catch (...)
	{
		return a_ppDesc ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CGlobalConfigManager::Config(REFGUID a_tID, IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		ObjectLock lock(this);
		Validate();
		CObjects::iterator i = m_cObjects.find(a_tID);
		if (i == m_cObjects.end())
			return E_RW_ITEMNOTFOUND;
		(*a_ppConfig = i->second.second.p)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CGlobalConfigManager::GetValue(REFGUID a_tID, BSTR a_bstrID, TConfigValue* a_pVal)
{
	CComPtr<IConfig> p;
	Config(a_tID, &p);
	if (p == NULL) return E_RW_ITEMNOTFOUND;
	return p->ItemValueGet(a_bstrID, a_pVal);
}

#include <StringParsing.h>

STDMETHODIMP CGlobalConfigManager::ItemIDsEnum(IEnumStrings** a_ppIDs)
{
	if (a_ppIDs == NULL) return E_POINTER;

	try
	{
		CComPtr<IEnumStringsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
		ObjectLock lock(this);
		Validate();
		for (CObjects::const_iterator i = m_cObjects.begin(); i != m_cObjects.end(); ++i)
		{
			OLECHAR szID[64];
			StringFromGUID(i->first, szID);
			CComBSTR lead(szID);
			pTmp->Insert(lead);
			lead += L"\\";
			CComPtr<IEnumStrings> pSubIDs;
			i->second.second->ItemIDsEnum(&pSubIDs);
			ULONG nSubIDs = 0;
			if (pSubIDs) pSubIDs->Size(&nSubIDs);
			for (ULONG i = 0; i < nSubIDs; ++i)
			{
				CComBSTR bstr;
				pSubIDs->Get(i, &bstr);
				CComBSTR b(lead);
				b += bstr;
				pTmp->Insert(b);
			}
		}
		*a_ppIDs = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CGlobalConfigManager::ItemValueGet(BSTR a_bstrID, TConfigValue* a_ptValue)
{
	if (a_bstrID == NULL || a_ptValue == NULL) return E_POINTER;

	try
	{
		GUID tID = GUID_NULL;
		if (!GUIDFromString(a_bstrID, &tID))
			return E_RW_ITEMNOTFOUND;
		if (a_bstrID[36] == L'\0')
		{
			a_ptValue->eTypeID = ECVTBool;
			a_ptValue->bVal = VARIANT_TRUE;
			return S_OK;
		}
		if (a_bstrID[36] != L'\\')
			return E_RW_ITEMNOTFOUND;
		return GetValue(tID, CComBSTR(static_cast<wchar_t const*>(a_bstrID+37)), a_ptValue);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CGlobalConfigManager::ItemValuesSet(ULONG a_nCount, BSTR* a_aIDs, TConfigValue const* a_atValues)
{
	if (a_aIDs == NULL || a_atValues == NULL) return E_POINTER;

	try
	{
		ObjectLock lock(this);
		Validate();

		m_bUpdating = true;
		m_bChanged = false;

		// now set values of subitems
		CAutoVectorPtr<BSTR> aSubIDs(new BSTR[a_nCount]); // temp buffer of sub-items
		CAutoVectorPtr<TConfigValue> aSubVals(new TConfigValue[a_nCount]);

		for (CObjects::const_iterator i = m_cObjects.begin(); i != m_cObjects.end(); ++i)
		{
			ULONG nSubCount = 0;
			for (ULONG j = 0; j < a_nCount; j++)
			{
				GUID tID = GUID_NULL;
				if (!GUIDFromString(a_aIDs[j], &tID) || a_aIDs[j][36] != L'\\' || !IsEqualGUID(tID, i->first))
					continue;

				aSubIDs[nSubCount] = SysAllocString(a_aIDs[j]+37);
				aSubVals[nSubCount++] = a_atValues[j]; // shallow copy (no need to free the item)
			}
			if (nSubCount)
			{
				i->second.second->ItemValuesSet(nSubCount, aSubIDs, aSubVals);
				for (ULONG j = 0; j < nSubCount; j++)
				{
					SysFreeString(aSubIDs[j]);
				}
			}
		}
	}
	catch (...)
	{
		m_bUpdating = false;
		return E_UNEXPECTED;
	}

	m_bUpdating = false;
	if (m_bChanged)
	{
		// only send the notification if something has changed
		Fire_Notify(NULL);
	}

	return S_OK;
}

STDMETHODIMP CGlobalConfigManager::ItemGetUIInfo(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
{
	if (a_ppItemInfo == NULL) return E_POINTER;

	try
	{
		GUID tID = GUID_NULL;
		if (!GUIDFromString(a_bstrID, &tID))
			return E_RW_ITEMNOTFOUND;
		if (a_bstrID[36] == L'\0')
			return E_NOTIMPL;
		if (a_bstrID[36] != L'\\')
			return E_RW_ITEMNOTFOUND;

		ObjectLock lock(this);
		Validate();

		CObjects::iterator i = m_cObjects.find(tID);
		if (i == m_cObjects.end())
			return E_RW_ITEMNOTFOUND;
		return i->second.second->ItemGetUIInfo(CComBSTR(static_cast<wchar_t const*>(a_bstrID+37)), a_iidInfo, a_ppItemInfo);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CGlobalConfigManager::SubConfigGet(BSTR a_bstrID, IConfig** a_ppSubConfig)
{
	if (a_ppSubConfig == NULL) return E_POINTER;

	try
	{
		GUID tID = GUID_NULL;
		if (!GUIDFromString(a_bstrID, &tID))
			return E_RW_ITEMNOTFOUND;
		if (a_bstrID[36] == L'\0')
			return Config(tID, a_ppSubConfig);
		if (a_bstrID[36] != L'\\')
			return E_RW_ITEMNOTFOUND;

		ObjectLock lock(this);
		Validate();

		CObjects::iterator i = m_cObjects.find(tID);
		if (i == m_cObjects.end())
			return E_RW_ITEMNOTFOUND;
		return i->second.second->SubConfigGet(CComBSTR(static_cast<wchar_t const*>(a_bstrID+37)), a_ppSubConfig);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CGlobalConfigManager::DuplicateCreate(IConfig** a_ppCopiedConfig)
{
	try
	{
		if (a_ppCopiedConfig == NULL) return E_POINTER;
		ObjectLock lock(this);
		Validate();
		CComObject<CGlobalConfigManager>* p = NULL;
		CComObject<CGlobalConfigManager>::CreateInstance(&p);
		CComPtr<IConfig> pTmp = p;
		p->m_nStamp = m_nStamp;
		for (CObjects::const_iterator i = m_cObjects.begin(); i != m_cObjects.end(); ++i)
		{
			std::pair<CComPtr<IGlobalConfigFactory>, CComPtr<IConfig> >& t = p->m_cObjects[i->first];
			t.first = i->second.first;
			i->second.second->DuplicateCreate(&t.second);
		}
		*a_ppCopiedConfig = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CGlobalConfigManager::CopyFrom(IConfig* a_pSource, BSTR a_bstrIDPrefix)
{
	if (a_pSource == NULL)
		return S_FALSE;

	size_t nIDPrefix = a_bstrIDPrefix ? SysStringLen(a_bstrIDPrefix) : 0;

	try
	{
		ObjectLock lock(this);
		Validate();

		m_bUpdating = true;
		m_bChanged = false;

		for (CObjects::const_iterator i = m_cObjects.begin(); i != m_cObjects.end(); ++i)
		{
			OLECHAR szGUID[40];
			StringFromGUID(i->first, szGUID);

			CComBSTR bstrSubIDPrefix;
			if (nIDPrefix > 36)
			{
				if (wcsncmp(szGUID, a_bstrIDPrefix, 36) == 0 && a_bstrIDPrefix[36] == L'\\')
					bstrSubIDPrefix = a_bstrIDPrefix+37;
				else
					continue;
			}
			else
			{
				if (nIDPrefix && wcsncmp(szGUID, a_bstrIDPrefix, 36) != 0)
					continue;
			}

			CComBSTR bstrTmp(szGUID);
			CComPtr<IConfig> pSub;
			a_pSource->SubConfigGet(bstrTmp, &pSub);
			if (pSub)
			{
				i->second.second->CopyFrom(pSub, bstrSubIDPrefix);
			}
		}
	}
	catch (...)
	{
		m_bUpdating = false;
		return E_UNEXPECTED;
	}

	m_bUpdating = false;
	if (m_bChanged)
	{
		// only send the notification if something has changed
		Fire_Notify(NULL);
	}

	return S_OK;
}

#include <PlugInCache.h>

void CGlobalConfigManager::Validate()
{
	ULONG ts = CPlugInEnumerator::GetCategoryTimestamp(CATID_GlobalConfigFactory);
	if (ts != m_nStamp)
	{
		// update global configs
		std::map<CATID, CComPtr<IGlobalConfigFactory>, CPlugInEnumerator::lessCATID> items;
		CPlugInEnumerator::GetCategoryPlugInMap(CATID_GlobalConfigFactory, items);

		// first remove possibly uninstalled items
		for (CObjects::iterator i = m_cObjects.begin(); i != m_cObjects.end(); )
		{
			if (items.find(i->first.operator const GUID &()) == items.end())
			{
				CObjects::iterator j = i;
				++i;
				m_cObjects.erase(j);
			}
			else
			{
				items.erase(i->first.operator const GUID &()); // so that we do not attempt to add it again
				++i;
			}
		}

		// add new items
		for (std::map<CATID, CComPtr<IGlobalConfigFactory>, CPlugInEnumerator::lessCATID>::iterator i = items.begin(); i != items.end(); ++i)
		{
			if (i->second == NULL) continue; // this actually should never happen
			std::pair<CComPtr<IGlobalConfigFactory>, CComPtr<IConfig> >& t = m_cObjects[i->first];
			t.first.Attach(i->second);
			i->second.Detach();
			t.first->Config(&(t.second));
		}

		m_nStamp = ts;
	}
}

