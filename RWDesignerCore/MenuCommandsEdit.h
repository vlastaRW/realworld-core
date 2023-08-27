
#pragma once

#include "MainFrame.h"
#include <PrintfLocalizedString.h>
#include <MultiLanguageString.h>
#include <IconRenderer.h>


extern __declspec(selectany) GUID const MenuCommandsClipboardID = {0x4ef24aa8, 0x63da, 0x4872, {0xa5, 0xd3, 0xd3, 0x83, 0x73, 0xa0, 0x3f, 0x7f}};

template<OLECHAR const* t_pszName, OLECHAR const* t_pszDesc, GUID const* t_pIconID,
	WORD t_wKeyCode1, WORD t_wVirtFlags1, WORD t_wKeyCode2, WORD t_wVirtFlags2,
	EDesignerViewClipboardAction t_eAction, bool t_bAllVisible, LPCWSTR t_pszNameTempl>
class ATL_NO_VTABLE CDocumentMenuCommandClipboard : 
	public CDocumentMenuCommandMLImpl<CDocumentMenuCommandClipboard<t_pszName, t_pszDesc, t_pIconID, t_wKeyCode1, t_wVirtFlags1, t_wKeyCode2, t_wVirtFlags2, t_eAction, t_bAllVisible, t_pszNameTempl>, t_pszName, t_pszDesc, t_pIconID, 0>,
	public ILocalizedString
{
public:
	void Init(IDesignerView* a_pView, IDocument* a_pDoc = NULL)
	{
		m_pView = a_pView;
		m_pDoc = a_pDoc;
	}

	typedef CDocumentMenuCommandClipboard<t_pszName, t_pszDesc, t_pIconID, t_wKeyCode1, t_wVirtFlags1, t_wKeyCode2, t_wVirtFlags2, t_eAction, t_bAllVisible, t_pszNameTempl> thisClass;

BEGIN_COM_MAP(thisClass)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	COM_INTERFACE_ENTRY(ILocalizedString)
END_COM_MAP()

	STDMETHOD(Get)(BSTR* a_pbstrString)
	{
		return GetLocalized(GetThreadLocale(), a_pbstrString);
	}
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
	{
		try
		{
			*a_pbstrString = NULL;
			CSortedViews cViews;
			GetViews(cViews);
			for (ULONG i = 0; i < cViews.size() && (t_bAllVisible || i < 1); ++i)
			{
				CComPtr<ILocalizedString> pName;
				CComBSTR bstrName;
				if ((/*cViews.size() == 1 || */S_OK == cViews[i].second->Check(t_eAction)) && SUCCEEDED(cViews[i].second->ObjectName(t_eAction, &pName)) && pName && SUCCEEDED(pName->GetLocalized(a_tLCID, &bstrName)) && bstrName.Length())
				{
					CComBSTR bstrTempl;
					CMultiLanguageString::GetLocalized(t_pszNameTempl, a_tLCID, &bstrTempl);
					OLECHAR szTmp[128] = L"";
					_snwprintf(szTmp, 128, bstrTempl, bstrName.m_str);
					CComBSTR bstr(szTmp);
					*a_pbstrString = bstr.Detach();
					return S_OK;
				}
			}
			CComBSTR bstrGeneric;
			CMultiLanguageString::GetLocalized(t_pszName, a_tLCID, &bstrGeneric);
			*a_pbstrString = bstrGeneric.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrString ? E_UNEXPECTED : E_POINTER;
		}
	}

	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			(*a_ppText = this)->AddRef();
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		try
		{
			*a_pIconID = GUID_NULL;

			CSortedViews cViews;
			GetViews(cViews);
			for (ULONG i = 0; i < cViews.size() && (t_bAllVisible || i < 1); ++i)
			{
				if (/*cViews.size() == 1 || */S_OK == cViews[i].second->Check(t_eAction))
				{
					cViews[i].second->ObjectIconID(t_eAction, a_pIconID);
					break;
				}
			}
			for (size_t i = 0; i < 4; ++i)
				reinterpret_cast<DWORD*>(a_pIconID)[i] ^= reinterpret_cast<DWORD const*>(t_pIconID)[i];

			return S_OK;
		}
		catch (...)
		{
			return a_pIconID ? E_UNEXPECTED : E_POINTER;
		}
	}
	virtual HICON BaseIcon(ULONG a_nSize, bool a_bOverlay) = 0;
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;

			BYTE bOverlay = 0;
			HICON hCustom = NULL;
			CSortedViews cViews;
			GetViews(cViews);
			for (ULONG i = 0; i < cViews.size() && (t_bAllVisible || i < 1); ++i)
			{
				CComPtr<ILocalizedString> pName;
				CComBSTR bstrName;
				if (/*cViews.size() == 1 || */S_OK == cViews[i].second->Check(t_eAction))
				{
					cViews[i].second->ObjectIcon(t_eAction, a_nSize, &hCustom, &bOverlay);
					break;
				}
			}
			if (hCustom && bOverlay == 0)
			{
				*a_phIcon = hCustom;
				return S_OK;
			}
			HICON hBase = BaseIcon(a_nSize, hCustom);
			if (hBase == NULL)
				return E_FAIL;
			if (hCustom == NULL)
			{
				*a_phIcon = hBase;
				return S_OK;
			}
			// blend the icons
			HICON hNew = BlendIcons(hCustom, hBase);
			if (hNew)
			{
				*a_phIcon = hNew;
				DestroyIcon(hBase);
				DestroyIcon(hCustom);
				return S_OK;
			}
			DestroyIcon(hCustom);
			*a_phIcon = hBase;
			return S_OK;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	static HICON BlendIcons(HICON a_hBase, HICON a_hOverlay)
	{
		ICONINFO tInfo;
		ZeroMemory(&tInfo, sizeof tInfo);
		GetIconInfo(a_hOverlay, &tInfo);
		if (tInfo.hbmMask)
			DeleteObject(tInfo.hbmMask);
		if (tInfo.hbmColor == NULL)
			return NULL;
		BITMAP tBmp;
		ZeroMemory(&tBmp, sizeof tBmp);
		GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
		if (tBmp.bmBitsPixel != 32)
		{
			DeleteObject(tInfo.hbmColor);
			return NULL;
		}
		int nSmSizeX = tBmp.bmWidth;
		int nSmSizeY = tBmp.bmHeight;
		DWORD nXOR = nSmSizeY*nSmSizeX<<2;
		DWORD nAND = nSmSizeY*((((nSmSizeX+7)>>3)+3)&0xfffffffc);
		CAutoVectorPtr<BYTE> pInterRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
		ZeroMemory(pInterRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);

		if (0 == GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pInterRes.m_p+sizeof BITMAPINFOHEADER))
		{
			DeleteObject(tInfo.hbmColor);
			return NULL; // failure
		}

		struct TRasterImagePixel { BYTE bR, bG, bB, bA; };
		TRasterImagePixel *pSrc = reinterpret_cast<TRasterImagePixel *>(pInterRes.m_p+sizeof BITMAPINFOHEADER);

		DeleteObject(tInfo.hbmColor);

		ZeroMemory(&tInfo, sizeof tInfo);
		GetIconInfo(a_hBase, &tInfo);
		if (tInfo.hbmMask)
			DeleteObject(tInfo.hbmMask);
		if (tInfo.hbmColor == NULL)
			return NULL;
		ZeroMemory(&tBmp, sizeof tBmp);
		GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
		if (tBmp.bmBitsPixel != 32 || nSmSizeX != tBmp.bmWidth || nSmSizeY != tBmp.bmHeight)
		{
			DeleteObject(tInfo.hbmColor);
			return NULL; // ignore low quality icons
		}
		CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
		ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);

		if (0 == GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pIconRes.m_p+sizeof BITMAPINFOHEADER))
		{
			DeleteObject(tInfo.hbmColor);
			return NULL; // failure
		}

		DeleteObject(tInfo.hbmColor);

		BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
		pHead->biSize = sizeof(BITMAPINFOHEADER);
		pHead->biWidth = nSmSizeX;
		pHead->biHeight = nSmSizeY<<1;
		pHead->biPlanes = 1;
		pHead->biBitCount = 32;
		pHead->biCompression = BI_RGB;
		pHead->biSizeImage = nXOR+nAND;
		pHead->biXPelsPerMeter = 0;
		pHead->biYPelsPerMeter = 0;
		TRasterImagePixel *pXOR = reinterpret_cast<TRasterImagePixel *>(pIconRes.m_p+sizeof BITMAPINFOHEADER);
		TRasterImagePixel *pDst = pXOR;

		for (int y = 0; y < tBmp.bmHeight; ++y)
		{
			for (int x = 0; x < tBmp.bmWidth; ++x)
			{
				if (pSrc->bA == 255)
				{
					*pDst = *pSrc;
				}
				else if (pSrc->bA != 0)
				{
					ULONG nR = ULONG(pSrc->bR)*pSrc->bA;
					ULONG nG = ULONG(pSrc->bG)*pSrc->bA;
					ULONG nB = ULONG(pSrc->bB)*pSrc->bA;
					ULONG const nA = pSrc->bA;
					// blend pixels
					ULONG nNewA = nA*255 + (255-nA)*pDst->bA;
					if (nNewA >= 255)
					{
						ULONG const bA1 = (255-nA)*pDst->bA;
						pDst->bR = (pDst->bR*bA1 + nR*255)/nNewA;
						pDst->bG = (pDst->bG*bA1 + nG*255)/nNewA;
						pDst->bB = (pDst->bB*bA1 + nB*255)/nNewA;
					}
					else
					{
						pDst->bR = pDst->bG = pDst->bB = 0;
					}
					pDst->bA = nNewA/255;
				}
				++pDst;
				++pSrc;
			}
		}
		pXOR = reinterpret_cast<TRasterImagePixel *>(pIconRes+sizeof BITMAPINFOHEADER);
		BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(nSmSizeY*nSmSizeX));
		int nANDLine = ((((nSmSizeX+7)>>3)+3)&0xfffffffc);
		for (int y = 0; y < nSmSizeY; ++y)
		{
			if (y < (nSmSizeY>>1))
			{
				for (int x = 0; x < nSmSizeX; ++x)
				{
					TRasterImagePixel t = pXOR[x];
					pXOR[x] = pXOR[(nSmSizeY-y-y-1)*nSmSizeX+x];
					pXOR[(nSmSizeY-y-y-1)*nSmSizeX+x] = t;
				}
			}
			for (int x = 0; x < nSmSizeX; ++x)
			{
				if (0 == pXOR->bA)
				{
					pAND[x>>3] |= 0x80 >> (x&7);
				}
				++pXOR;
			}
			pAND += nANDLine;
		}
		return CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, nSmSizeX, nSmSizeY, LR_DEFAULTCOLOR);
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			if (a_pAccel)
			{
				a_pAccel->wKeyCode = t_wKeyCode1;
				a_pAccel->fVirtFlags = t_wVirtFlags1;
			}
			if (a_pAuxAccel)
			{
				a_pAuxAccel->wKeyCode = t_wKeyCode2;
				a_pAuxAccel->fVirtFlags = t_wVirtFlags2;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	typedef std::vector<std::pair<ULONG, CComPtr<IDesignerViewClipboardHandler> > > CSortedViews;
	void GetViews(CSortedViews& a_cViews)
	{
		CComPtr<IEnumUnknownsInit> pViews;
		RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
		m_pView->QueryInterfaces(__uuidof(IDesignerViewClipboardHandler), EQIFActive, pViews);
		std::pair<ULONG, CComPtr<IDesignerViewClipboardHandler> > tItem;
		pViews->Get(0, __uuidof(IDesignerViewClipboardHandler), reinterpret_cast<void**>(&tItem.second));
		CComPtr<IDesignerViewClipboardHandler> pSkip;
		if (tItem.second)
		{
			tItem.first = 256;
			a_cViews.push_back(tItem);
			pSkip.Attach(tItem.second.Detach());
			if (!t_bAllVisible)
				return;
		}
		pViews = NULL;
		RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
		m_pView->QueryInterfaces(__uuidof(IDesignerViewClipboardHandler), EQIFVisible, pViews);
		ULONG i = 0;
		while (SUCCEEDED(pViews->Get(i, __uuidof(IDesignerViewClipboardHandler), reinterpret_cast<void**>(&tItem.second))) && tItem.second)
		{
			if (tItem.second != pSkip)
			{
				BYTE b = 128;
				tItem.second->Priority(&b);
				tItem.first = b;
				CSortedViews::const_iterator j = a_cViews.begin();
				while (j != a_cViews.end() && j->first >= b) ++j;
				if (j == a_cViews.end())
					a_cViews.push_back(tItem);
				else
					a_cViews.insert(j, tItem);
			}
			++i;
			tItem.second = NULL;
		}
	}
	EMenuCommandState IntState()
	{
		CSortedViews cViews;
		GetViews(cViews);
		for (ULONG i = 0; i < cViews.size() && (t_bAllVisible || i < 1); ++i)
		{
			HRESULT hRes = cViews[i].second->Check(t_eAction);
			if (hRes == S_OK)
				return EMCSNormal;
		}
		return EMCSDisabled;
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		try
		{
			CComPtr<ILocalizedString> pName;
			CComObject<CPrintfLocalizedString>* p = NULL;
			if (m_pDoc)
			{
				CComObject<CPrintfLocalizedString>::CreateInstance(&p);
				pName = p;
			}
			CSortedViews cViews;
			GetViews(cViews);
			CUndoBlock cUndo(m_pDoc, pName);
			for (ULONG i = 0; i < cViews.size() && (t_bAllVisible || i < 1); ++i)
			{
				HRESULT hRes = cViews[i].second->Exec(t_eAction);
				if (SUCCEEDED(hRes))
				{
					if (p)
					{
						CComPtr<ILocalizedString> pViewName;
						cViews[i].second->ObjectName(t_eAction, &pViewName);
						p->Init(CMultiLanguageString::GetAuto(t_pszNameTempl), pViewName);
					}
					return hRes;
				}
			}
			return E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IDesignerView> m_pView;
	CComPtr<IDocument> m_pDoc;
};

// Edit->Copy

extern __declspec(selectany) GUID const MenuCommandCopyID = {0x4145fbfc, 0x7759, 0x4c97, {0xa0, 0xb, 0x8b, 0xd5, 0xda, 0xce, 0xc5, 0xdb}};
extern __declspec(selectany) OLECHAR const MenuCommandCopyName[] = L"[0409]&Copy[0405]&Kopírovat";
extern __declspec(selectany) OLECHAR const MenuCommandCopyDesc[] = L"[0409]Copy the selection and put it on the Clipboard.[0405]Zkopírovat výběr do schránky.";
extern __declspec(selectany) OLECHAR const MenuCommandCopyTempl[] = L"[0409]&Copy %s[0405]&Kopírovat %s";
class CDocumentMenuCommandCopy : public CDocumentMenuCommandClipboard<
	MenuCommandCopyName, MenuCommandCopyDesc, &MenuCommandCopyID,
	'C', FCONTROL, VK_INSERT, FCONTROL|FVIRTKEY,
	EDVCACopy, false, MenuCommandCopyTempl>
{
	virtual HICON BaseIcon(ULONG a_nSize, bool a_bOverlay)
	{
		static IRPolyPoint const poly[] = {{0, 160}, {0, 256}, {256, 256}, {256, 160}, {176, 160}, {176, 99}, {227, 99}, {128, 0}, {29, 99}, {80, 99}, {80, 160}};

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));

		IRTarget target(a_bOverlay ? 0.65f : 0.8f, a_bOverlay ? 1.0f : 0.0f, a_bOverlay ? -1.0f : 0.0f);
		static IRGridItem const gridx[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
		static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 99.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 256.0f}};
		static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};

		CIconRendererReceiver cRenderer(a_nSize);
		cRenderer(&canvas, itemsof(poly), poly, pSI->GetMaterial(ESMManipulate), &target);
		return cRenderer.get();
	}
};

// Edit->Cut

extern __declspec(selectany) GUID const MenuCommandCutID = {0xc39074fc, 0xa980, 0x4e4a, {0xa1, 0xc3, 0x78, 0x6, 0xc6, 0x16, 0x38, 0x74}};
extern __declspec(selectany) OLECHAR const MenuCommandCutName[] = L"[0409]Cu&t[0405]Vyj&mout";
extern __declspec(selectany) OLECHAR const MenuCommandCutDesc[] = L"[0409]Cut the selection and put it on the Clipboard.[0405]Přesunout výběr do schránky.";
extern __declspec(selectany) OLECHAR const MenuCommandCutTempl[] = L"[0409]Cu&t %s[0405]Vyj&mout %s";
class CDocumentMenuCommandCut : public CDocumentMenuCommandClipboard<
	MenuCommandCutName, MenuCommandCutDesc, &MenuCommandCutID,
	'X', FCONTROL, VK_DELETE, FCONTROL|FVIRTKEY,
	EDVCACut, false, MenuCommandCutTempl>
{
	virtual HICON BaseIcon(ULONG a_nSize, bool a_bOverlay)
	{
		static IRPolyPoint const poly[] = {{0, 160}, {0, 256}, {128, 256}, {176, 208}, {176, 99}, {227, 99}, {128, 0}, {29, 99}, {80, 99}, {80, 160}};

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));

		IRTarget target(a_bOverlay ? 0.65f : 0.8f, a_bOverlay ? 1.0f : 0.0f, a_bOverlay ? -1.0f : 0.0f);
		static IRGridItem const gridx[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
		static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 99.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 256.0f}};
		static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};

		CIconRendererReceiver cRenderer(a_nSize);
		cRenderer(&canvas, itemsof(poly), poly, pSI->GetMaterial(ESMDelete), &target);
		return cRenderer.get();
	}
};

// Edit->Paste

extern __declspec(selectany) GUID const MenuCommandPasteID = {0x980c077b, 0x7d21, 0x4200, {0xa8, 0xbb, 0x9b, 0x4f, 0xb4, 0x7f, 0x60, 0x57}};
extern __declspec(selectany) OLECHAR const MenuCommandPasteName[] = L"[0409]&Paste[0405]&Vložit";
extern __declspec(selectany) OLECHAR const MenuCommandPasteDesc[] = L"[0409]Insert Clipboard contents.[0405]Vložit obsah schránky.";
extern __declspec(selectany) OLECHAR const MenuCommandPasteTempl[] = L"[0409]&Paste %s[0405]&Vložit %s";
class CDocumentMenuCommandPaste : public CDocumentMenuCommandClipboard<
	MenuCommandPasteName, MenuCommandPasteDesc, &MenuCommandPasteID,
	'V', FCONTROL, VK_INSERT, FSHIFT|FVIRTKEY,
	EDVCAPaste, true, MenuCommandPasteTempl>
{
	virtual HICON BaseIcon(ULONG a_nSize, bool a_bOverlay)
	{
		static IRPolyPoint const poly0[] = {{0, 160}, {0, 256}, {256, 256}, {256, 160}};
		static IRPolyPoint const poly1[] = {{80, 0}, {80, 114}, {29, 114}, {128, 213}, {227, 114}, {176, 114}, {176, 0}};

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));

		IRTarget target(a_bOverlay ? 0.65f : 0.8f, a_bOverlay ? 1.0f : 0.0f, a_bOverlay ? -1.0f : 0.0f);
		static IRGridItem const gridx[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
		static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 114.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 256.0f}};
		static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};

		CIconRendererReceiver cRenderer(a_nSize);
		cRenderer(&canvas, itemsof(poly0), poly0, pSI->GetMaterial(ESMManipulate), &target);
		cRenderer(&canvas, itemsof(poly1), poly1, pSI->GetMaterial(ESMManipulate), &target);
		return cRenderer.get();
	}
};


void EnumClipboard(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* a_pView, CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	{
		CComObject<CDocumentMenuCommandCut>* p = NULL;
		CComObject<CDocumentMenuCommandCut>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		p->Init(a_pView, a_pDocument);
		a_pCommands->Insert(pTmp);
	}
	{
		CComObject<CDocumentMenuCommandCopy>* p = NULL;
		CComObject<CDocumentMenuCommandCopy>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		p->Init(a_pView);
		a_pCommands->Insert(pTmp);
	}
	{
		CComObject<CDocumentMenuCommandPaste>* p = NULL;
		CComObject<CDocumentMenuCommandPaste>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		p->Init(a_pView, a_pDocument);
		a_pCommands->Insert(pTmp);
	}
}

// Edit->Select All

extern __declspec(selectany) GUID const MenuCommandsSelectAllID = {0x4966b68d, 0xcfae, 0x4847, {0x85, 0x9f, 0x5, 0xd, 0xda, 0x8b, 0x50, 0x3b}};
extern __declspec(selectany) OLECHAR const MenuCommandSelectAllName[] = L"[0409]Select &all[0405]V&ybrat vše";
extern __declspec(selectany) OLECHAR const MenuCommandSelectAllDesc[] = L"[0409]Mark all available items or objects as selected.[0405]Označit všechny dostupné položky nebo objekty jako vybrané.";
extern __declspec(selectany) OLECHAR const MenuCommandSelectAllTempl[] = L"[0409]Select &all (%s)[0405]V&ybrat vše (%s)";
class CDocumentMenuCommandSelectAll : public CDocumentMenuCommandClipboard<
	MenuCommandSelectAllName, MenuCommandSelectAllDesc, &MenuCommandsSelectAllID,
	'A', FCONTROL, 0, 0,
	EDVCASelectAll, false, MenuCommandSelectAllTempl>
{
	virtual HICON BaseIcon(ULONG a_nSize, bool a_bOverlay)
	{
		static IRPolyPoint const poly0[] = {{20, 20}, {20, 236}, {236, 236}, {236, 20}};
		static IRPolyPoint const poly1[] = {{48, 48}, {48, 112}, {112, 112}, {112, 48}};
		static IRPolyPoint const poly2[] = {{144, 48}, {144, 112}, {208, 112}, {208, 48}};
		static IRPolyPoint const poly3[] = {{48, 144}, {112, 144}, {112, 208}, {48, 208}};
		static IRPolyPoint const poly4[] = {{144, 144}, {144, 208}, {208, 208}, {208, 144}};
		static IRPolygon const shape[] =
		{
			{itemsof(poly1), poly1},
			{itemsof(poly2), poly2},
			{itemsof(poly3), poly3},
			{itemsof(poly4), poly4},
		};

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));

		IRTarget target(a_bOverlay ? 0.65f : 0.75f, a_bOverlay ? 0.875f : 0.0f, a_bOverlay ? -0.875f : 0.0f);
		static IRGridItem const grid[] = { {EGIFInteger, 20.0f}, {EGIFInteger, 236.0f}};
		static IRCanvas const canvas = {20, 20, 236, 236, itemsof(grid), itemsof(grid), grid, grid};

		CIconRendererReceiver cRenderer(a_nSize);
		cRenderer(&canvas, itemsof(poly0), poly0, pSI->GetMaterial(ESMManipulate), &target);
		cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMInterior), &target);
		return cRenderer.get();
	}
};

void EnumSelectAll(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* a_pView, CMainFrame* UNREF(a_pFrame), IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandSelectAll>* p = NULL;
	CComObject<CDocumentMenuCommandSelectAll>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pView);
	a_pCommands->Insert(pTmp);
}

// Edit->Invert Selection

extern __declspec(selectany) GUID const MenuCommandsInvertSelectionID = {0x18185cea, 0x16e9, 0x486d, {0xb0, 0xad, 0xec, 0x11, 0x71, 0x55, 0xe9, 0x97}};
extern __declspec(selectany) OLECHAR const MenuCommandInvertSelectionName[] = L"[0409]&Invert selection[0405]&Invertovat výběr";
extern __declspec(selectany) OLECHAR const MenuCommandInvertSelectionDesc[] = L"[0409]Select all objects that are currently not selected.[0405]Vybrat všechny položky, které nejsou aktuálně vybrané.";
extern __declspec(selectany) OLECHAR const MenuCommandInvertSelectionTempl[] = L"[0409]&Invert selection (%s)[0405]&Invertovat výběr (%s)";
class CDocumentMenuCommandInvertSelection : public CDocumentMenuCommandClipboard<
	MenuCommandInvertSelectionName, MenuCommandInvertSelectionDesc, &MenuCommandsInvertSelectionID,
	0, 0, 0, 0,
	EDVCAInvertSelection, false, MenuCommandInvertSelectionTempl>
{
	virtual HICON BaseIcon(ULONG a_nSize, bool a_bOverlay)
	{
		static IRPathPoint const path0[] =
		{
			{236, 236, 0, 0, 0, 0},
			{223, 236, 0, 0, 0, 0},
			{188, 201, -37.108, 35.5126, 0, 0},
			{56, 200, -36.5017, -36.5017, 36.5017, 36.5017},
			{55, 68, 0, 0, -35.5125, 37.108},
			{20, 33, 0, 0, 0, 0},
			{20, 20, 0, 0, 0, 0},
			{236, 20, 0, 0, 0, 0},
		};
		static IRPathPoint const path1[] =
		{
			{190, 66, -37.0031, -37.0031, 37.0031, 37.0031},
			{56, 66, 0, 0, 37.0031, -37.0031},
			{190, 200, 37.0031, -37.0031, 0, 0},
		};

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));

		IRTarget target(a_bOverlay ? 0.65f : 0.75f, a_bOverlay ? 0.875f : 0.0f, a_bOverlay ? -0.875f : 0.0f);
		static IRGridItem const grid[] = { {EGIFInteger, 20.0f}, {EGIFInteger, 236.0f}};
		static IRCanvas const canvas = {20, 20, 236, 236, itemsof(grid), itemsof(grid), grid, grid};

		CIconRendererReceiver cRenderer(a_nSize);
		cRenderer(&canvas, itemsof(path0), path0, pSI->GetMaterial(ESMManipulate), &target);
		cRenderer(&canvas, itemsof(path1), path1, pSI->GetMaterial(ESMInterior), &target);
		return cRenderer.get();
	}
};


void EnumInvertSelection(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* a_pView, CMainFrame* UNREF(a_pFrame), IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandInvertSelection>* p = NULL;
	CComObject<CDocumentMenuCommandInvertSelection>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pView);
	a_pCommands->Insert(pTmp);
}

// Edit->Delete

extern __declspec(selectany) GUID const MenuCommandsDeleteID = {0x36547bb8, 0x18, 0x4f3a, {0x84, 0xe5, 0x30, 0xf1, 0x15, 0xda, 0x77, 0xd1}};
extern __declspec(selectany) OLECHAR const MenuCommandDeleteName[] = L"[0409]&Delete[0405]&Odstranit";
extern __declspec(selectany) OLECHAR const MenuCommandDeleteDesc[] = L"[0409]Delete selected items.[0405]Odstranit vybrané položky.";
extern __declspec(selectany) OLECHAR const MenuCommandDeleteTempl[] = L"[0409]&Delete %s[0405]&Odstranit %s";
class CDocumentMenuCommandDelete : public CDocumentMenuCommandClipboard<
	MenuCommandDeleteName, MenuCommandDeleteDesc, &MenuCommandsDeleteID,
	VK_DELETE, 0, 0, 0,
	EDVCADelete, false, MenuCommandDeleteTempl>
{
	virtual HICON BaseIcon(ULONG a_nSize, bool a_bOverlay)
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);
		pSI->GetLayers(ESIDelete, cRenderer, IRTarget(a_bOverlay ? 0.65f : 0.8f, a_bOverlay ? 1.0f : 0.0f, a_bOverlay ? -1.0f : 0.0f));
		return cRenderer.get();
	}
};

void EnumDelete(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* a_pView, CMainFrame* UNREF(a_pFrame), IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandDelete>* p = NULL;
	CComObject<CDocumentMenuCommandDelete>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pView, a_pDocument);
	a_pCommands->Insert(pTmp);
}

// Edit->Duplicate

extern __declspec(selectany) GUID const MenuCommandsDuplicateID = {0x59949017, 0x3c2e, 0x40ed, {0x83, 0x33, 0x4e, 0x17, 0xb, 0x90, 0x47, 0x9}};
extern __declspec(selectany) OLECHAR const MenuCommandDuplicateName[] = L"[0409]Du&plicate[0405]&Duplikovat";
extern __declspec(selectany) OLECHAR const MenuCommandDuplicateDesc[] = L"[0409]Insert copies of selected items.[0405]Vložit kopie vybraných položek.";
extern __declspec(selectany) OLECHAR const MenuCommandDuplicateTempl[] = L"[0409]Du&plicate %s[0405]&Duplikovat %s";
class CDocumentMenuCommandDuplicate : public CDocumentMenuCommandClipboard<
	MenuCommandDuplicateName, MenuCommandDuplicateDesc, &MenuCommandsDuplicateID,
	0, 0, 0, 0,
	EDVCADuplicate, false, MenuCommandDuplicateTempl>
{
	virtual HICON BaseIcon(ULONG a_nSize, bool a_bOverlay)
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);
		pSI->GetLayers(ESIDuplicate, cRenderer, IRTarget(a_bOverlay ? 0.65f : 0.8f, a_bOverlay ? 1.0f : 0.0f, a_bOverlay ? -1.0f : 0.0f));
		return cRenderer.get();
	}
};

void EnumDuplicate(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* a_pView, CMainFrame* UNREF(a_pFrame), IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandDuplicate>* p = NULL;
	CComObject<CDocumentMenuCommandDuplicate>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pView, a_pDocument);
	a_pCommands->Insert(pTmp);
}


// Edit->Undo

extern __declspec(selectany) GUID const MenuCommandsUndoID = {0x1da53661, 0x695c, 0x4bc5, {0xb7, 0x4, 0x8c, 0x28, 0x26, 0x1a, 0xf2, 0x22}};
extern __declspec(selectany) GUID const MenuCommandUndoID = {0x79d59228, 0x84dc, 0x487a, {0xb6, 0xfe, 0x30, 0x8f, 0x40, 0x62, 0x6, 0xcc}};

class ATL_NO_VTABLE CDocumentMenuCommandMultiUndo : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandMultiUndo, 0, 0, NULL, 0>
{
public:
	void Init(CMainFrame* a_pFrame, IDesignerViewUndoOverride* a_pView, ULONG a_nSteps)
	{
		m_pFrame = a_pFrame;
		m_pView = a_pView;
		m_nSteps = a_nSteps;
	}

	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			if (m_pView && m_nSteps == 0)
			{
				return m_pView->UndoName(a_ppText);
				//*a_ppText = new CMultiLanguageString(L"[0409]Undo[0405]Zpět");
				//return S_OK;
			}
			CComPtr<ILocalizedString> pStep;
			m_pFrame->M_DocUndo()->StepName(FALSE, m_nSteps-1, &pStep);
			if (pStep)
			{
				*a_ppText = pStep.Detach();
				//CComObject<CPrintfLocalizedString>* p = NULL;
				//CComObject<CPrintfLocalizedString>::CreateInstance(&p);
				//CComPtr<ILocalizedString> pTmp = p;
				//p->Init(CMultiLanguageString::GetAuto(L"[0409]Undo \"%s\"[0405]Zpět \"%s\""), pStep);
				//*a_ppText = pTmp.Detach();
				return S_OK;
			}
			*a_ppText = new CMultiLanguageString(L"[0409]Undo[0405]Zpět");
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			*a_ppText = new CMultiLanguageString(L"[0409]Undo all operations upto this one.[0405]Vrátit zpět všechny kroky včetně tohoto.");
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	EMenuCommandState IntState()
	{
		return EMCSNormal;
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		try
		{
			ULONG nSteps = m_nSteps;
			if (m_pView)
			{
				HRESULT hRes = m_pView->Undo();
				if (nSteps == 0)
					return hRes;
			}
			return m_pFrame->M_DocUndo()->Undo(nSteps);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CMainFrame* m_pFrame;
	CComPtr<IDesignerViewUndoOverride> m_pView;
	ULONG m_nSteps;
};

class ATL_NO_VTABLE CDocumentMenuCommandUndo : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandUndo, IDS_MN_EDIT_UNDO, IDS_MD_EDIT_UNDO, &MenuCommandUndoID, 0>
{
public:
	void Init(CMainFrame* a_pFrame, IDesignerView* a_pView, bool a_bMultiStep)
	{
		m_pFrame = a_pFrame;
		m_pView = a_pView;
		m_eState = a_bMultiStep ? EMCSExecuteSubMenu : EMCSNormal;
	}

	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			static IRPathPoint const path[] =
			{
				{0, 240, 0, 0, 0, 0},
				{0, 184, 0, 0, 0, 0},
				{160, 184, 20, -32, 0, 0},
				{166, 103, -19.3573, -16.592, 21, 18},
				{89, 100, 0, 0, 30, -7},
				{100, 142, 0, 0, 0, 0},
				{1, 97, 0, 0, 0, 0},
				{67, 10, 0, 0, 0, 0},
				{78, 52, 39, -11, 0, 0},
				{196, 61, 41.849, 30.6212, -41, -30},
				{221, 184, 0, 0, 25, -45},
				{256, 184, 0, 0, 0, 0},
				{256, 240, 0, 0, 0, 0},
			};

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));

			static IRGridItem const gridX[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 256.0f}};
			static IRGridItem const gridY[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 184.0f}, {EGIFInteger, 240.0f}};
			static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};

			IRFill fill(0xffa4bd7e);
			IROutlinedFill mat(&fill, pSI->GetMaterial(ESMContrast));
			CIconRendererReceiver cRenderer(a_nSize);
			cRenderer(&canvas, itemsof(path), path, &mat);
			*a_phIcon = cRenderer.get();

			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			if (a_pAccel)
			{
				a_pAccel->wKeyCode = 'Z';
				a_pAccel->fVirtFlags = FCONTROL;
			}
			if (a_pAuxAccel)
			{
				a_pAuxAccel->wKeyCode = VK_BACK;
				a_pAuxAccel->fVirtFlags = FALT;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	EMenuCommandState IntState()
	{
		ULONG nSteps = 0;
		if (m_pFrame->M_DocUndo() && SUCCEEDED(m_pFrame->M_DocUndo()->StepCount(&nSteps, NULL)) && nSteps != 0)
			return static_cast<EMenuCommandState>(EMCSNormal|m_eState);
		if (m_pView)
		{
			CComPtr<IEnumUnknownsInit> pViews;
			RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
			m_pView->QueryInterfaces(__uuidof(IDesignerViewUndoOverride), EQIFVisible, pViews);
			ULONG i = 0;
			while (true)
			{
				CComPtr<IDesignerViewUndoOverride> pView;
				pViews->Get(i++, __uuidof(IDesignerViewUndoOverride), reinterpret_cast<void**>(&pView));
				if (pView == NULL)
					break;
				if (pView && S_OK == pView->CanUndo())
					return static_cast<EMenuCommandState>(EMCSNormal|m_eState);
			}
		}
		return static_cast<EMenuCommandState>(EMCSDisabled|m_eState);
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		try
		{
			if (m_pView)
			{
				CComPtr<IEnumUnknownsInit> pViews;
				RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
				m_pView->QueryInterfaces(__uuidof(IDesignerViewUndoOverride), EQIFVisible, pViews);
				ULONG i = 0;
				while (true)
				{
					CComPtr<IDesignerViewUndoOverride> pView;
					pViews->Get(i++, __uuidof(IDesignerViewUndoOverride), reinterpret_cast<void**>(&pView));
					if (pView == NULL)
						break;
					if (pView && S_OK == pView->CanUndo())
					{
						return pView->Undo();
					}
				}
			}
			return m_pFrame->M_DocUndo()->Undo(1);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;
			ULONG nSteps = 0;
			m_pFrame->M_DocUndo()->StepCount(&nSteps, NULL);
			CComPtr<IDesignerViewUndoOverride> pView;
			if (m_pView)
			{
				CComPtr<IEnumUnknownsInit> pViews;
				RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
				m_pView->QueryInterfaces(__uuidof(IDesignerViewUndoOverride), EQIFVisible, pViews);
				ULONG i = 0;
				while (true)
				{
					pViews->Get(i++, __uuidof(IDesignerViewUndoOverride), reinterpret_cast<void**>(&pView));
					if (pView == NULL || S_OK == pView->CanUndo())
						break;
					pView = NULL;
				}
			}
			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));
			if (pView)
			{
				CComObject<CDocumentMenuCommandMultiUndo>* p = NULL;
				CComObject<CDocumentMenuCommandMultiUndo>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pFrame, pView, 0);
				pCmds->Insert(pTmp);
			}
			if (nSteps > 10) nSteps = 10;
			for (ULONG i = 0; i < nSteps; ++i)
			{
				CComObject<CDocumentMenuCommandMultiUndo>* p = NULL;
				CComObject<CDocumentMenuCommandMultiUndo>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pFrame, pView, i+1);
				pCmds->Insert(pTmp);
			}
			*a_ppSubCommands = pCmds.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CMainFrame* m_pFrame;
	CComPtr<IDesignerView> m_pView;
	EMenuCommandState m_eState;
};

void EnumUndo(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* a_pView, CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	if (a_pFrame->M_DocUndo() == NULL)
		return;
	EUndoMode eMode = EUMAllSteps;
	a_pFrame->M_DocUndo()->UndoModeGet(&eMode);
	if (eMode == EUMDisabled)
		return;
	CComObject<CDocumentMenuCommandUndo>* p = NULL;
	CComObject<CDocumentMenuCommandUndo>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame, a_pView, eMode != EUMSingleStep);
	a_pCommands->Insert(pTmp);
}


extern __declspec(selectany) GUID const MenuCommandsRedoID = {0x418f9ffb, 0xb14e, 0x4388, {0xa4, 0xa3, 0x9d, 0xce, 0xb8, 0x7a, 0x72, 0x8b}};
extern __declspec(selectany) GUID const MenuCommandRedoID = {0x99d9639, 0x2202, 0x49b6, {0x85, 0x19, 0x32, 0x12, 0x5f, 0x5, 0x77, 0x90}};

class ATL_NO_VTABLE CDocumentMenuCommandRedo : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandRedo, IDS_MN_EDIT_REDO, IDS_MD_EDIT_REDO, &MenuCommandRedoID, 0>
{
public:
	void Init(CMainFrame* a_pFrame, IDesignerView* a_pView)
	{
		m_pFrame = a_pFrame;
		m_pView = a_pView;
	}

	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			static IRPathPoint const path[] =
			{
				{256, 240, 0, 0, 0, 0},
				{256, 184, 0, 0, 0, 0},
				{96, 184, -20, -32, 0, 0},
				{90, 103, 19.3573, -16.592, -21, 18},
				{167, 100, 0, 0, -30, -7},
				{156, 142, 0, 0, 0, 0},
				{255, 97, 0, 0, 0, 0},
				{189, 10, 0, 0, 0, 0},
				{178, 52, -39, -11, 0, 0},
				{60, 61, -41.849, 30.6212, 41, -30},
				{35, 184, 0, 0, -25, -45},
				{0, 184, 0, 0, 0, 0},
				{0, 240, 0, 0, 0, 0},
			};

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));

			static IRGridItem const gridX[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 256.0f}};
			static IRGridItem const gridY[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 184.0f}, {EGIFInteger, 240.0f}};
			static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};

			IRFill fill(0xffa4bd7e);
			IROutlinedFill mat(&fill, pSI->GetMaterial(ESMContrast));
			CIconRendererReceiver cRenderer(a_nSize);
			cRenderer(&canvas, itemsof(path), path, &mat);
			*a_phIcon = cRenderer.get();

			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			if (a_pAccel)
			{
				a_pAccel->wKeyCode = 'Y';
				a_pAccel->fVirtFlags = FCONTROL;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	EMenuCommandState IntState()
	{
		ULONG nSteps = 0;
		if (m_pFrame->M_DocUndo() && SUCCEEDED(m_pFrame->M_DocUndo()->StepCount(NULL, &nSteps)) && nSteps != 0)
			return EMCSNormal;
		//if (m_pView)
		//{
		//	CComPtr<IEnumUnknownsInit> pViews;
		//	RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
		//	m_pView->QueryInterfaces(__uuidof(IDesignerViewClipboardHandler), EQIFVisible, pViews);
		//	CComPtr<IDesignerViewUndoOverride> pView;
		//	pViews->Get(0, __uuidof(IDesignerViewUndoOverride), reinterpret_cast<void**>(&pView));
		//	if (pView && S_OK == pView->CanUndo())
		//		return EMCSNormal;
		//}
		return EMCSDisabled;
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		try
		{
			//if (m_pView)
			//{
			//	CComPtr<IEnumUnknownsInit> pViews;
			//	RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
			//	m_pView->QueryInterfaces(__uuidof(IDesignerViewClipboardHandler), EQIFVisible, pViews);
			//	CComPtr<IDesignerViewUndoOverride> pView;
			//	pViews->Get(0, __uuidof(IDesignerViewUndoOverride), reinterpret_cast<void**>(&pView));
			//	if (pView && S_OK == pView->CanUndo())
			//	{
			//		return pView->Undo();
			//	}
			//}
			return m_pFrame->M_DocUndo()->Redo(1);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CMainFrame* m_pFrame;
	CComPtr<IDesignerView> m_pView;
};

void EnumRedo(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* a_pView, CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	if (a_pFrame->M_DocUndo() == NULL)
		return;
	EUndoMode eMode = EUMAllSteps;
	a_pFrame->M_DocUndo()->UndoModeGet(&eMode);
	if (eMode == EUMDisabled)
		return;
	CComObject<CDocumentMenuCommandRedo>* p = NULL;
	CComObject<CDocumentMenuCommandRedo>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame, a_pView);
	a_pCommands->Insert(pTmp);
}

