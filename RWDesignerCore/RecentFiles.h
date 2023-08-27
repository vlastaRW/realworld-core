
#pragma once

#include <SharedStringTable.h>
#include "RWProcessing.h"
#include <PortablePath.h>


struct SViewStateInfo
{
	CComPtr<ISharedState> p;
	int nActNotifying;
};
typedef std::map<std::wstring, SViewStateInfo> CViewStates;

namespace RecentFiles
{
	static const OLECHAR CFGID_RECENTFILES[] = L"RecentFiles";
	static const OLECHAR CFGID_LAYOUTNAME[] = L"LayoutName";
	static const OLECHAR CFGID_BUILDERID[] = L"BuilderID";
	static const OLECHAR CFGID_STATES[] = L"States";
	static const OLECHAR CFGID_STATEID[] = L"StateID";
	static const OLECHAR CFGID_STATEVAL[] = L"StateVal";

	enum
	{
		MAXIMUM_COUNT = 50
	};

	inline HRESULT InitConfig(IConfigWithDependencies* a_pRoot)
	{
		CComPtr<IConfigWithDependencies> pCfg1;
		RWCoCreateInstance(pCfg1, __uuidof(ConfigWithDependencies));
		CComPtr<ILocalizedString> pStr;
		pStr.Attach(_SharedStringTable.GetString(IDS_RECENTFILES_NAME));
		pCfg1->ItemInsSimple(CComBSTR(CFGID_STATEID), pStr, pStr, CConfigValue(GUID_NULL), NULL, 0, NULL);
		pCfg1->ItemInsSimple(CComBSTR(CFGID_STATEVAL), pStr, pStr, CConfigValue(L""), NULL, 0, NULL);
		pCfg1->Finalize(NULL);

		CComPtr<ISubConfigVector> pStatesCfg;
		RWCoCreateInstance(pStatesCfg, __uuidof(SubConfigVector));
		pStatesCfg->Init(TRUE, pCfg1);

		CComPtr<IConfigWithDependencies> pCfg2;
		RWCoCreateInstance(pCfg2, __uuidof(ConfigWithDependencies));
		pCfg2->ItemInsSimple(CComBSTR(CFGID_STATES), pStr, pStr, CConfigValue(0L), pStatesCfg, 0, NULL);
		pCfg2->ItemInsSimple(CComBSTR(CFGID_LAYOUTNAME), pStr, pStr, CConfigValue(L""), NULL, 0, NULL);
		pCfg2->ItemInsSimple(CComBSTR(CFGID_BUILDERID), pStr, pStr, CConfigValue(GUID_NULL), NULL, 0, NULL);
		pCfg2->Finalize(NULL);

		CComPtr<ISubConfigVector> pSubCfg;
		RWCoCreateInstance(pSubCfg, __uuidof(SubConfigVector));
		pSubCfg->Init(TRUE, pCfg2);

		a_pRoot->ItemInsRanged(CComBSTR(CFGID_RECENTFILES), _SharedStringTable.GetStringAuto(IDS_RECENTFILES_NAME), _SharedStringTable.GetStringAuto(IDS_RECENTFILES_HELP), CConfigValue(0L), pSubCfg.p, CConfigValue(0L), CConfigValue(LONG(MAXIMUM_COUNT)), CConfigValue(1L), 0, NULL);

		return S_OK;
	}

	inline void SetRecentFileProps(IConfig* a_pConfig, LPCOLESTR a_pszLayoutName, GUID const& a_tBuilderID, CViewStates const& a_cStates)
	{
		std::vector<BSTR> aIDs;
		std::vector<TConfigValue> aVals;

		try
		{
			int x = 0;
			CComBSTR bstrID;
			TConfigValue tVal;
			OLECHAR sz[64];

			bstrID = CFGID_LAYOUTNAME; aIDs.push_back(bstrID); bstrID.Detach();
			tVal.eTypeID = ECVTString;
			bstrID = a_pszLayoutName; tVal.bstrVal = bstrID;
			aVals.push_back(tVal); bstrID.Detach();

			bstrID = CFGID_BUILDERID; aIDs.push_back(bstrID); bstrID.Detach();
			tVal.eTypeID = ECVTGUID;
			tVal.guidVal = a_tBuilderID;
			aVals.push_back(tVal); bstrID.Detach();

			for (CViewStates::const_iterator i = a_cStates.begin(); i != a_cStates.end(); ++i)
			{
				CLSID tCLSID = GUID_NULL;
				CComBSTR bstrData;
				if (i->second.p && SUCCEEDED(i->second.p->CLSIDGet(&tCLSID)) && !IsEqualCLSID(tCLSID, GUID_NULL) &&
					SUCCEEDED(i->second.p->ToText(&bstrData)) && bstrData.Length())
				{
					swprintf(sz, L"%s\\%08x", CFGID_STATES, x);
					bstrID = sz; aIDs.push_back(bstrID); bstrID.Detach();
					tVal.eTypeID = ECVTString;
					bstrID = i->first.c_str(); tVal.bstrVal = bstrID;
					aVals.push_back(tVal); bstrID.Detach();

					swprintf(sz, L"%s\\%08x\\%s", CFGID_STATES, x, CFGID_STATEID);
					bstrID = sz; aIDs.push_back(bstrID); bstrID.Detach();
					tVal.eTypeID = ECVTGUID;
					tVal.guidVal = tCLSID;
					aVals.push_back(tVal);

					swprintf(sz, L"%s\\%08x\\%s", CFGID_STATES, x, CFGID_STATEVAL);
					bstrID = sz; aIDs.push_back(bstrID); bstrID.Detach();
					tVal.eTypeID = ECVTString;
					tVal.bstrVal = bstrData;
					aVals.push_back(tVal);
					bstrData.Detach();

					++x;
				}
			}
			bstrID = CFGID_STATES; aIDs.push_back(bstrID); bstrID.Detach();
			tVal.eTypeID = ECVTInteger;
			tVal.iVal = x;
			aVals.push_back(tVal);

			a_pConfig->ItemValuesSet(aIDs.size(), &(aIDs[0]), &(aVals[0]));
		}
		catch (...) {}

		for (std::vector<BSTR>::iterator i = aIDs.begin(); i != aIDs.end(); ++i)
			SysFreeString(*i);
		for (std::vector<TConfigValue>::iterator i = aVals.begin(); i != aVals.end(); ++i)
			ConfigValueClear(*i);
	}

	inline bool IsTutorial(LPCOLESTR a_pszFilter)
	{
		return 0 == _wcsnicmp(L"res://", a_pszFilter, itemsof(L"res://")-1) && wcsstr(a_pszFilter, L"/TUTORIAL/");
	}

	inline HRESULT InsertRecentFile(IConfig* a_pStartDlgCfg, LPCOLESTR a_pszFilter, LPCOLESTR a_pszLayoutName, GUID const& a_tBuilderID, CViewStates const& a_cStates)
	{
		if (IsTutorial(a_pszFilter))
			return S_FALSE;

		SHAddToRecentDocs(SHARD_PATHW, a_pszFilter);

		CConfigValue cCount;
		a_pStartDlgCfg->ItemValueGet(CComBSTR(CFGID_RECENTFILES), &cCount);
		LONG nCount(cCount);

		CComPtr<IConfig> pFiles;
		a_pStartDlgCfg->SubConfigGet(CComBSTR(CFGID_RECENTFILES), &pFiles);

		CComBSTR bstrPath(a_pszFilter);
		PortablePath::Full2Portable(&(bstrPath.m_str));

		OLECHAR szTmp[32];
		LONG i;
		for (i = 0; i < nCount; i++)
		{
			swprintf(szTmp, L"%08x", i);
			CConfigValue cName;
			pFiles->ItemValueGet(CComBSTR(szTmp), &cName);
			if (_wcsicmp(cName, bstrPath.m_str) == 0)
			{
				break;
			}
		}

		if (i == nCount)
		{
			if (nCount < MAXIMUM_COUNT)
			{
				// increase the number of MRU files as there is still space
				nCount++;
				CComBSTR cCFGID_RECENTFILES(CFGID_RECENTFILES);
				a_pStartDlgCfg->ItemValuesSet(1, &(cCFGID_RECENTFILES.m_str), CConfigValue(nCount));
			}
			else
			{
				i--;
			}
		}

		// move the required portion of MRU vector
		for (; i > 0; i--)
		{
			swprintf(szTmp, L"%08x", i);
			CComBSTR bstrNameID2(szTmp);
			swprintf(szTmp, L"%08x", i-1);
			CComBSTR bstrNameID1(szTmp);
			CConfigValue cName;
			pFiles->ItemValueGet(bstrNameID1, &cName);
			pFiles->ItemValuesSet(1, &(bstrNameID2.m_str), cName);
			CComPtr<IConfig> pCfg1;
			pFiles->SubConfigGet(bstrNameID1, &pCfg1);
			CComPtr<IConfig> pCfg2;
			pFiles->SubConfigGet(bstrNameID2, &pCfg2);
			CopyConfigValues(pCfg2, pCfg1);
		}

		CComBSTR bstrHead(L"00000000");
		pFiles->ItemValuesSet(1, &bstrHead.m_str, CConfigValue(bstrPath.m_str));
		CComPtr<IConfig> pCfgHead;
		pFiles->SubConfigGet(bstrHead, &pCfgHead);
		SetRecentFileProps(pCfgHead, a_pszLayoutName, a_tBuilderID, a_cStates);

		return S_OK;
	}

	inline HRESULT UpdateRecentFile(IConfig* a_pStartDlgCfg, LPCOLESTR a_pszFilter, LPCOLESTR a_pszLayoutName, GUID const& a_tBuilderID, CViewStates const& a_cStates)
	{
		if (IsTutorial(a_pszFilter))
			return S_FALSE;

		CConfigValue cCount;
		a_pStartDlgCfg->ItemValueGet(CComBSTR(CFGID_RECENTFILES), &cCount);
		LONG nCount(cCount);

		CComPtr<IConfig> pFiles;
		a_pStartDlgCfg->SubConfigGet(CComBSTR(CFGID_RECENTFILES), &pFiles);

		CComBSTR bstrPath(a_pszFilter);
		PortablePath::Full2Portable(&(bstrPath.m_str));

		OLECHAR szTmp[32];
		LONG i;
		for (i = 0; i < nCount; i++)
		{
			swprintf(szTmp, L"%08x", i);
			CConfigValue cName;
			pFiles->ItemValueGet(CComBSTR(szTmp), &cName);
			if (_wcsicmp(cName, bstrPath.m_str) == 0)
			{
				CComPtr<IConfig> pCfgHead;
				pFiles->SubConfigGet(CComBSTR(szTmp), &pCfgHead);
				SetRecentFileProps(pCfgHead, a_pszLayoutName, a_tBuilderID, a_cStates);
				return S_OK;
			}
		}

		return S_FALSE;
	}

	inline HRESULT RemoveRecentFile(IConfig* a_pStartDlgCfg, LONG a_nIndex)
	{
		CConfigValue cCount;
		CComBSTR cCFGID_RECENTFILES(CFGID_RECENTFILES);
		a_pStartDlgCfg->ItemValueGet(cCFGID_RECENTFILES, &cCount);
		LONG nCount(cCount);

		if (a_nIndex >= nCount)
			return E_RW_INDEXOUTOFRANGE;

		if (a_nIndex != nCount-1)
		{
			CComPtr<IConfig> pFiles;
			a_pStartDlgCfg->SubConfigGet(cCFGID_RECENTFILES, &pFiles);
			CComQIPtr<IConfigVector> pVector(pFiles);
			pVector->Move(a_nIndex, nCount-1);
		}
		cCount = LONG(nCount-1);
		return a_pStartDlgCfg->ItemValuesSet(1, &(cCFGID_RECENTFILES.m_str), cCount);
	}

	inline HRESULT GetRecentFileConfig(IConfig* a_pStartDlgCfg, LPCOLESTR a_pszFilter, CComBSTR& a_bstrLayoutName, GUID& a_tBuilderID, CViewStates& a_cStates)
	{
		try
		{
			CConfigValue cCount;
			a_pStartDlgCfg->ItemValueGet(CComBSTR(CFGID_RECENTFILES), &cCount);
			if (cCount.TypeGet() != ECVTInteger)
				return E_FAIL;

			LONG nCount(cCount);

			CComPtr<IConfig> pFiles;
			a_pStartDlgCfg->SubConfigGet(CComBSTR(CFGID_RECENTFILES), &pFiles);

			CComBSTR bstrPath(a_pszFilter);
			PortablePath::Full2Portable(&(bstrPath.m_str));

			OLECHAR szTmp[64];
			LONG i;
			for (i = 0; i < nCount; i++)
			{
				swprintf(szTmp, L"%08x", i);
				CConfigValue cName;
				pFiles->ItemValueGet(CComBSTR(szTmp), &cName);
				if (_wcsicmp(cName, bstrPath.m_str) == 0)
				{
					CComPtr<IConfig> pCfg;
					pFiles->SubConfigGet(CComBSTR(szTmp), &pCfg);
					TConfigValue tVal;
					pCfg->ItemValueGet(CComBSTR(CFGID_LAYOUTNAME), &tVal);
					a_bstrLayoutName.Attach(tVal.bstrVal);
					pCfg->ItemValueGet(CComBSTR(CFGID_BUILDERID), &tVal);
					a_tBuilderID = tVal.guidVal;
					pCfg->ItemValueGet(CComBSTR(CFGID_STATES), &tVal);
					for (LONG i = 0; i < tVal.iVal; ++i)
					{
						swprintf(szTmp, L"%s\\%08x", CFGID_STATES, i);
						CConfigValue cState;
						pCfg->ItemValueGet(CComBSTR(szTmp), &cState);
						swprintf(szTmp, L"%s\\%08x\\%s", CFGID_STATES, i, CFGID_STATEID);
						CConfigValue cStateID;
						pCfg->ItemValueGet(CComBSTR(szTmp), &cStateID);
						swprintf(szTmp, L"%s\\%08x\\%s", CFGID_STATES, i, CFGID_STATEVAL);
						CConfigValue cStateVal;
						pCfg->ItemValueGet(CComBSTR(szTmp), &cStateVal);
						CComPtr<ISharedState> pState;
						RWCoCreateInstance(pState, cStateID.operator const GUID &());
						if (pState && SUCCEEDED(pState->FromText(cStateVal)))
						{
							SViewStateInfo& sInfo = a_cStates[cState.operator BSTR()];
							sInfo.p = pState;
							sInfo.nActNotifying = -1;
						}
					}
					return S_OK;
				}
			}
		}
		catch (...) {}

		return E_FAIL;
	}

	inline HRESULT GetRecentFileList(IConfig* a_pMainCfg, vector<tstring>& a_cMRUList)
	{
		CConfigValue cCount;
		a_pMainCfg->ItemValueGet(CComBSTR(CFGID_RECENTFILES), &cCount);
		LONG nCount(cCount);

		CComPtr<IConfig> pFiles;
		a_pMainCfg->SubConfigGet(CComBSTR(CFGID_RECENTFILES), &pFiles);

		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		CComBSTR bstrRoot;
		if (pAI && pAI->Portable() == S_OK)
			pAI->AppRootFolder(&bstrRoot);

		OLECHAR szTmp[32];
		LONG i;
		for (i = 0; i < nCount; i++)
		{
			swprintf(szTmp, L"%08x", i);
			CConfigValue cName;
			pFiles->ItemValueGet(CComBSTR(szTmp), &cName);
			if (bstrRoot.m_str)
			{
				CComBSTR bstr;
				bstr.Attach(cName.Detach().bstrVal);
				PortablePath::Portable2Full(&(bstr.m_str), bstrRoot.m_str);
				a_cMRUList.push_back(static_cast<LPCTSTR>(COLE2CT(bstr.m_str)));
			}
			else
			{
				a_cMRUList.push_back(static_cast<LPCTSTR>(COLE2CT(cName)));
			}
		}
		return S_OK;
	}
};