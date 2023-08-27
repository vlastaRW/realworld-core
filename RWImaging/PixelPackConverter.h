#pragma once

static const BYTE PROC_ENTER[] =
{
	0x55,									// push ebp
	0x8b, 0xec,								// mov ebp, esp
	0x53, 0x51, 0x52, 0x56, 0x57,			// push ebx, ecx, edx, esi, edi
	0x8b, 0x75, 0x0c,						// mov esi, [ebp+0x0c]
	0x8b, 0x7d, 0x10,						// mov edi, [ebp+0x10]
	0x8b, 0x6d, 0x14,						// mov ebp, [ebp+0x14]
};

static const BYTE PROC_LEAVE[] =
{
	0x5f, 0x5e, 0x5a, 0x59, 0x5b,			// pop edi, esi, edx, ecx, ebx
	0x5d,									// pop ebp
	0xc2, 0x10, 0x00,						// ret 0x10
};

static const BYTE PROC_CYCLE[] =
{
	0x4d,									// dec ebp
	0x0f, 0x85, 0x00, 0x00, 0x00, 0x00,		// jnz ....
};

static const BYTE READ_32[] =
{
	0x8b, 0x06,								// mov eax, [esi]
	0x81, 0xc6, 0x04, 0x00, 0x00, 0x00,		// add esi, 4
};

static const BYTE READ_24[] =
{
	0x8a, 0x46, 0x02,						// mov al, [esi+2]
	0xc1, 0xe0, 0x10,						// shl eax, 16
	0x8a, 0x66, 0x01,						// mov ah, [esi+1]
	0x8a, 0x06,								// mov al, [esi]
	0x81, 0xc6, 0x03, 0x00, 0x00, 0x00,		// add esi, 3
};

static const BYTE READ_16[] =
{
	0x66, 0x8b, 0x06,						// mov ax, [esi]
	0x81, 0xc6, 0x02, 0x00, 0x00, 0x00,		// add esi, 2
};

static const BYTE READ_8[] =
{
	0x8a, 0x06,								// mov al, [esi]
	0x46,									// inc esi
};

static const BYTE WRITE_32[] =
{
	0x89, 0x07,								// mov [edi], eax
	0x81, 0xc7, 0x04, 0x00, 0x00, 0x00,		// add edi, 4
};

static const BYTE WRITE_24[] =
{
	0x88, 0x07,								// mov [edi], al
	0x88, 0x67, 0x01,						// mov [edi+1], ah
	0xc1, 0xe8, 0x10,						// shr eax, 16
	0x88, 0x47, 0x02,						// mov [edi+2], al
	0x81, 0xc7, 0x03, 0x00, 0x00, 0x00,		// add edi, 3
};

static const BYTE WRITE_16[] =
{
	0x66, 0x89, 0x07,						// mov [edi], ax
	0x81, 0xc7, 0x02, 0x00, 0x00, 0x00,		// add edi, 2
};

static const BYTE WRITE_8[] =
{
	0x88, 0x07,								// mov [edi], al
	0x47,									// inc edi
};

inline void ProcEAX(BYTE*& a_pCode, DWORD a_dwMask, BYTE a_bShift)
{
	// and eax, a_dwMask
	if (a_dwMask != 0xffffffff)
	{
		*(a_pCode++) = 0x81; *(a_pCode++) = 0xe0;
		*reinterpret_cast<DWORD*>(a_pCode) = a_dwMask; a_pCode += 4;
	}
	// rol eax, xx (if xx != 0)
	if (a_bShift)
	{
		*(a_pCode++) = 0xc1; *(a_pCode++) = 0xc0;
		*(a_pCode++) = a_bShift;
	}
}

inline void ProcEBX(BYTE*& a_pCode, DWORD a_dwMask, BYTE a_bShift)
{
	// mov ebx, a_dwMask
	*(a_pCode++) = 0xc7; *(a_pCode++) = 0xc3;
	*reinterpret_cast<DWORD*>(a_pCode) = a_dwMask; a_pCode += 4;
	// and ebx, eax
	*(a_pCode++) = 0x23; *(a_pCode++) = 0xd8;
	// rol ebx, xx (if xx != 0)
	if (a_bShift)
	{
		*(a_pCode++) = 0xc1; *(a_pCode++) = 0xc3;
		*(a_pCode++) = a_bShift;
	}
}

inline void ProcECX(BYTE*& a_pCode, DWORD a_dwMask, BYTE a_bShift)
{
	// mov ecx, a_dwMask
	*(a_pCode++) = 0xc7; *(a_pCode++) = 0xc1;
	*reinterpret_cast<DWORD*>(a_pCode) = a_dwMask; a_pCode += 4;
	// and ecx, eax
	*(a_pCode++) = 0x23; *(a_pCode++) = 0xc8;
	// rol eax, xx (if xx != 0)
	if (a_bShift)
	{
		*(a_pCode++) = 0xc1; *(a_pCode++) = 0xc1;
		*(a_pCode++) = a_bShift;
	}
}
/*
inline ProcEDX(BYTE*& a_pCode, DWORD a_dwMask, BYTE a_bShift)
{
	// mov edx, a_dwMask
	*(a_pCode++) = 0xc7; *(a_pCode++) = 0xc2;
	*reinterpret_cast<DWORD*>(a_pCode) = a_dwMask; a_pCode += 4;
	// and edx, eax
	*(a_pCode++) = 0x23; *(a_pCode++) = 0xd0;
	// rol eax, xx (if xx != 0)
	if (a_bShift)
	{
		*(a_pCode++) = 0xc1; *(a_pCode++) = 0xc2;
		*(a_pCode++) = a_bShift;
	}
}
*/
inline void ProcMergeEBX(BYTE*& a_pCode)
{
	// or eax, ebx
	*(a_pCode++) = 0x0b; *(a_pCode++) = 0xc3;
}

inline void ProcMergeECX(BYTE*& a_pCode)
{
	// or ebx, ecx
	*(a_pCode++) = 0x0b; *(a_pCode++) = 0xd9;
}

inline void ProcMergeEDX(BYTE*& a_pCode)
{
	// or ebx, edx
	*(a_pCode++) = 0x0b; *(a_pCode++) = 0xda;
}

class CPixelPackConverter
{
private:
	typedef map<DWORD, ULONGLONG> CMasks; // maps shift to mask

public:
	CPixelPackConverter(DWORD a_nSrcWidth, DWORD a_nDstWidth) :
		m_nSrcWidth(a_nSrcWidth), m_nDstWidth(a_nDstWidth)
	{
	}

	void AddItem(DWORD a_nShift, DWORD a_nMask)
	{
		a_nShift &= 0x1f;

		if (a_nMask)
		{
			CMasks::iterator i = m_cMasks.find(a_nShift);
			if (i == m_cMasks.end())
			{
				m_cMasks[a_nShift] = a_nMask;
			}
			else
			{
				i->second |= a_nMask;
			}
		}
	}

	void AddChannel(TChannelSpec a_tIn, TChannelSpec a_tOut)
	{
		if (a_tIn.nWidth >= a_tOut.nWidth)
		{
			AddItem((32+a_tOut.nOffset-a_tIn.nOffset+a_tOut.nWidth-a_tIn.nWidth)&31, (0xffffffffUL>>(32-a_tOut.nWidth))<<(a_tIn.nOffset+a_tIn.nWidth-a_tOut.nWidth));
		}
		else
		{
			int nShift = a_tOut.nOffset-a_tIn.nOffset+a_tOut.nWidth-a_tIn.nWidth;
			for (int nWidth = a_tOut.nWidth; nWidth > 0; nWidth -= a_tIn.nWidth, nShift -= a_tIn.nWidth)
			{
				int nShiftStat = a_tIn.nOffset + (nWidth < a_tIn.nWidth ? a_tIn.nWidth-nWidth : 0);
				AddItem((32+nShift)&31, (0xffffffffUL>>(32-(nWidth < a_tIn.nWidth ? nWidth : a_tIn.nWidth))) << nShiftStat);
			}
		}
	}

	bool operator<(const CPixelPackConverter& a_cOther) const
	{
		if (m_nSrcWidth < a_cOther.m_nSrcWidth) return true;
		if (m_nSrcWidth > a_cOther.m_nSrcWidth) return false;
		if (m_nDstWidth < a_cOther.m_nDstWidth) return true;
		if (m_nDstWidth > a_cOther.m_nDstWidth) return false;
		return m_cMasks < a_cOther.m_cMasks;
	}

#define ADD_CODE(x) CopyMemory(pTmp,x,sizeof(x));pTmp+=sizeof(x)

	BYTE* CreateConverter() const
	{
		
		BYTE* pOut = reinterpret_cast<BYTE*>(VirtualAlloc(NULL, 512, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
		BYTE* pTmp = pOut;

		ADD_CODE(PROC_ENTER);
		BYTE* pLoop = pTmp;

		switch (m_nSrcWidth)
		{
		case 32:ADD_CODE(READ_32);break;
		case 24:ADD_CODE(READ_24);break;
		case 16:ADD_CODE(READ_16);break;
		case 8: ADD_CODE(READ_8); break;
		default:
			delete[] pOut;
			return NULL;
		}

		CMasks::const_iterator i = m_cMasks.begin();
		switch (m_cMasks.size())
		{
		case 0:
			VirtualFree(pOut, 0, MEM_RELEASE);
			return NULL;
		case 1:
			ProcEAX(pTmp, i->second, i->first);
			break;
		case 2:
			ProcEBX(pTmp, i->second, i->first); i++;
			ProcEAX(pTmp, i->second, i->first);
			ProcMergeEBX(pTmp);
			break;
		default:
			ProcEBX(pTmp, i->second, i->first); i++;
			{
				CMasks::const_iterator iLast = i;
				i++;
				for (; i != m_cMasks.end(); i++)
				{
					ProcECX(pTmp, i->second, i->first);
					ProcMergeECX(pTmp);
				}
				ProcEAX(pTmp, iLast->second, iLast->first);
				ProcMergeEBX(pTmp);
			}
			break;
		}
		switch (m_nDstWidth)
		{
		case 32:ADD_CODE(WRITE_32);break;
		case 24:ADD_CODE(WRITE_24);break;
		case 16:ADD_CODE(WRITE_16);break;
		case 8: ADD_CODE(WRITE_8); break;
		default:
			VirtualFree(pOut, 0, MEM_RELEASE);
			return NULL;
		}

		ADD_CODE(PROC_CYCLE);
		*reinterpret_cast<DWORD*>(pTmp-4) = pLoop-pTmp; // JNZ offset (unaligned access)

		ADD_CODE(PROC_LEAVE);

		return pOut;
	}
	static void DeleteConverter(BYTE* a_pConverter)
	{
		if (a_pConverter != NULL)
			VirtualFree(a_pConverter, 0, MEM_RELEASE);
	}
	static void __stdcall OptimizedConveter(CPixelPackConverter const* p, BYTE const* a_pSrc, BYTE* a_pDst, ULONG a_nItems)
	{
		while (a_nItems > 0)
		{
			DWORD dw;
			if (p->m_nSrcWidth > 2)
			{
				if (p->m_nSrcWidth == 4)
					dw = *reinterpret_cast<DWORD const*>(a_pSrc);
				else
					dw = a_pSrc[0]|(DWORD(a_pSrc[1])<<8)|(DWORD(a_pSrc[2])<<16);
			}
			else
			{
				if (p->m_nSrcWidth == 2)
					dw = *reinterpret_cast<WORD const*>(a_pSrc);
				else
					dw = *a_pSrc;
			}
			a_pSrc += p->m_nSrcWidth;

			DWORD dwDst = 0;
			for (CMasks::const_iterator i = p->m_cMasks.begin(); i != p->m_cMasks.end(); ++i)
			{
				// doing rol in c is lame
				dwDst |= ((dw>>(32-i->first))|(dw<<i->first))&ULONG(i->second);
			}

			if (p->m_nDstWidth > 2)
			{
				if (p->m_nDstWidth == 4)
					*reinterpret_cast<DWORD*>(a_pDst) = dwDst;
				else
				{
					a_pDst[0] = dwDst;
					a_pDst[1] = dwDst>>8;
					a_pDst[2] = dwDst>>16;
				}
			}
			else
			{
				if (p->m_nDstWidth == 2)
					*reinterpret_cast<WORD*>(a_pDst) = dwDst;
				else
					*a_pDst = dwDst;
			}
			a_pDst += p->m_nDstWidth;

			--a_nItems;
		}
	}
	static void __stdcall DefaultConveter(CPixelPackConverter const* p, BYTE const* a_pSrc, BYTE* a_pDst, ULONG a_nItems)
	{
		while (a_nItems > 0)
		{
			ULONGLONG dw;
			if (p->m_nSrcWidth == 8)
			{
				dw = *reinterpret_cast<ULONGLONG const*>(a_pSrc);
			}
			else
			{
				dw = 0;
				for (ULONG i = 0; i < p->m_nSrcWidth; ++i)
					dw |= ULONGLONG(a_pSrc[i])<<(i<<3);
			}
			a_pSrc += p->m_nSrcWidth;

			ULONGLONG dwDst = 0;
			for (CMasks::const_iterator i = p->m_cMasks.begin(); i != p->m_cMasks.end(); ++i)
			{
				dwDst |= (dw<<i->first)&i->second;
			}

			if (p->m_nDstWidth == 8)
			{
				*reinterpret_cast<ULONGLONG*>(a_pDst) = dwDst;
			}
			else
			{
				for (ULONG i = 0; i < p->m_nDstWidth; ++i)
				{
					*a_pDst = dwDst;
					dw >>= 8;
				}
			}
			a_pDst += p->m_nDstWidth;

			--a_nItems;
		}
	}

private:
	DWORD m_nSrcWidth;
	DWORD m_nDstWidth;
	CMasks m_cMasks;
};
