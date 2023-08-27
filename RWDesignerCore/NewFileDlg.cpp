// NewFileDlg.cpp : Implementation of CNewFileDlg

#include "stdafx.h"
#include "NewFileDlg.h"
#include <PlugInCache.h>
#include <XPGUI.h>
#include <DocumentName.h>
#include <StringParsing.h>
#include "ConfigIDsApp.h"
#if __has_include("../../RWInput/RWCodecImagePNG/RWImageCodecPNG.h")
#include "../../RWInput/RWCodecImagePNG/RWImageCodecPNG.h"
#else
#endif

//static const OLECHAR CFGID_TEMPLATES[] = L"DocTemplates";
static const OLECHAR CFGID_SELECTEDWIZARD[] = L"SelectedWizard";

struct initwizardconfig
{
	IConfigWithDependencies* a_pConfig;
	initwizardconfig(IConfigWithDependencies* a_pConfig) : a_pConfig(a_pConfig) {}
	void operator()(CLSID const* begin, CLSID const* const end)
	{
		CConfigValue cTrue(true);
		while (begin < end)
		{
			CComPtr<IDesignerWizard> p;
			RWCoCreateInstance(p, *begin);
			if (p == NULL)
			{
				++begin;
				continue;
			}
			CComPtr<IConfig> pPlugInCfg;
			p->Config(&pPlugInCfg);
			if (pPlugInCfg)
			{
				wchar_t szCLSID[64] = L"Wizard";
				StringFromGUID(*begin, szCLSID+6);
				CComQIPtr<ISubConfig> pSC(pPlugInCfg);
				if (pSC)
				{
					a_pConfig->ItemInsSimple(CComBSTR(szCLSID), NULL, NULL, cTrue, pSC, 0, NULL);
				}
				else
				{
					CComPtr<ISubConfigSwitch> pSubCfg;
					RWCoCreateInstance(pSubCfg, __uuidof(SubConfigSwitch));
					pSubCfg->ItemInsert(cTrue, pPlugInCfg);
					a_pConfig->ItemInsSimple(CComBSTR(szCLSID), NULL, NULL, cTrue, pSubCfg, 0, NULL);
				}
			}
			++begin;
		}
	}
};

HRESULT CNewFileDlg::InitConfig(IConfigWithDependencies* a_pConfig)
{
	a_pConfig->ItemInsSimple(CComBSTR(CFGID_SELECTEDWIZARD), NULL, NULL, CConfigValue(GUID_NULL), NULL, 0, NULL);

	CPlugInEnumerator::EnumCategoryCLSIDs(CATID_DesignerWizard, initwizardconfig(a_pConfig));
	
	return S_OK;
}

void CNewFileDlg::InitBuilders()
{
	CConfigValue cLayouts;
	m_pAppConfig->ItemValueGet(CComBSTR(CFGID_VIEWPROFILES), &cLayouts);
	std::vector<GUID> cLayFact;
	bool bNullGuid = false;
	for (LONG i = 0; i < cLayouts.operator LONG(); ++i)
	{
		OLECHAR szID[128];
		swprintf(szID, L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DOCBUILDER);
		CConfigValue cBuilder;
		m_pAppConfig->ItemValueGet(CComBSTR(szID), &cBuilder);
		if (IsEqualGUID(cBuilder, GUID_NULL))
		{
			bNullGuid = true;
			continue;
		}
		bool bAdd = true;
		for (std::vector<GUID>::const_iterator i = cLayFact.begin(); i != cLayFact.end(); ++i)
		{
			if (IsEqualGUID(*i, cBuilder))
			{
				bAdd = false;
				break;
			}
		}
		if (bAdd)
			cLayFact.push_back(cBuilder);
	}
	CBuilders cBuilders;
	if (cLayFact.empty() && bNullGuid)
	{
		// no layout is using explicit layout, add all builders
		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumUnknowns> pBuilders;
		pPIC->InterfacesEnum(CATID_DocumentBuilder, __uuidof(IDocumentBuilder), 0, &pBuilders, NULL);
		ULONG nBuilders = 0;
		if (pBuilders) pBuilders->Size(&nBuilders);
		for (ULONG i = 0; i < nBuilders; ++i)
		{
			CComPtr<IDocumentBuilder> pBuilder;
			pBuilders->Get(i, __uuidof(IDocumentBuilder), reinterpret_cast<void**>(&pBuilder));
			if (pBuilder) cBuilders.push_back(pBuilder);
		}
	}
	for (std::vector<GUID>::const_iterator i = cLayFact.begin(); i != cLayFact.end(); ++i)
	{
		CComPtr<IDocumentBuilder> pBuilder;
		RWCoCreateInstance(pBuilder, *i);
		if (pBuilder) cBuilders.push_back(pBuilder);
	}
	bool bSwapped = true;
	while (bSwapped)
	{
		bSwapped = false;
		for (size_t j = 0; j < cBuilders.size()-1; ++j)
		{
			ULONG nP1 = 0;
			cBuilders[j]->Priority(&nP1);
			ULONG nP2 = 0;
			cBuilders[j+1]->Priority(&nP2);
			if (nP1 < nP2)
			{
				std::swap(cBuilders[j].p, cBuilders[j+1].p);
				bSwapped = true;
			}
		}
	}
	std::swap(cBuilders, m_cBuilders);
}

HRESULT CNewFileDlg::Activate()
{
	ULONG nTimeStamp = CPlugInEnumerator::GetCategoryTimestamp(CATID_DesignerWizard);
	if (m_nTimeStamp != nTimeStamp)
	{
		m_cTemplates.clear();
		m_wndList.DeleteAllItems();
		if (XPGUI::IsXP())
			m_wndList.RemoveAllGroups();
		InitWizardList();
		m_nTimeStamp = nTimeStamp;
	}
	ShowWindow(SW_SHOW);
	GotoDlgCtrl(m_wndList);
	SetDefaultButtonState(_SharedStringTable.GetStringAuto(IDS_STARTVIEWBTN_NEW), m_wndList.GetSelectedIndex() != -1, HasChoices() == S_OK);
	return S_OK;
}

HRESULT CNewFileDlg::Deactivate()
{
	if (IsWindow())
	{
		ShowWindow(SW_HIDE);
	}
	return S_OK;
}

HRESULT CNewFileDlg::ClickedDefault()
{
	if (m_iActiveItem == -1)
	{
		return S_FALSE;
	}

	HWND h = GetFocus();
	m_wndList.SetFocus(); // force update of the configuration panel

	wchar_t szCLSID[64] = L"Wizard";
	StringFromGUID(m_cTemplates[m_iActiveItem].id, szCLSID+6);
	CComPtr<IConfig> pCfg;
	m_pAppConfig->SubConfigGet(CComBSTR(szCLSID), &pCfg);
	CComPtr<IDocument> pDoc;
	RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
	HRESULT hRes = m_cTemplates[m_iActiveItem].p->Activate(m_hWnd, m_tLocaleID, pCfg, m_cBuilders.size(), !m_cBuilders.empty() ? &(m_cBuilders[0].p) : NULL, NULL, CComQIPtr<IDocumentBase>(pDoc));
	if (pDoc == NULL || hRes != S_OK)
	{
		return hRes;
	}

	CComPtr<IStorageFilter> pOldLoc;
	pDoc->LocationGet(&pOldLoc);
	CComQIPtr<IDocumentName> pDocName(pOldLoc);
	if (pDocName == NULL)
	{
		CComBSTR bstrName;
		CMultiLanguageString::GetLocalized(L"[0409]unnamed[0405]nepojmenovaný", m_tLocaleID, &bstrName);
		CComPtr<IConfig> pCfg;
		GUID tEncID = GUID_NULL;
		pDoc->EncoderGet(&tEncID, &pCfg);
		CComPtr<IDocumentEncoder> pEnc;
		if (!IsEqualGUID(tEncID, GUID_NULL)) RWCoCreateInstance(pEnc, tEncID);
		CComPtr<IDocumentType> pDocType;
		if (pEnc) pEnc->DocumentType(&pDocType);
		CComBSTR bstrType;
		if (pDocType != NULL)
		{
			pDocType->DefaultExtensionGet(&bstrType);
		}
		if (bstrType != NULL)
		{
			bstrName += L".";
			bstrName += bstrType;
		}
		CComObject<CDocumentName>* pName = NULL;
		CComObject<CDocumentName>::CreateInstance(&pName);
		CComPtr<IStorageFilter> pTmp = pName;
		pName->Init(bstrName);
		pDoc->LocationSet(pName);
	}

	m_pClbk->OpenDocument(pDoc);
	return S_OK;
}

STDMETHODIMP CNewFileDlg::OnIdle()
{
	if (m_iAutoSelectedItem >= 0 && m_iAutoSelectedItem < int(m_cTemplates.size()))
	{
		CComQIPtr<IDesignerWizardClipboard> pDWCB(m_cTemplates[m_iAutoSelectedItem].p);
		if (pDWCB)
		{
			if (S_OK == pDWCB->CanActivate())
				return S_OK;
			m_iAutoSelectedItem = -1;
		}
	}
	// check if wizards became active
	for (CTemplates::const_iterator i = m_cTemplates.begin(); i != m_cTemplates.end(); ++i)
	{
		CComQIPtr<IDesignerWizardClipboard> pDWCB(i->p);
		if (pDWCB == NULL)
			continue;
		if (pDWCB->CanActivate() == S_OK)
		{
			int selItem = i-m_cTemplates.begin();
			if (m_iActiveItem != selItem)
			{
				int n = m_wndList.GetItemCount();
				for (int i = 0; i < n; ++i)
				{
					if (m_wndList.GetItemData(i) == selItem)
					{
						m_iAutoSelectedItem = i;
						m_wndList.SelectItem(i);
						return S_OK;
					}
				}
			}
			return S_OK;
		}
	}

	return S_OK;
}

void CNewFileDlg::OnFinalMessage(HWND a_hWnd)
{
	Release();
}

COLORREF GammaBlend(COLORREF a_clrBg, COLORREF a_clrFg, ULONG a_nAlpha);

LRESULT CNewFileDlg::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam != IDC_FILL)
	{
		bHandled = FALSE;
		return 0;
	}

	DRAWITEMSTRUCT* pDIS = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
	FillRect(pDIS->hDC, &pDIS->rcItem, reinterpret_cast<HBRUSH>(COLOR_3DFACE+1));
	return 0;
}

struct initwizarddialog
{
	CNewFileDlg::CBuilders const& m_cBuilders;
	CNewFileDlg::CTemplates& m_cTemplates;
	initwizarddialog(CNewFileDlg::CBuilders const& m_cBuilders, CNewFileDlg::CTemplates& m_cTemplates) :
	m_cBuilders(m_cBuilders), m_cTemplates(m_cTemplates) {}
	void operator()(CLSID const* begin, CLSID const* const end)
	{
		CConfigValue cTrue(true);
		while (begin < end)
		{
			CComPtr<IDesignerWizard> p;
			RWCoCreateInstance(p, *begin);
			if (p == NULL)
			{
				++begin;
				continue;
			}
			if (!m_cBuilders.empty() && p->IsCompatible(m_cBuilders.size(), &(m_cBuilders[0].p)) != S_OK)
			{
				++begin;
				continue;
			}
			CNewFileDlg::STemplate sTmp;
			sTmp.id = *begin;
			std::swap(sTmp.p.p, p.p);
			m_cTemplates.push_back(sTmp);
			++begin;
		}
	}
};

void CNewFileDlg::InitWizardList()
{
	ATLASSERT(sizeof(CComPtr<IDocumentBuilder>) == sizeof(IDocumentBuilder*));
	CPlugInEnumerator::EnumCategoryCLSIDs(CATID_DesignerWizard, initwizarddialog(m_cBuilders, m_cTemplates));
	m_iAutoSelectedItem = -1;

	{
		// sort by priority
		std::vector<ULONG> cPrio;

		ULONG nSize = m_cTemplates.size();
		cPrio.resize(nSize);
		for (ULONG i = 0; i < nSize; ++i)
			m_cTemplates[i].p->Priority(&(cPrio[i]));

		for (ULONG i = 1; i < nSize; ++i)
			for (ULONG j = nSize-1; j >= i; --j)
				if (cPrio[j] > cPrio[j-1])
				{
					ULONG n = cPrio[j];
					cPrio[j] = cPrio[j-1];
					cPrio[j-1] = n;
					STemplate t = m_cTemplates[j];
					m_cTemplates[j] = m_cTemplates[j-1];
					m_cTemplates[j-1] = t;
				}
	}

	CConfigValue cActiveWizard;
	m_pAppConfig->ItemValueGet(CComBSTR(CFGID_SELECTEDWIZARD), &cActiveWizard);

	int iSelect = -1;

	if (XPGUI::IsXP())
	{
		wchar_t szUnknownCat[64] = L"";
		Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_STARTVIEW_UNKNOWNCATEGORY, szUnknownCat, itemsof(szUnknownCat), LANGIDFROMLCID(m_tLocaleID));

		std::map<std::wstring, std::pair<std::wstring, int> > cCategoryToLocalizedAndID;
		std::map<size_t, std::wstring> cItemToCategory;

		// classify by categories
		ULONG nSize = m_cTemplates.size();
		for (ULONG i = 0; i < nSize; ++i)
		{
			CComPtr<ILocalizedString> pStr;
			m_cTemplates[i].p->Category(&pStr);
			CComBSTR bstr;
			if (pStr != NULL)
				pStr->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), &bstr);
			if (bstr.Length() == 0)
				bstr = szUnknownCat;

			std::wstring strCat(bstr);
			std::pair<std::wstring, int>& catRec = cCategoryToLocalizedAndID[strCat];
			catRec.second = -1;
			if (catRec.first.empty() && m_tLocaleID != MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT))
			{
				CComBSTR bstrLoc;
				if (pStr != NULL)
					pStr->GetLocalized(m_tLocaleID, &bstrLoc);
				if (bstrLoc.Length() == 0)
				{
					bstrLoc = szUnknownCat;
				}
				if (wcscmp(bstr, bstrLoc) != 0)
					catRec.first = std::wstring(bstrLoc);
			}
			cItemToCategory[i] = strCat;
		}

		int nIconSize = XPGUI::GetSmallIconSize()*3;
		HIMAGELIST hImageList = ImageList_Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 8, 8);
		for (ULONG i = 0; i < nSize; ++i)
		{
			HICON hIcon = NULL;
			m_cTemplates[i].p->Icon(nIconSize, &hIcon);
			ImageList_AddIcon(hImageList, hIcon);
			DestroyIcon(hIcon);
		}

		//m_wndList.SetImageList(hImageList, LVSIL_SMALL);
		HIMAGELIST hILP = m_wndList.SetImageList(hImageList, LVSIL_NORMAL);
		if (hILP) ImageList_Destroy(hILP);

		// insert items
		for (ULONG i = 0; i < nSize; ++i)
		{
			CComPtr<ILocalizedString> pStr;
			m_cTemplates[i].p->Name(&pStr);
			CComBSTR bstr;
			pStr->GetLocalized(m_tLocaleID, &bstr);
			int j = m_wndList.AddItem(i, 0, OLE2CT(bstr), i);
			m_wndList.SetItemData(j, i);
			if (IsEqualGUID(cActiveWizard, m_cTemplates[i].id))
			{
				iSelect = j;
			}
		}

		// insert groups
		LVGROUP tGroup;
		ZeroMemory(&tGroup, sizeof(tGroup));
		tGroup.cbSize = sizeof(tGroup);
		tGroup.mask = LVGF_HEADER|LVGF_ALIGN|LVGF_GROUPID|LVGF_STATE;
		tGroup.iGroupId = 1;
		tGroup.stateMask = LVGS_NORMAL;
		tGroup.state = LVGS_NORMAL;
		tGroup.uAlign = LVGA_HEADER_LEFT;

		for (std::map<std::wstring, std::pair<std::wstring, int> >::iterator i = cCategoryToLocalizedAndID.begin(); i != cCategoryToLocalizedAndID.end(); ++i)
		{
			wstring const& str = i->second.first.empty() ? i->first : i->second.first;

			tGroup.pszHeader = const_cast<LPWSTR>(str.c_str());
			tGroup.cchHeader = str.length();
			m_wndList.InsertGroup(-1, &tGroup);
			i->second.second = tGroup.iGroupId++;
		}

		LVGROUPMETRICS tMetrics;
		ZeroMemory(&tMetrics, sizeof(tMetrics));
		tMetrics.cbSize = sizeof(tMetrics);
		tMetrics.mask = LVGMF_BORDERSIZE;
		tMetrics.Left = 10;
		tMetrics.Right = 300;
		tMetrics.Top = 10;
		tMetrics.Bottom = 10;
		//m_wndList.SetGroupMetrics(&tMetrics);

		for (ULONG i = 0; i < nSize; ++i)
		{
			LVITEM t;
			ZeroMemory(&t, sizeof t);
			t.mask = LVIF_GROUPID;
			t.iItem = i;
			t.iGroupId = cCategoryToLocalizedAndID[cItemToCategory[i]].second;
			m_wndList.SetItem(&t);
		}

		m_wndList.EnableGroupView(TRUE);

		m_wndList.Arrange(LVA_ALIGNLEFT);
		for (ULONG i = 0; i < nSize; ++i)
		{
			m_wndList.Update(i);
		}
	}
	else
	{
		ULONG nSize = m_cTemplates.size();
		HIMAGELIST hImageList = ImageList_Create(32, 32, XPGUI::GetImageListColorFlags(), 8, 8);
		ULONG i;
		for (i = 0; i < nSize; ++i)
		{
			HICON hIcon = NULL;
			m_cTemplates[i].p->Icon(32, &hIcon);
			ImageList_AddIcon(hImageList, hIcon);
			DestroyIcon(hIcon);
		}

		HIMAGELIST hILP = m_wndList.SetImageList(hImageList, LVSIL_SMALL);
		if (hILP) ImageList_Destroy(hILP);
		for (ULONG i = 0; i < nSize; ++i)
		{
			CComPtr<ILocalizedString> pStr;
			m_cTemplates[i].p->Name(&pStr);
			CComBSTR bstr;
			pStr->GetLocalized(m_tLocaleID, &bstr);
			int j = m_wndList.AddItem(i, 0, OLE2CT(bstr), i);
			m_wndList.SetItemData(j, i);
			if (IsEqualGUID(cActiveWizard, m_cTemplates[i].id))
			{
				iSelect = j;
			}
		}
	}
	int nItemCount = m_wndList.GetItemCount();
	for (int i = 0; i < nItemCount; ++i)
	{
		int wiz = m_wndList.GetItemData(i);
		if (wiz >= 0 && size_t(wiz) < m_cTemplates.size())
		{
			CComQIPtr<IDesignerWizardClipboard> pClip(m_cTemplates[wiz].p);
			if (pClip)
			{
				if (S_OK == pClip->CanActivate())
				{
					m_iAutoSelectedItem = i;
					iSelect = i;
					break;
				}
			}
		}
	}
	m_iActiveItem = -1;
	if (nItemCount > 0)
		m_wndList.SetItemState(iSelect >= 0 ? iSelect : 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
}

LRESULT CNewFileDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	AddRef();

	m_hBGBrush.CreateSolidBrush(m_clrBG = GammaBlend(GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_3DSHADOW), 0x80));
	m_wndOKButton.SubclassWindow(GetDlgItem(IDOK), this);

	m_wndList = GetDlgItem(IDC_LIST);
	m_wndList.SetBkColor(GetSysColor(COLOR_3DFACE));

	//LOGFONT lf = {0};
	//::GetObject(m_wndList.GetFont(), sizeof(lf), &lf);
	//lf.lfHeight = (lf.lfHeight*5)>>2;
	//m_wndList.SetFont(m_font.CreateFontIndirect(&lf));

	CWindow wnd = GetDlgItem(IDC_PANEL);
	RECT rc;
	wnd.GetWindowRect(&rc);
	ScreenToClient(&rc);
	RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
	m_pConfigWnd->Create(m_hWnd, &rc, IDC_PANEL, m_tLocaleID, TRUE, ECWBMNothing);
	RWHWND h = NULL;
	m_pConfigWnd->Handle(&h);
	if (h)
		::SetWindowPos(h, wnd, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
	wnd.DestroyWindow();

	if (XPGUI::IsXP())
	{
		m_wndList.SetExtendedListViewStyle(LVS_EX_INFOTIP|LVS_EX_UNDERLINEHOT|LVS_EX_DOUBLEBUFFER);
		m_wndList.SetView(LV_VIEW_TILE);

		if (XPGUI::IsVista() && CTheme::IsThemingSupported())
		{
			::SetWindowTheme(m_wndList, L"explorer", NULL);
		}
		LVBKIMAGE tLVBI;
		tLVBI.ulFlags = LVBKIF_TYPE_WATERMARK/*|LVBKIF_SOURCE_HBITMAP*/;

		HRSRC hRsrc = FindResource(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDB_WATERMARK), _T("PNG"));
		HBITMAP hBmp = NULL;
		if (hRsrc)
		{
			HGLOBAL hGlob = LoadResource(_pModule->get_m_hInst(), hRsrc);
			if (hGlob)
			{
				void* pData = LockResource(hGlob);
				if (pData)
				{
					CLSID const CLSID_DocumentDecoderPNG = { 0x1953b6dc, 0xc029, 0x4c0f, { 0xbc, 0x93, 0x1b, 0x23, 0x1c, 0xfe, 0x97, 0x50 } };

					CComPtr<IDocumentDecoder> pDec;
					RWCoCreateInstance(pDec, CLSID_DocumentDecoderPNG);
					if (pDec)
					{
						CComPtr<IDocumentImage> pImage;
						RWCoCreateInstance(pImage, __uuidof(ImageCache));
						if (pImage)
						{
							CComQIPtr<IDocumentBuilder> pBuilder(pImage);
							if (SUCCEEDED(pDec->Parse(SizeofResource(_pModule->get_m_hInst(), hRsrc), reinterpret_cast<BYTE const*>(pData), NULL, 1, &(pBuilder.p), NULL, NULL, NULL, NULL, NULL)))
							{
								hBmp = CreateWatermark(pImage, GetSysColor(COLOR_3DFACE));
							}
						}
					}
					UnlockResource(pData);
				}
				FreeResource(hGlob);
			}
		}
		//HBITMAP hBmp = LoadBitmap(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDB_WATERMARK));
		COLORREF clrBG = GetSysColor(COLOR_WINDOW);
		COLORREF clrFG = GetSysColor(COLOR_WINDOWTEXT);
		if (clrBG != 0xffffff || clrFG != 0)
		{
			HDC hDC = GetDC();
			BITMAPINFO tBMI;
			ZeroMemory(&tBMI, sizeof tBMI);
			tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
			CAutoVectorPtr<DWORD> pData;
			if (GetDIBits(hDC, hBmp, 0, 0, NULL, &tBMI, DIB_RGB_COLORS) && pData.Allocate(tBMI.bmiHeader.biWidth*abs(tBMI.bmiHeader.biHeight)))
			{
				tBMI.bmiHeader.biPlanes = 1;
				tBMI.bmiHeader.biBitCount = 32;
				tBMI.bmiHeader.biCompression = BI_RGB;
				GetDIBits(hDC, hBmp, 0, abs(tBMI.bmiHeader.biHeight), pData.m_p, &tBMI, DIB_RGB_COLORS);
				DWORD aMap[256];
				for (int i = 0; i < 256; ++i)
				{
					aMap[i] = 0xff000000|RGB(
						(GetRValue(clrBG)*i+GetRValue(clrFG)*(255-i)+127)/255,
						(GetGValue(clrBG)*i+GetGValue(clrFG)*(255-i)+127)/255,
						(GetBValue(clrBG)*i+GetBValue(clrFG)*(255-i)+127)/255);
				}
				DWORD* pEnd = pData.m_p+tBMI.bmiHeader.biWidth*abs(tBMI.bmiHeader.biHeight);
				for (DWORD* p = pData.m_p; p != pEnd; ++p)
					*p = aMap[*p&0xff];
				hBmp = CreateDIBitmap(hDC, &tBMI.bmiHeader, CBM_INIT, pData.m_p, &tBMI, DIB_RGB_COLORS);
			}
			ReleaseDC(hDC);
		}
		tLVBI.hbm = hBmp;
		tLVBI.pszImage = NULL;
		tLVBI.cchImageMax = 0;
		tLVBI.xOffsetPercent = 0;
		tLVBI.yOffsetPercent = 0;
		m_wndList.SetBkImage(&tLVBI);

		LVTILEVIEWINFO tTileInfo;
		ZeroMemory(&tTileInfo, sizeof(tTileInfo));
		tTileInfo.cbSize = sizeof(tTileInfo);
		tTileInfo.dwMask = LVTVIM_TILESIZE|LVTVIM_COLUMNS|LVTVIM_LABELMARGIN;
		tTileInfo.dwFlags = LVTVIF_FIXEDWIDTH;//LVTVIF_AUTOSIZE;
		HDC hdc = GetDC();
		float fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0;
		ReleaseDC(hdc);
		tTileInfo.sizeTile.cx = 200*fScale;
		tTileInfo.sizeTile.cy = 0;
		tTileInfo.cLines = 3;
		tTileInfo.rcLabelMargin.left = 4*fScale;
		tTileInfo.rcLabelMargin.right = 4*fScale;
		tTileInfo.rcLabelMargin.top = 0;
		tTileInfo.rcLabelMargin.bottom = 0;
		m_wndList.SetTileViewInfo(&tTileInfo);
	}
	else
	{
		m_wndList.SetExtendedListViewStyle(LVS_EX_INFOTIP|LVS_EX_UNDERLINEHOT);
	}
	m_nTimeStamp = CPlugInEnumerator::GetCategoryTimestamp(CATID_DesignerWizard);
	InitWizardList();

	DlgResize_Init(false, false, 0);

	return 1;  // Let the system set the focus
}

#include <GammaCorrection.h>

HBITMAP CNewFileDlg::CreateWatermark(IDocumentImage* a_pImage, COLORREF a_clrBG)
{
	TImageSize tSize = {0, 0};
	a_pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
	if (tSize.nX*tSize.nY == 0)
		return NULL;
	CAutoVectorPtr<TPixelChannel> pData(new TPixelChannel[tSize.nX*tSize.nY]);
	a_pImage->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, pData, NULL, EIRIAccurate);
	CGammaTables cGT(2.2f);
	ULONG const nBgR = cGT.m_aGamma[GetRValue(a_clrBG)];
	ULONG const nBgG = cGT.m_aGamma[GetGValue(a_clrBG)];
	ULONG const nBgB = cGT.m_aGamma[GetBValue(a_clrBG)];
	TPixelChannel tBg;
	tBg.bR = GetRValue(a_clrBG);
	tBg.bG = GetGValue(a_clrBG);
	tBg.bB = GetBValue(a_clrBG);
	tBg.bA = 255;
	TPixelChannel* p = pData;
	for (TPixelChannel* const pEnd = p+tSize.nX*tSize.nY; p != pEnd; ++p)
	{
		if (p->bA == 0)
		{
			*p = tBg;
		}
		else
		{
			ULONG const nA = p->bA;
			ULONG const nIA = 256-nA;
			p->bR = cGT.InvGamma((cGT.m_aGamma[p->bR]*nA + nBgR*nIA)>>8);
			p->bG = cGT.InvGamma((cGT.m_aGamma[p->bG]*nA + nBgG*nIA)>>8);
			p->bB = cGT.InvGamma((cGT.m_aGamma[p->bB]*nA + nBgB*nIA)>>8);
		}
	}
	HDC hDC = GetDC();
	BITMAPINFO tBMI;
	ZeroMemory(&tBMI, sizeof tBMI);
	tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
	tBMI.bmiHeader.biWidth = tSize.nX;
	tBMI.bmiHeader.biHeight = -tSize.nY;
	tBMI.bmiHeader.biPlanes = 1;
	tBMI.bmiHeader.biBitCount = 32;
	tBMI.bmiHeader.biCompression = BI_RGB;
	HBITMAP hBmp = CreateDIBitmap(hDC, &tBMI.bmiHeader, CBM_INIT, pData.m_p, &tBMI, DIB_RGB_COLORS);
	ReleaseDC(hDC);
	return hBmp;
}

LRESULT CNewFileDlg::OnSubMenuClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (HasChoices() != S_OK)
	{
		m_pClbk->ReportError(_SharedStringTable.GetStringAuto(IDS_ACTIONFAILED));
	}
	CImageList cImageList;
	cImageList.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 4, 4);
	Reset(cImageList);
	CComPtr<ILocalizedString> pName;
	HICON hIcon = NULL;
	ULONG nIcon = 0;
	CMenu cMenu;
	cMenu.CreatePopupMenu();
	TCHAR szTempl[64] = _T("");
	CComBSTR bstrTemplate;
	m_pSubmenuTemplate->GetLocalized(m_tLocaleID, &bstrTemplate);
	TCHAR szBuf[256] = _T("");
	HRESULT hRes;
	for (ULONG i = 0; E_RW_INDEXOUTOFRANGE != (hRes = GetChoiceProps(i, &pName, XPGUI::GetSmallIconSize(), &hIcon)); ++i, pName = NULL, hIcon = NULL)
	{
		if (FAILED(hRes) || pName == NULL) continue;

		CComBSTR bstr;
		pName->GetLocalized(m_tLocaleID, &bstr);
		_stprintf(szBuf, bstrTemplate.m_str, (LPCTSTR)(COLE2CT(bstr)));
		if (hIcon)
		{
			cImageList.AddIcon(hIcon);
			DestroyIcon(hIcon);
			AddItem(cMenu, i+1, szBuf, nIcon++);
		}
		else
		{
			AddItem(cMenu, i+1, szBuf, -1);
		}
	}
	RECT rc;
	m_wndOKButton.GetWindowRect(&rc);
	TPMPARAMS tTPMP = { sizeof(TPMPARAMS), rc };
	UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, rc.left, rc.bottom, m_hWnd, &tTPMP);
	cImageList.Destroy();
	if (nSelection != 0)
	{
		CWaitCursor cWait;
		if (FAILED(ClickedChoice(nSelection-1)))
		{
			m_pClbk->ReportError(_SharedStringTable.GetStringAuto(IDS_ACTIONFAILED));
		}
	}
	return 0;
}

LRESULT CNewFileDlg::OnItemActivateList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(a_pNMHDR);

	m_iActiveItem = pNMIA->iItem;

	m_pClbk->OnOKEx();

	return 0;
}

LRESULT CNewFileDlg::OnItemChangedList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(a_pNMHDR);

	int const iItem = pNMLV->iItem;
	if (pNMLV->uNewState == (LVIS_SELECTED|LVIS_FOCUSED) && iItem != m_iActiveItem)
	{
		m_iActiveItem = iItem;
		wchar_t szCLSID[64] = L"Wizard";
		StringFromGUID(m_cTemplates[m_iActiveItem].id, szCLSID+6);
		CComPtr<IConfig> pCfg;
		m_pAppConfig->SubConfigGet(CComBSTR(szCLSID), &pCfg);
		if (pCfg)
		{
			m_pConfigWnd->ConfigSet(pCfg, ECPMFull);
			m_pConfigWnd->Show(TRUE);
		}
		else
		{
			m_cTemplates[m_iActiveItem].p->Config(&pCfg);
			if (pCfg)
			{
				m_pConfigWnd->ConfigSet(pCfg, ECPMFull);
				m_pConfigWnd->Show(TRUE);
			}
			else
				m_pConfigWnd->Show(FALSE);
		}
		CComBSTR bstrID(CFGID_SELECTEDWIZARD);
		CComQIPtr<IDesignerWizardClipboard> pDWCB(m_cTemplates[m_iActiveItem].p);
		if (pDWCB == NULL) // don't save clipboard wizards as the active ones (they will get selected automatically if appropriate)
			m_pAppConfig->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(m_cTemplates[m_iActiveItem].id));

		SetDefaultButtonState(m_iActiveItem != -1 ? _SharedStringTable.GetStringAuto(IDS_STARTVIEWBTN_NEW).p : NULL, m_iActiveItem != -1, HasChoices() == S_OK);
	}
	else if (m_iActiveItem >= 0 && m_wndList.GetSelectedIndex() < 0)
	{
		PostMessage(WM_RESTORESELECTION);
	}

	return 0;
}

LRESULT CNewFileDlg::OnRestoreSelection(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_wndList.SelectItem(m_iActiveItem);
	return 0;
}

void CNewFileDlg::SetDefaultButtonState(ILocalizedString* a_pText, bool a_bEnabled, bool a_bChoices)
{
	if (a_pText)
	{
		CComBSTR bstr;
		a_pText->GetLocalized(m_tLocaleID, &bstr);
		m_wndOKButton.SetWindowText(COLE2CT(bstr));
		m_wndOKButton.EnableMenu(a_bChoices);
	}
	m_wndOKButton.EnableWindow(a_bEnabled);
}

LRESULT CNewFileDlg::OnGetInfoTipList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(a_pNMHDR);
	CComPtr<ILocalizedString> pStr;
	m_cTemplates[pGetInfoTip->iItem].p->HelpText(&pStr);
	CComBSTR bstr;
	pStr->GetLocalized(m_tLocaleID, &bstr);
	USES_CONVERSION;
	_tcsncpy(pGetInfoTip->pszText, OLE2CT(bstr), pGetInfoTip->cchTextMax);

	return 0;
}

HRESULT CNewFileDlg::HasChoices()
{
	if (ULONG(m_iActiveItem) >= m_cTemplates.size())
		return S_FALSE;
	bool bSecond = false;
	for (CBuilders::const_iterator i = m_cBuilders.begin(); i != m_cBuilders.end(); ++i)
		if (S_OK == m_cTemplates[m_iActiveItem].p->IsCompatible(1, &(i->p)))
			if (bSecond)
				return S_OK;
			else
				bSecond = true;
	return S_FALSE;
}

HRESULT CNewFileDlg::GetChoiceProps(ULONG a_nIndex, ILocalizedString** a_ppName, ULONG a_nSize, HICON* a_phIcon)
{
	if (ULONG(m_iActiveItem) >= m_cTemplates.size())
		return E_RW_INDEXOUTOFRANGE;
	for (CBuilders::const_iterator i = m_cBuilders.begin(); i != m_cBuilders.end(); ++i)
		if (S_OK == m_cTemplates[m_iActiveItem].p->IsCompatible(1, &(i->p)))
			if (a_nIndex == 0)
			{
				if (a_ppName)
					i->p->TypeName(a_ppName);
				if (a_phIcon)
					i->p->Icon(a_nSize, a_phIcon);
				return S_OK;
			}
			else
				--a_nIndex;
	return E_RW_INDEXOUTOFRANGE;
}

HRESULT CNewFileDlg::ClickedChoice(ULONG a_nIndex)
{
	if (ULONG(m_iActiveItem) >= m_cTemplates.size())
		return E_RW_INDEXOUTOFRANGE;

	HWND h = GetFocus();
	m_wndList.SetFocus(); // force update of the configuration panel

	CBuilders::const_iterator i;
	for (i = m_cBuilders.begin(); i != m_cBuilders.end(); ++i)
		if (S_OK == m_cTemplates[m_iActiveItem].p->IsCompatible(1, &(i->p)))
			if (a_nIndex == 0)
				break;
			else
				--a_nIndex;
	if (i == m_cBuilders.end())
		return E_RW_INDEXOUTOFRANGE;

	wchar_t szCLSID[64] = L"Wizard";
	StringFromGUID(m_cTemplates[m_iActiveItem].id, szCLSID+6);
	CComPtr<IConfig> pCfg;
	m_pAppConfig->SubConfigGet(CComBSTR(szCLSID), &pCfg);
	CComPtr<IDocument> pDoc;
	RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
	HRESULT hRes = m_cTemplates[m_iActiveItem].p->Activate(m_hWnd, m_tLocaleID, pCfg, 1, &(i->p), NULL, CComQIPtr<IDocumentBase>(pDoc));
	if (pDoc == NULL || hRes != S_OK)
	{
		return hRes;
	}

	CComPtr<IStorageFilter> pOldLoc;
	pDoc->LocationGet(&pOldLoc);
	CComQIPtr<IDocumentName> pDocName(pOldLoc);
	if (pDocName == NULL)
	{
		CComBSTR bstrName;
		CMultiLanguageString::GetLocalized(L"[0409]unnamed[0405]nepojmenovaný", m_tLocaleID, &bstrName);
		CComPtr<IConfig> pCfg;
		GUID tEncID = GUID_NULL;
		pDoc->EncoderGet(&tEncID, &pCfg);
		CComPtr<IDocumentEncoder> pEnc;
		if (!IsEqualGUID(tEncID, GUID_NULL)) RWCoCreateInstance(pEnc, tEncID);
		CComPtr<IDocumentType> pDocType;
		if (pEnc) pEnc->DocumentType(&pDocType);
		CComBSTR bstrType;
		if (pDocType != NULL)
		{
			pDocType->DefaultExtensionGet(&bstrType);
		}
		if (bstrType != NULL)
		{
			bstrName += L".";
			bstrName += bstrType;
		}
		CComObject<CDocumentName>* pName = NULL;
		CComObject<CDocumentName>::CreateInstance(&pName);
		CComPtr<IStorageFilter> pTmp = pName;
		pName->Init(bstrName);
		pDoc->LocationSet(pName);
	}

	m_pClbk->OpenDocument(pDoc);
	return S_OK;
}

