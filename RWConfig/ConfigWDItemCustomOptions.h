
#pragma once

#include "ConfigWDItem.h"
#include "EnumConfigItemOptions.h"

class ATL_NO_VTABLE CConfigWDItemCustomOptions : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CConfigWDItemImpl<IConfigItemOptions>
{
public:
	CConfigWDItemCustomOptions() : m_pConditions(NULL)
	{
	}
	~CConfigWDItemCustomOptions()
	{
		delete m_pConditions;
	}


	BEGIN_COM_MAP(CConfigWDItemCustomOptions)
		COM_INTERFACE_ENTRY(IConfigItem)
		COM_INTERFACE_ENTRY(IConfigItemOptions)
	END_COM_MAP()

	void Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, TConfigValue const& a_tDefault, IConfig* a_pParent, IConfigItemCustomOptions* a_pOptions, CConfigItemConditions* a_pConditions)
	{
		CConfigWDItemImpl<IConfigItemOptions>::Init(a_pName, a_pDesc, a_tDefault, a_pParent);
		m_pOptions = a_pOptions;
		m_pConditions = a_pConditions;
	}

	// specific IConfigItem methods
public:
	STDMETHOD(ValueGetName)(const TConfigValue* a_ptValue, ILocalizedString** a_ppName)
	{
		return m_pOptions->GetValueName(a_ptValue, a_ppName);
	}

	// IConfigItemOptions methods
public:
	STDMETHOD(OptionsEnum)(IEnumConfigItemOptions** a_ppOptions)
	{
		if (m_pConditions && !m_pConditions->Test(ParentConfigGet()))
		{
			CComObject<CEnumConfigItemOptions>* pTmp = NULL;
			CComObject<CEnumConfigItemOptions>::CreateInstance(&pTmp);
			(*a_ppOptions = pTmp)->AddRef();
			return S_OK;
		}
		(*a_ppOptions = m_pOptions.p)->AddRef();
		return S_OK;
	}

	// some IConfigWDControl methods
private:
	IConfigWDControl* Clone(IConfig* a_pNewParent) const
	{
		CComObject<CConfigWDItemCustomOptions>* pCopy = NULL;
		CComObject<CConfigWDItemCustomOptions>::CreateInstance(&pCopy);

		pCopy->InitCloned(*this, a_pNewParent);
		pCopy->m_pOptions = m_pOptions;
		pCopy->m_pConditions = new CConfigItemConditions(*m_pConditions);

		return pCopy;
	}
	bool IsValid(const TConfigValue& a_tValue) const
	{
		// is enabled ?
		if (!m_pConditions->Test(ParentConfigGet()))
			return false;

		CConfigValue cVal1(a_tValue);
		CConfigValue cVal2;
		for (ULONG i = 0; SUCCEEDED(m_pOptions->Get(i, &cVal2)); ++i)
		{
			if (cVal1 == cVal2)
			{
				return true;
			}
		}

		return false;
	}
	void SuperiorsGet(set<CComBSTR>& a_aSuperiors) const
	{
		if (m_pConditions)
			m_pConditions->SuperiorsGet(a_aSuperiors);
	}

private:
	CComPtr<IConfigItemCustomOptions> m_pOptions;
	CConfigItemConditions* m_pConditions;
};

