
#pragma once

#include "ConfigWDItem.h"

class CConfigItemConditions;

class ATL_NO_VTABLE CConfigWDItemOptions : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CConfigWDItemImpl<IConfigItemOptions>
{
public:
	CConfigWDItemOptions()
	{
	}
	~CConfigWDItemOptions();

	BEGIN_COM_MAP(CConfigWDItemOptions)
		COM_INTERFACE_ENTRY(IConfigItem)
		COM_INTERFACE_ENTRY(IConfigItemOptions)
	END_COM_MAP()

	void Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, TConfigValue const& a_tDefault, IConfig* a_pParent);
	void InsertOption(const TConfigValue* a_ptValue, ILocalizedString* a_pName, CConfigItemConditions* a_pConditions);

	// specific IConfigItem methods
public:
	STDMETHOD(ValueGetName)(const TConfigValue* a_ptValue, ILocalizedString** a_ppName);

	// IConfigItemOptions methods
public:
	STDMETHOD(OptionsEnum)(IEnumConfigItemOptions** a_ppOptions);

	// some IConfigWDControl methods
private:
	IConfigWDControl* Clone(IConfig* a_pNewParent) const;
	bool IsValid(const TConfigValue& a_tValue) const;
	void SuperiorsGet(set<CComBSTR>& a_aSuperiors) const;

private:
	struct SOption
	{
		TConfigValue tValue;
		ILocalizedString* pFriendlyName;
		CConfigItemConditions* pConditions;
	};
	typedef vector<SOption> AOptions;

private:
	AOptions m_aOptions;
};

