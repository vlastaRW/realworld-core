
#pragma once

#include <math.h>
#include <Quaternion.h>

namespace WTL
{

class CGradientFillHelper
{
public:
	typedef WINGDIAPI BOOL WINAPI GradientFillProc(HDC,PTRIVERTEX,ULONG,PVOID,ULONG,ULONG);

public:
	CGradientFillHelper() : m_hLib(NULL), m_pfnGradinetFill(NULL), m_bGradientFillTried(false)
	{
	}
	~CGradientFillHelper()
	{
		if (m_hLib)
		{
			FreeLibrary(m_hLib);
		}
	}

	GradientFillProc* Proc()
	{
		if (!m_bGradientFillTried)
		{
			m_bGradientFillTried = true;
			m_hLib = LoadLibrary(_T("msimg32.dll"));
			if (m_hLib)
			{
				m_pfnGradinetFill = (GradientFillProc*)GetProcAddress(m_hLib, "GradientFill");
			}
		}
		return m_pfnGradinetFill;
	}

private:
	HINSTANCE m_hLib;
	GradientFillProc* m_pfnGradinetFill;
	bool m_bGradientFillTried;
};

extern __declspec(selectany) CGradientFillHelper g_cGradientFillHelper;

class CRotation :
	public CWindowImpl<CRotation>,
	public CThemeImpl<CRotation>
{
public:
	enum
	{
		// notifications
		RTN_ROTATED = 1,
		RTN_PAINTCOMPLETE = 2,
	};
	struct NTNLVERTEX
	{
		float fX, fY, fZ;
		float fNX, fNY, fNZ;
		WORD r, g, b, a;
	};
	struct IBTDATA
	{
		DWORD dwFCC;
		DWORD dwLen;
		DWORD dwVNum;
		DWORD dwTNum;
		BYTE data[0]; // NTNLVERTEX followed by triangles
	};

	CRotation() :
		m_clrBackground(GetSysColor(COLOR_BACKGROUND)), m_pData(NULL), m_fZoom(1.0f),
		m_bColoredWhileDisabled(false), m_bNoBackBuffer(false),
		m_bDragging(0), m_fLastPosX(0.0f), m_fLastPosY(0.0f),
		m_bEnableNotify(true)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
		SetRotation(1.0f, 0.0f, 0.0f, 0.0f);
	}
	~CRotation()
	{
		DestroyData();
	}


	DECLARE_WND_CLASS_EX(_T("WTL_RotationWndClass"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_BACKGROUND);

	BEGIN_MSG_MAP(CRotation)
		CHAIN_MSG_MAP(CThemeImpl<CRotation>)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
	END_MSG_MAP()

	// operations
public:
	void SetRotation(float a_fAxeX, float a_fAxeY, float a_fAxeZ, float a_fRotation)
	{
		m_cQuat = CQuaternionf(a_fAxeX, a_fAxeY, a_fAxeZ, a_fRotation);
		if (m_hWnd) Invalidate(FALSE);
	}

	void SetQuaternion(TQuaternionf const& a_tQuat)
	{
		m_cQuat = a_tQuat;
		if (m_hWnd) Invalidate(FALSE);
	}

	void SetDirection(float a_fX, float a_fY, float a_fZ)
	{
//			static float s_aStdVect[] = {1.0f, 0.0f, 0.0f};
//			float fMag, *fInp = (float*)lParam;
//			fMag = sqrtf(fInp[0]*fInp[0] + fInp[1]*fInp[1] + fInp[2]*fInp[2]);
//			if (fMag < 1e-6f)
//			{
//				fMag = 1.0f;
//				fInp = s_aStdVect;
//			}
//			fMag = wParam & 1 ? -1.0f/fMag : 1.0f/fMag;
///*			float fNorm[3] = {fInp[0]*fMag, fInp[1]*fMag, fInp[2]*fMag};
//			float beta = asinf(fInp[2]*fMag);
//			float alfa = asinf(fInp[1]*fMag/cosf(beta));
//			float q1[4], qWorld[4];
//			float fAxe[4] = {0.0f, 1.0f, 0.0f, 0.0f};
//			axis_to_quat(fAxe+1, beta, q1);
//			axis_to_quat(fAxe, alfa, qWorld);
//			add_quats(q1, qWorld, qWorld);
//*/
//			float qWorld[4];
//			float alfa, temp;
//			switch (wParam & 0xfffffffe)
//			{
//			case ROTATION_VECTOR_PX:
//				qWorld[0] = 0.0f;
//				qWorld[1] = fInp[2]*fMag;
//				qWorld[2] = -fInp[1]*fMag;
//				temp = fInp[0];
//				alfa = acosf(fInp[0]*fMag);
//				qWorld[3] = cosf(alfa*0.5f);
//				break;
//			case ROTATION_VECTOR_PY:
//				qWorld[0] = -fInp[2]*fMag;
//				qWorld[1] = 0.0f;
//				qWorld[2] = fInp[0]*fMag;
//				temp = fInp[1];
//				alfa = acosf(fInp[1]*fMag);
//				qWorld[3] = cosf(alfa*0.5f);
//				break;
//			default:
//				qWorld[0] = fInp[1]*fMag;
//				qWorld[1] = -fInp[0]*fMag;
//				qWorld[2] = 0.0f;
//				temp = fInp[2];
//				alfa = acosf(fInp[2]*fMag);
//				qWorld[3] = cosf(alfa*0.5f);
//				break;
//			}
//			if ((qWorld[0] == 0.0f && qWorld[1] == 0.0f && qWorld[2] == 0.0f) || fMag > 1e+7)
//			{
//				qWorld[0] = (wParam & 1) ? (temp < 0.0f ? -1.0f : 1.0f) : (temp < 0.0f ? 1.0f : -1.0f);
//				qWorld[3] = 0.0f;
//			}
//			else
//			{
//				fMag = 1.0f/sqrtf(qWorld[0]*qWorld[0] + qWorld[1]*qWorld[1] + qWorld[2]*qWorld[2])*sinf(alfa*0.5f);
//				qWorld[0] *= fMag;
//				qWorld[1] *= fMag;
//				qWorld[2] *= fMag;
//			}
//			SetWindowLong(hWnd, RTWL_QUAT0, *(LONG*)qWorld);
//			SetWindowLong(hWnd, RTWL_QUAT1, *(LONG*)(qWorld+1));
//			SetWindowLong(hWnd, RTWL_QUAT2, *(LONG*)(qWorld+2));
//			SetWindowLong(hWnd, RTWL_QUAT3, *(LONG*)(qWorld+3));
//			InvalidateRect(hWnd, NULL, FALSE);
//		}
	}

	void GetMatrix3x3f(float* a_afMatrix3x3) const
	{
	}

	void GetMatrix4x4f(TMatrix4x4f* a_pMatrix4x4) const
	{
		MatrixAssign(a_pMatrix4x4, m_cQuat);
	}

	TQuaternionf const& GetQuaternion() const
	{
		return m_cQuat;
	}

	void GetDirection(float* a_pfX, float* a_pfY, float* a_pfZ) const
	{
			//float qWorld[4], fMatrix[16];
			//DWORD *dwWorld = (DWORD*)qWorld;
			//dwWorld[0] = GetWindowLong(hWnd, RTWL_QUAT0);
			//dwWorld[1] = GetWindowLong(hWnd, RTWL_QUAT1);
			//dwWorld[2] = GetWindowLong(hWnd, RTWL_QUAT2);
			//dwWorld[3] = GetWindowLong(hWnd, RTWL_QUAT3);
			//build_rotmatrix(fMatrix, qWorld);
			//float *fOut = (float*)lParam;
			//switch (wParam)
			//{
			//case ROTATION_VECTOR_PX:
			//	fOut[0] = fMatrix[0];
			//	fOut[1] = fMatrix[1];
			//	fOut[2] = fMatrix[2];
			//	break;
			//case ROTATION_VECTOR_NX:
			//	fOut[0] = -fMatrix[0];
			//	fOut[1] = -fMatrix[1];
			//	fOut[2] = -fMatrix[2];
			//	break;
			//case ROTATION_VECTOR_PY:
			//	fOut[0] = fMatrix[4];
			//	fOut[1] = fMatrix[5];
			//	fOut[2] = fMatrix[6];
			//	break;
			//case ROTATION_VECTOR_NY:
			//	fOut[0] = -fMatrix[4];
			//	fOut[1] = -fMatrix[5];
			//	fOut[2] = -fMatrix[6];
			//	break;
			//case ROTATION_VECTOR_NZ:
			//	fOut[0] = -fMatrix[8];
			//	fOut[1] = -fMatrix[9];
			//	fOut[2] = -fMatrix[10];
			//	break;
			//default:
			//	fOut[0] = fMatrix[8];
			//	fOut[1] = fMatrix[9];
			//	fOut[2] = fMatrix[10];
			//	break;
			//}
	}

	void ApplyRotation(float a_fAxeX, float a_fAxeY, float a_fAxeZ, float a_fRotation)
	{
	}

	bool LoadGeometryFromResource(HINSTANCE a_hInstance, LPCTSTR a_pszResourceID) // resource type must be "RT_IBTFILE"
	{
		HRSRC hRC;
		HGLOBAL hRS;

		if ((hRC = FindResource(a_hInstance, a_pszResourceID, _T("RT_IBTFILE"))) &&
			(hRS = LoadResource(a_hInstance, hRC)))
		{
			bool bReturn = LoadGeometryFromMemory(reinterpret_cast<const IBTDATA*>(LockResource(hRS)));
			FreeResource(hRS);
			return bReturn;
		}
		else
		{
			return false;
		}
	}

	bool LoadGeometryFromMemory(const IBTDATA* a_pData)
	{
		DestroyData();
		if (a_pData)
		{
			try
			{
				if (a_pData->dwFCC != mmioFOURCC('I', 'B', 'T', '1') || a_pData->dwLen < sizeof(IBTDATA))
					return false;

				m_pData = reinterpret_cast<IBTDATA*>(HeapAlloc(GetProcessHeap(), 0, a_pData->dwLen));
				if (m_pData == NULL)
					return false;

				CopyMemory(m_pData, a_pData, a_pData->dwLen);

				m_fZoom = ComputeGoodZoom(m_pData);
			}
			catch (...)
			{
				DestroyData();
				return false;
			}
		}
		Invalidate(FALSE);
		return true;
	}

	void SetBackgroundColor(COLORREF a_clrBackground)
	{
		m_clrBackground = a_clrBackground;
	}

	COLORREF GetBackgroundColor() const
	{
		return m_clrBackground;
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_bDragging = false;
		ReleaseCapture();
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT rWin;
		GetClientRect(&rWin);
		POINT p = {(short)LOWORD(a_lParam), (short)HIWORD(a_lParam)};

		m_bDragging = true;
		float mult = 2.0f/(rWin.right > rWin.bottom ? rWin.bottom : rWin.right);
		m_fLastPosX = (p.x-((rWin.right+rWin.left)>>1)) * mult;
		m_fLastPosY = (((rWin.bottom+rWin.top)>>1)-p.y) * mult;
		SetCapture();
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_bDragging)
		{
			RECT rWin;
			GetClientRect(&rWin);
			POINT p = {(short)LOWORD(a_lParam), (short)HIWORD(a_lParam)};

			float mult = 2.0f/(rWin.right > rWin.bottom ? rWin.bottom : rWin.right);
			float x = (p.x-((rWin.right+rWin.left)>>1)) * mult;
			float y = (((rWin.bottom+rWin.top)>>1)-p.y) * mult;
			m_cQuat = TrackballRotation(m_fLastPosX, m_fLastPosY, x, y) + m_cQuat;
			m_fLastPosX = x;
			m_fLastPosY = y;
			InvalidateRect(NULL, FALSE);
			if (m_bEnableNotify)
			{
				HWND hPar = GetParent();
				if (hPar)
				{
					NMHDR nm = {m_hWnd, GetWindowLong(GWLP_ID), RTN_ROTATED};
					::SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
				}
			}
		}
		return 0;
	}

	LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
//		HWND hPar = GetParent(hWnd);

		// get window style
		DWORD dwStyle = GetWindowLong(GWL_STYLE);

		PAINTSTRUCT ps;
		HDC hDC;
		HDC hDC2 = BeginPaint(&ps);
		HGDIOBJ hBmp;
		RECT rcWin;
		GetClientRect(&rcWin);
		if (m_bNoBackBuffer)
		{
			hDC = hDC2;
		}
		else
		{
			hDC = CreateCompatibleDC(hDC2);
			hBmp = SelectObject(hDC, CreateCompatibleBitmap(hDC2, rcWin.right, rcWin.bottom));
		}

		// get client rectangle and rectangle to update
		RECT rUpd;
		if (ps.rcPaint.bottom == 0 || ps.rcPaint.right == 0)
			rUpd = rcWin;
		else
			rUpd = ps.rcPaint;

		if (dwStyle & WS_DISABLED && !m_bColoredWhileDisabled)
		{
			DWORD sh = GetSysColor(COLOR_3DSHADOW);
			DWORD hl = GetSysColor(COLOR_3DHIGHLIGHT);
			DWORD col = m_clrBackground;
			BYTE *b = (BYTE*)&col;
			col = b[0] + b[1] + b[2];
			BYTE *bs = (BYTE*)&sh;
			BYTE *bh = (BYTE*)&hl;
			HBRUSH hbr = CreateSolidBrush((bs[0] + (bh[0] - bs[0]) * col / 0x300) | ((bs[1] + (bh[1] - bs[1]) * col / 0x300)<<8) | ((bs[2] + (bh[2] - bs[2]) * col / 0x300)<<16));
			FillRect(hDC, &rUpd, hbr);
			DeleteObject(hbr);
		}
		else
		{
			HBRUSH hbr = CreateSolidBrush(m_clrBackground);
			FillRect(hDC, &rUpd, hbr);
			DeleteObject(hbr);
		}

		if (m_pData && m_pData->dwLen && m_pData->dwVNum && m_pData->dwTNum)
		{
			NTNLVERTEX *pNTLV = (NTNLVERTEX *)&m_pData->data;
			GRADIENT_TRIANGLE *pTRI = (GRADIENT_TRIANGLE *)(pNTLV+m_pData->dwVNum);
			LPTRIVERTEX pTmp = reinterpret_cast<LPTRIVERTEX>(HeapAlloc(GetProcessHeap(), 0, sizeof(TRIVERTEX)*m_pData->dwVNum));
			float *pX = reinterpret_cast<float*>(HeapAlloc(GetProcessHeap(), 0, sizeof(float)*3*m_pData->dwVNum));
			float *pY = pX + m_pData->dwVNum;
			float *pZ = pY + m_pData->dwVNum;

			float fWorld[16];
			GetMatrix4x4f(reinterpret_cast<TMatrix4x4f*>(fWorld));

			DWORD dwWidth = (rcWin.right-rcWin.left)>>1;
			DWORD dwHeight = (rcWin.bottom-rcWin.top)>>1;

			// transformation and lighting
			float fLight[3] = {0.0f, 0.0f,-1.0f};
			UINT i;
			float mult = (dwWidth > dwHeight ? dwHeight : dwWidth) / m_fZoom * 4.0f;
			if (dwStyle & WS_DISABLED && !m_bColoredWhileDisabled)
			{
				DWORD sh = GetSysColor(COLOR_3DSHADOW);
				DWORD hl = GetSysColor(COLOR_3DHIGHLIGHT);
				for (i = 0; i < m_pData->dwVNum; i++)
				{
					pZ[i] = -(pNTLV[i].fX*fWorld[2] + pNTLV[i].fY*fWorld[6] + pNTLV[i].fZ*fWorld[10])/m_fZoom;
					pX[i] = pNTLV[i].fX*fWorld[0] + pNTLV[i].fY*fWorld[4] + pNTLV[i].fZ*fWorld[8];
					pTmp[i].x = dwWidth + (LONG)(pX[i]/(pZ[i]+5.0f)*mult);
					pY[i] = pNTLV[i].fX*fWorld[1] + pNTLV[i].fY*fWorld[5] + pNTLV[i].fZ*fWorld[9];
					pTmp[i].y = dwHeight - (LONG)(pY[i]/(pZ[i]+5.0f)*mult);
					float fNX = pNTLV[i].fNX*fWorld[0] + pNTLV[i].fNY*fWorld[4] + pNTLV[i].fNZ*fWorld[8];
					float fNY = -(pNTLV[i].fNX*fWorld[1] + pNTLV[i].fNY*fWorld[5] + pNTLV[i].fNZ*fWorld[9]);
					float fNZ = -(pNTLV[i].fNX*fWorld[2] + pNTLV[i].fNY*fWorld[6] + pNTLV[i].fNZ*fWorld[10]);
					DWORD dwLight = (DWORD)((0.3f + 0.7f*(fNX*fLight[0] + fNY*fLight[1] + fNZ*fLight[2]))*0xffff);
					if (dwLight > 0xffff)
						dwLight = 0xffff;
					DWORD col = ((pNTLV[i].r*dwLight)>>16) + ((pNTLV[i].g*dwLight)>>16) + ((pNTLV[i].b*dwLight)>>16);
//							BYTE *b = (BYTE*)&col;
					BYTE *bs = (BYTE*)&sh;
					BYTE *bh = (BYTE*)&hl;
					pTmp[i].Red = (COLOR16)((bs[0]<<8) + (bh[0] - bs[0]) * col / 0x300);
					pTmp[i].Green = (COLOR16)((bs[1]<<8) + (bh[1] - bs[1]) * col / 0x300);
					pTmp[i].Blue = (COLOR16)((bs[2]<<8) + (bh[2] - bs[2]) * col / 0x300);
					pTmp[i].Alpha = pNTLV[i].a;
				}
			}
			else
				for (i = 0; i < m_pData->dwVNum; i++)
				{
					pZ[i] = -(pNTLV[i].fX*fWorld[2] + pNTLV[i].fY*fWorld[6] + pNTLV[i].fZ*fWorld[10])/m_fZoom;
					pX[i] = pNTLV[i].fX*fWorld[0] + pNTLV[i].fY*fWorld[4] + pNTLV[i].fZ*fWorld[8];
					pTmp[i].x = dwWidth + (LONG)(pX[i]/(pZ[i]+5.0f)*mult);
					pY[i] = pNTLV[i].fX*fWorld[1] + pNTLV[i].fY*fWorld[5] + pNTLV[i].fZ*fWorld[9];
					pTmp[i].y = dwHeight - (LONG)(pY[i]/(pZ[i]+5.0f)*mult);
					float fNX = pNTLV[i].fNX*fWorld[0] + pNTLV[i].fNY*fWorld[4] + pNTLV[i].fNZ*fWorld[8];
					float fNY = -(pNTLV[i].fNX*fWorld[1] + pNTLV[i].fNY*fWorld[5] + pNTLV[i].fNZ*fWorld[9]);
					float fNZ = -(pNTLV[i].fNX*fWorld[2] + pNTLV[i].fNY*fWorld[6] + pNTLV[i].fNZ*fWorld[10]);
					DWORD dwLight = (DWORD)((0.3f + 0.7f*(fNX*fLight[0] + fNY*fLight[1] + fNZ*fLight[2]))*0xffff);
					if (dwLight > 0xffff)
						dwLight = 0xffff;
					pTmp[i].Red = (COLOR16)((pNTLV[i].r*dwLight)>>16);
					pTmp[i].Green = (COLOR16)((pNTLV[i].g*dwLight)>>16);
					pTmp[i].Blue = (COLOR16)((pNTLV[i].b*dwLight)>>16);
					pTmp[i].Alpha = pNTLV[i].a;
				}

			GRADIENT_TRIANGLE *pTri = (GRADIENT_TRIANGLE *)HeapAlloc(GetProcessHeap(), 0, sizeof(GRADIENT_TRIANGLE)*m_pData->dwTNum);
			float *pTZs = (float *)HeapAlloc(GetProcessHeap(), 0, sizeof(float)*m_pData->dwTNum);
			DWORD ii;
			for (i = ii = 0; i < m_pData->dwTNum; i++)
			{
				if (((pTmp[pTRI[i].Vertex2].x-pTmp[pTRI[i].Vertex1].x)*(pTmp[pTRI[i].Vertex3].y-pTmp[pTRI[i].Vertex1].y) -
						(pTmp[pTRI[i].Vertex3].x-pTmp[pTRI[i].Vertex1].x)*(pTmp[pTRI[i].Vertex2].y-pTmp[pTRI[i].Vertex1].y)) > 0.0f)
				{
					pTri[ii] = pTRI[i];
					pTZs[ii] = pZ[pTRI[i].Vertex1] + pZ[pTRI[i].Vertex2] + pZ[pTRI[i].Vertex3];
					ii++;
				}
			}

			DWORD j;
			for (i = 1; i < ii; i++)
				for (j = ii-1; j >= i; j--)
					if (pTZs[j] > pTZs[j-1])
					{
						GRADIENT_TRIANGLE tmp;
						tmp = pTri[j]; pTri[j] = pTri[j-1]; pTri[j-1] = tmp;
						float tmpZ;
						tmpZ = pTZs[j]; pTZs[j] = pTZs[j-1]; pTZs[j-1] = tmpZ;
					}
			if (ii)
			{
				if (g_cGradientFillHelper.Proc())
					g_cGradientFillHelper.Proc()(hDC, pTmp, m_pData->dwVNum, pTri, ii, GRADIENT_FILL_TRIANGLE);
				else
				{ // windows 95 or Windows NT 4 - msimg32.dll not found
					HGDIOBJ hSavePen, hSaveBrush;
					hSavePen = SelectObject(hDC, CreatePen(PS_SOLID, 1, 0));
					hSaveBrush = SelectObject(hDC, CreateSolidBrush(0));
					for (i = 0; i < ii; i++)
					{
						POINT p[3] = {{pTmp[pTri[i].Vertex1].x, pTmp[pTri[i].Vertex1].y}, {pTmp[pTri[i].Vertex2].x, pTmp[pTri[i].Vertex2].y}, {pTmp[pTri[i].Vertex3].x, pTmp[pTri[i].Vertex3].y}};
						DWORD clr = ((((pTmp[pTri[i].Vertex1].Red + pTmp[pTri[i].Vertex2].Red + pTmp[pTri[i].Vertex3].Red)/3) & 0xff00)>> 8) |
									(((pTmp[pTri[i].Vertex1].Green + pTmp[pTri[i].Vertex2].Green + pTmp[pTri[i].Vertex3].Green)/3) & 0xff00) |
									((((pTmp[pTri[i].Vertex1].Blue + pTmp[pTri[i].Vertex2].Blue + pTmp[pTri[i].Vertex3].Blue)/3) & 0xff00) << 8);
						DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 1, clr)));
						DeleteObject(SelectObject(hDC, CreateSolidBrush(clr)));
						Polygon(hDC, p, 3);
					}
					DeleteObject(SelectObject(hDC, hSavePen));
					DeleteObject(SelectObject(hDC, hSaveBrush));
				}
			}

			HeapFree(GetProcessHeap(), 0, pTZs);
			HeapFree(GetProcessHeap(), 0, pTri);

			HeapFree(GetProcessHeap(), 0, pTmp);
			HeapFree(GetProcessHeap(), 0, pX);
		}

		if (!m_bNoBackBuffer)
		{
			BitBlt(hDC2, 0, 0, rcWin.right, rcWin.bottom, hDC, 0, 0, SRCCOPY);
			DeleteObject(SelectObject(hDC, hBmp));
			DeleteDC(hDC);
		}
		EndPaint(&ps);

		if (m_bEnableNotify)
		{
			NMHDR nm = {m_hWnd, GetWindowLong(GWLP_ID), RTN_PAINTCOMPLETE};
			::SendMessage(GetParent(), WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
		return 0;
	}

	void ModifyGeomenty(NTNLVERTEX*& v, ULONG& nv, GRADIENT_TRIANGLE*& t, ULONG& nt)
	{
		v = (NTNLVERTEX *)&m_pData->data;
		nv = m_pData->dwVNum;
		t = (GRADIENT_TRIANGLE*)(v+nv);
		nt = m_pData->dwTNum;
	}

private:
	void DestroyData()
	{
		if (m_pData)
			HeapFree(GetProcessHeap(), 0, m_pData);
		m_pData = NULL;
	}
	static float ComputeGoodZoom(IBTDATA* a_pData)
	{
		NTNLVERTEX *vert = (NTNLVERTEX *)&a_pData->data;
		DWORD i;
		float max = 0.0f;
		for (i = 0; i < a_pData->dwVNum; i++, vert++)
		{
			float r = vert->fX*vert->fX + vert->fY*vert->fY + vert->fZ*vert->fZ;
			if (r > max)
				max = r;
		}
		return sqrtf(max);
	}
	CQuaternionf TrackballRotation(float const a_fX1, float const a_fY1, float const a_fX2, float const a_fY2)
	{
		if (a_fX1 == a_fX2 && a_fY1 == a_fY2)
		{
			return TQUATERNIONF_IDENTITY;
		}

		float const fD1 = a_fX1*a_fX1 + a_fY1*a_fY1;
		float const fZ1 = fD1 < 0.5f ? sqrtf(1-fD1) : (0.5f/sqrtf(fD1));

		float const fD2 = a_fX2*a_fX2 + a_fY2*a_fY2;
		float const fZ2 = fD2 < 0.5f ? sqrtf(1-fD2) : (0.5f/sqrtf(fD2));

		float const tAxis[3] = {a_fY2*fZ1-fZ2*a_fY1, fZ2*a_fX1-a_fX2*fZ1, a_fX2*a_fY1-a_fY2*a_fX1};

		float fT = 0.5f*sqrtf((a_fX1-a_fX2)*(a_fX1-a_fX2) + (a_fY1-a_fY2)*(a_fY1-a_fY2) + (fZ1-fZ2)*(fZ1-fZ2));
		if (fT > 1.0f) fT = 1.0f;
		if (fT < -1.0f) fT = -1.0f;

		return CQuaternionf(tAxis, 2.0f * asinf(fT));
	}

private:
	COLORREF m_clrBackground;
	CQuaternionf m_cQuat;
	IBTDATA* m_pData;
	float m_fZoom;
	bool m_bColoredWhileDisabled;
	bool m_bNoBackBuffer;
	bool m_bDragging;
	float m_fLastPosX;
	float m_fLastPosY;
	bool m_bEnableNotify;
};

}; // namespace WTL