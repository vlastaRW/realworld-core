// ConfigListDlg.h : interface of the CConfigListDlg class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CCONFIGLISTDLG_H__09045258_F08B_40DA_886D_535DC527F508__INCLUDED_)
#define AFX_CCONFIGLISTDLG_H__09045258_F08B_40DA_886D_535DC527F508__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ConfigVisualizer.h"
#include "SyncedConfigData.h"
#include "SyncedData.h"

#include "ConfigItemCtl.h"

#include "resource.h"

#include <Win32LangEx.h>

class CConfigListDlg :
	public Win32LangEx::CLangDialogImpl<CConfigListDlg>,
	public IConfigVisualizer,
	public CSyncedDataListener<TScrollInfo>,
	public CSyncedDataListener<TColumnInfo>,
	public CConfigItemParent
{
public:
	enum { IDD = IDD_DUMMY };

public:
	CConfigListDlg(CSyncedData<TScrollInfo>* a_pScrollInfo, CSyncedData<TColumnInfo>* a_pColumnInfo) :
		m_pScrollInfo(a_pScrollInfo), m_pColumnInfo(a_pColumnInfo), m_nNextCtlID(CHILDCTL_BASE),
		m_bDisableUpdates(false)
	{
		m_pScrollInfo->ListenerAdd(this);
		m_pColumnInfo->ListenerAdd(this);
	}
	~CConfigListDlg()
	{
		m_pColumnInfo->ListenerRemove(this);
		m_pScrollInfo->ListenerRemove(this);
	}

	// CConfigItemParent methods
public:
	IConfig* GetConfig() const
	{
		return m_pConfig;
	}
	HWND GetHWND() const
	{
		return m_hWnd;
	}
	HFONT GetFont() const
	{
		return Win32LangEx::CLangDialogImpl<CConfigListDlg>::GetFont();
	}

private:
	enum { CHILDCTL_BASE = 100,
		   MAX_CHILDCLTS = 100,
		   IDRANGE_PER_CONTROL = 16};

	BEGIN_MSG_MAP(CConfigListDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_RANGE_HANDLER(CHILDCTL_BASE, CHILDCTL_BASE+MAX_CHILDCLTS*IDRANGE_PER_CONTROL, ReflectEvent)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_HELP, OnHelp)
	END_MSG_MAP()

public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT ReflectEvent(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnHScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnHelp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	// IConfigVisualizer methods
public:
	void ConfigSet(IConfig* a_pConfig);
	void ItemsChanged();

	// CSyncedDataListener<TScrollInfo>::Notify methods
public:
	void Notify(const TScrollInfo& a_tScrollInfo);

	// CSyncedDataListener<TColumnInfo>::Notify methods
public:
	void Notify(const TColumnInfo& a_tColumnInfo);

private:
	struct SControl
	{
		SControl() : pCtl(NULL) {};

		wstring strCfgID;
		UINT nBaseCtlID;
		CConfigItemCtl *pCtl;
		CStatic wndPropName;
	};

private:
	void UpdateControls();
	void DestroyControls();
	void ControlsErase(vector<SControl>::iterator a_iFirst, vector<SControl>::iterator a_iLast);
	UINT GetFreeCtlID();

private:
	vector<SControl> m_aControls;
	CComPtr<IConfig> m_pConfig;
	queue<UINT> m_aFreeCtlIDs;
	UINT m_nNextCtlID;

	CSyncedData<TScrollInfo>* m_pScrollInfo;
	CSyncedData<TColumnInfo>* m_pColumnInfo;

	bool m_bDisableUpdates; // edit control updates config during destruction - eeek
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCONFIGLISTDLG_H__09045258_F08B_40DA_886D_535DC527F508__INCLUDED_)
