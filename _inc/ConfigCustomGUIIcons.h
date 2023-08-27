
#pragma once

#include <RWConceptDesignerExtension.h>
#include <ConfigCustomGUIImpl.h>
#include <XPGUI.h>


template <class T, class TBase = CCustomConfigWndImpl<T> >
class ATL_NO_VTABLE CCustomConfigWndWithIcons :
	public TBase
{
public:
	CCustomConfigWndWithIcons()
	{
		RWCoCreateInstance(m_pIconsManager, __uuidof(DesignerFrameIconsManager));
		m_cIcons.Create(16, 16, XPGUI::GetImageListColorFlags(), 4, 4);
	}
	~CCustomConfigWndWithIcons()
	{
		m_cIcons.Destroy();
	}

	int GetIconIndex(GUID const& a_tID)
	{
		CIconMap::const_iterator i = m_cIconMap.find(a_tID);
		if (i != m_cIconMap.end())
			return i->second;
		HICON h = NULL;
		m_pIconsManager->GetIcon(a_tID, 16, &h);
		if (h == NULL)
			return -1;
		m_cIcons.AddIcon(h);
		DestroyIcon(h);
		return m_cIcons.GetImageCount()-1;
	}
	HIMAGELIST M_ImageList()
	{
		return m_cIcons;
	}

private:
	struct lessGUID { bool operator()(GUID const& a_1, GUID const& a_2) const { return memcmp(&a_1, &a_2, sizeof GUID)<0; }};
	typedef std::map<GUID, int, lessGUID> CIconMap;

private:
	CImageList m_cIcons;
	CIconMap m_cIconMap;
	CComPtr<IDesignerFrameIcons> m_pIconsManager;
};
