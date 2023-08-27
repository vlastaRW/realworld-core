#include "stdafx.h"
#include "RWConfig.h"

#include "ConfigItemCheckBox.h"
#include "resource.h"
#include <Win32LangEx.h>

CConfigItemCheckBox::CConfigItemCheckBox(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight) :
	CConfigItemCtl(a_pParentWnd, a_pszCfgID, a_nCtlBaseID, a_tLocaleID), m_rcPosition(a_rcPosition)
{
	RECT rcCtl = m_rcPosition;
	if (a_bAdjustHeight)
	{
		int nHeight = 20;
		if ((m_rcPosition.bottom - m_rcPosition.top) < nHeight)
		{
			m_rcPosition.bottom = m_rcPosition.top + nHeight;
		}
		rcCtl = m_rcPosition;
		rcCtl.top += (rcCtl.bottom-rcCtl.top-nHeight)>>1;
		rcCtl.bottom = rcCtl.top + nHeight;
	}
	TCHAR szTmp[32] = _T("");
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CHECKBOX_TITLE, szTmp, itemsof(szTmp), LANGIDFROMLCID(a_tLocaleID));
	m_cCheckBox.Create(a_pParentWnd->GetHWND(), rcCtl, szTmp, WS_VISIBLE | WS_CHILDWINDOW | WS_TABSTOP | BS_CHECKBOX, 0);
	m_cCheckBox.SetFont(a_pParentWnd->GetFont());
	m_cCheckBox.SetWindowLong(GWL_ID, a_nCtlBaseID);
}

CConfigItemCheckBox::~CConfigItemCheckBox()
{
}

CConfigItemCtl* CConfigItemCheckBox::CreateControl(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight)
{
	ATLASSERT(a_pParentWnd);

	CConfigValue cValue;
	a_pParentWnd->GetConfig()->ItemValueGet(CComBSTR(a_pszCfgID), &cValue);
	if (cValue.TypeGet() == ECVTBool)
	{
		return new CConfigItemCheckBox(a_pParentWnd, a_rcPosition, a_pszCfgID, a_nCtlBaseID, a_tLocaleID, a_bAdjustHeight);
	}
	else
	{
		return NULL;
	}
}

void CConfigItemCheckBox::PositionSet(const RECT& a_rcNewArea, bool a_bAdjustHeight)
{
	m_rcPosition = a_rcNewArea;
	m_cCheckBox.MoveWindow(&a_rcNewArea);
}

const RECT& CConfigItemCheckBox::PositionGet() const
{
	return m_rcPosition;
}

bool CConfigItemCheckBox::IsValid() const
{
	CComPtr<IConfigItemSimple> pItem;
	if (FAILED(ConfigGet()->ItemGetUIInfo(CfgIDGet(), __uuidof(IConfigItemSimple), (void**)&pItem)))
	{
		return true; // TODO: or false if unable to decide
	}
	return pItem->IsEnabled() == S_OK;
}

void CConfigItemCheckBox::Delete()
{
	m_cCheckBox.DestroyWindow();
	delete this;
}

void CConfigItemCheckBox::Update()
{
	CConfigValue cValue;
	ConfigGet()->ItemValueGet(CfgIDGet(), &cValue);
	m_cCheckBox.SetCheck(cValue == CConfigValue(true) ? 1 : 0);
}

void CConfigItemCheckBox::EventReflected(WORD a_wNotifyCode)
{
	if (a_wNotifyCode == BN_CLICKED)
	{
		TConfigValue tValue;
		ConfigValueInit(tValue);
		BSTR bstrID = CfgIDGet();
		ConfigGet()->ItemValueGet(bstrID, &tValue);
		tValue.bVal = !tValue.bVal;
		ConfigGet()->ItemValuesSet(1, &bstrID, &tValue);
		ConfigValueClear(tValue);
	}
}

