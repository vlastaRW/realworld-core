
#include "stdafx.h"
#include "RWConfig.h"

#include "ConfigItemCheckBox.h"
#include "ConfigItemComboBox.h"
#include "ConfigItemSlider.h"
#include "ConfigItemEditBox.h"
#include "ConfigItemEditBoxNumber.h"
#include "ConfigItemEditBoxFloat.h"

#include "ConfigListDlg.h"
#include <htmlhelp.h>
#pragma comment(lib, "htmlhelp.lib")

int BackSlashCount(LPCWSTR a_psz)
{
	int nCount = 1;
	do
	{
		if (*a_psz == L'\\') nCount++;
	} while (*(a_psz++));
	return nCount;
}

LRESULT CConfigListDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	UpdateControls();
	return TRUE;
}

LRESULT CConfigListDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DestroyControls();
	return 0;
}

LRESULT CConfigListDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
//	EndDialog(wID);
	return 0;
}

LRESULT CConfigListDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
//	EndDialog(wID);
	return 0;
}

LRESULT CConfigListDlg::OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CConfigListDlg::OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	SetBkColor((HDC)a_wParam, GetSysColor(COLOR_WINDOW));
	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CConfigListDlg::ReflectEvent(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	vector<SControl>::iterator i;
	for (i = m_aControls.begin(); i != m_aControls.end(); i++)
	{
		if (wID >= i->nBaseCtlID && wID < (i->nBaseCtlID+IDRANGE_PER_CONTROL))
		{
			i->pCtl->EventReflected(wNotifyCode);
			break;
		}
	}
	return 0;
}

LRESULT CConfigListDlg::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hWndScrl = reinterpret_cast<HWND>(lParam);
	if (hWndScrl)
	{
		WORD wID = ::GetWindowLong(hWndScrl, GWL_ID);
		vector<SControl>::iterator i;
		for (i = m_aControls.begin(); i != m_aControls.end(); i++)
		{
			if (wID >= i->nBaseCtlID && wID < (i->nBaseCtlID+IDRANGE_PER_CONTROL))
			{
				i->pCtl->ScrollReflected(LOWORD(wParam), HIWORD(wParam));
				break;
			}
		}
	}
	return 0;
}

void CConfigListDlg::Notify(const TScrollInfo& a_tScrollInfo)
{
	RECT rcTmp;
	rcTmp.left = -a_tScrollInfo.nActPosH;
	rcTmp.top = -a_tScrollInfo.nActPosV;
	rcTmp.right = a_tScrollInfo.nPageH;
	rcTmp.bottom = a_tScrollInfo.nPageV;
	MoveWindow(&rcTmp);
	Notify(*m_pColumnInfo);
}

void CConfigListDlg::Notify(const TColumnInfo& a_tColumnInfo)
{
	vector<SControl>::iterator i;
	for (i = m_aControls.begin(); i != m_aControls.end(); i++)
	{
		SControl& sCtl = *i;
		RECT rcCtl = sCtl.pCtl->PositionGet();
		rcCtl.left = a_tColumnInfo.nWidthName;
		rcCtl.right = rcCtl.left + a_tColumnInfo.nWidthValue;
		sCtl.pCtl->PositionSet(rcCtl);
		rcCtl.right = rcCtl.left-10;
		rcCtl.left = 10*BackSlashCount(sCtl.strCfgID.c_str());
		sCtl.wndPropName.MoveWindow(&rcCtl);
	}
}

void CConfigListDlg::ConfigSet(IConfig* a_pConfig)
{
	DestroyControls();
	m_pConfig = a_pConfig;
	UpdateControls();
}

void CConfigListDlg::ItemsChanged()
{
	UpdateControls();
}

void CConfigListDlg::UpdateControls()
{
	if (m_hWnd && m_pConfig.p && !m_bDisableUpdates)
	{
		SetRedraw(FALSE);
		USES_CONVERSION;

		CComPtr<IEnumStrings> pIDs;
		m_pConfig->ItemIDsEnum(&pIDs);
		TScrollInfo tScroll = *m_pScrollInfo;
		tScroll.nRangeV = 0;

		ULONG iNew = 0;
		size_t iOld = 0; // cannot use iterator because it could be invalidated
		for (iNew = 0, iOld = 0; true; iNew++)
		{
			CComBSTR bstrCfgID;
			if (FAILED(pIDs->Get(iNew, &bstrCfgID)))
			{
				// all new IDs processed - delete old items and break from cycle
				ControlsErase(m_aControls.begin()+iOld, m_aControls.end());
				break;
			}
			// find if the processed ID is in the old array
			vector<SControl>::iterator iTmp;
			for (iTmp = m_aControls.begin()+iOld; iTmp != m_aControls.end() && iTmp->strCfgID != implicit_cast<const wchar_t*>(bstrCfgID); iTmp++) ;
			if (iTmp != m_aControls.end())
			{
				// old control found - delete skipped controls and update the control
				if (iTmp != m_aControls.begin()+iOld)
				{
					// something to delete
					ControlsErase(m_aControls.begin()+iOld, iTmp);
				}
				if (m_aControls[iOld].pCtl->IsValid())
				{
					// control still valid - update it
					RECT rcInit =
					{
						m_pColumnInfo->ValueGet().nWidthName, tScroll.nRangeV+3,
						m_pColumnInfo->ValueGet().nWidthName+m_pColumnInfo->ValueGet().nWidthValue, tScroll.nRangeV+23
					};
					m_aControls[iOld].pCtl->PositionSet(rcInit);
					m_aControls[iOld].pCtl->Update();
					tScroll.nRangeV = m_aControls[iOld].pCtl->PositionGet().bottom + 3;
					RECT rcName = m_aControls[iOld].pCtl->PositionGet();
					rcName.right = m_pColumnInfo->ValueGet().nWidthName - 10;
					rcName.left = 10*BackSlashCount(bstrCfgID);
					m_aControls[iOld].wndPropName.MoveWindow(rcName.left, rcName.top, rcName.right-rcName.left, rcName.bottom-rcName.top);
					iOld++;
					continue;
				}
				else
				{
					// control became invalid - delete it
					ControlsErase(m_aControls.begin()+iOld, m_aControls.begin()+iOld+1);
				}
			}
			// old control is not available - insert new control if possible
			static FControlCreator* afControlCreators[] = 
			{
				// controls creators sorted by priority
				CConfigItemCheckBox::CreateControl,
				CConfigItemComboBox::CreateControl,
				CConfigItemSlider::CreateControl,
				CConfigItemEditBox::CreateControl,
				CConfigItemEditBoxNumber::CreateControl,
				CConfigItemEditBoxFloat::CreateControl,
			};

			SControl sCtl;
			sCtl.nBaseCtlID = GetFreeCtlID();
			size_t j;
			for (j = 0; sCtl.pCtl == NULL && j < itemsof(afControlCreators); j++)
			{
				RECT rcInit =
				{
					m_pColumnInfo->ValueGet().nWidthName, tScroll.nRangeV+3,
					m_pColumnInfo->ValueGet().nWidthName+m_pColumnInfo->ValueGet().nWidthValue, tScroll.nRangeV+23
				};
				sCtl.pCtl = afControlCreators[j](this, rcInit, bstrCfgID, sCtl.nBaseCtlID+1, m_tLocaleID, true);
			}
			if (sCtl.pCtl)
			{
				sCtl.strCfgID = bstrCfgID;
				RECT rcDesc = {0, 0, 100, 20};
				CComBSTR bstrName;
				CComPtr<ILocalizedString> pStr;
				CComPtr<IConfigItem> pItem;
				if (FAILED(m_pConfig->ItemGetUIInfo(bstrCfgID, __uuidof(IConfigItem), (void**)&pItem)) ||
					FAILED(pItem->NameGet(&pStr, NULL)) ||
					FAILED(pStr->GetLocalized(m_tLocaleID, &bstrName)))
				{
					bstrName = bstrCfgID;
				}
				RECT rcName = sCtl.pCtl->PositionGet();
				rcName.right = m_pColumnInfo->ValueGet().nWidthName - 10;
				rcName.left = 10*BackSlashCount(bstrCfgID);
				sCtl.wndPropName.Create(*this, rcName, OLE2T(bstrName), WS_CHILDWINDOW | WS_VISIBLE, 0);
				sCtl.wndPropName.SetWindowLong(GWL_ID, sCtl.nBaseCtlID);
				sCtl.wndPropName.SetFont(GetFont());
				sCtl.pCtl->Update();
				tScroll.nRangeV = sCtl.pCtl->PositionGet().bottom + 3;
				m_aControls.insert(m_aControls.begin()+iOld, sCtl);
				iOld++;
			}
			else
			{
				// free the ctlID
				m_aFreeCtlIDs.push(sCtl.nBaseCtlID);
			}
		}
		if (tScroll.nRangeV <= tScroll.nPageV)
		{
			tScroll.nActPosV = 0;
		}
		else if (tScroll.nActPosV > (tScroll.nRangeV - tScroll.nPageV))
		{
			tScroll.nActPosV = tScroll.nRangeV - tScroll.nPageV;
		}
		m_pScrollInfo->ValueSet(tScroll);
		SetRedraw(TRUE);
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
}

void CConfigListDlg::DestroyControls()
{
	ControlsErase(m_aControls.begin(), m_aControls.end());
}

void CConfigListDlg::ControlsErase(vector<SControl>::iterator a_iFirst, vector<SControl>::iterator a_iLast)
{
	m_bDisableUpdates = true; // hack
	vector<SControl>::iterator i;
	for (i = a_iFirst; i != a_iLast; i++)
	{
		m_aFreeCtlIDs.push(i->nBaseCtlID);
		if (i->pCtl)				i->pCtl->Delete();
		if (i->wndPropName.m_hWnd)	i->wndPropName.DestroyWindow();
	}
	m_aControls.erase(a_iFirst, a_iLast);
	m_bDisableUpdates = false;
}

UINT CConfigListDlg::GetFreeCtlID()
{
	if (m_aFreeCtlIDs.empty())
	{
		UINT nRet = m_nNextCtlID;
		m_nNextCtlID += IDRANGE_PER_CONTROL;
		return nRet;
	}
	else
	{
		UINT nRet = m_aFreeCtlIDs.front();
		m_aFreeCtlIDs.pop();
		return nRet;
	}
}

LRESULT CConfigListDlg::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
	if (pHelpInfo->iContextType == HELPINFO_WINDOW && m_pConfig != NULL)
	{
		for (vector<SControl>::const_iterator i = m_aControls.begin(); i != m_aControls.end(); ++i)
		{
			if (UINT(pHelpInfo->iCtrlId) >= i->nBaseCtlID && UINT(pHelpInfo->iCtrlId) < (i->nBaseCtlID+IDRANGE_PER_CONTROL))
			{
				CComPtr<IConfigItem> pUIInfo;
				m_pConfig->ItemGetUIInfo(CComBSTR(i->strCfgID.c_str()), __uuidof(IConfigItem), reinterpret_cast<void**>(&pUIInfo));
				CComPtr<ILocalizedString> pHelpText;
				if (pUIInfo != NULL)
					pUIInfo->NameGet(NULL, &pHelpText);
				CComBSTR bstrHelp;
				if (pHelpText != NULL)
					pHelpText->GetLocalized(m_tLocaleID, &bstrHelp);
				if (bstrHelp.Length() != 0)
				{
					RECT rcItem;
					::GetWindowRect(static_cast<HWND>(pHelpInfo->hItemHandle), &rcItem);
					HH_POPUP hhp;
					hhp.cbStruct = sizeof(hhp);
					hhp.hinst = _pModule->get_m_hInst();
					hhp.idString = 0;
					COLE2CT cText(bstrHelp);
					hhp.pszText = cText;
					hhp.pt.x = rcItem.right;
					hhp.pt.y = rcItem.bottom;
					hhp.clrForeground = 0xffffffff;
					hhp.clrBackground = 0xffffffff;
					hhp.rcMargins.left = -1;
					hhp.rcMargins.top = -1;
					hhp.rcMargins.right = -1;
					hhp.rcMargins.bottom = -1;
					hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
					HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
					return 0;
				}
			}
		}
	}
	a_bHandled = FALSE;
	return 0;
}

