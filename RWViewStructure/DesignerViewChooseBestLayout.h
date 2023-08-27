// DesignerViewChooseBestLayout.h : Declaration of the CDesignerViewChooseBestLayout

#pragma once
#include "resource.h"       // main symbols
#include "RWViewStructure.h"


// CDesignerViewChooseBestLayout

class ATL_NO_VTABLE CDesignerViewChooseBestLayout : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDesignerView
{
public:
	CDesignerViewChooseBestLayout()
	{
	}

	void Init(IViewManager* a_pViewMgr, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, LONG nLayouts, IConfig* pLayouts, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc)
	{
		CComPtr<IConfig> pLayoutCfg;

		CLSID tBuilderID = GUID_NULL;
		a_pDoc->BuilderID(&tBuilderID);

		if (!IsEqualGUID(tBuilderID, GUID_NULL))
		{
			// have builder ID -> try to find layouts that match
			OLECHAR szNameID[64];
			LONG iSelected = -1;
			for (LONG i = 0; i < nLayouts; ++i)
			{
				_snwprintf(szNameID, itemsof(szNameID), L"%08x\\%s", i, CFGID_BEST_BUILDERID);
				CConfigValue cBuilderID;
				pLayouts->ItemValueGet(CComBSTR(szNameID), &cBuilderID);
				if (IsEqualGUID(tBuilderID, cBuilderID))
				{
					iSelected = i;
					break;
				}
			}
			if (iSelected != -1)
			{
				_snwprintf(szNameID, itemsof(szNameID), L"%08x", iSelected);
				pLayouts->SubConfigGet(CComBSTR(szNameID), &pLayoutCfg);
			}
		}

		if (pLayoutCfg == NULL)
		{
			// 1. gather information
			struct SRating {int nUsed; int nOthers;};
			SRating* aRatings = reinterpret_cast<SRating*>(_alloca(nLayouts*sizeof(SRating)));

			OLECHAR szNameID[64];
			LONG i;
			for (i = 0; i < nLayouts; i++)
			{
				_snwprintf(szNameID, itemsof(szNameID), L"%08x\\%s", i, CFGID_BEST_SUBVIEW);
				CConfigValue cView;
				pLayouts->ItemValueGet(CComBSTR(szNameID), &cView);
				CComPtr<IConfig> pViewCfg;
				pLayouts->SubConfigGet(CComBSTR(szNameID), &pViewCfg);
				CComObjectStackEx<CCheckSuitabilityCallback> cCallback;
				a_pViewMgr->CheckSuitability(a_pViewMgr, cView, pViewCfg, a_pDoc, &cCallback);
				aRatings[i].nUsed = cCallback.cUsed.size();
				aRatings[i].nOthers = cCallback.cMissing.size();
			}

			// 2. find the best layout
			//   layout comparison conditions:
			//   - number of used interfaces
			//   - no other interface required
			//   - layout position in the config
			LONG iBest = 0;
			for (i = 1; i < nLayouts; i++)
			{
				if (aRatings[iBest].nUsed < aRatings[i].nUsed ||
					(aRatings[iBest].nUsed == aRatings[i].nUsed && aRatings[iBest].nOthers && aRatings[i].nOthers == 0))
				{
					// this one is better
					iBest = i;
				}
			}

			_snwprintf(szNameID, itemsof(szNameID), L"%08x", iBest);
			pLayouts->SubConfigGet(CComBSTR(szNameID), &pLayoutCfg);
		}

		CConfigValue cSubViewID;
		pLayoutCfg->ItemValueGet(CComBSTR(CFGID_BEST_SUBVIEW), &cSubViewID);
		CComPtr<IConfig> pSubViewCfg;
		pLayoutCfg->SubConfigGet(CComBSTR(CFGID_BEST_SUBVIEW), &pSubViewCfg);

		a_pViewMgr->CreateWnd(a_pViewMgr, cSubViewID, pSubViewCfg, a_pFrame, a_pStatusBar, a_pDoc, a_hParent, a_rcWindow, a_nStyle, a_tLocaleID, &m_pSubWnd);
		if (m_pSubWnd == NULL)
			throw E_FAIL;
	}

BEGIN_COM_MAP(CDesignerViewChooseBestLayout)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()


	// IChildWindow methods
public:
	STDMETHOD(Handle)(RWHWND *a_pHandle) { return m_pSubWnd ? m_pSubWnd->Handle(a_pHandle) : E_FAIL; }
	STDMETHOD(SendMessage)(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam) { return m_pSubWnd ? m_pSubWnd->SendMessage(a_uMsg, a_wParam, a_lParam) : E_FAIL; }
	STDMETHOD(Show)(BOOL a_bShow) { return m_pSubWnd ? m_pSubWnd->Show(a_bShow) : E_FAIL; }
	STDMETHOD(Move)(RECT const* a_prcPosition) { return m_pSubWnd ? m_pSubWnd->Move(a_prcPosition) : E_FAIL; }
	STDMETHOD(Destroy)() { return m_pSubWnd ? m_pSubWnd->Destroy() : E_FAIL; }
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel) { return m_pSubWnd ? m_pSubWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE; }

	// IDesignerView methods
public:
	STDMETHOD(OnIdle)() { return m_pSubWnd ? m_pSubWnd->OnIdle() : S_FALSE; }
	STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges) { return m_pSubWnd ? m_pSubWnd->OnDeactivate(a_bCancelChanges) : S_FALSE; }
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces) { return m_pSubWnd ? m_pSubWnd->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces) : S_OK; }
	STDMETHOD(OptimumSize)(SIZE* a_pSize){ return m_pSubWnd ? m_pSubWnd->OptimumSize(a_pSize) : E_NOTIMPL; }
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges) { return m_pSubWnd ? m_pSubWnd->DeactivateAll(a_bCancelChanges) : S_OK; }

private:
	CComPtr<IDesignerView> m_pSubWnd;
};

