#pragma once
#include "RWConfig.h"

class CConfigItemParent
{
public:
	virtual IConfig* GetConfig() const = 0;
	virtual HWND GetHWND() const = 0;
	virtual HFONT GetFont() const = 0;
};

class CConfigItemCtl
{
public:
	CConfigItemCtl(CConfigItemParent const* a_pParentWnd, LPCWSTR a_pszCfgID, UINT a_nCtlBaseID, LCID a_tLocaleID) :
		m_pParentWnd(a_pParentWnd), m_bstrCfgID(SysAllocString(a_pszCfgID)), m_nCtlBaseID(a_nCtlBaseID), m_tLocaleID(a_tLocaleID)
	{
	}
	virtual ~CConfigItemCtl()
	{
		SysFreeString(m_bstrCfgID);
	}

	virtual void PositionSet(const RECT& a_rcNewArea, bool a_bAdjustHeight = true) = 0;
	virtual const RECT& PositionGet() const = 0;

	virtual bool IsValid() const = 0;
	virtual void Delete() = 0;
	virtual void Update() = 0;

	virtual void EventReflected(WORD a_wNotifyCode) {};
	virtual void ScrollReflected(WORD a_wRequest, WORD a_wPosition) {};

	IConfig* ConfigGet() const
	{
		return m_pParentWnd->GetConfig();
	}
	UINT CtlIDGetBase() const
	{
		return m_nCtlBaseID;
	}
	BSTR CfgIDGet() const
	{
		return m_bstrCfgID;
	}
	LCID LCIDGet() const
	{
		return m_tLocaleID;
	}

private:
	UINT m_nCtlBaseID;
	CConfigItemParent const* m_pParentWnd;
	BSTR m_bstrCfgID;
	LCID m_tLocaleID;
};

typedef CConfigItemCtl* (FControlCreator)(CConfigItemParent const*, const RECT&, LPCWSTR, UINT, LCID, bool);

