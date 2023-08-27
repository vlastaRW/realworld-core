
#pragma once

#include "ConfigWDItem.h"

class CConfigItemConditions;

class ATL_NO_VTABLE CConfigWDItemSimple : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CConfigWDItemImpl<IConfigItemSimple>
{
public:
	CConfigWDItemSimple() : m_pConditions(NULL)
	{
	}
	~CConfigWDItemSimple();

	BEGIN_COM_MAP(CConfigWDItemSimple)
		COM_INTERFACE_ENTRY(IConfigItem)
		COM_INTERFACE_ENTRY(IConfigItemSimple)
	END_COM_MAP()

	void Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, TConfigValue const& a_tDefault, IConfig* a_pParent, EConfigValueType a_eType, CConfigItemConditions* a_pConditions);

	// specific IConfigItem methods
public:
	STDMETHOD(ValueGetName)(const TConfigValue* a_ptValue, ILocalizedString** a_ppName);

	// IConfigItemSimple methods
public:
	STDMETHOD(IsEnabled)();

	// IConfigWDAccess methods
private:
	IConfigWDControl* Clone(IConfig* a_pNewParent) const;
	bool IsValid(const TConfigValue& a_tValue) const;
	void SuperiorsGet(set<CComBSTR>& a_aSuperiors) const;

private:
	EConfigValueType m_eType;
	CConfigItemConditions* m_pConditions;
};

