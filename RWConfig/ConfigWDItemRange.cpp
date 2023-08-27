
#include "stdafx.h"
#include "ConfigWDItemRange.h"
#include "ConfigItemConditions.h"

CConfigWDItemRange::~CConfigWDItemRange()
{
	 delete m_pConditions;
}

void CConfigWDItemRange::Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, TConfigValue const& a_tDefault, IConfig* a_pParent, const TConfigValue& a_tFrom, const TConfigValue& a_tTo, const TConfigValue& a_tStep, CConfigItemConditions* a_pConditions)
{
	CConfigWDItemImpl<IConfigItemRangeAndSimple>::Init(a_pName, a_pDesc, a_tDefault, a_pParent);
	m_cFrom = a_tFrom;
	m_cTo = a_tTo;
	m_cStep = a_tStep;
	m_pConditions = a_pConditions;
	ATLASSERT(m_cFrom.TypeGet() == ECVTInteger || m_cFrom.TypeGet() == ECVTFloat);
}

IConfigWDControl* CConfigWDItemRange::Clone(IConfig* a_pNewParent) const
{
	CComObject<CConfigWDItemRange>* pCopy = NULL;
	CComObject<CConfigWDItemRange>::CreateInstance(&pCopy);

	CComPtr<ILocalizedString> pName;
	CComPtr<ILocalizedString> pDesc;
	const_cast<CConfigWDItemRange*>(this)->NameGet(&pName, &pDesc); // const casted away due to COM
	pCopy->InitCloned(*this, a_pNewParent);
	CConfigWDItemRange* pAccess = pCopy;
	pAccess->m_cFrom = m_cFrom;
	pAccess->m_cTo = m_cTo;
	pAccess->m_cStep = m_cStep;
	pAccess->m_pConditions = new CConfigItemConditions(*m_pConditions);

	return pCopy;
}

bool CConfigWDItemRange::IsValid(const TConfigValue& a_tValue) const
{
	// range enabled ?
	if (!m_pConditions->Test(ParentConfigGet()))
		return false;
	// correct type ?
	if (m_cFrom.TypeGet() != a_tValue.eTypeID)
		return false;
	// GE than lower bound ?
	if (CConfigValue(a_tValue) < m_cFrom)
		return false;
	// LE than upper bound ?
	if (m_cTo < a_tValue)
		return false;

	// TODO: check that the steps are OK

	return true;
}

void CConfigWDItemRange::SuperiorsGet(set<CComBSTR>& a_aSuperiors) const
{
	if (m_pConditions)
		m_pConditions->SuperiorsGet(a_aSuperiors);
}

STDMETHODIMP CConfigWDItemRange::ValueGetName(const TConfigValue* UNREF(a_ptValue), ILocalizedString** a_ppName)
{
	CHECKPOINTER(a_ppName);

	*a_ppName = NULL;

	return E_NOTIMPL;
}

STDMETHODIMP CConfigWDItemRange::RangePropsGet(TConfigValue* a_ptFrom, TConfigValue* a_ptTo, TConfigValue* a_ptStep)
{
	if (a_ptFrom)
		*a_ptFrom = ConfigValueCopy(m_cFrom);

	if (a_ptTo)
		*a_ptTo = ConfigValueCopy(m_cTo);

	if (a_ptStep)
		*a_ptStep = ConfigValueCopy(m_cStep);

	return S_OK;
}

STDMETHODIMP CConfigWDItemRange::IsEnabled()
{
	return m_pConditions->Test(ParentConfigGet()) ? S_OK : S_FALSE;
}

