
#pragma once

class CThreadCommand
{
public:
	CThreadCommand(HANDLE a_hWndCreated, RWHWND* a_pCreatedWindow, CLSID const& a_tStartPage) :
		m_hWndCreated(a_hWndCreated), m_pCreatedWindow(a_pCreatedWindow),
		m_eType(ETStartPage), m_tStartPage(a_tStartPage)
	{
	}
	CThreadCommand() : m_hWndCreated(NULL), m_eType(ETQuitRequest), m_pCreatedWindow(NULL)
	{
	}
	CThreadCommand(HANDLE a_hWndCreated, RWHWND* a_pCreatedWindow, LPCWSTR a_pszSourceName) :
		m_hWndCreated(a_hWndCreated), m_pCreatedWindow(a_pCreatedWindow),
		m_eType(ETSourceName), m_bstrSourceName(SysAllocString(a_pszSourceName))
	{
	}
	CThreadCommand(HANDLE a_hWndCreated, RWHWND* a_pCreatedWindow, IDocument* a_pDocument) :
		m_hWndCreated(a_hWndCreated), m_pCreatedWindow(a_pCreatedWindow),
		m_eType(ETIDocument), m_pDocument(NULL)
	{
		if (a_pDocument)
		{
			a_pDocument->AddRef();
			m_pDocument = a_pDocument;
		}
	}
	~CThreadCommand()
	{
		if (m_eType == ETIDocument)
		{
			if (m_pDocument)
				m_pDocument->Release();
		}
		else if (m_eType == ETSourceName)
		{
			SysFreeString(m_bstrSourceName);
		}
		if (m_hWndCreated)
		{
			SetEvent(m_hWndCreated);
		}
	}

	bool IsIDocument() const {return m_eType == ETIDocument;}
	bool IsSourceName() const {return m_eType == ETSourceName;}
	bool IsStartPage() const {return m_eType == ETStartPage;}
	bool IsQuitRequest() const {return m_eType == ETQuitRequest;}

	IDocument* M_IDocument() const {return m_pDocument;}
	BSTR M_SourceName() const {return m_bstrSourceName;}
	CLSID M_StartPage() const {return m_tStartPage;}
	HANDLE M_WndCreated() const {return m_hWndCreated;}
	RWHWND* M_CreatedWindow() const {return m_pCreatedWindow;}

private:
	enum EType
	{
		ETIDocument = 0,
		ETSourceName,
		ETStartPage,
		ETQuitRequest
	};

private:
	EType m_eType;
	union
	{
		CLSID m_tStartPage;
		BSTR m_bstrSourceName;
		IDocument* m_pDocument;
	};
	HANDLE m_hWndCreated;
	RWHWND* m_pCreatedWindow;
};

