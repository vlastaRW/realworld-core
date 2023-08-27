// ClipAreaDlg.h : interface of the CClipAreaDlg class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CCLIPAREADLG_H__09045258_F08B_40DA_886D_535DC527F508__INCLUDED_)
#define AFX_CCLIPAREADLG_H__09045258_F08B_40DA_886D_535DC527F508__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "RWConfig.h"

#include "resource.h"
#include "ConfigListDlg.h"

class CClipAreaDlg :
	public CDialogImpl<CClipAreaDlg>,
	public IConfigVisualizer
{
public:
	enum { IDD = IDD_DUMMY };

public:
	CClipAreaDlg(CSyncedData<TScrollInfo>* a_pData, CSyncedData<TColumnInfo>* a_pColumnInfo) :
		m_wndCfgList(a_pData, a_pColumnInfo)
	{
	}

	void SetLocaleID(LCID a_tLocaleID)
	{
		m_wndCfgList.m_tLocaleID = a_tLocaleID;
	}

private:

	BEGIN_MSG_MAP(CClipAreaDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		m_wndCfgList.Create(*this);
		return TRUE;
	}

	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	// IConfigVisualizer methods
public:
	void ConfigSet(IConfig* a_pConfig)
	{
		m_wndCfgList.ConfigSet(a_pConfig);
	}
	void ItemsChanged()
	{
		m_wndCfgList.ItemsChanged();
	}

private:
	CConfigListDlg m_wndCfgList;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCLIPAREADLG_H__09045258_F08B_40DA_886D_535DC527F508__INCLUDED_)
