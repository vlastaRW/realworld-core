
#pragma once

struct TConfigOptionCondition;
struct IConfig;

class CConfigItemConditions
{
public:
	CConfigItemConditions(const TConfigOptionCondition* a_aConditions, ULONG a_nConditions);
	CConfigItemConditions(const CConfigItemConditions& a_cOrig);
	~CConfigItemConditions();

	bool Test(IConfig* a_pConfig) const;
	void SuperiorsGet(set<CComBSTR>& a_aSuperiors) const;

private:
	TConfigOptionCondition* m_aConditions;
	ULONG m_nConditions;
};