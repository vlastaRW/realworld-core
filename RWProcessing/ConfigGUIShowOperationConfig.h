
#pragma once

#include <ConfigCustomGUIIcons.h>
#include <ConfigCustomGUIML.h>

class ATL_NO_VTABLE CConfigGUIShowOperationConfigDlg :
	public CCustomConfigWndMultiLang<CConfigGUIShowOperationConfigDlg, CCustomConfigWndWithIcons<CConfigGUIShowOperationConfigDlg> >,
	public CDialogResize<CConfigGUIShowOperationConfigDlg>
{
public:
	CConfigGUIShowOperationConfigDlg() :
		CCustomConfigWndMultiLang<CConfigGUIShowOperationConfigDlg, CCustomConfigWndWithIcons<CConfigGUIShowOperationConfigDlg> >(CFGID_CFG_CAPTION, CFGID_CFG_HELPTOPIC)
	{
	}
	enum { IDD = IDD_CONFIGGUI_OPITEM };

	BEGIN_MSG_MAP(CConfigGUIShowOperationConfigDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIShowOperationConfigDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndWithIcons<CConfigGUIShowOperationConfigDlg>)
		COMMAND_HANDLER(IDC_CG_PARAM1NAME, CBN_SELCHANGE, OnPar1NameChange)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIShowOperationConfigDlg)
		DLGRESIZE_CONTROL(IDC_CG_CAPTION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_ICONLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_ICON, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPERATION, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CG_OPCONFIG, DLSZ_DIVSIZE_X(2)|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CG_PREVIEWLABEL, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CG_PREVIEW, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CG_VIEWCONFIG, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2)|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CG_HELPTOPIC_LABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CG_HELPTOPIC, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_PREVIEWMODE_LABEL, DLSZ_MOVE_Y|DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_PREVIEWMODE, DLSZ_MOVE_Y|DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_PARAM1LABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CG_PARAM1NAME, DLSZ_MOVE_Y|DLSZ_MULDIVSIZE_X(1,4))
		DLGRESIZE_CONTROL(IDC_CG_PARAM1BOUNDS, DLSZ_MOVE_Y|DLSZ_MULDIVMOVE_X(1,4))
		DLGRESIZE_CONTROL(IDC_CG_PARAM1MIN, DLSZ_MOVE_Y|DLSZ_MULDIVMOVE_X(1,4)|DLSZ_MULDIVSIZE_X(1,4))
		DLGRESIZE_CONTROL(IDC_CG_PARAM1MAX, DLSZ_MOVE_Y|DLSZ_MULDIVMOVE_X(2,4)|DLSZ_MULDIVSIZE_X(1,4))
		DLGRESIZE_CONTROL(IDC_CG_PARAM1SCALE, DLSZ_MOVE_Y|DLSZ_MULDIVMOVE_X(3,4)|DLSZ_MULDIVSIZE_X(1,4))
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIShowOperationConfigDlg)
		CONFIGITEM_EDITBOX(IDC_CG_CAPTION, CFGID_CFG_CAPTION)
		CONFIGITEM_ICONCOMBO(IDC_CG_ICON, CFGID_CFG_ICONID)
		CONFIGITEM_COMBOBOX(IDC_CG_OPERATION, CFGID_CFG_OPERATION)
		CONFIGITEM_SUBCONFIG(IDC_CG_OPCONFIG, CFGID_CFG_OPERATION)
		CONFIGITEM_COMBOBOX(IDC_CG_PREVIEW, CFGID_CFG_PREVIEW)
		CONFIGITEM_SUBCONFIG(IDC_CG_VIEWCONFIG, CFGID_CFG_PREVIEW)
		CONFIGITEM_EDITBOX(IDC_CG_HELPTOPIC, CFGID_CFG_HELPTOPIC)
		CONFIGITEM_EDITBOX(IDC_CG_PARAM1MIN, CFGID_CFG_PARLOWER)
		CONFIGITEM_EDITBOX(IDC_CG_PARAM1MAX, CFGID_CFG_PARUPPER)
		CONFIGITEM_COMBOBOX(IDC_CG_PARAM1SCALE, CFGID_CFG_PARSCALE)
		CONFIGITEM_COMBOBOX(IDC_CG_PREVIEWMODE, CFGID_CFG_PREVIEWMODE)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnPar1NameChange(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_pIDs)
		{
			int iCur = m_wndParName.GetCurSel();
			if (iCur >= 0)
			{
				ULONG i = m_wndParName.GetItemData(iCur);
				CComBSTR bstr;
				m_pIDs->Get(i, &bstr);
				CComBSTR cCFGID_CFG_PARAMETER(CFGID_CFG_PARAMETER);
				if (bstr == NULL) bstr = L"";
				M_Config()->ItemValuesSet(1, &(cCFGID_CFG_PARAMETER.m_str), CConfigValue(bstr));
			}
		}
		return 0;
	}

	void ExtraConfigNotify()
	{
		try
		{
			if (m_wndParName == NULL)
				m_wndParName = GetDlgItem(IDC_CG_PARAM1NAME);
			m_wndParName.ResetContent();
			m_wndParName.SetItemData(m_wndParName.AddString(_T("")), -1);
			m_pIDs = NULL;
			CComPtr<IConfig> pCfg;
			M_Config()->SubConfigGet(CComBSTR(CFGID_CFG_OPERATION), &pCfg);
			CComPtr<IEnumStrings> pIDs;
			if (pCfg) pCfg->ItemIDsEnum(&pIDs);
			ULONG nIDs = 0;
			if (pIDs) pIDs->Size(&nIDs);
			CConfigValue cCur;
			M_Config()->ItemValueGet(CComBSTR(CFGID_CFG_PARAMETER), &cCur);
			bool bSelSet = false;
			for (ULONG i = 0; i < nIDs; ++i)
			{
				CComBSTR bstrID;
				pIDs->Get(i, &bstrID);
				CConfigValue cVal;
				pCfg->ItemValueGet(bstrID, &cVal);
				if (cVal.TypeGet() == ECVTInteger || cVal.TypeGet() == ECVTFloat)
				{
					CComPtr<IConfigItem> pItem;
					pCfg->ItemGetUIInfo(bstrID, __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
					CComPtr<ILocalizedString> pName;
					if (pItem) pItem->NameGet(&pName, NULL);
					CComBSTR bstrName;
					if (pName) pName->GetLocalized(m_tLocaleID, &bstrName);
					int iPos = m_wndParName.AddString(COLE2T(bstrName ? bstrName : bstrID));
					m_wndParName.SetItemData(iPos, i);
					if (!bSelSet && cCur.TypeGet() == ECVTString && bstrID == cCur.operator BSTR())
					{
						m_wndParName.SetCurSel(iPos);
						bSelSet = true;
					}
				}
			}
			if (!bSelSet)
			{
				m_wndParName.SetCurSel(0);
			}
			m_wndParName.EnableWindow(m_wndParName.GetCount() > 1);
			m_pIDs.Attach(pIDs.Detach());
		}
		catch (...)
		{
		}
	}

private:
	CComboBox m_wndParName;
	CComPtr<IEnumStrings> m_pIDs;
};

