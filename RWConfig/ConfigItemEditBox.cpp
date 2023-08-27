#include "stdafx.h"
#include "RWConfig.h"

#include "ConfigItemEditBox.h"

CConfigItemEditBox::CConfigItemEditBox(CConfigItemParent const *a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight) :
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
	m_cEditBox.Create(a_pParentWnd->GetHWND(), rcCtl, _T(""), ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILDWINDOW | WS_TABSTOP, WS_EX_CLIENTEDGE);
	m_cEditBox.SetFont(a_pParentWnd->GetFont());
	m_cEditBox.SetWindowLong(GWL_ID, a_nCtlBaseID);
}

CConfigItemEditBox::~CConfigItemEditBox()
{
}

CConfigItemCtl* CConfigItemEditBox::CreateControl(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight)
{
	ATLASSERT(a_pParentWnd);

	CComPtr<IConfigItemSimple> pSpecItem;
	a_pParentWnd->GetConfig()->ItemGetUIInfo(CComBSTR(a_pszCfgID), __uuidof(IConfigItemSimple), reinterpret_cast<void**>(&pSpecItem));
	if (pSpecItem == NULL || pSpecItem->IsEnabled() != S_OK)
		return NULL;

	CConfigValue cValue;
	a_pParentWnd->GetConfig()->ItemValueGet(CComBSTR(a_pszCfgID), &cValue);
	if (cValue.TypeGet() == ECVTString)
	{
		return new CConfigItemEditBox(a_pParentWnd, a_rcPosition, a_pszCfgID, a_nCtlBaseID, a_tLocaleID, a_bAdjustHeight);
	}
	else
	{
		return NULL;
	}
}

void CConfigItemEditBox::PositionSet(const RECT& a_rcNewArea, bool a_bAdjustHeight)
{
	m_rcPosition = a_rcNewArea;
	m_cEditBox.MoveWindow(&a_rcNewArea);
}

const RECT& CConfigItemEditBox::PositionGet() const
{
	return m_rcPosition;
}

bool CConfigItemEditBox::IsValid() const
{
	CComPtr<IConfigItemSimple> pItem;
	if (FAILED(ConfigGet()->ItemGetUIInfo(CfgIDGet(), __uuidof(IConfigItemSimple), (void**)&pItem)))
	{
		return true; // TODO: or false if unable to decide
	}
	return pItem->IsEnabled() == S_OK;
}

void CConfigItemEditBox::Delete()
{
	m_cEditBox.DestroyWindow();
	delete this;
}

void CConfigItemEditBox::Update()
{
	CConfigValue cValue;
	ConfigGet()->ItemValueGet(CfgIDGet(), &cValue);
	ATLASSERT(cValue.TypeGet() == ECVTString);
	USES_CONVERSION;
	m_cEditBox.SetWindowText(OLE2CT(cValue));
}

void CConfigItemEditBox::EventReflected(WORD a_wNotifyCode)
{
	if (a_wNotifyCode == EN_KILLFOCUS)
	{
		CConfigValue cValue;
		BSTR bstrID = CfgIDGet();
		ConfigGet()->ItemValueGet(bstrID, &cValue);
		if (cValue.TypeGet() == ECVTString)
		{
			TCHAR szTmp[256] = _T("");
			m_cEditBox.GetWindowText(szTmp, itemsof(szTmp));
			cValue = szTmp;
		}
		else if (cValue.TypeGet() == ECVTInteger)
		{
			TCHAR szTmp[256] = _T("");
			m_cEditBox.GetWindowText(szTmp, itemsof(szTmp));
			long iTmp = _ttoi(szTmp);
			cValue = iTmp;
		}
		ConfigGet()->ItemValuesSet(1, &bstrID, cValue);
	}
}

