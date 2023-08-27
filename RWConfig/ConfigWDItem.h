
#pragma once

#include "RWConfig.h"

struct IConfigWDControl : public IUnknown
{
	virtual IConfigWDControl* Clone(IConfig* a_pNewParent) const = 0;
	virtual bool IsValid(const TConfigValue& a_tValue) const = 0;
	virtual void ParentDestroyed() = 0;
	virtual void DependantsMerge(set<BSTR>& a_xOutput) const = 0;
	virtual void DependantInsert(BSTR a_bstrID) = 0;
	virtual void SuperiorsGet(set<CComBSTR>& a_aSuperiors) const = 0;
protected:
	virtual IConfig* ParentConfigGet() const = 0;
};

template<class TConfigItemBase>
class CConfigWDItemImpl :
	public TConfigItemBase,
	public IConfigWDControl,
	public ILocalizedString
{
public:
	CConfigWDItemImpl() : m_pParent(NULL)
	{
	}

protected:
	void Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, TConfigValue const& a_tDefault, IConfig* a_pParent)
	{
		ATLASSERT(a_pParent);
		m_pName = a_pName;
		m_pDesc = a_pDesc;
		m_cDefault = a_tDefault;
		m_pParent = a_pParent;
	}
	void InitCloned(const CConfigWDItemImpl<TConfigItemBase>& a_cOrig, IConfig* a_pParent)
	{
		m_pName = a_cOrig.m_pName;
		m_pDesc = a_cOrig.m_pDesc;
		m_cDefault = a_cOrig.m_cDefault;
		m_pParent = a_pParent;
		m_aDependants.insert(a_cOrig.m_aDependants.begin(), a_cOrig.m_aDependants.end());
	}

	// some IConfigItem methods
public:
	STDMETHOD(NameGet)(ILocalizedString** a_ppName, ILocalizedString** a_ppHelpText)
	{
		if (a_ppName)
		{
			if ((*a_ppName = m_pName) != NULL)
				(*a_ppName)->AddRef();
			else
				(*a_ppName = this)->AddRef();
		}
		if (a_ppHelpText)
		{
			if ((*a_ppHelpText = m_pDesc) != NULL)
				(*a_ppHelpText)->AddRef();
			else
				(*a_ppHelpText = this)->AddRef();
		}
		return S_OK;
	}

	STDMETHOD(ValueIsValid)(const TConfigValue* a_ptValue)
	{
		if (ParentConfigGet() == NULL)
			return E_FAIL;

		if (a_ptValue == NULL)
			return S_FALSE;

		return IsValid(*a_ptValue) ? S_OK : S_FALSE;
	}
	STDMETHOD(Default)(TConfigValue* a_ptValue)
	{
		a_ptValue->eTypeID = ECVTEmpty;
		*a_ptValue = ConfigValueCopy(m_cDefault);
		return S_OK;
	}

	// some IConfigWDControl methods
public:
	void DependantsMerge(set<BSTR>& a_xOutput) const
	{
		a_xOutput.insert(m_aDependants.begin(), m_aDependants.end());
	}
	void DependantInsert(BSTR a_bstrID)
	{
		m_aDependants.insert(a_bstrID);
	}

	// dummy ILocalizedString methods
private:
	STDMETHOD(Get)(BSTR* a_pbstrString) { return GetLocalized(0, a_pbstrString); }
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
	{
		*a_pbstrString = NULL;
		*a_pbstrString = SysAllocString(L"");
		return S_FALSE;
	}

protected:
	IConfig* ParentConfigGet() const
	{
		return m_pParent;
	}
	void ParentDestroyed()
	{
		m_pParent = NULL;
	}

private:
	CComPtr<ILocalizedString> m_pName;
	CComPtr<ILocalizedString> m_pDesc;
	IConfig* m_pParent;
	set<CComBSTR> m_aDependants;
	CConfigValue m_cDefault;
};
