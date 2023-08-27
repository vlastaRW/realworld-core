
#include "stdafx.h"
#include "ConfigWDItemSimple.h"
#include "ConfigItemConditions.h"

CConfigWDItemSimple::~CConfigWDItemSimple()
{
	 delete m_pConditions;
}

void CConfigWDItemSimple::Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, TConfigValue const& a_tDefault, IConfig* a_pParent, EConfigValueType a_eType, CConfigItemConditions* a_pConditions)
{
	CConfigWDItemImpl<IConfigItemSimple>::Init(a_pName, a_pDesc, a_tDefault, a_pParent);
	m_eType = a_eType;
	m_pConditions = a_pConditions;
}

IConfigWDControl* CConfigWDItemSimple::Clone(IConfig* a_pNewParent) const
{
	CComObject<CConfigWDItemSimple>* pCopy = NULL;
	CComObject<CConfigWDItemSimple>::CreateInstance(&pCopy);

	CComPtr<ILocalizedString> pName;
	CComPtr<ILocalizedString> pDesc;
	const_cast<CConfigWDItemSimple*>(this)->NameGet(&pName, &pDesc); // const cast away due to COM
	pCopy->InitCloned(*this, a_pNewParent);
	CConfigWDItemSimple* pAccess = pCopy;
	pAccess->m_eType = m_eType;
	pAccess->m_pConditions = new CConfigItemConditions(*m_pConditions);

	return pCopy;
}

bool CConfigWDItemSimple::IsValid(const TConfigValue& a_tValue) const
{
	// is enabled ?
	if (!m_pConditions->Test(ParentConfigGet()))
		return false;
	// correct type ?
	if (m_eType != a_tValue.eTypeID)
		return false;

	return true;
}

void CConfigWDItemSimple::SuperiorsGet(set<CComBSTR>& a_aSuperiors) const
{
	if (m_pConditions)
		m_pConditions->SuperiorsGet(a_aSuperiors);
}

STDMETHODIMP CConfigWDItemSimple::ValueGetName(const TConfigValue* UNREF(a_ptValue), ILocalizedString** a_ppName)
{
	CHECKPOINTER(a_ppName);

	*a_ppName = NULL;

	return E_NOTIMPL;
}

STDMETHODIMP CConfigWDItemSimple::IsEnabled()
{
	return m_pConditions->Test(ParentConfigGet()) ? S_OK : S_FALSE;
}

