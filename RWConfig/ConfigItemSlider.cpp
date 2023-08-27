#include "stdafx.h"
#include "RWConfig.h"
#include "ConfigItemSlider.h"

CConfigItemSlider::CConfigItemSlider(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, IConfigItemRange* a_pRange, LCID a_tLocaleID, bool a_bFloat, bool a_bAdjustHeight) :
	CConfigItemCtl(a_pParentWnd, a_pszCfgID, a_nCtlBaseID, a_tLocaleID), m_rcPosition(a_rcPosition), m_pItem(a_pRange), m_bFloat(a_bFloat)
{
	RECT rcCtl = m_rcPosition;
	if (a_bAdjustHeight)
	{
		int nHeight = 50;
		if ((m_rcPosition.bottom - m_rcPosition.top) < nHeight)
		{
			m_rcPosition.bottom = m_rcPosition.top + nHeight;
		}
		rcCtl = m_rcPosition;
	//	rcCtl.top += (rcCtl.bottom-rcCtl.top-nHeight)>>1;
	//	rcCtl.bottom = rcCtl.top + nHeight + 120;
	}
	m_wndSlider.Create(a_pParentWnd->GetHWND(), rcCtl, _T("Slider"), WS_VISIBLE | WS_CHILDWINDOW | WS_TABSTOP | /*TBS_AUTOTICKS | */TBS_HORZ | /*TBS_BOTH | */TBS_NOTICKS, 0);
	m_wndSlider.SetWindowLong(GWL_ID, a_nCtlBaseID);
}

CConfigItemSlider::~CConfigItemSlider()
{
}

CConfigItemCtl* CConfigItemSlider::CreateControl(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight)
{
	ATLASSERT(a_pParentWnd);

	CComPtr<IConfigItemRange> pRange;
	if (FAILED(a_pParentWnd->GetConfig()->ItemGetUIInfo(CComBSTR(a_pszCfgID), __uuidof(IConfigItemRange), (void**)&pRange)))
	{
		return NULL;
	}

	CConfigValue cValue;
	a_pParentWnd->GetConfig()->ItemValueGet(CComBSTR(a_pszCfgID), &cValue);

	if (cValue.TypeGet() == ECVTInteger || cValue.TypeGet() == ECVTFloat)
	{
		CConfigValue cFrom;
		pRange->RangePropsGet(&cFrom, NULL, NULL);
		if (pRange->ValueIsValid(cFrom) == S_OK)
		{
			return new CConfigItemSlider(a_pParentWnd, a_rcPosition, a_pszCfgID, a_nCtlBaseID, pRange, a_tLocaleID, cValue.TypeGet() == ECVTFloat, a_bAdjustHeight);
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

void CConfigItemSlider::PositionSet(const RECT& a_rcNewArea, bool a_bAdjustHeight)
{
	RECT rcOldPos = m_rcPosition;
	m_rcPosition = a_rcNewArea;
	if (a_bAdjustHeight)
	{
		int nHeight = 50;
		if ((m_rcPosition.bottom - m_rcPosition.top) < nHeight)
		{
			m_rcPosition.bottom = m_rcPosition.top + nHeight;
		}
	}

	if (rcOldPos.left != m_rcPosition.left || rcOldPos.right != m_rcPosition.right ||
		rcOldPos.top != m_rcPosition.top || rcOldPos.bottom != m_rcPosition.bottom)
	{
		RECT rcTmp = m_rcPosition;
		m_wndSlider.MoveWindow(&rcTmp);
	}
}

const RECT& CConfigItemSlider::PositionGet() const
{
	return m_rcPosition;
}

bool CConfigItemSlider::IsValid() const
{
	CComPtr<IConfigItemRange> pRange;
	if (FAILED(ConfigGet()->ItemGetUIInfo(CfgIDGet(), __uuidof(IConfigItemRange), (void**)&pRange)))
	{
		return false;
	}

	CConfigValue cFrom;
	pRange->RangePropsGet(&cFrom, NULL, NULL);
	return pRange->ValueIsValid(cFrom) == S_OK;
}

void CConfigItemSlider::Delete()
{
	m_wndSlider.DestroyWindow();
	delete this;
}

void CConfigItemSlider::Update()
{
	CConfigValue cFrom;
	CConfigValue cTo;
	CConfigValue cStep;
	m_pItem->RangePropsGet(&cFrom, &cTo, &cStep);
	if (m_bFloat)
	{
		m_wndSlider.SetRange(0, 1000);
		CConfigValue cValue;
		ConfigGet()->ItemValueGet(CfgIDGet(), &cValue);
		float fFrom = cFrom;
		float fTo = cTo;
		float fVal = cValue;
		m_wndSlider.SetPos(static_cast<int>((fVal-fFrom)*1000.0f/(fTo-fFrom)+0.5f));
	}
	else
	{
		m_wndSlider.SetRange(implicit_cast<LONG>(cFrom), implicit_cast<LONG>(cTo));

		CConfigValue cValue;
		ConfigGet()->ItemValueGet(CfgIDGet(), &cValue);

		m_wndSlider.SetPos(implicit_cast<LONG>(cValue));
	}
}

void CConfigItemSlider::ScrollReflected(WORD a_wRequest, WORD a_wPosition)
{
	if (m_bFloat)
	{
		CConfigValue cFrom;
		CConfigValue cTo;
		CConfigValue cStep;
		m_pItem->RangePropsGet(&cFrom, &cTo, &cStep);
		float fFrom = cFrom;
		float fTo = cTo;
		CConfigValue cValue(static_cast<float>(m_wndSlider.GetPos())*(fTo-fFrom)*0.001f + fFrom);
		BSTR bstrID = CfgIDGet();
		ConfigGet()->ItemValuesSet(1, &bstrID, cValue);
	}
	else
	{
		CConfigValue cValue(static_cast<LONG>(m_wndSlider.GetPos()));
		BSTR bstrID = CfgIDGet();
		ConfigGet()->ItemValuesSet(1, &bstrID, cValue);
	}
}

