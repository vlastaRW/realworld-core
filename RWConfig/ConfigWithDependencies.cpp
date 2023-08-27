// ConfigWithDependencies.cpp : Implementation of CConfigWithDependencies

#include "stdafx.h"

#include "ConfigWithDependencies.h"

#include "ConfigWDItemOptions.h"
#include "ConfigWDItemRange.h"
#include "ConfigWDItemSimple.h"
#include "ConfigItemConditions.h"
#include "ConfigWDItemCustomOptions.h"

// CConfigWithDependencies

CConfigWithDependencies::~CConfigWithDependencies()
{
	// disconnect the observers
	AItems::iterator i;
	ULONG j;
	for (j = 0, i = m_aItems.begin(); i != m_aItems.end(); i++, j++)
	{
		if (i->pHandler != NULL)
		{
			i->pHandler->ObserverDel(ObserverGet(), static_cast<ULONG>(j));
		}
	}
	// if some fool still keeps references to any item of this config,
	// it will stop working now (methods returning E_FAIL)
	for (i = m_aItems.begin(); i != m_aItems.end(); i++)
	{
		i->pItem->ParentDestroyed();
	}
}

STDMETHODIMP CConfigWithDependencies::ItemIDsEnum(IEnumStrings** a_ppIDs)
{
	ATLASSERT(m_bFinalized);

	CHECKPOINTER(a_ppIDs);
	*a_ppIDs = NULL;

	CComPtr<IEnumStringsInit> pESInit;
	HRESULT hRes = RWCoCreateInstance(pESInit, __uuidof(EnumStrings));
	if (FAILED(hRes))
		return hRes;

	ObjectLock cLock(this);

	AItems::const_iterator i;
	for (i = m_aItems.begin(); i != m_aItems.end(); i++)
	{
		pESInit->Insert(i->bstrID);
		if (i->pHandler)
		{
			CComPtr<IEnumStrings> pSubStrings;
			if (SUCCEEDED(i->pHandler->ItemIDsEnum(&pSubStrings)))
			{
				ULONG j;
				BSTR bstrSubID = NULL;
				for (j = 0; SUCCEEDED(pSubStrings->Get(j, &bstrSubID)); j++)
				{
					CComBSTR strComposed(i->bstrID);
					strComposed += L"\\";
					strComposed += bstrSubID;
					SysFreeString(bstrSubID);
					pESInit->Insert(strComposed);
				}
			}
		}
	}

	*a_ppIDs = pESInit.Detach();

	return S_OK;
}

STDMETHODIMP CConfigWithDependencies::ItemValueGet(BSTR a_bstrID, TConfigValue* a_ptValue)
{
	ATLASSERT(m_bFinalized);

	CHECKPOINTER(a_ptValue);
	a_ptValue->eTypeID = ECVTEmpty;

	ObjectLock cLock(this);

	const SItem* pItem = NULL;
	CComBSTR bstrSubID;
	HRESULT hRes = FindItem(a_bstrID, &pItem, &bstrSubID);
	if (FAILED(hRes))
	{
		return hRes;
	}

	if (bstrSubID != NULL)
	{
		// sub-item
		return pItem->pHandler->ItemValueGet(bstrSubID, a_ptValue);
	}
	else
	{
		*a_ptValue = ConfigValueCopy(pItem->tActualValue);
		return S_OK;
	}
}

struct lessBSTR
{
	bool operator()(BSTR a_1, BSTR a_2) const
	{
		return a_1 != a_2 && wcscmp(a_1, a_2) < 0;
	}
};

struct SSetItem
{
	SSetItem() : nIndex(0xffffffff) {}
	ULONG nIndex;
	std::vector<BSTR> cSubIDs;
	std::vector<TConfigValue> cSubVals;
};

STDMETHODIMP CConfigWithDependencies::ItemValuesSet(ULONG a_nCount, BSTR* a_aIDs, const TConfigValue* a_atValues)
{
	ATLASSERT(m_bFinalized);

	if (a_nCount == 0)
		return S_FALSE;
	CHECKPOINTER(a_aIDs);
	CHECKPOINTER(a_atValues);

	HRESULT hRes = S_OK;
	std::map<BSTR, SSetItem, lessBSTR> aItems;
	try
	{
		{
		ObjectLock cLock(this);
		m_bUpdating = true;
		m_bChanged = false;

		// prepare input values
		for (ULONG j = 0; j < a_nCount; j++)
		{
			LPCOLESTR p = wcschr(a_aIDs[j], L'\\');
			if (p)
			{
				CComBSTR bstr1(p-a_aIDs[j], a_aIDs[j]);
				std::map<BSTR, SSetItem, lessBSTR>::iterator i1 = aItems.find(bstr1);
				if (i1 != aItems.end())
				{
					CComBSTR bstr1(SysStringLen(a_aIDs[j])-(p-a_aIDs[j])-1, p+1);
					i1->second.cSubIDs.push_back(bstr1);
					bstr1.Detach();
					i1->second.cSubVals.push_back(a_atValues[j]);
				}
				else
				{
					XItemIDs::iterator i2 = m_xItemIDs.find(bstr1);
					if (i2 != m_xItemIDs.end())
					{
						SSetItem& s = aItems[i2->first];
						CComBSTR bstr1(SysStringLen(a_aIDs[j])-(p-a_aIDs[j])-1, p+1);
						s.cSubIDs.push_back(bstr1);
						bstr1.Detach();
						s.cSubVals.push_back(a_atValues[j]);
					}
				}
			}
			else
			{
				SSetItem& tItem = aItems[a_aIDs[j]];
				tItem.nIndex = j;
			}
		}

		std::set<BSTR> aAffected;

		// go through all items in predefined order and set them
		AItems::iterator i;
		for (i = m_aItems.begin(); i != m_aItems.end(); i++)
		{
			bool bModified = false;
			std::map<BSTR, SSetItem, lessBSTR>::iterator i1 = aItems.find(i->bstrID);
			// check if this item is in the input array and its type is correct
			if (i1 != aItems.end())
			{
				ULONG iIn = i1->second.nIndex;
				if (iIn != 0xffffffff && i->tActualValue != a_atValues[iIn] && (i->pItem->IsValid(a_atValues[iIn]) || (i->tActualValue.TypeGet() == a_atValues[iIn].eTypeID && !i->pItem->IsValid(i->tActualValue))))
				{
					// set the new value and update set of potentially affected items
					i->tActualValue = a_atValues[iIn];
					bModified = true;
				}
			}
			if (!bModified && aAffected.find(i->bstrID) != aAffected.end())
			{
				// this item needs to be checked and fixed if neccessary
				if (!i->pItem->IsValid(i->tActualValue))
				{
					if (i->pItem->IsValid(i->tDefaultValue))
					{
						i->tActualValue = i->tDefaultValue; // TODO: is default always valid?
						bModified = true;
					}
				}
			}
			if (bModified)
			{
				m_bChanged = true;
				i->pItem->DependantsMerge(aAffected);
				if (i->pHandler)
				{
					i->pHandler->ControllerSet(i->tActualValue);
				}
			}
			// subitem values
			if (i1 != aItems.end() && i1->second.cSubIDs.size() && i->pHandler)
			{
				i->pHandler->ItemValuesSet(i1->second.cSubIDs.size(), &(i1->second.cSubIDs[0]), &(i1->second.cSubVals[0]));
			}
		}
		}
		if (m_bChanged)
		{
			// only send the notification if something has changed
			Fire_Notify(NULL);
		}
		m_bUpdating = false;
	}
	catch (...)
	{
		hRes = E_UNEXPECTED;
	}
	for (std::map<BSTR, SSetItem, lessBSTR>::iterator i = aItems.begin(); i != aItems.end(); ++i)
	{
		for (std::vector<BSTR>::iterator ii = i->second.cSubIDs.begin(); ii != i->second.cSubIDs.end(); ++ii)
		{
			SysFreeString(*ii);
		}
	}
	return hRes;
}

STDMETHODIMP CConfigWithDependencies::ItemGetUIInfo(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
{
	ATLASSERT(m_bFinalized);

	CHECKPOINTER(a_ppItemInfo);
	*a_ppItemInfo = NULL;

	ObjectLock cLock(this);

	const SItem* pItem = NULL;
	CComBSTR bstrSubID;
	HRESULT hRes = FindItem(a_bstrID, &pItem, &bstrSubID);
	if (FAILED(hRes))
	{
		return hRes;
	}

	if (bstrSubID != NULL)
	{
		// sub-item
		return pItem->pHandler->ItemGetUIInfo(bstrSubID, a_iidInfo, a_ppItemInfo);
	}
	else
	{
		return pItem->pItem->QueryInterface(a_iidInfo, a_ppItemInfo);
	}
}

STDMETHODIMP CConfigWithDependencies::SubConfigGet(BSTR a_bstrID, IConfig** a_ppSubConfig)
{
	ATLASSERT(m_bFinalized);

	CHECKPOINTER(a_ppSubConfig);
	*a_ppSubConfig = NULL;

	ObjectLock cLock(this);

	const SItem* pItem = NULL;
	CComBSTR bstrSubID;
	HRESULT hRes = FindItem(a_bstrID, &pItem, &bstrSubID);
	if (FAILED(hRes))
	{
		return hRes;
	}

	if (bstrSubID != NULL)
	{
		// sub-item
		return pItem->pHandler->SubConfigGet(bstrSubID, a_ppSubConfig);
	}
	else
	{
		*a_ppSubConfig = pItem->pHandler;
		if (*a_ppSubConfig)
		{
			(*a_ppSubConfig)->AddRef();
			return S_OK;
		}
		else
		{
			return S_FALSE; // TODO: ???
		}
	}
}

STDMETHODIMP CConfigWithDependencies::DuplicateCreate(IConfig** a_ppCopiedConfig)
{
	ATLASSERT(m_bFinalized);

	CHECKPOINTER(a_ppCopiedConfig);
	*a_ppCopiedConfig = NULL;

	HRESULT hRes;
	CComObject<CConfigWithDependencies> *pCopy;
	hRes = CComObject<CConfigWithDependencies>::CreateInstance(&pCopy);
	if (FAILED(hRes))
		return hRes;

	ObjectLock cLock(this);

	CConfigWithDependencies* pAccess = pCopy;
	pAccess->m_pCustomGUI = m_pCustomGUI;

	XItemIDs::const_iterator iIDs;
	for (iIDs = m_xItemIDs.begin(); iIDs != m_xItemIDs.end(); iIDs++)
	{
		pAccess->m_xItemIDs[iIDs->first] = iIDs->second;
	}

	AItems::const_iterator i;
	for (i = m_aItems.begin(); i != m_aItems.end(); i++)
	{
		SItem sTmp;
		sTmp.bstrID = i->bstrID;
		sTmp.tActualValue = i->tActualValue;
		sTmp.tDefaultValue = i->tDefaultValue;
		CComPtr<IConfig> pTmpCfg;
		if (i->pHandler != NULL)
		{
			i->pHandler->DuplicateCreate(&pTmpCfg);
			sTmp.pHandler = com_cast<ISubConfig>(pTmpCfg);
			sTmp.pHandler->ObserverIns(pAccess->ObserverGet(), static_cast<ULONG>(i-m_aItems.begin()));
		}
		else
		{
			sTmp.pHandler = NULL;
		}
		sTmp.pItem = i->pItem->Clone(static_cast<IConfigWithDependencies*>(pAccess));
		pAccess->m_aItems.push_back(sTmp);
	}
#ifdef DEBUG
	pAccess->m_bFinalized = true;
#endif

	return pCopy->QueryInterface<IConfig>(a_ppCopiedConfig);
}

STDMETHODIMP CConfigWithDependencies::CopyFrom(IConfig* a_pSource, BSTR a_bstrIDPrefix)
{
	ATLASSERT(m_bFinalized);

	if (a_pSource == NULL)
		return S_FALSE;

	size_t nIDPrefix = a_bstrIDPrefix ? SysStringLen(a_bstrIDPrefix) : 0;

	ObjectLock cLock(this);

	HRESULT hRes = S_OK;
	try
	{
		m_bUpdating = true;
		m_bChanged = false;

		std::set<BSTR> aAffected;

		// go through all items in predefined order and set them
		AItems::iterator i;
		for (i = m_aItems.begin(); i != m_aItems.end(); i++)
		{
			CComBSTR bstrSubIDPrefix;
			size_t const nIDLen = SysStringLen(i->bstrID);
			if (nIDLen < nIDPrefix)
			{
				if (wcsncmp(i->bstrID, a_bstrIDPrefix, nIDLen) == 0 && a_bstrIDPrefix[nIDLen] == L'\\')
					bstrSubIDPrefix = a_bstrIDPrefix+nIDLen+1;
				else
					continue;
			}
			else
			{
				if (nIDPrefix && wcsncmp(i->bstrID, a_bstrIDPrefix, nIDPrefix) != 0)
					continue;
			}

			bool bModified = false;
			CConfigValue cVal;
			if (SUCCEEDED(a_pSource->ItemValueGet(i->bstrID, &cVal)))
			{
				if (cVal.TypeGet() == ECVTInteger && i->tDefaultValue.TypeGet() == ECVTFloat)
					cVal = float(cVal.operator LONG());
				if (cVal.TypeGet() == i->tDefaultValue.TypeGet() && i->tActualValue != cVal)
				{
					// set the new value and update set of potentially affected items
					i->tActualValue = cVal;
					bModified = true;
				}
			}
			if (!bModified && aAffected.find(i->bstrID) != aAffected.end())
			{
				// this item needs to be checked and fixed if neccessary
				if (!i->pItem->IsValid(i->tActualValue))
				{
					if (i->pItem->IsValid(i->tDefaultValue))
					{
						i->tActualValue = i->tDefaultValue; // TODO: is default always valid?
						bModified = true;
					}
				}
			}
			if (bModified)
			{
				m_bChanged = true;
				i->pItem->DependantsMerge(aAffected);
				if (i->pHandler)
				{
					i->pHandler->ControllerSet(i->tActualValue);
				}
			}
			// subitem values
			CComPtr<IConfig> pSub;
			a_pSource->SubConfigGet(i->bstrID, &pSub);
			if (pSub && i->pHandler)
			{
				i->pHandler->CopyFrom(pSub, bstrSubIDPrefix);
			}
		}

		if (m_bChanged)
		{
			// only send the notification if anything has changed
			Fire_Notify(NULL);
		}
		m_bUpdating = false;
	}
	catch (...)
	{
		hRes = E_UNEXPECTED;
	}
	return hRes;
}

STDMETHODIMP CConfigWithDependencies::ItemIns1ofN(BSTR a_bstrID, ILocalizedString* a_pName, ILocalizedString* a_pDescription, const TConfigValue* a_ptDefaultValue, ISubConfig* a_pHandler)
{
	ATLASSERT(!m_bFinalized);

	ObjectLock cLock(this);

#ifndef RW_FASTBUTUNSAFE
	if (m_xItemIDs.find(a_bstrID) != m_xItemIDs.end())
	{
		// item already exists
		ATLASSERT(0);
		return E_FAIL;
	}
#endif

	SItem sItem;
	sItem.bstrID = a_bstrID;
	sItem.tDefaultValue = *a_ptDefaultValue;
	sItem.tActualValue = *a_ptDefaultValue;
	sItem.pHandler = a_pHandler;
	CComObject<CConfigWDItemOptions>* pItem = NULL;
	CComObject<CConfigWDItemOptions>::CreateInstance(&pItem);
	pItem->Init(a_pName, a_pDescription, *a_ptDefaultValue, static_cast<IConfigWithDependencies*>(this));
	sItem.pItem = pItem;

	m_aItems.push_back(sItem);
	m_xItemIDs[a_bstrID] = m_aItems.size()-1; // ugh

	return S_OK;
}

STDMETHODIMP CConfigWithDependencies::ItemOptionAdd(BSTR a_bstrID, const TConfigValue* a_ptValue, ILocalizedString* a_pName, ULONG a_nConditions, const TConfigOptionCondition* a_aConditions)
{
	ATLASSERT(!m_bFinalized);

	ObjectLock cLock(this);

#ifndef RW_FASTBUTUNSAFE
	if (m_xItemIDs.find(a_bstrID) == m_xItemIDs.end())
	{
		// item does not exist
		ATLASSERT(0);
		return E_FAIL;
	}
#endif

	IConfigWDControl* pTmp = m_aItems[m_xItemIDs[a_bstrID]].pItem; // shit 1
	CConfigWDItemOptions* pItem = static_cast<CConfigWDItemOptions*>(pTmp); // shit 2

	pItem->InsertOption(a_ptValue, a_pName, new CConfigItemConditions(a_aConditions, a_nConditions));

	return S_OK;
}

STDMETHODIMP CConfigWithDependencies::ItemInsRanged(BSTR a_bstrID, ILocalizedString* a_pName, ILocalizedString* a_pHelpText, const TConfigValue* a_ptDefaultValue, ISubConfig* a_pSubConfig, const TConfigValue* a_ptFrom, const TConfigValue* a_ptTo, const TConfigValue* a_ptStep, ULONG a_nConditions, const TConfigOptionCondition* a_aConditions)
{
	ATLASSERT(!m_bFinalized);

	ObjectLock cLock(this);

#ifndef RW_FASTBUTUNSAFE
	if (m_xItemIDs.find(a_bstrID) != m_xItemIDs.end())
	{
		// item already exists
		ATLASSERT(0);
		return E_FAIL;
	}
#endif

	SItem sItem;
	sItem.bstrID = a_bstrID;
	sItem.tDefaultValue = *a_ptDefaultValue;
	sItem.tActualValue = *a_ptDefaultValue;
	sItem.pHandler = a_pSubConfig;
	CComObject<CConfigWDItemRange>* pItem = NULL;
	CComObject<CConfigWDItemRange>::CreateInstance(&pItem);
	pItem->Init(a_pName, a_pHelpText, *a_ptDefaultValue, static_cast<IConfigWithDependencies*>(this), *a_ptFrom, *a_ptTo, *a_ptStep, new CConfigItemConditions(a_aConditions, a_nConditions));
	sItem.pItem = pItem;

	m_aItems.push_back(sItem);
	m_xItemIDs[a_bstrID] = m_aItems.size()-1; // ugh

	return S_OK;
}

STDMETHODIMP CConfigWithDependencies::ItemInsSimple(BSTR a_bstrID, ILocalizedString* a_pName, ILocalizedString* a_pHelpText, const TConfigValue* a_ptDefaultValue, ISubConfig* a_pSubConfig, ULONG a_nConditions, const TConfigOptionCondition* a_aConditions)
{
	ATLASSERT(!m_bFinalized);

	ObjectLock cLock(this);

#ifndef RW_FASTBUTUNSAFE
	if (m_xItemIDs.find(a_bstrID) != m_xItemIDs.end())
	{
		// item already exists
		ATLASSERT(0);
		return E_FAIL;
	}
#endif

	SItem sItem;
	sItem.bstrID = a_bstrID;
	sItem.tDefaultValue = *a_ptDefaultValue;
	sItem.tActualValue = *a_ptDefaultValue;
	sItem.pHandler = a_pSubConfig;
	CComObject<CConfigWDItemSimple>* pItem = NULL;
	CComObject<CConfigWDItemSimple>::CreateInstance(&pItem);
	pItem->Init(a_pName, a_pHelpText, *a_ptDefaultValue, static_cast<IConfigWithDependencies*>(this), a_ptDefaultValue->eTypeID, new CConfigItemConditions(a_aConditions, a_nConditions));
	sItem.pItem = pItem;

	m_aItems.push_back(sItem);
	m_xItemIDs[a_bstrID] = m_aItems.size()-1; // ugh

	return S_OK;
}

STDMETHODIMP CConfigWithDependencies::ItemIns1ofNWithCustomOptions(BSTR a_bstrID, ILocalizedString* a_pName, ILocalizedString* a_pHelpText, const TConfigValue* a_ptDefaultValue, IConfigItemCustomOptions* a_pOptions, ISubConfig* a_pSubConfig, ULONG a_nConditions, const TConfigOptionCondition* a_aConditions)
{
	ATLASSERT(!m_bFinalized);

	ObjectLock cLock(this);

#ifndef RW_FASTBUTUNSAFE
	if (m_xItemIDs.find(a_bstrID) != m_xItemIDs.end())
	{
		// item already exists
		ATLASSERT(0);
		return E_FAIL;
	}
#endif

	SItem sItem;
	sItem.bstrID = a_bstrID;
	sItem.tDefaultValue = *a_ptDefaultValue;
	sItem.tActualValue = *a_ptDefaultValue;
	sItem.pHandler = a_pSubConfig;
	CComObject<CConfigWDItemCustomOptions>* pItem = NULL;
	CComObject<CConfigWDItemCustomOptions>::CreateInstance(&pItem);
	pItem->Init(a_pName, a_pHelpText, *a_ptDefaultValue, static_cast<IConfigWithDependencies*>(this), a_pOptions, new CConfigItemConditions(a_aConditions, a_nConditions));
	sItem.pItem = pItem;

	m_aItems.push_back(sItem);
	m_xItemIDs[a_bstrID] = m_aItems.size()-1; // ugh

	return S_OK;
}


STDMETHODIMP CConfigWithDependencies::Finalize(IConfigCustomGUI* a_pCustomGUI)
{
	ObjectLock cLock(this);

#ifdef DEBUG
	ATLASSERT(!m_bFinalized);
	m_bFinalized = true;
#endif

	try
	{
		m_pCustomGUI = a_pCustomGUI;

		bool bError = false;
		// extract superior itemIDs from all items and initialize their dependat sets
		AItems::iterator i;
		for (i = m_aItems.begin(); i != m_aItems.end(); i++)
		{
			set<CComBSTR> xTmp;
			i->pItem->SuperiorsGet(xTmp);
			set<CComBSTR>::const_iterator j;
			for (j = xTmp.begin(); j != xTmp.end(); j++)
			{
				XItemIDs::const_iterator iIndex = m_xItemIDs.find(*j);
				if (iIndex != m_xItemIDs.end())
				{
					m_aItems[iIndex->second].pItem->DependantInsert(i->bstrID);
				}
				else
				{
					bError = true;
				}
			}
		}

		// register observer from all sub-configs and set their controllers
		m_bUpdating = true; // prevent notifications
		ULONG j;
		for (j = 0, i = m_aItems.begin(); i != m_aItems.end(); i++, j++)
		{
			if (i->pHandler != NULL)
			{
				i->pHandler->ControllerSet(i->tActualValue);
				i->pHandler->ObserverIns(ObserverGet(), static_cast<ULONG>(j));
			}
		}
		m_bUpdating = false;

		return bError ? E_FAIL : S_OK;
		// TODO: what to do if default values give out an invalid config ?
//		return InternItemValuesSet(m_aItemIDs, 0, NULL, NULL);
	}
	catch (...)
	{
		return E_FAIL;
	}
}

HRESULT CConfigWithDependencies::OwnerNotify(TCookie a_tCookie, IUnknown* a_pChangedParam)
{
	if (m_bUpdating)
	{
		m_bChanged = true;
	}
	else
	{
		Fire_Notify(a_pChangedParam);
	}
	return S_OK;
}

HRESULT CConfigWithDependencies::FindItem(BSTR a_bstrID, const SItem** a_ppItem, BSTR* a_pbstrSubID) const
{
	*a_ppItem = NULL;
	*a_pbstrSubID = NULL;

	ULONG i;
	LPCOLESTR pStr = a_bstrID;
	for (i = 0; pStr[i] && pStr[i] != L'\\'; i++) ;
	if (pStr[i])
	{
		// '\' found
		BSTR bstrTmp = SysAllocStringLen(a_bstrID, i);
		XItemIDs::const_iterator tItem = m_xItemIDs.find(bstrTmp);
		SysFreeString(bstrTmp);
		if (tItem == m_xItemIDs.end())
			return E_RW_ITEMNOTFOUND;
		// this item should have a sub-config
		if (m_aItems[tItem->second].pHandler == NULL)
			return E_FAIL; // TODO: error code
		*a_ppItem = &m_aItems[tItem->second];
		*a_pbstrSubID = SysAllocString(pStr+i+1);
		return S_OK;
	}
	else
	{
		XItemIDs::const_iterator tItem = m_xItemIDs.find(a_bstrID);
		if (tItem == m_xItemIDs.end())
			return E_RW_ITEMNOTFOUND;
		*a_ppItem = &m_aItems[tItem->second];
		return S_OK;
	}
}

STDMETHODIMP CConfigWithDependencies::UIDGet(GUID* a_pguid)
{
	if (m_pCustomGUI == NULL)
		return E_NOTIMPL;
	return m_pCustomGUI->UIDGet(a_pguid);
}

STDMETHODIMP CConfigWithDependencies::RequiresMargins()
{
	if (m_pCustomGUI == NULL)
		return E_NOTIMPL;
	return m_pCustomGUI->RequiresMargins();
}

STDMETHODIMP CConfigWithDependencies::MinSizeGet(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY)
{
	if (m_pCustomGUI == NULL)
		return E_NOTIMPL;
	return m_pCustomGUI->MinSizeGet(a_pConfig, a_tLocaleID, a_eMode, a_nSizeX, a_nSizeY);
}

STDMETHODIMP CConfigWithDependencies::WindowCreate(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;
		if (m_pCustomGUI == NULL)
			return E_NOTIMPL;
		return m_pCustomGUI->WindowCreate(a_hParent, a_prcPositon, a_nCtlID, a_tLocaleID, a_bVisible, a_bParentBorder, a_pConfig, a_eMode, a_ppWindow);
	}
	catch (...)
	{
		return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

