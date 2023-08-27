
#pragma once

#include <shlobj.h>


class CDragDropHelper :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDragDropHelper>,
	public IDataObject,
	public IDropSource
{
public:
	CDragDropHelper() : m_dwLastEffect(DROPEFFECT_NONE), m_cde(0), m_rgde(NULL)
	{
	}
	~CDragDropHelper()
	{
		for (int i = 0; i < m_cde; ++i)
		{
			CoTaskMemFree(m_rgde[i].fe.ptd);
			ReleaseStgMedium(&m_rgde[i].stgm);
		}

		CoTaskMemFree(m_rgde);
	}

    // Maps
    BEGIN_COM_MAP(CDragDropHelper)
        COM_INTERFACE_ENTRY(IDataObject)
        COM_INTERFACE_ENTRY(IDropSource)
    END_COM_MAP()

    // Operations
    bool InitFile(LPCTSTR a_pszDraggedFile)
	{
		FORMATETC fetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stg = { TYMED_HGLOBAL };
		HGLOBAL   hgl;
		size_t    cbyData = sizeof(DROPFILES);

		// Calc how much space is needed to hold all the filenames
		cbyData += sizeof(TCHAR) * (1 + _tcslen(a_pszDraggedFile));
		//std::vector<CDraggedFileInfo>::iterator it;

		//for ( it = m_vecDraggedFiles.begin(); it != m_vecDraggedFiles.end(); it++ )
		//	{
		//	sTempFilePath = sTempDir + it->sFilename;

		//	cbyData += sizeof(TCHAR) * (1 + sTempFilePath.GetLength());
		//	}

		// One more TCHAR for the final null char
		cbyData += sizeof(TCHAR);

		// Alloc a buffer to hold the DROPFILES data.
		hgl = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, cbyData);

		if (NULL == hgl)
			return false;

		DROPFILES* pDrop = (DROPFILES*) GlobalLock(hgl);

		if (NULL == pDrop)
		{
			GlobalFree(hgl);
			return false;
		}

		pDrop->pFiles = sizeof(DROPFILES);

	#ifdef _UNICODE
		pDrop->fWide = 1;
	#endif

		// Copy the filenames into the buffer.
		LPTSTR pszFilename = (LPTSTR) (pDrop + 1);
		_tcscpy(pszFilename, a_pszDraggedFile);
	    
		//for ( it = m_vecDraggedFiles.begin(); it != m_vecDraggedFiles.end(); it++ )
		//	{
		//	sTempFilePath = sTempDir + it->sFilename;
		//	int fd = _tcreat ( sTempFilePath, _S_IREAD|_S_IWRITE );

		//	if ( -1 != fd )
		//		_close ( fd );

		//	_tcscpy ( pszFilename, sTempFilePath );
		//	pszFilename += sTempFilePath.GetLength() + 1;

		//	it->sTempFilePath = sTempFilePath;

		//	// Keep track of this temp file so we can clean up when the app exits.
		//	g_vecsTempFiles.push_back ( sTempFilePath );
		//	}

		GlobalUnlock(hgl);
		stg.hGlobal = hgl;

		if (FAILED(SetData(&fetc, &stg, TRUE)))
		{
			GlobalFree ( hgl );
			return false;
		}

		return true;
	}

	template<typename TStruct>
    bool InitStruct(CLIPFORMAT a_eFormat, TStruct const& a_tData)
	{
		return InitStruct(CLIPFORMAT a_eFormat, reinterpret_cast<BYTE const*>(&a_tData), sizeof a_tData);
	}

    bool InitStruct(CLIPFORMAT a_eFormat, BYTE const* a_pData, ULONG a_nData)
	{
		FORMATETC fetc = { a_eFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stg = { TYMED_HGLOBAL };
		stg.hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, a_nData);
		CopyMemory(GlobalLock(stg.hGlobal), a_pData, a_nData);
		GlobalUnlock(stg.hGlobal);
		return SUCCEEDED(SetData(&fetc, &stg, TRUE));
	}

	// IDataObject
	STDMETHODIMP SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
	{
		if (!fRelease)
			return E_NOTIMPL;

		LPDATAENTRY pde;
		HRESULT hres = FindFORMATETC(pformatetc, &pde, TRUE);

		if ( SUCCEEDED(hres) )
		{
			if ( pde->stgm.tymed )
			{
				ReleaseStgMedium ( &pde->stgm );
				ZeroMemory ( &pde->stgm, sizeof(STGMEDIUM) );
			}

			if ( fRelease ) 
			{
				pde->stgm = *pmedium;
				hres = S_OK;
			}
			else 
			{
				hres = AddRefStgMedium ( pmedium, &pde->stgm, TRUE );
			}

			pde->fe.tymed = pde->stgm.tymed;    /* Keep in sync */

			/* Subtlety!  Break circular reference loop */
			if ( GetCanonicalIUnknown ( pde->stgm.pUnkForRelease ) ==
				 GetCanonicalIUnknown ( static_cast<IDataObject*>(this) ))
			{
				pde->stgm.pUnkForRelease->Release();
				pde->stgm.pUnkForRelease = NULL;
			}
		}

		return hres;
	}

	STDMETHODIMP GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium)
	{
		LPDATAENTRY pde;
		HRESULT hres;

		hres = FindFORMATETC(pformatetcIn, &pde, FALSE);
		if (SUCCEEDED(hres))
			hres = AddRefStgMedium (&pde->stgm, pmedium, FALSE);

		return hres;
	}

	// This typedef creates an IEnumFORMATETC enumerator that the drag source
	// uses in EnumFormatEtc().
	typedef CComEnumOnSTL<IEnumFORMATETC,           // name of enumerator interface
						  &IID_IEnumFORMATETC,      // IID of enumerator interface
						  FORMATETC,                // type of object to return
						  _Copy<FORMATETC>,         // copy policy class
						  std::vector<FORMATETC> >  // type of collection holding the data
		CEnumFORMATETCImpl;

	STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
	{   
		HRESULT hr;
		CComObject<CEnumFORMATETCImpl>* pImpl = NULL;
		hr = CComObject<CEnumFORMATETCImpl>::CreateInstance(&pImpl);
		if (FAILED(hr)) return hr;

		CComPtr<IEnumFORMATETC> pTmp = pImpl;
	    
		// Fill in m_vecSupportedFormats with the formats that the caller has
		// put in this object.
		m_vecSupportedFormats.clear();

		for ( int i = 0; i < m_cde; i++ )
			m_vecSupportedFormats.push_back(m_rgde[i].fe);

		// Init the enumerator, passing it our vector of supported FORMATETCs.
		hr = pImpl->Init(GetUnknown(), m_vecSupportedFormats);
	    
		if (FAILED(hr))
		{
			return E_UNEXPECTED;
		}

		// Return the requested interface to the caller.
		hr = pImpl->QueryInterface(ppenumFormatEtc);

		return hr;
	}
	STDMETHODIMP QueryGetData(FORMATETC* pformatetc)
	{
		LPDATAENTRY pde;
		return FindFORMATETC(pformatetc, &pde, FALSE);
	}

	STDMETHODIMP GetDataHere(FORMATETC* pformatetc, STGMEDIUM *pmedium) { return E_NOTIMPL; }
	STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut) { return E_NOTIMPL; }
	STDMETHODIMP DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) { return E_NOTIMPL; }
	STDMETHODIMP DUnadvise(DWORD dwConnection) { return E_NOTIMPL; }
	STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise) { return E_NOTIMPL; }

    // IDropSource
    STDMETHODIMP QueryContinueDrag ( BOOL fEscapePressed, DWORD grfKeyState )
	{
		// If ESC was pressed, cancel the drag. If the left button was released,
		// do the drop.
		if (fEscapePressed)
			return DRAGDROP_S_CANCEL;
		if (!(grfKeyState & MK_LBUTTON))
		{
			if (DROPEFFECT_NONE == m_dwLastEffect)
				return DRAGDROP_S_CANCEL;

			return DRAGDROP_S_DROP;
		}
		return S_OK;
	}
	STDMETHODIMP GiveFeedback(DWORD dwEffect)
	{
		m_dwLastEffect = dwEffect;
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}

    // Implementation
protected:
	typedef struct 
	{
		FORMATETC fe;
		STGMEDIUM stgm;
	} DATAENTRY, *LPDATAENTRY;

	// Helper functions used by IDataObject methods
	HRESULT FindFORMATETC(FORMATETC* pfe, LPDATAENTRY* ppde, BOOL fAdd)
	{
		*ppde = NULL;

		/* Comparing two DVTARGETDEVICE structures is hard, so we don't even try */
		if ( pfe->ptd != NULL )
			return DV_E_DVTARGETDEVICE;

		/* See if it's in our list */
		for (int ide = 0; ide < m_cde; ++ide)
		{
			if (m_rgde[ide].fe.cfFormat == pfe->cfFormat &&
				m_rgde[ide].fe.dwAspect == pfe->dwAspect &&
				m_rgde[ide].fe.lindex == pfe->lindex)
			{
				if (fAdd || (m_rgde[ide].fe.tymed & pfe->tymed))
				{
					*ppde = &m_rgde[ide];
					return S_OK;
				}
				else 
				{
					return DV_E_TYMED;
				}
			}
		}

		if (!fAdd)
			return DV_E_FORMATETC;

		LPDATAENTRY pdeT = (LPDATAENTRY) CoTaskMemRealloc(m_rgde, sizeof(DATAENTRY) * (m_cde+1));

		if (pdeT)
		{
			m_rgde = pdeT;
			m_cde++;
			m_rgde[m_cde-1].fe = *pfe;
	        
			ZeroMemory(&pdeT[m_cde-1].stgm, sizeof(STGMEDIUM));
			*ppde = &m_rgde[m_cde-1];

			return S_OK;
		} 
		else 
		{
			return E_OUTOFMEMORY;
		}
	}
	HRESULT AddRefStgMedium(STGMEDIUM* pstgmIn, STGMEDIUM* pstgmOut, BOOL fCopyIn)
	{
		HRESULT hres = S_OK;
		STGMEDIUM stgmOut = *pstgmIn;

		if (pstgmIn->pUnkForRelease == NULL && !(pstgmIn->tymed & (TYMED_ISTREAM | TYMED_ISTORAGE)))
		{
			if (fCopyIn)
			{
				/* Object needs to be cloned */
				if (pstgmIn->tymed == TYMED_HGLOBAL)
				{
					stgmOut.hGlobal = GlobalClone(pstgmIn->hGlobal);

					if (!stgmOut.hGlobal)
						hres = E_OUTOFMEMORY;
				}
				else 
					hres = DV_E_TYMED;      /* Don't know how to clone GDI objects */
			} 
			else  
				stgmOut.pUnkForRelease = static_cast<IDataObject*>(this);
		}

		if (SUCCEEDED(hres))
		{
			switch (stgmOut.tymed)
			{
			case TYMED_ISTREAM:
				stgmOut.pstm->AddRef();
				break;

			case TYMED_ISTORAGE:
				stgmOut.pstg->AddRef();
				break;
			}

			if (stgmOut.pUnkForRelease)
				stgmOut.pUnkForRelease->AddRef();

			*pstgmOut = stgmOut;
		}

		return hres;
	}
	HGLOBAL GlobalClone(HGLOBAL hglobIn)
	{
		HGLOBAL hglobOut = NULL;
		LPVOID pvIn = GlobalLock(hglobIn);

		if (pvIn)
		{
			SIZE_T cb = GlobalSize(hglobIn);
			HGLOBAL hglobOut = GlobalAlloc(GMEM_FIXED, cb);
			if (hglobOut)
				CopyMemory(hglobOut, pvIn, cb);
			GlobalUnlock(hglobIn);
		}

		return hglobOut;
	}
	IUnknown* GetCanonicalIUnknown(IUnknown* a_p)
	{
		IUnknown* pCanonical = NULL;

		if (a_p && SUCCEEDED(a_p->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&pCanonical))))
			pCanonical->Release();
		else 
			pCanonical = a_p;
	        
		return pCanonical;
	}

private:
    DWORD m_dwLastEffect;
	std::vector<FORMATETC> m_vecSupportedFormats; // list of FORMATETCs for which we have data, used in EnumFormatEtc
	LPDATAENTRY m_rgde; // array of active DATAENTRY entries
	int m_cde; // size of m_rgde
};
