// Copyright (c) 2006
// Sergey Klimov (kidd@ukr.net)

#ifndef WTL_DW_STGXML_H_INCLUDED_
#define WTL_DW_STGXML_H_INCLUDED_

#include <stg.h>
#include <cassert>

namespace sstate{


class CStgXML
	: public IStorge
{
public:
	CStgXML(void)
	{
	}

	CStgXML(IXMLDOMNode* node)
	{
		node->QueryInterface(IID_IXMLDOMElement,reinterpret_cast<void**>(&m_key));
//		node->QueryInterface(__uuidof(IID_IXMLDOMElement),reinterpret_cast<void**>(&m_key));
	}

	virtual ~CStgXML(void)
	{
	}

	IXMLDOMElement* Root(void)
	{
		return m_key;
	}

	bool operator !(void) const
	{
		return !m_key;
	}

	long Create(LPCTSTR name=_T("config"),LPCOLESTR msxmlDocClassName=OLESTR("Msxml2.FreeThreadedDOMDocument"))
	{
		assert(!m_key);
		CComPtr<IXMLDOMDocument> doc;
		HRESULT res=RWCoCreateInstance(doc, msxmlDocClassName);
		if(SUCCEEDED(res))
		{
			res=doc->createElement(CComBSTR(name),&m_key);
			if(!m_key)
				res=E_UNEXPECTED;
		}
		return res;
	}
	
	virtual long Create(IStorge& parent,LPCTSTR name,Modes mode)
	{
		assert(!(!static_cast<CStgXML&>(parent)));
		assert(!m_key);
		HRESULT res=Open(parent,name,mode);
		if(res!=S_OK)
		{
			CComPtr<IXMLDOMDocument> doc;
			res=static_cast<CStgXML&>(parent).m_key->get_ownerDocument(&doc);
			if(SUCCEEDED(res))
			{
				if(!doc)
					res=E_UNEXPECTED;
				else
				{
					res=doc->createElement(CComBSTR(name),&m_key);
					if(SUCCEEDED(res))
					{
						if(!m_key)
							res=E_UNEXPECTED;
						else
						{
							CComPtr<IXMLDOMNode> nu;
							res=static_cast<CStgXML&>(parent).m_key->appendChild(m_key,&nu);
						}
					}
				}
			}
		}
		return res;
	}

	virtual long Open(IStorge& parent,LPCTSTR name,Modes /*mode*/)
	{
		assert(!(!static_cast<CStgXML&>(parent)));
		assert(!m_key);
		return Open(static_cast<IXMLDOMNode*>(static_cast<CStgXML&>(parent).m_key),name);
	}


	virtual long SetString(LPCTSTR name,LPCTSTR data)
	{
		assert(!(!m_key));
		return m_key->setAttribute(CComBSTR(name),CComVariant(data));

	}

	virtual long GetString(LPCTSTR name,LPTSTR data,size_t& size)
	{
		assert(!(!m_key));
		CComVariant val;
		long res=m_key->getAttribute(CComBSTR(name),&val);
		if(SUCCEEDED(res))
		{
			if(val.vt==VT_BSTR)
			{
				size_t len=size_t(::SysStringLen(val.bstrVal));
#ifdef  UNICODE 
				if(size>=(len+1))
				{
					std::char_traits<OLECHAR>::copy(data,val.bstrVal,len);
					data[len]=_T('\0');
				}
#else
				int slen=len;
				len=::WideCharToMultiByte(CP_OEMCP,0,val.bstrVal,slen,0,0,NULL,NULL)/sizeof(TCHAR);
				if(size>=len)
				{
					len=::WideCharToMultiByte(CP_OEMCP,0,val.bstrVal,slen,data,size*sizeof(TCHAR),NULL,NULL)/sizeof(TCHAR);
					if(len==0)
						res=::GetLastError();
					--len;
				}
#endif	//UNICODE
				else
					res=ERROR_MORE_DATA;
				size=len;
			}
			else
				res=(val.vt==VT_NULL)?ERROR_FILE_NOT_FOUND:ERROR_UNSUPPORTED_TYPE;
		}
		return res;
	}

	long Load(LPCTSTR filename,LPCTSTR name=_T("config"),LPCOLESTR msxmlDocClassName=OLESTR("Msxml2.FreeThreadedDOMDocument"))
	{
		assert(!m_key);
		CComPtr<IXMLDOMDocument> doc;
		long res=RWCoCreateInstance(doc, msxmlDocClassName);
		if(SUCCEEDED(res))
		{
			if(!doc)
				res=E_UNEXPECTED;
			else
			{
				res=doc->put_async(VARIANT_FALSE);
				if(SUCCEEDED(res))
				{
					VARIANT_BOOL loaded;
					res=doc->load(CComVariant(filename),&loaded);
					if(SUCCEEDED(res))
					{
						if(loaded==VARIANT_TRUE)
							res=Open(static_cast<IXMLDOMNode*>(doc),name);
						else
							res=E_FAIL;
					}

				}
			}
		}
		return res;
	}

	long Save(LPCTSTR filename)
	{
		assert(!(!m_key));
		CComPtr<IXMLDOMDocument> doc;
		long res=m_key->get_ownerDocument(&doc);
		if(SUCCEEDED(res))
		{
			if(!doc)
				res=E_UNEXPECTED;
			else
			{
				CComPtr<IXMLDOMNode> parent;
				res=m_key->get_parentNode(&parent);
				if(SUCCEEDED(res)
					&& !parent)
				{
					CComPtr<IXMLDOMProcessingInstruction> pi;
					if(SUCCEEDED(doc->createProcessingInstruction(CComBSTR(OLESTR("xml")),CComBSTR(OLESTR("version=\"1.0\"")),&pi)))
					{
						CComPtr<IXMLDOMNode> nu;
						doc->appendChild(pi,&nu);
					}

					CComPtr<IXMLDOMNode> nu;
					res=doc->appendChild(m_key,&nu);
					if(FAILED(res))
						return res;
				}
				res=doc->save(CComVariant(filename));
			}
		}
		return res;
	}
protected:
	long Open(IXMLDOMNode* parent,LPCTSTR name)
	{
		assert(parent!=0);
		assert(!m_key);
		CComPtr<IXMLDOMNode> node;
		HRESULT res=parent->selectSingleNode(CComBSTR(name),&node);
		if(SUCCEEDED(res))
		{
			if(!node)
				res=ERROR_FILE_NOT_FOUND;
			else
			{
				res=node.QueryInterface(&m_key);
				if(SUCCEEDED(res)
					&& (!m_key))
					res=E_UNEXPECTED;
			}
		}
		return res;
	}
private:
	CStgXML(const CStgXML&);
	CStgXML& operator=(const CStgXML&);
private:
	CComPtr<IXMLDOMElement> m_key;
};

}//namespace sstate

#endif // WTL_DW_STGXML_H_INCLUDED_