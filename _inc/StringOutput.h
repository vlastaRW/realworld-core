
#pragma once

class CStringOutput
{
public:
	virtual ~CStringOutput() {}

	virtual void Write(wchar_t const* a_pData, size_t a_nSize) = 0;
};


class CStdTypesToStringOutput
{
public:
	class CXMLElement
	{
	public:
		CXMLElement(CStdTypesToStringOutput& a_cOutput, wchar_t const* a_psz) :
			m_cOutput(a_cOutput), m_psz(a_psz) // objects must remain valid
		{
			m_cOutput << L'<' << m_psz << L'>';
		}
		~CXMLElement()
		{
			m_cOutput << L"</" << m_psz << L'>';
		}
	private:
		wchar_t const* m_psz;
		CStdTypesToStringOutput& m_cOutput;
	};

	CStdTypesToStringOutput(CStringOutput& a_cOutput) :
		m_cOutput(a_cOutput)
	{
	}

	CStdTypesToStringOutput& operator<<(wchar_t const* a_pszText)
	{
		m_cOutput.Write(a_pszText, wcslen(a_pszText));
		return *this;
	}
	CStdTypesToStringOutput& operator<<(wchar_t const a_cChar)
	{
		m_cOutput.Write(&a_cChar, 1);
		return *this;
	}
	CStdTypesToStringOutput& operator<<(bool a_bVal)
	{
		m_cOutput.Write(a_bVal ? L"true" : L"false", a_bVal ? 4 : 5);
		return *this;
	}
	CStdTypesToStringOutput& operator<<(float a_fNum)
	{
		wchar_t sz[32];
		m_cOutput.Write(sz, swprintf(sz, L"%g", a_fNum)); // TODO: ensure that it is locale independent
		return *this;
	}
	CStdTypesToStringOutput& operator<<(int a_iNum)
	{
		wchar_t sz[32];
		m_cOutput.Write(sz, swprintf(sz, L"%i", a_iNum));
		return *this;
	}
	CStdTypesToStringOutput& operator<<(unsigned int a_iNum)
	{
		wchar_t sz[32];
		m_cOutput.Write(sz, swprintf(sz, L"%u", a_iNum));
		return *this;
	}

private:
	CStringOutput& m_cOutput;
};

