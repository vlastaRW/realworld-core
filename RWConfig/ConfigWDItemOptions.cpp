
#include "stdafx.h"
#include "ConfigWDItemOptions.h"
#include "ConfigItemConditions.h"
#include "EnumConfigItemOptions.h"

CConfigWDItemOptions::~CConfigWDItemOptions()
{
	AOptions::iterator i;
	for (i = m_aOptions.begin(); i != m_aOptions.end(); i++)
	{
		ConfigValueClear(i->tValue);
		if (i->pFriendlyName)
			i->pFriendlyName->Release();
		delete i->pConditions;
	}
}

void CConfigWDItemOptions::Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, TConfigValue const& a_tDefault, IConfig* a_pParent)
{
	CConfigWDItemImpl<IConfigItemOptions>::Init(a_pName, a_pDesc, a_tDefault, a_pParent);
}

void CConfigWDItemOptions::InsertOption(const TConfigValue* a_ptValue, ILocalizedString* a_pName, CConfigItemConditions* a_pConditions)
{
	SOption sTmp;
	sTmp.tValue	= ConfigValueCopy(*a_ptValue);
	sTmp.pFriendlyName = a_pName;
	if (a_pName) a_pName->AddRef();;
	sTmp.pConditions = a_pConditions;
	m_aOptions.push_back(sTmp);
}

IConfigWDControl* CConfigWDItemOptions::Clone(IConfig* a_pNewParent) const
{
	CComObject<CConfigWDItemOptions>* pCopy = NULL;
	CComObject<CConfigWDItemOptions>::CreateInstance(&pCopy);

	pCopy->InitCloned(*this, a_pNewParent);
	AOptions::const_iterator i;
	for (i = m_aOptions.begin(); i != m_aOptions.end(); i++)
	{
		SOption sTmp;
		sTmp.tValue = ConfigValueCopy(i->tValue);
		sTmp.pFriendlyName = i->pFriendlyName;
		if (sTmp.pFriendlyName) sTmp.pFriendlyName->AddRef();
		sTmp.pConditions = new CConfigItemConditions(*i->pConditions);
		implicit_cast<CConfigWDItemOptions*>(pCopy)->m_aOptions.push_back(sTmp);
	}

	return pCopy;
}

bool CConfigWDItemOptions::IsValid(const TConfigValue& a_tValue) const
{
	CConfigValue cVal(a_tValue);

	AOptions::const_iterator i;
	for (i = m_aOptions.begin(); i != m_aOptions.end(); i++)
	{
		if (cVal == i->tValue)
		{
			if (i->pConditions->Test(ParentConfigGet()))
				return true;
		}
	}

	return false;
}

void CConfigWDItemOptions::SuperiorsGet(set<CComBSTR>& a_aSuperiors) const
{
	AOptions::const_iterator i;
	for (i = m_aOptions.begin(); i != m_aOptions.end(); i++)
	{
		if (i->pConditions)
			i->pConditions->SuperiorsGet(a_aSuperiors);
	}
}

STDMETHODIMP CConfigWDItemOptions::ValueGetName(const TConfigValue* a_ptValue, ILocalizedString** a_ppName)
{
	CHECKPOINTER(a_ppName);
	*a_ppName = NULL;
	CHECKPOINTER(a_ptValue);

	AOptions::iterator i;
	for (i = m_aOptions.begin(); i != m_aOptions.end(); i++)
	{
		if (CConfigValue(*a_ptValue) == i->tValue)
		{
			if (i->pFriendlyName)
			{
				(*a_ppName = i->pFriendlyName)->AddRef();
				return S_OK;
			}
			else
			{
				return E_FAIL; // TODO: something
			}
		}
	}

	return E_RW_ITEMNOTFOUND;
}

STDMETHODIMP CConfigWDItemOptions::OptionsEnum(IEnumConfigItemOptions** a_ppOptions)
{
	CHECKPOINTER(a_ppOptions);
	*a_ppOptions = NULL;

	CComObject<CEnumConfigItemOptions>* pTmp = NULL;
	CComObject<CEnumConfigItemOptions>::CreateInstance(&pTmp);

	AOptions::const_iterator i;
	for (i = m_aOptions.begin(); i != m_aOptions.end(); i++)
	{
		if (IsValid(i->tValue))
			pTmp->Insert(i->tValue);
	}

	(*a_ppOptions = pTmp)->AddRef();

	return S_OK;
}



