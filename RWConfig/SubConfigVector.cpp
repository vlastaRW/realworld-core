// SubConfigVector.cpp : Implementation of CSubConfigVector

#include "stdafx.h"
#include "SubConfigVector.h"
#include "ConfigVctItem.h"
#include <SharedStringTable.h>

// CSubConfigVector

CSubConfigVector::~CSubConfigVector()
{
	ATLASSERT(!m_bUpdating); // this really is a very redundant check

	AItems::const_iterator i;
	ULONG j;
	for (j = 0, i = m_aItems.begin(); i != m_aItems.end(); i++, j++)
	{
		i->pConfig->ObserverDel(ObserverGet(), 0/*j*/);
	}
}

STDMETHODIMP CSubConfigVector::ItemIDsEnum(IEnumStrings** a_ppIDs)
{
	CHECKPOINTER(a_ppIDs);
	*a_ppIDs = NULL;

	ObjectLock cLock(this);

	CComPtr<IEnumStringsInit> pESInit;
	RWCoCreateInstance(pESInit, __uuidof(EnumStrings));

	AItems::const_iterator i;
	ULONG j;
	for (j = 0, i = m_aItems.begin(); i != m_aItems.end(); i++, j++)
	{
		TCHAR szTmp[16];
		_stprintf(szTmp, _T("%08x"), j);
		pESInit->Insert(CComBSTR(szTmp));
		szTmp[8] = _T('\\');
		szTmp[9] = _T('\0');
		InsertSubCfgIDs(pESInit, *i, szTmp);
	}

	*a_ppIDs = pESInit.Detach();

	return S_OK;
}

STDMETHODIMP CSubConfigVector::ItemValueGet(BSTR a_bstrID, TConfigValue* a_ptValue)
{
	CHECKPOINTER(a_ptValue);
	ConfigValueInit(*a_ptValue);

	ObjectLock cLock(this);

	CComBSTR bstrSubID;
	AItems::const_iterator iItem;
	if (!ParseID(a_bstrID, iItem, bstrSubID))
		return E_RW_ITEMNOTFOUND;

	if (bstrSubID.m_str == NULL)
	{
		// this item
		*a_ptValue = CConfigValue(iItem->strName.c_str()).Detach();
		return S_OK;
	}
	else
	{
		// subconfig item
		return iItem->pConfig->ItemValueGet(bstrSubID, a_ptValue);
	}
}

STDMETHODIMP CSubConfigVector::ItemValuesSet(ULONG a_nCount, BSTR* a_aIDs, const TConfigValue* a_atValues)
{
	CHECKPOINTER(a_aIDs);
	CHECKPOINTER(a_atValues);

	{
	ObjectLock cLock(this);

	m_bUpdating = true;

	// now set values of subitems
	CAutoVectorPtr<BSTR> aSubIDs(new BSTR[a_nCount]); // temp buffer of sub-items
	CAutoVectorPtr<TConfigValue> aSubVals(new TConfigValue[a_nCount]);
	AItems::iterator i;
	for (i = m_aItems.begin(); i != m_aItems.end(); i++)
	{
		wchar_t szTmp[16];
		swprintf(szTmp, L"%08x", i - m_aItems.begin());
		LPCOLESTR pszActID = szTmp;
		size_t nActID = 8;
		ULONG nSubCount = 0;
		ULONG j;
		for (j = 0; j < a_nCount; j++)
		{
			LPCOLESTR pszChgID = a_aIDs[j];
			if (wcsncmp(pszActID, pszChgID, nActID) == 0 && pszChgID[nActID] == L'\\')
			{
				aSubIDs[nSubCount] = SysAllocString(pszChgID+nActID+1);
				aSubVals[nSubCount++] = a_atValues[j]; // shallow copy (no need to free the item)
			}
			else if (wcscmp(pszActID, pszChgID) == 0 && m_bEditableNames && a_atValues[j].eTypeID == ECVTString)
			{
				i->strName = a_atValues[j].bstrVal;
			}
		}
		if (nSubCount)
		{
			i->pConfig->ItemValuesSet(nSubCount, aSubIDs, aSubVals);
			for (j = 0; j < nSubCount; j++)
			{
				SysFreeString(aSubIDs[j]);
			}
		}
	}

	m_bUpdating = false;
	}

	if (m_bChanged)
	{
		// only send the notification if something has changed
		Fire_Notify(NULL);
	}

	return S_OK;
}

STDMETHODIMP CSubConfigVector::ItemGetUIInfo(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
{
	CHECKPOINTER(a_ppItemInfo);
	*a_ppItemInfo = NULL;

	ObjectLock cLock(this);

	CComBSTR bstrSubID;
	AItems::const_iterator iItem;
	if (!ParseID(a_bstrID, iItem, bstrSubID))
		return E_RW_ITEMNOTFOUND;

	if (bstrSubID.m_str == NULL)
	{
		if (m_bEditableNames)
		{
			CComObject<CConfigVctItem>* pItem = NULL;
			CComObject<CConfigVctItem>::CreateInstance(&pItem);
			pItem->Init(_SharedStringTable.GetStringAuto(IDS_SCVECT_NAME), _SharedStringTable.GetStringAuto(IDS_SCVECT_DESC));
			pItem->AddRef();
			HRESULT hRes = pItem->QueryInterface(a_iidInfo, a_ppItemInfo);
			pItem->Release();
			return hRes;
		}
		else if (m_pCustomName)
		{
			CComObject<CConfigVctItem>* pItem = NULL;
			CComObject<CConfigVctItem>::CreateInstance(&pItem);
			pItem->Init(_SharedStringTable.GetStringAuto(IDS_SCVECT_NAME), _SharedStringTable.GetStringAuto(IDS_SCVECT_DESC), m_pCustomName, iItem->pConfig, iItem-m_aItems.begin());
			pItem->AddRef();
			HRESULT hRes = pItem->QueryInterface(a_iidInfo, a_ppItemInfo);
			pItem->Release();
			return hRes;
		}
		else
		{
			return E_FAIL;
		}
		// this item
//		*a_ptValue = CConfigValue(iItem->strName.c_str()).Detach();
		return E_FAIL;//S_OK;
	}
	else
	{
		// subconfig item
		return iItem->pConfig->ItemGetUIInfo(bstrSubID, a_iidInfo, a_ppItemInfo);
	}
}

STDMETHODIMP CSubConfigVector::SubConfigGet(BSTR a_bstrID, IConfig** a_ppSubConfig)
{
	CHECKPOINTER(a_ppSubConfig);
	*a_ppSubConfig = NULL;

	ObjectLock cLock(this);

	CComBSTR bstrSubID;
	AItems::const_iterator iItem;
	if (!ParseID(a_bstrID, iItem, bstrSubID))
		return E_RW_ITEMNOTFOUND;

	if (bstrSubID.m_str == NULL)
	{
		// this item
		(*a_ppSubConfig = iItem->pConfig)->AddRef();
		return S_OK;
	}
	else
	{
		// subconfig item
		return iItem->pConfig->SubConfigGet(bstrSubID, a_ppSubConfig);
	}
}

STDMETHODIMP CSubConfigVector::DuplicateCreate(IConfig** a_ppCopiedConfig)
{
	CHECKPOINTER(a_ppCopiedConfig);
	*a_ppCopiedConfig = NULL;

	ObjectLock cLock(this);

	HRESULT hRes;
	CComObject<CSubConfigVector> *pCopy;
	hRes = CComObject<CSubConfigVector>::CreateInstance(&pCopy);
	if (FAILED(hRes))
		return hRes;

	CSubConfigVector* pAccess = pCopy; // the CComObject... is not accessible from here
	pAccess->m_pPattern = m_pPattern;
	pAccess->m_bEditableNames = m_bEditableNames;
	pAccess->m_pCustomName = m_pCustomName;
	AItems::const_iterator i;
	ULONG j;
	for (j = 0, i = m_aItems.begin(); i != m_aItems.end(); i++, j++)
	{
		SItem sTmp;
		sTmp.strName = i->strName;
		i->pConfig->DuplicateCreate(&sTmp.pConfig);
		sTmp.pConfig->ObserverIns(pAccess->ObserverGet(), 0/*j*/);
		pAccess->m_aItems.push_back(sTmp);
	}

	return pCopy->QueryInterface<IConfig>(a_ppCopiedConfig);
}

STDMETHODIMP CSubConfigVector::CopyFrom(IConfig* a_pSource, BSTR a_bstrIDPrefix)
{
	if (a_pSource == NULL)
		return S_FALSE;

	size_t nIDPrefix = a_bstrIDPrefix ? SysStringLen(a_bstrIDPrefix) : 0;

	ObjectLock cLock(this);

	m_bUpdating = true;

	AItems::iterator i;
	for (i = m_aItems.begin(); i != m_aItems.end(); i++)
	{
		wchar_t szTmp[16];
		swprintf(szTmp, L"%08x", i - m_aItems.begin());

		CComBSTR bstrSubIDPrefix;
		size_t const nIDLen = 8;
		if (nIDLen < nIDPrefix)
		{
			if (wcsncmp(szTmp, a_bstrIDPrefix, nIDLen) == 0 && a_bstrIDPrefix[nIDLen] == L'\\')
				bstrSubIDPrefix = a_bstrIDPrefix+nIDLen+1;
			else
				continue;
		}
		else
		{
			if (nIDPrefix && wcsncmp(szTmp, a_bstrIDPrefix, nIDPrefix) != 0)
				continue;
		}

		CComBSTR bstrTmp(szTmp);
		CConfigValue cVal;
		if (SUCCEEDED(a_pSource->ItemValueGet(bstrTmp, &cVal)))
		{
			if (m_bEditableNames && cVal.TypeGet() == ECVTString && i->strName != cVal.operator BSTR())
			{
				i->strName = cVal.operator BSTR();
				m_bChanged = true;
			}
		}

		CComPtr<IConfig> pSub;
		a_pSource->SubConfigGet(bstrTmp, &pSub);
		if (pSub)
		{
			i->pConfig->CopyFrom(pSub, bstrSubIDPrefix);
		}
	}

	m_bUpdating = false;

	if (m_bChanged)
	{
		// only send the notification if something has changed
		Fire_Notify(NULL);
	}

	return S_OK;
}

STDMETHODIMP CSubConfigVector::Swap(ULONG a_nIndex1, ULONG a_nIndex2)
{
	try
	{
		ObjectLock cLock(this);
		if (a_nIndex1 >= m_aItems.size() || a_nIndex2 >= m_aItems.size())
			return E_INVALIDARG;
		swap(m_aItems[a_nIndex1].strName, m_aItems[a_nIndex2].strName);
		swap(m_aItems[a_nIndex1].pConfig.p, m_aItems[a_nIndex2].pConfig.p);
		Fire_Notify(NULL);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSubConfigVector::Move(ULONG a_nIndexSrc, ULONG a_nIndexDst)
{
	try
	{
		ObjectLock cLock(this);
		if (a_nIndexSrc >= m_aItems.size() || a_nIndexSrc >= m_aItems.size())
			return E_INVALIDARG;
		LONG nDelta = a_nIndexSrc < a_nIndexDst ? 1 : -1;
		for (ULONG i = a_nIndexSrc; i != a_nIndexDst; i += nDelta)
		{
			swap(m_aItems[i].strName, m_aItems[i+nDelta].strName);
			swap(m_aItems[i].pConfig.p, m_aItems[i+nDelta].pConfig.p);
		}
		Fire_Notify(NULL);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSubConfigVector::ControllerSet(const TConfigValue* a_ptValue)
{
	CHECKPOINTER(a_ptValue);

	try
	{
		if (a_ptValue->eTypeID != ECVTInteger) // wrong type
			return E_INVALIDARG;
		if (a_ptValue->iVal < 0) // negative size
			return E_INVALIDARG;

		CComPtr<IEnumStringsInit> pESInit;
		RWCoCreateInstance(pESInit, __uuidof(EnumStrings));

		ObjectLock cLock(this);

		if (m_aItems.size() > ULONG(a_ptValue->iVal))
		{
			// delete some items
			AItems::const_iterator i;
			ULONG j;
			for (j = a_ptValue->iVal, i = m_aItems.begin()+a_ptValue->iVal; i != m_aItems.end(); i++, j++)
			{
				i->pConfig->ObserverDel(ObserverGet(), 0/*j*/);
				TCHAR szTmp[16];
				_stprintf(szTmp, _T("%08x"), j);
				pESInit->Insert(CComBSTR(szTmp));
				szTmp[8] = _T('\\');
				szTmp[9] = _T('\0');
				InsertSubCfgIDs(pESInit, *i, szTmp);
			}
			m_aItems.resize(a_ptValue->iVal);
		}
		else if (m_aItems.size() < ULONG(a_ptValue->iVal))
		{
			USES_CONVERSION;
			// insert more items
			AItems::const_iterator i;
			for (LONG j = m_aItems.size(); j < a_ptValue->iVal; j++)
			{
				TCHAR szTmp[16];
				_stprintf(szTmp, _T("%08x"), j);
				SItem sTmp;
				sTmp.strName = T2CW(szTmp);
				m_pPattern->DuplicateCreate(&sTmp.pConfig);
				m_aItems.push_back(sTmp);
				sTmp.pConfig->ObserverIns(ObserverGet(), 0/*j*/);
				pESInit->Insert(CComBSTR(szTmp));
				szTmp[8] = _T('\\');
				szTmp[9] = _T('\0');
				InsertSubCfgIDs(pESInit, sTmp, szTmp);
			}
			m_aItems.resize(a_ptValue->iVal);
		}

		ULONG nChgSize = 0;
		pESInit->Size(&nChgSize);
		if (nChgSize > 0)
		{
			Fire_Notify(pESInit.p);
		}

		return S_OK;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSubConfigVector::Init(BOOL a_bEditableNames, IConfig* a_pPattern)
{
	CHECKPOINTER(a_pPattern);

	m_pPattern = a_pPattern;
	m_bEditableNames = static_cast<bool>(a_bEditableNames);

	return S_OK;
}

STDMETHODIMP CSubConfigVector::InitName(IVectorItemName* a_pCustomName, IConfig* a_pPattern)
{
	CHECKPOINTER(a_pPattern);

	m_pCustomName = a_pCustomName;
	m_pPattern = a_pPattern;
	m_bEditableNames = false;

	return S_OK;
}

HRESULT CSubConfigVector::OwnerNotify(TCookie a_tCookie, IUnknown* a_pChangedParam)
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

bool CSubConfigVector::ParseID(const wchar_t* a_pszID, AItems::const_iterator& a_iItem, CComBSTR& a_bstrSubID) const
{
	if (a_pszID == NULL)
		return false;

	size_t nLen = wcslen(a_pszID);
	if (nLen < 8)
		return false;

	ULONG nVal = 0;
	int i;
	for (i = 0; i < 8; i++)
	{
		if (a_pszID[i] >= L'0' && a_pszID[i] <= L'9')
		{
			nVal = (nVal<<4) + a_pszID[i] - L'0';
		}
		else if (a_pszID[i] >= L'a' && a_pszID[i] <= L'f')
		{
			nVal = (nVal<<4) + a_pszID[i] - L'a' + 10;
		}
		else
		{
			return false;
		}
	}

	if (nLen == 8)
	{
		if (m_aItems.size() > nVal)
		{
			a_iItem = m_aItems.begin()+nVal;
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (a_pszID[8] == L'\\')
	{
		a_bstrSubID = a_pszID+9;
		if (m_aItems.size() > nVal)
		{
			a_iItem = m_aItems.begin()+nVal;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void CSubConfigVector::InsertSubCfgIDs(IEnumStringsInit* a_pESInit, const SItem& a_sItem, LPCTSTR a_pszPrefix)
{
	CComPtr<IEnumStrings> pESSub;
	a_sItem.pConfig->ItemIDsEnum(&pESSub);

	USES_CONVERSION;
	LPCWSTR pszPrefix = T2CW(a_pszPrefix);

	CComBSTR bstrSubID;
	ULONG i;
	for (i = 0; SUCCEEDED(pESSub->Get(i, &bstrSubID)); i++, bstrSubID.Empty())
	{
		CComBSTR bstrTmp(pszPrefix);
		bstrTmp += bstrSubID;
		a_pESInit->Insert(bstrTmp);
	}
}
