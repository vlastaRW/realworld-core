#pragma once
#include "ConfigItemCtl.h"

class CConfigItemSlider : public CConfigItemCtl
{
public:
	CConfigItemSlider(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, IConfigItemRange* a_pRange, LCID a_tLocaleID, bool a_bFloat, bool a_bAdjustHeight);
	~CConfigItemSlider();

	static CConfigItemCtl* CreateControl(CConfigItemParent const* a_pParentWnd, const RECT& a_rcPosition, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID, bool a_bAdjustHeight);

	virtual void PositionSet(const RECT& a_rcNewArea, bool a_bAdjustHeight = true);
	virtual const RECT& PositionGet() const;

	virtual bool IsValid() const;
	virtual void Delete();
	virtual void Update();

	virtual void ScrollReflected(WORD a_wRequest, WORD a_wPosition);

private:
	CTrackBarCtrl m_wndSlider;
	RECT m_rcPosition;
	CComPtr<IConfigItemRange> m_pItem;
	bool m_bFloat;
};
