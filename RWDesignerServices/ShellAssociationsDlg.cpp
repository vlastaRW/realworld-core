#include "stdafx.h"
#include "ShellAssociationsDlg.h"

#include <XPGUI.h>
#include <ApplicationFileName.h>
#include <DPIUtils.h>

CShellAssociationsDlg::CShellAssociationsDlg(LCID a_tLocaleID, HICON a_hIcon):
	Win32LangEx::CLangIndirectDialogImpl<CShellAssociationsDlg>(a_tLocaleID),
	CContextHelpDlg<CShellAssociationsDlg>(_T("http://www.rw-designer.com/file-associations")),
	m_hIcon(a_hIcon)
{
}

CShellAssociationsDlg::~CShellAssociationsDlg()
{
}

LRESULT CShellAssociationsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	SetIcon(m_hIcon, FALSE);

	m_wndList = GetDlgItem(IDC_SHLASSOC_LIST);
	m_wndList.SetExtendedListViewStyle(LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT, LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT);
	RECT rcClient;
	m_wndList.GetClientRect(&rcClient);
	rcClient.right -= GetSystemMetrics(SM_CXVSCROLL);
	CComBSTR bstrTmp;
	CMultiLanguageString::GetLocalized(L"[0409]File type name[0405]Jméno typu souboru", m_tLocaleID, &bstrTmp);
	m_wndList.InsertColumn(0, bstrTmp, LVCFMT_LEFT, rcClient.right*2/3, -1);
	bstrTmp.Empty();
	CMultiLanguageString::GetLocalized(L"[0409]Extension[0405]Přípona typu souboru", m_tLocaleID, &bstrTmp);
	m_wndList.InsertColumn(1, bstrTmp, LVCFMT_LEFT, rcClient.right-(rcClient.right*2/3), -1);

	HIMAGELIST hIL = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), XPGUI::GetImageListColorFlags(), 8, 8);
	ImageList_AddIcon(hIL, ::LoadIcon(NULL, IDI_QUESTION));
	int nIcons = 1;
	int nItems = 0;

	std::vector<CComPtr<IDocumentBuilder> > cBuilders;

	RWCoCreateInstance(m_pInMgr, __uuidof(InputManager));

	CComPtr<IPlugInCache> pPIC;
	RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
	CComPtr<IEnumUnknowns> pDecoders;
	if (pPIC) pPIC->InterfacesEnum(CATID_DocumentDecoder, __uuidof(IDocumentDecoder), 0, &pDecoders, NULL);
	ULONG nDecoders = 0;
	if (pDecoders) pDecoders->Size(&nDecoders);
	CComBSTR bstrAll;
	std::set<CComBSTR> cUsedExts;
	for (ULONG i = 0; i < nDecoders; ++i)
	{
		CComPtr<IDocumentDecoder> pDD;
		pDecoders->Get(i, __uuidof(IDocumentDecoder), reinterpret_cast<void**>(&pDD));
		CComPtr<IDocumentType> pFilter;
		pDD->DocumentType(&pFilter);
		CComPtr<IEnumStrings> pSuppExts;
		pFilter->SupportedExtensionsGet(&pSuppExts);
		if (pSuppExts != NULL)
		{
			CComBSTR bstrExt;
			for (ULONG iExt = 0; SUCCEEDED(pSuppExts->Get(iExt, &bstrExt)); ++iExt, bstrExt.Empty())
			{
				if (cUsedExts.find(bstrExt) != cUsedExts.end())
					continue;
				cUsedExts.insert(bstrExt);
				CComBSTR bstrIconPath;
				pFilter->IconPathGet(bstrExt, &bstrIconPath);
				CComPtr<ILocalizedString> pTypeName;
				pFilter->TypeNameGet(bstrExt, &pTypeName);
				if (pTypeName != NULL)
				{
					if (bstrIconPath == NULL || bstrIconPath[0] == L'\0')
					{
						// try to get icon from a compatible builder instead
						if (cBuilders.empty())
						{
							CComPtr<IEnumUnknowns> pBuilders;
							if (pPIC) pPIC->InterfacesEnum(CATID_DocumentBuilder, __uuidof(IDocumentBuilder), 0, &pBuilders, NULL);
							ULONG nBuilders = 0;
							pBuilders->Size(&nBuilders);
							for (ULONG i = 0; i < nBuilders; ++i)
							{
								CComPtr<IDocumentBuilder> pDB;
								pBuilders->Get(i, __uuidof(IDocumentBuilder), reinterpret_cast<void**>(&pDB));
								ULONG nPriority = EDPAverage;
								pDB->Priority(&nPriority);
								size_t j = 0;
								for (; j < cBuilders.size(); ++j)
								{
									ULONG nPriority2 = EDPAverage;
									cBuilders[j]->Priority(&nPriority2);
									if (nPriority2 < nPriority)
									{
										cBuilders.insert(cBuilders.begin()+j, pDB);
										break;
									}
								}
								if (j == cBuilders.size())
									cBuilders.push_back(pDB);
							}
						}
						for (std::vector<CComPtr<IDocumentBuilder> >::const_iterator j = cBuilders.begin(); j != cBuilders.end(); ++j)
						{
							if (pDD->IsCompatible(1, &(j->p)) == S_OK)
							{
								(*j)->FormatInfo(NULL, &bstrIconPath);
								if (bstrIconPath.m_str && bstrIconPath.m_str[0])
									break;
							}
						}
					}
					int nIconIndex = 0;
					LPTSTR pszIconPath = NULL;
					if (bstrIconPath)
					{
						TCHAR szIconFilePath[MAX_PATH] = _T("");
						int nIconFileIndex = 0;
						pszIconPath = _tcsdup(COLE2CT(bstrIconPath));
						_tcscpy(szIconFilePath, pszIconPath);
						int iLastComma = -1;
						for (int ii = 0; szIconFilePath[ii]; ii++)
						{
							switch (szIconFilePath[ii])
							{
							case _T(','):
								iLastComma = ii;
								break;
							case _T('.'):
							case _T('\\'):
								iLastComma = -1;
								break;
							}
						}
						if (iLastComma > 0)
						{
							szIconFilePath[iLastComma] = _T('\0');
							nIconFileIndex = _ttoi(szIconFilePath+iLastComma+1);
						}

						HICON hIc = NULL;
						ExtractIconEx(szIconFilePath, nIconFileIndex, NULL, &hIc, 1);
						if (hIc)
						{
							ImageList_AddIcon(hIL, hIc);
							nIconIndex = nIcons++;
							DestroyIcon(hIc);
						}
					}
					CComBSTR bstrTypeName;
					pTypeName->GetLocalized(m_tLocaleID, &bstrTypeName);
					m_wndList.InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE, nItems, COLE2CT(bstrTypeName), 0, 0, nIconIndex, reinterpret_cast<LPARAM>(pszIconPath));
					TCHAR szKeyName[MAX_PATH] = _T(".");
					_tcscpy(szKeyName+1, COLE2CT(bstrExt));
					m_wndList.SetItem(nItems, 1, LVIF_TEXT, szKeyName+1, -1, 0, 0, 0);
					BOOL bAssociated = FALSE;
					HKEY hKey = NULL;
					TCHAR szProgIDName[MAX_PATH] = _T("");
					_stprintf(szProgIDName, _T("%s.%s.1"), GetAppFileName(), szKeyName+1);
					if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyName, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
					{
						TCHAR szValue[MAX_PATH] = _T("");
						DWORD dwValue = sizeof(szValue);
						RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)szValue, &dwValue);
						if (_tcsicmp(szProgIDName, szValue) == 0)
						{
							bAssociated = TRUE;
						}
						RegCloseKey(hKey);
					}
					m_wndList.SetCheckState(nItems, bAssociated);
					nItems++;
				}
			}
		}
	}

	m_wndList.SetImageList(hIL, LVSIL_SMALL);

	//Button_SetElevationRequiredState();

	DlgResize_Init();

	return TRUE;
}

LRESULT CShellAssociationsDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_wndList.IsWindow())
	{
		int nItems = m_wndList.GetItemCount();
		int i;
		for (i = 0; i < nItems; i++)
		{
			free(reinterpret_cast<void*>(m_wndList.GetItemData(i)));
		}
	}
	return 0;
}

LRESULT CShellAssociationsDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

LRESULT CShellAssociationsDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CWaitCursor cCursor;

	HKEY hClassesRoot = HKEY_CLASSES_ROOT;
	if (XPGUI::IsVista())
	{
		DWORD dummy;
		RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Classes"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hClassesRoot, &dummy);
	}
	if (m_wndList.IsWindow())
	{
		int nItems = m_wndList.GetItemCount();
		int i;
		for (i = 0; i < nItems; i++)
		{
			LPCTSTR pszIconPath = reinterpret_cast<LPCTSTR>(m_wndList.GetItemData(i));
			TCHAR szExtension[MAX_PATH] = _T("");
			m_wndList.GetItemText(i, 1, szExtension, itemsof(szExtension));
			TCHAR szTypeName[MAX_PATH] = _T("");
			m_wndList.GetItemText(i, 0, szTypeName, itemsof(szTypeName));
			TCHAR szExeName[MAX_PATH+10] = _T("\"");
			GetModuleFileName(NULL, szExeName+1, MAX_PATH);
			_tcscat(szExeName, _T("\" \"%1\""));
			m_wndList.GetItemText(i, 0, szTypeName, itemsof(szTypeName));
			CComBSTR bstrOpenCommand;
			CMultiLanguageString::GetLocalized(L"[0409]Open[0405]Otevřít", m_tLocaleID, &bstrOpenCommand);
			if (m_wndList.GetCheckState(i))
			{
				// associate
				TCHAR szKeyName[MAX_PATH] = _T(".");
				_tcscpy(szKeyName+1, szExtension);
				HKEY hKey = NULL;
				TCHAR szProgIDName[MAX_PATH] = _T("");
				_stprintf(szProgIDName, _T("%s.%s.1"), GetAppFileName(), szExtension);
				DWORD dummy;
				if (RegCreateKeyEx(hClassesRoot, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dummy) == ERROR_SUCCESS)
				{
					TCHAR szValue[MAX_PATH] = _T("");
					DWORD dwValue = sizeof(szValue);
					RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)szValue, &dwValue);
					if (_tcsicmp(szProgIDName, szValue) != 0)
					{
						if (_tcslen(szValue) > 0)
						{
							HKEY hKeyProgids = NULL;
							if (RegCreateKeyEx(hKey, _T("OpenWithProgids"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyProgids, &dummy) == ERROR_SUCCESS)
							{
								HKEY hKeyOldProgID = NULL;
								if (RegCreateKeyEx(hKeyProgids, szValue, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyOldProgID, &dummy) == ERROR_SUCCESS)
								{
									RegCloseKey(hKeyOldProgID);
								}
								RegCloseKey(hKeyProgids);
							}
						}
						RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE const*)szProgIDName, sizeof(szProgIDName[0])*(_tcslen(szProgIDName)+1));
					}
					HKEY hKey2 = NULL;
					if (RegOpenKeyEx(hKey, _T("shell"), 0, KEY_ALL_ACCESS, &hKey2) == ERROR_SUCCESS)
					{
						DeleteSubTree(hKey2);
						RegCloseKey(hKey2);
						RegDeleteKey(hKey, _T("shell"));
					}
					RegCloseKey(hKey);
				}
				hKey = NULL;
				if (RegCreateKeyEx(hClassesRoot, szProgIDName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dummy) == ERROR_SUCCESS)
				{
					RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE const*)szTypeName, sizeof(szTypeName[0])*(_tcslen(szTypeName)+1));
					HKEY hKeyIcon = NULL;
					if (RegCreateKeyEx(hKey, _T("DefaultIcon"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyIcon, &dummy) == ERROR_SUCCESS)
					{
						RegSetValueEx(hKeyIcon, NULL, 0, REG_SZ, (BYTE const*)pszIconPath, sizeof(pszIconPath[0])*(_tcslen(pszIconPath)+1));
						RegCloseKey(hKeyIcon);
					}
					HKEY hKeyShell = NULL;
					if (RegCreateKeyEx(hKey, _T("shell"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyShell, &dummy) == ERROR_SUCCESS)
					{
						HKEY hKeyOpen = NULL;
						if (RegCreateKeyEx(hKeyShell, _T("open"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyOpen, &dummy) == ERROR_SUCCESS)
						{
							RegSetValueExW(hKeyOpen, NULL, 0, REG_SZ, (BYTE const*)bstrOpenCommand.m_str, sizeof(wchar_t)*(bstrOpenCommand.Length()+1));
							HKEY hKeyCommand = NULL;
							if (RegCreateKeyEx(hKeyOpen, _T("command"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyCommand, &dummy) == ERROR_SUCCESS)
							{
								RegSetValueEx(hKeyCommand, NULL, 0, REG_SZ, (BYTE const*)szExeName, sizeof(szExeName[0])*(_tcslen(szExeName)+1));
								RegCloseKey(hKeyCommand);
							}
							RegCloseKey(hKeyOpen);
						}
						RegCloseKey(hKeyShell);
					}
					RegCloseKey(hKey);
				}
			}
			else
			{
				// remove association if it exist
				TCHAR szKeyName[MAX_PATH] = _T(".");
				_tcscpy(szKeyName+1, szExtension);
				HKEY hKey = NULL;
				TCHAR szProgIDName[MAX_PATH] = _T("");
				_stprintf(szProgIDName, _T("%s.%s.1"), GetAppFileName(), szExtension);
				if (RegOpenKeyEx(hClassesRoot, szKeyName, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
				{
					TCHAR szValue[MAX_PATH] = _T("");
					DWORD dwValue = sizeof(szValue);
					RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)szValue, &dwValue);
					if (_tcsicmp(szProgIDName, szValue) == 0)
					{
						szValue[0] = _T('\0');
						HKEY hKeyProgids = NULL;
						if (RegOpenKeyEx(hKey, _T("OpenWithProgids"), 0, KEY_ALL_ACCESS, &hKeyProgids) == ERROR_SUCCESS)
						{
							DWORD nSubKeys = 0;
							if (RegQueryInfoKey(hKeyProgids, NULL, NULL, 0, &nSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS && nSubKeys > 0)
							{
								DWORD nValLen = itemsof(szValue);
								if (RegEnumKeyEx(hKeyProgids, nSubKeys-1, szValue, &nValLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
								{
									RegDeleteKey(hKeyProgids, szValue);
								}
							}
							RegCloseKey(hKeyProgids);
						}
						RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE const*)szValue, sizeof(szValue[0])*_tcslen(szValue));
					}
					RegCloseKey(hKey);
				}
				hKey = NULL;
				if (RegOpenKeyEx(hClassesRoot, szProgIDName, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
				{
					DeleteSubTree(hKey);
					RegCloseKey(hKey);
					RegDeleteKey(hClassesRoot, szProgIDName);
				}
			}
		}
	}
	if (hClassesRoot != HKEY_CLASSES_ROOT)
	{
		RegCloseKey(hClassesRoot);
	}

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
	EndDialog(wID);
	return 0;
}

HRESULT CShellAssociationsDlg::DeleteSubTree(HKEY a_hRootKey)
{
	HRESULT hRes = S_OK;

	vector<tstring> aValNames;
	DWORD i;
	for (i = 0; true; i++)
	{
		TCHAR szValName[MAX_PATH];
		DWORD nValName = MAX_PATH;
		szValName[0] = _T('\0');
		if (RegEnumValue(a_hRootKey, i, szValName, &nValName, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
		{
			break;
		}
		aValNames.push_back(szValName);
	}
	vector<tstring> aKeyNames;
	for (i = 0; true; i++)
	{
		TCHAR szKeyName[MAX_PATH];
		DWORD nKeyName = MAX_PATH;
		szKeyName[0] = _T('\0');
		if (RegEnumKey(a_hRootKey, i, szKeyName, nKeyName) != ERROR_SUCCESS)
		{
			break;
		}
		aKeyNames.push_back(szKeyName);
	}

	vector<tstring>::const_iterator j;
	for (j = aValNames.begin(); j != aValNames.end(); j++)
	{
		if (RegDeleteValue(a_hRootKey, j->c_str()) != ERROR_SUCCESS)
		{
			hRes = E_FAIL;
		}
	}

	for (j = aKeyNames.begin(); j != aKeyNames.end(); j++)
	{
		HKEY hKey = NULL;
		if (ERROR_SUCCESS == RegOpenKeyEx(a_hRootKey, j->c_str(), 0, KEY_ALL_ACCESS, &hKey))
		{
			if (FAILED(DeleteSubTree(hKey)))
				hRes = E_FAIL;
			RegCloseKey(hKey);
		}
		if (RegDeleteKey(a_hRootKey, j->c_str()) != ERROR_SUCCESS)
		{
			hRes = E_FAIL;
		}
	}

	return hRes;
}
