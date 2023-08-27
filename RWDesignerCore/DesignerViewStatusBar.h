
#pragma once


struct SStatusBarItem
{
	std::tstring strText;
	std::tstring strTooltip;
	HICON hIcon;
	int nWidth;
	int nPane;
};
typedef std::vector<SStatusBarItem> CStatusBarItems;


class ATL_NO_VTABLE CDesignerViewStatusBar :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDesignerStatusBar
{
public:
	CDesignerViewStatusBar() : m_fDPIScale(1.0f)
	{
	}
	~CDesignerViewStatusBar()
	{
		for (CItems::iterator i = m_cItems.begin(); i != m_cItems.end(); ++i)
		{
			if (i->second.a_hIcon)
				ATLVERIFY(DestroyIcon(i->second.a_hIcon));
		}
	}
	void SetScale(float a_fDPIScale)
	{
		m_fDPIScale = a_fDPIScale;
	}

	BEGIN_COM_MAP(CDesignerViewStatusBar)
		COM_INTERFACE_ENTRY(IDesignerStatusBar)
	END_COM_MAP()

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(PaneKeep)(BSTR a_bstrPaneID)
	{
		try
		{
			if (m_cItems.find(a_bstrPaneID) != m_cItems.end())
				m_cItems[a_bstrPaneID].bValid = true;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(PaneSet)(BSTR a_bstrPaneID, HICON a_hIcon, BSTR a_bstrText, ULONG a_nWidth, LONG a_nPosition)
	{
		try
		{
			SItem sItem = {a_hIcon, a_bstrText, a_nWidth*m_fDPIScale+0.5f, a_nPosition, true};
			m_cItems[a_bstrPaneID] = sItem;
			m_bChange = true;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SimpleModeKeep)()
	{
		m_bSimple = true;
		return S_OK;
	}
	STDMETHOD(SimpleModeSet)(BSTR a_bstrText)
	{
		if (m_bstrSimpleMessage != a_bstrText)
		{
			m_bstrSimpleMessage = a_bstrText;
			m_bTextChange = true;
		}
		m_bSimple = true;
		return S_OK;
	}

	// internal methods
	bool UpdateStatusBar(IDesignerView* a_pView, IDesignerViewStatusBar* a_pExtra = NULL)
	{
		bool bSimple = m_bSimple;
		m_bSimple = false;
		CComPtr<IEnumUnknownsInit> pIfaces;
		RWCoCreateInstance(pIfaces, __uuidof(EnumUnknowns));
		if (a_pView)
			a_pView->QueryInterfaces(__uuidof(IDesignerViewStatusBar), EQIFVisible, pIfaces);
		for (CItems::iterator i = m_cItems.begin(); i != m_cItems.end(); ++i)
			i->second.bValid = false;
		m_bChange = false;
		m_bModeChange = false;
		m_bTextChange = false;
		CComPtr<IDesignerViewStatusBar> pTmp;
		for (ULONG i = 0; SUCCEEDED(pIfaces->Get(i, __uuidof(IDesignerViewStatusBar), reinterpret_cast<void**>(&pTmp))); ++i, pTmp = NULL)
		{
			pTmp->Update(this);
		}
		if (a_pExtra)
			a_pExtra->Update(this);
		m_cSortedItems.clear();
		for (CItems::iterator i = m_cItems.begin(); i != m_cItems.end(); )
		{
			if (i->second.bValid)
			{
				m_cSortedItems.push_back(i);
				++i;
			}
			else
			{
				CItems::iterator j = i;
				++i;
				if (j->second.a_hIcon)
					ATLVERIFY(DestroyIcon(j->second.a_hIcon));
				m_cItems.erase(j);
				m_bChange = true;
			}
		}
		std::stable_sort(m_cSortedItems.begin(), m_cSortedItems.end(), position_compare());
		if (bSimple != m_bSimple)
			m_bModeChange = true;
		return m_bChange;
	}
	template<class TOutputIterator>
	void GetPanes(TOutputIterator a_tOutput) const
	{
		for (CSortedItems::const_iterator i = m_cSortedItems.begin(); i != m_cSortedItems.end(); ++i)
		{
			SStatusBarItem tItem;
			tItem.strText = COLE2T((*i)->second.a_bstrText);
			//tItem.strTooltip;
			tItem.hIcon = (*i)->second.a_hIcon;
			tItem.nWidth = (*i)->second.a_nWidth;
			tItem.nPane = -1;
			*a_tOutput = tItem;
			++a_tOutput;
		}
	}
	BSTR GetSimpleMessage() const
	{
		return m_bstrSimpleMessage;
	}
	bool IsSimpleMode() const
	{
		return m_bSimple;
	}
	bool ModeChanged() const
	{
		return m_bModeChange;
	}
	bool TextChanged() const
	{
		return m_bTextChange;
	}

private:
	struct SItem
	{
		HICON a_hIcon;
		CComBSTR a_bstrText;
		ULONG a_nWidth;
		LONG a_nPosition;
		bool bValid;
	};
	typedef std::map<CComBSTR, SItem> CItems;
	typedef std::vector<CItems::const_iterator> CSortedItems;
	struct position_compare
	{
		bool operator()(CItems::const_iterator a_1, CItems::const_iterator a_2) const
		{
			return a_1->second.a_nPosition < a_2->second.a_nPosition;
		}
	};

private:
	CItems m_cItems;
	CSortedItems m_cSortedItems;
	CComBSTR m_bstrSimpleMessage;
	bool m_bSimple;
	bool m_bChange;
	bool m_bTextChange;
	bool m_bModeChange;
	float m_fDPIScale;
};

