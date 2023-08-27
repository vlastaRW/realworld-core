#pragma once
#include "ConfigItemCtl.h"

class CConfigItemComboBox : public CConfigItemCtl
{
public:
	CConfigItemComboBox(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, IConfigItemOptions* a_pOptions, LCID a_tLocaleID, bool a_bAdjustHeight);
	virtual ~CConfigItemComboBox();

	static CConfigItemCtl* CreateControl(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight);

	virtual void PositionSet(const RECT& a_rcNewArea, bool a_bAdjustHeight = true);
	virtual const RECT& PositionGet() const;

	virtual bool IsValid() const;
	virtual void Delete();
	virtual void Update();

	virtual void EventReflected(WORD a_wNotifyCode);

private:
	CComboBox m_cComboBox;
	RECT m_rcPosition;
	CComPtr<IEnumConfigItemOptions> m_pOptions;
	CComPtr<IConfigItemOptions> m_pItem;
};
