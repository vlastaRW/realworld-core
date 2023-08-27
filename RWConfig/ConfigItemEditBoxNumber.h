
#pragma once

#include "RWConfig.h"
#include "ConfigItemCtl.h"

class CConfigItemEditBoxNumber : public CConfigItemCtl
{
public:
	CConfigItemEditBoxNumber(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight) :
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
		m_cEditBox.Create(a_pParentWnd->GetHWND(), rcCtl, _T(""), ES_AUTOHSCROLL | ES_NUMBER | WS_VISIBLE | WS_CHILDWINDOW | WS_TABSTOP, WS_EX_CLIENTEDGE);
		m_cEditBox.SetFont(a_pParentWnd->GetFont());
		m_cEditBox.SetWindowLong(GWL_ID, a_nCtlBaseID);
	}
	virtual ~CConfigItemEditBoxNumber()
	{
	}

	inline static CConfigItemCtl* CreateControl(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight)
	{
		ATLASSERT(a_pParentWnd);

		CComPtr<IConfigItemSimple> pSpecItem;
		a_pParentWnd->GetConfig()->ItemGetUIInfo(CComBSTR(a_pszCfgID), __uuidof(IConfigItemSimple), reinterpret_cast<void**>(&pSpecItem));
		if (pSpecItem == NULL || pSpecItem->IsEnabled() != S_OK)
			return NULL;

		CConfigValue cValue;
		a_pParentWnd->GetConfig()->ItemValueGet(CComBSTR(a_pszCfgID), &cValue);
		if (cValue.TypeGet() == ECVTInteger)
		{
			return new CConfigItemEditBoxNumber(a_pParentWnd, a_rcPosition, a_pszCfgID, a_nCtlBaseID, a_tLocaleID, a_bAdjustHeight);
		}
		else
		{
			return NULL;
		}
	}

	virtual void PositionSet(const RECT& a_rcNewArea, bool a_bAdjustHeight = true)
	{
		m_rcPosition = a_rcNewArea;
		m_cEditBox.MoveWindow(&a_rcNewArea);
	}
	virtual const RECT& PositionGet() const
	{
		return m_rcPosition;
	}

	virtual bool IsValid() const
	{
		CComPtr<IConfigItemSimple> pItem;
		if (FAILED(ConfigGet()->ItemGetUIInfo(CfgIDGet(), __uuidof(IConfigItemSimple), (void**)&pItem)))
		{
			return true; // TODO: or false if unable to decide
		}
		return pItem->IsEnabled() == S_OK;
	}
	virtual void Delete()
	{
		m_cEditBox.DestroyWindow();
		delete this;
	}
	virtual void Update()
	{
		CConfigValue cValue;
		ConfigGet()->ItemValueGet(CfgIDGet(), &cValue);
		ATLASSERT(cValue.TypeGet() == ECVTInteger);
		TCHAR buf[32];
		_stprintf(buf, _T("%i"), cValue.operator LONG());
		m_cEditBox.SetWindowText(buf);
	}

	virtual void EventReflected(WORD a_wNotifyCode)
	{
		if (a_wNotifyCode == EN_KILLFOCUS)
		{
			CConfigValue cValue;
			BSTR bstrID = CfgIDGet();
			ConfigGet()->ItemValueGet(bstrID, &cValue);
			if (cValue.TypeGet() == ECVTInteger)
			{
				TCHAR szTmp[256] = _T("");
				m_cEditBox.GetWindowText(szTmp, itemsof(szTmp));
				long iTmp = _ttoi(szTmp);
				cValue = iTmp;
			}
			ConfigGet()->ItemValuesSet(1, &bstrID, cValue);
		}
	}

private:
	CEdit m_cEditBox;
	RECT m_rcPosition;
};
