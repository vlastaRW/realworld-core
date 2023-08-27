
#pragma once

namespace Win32LangEx
{
	inline HICON LoadIcon(HINSTANCE a_hInstance, LPCTSTR a_pszName, int a_nXDesired, int a_nYDesired, UINT a_fuLoad, WORD a_wLanguage, bool a_bLoadAnyLanguage = true)
	{
		// Find the icon directory identified by a_pszName.
		HRSRC hDirectory = FindResourceEx(a_hInstance, RT_GROUP_ICON, a_pszName, a_wLanguage);
		if (hDirectory == NULL && a_bLoadAnyLanguage)
		{
			hDirectory = FindResource(a_hInstance, a_pszName, RT_GROUP_ICON);
		}
		if (hDirectory == NULL) return NULL;

		// Load and lock the icon directory.
		HGLOBAL hDirMem = LoadResource(a_hInstance, hDirectory);
		if (hDirMem == NULL)
			return NULL;
		LPVOID pDirMem = LockResource(hDirMem);

		// Get the identifier of the icon that is most appropriate.
		int nID = LookupIconIdFromDirectoryEx(reinterpret_cast<PBYTE>(pDirMem), TRUE, a_nXDesired, a_nYDesired, a_fuLoad);

		// Find the bits for the nID icon.
		HRSRC hIcon = FindResource(a_hInstance, MAKEINTRESOURCE(nID), RT_ICON);
		if (hIcon == NULL)
			return NULL;

		// Load and lock the icon.
		HGLOBAL hIconMem = LoadResource(a_hInstance, hIcon);
		LPVOID pIconMem = LockResource(hIconMem);
	 
		// Create a handle to the icon.
		return CreateIconFromResourceEx(reinterpret_cast<PBYTE>(pIconMem), SizeofResource(a_hInstance, hIcon), TRUE, 0x00030000, a_nXDesired, a_nYDesired, a_fuLoad);
	}
	inline HICON LoadIcon(HINSTANCE a_hInstance, LPCTSTR a_pszName, WORD a_wLanguage, bool a_bLoadAnyLanguage = true)
	{
		return LoadIcon(a_hInstance, a_pszName, 0, 0, LR_DEFAULTCOLOR, a_wLanguage, a_bLoadAnyLanguage);
	}

	inline int LoadStringW(HINSTANCE a_hInstance, UINT a_uID, LPWSTR a_pszBuffer, size_t a_nBufferMax, WORD a_wLanguage, bool a_bLoadAnyLanguage = true)
	{
		HRSRC hBlock = FindResourceEx(a_hInstance, RT_STRING, MAKEINTRESOURCE((a_uID>>4)+1), a_wLanguage);
		bool bTranslate = false;
		if (hBlock == NULL && a_bLoadAnyLanguage)
		{
			hBlock = FindResourceEx(a_hInstance, RT_STRING, MAKEINTRESOURCE((a_uID>>4)+1), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT));
			if (hBlock)
				bTranslate = true;
			else
				hBlock = FindResource(a_hInstance, MAKEINTRESOURCE((a_uID>>4)+1), RT_STRING);
		}
		if (hBlock == NULL) return NULL;

		HGLOBAL hMem = LoadResource(a_hInstance, hBlock);
		WORD const* pMem = reinterpret_cast<WORD const*>(LockResource(hMem));
		for (a_uID &= 0xf; a_uID > 0; a_uID--) pMem += 1+*pMem;

		if (*pMem < a_nBufferMax)
		{
			int nLen = *pMem;
			CopyMemory(a_pszBuffer, pMem+1, nLen*sizeof(WORD));
			a_pszBuffer[nLen] = 0;
			if (bTranslate)
			{
				CComPtr<ITranslator> pTr;
				RWCoCreateInstance(pTr, __uuidof(Translator));
				if (pTr && SUCCEEDED(pTr->TranslateInPlace(ULONG(a_nBufferMax), a_pszBuffer, MAKELCID(a_wLanguage, SORT_DEFAULT))))
					return int(wcslen(a_pszBuffer));
			}
			return nLen;
		}

		return 0; // string does not fit into buffer
	}
	inline int LoadStringA(HINSTANCE a_hInstance, UINT a_uID, LPSTR a_pszBuffer, size_t a_nBufferMax, WORD a_wLanguage, bool a_bLoadAnyLanguage = true)
	{
		HRSRC hBlock = FindResourceEx(a_hInstance, RT_STRING, MAKEINTRESOURCE((a_uID>>4)+1), a_wLanguage);
		bool bTranslate = false;
		if (hBlock == NULL && a_bLoadAnyLanguage)
		{
			hBlock = FindResourceEx(a_hInstance, RT_STRING, MAKEINTRESOURCE((a_uID>>4)+1), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT));
			if (hBlock)
				bTranslate = true;
			else
				hBlock = FindResource(a_hInstance, MAKEINTRESOURCE((a_uID>>4)+1), RT_STRING);
		}
		if (hBlock == NULL) return NULL;

		HGLOBAL hMem = LoadResource(a_hInstance, hBlock);
		wchar_t const* pMem = reinterpret_cast<wchar_t const*>(LockResource(hMem));
		for (a_uID &= 0xf; a_uID > 0; a_uID--) pMem += 1+*pMem;

		if (bTranslate)
		{
			CComPtr<ITranslator> pTr;
			RWCoCreateInstance(pTr, __uuidof(Translator));
			if (pTr)
			{
				CComBSTR bstrOrig(*pMem, pMem+1);
				CComBSTR bstrLoc;
				if (SUCCEEDED(pTr->Translate(bstrOrig, MAKELCID(a_wLanguage, SORT_DEFAULT), &bstrLoc)) && bstrLoc.m_str)
				{
					int nWritten = WideCharToMultiByte(GetACP(), 0, bstrLoc, -1, a_pszBuffer, static_cast<int>(a_nBufferMax)-1, NULL, NULL);
					a_pszBuffer[nWritten] = '\0';
					return nWritten;
				}
			}
		}

		int nWritten = WideCharToMultiByte(GetACP(), 0, pMem+1, *pMem, a_pszBuffer, static_cast<int>(a_nBufferMax)-1, NULL, NULL);
		a_pszBuffer[nWritten] = '\0';
		return nWritten;
	}
//	inline int LoadString(HINSTANCE a_hInstance, UINT a_uID, LPTSTR a_pszBuffer, int a_nBufferMax, WORD a_wLanguage, bool a_bLoadAnyLanguage = true)
//	{
//#ifdef UNICODE
//		return LoadStringW(a_hInstance, a_uID, a_pszBuffer, a_nBufferMax, a_wLanguage, a_bLoadAnyLanguage);
//#else
//		return LoadStringA(a_hInstance, a_uID, a_pszBuffer, a_nBufferMax, a_wLanguage, a_bLoadAnyLanguage);
//#endif
//	}

	inline HMENU LoadMenu(HINSTANCE a_hInstance, LPCTSTR a_pszName, WORD a_wLanguage, bool a_bLoadAnyLanguage = true)
	{
		HRSRC hMenu = FindResourceEx(a_hInstance, RT_MENU, a_pszName, a_wLanguage);
		if (hMenu == NULL && a_bLoadAnyLanguage)
		{
			hMenu = FindResource(a_hInstance, a_pszName, RT_MENU);
		}
		if (hMenu == NULL) return NULL;

		HGLOBAL hMem = LoadResource(a_hInstance, hMenu);
		void const* pMem = LockResource(hMem);
		return LoadMenuIndirect(reinterpret_cast<MENUTEMPLATE const*>(pMem));
	}

#ifdef __ATLWIN_H__

	inline void GetDisplayFont(LOGFONT* a_pFont, WORD* a_pDefSize)
	{
		NONCLIENTMETRICS ncm;
		ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
		*a_pFont = ncm.lfMessageFont;
		if (a_pFont->lfHeight < 0)
			a_pFont->lfHeight = -a_pFont->lfHeight;

		static int nLogPixelsY = 0;
		if (nLogPixelsY == 0)
		{
			HDC hDC = ::GetDC(NULL);
			nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
			::ReleaseDC(NULL, hDC);
		}
		*a_pDefSize = (WORD)MulDiv(a_pFont->lfHeight, 72, nLogPixelsY);
	}
	inline bool UseCustomDialogFont() // true on Vista
	{
		static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		if (tVersion.dwMajorVersion == 0)
		{
			GetVersionEx(&tVersion);
		}
		return tVersion.dwMajorVersion >= 6;
	}

	inline bool GetDialogSize(HINSTANCE a_hInstance, UINT a_uID, SIZE* a_pSize, WORD a_wLanguage, bool a_bLoadAnyLanguage = true)
	{
		HRSRC hRes = FindResourceEx(a_hInstance, RT_DIALOG, MAKEINTRESOURCE(a_uID), a_wLanguage);
		if (hRes == NULL && a_bLoadAnyLanguage)
		{
			hRes = FindResourceEx(a_hInstance, RT_DIALOG, MAKEINTRESOURCE(a_uID), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT));
			if (hRes == NULL)
				hRes = FindResource(a_hInstance, MAKEINTRESOURCE(a_uID), RT_DIALOG);
		}
		if (hRes == NULL) return false;
		HGLOBAL hMem = LoadResource(a_hInstance, hRes);
		if (hMem == NULL) return false;
		DLGTEMPLATE const* pMem = reinterpret_cast<DLGTEMPLATE const*>(LockResource(hMem));
		if (UseCustomDialogFont() && _DialogSizeHelper::HasFont(pMem))
		{
			LOGFONT tLF;
			WORD wSize;
			GetDisplayFont(&tLF, &wSize);
			_DialogSizeHelper::GetSizeInDialogUnits(pMem, a_pSize);
			_DialogSizeHelper::ConvertDialogUnitsToPixels(tLF.lfFaceName, wSize, a_pSize, false);
			return true;
		}
		else
		{
			AtlGetDialogSize(pMem, a_pSize);
			return true;
		}
	}

	inline BYTE* LocalizeDialogTemplate(LPCDLGTEMPLATE a_pOrig, ULONG a_nLen, ITranslator* a_pTranslator, WORD a_tLanguageID, ULONG* a_pNewLen)
	{
		BYTE const* pStart = reinterpret_cast<BYTE const*>(a_pOrig);
		struct STmpl
		{
			STmpl() : nLen(0), nAlloc(0) {}
			void AddData(void const* a_pData, ULONG a_nLen, bool a_bDWAlign = false)
			{
				if (a_bDWAlign)
					nLen = (nLen+3)&~3;
				if (nLen+a_nLen > nAlloc)
				{
					ULONG nNew = (((nLen+a_nLen) > (nAlloc+(nAlloc>>1)) ? (nLen+a_nLen) : (nAlloc+(nAlloc>>1)))+3)&~3;
					BYTE* pNew = new BYTE[nNew];
					CopyMemory(pNew, cData.m_p, nLen);
					cData.Free();
					cData.Attach(pNew);
					nAlloc = nNew;
				}
				CopyMemory(cData.m_p+nLen, a_pData, a_nLen);
				nLen += a_nLen;
			}
			CAutoVectorPtr<BYTE> cData;
			ULONG nLen;
			ULONG nAlloc;
		};
		STmpl cNew;
		if (_DialogSizeHelper::IsDialogEx(a_pOrig))
		{
			_DialogSplitHelper::DLGTEMPLATEEX const* pEx = reinterpret_cast<_DialogSplitHelper::DLGTEMPLATEEX const*>(a_pOrig);
			cNew.AddData(pEx, sizeof*pEx);
			// menu
			wchar_t const* pW = reinterpret_cast<wchar_t const*>(pEx+1);
			size_t nLen;
			if (*pW == 0x00ff)
			{
				cNew.AddData(pW, 4);
				pW += 2;
			}
			else
			{
				nLen = wcslen(pW);
				cNew.AddData(pW, (nLen+1)*2);
				pW += 1+nLen;
			}
			// class
			nLen = wcslen(pW);
			cNew.AddData(pW, (nLen+1)*2);
			pW += 1+nLen;
			// caption
			nLen = wcslen(pW);
			CComBSTR bstrCap;
			if (nLen)
				a_pTranslator->Translate(CComBSTR(pW), MAKELCID(a_tLanguageID, SORT_DEFAULT), &bstrCap);
			if (bstrCap.m_str)
			{
				cNew.AddData(bstrCap.m_str, (bstrCap.Length()+1)*2);
			}
			else
			{
				cNew.AddData(pW, (nLen+1)*2);
			}
			pW += 1+nLen;
			// font
			if (_DialogSizeHelper::HasFont(a_pOrig))
			{
				cNew.AddData(pW, 6);
				pW += 3;
				nLen = wcslen(pW);
				cNew.AddData(pW, (nLen+1)*2);
				pW += 1+nLen;
			}
			BYTE const* pAlign = reinterpret_cast<BYTE const*>(pW);
			for (WORD nCtl = 0; nCtl < pEx->cDlgItems; ++nCtl)
			{
				pStart = pStart+(((pAlign-pStart)+3)&~3);
				_DialogSplitHelper::DLGITEMTEMPLATEEX const* pItem = reinterpret_cast<_DialogSplitHelper::DLGITEMTEMPLATEEX const*>(pStart);
				cNew.AddData(pItem, sizeof*pItem, true);
				pW = reinterpret_cast<wchar_t const*>(pItem+1);
				// class
				if (*pW == 0xffff)
				{
					cNew.AddData(pW, 4);
					pW += 2;
				}
				else
				{
					size_t nLen = wcslen(pW);
					cNew.AddData(pW, (nLen+1)*2);
					pW += 1+nLen;
				}
				// caption
				if (*pW == 0xffff)
				{
					cNew.AddData(pW, 4);
					pW += 2;
				}
				else
				{
					size_t nLen = wcslen(pW);
					CComBSTR bstrTxt;
					if (nLen)
						a_pTranslator->Translate(CComBSTR(pW), MAKELCID(a_tLanguageID, SORT_DEFAULT), &bstrTxt);
					if (bstrTxt.m_str)
					{
						cNew.AddData(bstrTxt.m_str, (bstrTxt.Length()+1)*2);
					}
					else
					{
						cNew.AddData(pW, (nLen+1)*2);
					}
					pW += 1+nLen;
				}
				// extra data
				cNew.AddData(pW, 2+*pW);
				pAlign = reinterpret_cast<BYTE const*>(pW)+2+*pW;
			}
		}
		else
		{
			cNew.AddData(a_pOrig, sizeof*a_pOrig);
			// menu
			wchar_t const* pW = reinterpret_cast<wchar_t const*>(a_pOrig+1);
			size_t nLen;
			if (*pW == 0x00ff)
			{
				cNew.AddData(pW, 4);
				pW += 2;
			}
			else
			{
				nLen = wcslen(pW);
				cNew.AddData(pW, (nLen+1)*2);
				pW += 1+nLen;
			}
			// class
			nLen = wcslen(pW);
			cNew.AddData(pW, (nLen+1)*2);
			pW += 1+nLen;
			// caption
			nLen = wcslen(pW);
			CComBSTR bstrCap;
			if (nLen)
				a_pTranslator->Translate(CComBSTR(pW), MAKELCID(a_tLanguageID, SORT_DEFAULT), &bstrCap);
			if (bstrCap.m_str)
			{
				cNew.AddData(bstrCap.m_str, (bstrCap.Length()+1)*2);
			}
			else
			{
				cNew.AddData(pW, (nLen+1)*2);
			}
			pW += 1+nLen;
			// font
			if (_DialogSizeHelper::HasFont(a_pOrig))
			{
				cNew.AddData(pW, 2);
				pW += 1;
				nLen = wcslen(pW);
				cNew.AddData(pW, (nLen+1)*2);
				pW += 1+nLen;
			}
			BYTE const* pAlign = reinterpret_cast<BYTE const*>(pW);
			for (WORD nCtl = 0; nCtl < a_pOrig->cdit; ++nCtl)
			{
				pStart = pStart+(((pAlign-pStart)+3)&~3);
				DLGITEMTEMPLATE const* pItem = reinterpret_cast<DLGITEMTEMPLATE const*>(pStart);
				cNew.AddData(pItem, sizeof*pItem, true);
				pW = reinterpret_cast<wchar_t const*>(pItem+1);
				// class
				if (*pW == 0xffff)
				{
					cNew.AddData(pW, 4);
					pW += 2;
				}
				else
				{
					size_t nLen = wcslen(pW);
					cNew.AddData(pW, (nLen+1)*2);
					pW += 1+nLen;
				}
				// caption
				if (*pW == 0xffff)
				{
					cNew.AddData(pW, 4);
					pW += 2;
				}
				else
				{
					size_t nLen = wcslen(pW);
					CComBSTR bstrTxt;
					if (nLen)
						a_pTranslator->Translate(CComBSTR(pW), MAKELCID(a_tLanguageID, SORT_DEFAULT), &bstrTxt);
					if (bstrTxt.m_str)
					{
						cNew.AddData(bstrTxt.m_str, (bstrTxt.Length()+1)*2);
					}
					else
					{
						cNew.AddData(pW, (nLen+1)*2);
					}
					pW += 1+nLen;
				}
				// extra data
				cNew.AddData(pW, 2+*pW);
				pAlign = reinterpret_cast<BYTE const*>(pW)+2+*pW;
			}
		}
		if (a_pNewLen)
			*a_pNewLen = cNew.nLen;
		return cNew.cData.Detach();
	}

	inline BYTE* ModifyFontDialog(LPCDLGTEMPLATE a_pOrig, ULONG a_nLen)
	{
		if (_DialogSizeHelper::HasFont(a_pOrig))
		{
			BYTE* pFontDim = _DialogSizeHelper::GetFontSizeField(a_pOrig);
			LOGFONT tLF;
			WORD wSize;
			GetDisplayFont(&tLF, &wSize);
			CT2W strNew(tLF.lfFaceName);
			int nNewFLen = wcslen(strNew);
			BYTE* pFontName = pFontDim + sizeof(WORD) * (_DialogSizeHelper::IsDialogEx(a_pOrig) ? 3 : 1);
			int nOldFLen = wcslen(reinterpret_cast<wchar_t const*>(pFontName));
			BYTE* p = new BYTE[a_nLen+(nNewFLen-nOldFLen)*sizeof(wchar_t)+4]; // for eventual padding
			BYTE const* pOrig = reinterpret_cast<BYTE const*>(a_pOrig);
			CopyMemory(p, a_pOrig, pFontName-pOrig);
			*reinterpret_cast<WORD*>(p+(pFontDim-pOrig)) = wSize;
			CopyMemory(p+(pFontName-pOrig), strNew.operator LPWSTR(), (nNewFLen+1)*sizeof(wchar_t));
			if (_DialogSizeHelper::IsDialogEx(a_pOrig))
			{
				BYTE const* pOrigRest = pOrig+(((pFontName-pOrig)+(nOldFLen+1)*sizeof(wchar_t)+3)&~3);
				if ((pOrig+a_nLen) > pOrigRest)
					CopyMemory(p+(((pFontName-pOrig)+(nNewFLen+1)*sizeof(wchar_t)+3)&~3),
						pOrigRest, a_nLen-(pOrigRest-pOrig));
			}
			else
			{
				if (a_nLen > ((pFontName-pOrig)+(nOldFLen+1)*sizeof(wchar_t)))
					CopyMemory(p+(pFontName-pOrig)+(nNewFLen+1)*sizeof(wchar_t),
						pOrig+(pFontName-pOrig)+(nOldFLen+1)*sizeof(wchar_t), a_nLen-(pFontName-pOrig)-(nOldFLen+1)*sizeof(wchar_t));
			}
			return p;
		}
		else
		{
			// TODO: set font to dialogs without a font specified
			BYTE* p = new BYTE[a_nLen];
			CopyMemory(p, a_pOrig, a_nLen);
			return p;
		}
	}

	template <class T, class TBase = CWindow, bool t_bLoadAnyLanguage = true>
	class ATL_NO_VTABLE CLangDialogImpl : public CDialogImpl<T, TBase>
	{
	public:
		CLangDialogImpl(LCID a_tLocaleID = GetThreadLocale()) : m_tLocaleID(a_tLocaleID)
		{
		}

		// modal dialogs
		INT_PTR DoModal(HWND a_hWndParent = ::GetActiveWindow(), LPARAM a_dwInitParam = NULL)
		{
			ATLASSERT(a_hWndParent != GetDesktopWindow());
			if (a_hWndParent) a_hWndParent = GetAncestor(a_hWndParent, GA_ROOT);

			m_bEndModalWithPreTranslate = false;
			static_cast<T*>(this)->Create(a_hWndParent, a_dwInitParam);
			ShowWindow(SW_SHOW);
			if (m_hWnd == NULL)
				return -1;

			if (a_hWndParent)
				::EnableWindow(a_hWndParent, FALSE);
			MSG msg = {NULL, WM_NULL, 0, 0, 0, {0, 0}};
			while (!m_bEndModalWithPreTranslate)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						break;
					}
					if (msg.message == WM_MOUSEWHEEL)
					{
						// forward mouse wheel message to the window under the mouse pointer
						POINT tPt = {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)};
						HWND hWnd = WindowFromPoint(tPt);
						if (hWnd)
						{
							DWORD dwPID = 0;
							GetWindowThreadProcessId(hWnd, &dwPID);
							if (dwPID && GetCurrentProcessId() == dwPID)
							{
								msg.hwnd = hWnd;
								DispatchMessage(&msg);
								continue;
							}
						}
					}
					if (!static_cast<T*>(this)->DoModalPreTranslate(&msg) && !IsDialogMessage(&msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				else if (!m_bEndModalWithPreTranslate)
				{
					WaitMessage();
				}
			}
			if (msg.message == WM_QUIT)
			{
				PostQuitMessage((int)msg.wParam);
			}
			::EnableWindow(a_hWndParent, TRUE);
			DestroyWindow();
			return m_nResultModalWithPreTranslate;
		}
		void EndDialog(int iResult)
		{
			m_nResultModalWithPreTranslate = iResult;
			m_bEndModalWithPreTranslate = true;
		}

		// modeless dialogs
		HWND Create(HWND a_hWndParent, LPARAM a_dwInitParam = NULL)
		{
			ATLASSERT(m_hWnd == NULL);
			_AtlWinModule.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
#ifdef _DEBUG
			m_bModal = false;
#endif //_DEBUG

			bool bLocalize = false;
			HRSRC hDlg = FindResourceEx(_pModule->get_m_hInst(), RT_DIALOG, MAKEINTRESOURCE(static_cast<T*>(this)->IDD), LANGIDFROMLCID(m_tLocaleID));
			if (hDlg == NULL && t_bLoadAnyLanguage)
			{
				hDlg = FindResourceEx(_pModule->get_m_hInst(), RT_DIALOG, MAKEINTRESOURCE(static_cast<T*>(this)->IDD), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT));
				if (hDlg)
					bLocalize = true;
				else
					hDlg = FindResource(_pModule->get_m_hInst(), MAKEINTRESOURCE(static_cast<T*>(this)->IDD), RT_DIALOG);
			}
			if (hDlg == NULL) return 0;

			HGLOBAL hMem = LoadResource(_pModule->get_m_hInst(), hDlg);
			LPCDLGTEMPLATE pMem = reinterpret_cast<LPCDLGTEMPLATE>(LockResource(hMem));
			ULONG nMem = SizeofResource(_pModule->get_m_hInst(), hDlg);

			CAutoVectorPtr<BYTE> pTmp;
			if (bLocalize)
			{
				CComPtr<ITranslator> pTr;
				RWCoCreateInstance(pTr, __uuidof(Translator));
				if (pTr)
				{
					ULONG nNewLen = 0;
					pTmp.Attach(LocalizeDialogTemplate(pMem, nMem, pTr, LANGIDFROMLCID(m_tLocaleID), &nNewLen));
					pMem = reinterpret_cast<LPCDLGTEMPLATE>(pTmp.m_p);
					nMem = nNewLen;
				}
			}

			if (UseCustomDialogFont())
			{
				CAutoVectorPtr<BYTE> pNewTemplate(ModifyFontDialog(pMem, nMem));
				HWND hWnd = ::CreateDialogIndirectParam(_pModule->get_m_hInst(), reinterpret_cast<LPCDLGTEMPLATE>(pNewTemplate.m_p), a_hWndParent, T::StartDialogProc, a_dwInitParam);
				ATLASSERT(m_hWnd == hWnd);
				return hWnd;
			}
			else
			{
				HWND hWnd = ::CreateDialogIndirectParam(_pModule->get_m_hInst(), pMem, a_hWndParent, T::StartDialogProc, a_dwInitParam);
				ATLASSERT(m_hWnd == hWnd);
				return hWnd;
			}
		}
		// for CComControl
		HWND Create(HWND a_hWndParent, RECT&, LPARAM a_dwInitParam = NULL)
		{
			return Create(a_hWndParent, a_dwInitParam);
		}

		bool GetDialogSize(SIZE* a_pSize, WORD a_wLanguage, bool a_bLoadAnyLanguage = true)
		{
			return Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), T::IDD, a_pSize, a_wLanguage, a_bLoadAnyLanguage);
		}

		// to be overriden
		bool DoModalPreTranslate(MSG const*) { return false; }

		LCID m_tLocaleID;

	private:
		bool m_bEndModalWithPreTranslate;
		int m_nResultModalWithPreTranslate;
	};

#endif//__ATLWIN_H__

#ifdef __ATLFRAME_H__

	template <class T, class TBase = CWindow, class TWinTraits = CFrameWinTraits, bool t_bLoadAnyLanguage = true>
	class ATL_NO_VTABLE CLangFrameWindowImpl : public CFrameWindowImpl<T, TBase, TWinTraits>
	{
	public:
		CLangFrameWindowImpl(LCID a_tLocaleID = GetThreadLocale()) : m_tLocaleID(a_tLocaleID)
		{
		}

		typedef CFrameWindowImpl<T, TBase, TWinTraits> baseClass;

		BEGIN_MSG_MAP(thisClass)
			NOTIFY_CODE_HANDLER(TTN_GETDISPINFOA, OnToolTipTextA)
			NOTIFY_CODE_HANDLER(TTN_GETDISPINFOW, OnToolTipTextW)
			CHAIN_MSG_MAP(baseClass)
			MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		END_MSG_MAP()

		LRESULT OnMenuSelect(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			bHandled = FALSE;

			if(m_hWndStatusBar == NULL)
				return 1;

			WORD wFlags = HIWORD(wParam);
			if(wFlags != 0xFFFF || lParam != NULL)
			{
				TCHAR szBuff[256];
				szBuff[0] = 0;
				if(!(wFlags & MF_POPUP))
				{
					WORD wID = LOWORD(wParam);
					// check for special cases
					if(wID >= 0xF000 && wID < 0xF1F0)				// system menu IDs
						wID = (WORD)(((wID - 0xF000) >> 4) + ATL_IDS_SCFIRST);
					else if(wID >= ID_FILE_MRU_FIRST && wID <= ID_FILE_MRU_LAST)	// MRU items
						wID = ATL_IDS_MRU_FILE;
					else if(wID >= ATL_IDM_FIRST_MDICHILD)				// MDI child windows
						wID = ATL_IDS_MDICHILD;

					size_t nRet = LoadString(_pModule->get_m_hInst(), wID, szBuff, itemsof(szBuff), LANGIDFROMLCID(m_tLocaleID), t_bLoadAnyLanguage);
					for(size_t i = 0; i < nRet; i++)
					{
						if(szBuff[i] == _T('\n'))
						{
							szBuff[i] = 0;
							break;
						}
					}
				}
				::SendMessage(m_hWndStatusBar, SB_SIMPLE, TRUE, 0L);
				::SendMessage(m_hWndStatusBar, SB_SETTEXT, (255 | SBT_NOBORDERS), (LPARAM)szBuff);
			}

			return 1;
		}

		LRESULT OnToolTipTextA(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/)
		{
			LPNMTTDISPINFOA pDispInfo = (LPNMTTDISPINFOA)pnmh;
			pDispInfo->szText[0] = 0;

			if((idCtrl != 0) && !(pDispInfo->uFlags & TTF_IDISHWND))
			{
				char szBuff[256];
				szBuff[0] = 0;
				size_t nRet = LoadStringA(_pModule->get_m_hInst(), idCtrl, szBuff, itemsof(szBuff), LANGIDFROMLCID(m_tLocaleID), t_bLoadAnyLanguage);
				for(size_t i = 0; i < nRet; i++)
				{
					if(szBuff[i] == '\n')
					{
						strncpy(pDispInfo->szText, &szBuff[i + 1], sizeof(pDispInfo->szText) / sizeof(pDispInfo->szText[0]));
						break;
					}
				}
#if (_WIN32_IE >= 0x0300)
				if(nRet > 0)	// string was loaded, save it
					pDispInfo->uFlags |= TTF_DI_SETITEM;
#endif //(_WIN32_IE >= 0x0300)
			}

			return 0;
		}

		LRESULT OnToolTipTextW(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/)
		{
			LPNMTTDISPINFOW pDispInfo = (LPNMTTDISPINFOW)pnmh;
			pDispInfo->szText[0] = 0;

			if((idCtrl != 0) && !(pDispInfo->uFlags & TTF_IDISHWND))
			{
				wchar_t szBuff[256];
				szBuff[0] = 0;
				size_t nRet = LoadStringW(_pModule->get_m_hInst(), idCtrl, szBuff, itemsof(szBuff), LANGIDFROMLCID(m_tLocaleID), t_bLoadAnyLanguage);
				for(size_t i = 0; i < nRet; i++)
				{
					if(szBuff[i] == L'\n')
					{
						wcsncpy(pDispInfo->szText, &szBuff[i + 1], sizeof(pDispInfo->szText) / sizeof(pDispInfo->szText[0]));
						break;
					}
				}
#if (_WIN32_IE >= 0x0300)
				if(nRet > 0)	// string was loaded, save it
					pDispInfo->uFlags |= TTF_DI_SETITEM;
#endif //(_WIN32_IE >= 0x0300)
			}

			return 0;
		}

		BOOL CreateSimpleStatusBar(UINT nTextID = ATL_IDS_IDLEMESSAGE, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP, UINT nID = ATL_IDW_STATUS_BAR)
		{
			TCHAR szText[128];	// max text lentgth is 127 for status bars
			szText[0] = 0;
			LoadString(_pModule->get_m_hInst(), nTextID, szText, itemsof(szText), LANGIDFROMLCID(m_tLocaleID), t_bLoadAnyLanguage);
			return baseClass::CreateSimpleStatusBar(szText, dwStyle, nID);
		}

		LCID m_tLocaleID;
	};

#endif//__ATLFRAME_H__

#ifdef __ATLDLGS_H__

	#define DIALOG_FONT_AUTO() \
		ATLASSERT(bExTemplate); \
		static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") }; \
		if (tVersion.dwMajorVersion == 0) \
			GetVersionEx(&tVersion); \
		NONCLIENTMETRICS ncm; \
		if (tVersion.dwMajorVersion >= 6) \
		{ \
			ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS(); \
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0); \
			szFontName = ncm.lfMessageFont.lfFaceName; \
			static int nLogPixelsY = 0; \
			if (nLogPixelsY == 0) \
			{ \
				HDC hDC = ::GetDC(NULL); \
				nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY); \
				::ReleaseDC(NULL, hDC); \
			} \
			wFontSize = (WORD)MulDiv(ncm.lfMessageFont.lfHeight < 0 ? -ncm.lfMessageFont.lfHeight : ncm.lfMessageFont.lfHeight, 72, nLogPixelsY); \
			wWeight = ncm.lfMessageFont.lfWeight; \
			bItalic = ncm.lfMessageFont.lfItalic; \
			bCharset = ncm.lfMessageFont.lfCharSet; \
		} \
		else \
		{ \
			wFontSize = 8; \
			szFontName = _T("MS Shell Dlg"); \
			wWeight = 400; \
			bItalic = 0; \
			bCharset = 0x1; \
		}

	template <class T, class TDlgTemplate = CMemDlgTemplate, class TBase = ATL::CDialogImpl<T, ATL::CWindow> >
	class ATL_NO_VTABLE CLangIndirectDialogImpl : public TBase
	{
	public:
		enum { IDD = 0 };   // no dialog template resource

		CLangIndirectDialogImpl(LCID a_tLocaleID = GetThreadLocale()) : m_tLocaleID(a_tLocaleID), m_Template(&m_tLocaleID, &m_OrigTemplate)
		{
		}

		struct CTemplTranslator
		{
			enum StdCtrlType
			{
				CTRL_BUTTON = 0x0080,
				CTRL_EDIT = 0x0081,
				CTRL_STATIC = 0x0082,
				CTRL_LISTBOX = 0x0083,
				CTRL_SCROLLBAR = 0x0084,
				CTRL_COMBOBOX = 0x0085
			};

			CTemplTranslator(LCID* a_pLocaleID, TDlgTemplate* a_pTemplate) : m_pLocaleID(a_pLocaleID), m_pTemplate(a_pTemplate) {}

			void Create(bool bDlgEx, LPCTSTR lpszCaption, short nX, short nY, short nWidth, short nHeight, DWORD dwStyle = 0, DWORD dwExStyle = 0, 
						LPCTSTR lpstrFontName = NULL, WORD wFontSize = 0, WORD wWeight = 0, BYTE bItalic = 0, BYTE bCharset = 0, DWORD dwHelpID = 0,
						ATL::_U_STRINGorID ClassName = 0U, ATL::_U_STRINGorID Menu = 0U)
			{
				if (lpszCaption && *lpszCaption)
				{
					CAutoVectorPtr<TCHAR> pszTran;
					Translate(lpszCaption, pszTran);
					m_pTemplate->Create(bDlgEx, pszTran.m_p, nX, nY, nWidth, nHeight, dwStyle, dwExStyle, lpstrFontName, wFontSize, wWeight, bItalic, bCharset, dwHelpID, ClassName, Menu);
				}
				else
				{
					m_pTemplate->Create(bDlgEx, lpszCaption, nX, nY, nWidth, nHeight, dwStyle, dwExStyle, lpstrFontName, wFontSize, wWeight, bItalic, bCharset, dwHelpID, ClassName, Menu);
				}
			}

			void AddControl(ATL::_U_STRINGorID ClassName, WORD wId, short nX, short nY, short nWidth, short nHeight, DWORD dwStyle, DWORD dwExStyle,
							ATL::_U_STRINGorID Text, const WORD* pCreationData = NULL, WORD nCreationData = 0, DWORD dwHelpID = 0)
			{
				if (Text.m_lpstr && !IS_INTRESOURCE(Text.m_lpstr) && *Text.m_lpstr)
				{
					CAutoVectorPtr<TCHAR> pszTran;
					Translate(Text.m_lpstr, pszTran);
					m_pTemplate->AddControl(ClassName, wId, nX, nY, nWidth, nHeight, dwStyle, dwExStyle, pszTran.m_p, pCreationData, nCreationData, dwHelpID);
				}
				else
				{
					m_pTemplate->AddControl(ClassName, wId, nX, nY, nWidth, nHeight, dwStyle, dwExStyle, Text, pCreationData, nCreationData, dwHelpID);
				}
			}

			void AddStdControl(StdCtrlType CtrlType, WORD wId, short nX, short nY, short nWidth, short nHeight,
							   DWORD dwStyle, DWORD dwExStyle, ATL::_U_STRINGorID Text, const WORD* pCreationData = NULL, WORD nCreationData = 0, DWORD dwHelpID = 0)
			{
				AddControl(CtrlType, wId, nX, nY, nWidth, nHeight, dwStyle, dwExStyle, Text, pCreationData, nCreationData, dwHelpID);
			}

			void Translate(LPCWSTR a_psz, CAutoVectorPtr<WCHAR>& a_cDst)
			{
				LPCOLESTR a_p = NULL;
				ULONG a_n = 0;
				bool bTranslate = false;

				OLECHAR szExact[7];
				swprintf(szExact, L"[%04x]", *m_pLocaleID);
				static OLECHAR szEng[] = L"[0409]";
				LPCOLESTR pExact = NULL;
				LPCOLESTR pEng = NULL;
				LPCOLESTR pEngEnd = NULL;
				LPCOLESTR pFirst = NULL;
				LPCOLESTR pFirstEnd = NULL;
				LPCOLESTR pStart = a_psz;
				while (*a_psz)
				{
					if (*a_psz == L'[' &&
						((a_psz[1] >= L'0' && a_psz[1] <= L'9') || (a_psz[1] >= L'A' && a_psz[1] <= L'F') || (a_psz[1] >= L'a' && a_psz[1] <= L'f')) &&
						((a_psz[2] >= L'0' && a_psz[2] <= L'9') || (a_psz[2] >= L'A' && a_psz[2] <= L'F') || (a_psz[2] >= L'a' && a_psz[2] <= L'f')) &&
						((a_psz[3] >= L'0' && a_psz[3] <= L'9') || (a_psz[3] >= L'A' && a_psz[3] <= L'F') || (a_psz[3] >= L'a' && a_psz[3] <= L'f')) &&
						((a_psz[4] >= L'0' && a_psz[4] <= L'9') || (a_psz[4] >= L'A' && a_psz[4] <= L'F') || (a_psz[4] >= L'a' && a_psz[4] <= L'f')) &&
						a_psz[5] == L']')
					{
						if (0 == _wcsnicmp(szExact, a_psz, 6) || 0 == _wcsnicmp(L"[0000]", a_psz, 6))
						{
							a_psz += 6;
							pExact = a_psz;
						}
						else if (pExact)
						{
							a_p = pExact;
							a_n = a_psz-pExact;
							a_cDst.Allocate(a_n+1);
							CopyMemory(a_cDst.m_p, a_p, a_n*sizeof*a_cDst.m_p);
							a_cDst[a_n] = L'\0';
							return;
						}
						else if (0 == _wcsnicmp(szEng, a_psz, 6))
						{
							a_psz += 6;
							pEng = a_psz;
						}
						else if (pEng)
						{
							pEngEnd = a_psz;
							a_psz += 6;
						}
						else if (pFirst == NULL)
						{
							a_psz += 6;
							pFirst = a_psz;
						}
						else if (pFirstEnd == NULL)
						{
							pFirstEnd = a_psz;
							a_psz += 6;
						}
						else
						{
							++a_psz;
						}
					}
					else
					{
						++a_psz;
					}
				}
				if (pExact)
				{
					a_p = pExact;
					a_n = a_psz-pExact;
				}
				else if (pEng)
				{
					a_p = pEng;
					a_n = pEngEnd ? pEngEnd-pEng : a_psz-pEng;
					bTranslate = true;
				}
				else if (pFirst)
				{
					a_p = pFirst;
					a_n = pFirstEnd ? pFirstEnd-pFirst : a_psz-pFirst;
				}
				else
				{
					a_p = pStart;
					a_n = a_psz-pStart;
					bTranslate = true;
				}
				a_cDst.Allocate(a_n+1);
				CopyMemory(a_cDst.m_p, a_p, a_n*sizeof*a_cDst.m_p);
				a_cDst[a_n] = L'\0';
				if (bTranslate)
				{
					if (m_pTr == NULL)
					{
						RWCoCreateInstance(m_pTr, __uuidof(Translator));
					}
					if (m_pTr)
					{
						CComBSTR bstrTranslated;
						m_pTr->Translate(CComBSTR(a_cDst.m_p), *m_pLocaleID, &bstrTranslated);
						if (bstrTranslated.m_str)
						{
							a_cDst.Free();
							a_cDst.Allocate(bstrTranslated.Length()+1);
							wcscpy(a_cDst.m_p, bstrTranslated.m_str);
						}
					}
				}
			}

		private:
			LCID* m_pLocaleID;
			TDlgTemplate* m_pTemplate;
			CComPtr<ITranslator> m_pTr;
		};

		CTemplTranslator m_Template;
		TDlgTemplate m_OrigTemplate;

		void CreateTemplate()
		{
			T* pT = static_cast<T*>(this);
			pT->DoInitTemplate();
			pT->DoInitControls();
		}

		INT_PTR DoModal(HWND a_hWndParent = ::GetActiveWindow(), LPARAM a_dwInitParam = NULL)
		{
			T* pT = static_cast<T*>(this);
			ATLASSERT(pT->m_hWnd == NULL);

			if (!m_OrigTemplate.IsValid())
				CreateTemplate();

	#if (_ATL_VER >= 0x0800)
			// Allocate the thunk structure here, where we can fail gracefully.
			BOOL result = m_thunk.Init(NULL, NULL);
			if (result == FALSE)
			{
				SetLastError(ERROR_OUTOFMEMORY);
				return -1;
			}
	#endif // (_ATL_VER >= 0x0800)

	#ifdef _DEBUG
			m_bModal = true;
	#endif // _DEBUG

			ATLASSERT(a_hWndParent != GetDesktopWindow());
			if (a_hWndParent) a_hWndParent = GetAncestor(a_hWndParent, GA_ROOT);

			m_bEndModalWithPreTranslate = false;
			Create(a_hWndParent, a_dwInitParam);
			ShowWindow(SW_SHOW);
			if (m_hWnd == NULL)
				return -1;

			if (a_hWndParent)
				::EnableWindow(a_hWndParent, FALSE);
			MSG msg;
			msg.message = WM_NULL;
			while (!m_bEndModalWithPreTranslate)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						break;
					}
					if (msg.message == WM_MOUSEWHEEL)
					{
						// forward mouse wheel message to the window under the mouse pointer
						POINT tPt = {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)};
						HWND hWnd = WindowFromPoint(tPt);
						if (hWnd)
						{
							DWORD dwPID = 0;
							GetWindowThreadProcessId(hWnd, &dwPID);
							if (dwPID && GetCurrentProcessId() == dwPID)
							{
								msg.hwnd = hWnd;
								DispatchMessage(&msg);
								continue;
							}
						}
					}
					if (!static_cast<T*>(this)->DoModalPreTranslate(&msg) && !IsDialogMessage(&msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				else if (!m_bEndModalWithPreTranslate)
				{
					WaitMessage();
				}
			}
			if (msg.message == WM_QUIT)
			{
				PostQuitMessage((int)msg.wParam);
			}
			::EnableWindow(a_hWndParent, TRUE);
			DestroyWindow();
			return m_nResultModalWithPreTranslate;
		}
		void EndDialog(int iResult)
		{
			m_nResultModalWithPreTranslate = iResult;
			m_bEndModalWithPreTranslate = true;
		}

		HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
		{
			T* pT = static_cast<T*>(this);
			ATLASSERT(pT->m_hWnd == NULL);

			if (!m_OrigTemplate.IsValid())
				CreateTemplate();

	#if (_ATL_VER >= 0x0800)
			// Allocate the thunk structure here, where we can fail gracefully.
			BOOL result = m_thunk.Init(NULL, NULL);
			if (result == FALSE) 
			{
				SetLastError(ERROR_OUTOFMEMORY);
				return NULL;
			}
	#endif // (_ATL_VER >= 0x0800)

			ModuleHelper::AddCreateWndData(&m_thunk.cd, this);

	#ifdef _DEBUG
			m_bModal = false;
	#endif // _DEBUG

			HWND hWnd = ::CreateDialogIndirectParam(ModuleHelper::GetResourceInstance(), (LPCDLGTEMPLATE)m_OrigTemplate.GetTemplatePtr(), hWndParent, (DLGPROC)T::StartDialogProc, dwInitParam);
			ATLASSERT(m_hWnd == hWnd);

			return hWnd;
		}

		// for CComControl
		HWND Create(HWND hWndParent, RECT&, LPARAM dwInitParam = NULL)
		{
			return Create(hWndParent, dwInitParam);
		}

		void DoInitTemplate() 
		{
			ATLASSERT(FALSE);   // MUST be defined in derived class
		}

		void DoInitControls() 
		{
			ATLASSERT(FALSE);   // MUST be defined in derived class
		}

		bool GetDialogSize(SIZE* a_pSize, WORD /*a_wLanguage*/, bool a_bLoadAnyLanguage = true)
		{
			if (!m_OrigTemplate.IsValid())
				CreateTemplate();
			AtlGetDialogSize((LPCDLGTEMPLATE)m_OrigTemplate.GetTemplatePtr(), a_pSize);
			return true;
		}

		// to be overriden
		bool DoModalPreTranslate(MSG const*) { return false; }

		LCID m_tLocaleID;

	private:
		bool m_bEndModalWithPreTranslate;
		int m_nResultModalWithPreTranslate;
	};

#endif//__ATLDLGS_H__

};
