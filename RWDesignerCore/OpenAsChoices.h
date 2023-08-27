
#pragma once

#include <PlugInCache.h>
#include "ConfigIDsApp.h"


struct COpenAsChoicesHelper
{
	COpenAsChoicesHelper() : m_bInitialized(false)
	{
	}

	void GetBuilders(IConfig* a_pAppConfig, IEnumUnknownsInit* a_pEnum)
	{
		InitChoices(a_pAppConfig);
		for (CBuilders::const_iterator i = m_cBuilders.begin(); i != m_cBuilders.end(); ++i)
		{
			a_pEnum->Insert(*i);
		}
	}
	void GetBuilder(IConfig* a_pAppConfig, ULONG a_nIndex, IDocumentBuilder** a_ppBuilder)
	{
		InitChoices(a_pAppConfig);
		if (a_nIndex < m_cBuilders.size())
			(*a_ppBuilder = m_cBuilders[a_nIndex])->AddRef();
	}
	ULONG BuilderCount(IConfig* a_pAppConfig)
	{
		InitChoices(a_pAppConfig);
		return m_cBuilders.size();
	}

	void InitChoices(IConfig* const pCfg)
	{
		if (m_bInitialized)
			return;
		CConfigValue cLayouts;
		pCfg->ItemValueGet(CComBSTR(CFGID_VIEWPROFILES), &cLayouts);
		std::vector<GUID> cLayFact;
		bool bNullGuid = false;
		for (LONG i = 0; i < cLayouts.operator LONG(); ++i)
		{
			OLECHAR szID[128];
			swprintf(szID, L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DOCBUILDER);
			CConfigValue cBuilder;
			pCfg->ItemValueGet(CComBSTR(szID), &cBuilder);
			if (IsEqualGUID(cBuilder, GUID_NULL))
			{
				bNullGuid = true;
				continue;
			}
			bool bAdd = true;
			for (std::vector<GUID>::const_iterator i = cLayFact.begin(); i != cLayFact.end(); ++i)
			{
				if (IsEqualGUID(*i, cBuilder))
				{
					bAdd = false;
					break;
				}
			}
			if (bAdd)
				cLayFact.push_back(cBuilder);
		}
		CBuilders cBuilders;
		if (cLayFact.empty() && bNullGuid)
		{
			// no layout is using explicit layout, add all builders
			CComPtr<IPlugInCache> pPIC;
			RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
			CComPtr<IEnumUnknowns> pBuilders;
			pPIC->InterfacesEnum(CATID_DocumentBuilder, __uuidof(IDocumentBuilder), 0, &pBuilders, NULL);
			ULONG nBuilders = 0;
			if (pBuilders) pBuilders->Size(&nBuilders);
			for (ULONG i = 0; i < nBuilders; ++i)
			{
				CComPtr<IDocumentBuilder> pBuilder;
				pBuilders->Get(i, __uuidof(IDocumentBuilder), reinterpret_cast<void**>(&pBuilder));
				if (pBuilder) cBuilders.push_back(pBuilder);
			}
		}
		for (std::vector<GUID>::const_iterator i = cLayFact.begin(); i != cLayFact.end(); ++i)
		{
			CComPtr<IDocumentBuilder> pBuilder;
			RWCoCreateInstance(pBuilder, *i);
			if (pBuilder) cBuilders.push_back(pBuilder);
		}
		bool bSwapped = true;
		while (bSwapped)
		{
			bSwapped = false;
			for (size_t j = 0; j < cBuilders.size()-1; ++j)
			{
				ULONG nP1 = 0;
				cBuilders[j]->Priority(&nP1);
				ULONG nP2 = 0;
				cBuilders[j+1]->Priority(&nP2);
				if (nP1 < nP2)
				{
					std::swap(cBuilders[j].p, cBuilders[j+1].p);
					bSwapped = true;
				}
			}
		}

		std::swap(cBuilders, m_cBuilders);
	}

private:
	typedef std::vector<CComPtr<IDocumentBuilder> > CBuilders;

	CBuilders m_cBuilders;
	bool m_bInitialized;
};

