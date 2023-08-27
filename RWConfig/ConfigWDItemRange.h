
#pragma once

#include "ConfigWDItem.h"

class CConfigItemConditions;

struct IConfigItemRangeAndSimple : public IConfigItemRange, public IConfigItemSimple {};

class ATL_NO_VTABLE CConfigWDItemRange : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CConfigWDItemImpl<IConfigItemRangeAndSimple>
{
public:
	CConfigWDItemRange() : m_pConditions(NULL)
	{
	}
	~CConfigWDItemRange();

	BEGIN_COM_MAP(CConfigWDItemRange)
		COM_INTERFACE_ENTRY2(IConfigItem, IConfigItemRange)
		COM_INTERFACE_ENTRY(IConfigItemRange)
		COM_INTERFACE_ENTRY(IConfigItemSimple)
	END_COM_MAP()

	void Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, TConfigValue const& a_tDefault, IConfig* a_pParent, const TConfigValue& a_tFrom, const TConfigValue& a_tTo, const TConfigValue& a_tStep, CConfigItemConditions* a_pConditions);

	// specific IConfigItem methods
public:
	STDMETHOD(ValueGetName)(const TConfigValue* a_ptValue, ILocalizedString** a_ppName);

	// IConfigItemRange methods
public:
	STDMETHOD(RangePropsGet)(TConfigValue* a_ptFrom, TConfigValue* a_ptTo, TConfigValue* a_ptStep);

	// IConfigItemSimple methods
public:
	STDMETHOD(IsEnabled)();

	// IConfigWDAccess methods
private:
	IConfigWDControl* Clone(IConfig* a_pNewParent) const;
	bool IsValid(const TConfigValue& a_tValue) const;
	void SuperiorsGet(set<CComBSTR>& a_aSuperiors) const;

private:
	CConfigValue m_cFrom;
	CConfigValue m_cTo;
	CConfigValue m_cStep;
	CConfigItemConditions* m_pConditions;
};

