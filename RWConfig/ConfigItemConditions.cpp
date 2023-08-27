
#include "stdafx.h"
#include "ConfigItemConditions.h"
#include "RWConfig.h"

CConfigItemConditions::CConfigItemConditions(const TConfigOptionCondition* a_aConditions, ULONG a_nConditions) :
	m_nConditions(a_nConditions), m_aConditions(a_nConditions ? new TConfigOptionCondition[a_nConditions] : NULL)
{
	ULONG i;
	for (i = 0; i < m_nConditions; i++)
	{
		m_aConditions[i].bstrID = SysAllocString(a_aConditions[i].bstrID);
		m_aConditions[i].eConditionType = a_aConditions[i].eConditionType;
		m_aConditions[i].tValue = ConfigValueCopy(a_aConditions[i].tValue);
	}
}

CConfigItemConditions::CConfigItemConditions(const CConfigItemConditions& a_cOrig) :
	m_nConditions(a_cOrig.m_nConditions), m_aConditions(a_cOrig.m_nConditions ? new TConfigOptionCondition[a_cOrig.m_nConditions] : NULL)
{
	ULONG i;
	for (i = 0; i < m_nConditions; i++)
	{
		m_aConditions[i].bstrID = SysAllocString(a_cOrig.m_aConditions[i].bstrID);
		m_aConditions[i].eConditionType = a_cOrig.m_aConditions[i].eConditionType;
		m_aConditions[i].tValue = ConfigValueCopy(a_cOrig.m_aConditions[i].tValue);
	}
}

CConfigItemConditions::~CConfigItemConditions()
{
	ULONG i;
	for (i = 0; i < m_nConditions; i++)
	{
		SysFreeString(m_aConditions[i].bstrID);
		ConfigValueClear(m_aConditions[i].tValue);
	}
	delete[] m_aConditions;
}

bool CConfigItemConditions::Test(IConfig* a_pConfig) const
{
	if (a_pConfig == NULL)
		return false;

	TConfigOptionCondition* i;
	for (i = m_aConditions; i < m_aConditions+m_nConditions; i++)
	{
		CConfigValue cVal1;
		ATLVERIFY(SUCCEEDED(a_pConfig->ItemValueGet(i->bstrID, &cVal1)));
		CConfigValue cVal2(i->tValue);

		switch (i->eConditionType)
		{
		case ECOCEqual:
			if (cVal1 != cVal2)
				return false;
			break;
		case ECOCNotEqual:
			if (cVal1 == cVal2)
				return false;
			break;
		case ECOCLess:
			if (cVal2 < cVal1 || cVal1 == cVal2)
				return false;
            break;
		case ECOCLessEqual:
			if (cVal2 < cVal1)
				return false;
			break;
		case ECOCGreater:
			if (cVal1 < cVal2 || cVal1 == cVal2)
				return false;
			break;
		case ECOCGreaterEqual:
			if (cVal1 < cVal2)
				return false;
			break;
		default:
			ATLASSERT(0);
		}
	}

	return true;
}

void CConfigItemConditions::SuperiorsGet(set<CComBSTR>& a_xSuperiors) const
{
	TConfigOptionCondition* i;
	for (i = m_aConditions; i < m_aConditions+m_nConditions; i++)
	{
		a_xSuperiors.insert(i->bstrID);
	}
}
