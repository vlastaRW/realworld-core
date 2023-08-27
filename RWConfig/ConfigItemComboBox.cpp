#include "stdafx.h"
#include "RWConfig.h"

#include "ConfigItemComboBox.h"

CConfigItemComboBox::CConfigItemComboBox(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, IConfigItemOptions* a_pOptions, LCID a_tLocaleID, bool a_bAdjustHeight) :
	CConfigItemCtl(a_pParentWnd, a_pszCfgID, a_nCtlBaseID, a_tLocaleID), m_rcPosition(a_rcPosition), m_pItem(a_pOptions)
{
	m_pItem->OptionsEnum(&m_pOptions);
	RECT rcCtl = m_rcPosition;
	if (a_bAdjustHeight)
	{
		int nHeight = 30;
		if ((m_rcPosition.bottom - m_rcPosition.top) < nHeight)
		{
			m_rcPosition.bottom = m_rcPosition.top + nHeight;
		}
		rcCtl = m_rcPosition;
		rcCtl.top += (rcCtl.bottom-rcCtl.top-nHeight)>>1;
		rcCtl.bottom = rcCtl.top + nHeight + 300;
	}
	else
	{
		rcCtl = m_rcPosition;
	}
	m_cComboBox.Create(a_pParentWnd->GetHWND(), rcCtl, _T(""), WS_VISIBLE | WS_CHILDWINDOW | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, 0);
	m_cComboBox.SetFont(a_pParentWnd->GetFont());
	m_cComboBox.SetWindowLong(GWL_ID, a_nCtlBaseID);
}

CConfigItemComboBox::~CConfigItemComboBox()
{
}

CConfigItemCtl* CConfigItemComboBox::CreateControl(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight)
{
	ATLASSERT(a_pParentWnd);

	CComPtr<IConfigItemOptions> pOptions;
	if (FAILED(a_pParentWnd->GetConfig()->ItemGetUIInfo(CComBSTR(a_pszCfgID), __uuidof(IConfigItemOptions), (void**)&pOptions)))
	{
		return NULL;
	}
	CComPtr<IEnumConfigItemOptions> pEnumOpts;
	pOptions->OptionsEnum(&pEnumOpts);
	ULONG nOptCount = 0;
	pEnumOpts->Size(&nOptCount);
	if (nOptCount == 0)
	{
		return NULL;
	}

	CConfigValue cValue;
	a_pParentWnd->GetConfig()->ItemValueGet(CComBSTR(a_pszCfgID), &cValue);

	if (cValue.TypeGet() == ECVTInteger || cValue.TypeGet() == ECVTFloat ||
		cValue.TypeGet() == ECVTString || cValue.TypeGet() == ECVTGUID)
	{
		return new CConfigItemComboBox(a_pParentWnd, a_rcPosition, a_pszCfgID, a_nCtlBaseID, pOptions, a_tLocaleID, a_bAdjustHeight);
	}
	else
	{
		return NULL;
	}
}

void CConfigItemComboBox::PositionSet(const RECT& a_rcNewArea, bool a_bAdjustHeight)
{
	RECT rcOldPos = m_rcPosition;
	m_rcPosition = a_rcNewArea;
	int nHeight = 30;
	if ((m_rcPosition.bottom - m_rcPosition.top) < nHeight)
	{
		m_rcPosition.bottom = m_rcPosition.top + nHeight;
	}

	if (rcOldPos.left != m_rcPosition.left || rcOldPos.right != m_rcPosition.right ||
		rcOldPos.top != m_rcPosition.top || rcOldPos.bottom != m_rcPosition.bottom)
	{
		RECT rcTmp = m_rcPosition;
		rcTmp.bottom = rcTmp.top + 300;
		m_cComboBox.MoveWindow(&rcTmp);
	}
}

const RECT& CConfigItemComboBox::PositionGet() const
{
	return m_rcPosition;
}

bool CConfigItemComboBox::IsValid() const
{
	CComPtr<IConfigItemOptions> pOptions;
	if (FAILED(ConfigGet()->ItemGetUIInfo(CfgIDGet(), __uuidof(IConfigItemOptions), (void**)&pOptions)))
	{
		return false;
	}
	CComPtr<IEnumConfigItemOptions> pEnumOpts;
	pOptions->OptionsEnum(&pEnumOpts);
	ULONG nOptCount = 0;
	pEnumOpts->Size(&nOptCount);
	if (nOptCount == 0)
	{
		return false;
	}

	CConfigValue cValue;
	ConfigGet()->ItemValueGet(CfgIDGet(), &cValue);

	if (cValue.TypeGet() == ECVTInteger || cValue.TypeGet() == ECVTFloat ||
		cValue.TypeGet() == ECVTString || cValue.TypeGet() == ECVTGUID)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CConfigItemComboBox::Delete()
{
	m_cComboBox.DestroyWindow();
	delete this;
}

void CConfigItemComboBox::Update()
{
	m_cComboBox.ResetContent();
	CConfigValue cValue;
	ConfigGet()->ItemValueGet(CfgIDGet(), &cValue);

	m_pOptions = NULL;
	m_pItem->OptionsEnum(&m_pOptions);

	ULONG i;
	CConfigValue cOption;
	for (i = 0; SUCCEEDED(m_pOptions->Get(i, &cOption)); i++)
	{
		CComBSTR bstrText;
		TCHAR szText[256];
		CComPtr<ILocalizedString> pStr;
		if (SUCCEEDED(m_pItem->ValueGetName(cOption, &pStr)) &&
			SUCCEEDED(pStr->GetLocalized(LCIDGet(), &bstrText)))
		{
			USES_CONVERSION;
			_tcscpy(szText, OLE2T(bstrText));
		}
		else
		{
			switch (cOption.TypeGet())
			{
			case ECVTInteger:
				_stprintf(szText, _T("%i"), (LONG)cOption);
				break;
			case ECVTFloat:
				_stprintf(szText, _T("%f"), (float)cOption);
				break;
			case ECVTString:
				{
					USES_CONVERSION;
					_tcscpy(szText, OLE2CT(cOption));
				}
				break;
			case ECVTGUID:
				{
					GUID guid(cOption);
					_stprintf(szText, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
						guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
						guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
				}
				break;
			default:
				ATLASSERT(0);
				_tcscpy(szText, _T("error"));
			}
		}
		m_cComboBox.AddString(szText);
		if (cOption == cValue)
			m_cComboBox.SetCurSel(i);
		RECT rcTmp = m_rcPosition;
		rcTmp.bottom = rcTmp.top + 300;
		m_cComboBox.MoveWindow(&rcTmp);
	}
}

void CConfigItemComboBox::EventReflected(WORD a_wNotifyCode)
{
	if (a_wNotifyCode == CBN_SELCHANGE)
	{
		CConfigValue cValue;
		BSTR bstrID = CfgIDGet();
		m_pOptions->Get(m_cComboBox.GetCurSel(), &cValue);
		ConfigGet()->ItemValuesSet(1, &bstrID, cValue);
	}
}

