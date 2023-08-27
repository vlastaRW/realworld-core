
#pragma once

#include "ConfigItemCtl.h"

class CConfigItemEditBox : public CConfigItemCtl
{
public:
	CConfigItemEditBox(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight);
	virtual ~CConfigItemEditBox();

	static CConfigItemCtl* CreateControl(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight);

	virtual void PositionSet(const RECT& a_rcNewArea, bool a_bAdjustHeight = true);
	virtual const RECT& PositionGet() const;

	virtual bool IsValid() const;
	virtual void Delete();
	virtual void Update();

	virtual void EventReflected(WORD a_wNotifyCode);

private:
	CEdit m_cEditBox;
	RECT m_rcPosition;
};
