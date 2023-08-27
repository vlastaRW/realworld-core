
#pragma once

typedef HICON (*pfnGetDFIcon)(ULONG size);

template<size_t t_nCount, GUID const* t_aUIDs, UINT const* t_aResIDs, pfnGetDFIcon const* t_aGetIcons = NULL>
class CDesignerFrameIconsImpl :
	public IDesignerFrameIcons
{
public:
	STDMETHOD(TimeStamp)(ULONG* a_pTimeStamp)
	{
		try
		{
			*a_pTimeStamp = 0;
			return S_OK;
		}
		catch (...)
		{
			return a_pTimeStamp == NULL ? E_POINTER: E_UNEXPECTED;
		}
	}
	STDMETHOD(EnumIconIDs)(IEnumGUIDs** a_ppIDs)
	{
		try
		{
			*a_ppIDs = NULL;
			CComPtr<IEnumGUIDsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumGUIDs));
			pTmp->InsertMultiple(t_nCount, t_aUIDs);
			*a_ppIDs = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppIDs == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(GetIcon)(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;

			for (size_t i = 0; i < t_nCount; ++i)
			{
				if (IsEqualGUID(a_tIconID, t_aUIDs[i]))
				{
					if (t_aGetIcons != NULL && t_aGetIcons[i] != NULL)
						*a_phIcon = t_aGetIcons[i](a_nSize);
					else
						*a_phIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_aResIDs[i]), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
					break;
				}
			}
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
};
