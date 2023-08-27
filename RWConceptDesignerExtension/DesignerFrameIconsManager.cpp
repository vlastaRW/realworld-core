// DesignerFrameIconsManager.cpp : Implementation of CDesignerFrameIconsManager

#include "stdafx.h"
#include "DesignerFrameIconsManager.h"
#include <PlugInCache.h>


// CDesignerFrameIconsManager

void CDesignerFrameIconsManager::Init()
{
	ObjectLock cLock(this);

	ULONG const nPluginTimeStamp = CPlugInEnumerator::GetCategoryTimestamp(CATID_DesignerFrameIcons);

	if (m_bInitialized)
	{
		if (nPluginTimeStamp != m_nPluginTimeStamp)
		{
			m_bInitialized = false;
		}
		else
		{
			for (CTimeStamps::const_iterator i = m_cTimeStamps.begin(); i != m_cTimeStamps.end(); ++i)
			{
				ULONG tmp = i->second;
				i->first->TimeStamp(&tmp);
				if (tmp != i->second)
				{
					m_bInitialized = false;
					break;
				}
			}
		}
	}

	if (m_bInitialized)
		return;

	CCache cCache;
	CTimeStamps cTimeStamps;

	std::map<CATID, CComPtr<IDesignerFrameIcons>, CPlugInEnumerator::lessCATID> plugins;
	CPlugInEnumerator::GetCategoryPlugInMap<IDesignerFrameIcons>(CATID_DesignerFrameIcons, plugins);
	for (std::map<CATID, CComPtr<IDesignerFrameIcons>, CPlugInEnumerator::lessCATID>::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
	{
		if (i->second == NULL)
			continue;
		try
		{
			ULONG nTimeStamp = 0; // this is not thread safe (client may change during enumeration (EnumIconIDs should return the value)
			i->second->TimeStamp(&nTimeStamp);
			cTimeStamps[i->second] = nTimeStamp;
			CComPtr<IEnumGUIDs> pIconIDs;
			i->second->EnumIconIDs(&pIconIDs);
			GUID tGUID;
			for (ULONG j = 0; SUCCEEDED(pIconIDs->Get(j, &tGUID)); j++)
				cCache[tGUID] = i->second;
		}
		catch (...)
		{
			// one plug-in failed -> continue TODO: log
		}
	}
	std::swap(m_cTimeStamps, cTimeStamps);
	std::swap(m_cCache, cCache);
	m_nPluginTimeStamp = nPluginTimeStamp;
	++m_nTimeStamp;
	m_bInitialized = true;
}

STDMETHODIMP CDesignerFrameIconsManager::TimeStamp(ULONG* a_pTimeStamp)
{
	try
	{
		*a_pTimeStamp = 0;
		return S_OK;
	}
	catch (...)
	{
		return a_pTimeStamp == NULL ? E_POINTER: E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameIconsManager::EnumIconIDs(IEnumGUIDs** a_ppIDs)
{
	try
	{
		*a_ppIDs = NULL;
		Init();

		CComPtr<IEnumGUIDsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumGUIDs));
		if (FAILED(pTmp->Insert(GUID_NULL)))
			return E_FAIL;

		CCache::const_iterator i;
		for (i = m_cCache.begin(); i != m_cCache.end(); i++)
		{
			if (FAILED(pTmp->Insert(i->first)))
				return E_FAIL;
		}

		*a_ppIDs = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppIDs == NULL ? E_POINTER: E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameIconsManager::GetIcon(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		Init();

		CCache::const_iterator iItem = m_cCache.find(a_tIconID);
		if (iItem != m_cCache.end())
		{
			return iItem->second->GetIcon(a_tIconID, a_nSize, a_phIcon);
		}
		else if (IsEqualGUID(a_tIconID, GUID_NULL))
		{
			// empty icon
			DWORD nXOR = a_nSize * a_nSize << 2;
			DWORD nAND = a_nSize * ((((a_nSize + 7) >> 3) + 3) & 0xfffffffc);
			CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR + nAND + sizeof BITMAPINFOHEADER]);
			ZeroMemory(pIconRes.m_p, nXOR + sizeof BITMAPINFOHEADER);
			FillMemory(pIconRes.m_p + nXOR + sizeof BITMAPINFOHEADER, nAND, 0xff);
			BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
			pHead->biSize = sizeof(BITMAPINFOHEADER);
			pHead->biWidth = a_nSize;
			pHead->biHeight = a_nSize << 1;
			pHead->biPlanes = 1;
			pHead->biBitCount = 32;
			pHead->biCompression = BI_RGB;
			pHead->biSizeImage = nXOR + nAND;
			pHead->biXPelsPerMeter = 0;
			pHead->biYPelsPerMeter = 0;
			*a_phIcon = CreateIconFromResourceEx(pIconRes.m_p, nXOR + nAND + sizeof BITMAPINFOHEADER, TRUE, 0x00030000, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}

		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameIconsManager::GetValueName(const TConfigValue *a_pValue, ILocalizedString **a_ppName)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDesignerFrameIconsManager::Size(ULONG *a_pnSize)
{
	try
	{
		*a_pnSize = 0;

		CComPtr<IEnumGUIDs> pIDs;
		if (FAILED(EnumIconIDs(&pIDs)))
			return E_FAIL;

		return pIDs->Size(a_pnSize);
	}
	catch (...)
	{
		return a_pnSize == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameIconsManager::Get(ULONG a_nIndex, TConfigValue *a_ptItem)
{
	try
	{
		a_ptItem->eTypeID = ECVTEmpty;

		CComPtr<IEnumGUIDs> pIDs;
		if (FAILED(EnumIconIDs(&pIDs)))
			return E_FAIL;

		if (FAILED(pIDs->Get(a_nIndex, &a_ptItem->guidVal)))
			return E_FAIL;
		a_ptItem->eTypeID = ECVTGUID;

		return S_OK;
	}
	catch (...)
	{
		return a_ptItem == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameIconsManager::GetMultiple(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue *a_atItems)
{
	try
	{
		CComPtr<IEnumGUIDs> pIDs;
		if (FAILED(EnumIconIDs(&pIDs)))
			return E_FAIL;

		while (a_nCount > 0)
		{
			if (FAILED(pIDs->Get(a_nIndexFirst, &a_atItems->guidVal)))
				return E_FAIL;
			a_atItems->eTypeID = ECVTGUID;
			++a_atItems;
			++a_nIndexFirst;
			--a_nCount;
		}

		return S_OK;
	}
	catch (...)
	{
		return a_atItems == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

