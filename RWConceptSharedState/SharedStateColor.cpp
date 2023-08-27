// SharedStateColor.cpp : Implementation of CSharedStateColor

#include "stdafx.h"
#include "SharedStateColor.h"


// CSharedStateColor

STDMETHODIMP CSharedStateColor::RGBAGet(float* a_pRGBA)
{
	try
	{
		a_pRGBA[0] = m_fR;
		a_pRGBA[1] = m_fG;
		a_pRGBA[2] = m_fB;
		a_pRGBA[3] = m_bAlphaEnabled ? m_fA : 1.0f;
		return S_OK;
	}
	catch (...)
	{
		return a_pRGBA ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CSharedStateColor::HLSAGet(float* a_pHLSA)
{
	try
	{
		a_pHLSA[0] = m_fH;
		a_pHLSA[1] = m_fL;
		a_pHLSA[2] = m_fS;
		a_pHLSA[3] = m_bAlphaEnabled ? m_fA : 1.0f;
		return S_OK;
	}
	catch (...)
	{
		return a_pHLSA ? E_UNEXPECTED : E_POINTER;
	}
}

static float hls_value(float n1, float n2, float h)
{
	h += 360.0f;
    float hue = h - 360.0f*(int)(h/360.0f);

	if (hue < 60.0f)
		return n1 + ( n2 - n1 ) * hue / 60.0f;
	else if (hue < 180.0f)
		return n2;
	else if (hue < 240.0f)
		return n1 + ( n2 - n1 ) * ( 240.0f - hue ) / 60.0f;
	else
		return n1;
}

void HLS2RGB(float h, float l, float s, float& r, float& g, float& b)
{ // h from <0, 360)
	float m1, m2;
	m2 = l + (l <= 0.5f ? l*s : s - l*s);
	m1 = 2.0f * l - m2;
	if (s == 0.0f)
		r = g = b = l;
	else
	{
		r = hls_value(m1, m2, h+120.0f);
		g = hls_value(m1, m2, h);
		b = hls_value(m1, m2, h-120.0f);
	}
	if (r > 1.0f) r = 1.0f;
	if (g > 1.0f) g = 1.0f;
	if (b > 1.0f) b = 1.0f;
	if (r < 0.0f) r = 0.0f;
	if (g < 0.0f) g = 0.0f;
	if (b < 0.0f) b = 0.0f;
}

static bool RGB2HLS(float r, float g, float b, float& h, float& l, float& s)
{
	float bc, gc, rc, rgbmax, rgbmin;

	// Compute lightness.
	rgbmax = r>g ? (r>b ? r : b) : (g>b ? g : b);
	rgbmin = r<g ? (r<b ? r : b) : (g<b ? g : b);
	l = (rgbmax + rgbmin) * 0.5f;

	// Compute saturation.
	if (rgbmax == rgbmin)
		s = 0.0f;
	else if (l <= 0.5f)
		s = (rgbmax - rgbmin) / (rgbmax + rgbmin);
	else
		s = (rgbmax - rgbmin) / (2.0f - rgbmax - rgbmin);

	// Compute the hue.
	if (rgbmax == rgbmin)
	{
		h = 0.0f;
		return false;
	}
	else
	{
		rc = (rgbmax - r) / (rgbmax - rgbmin);
		gc = (rgbmax - g) / (rgbmax - rgbmin);
		bc = (rgbmax - b) / (rgbmax - rgbmin);

		if (r == rgbmax)
			h = bc - gc;
		else if (g == rgbmax)
			h = 2.0f + rc - bc;
		else
			h = 4.0f + gc - rc;

		h *= 60.0f;
		h += 360.0f;
		h = h - 360.0f*(int)(h/360.0f);
		return true;
	}
}

STDMETHODIMP CSharedStateColor::RGBASet(float const* a_pRGBA, ISharedStateColor* a_pOldState)
{
	try
	{
		float aPrevColor[4];
		if (a_pOldState)
			a_pOldState->RGBAGet(aPrevColor);
		m_fR = a_pRGBA[0];
		m_fG = a_pRGBA[1];
		m_fB = a_pRGBA[2];
		m_fA = a_pRGBA[3];
		if (!RGB2HLS(min(1.0f, max(0.0f, m_fR)), min(1.0f, max(0.0f, m_fG)), min(1.0f, max(0.0f, m_fB)), m_fH, m_fL, m_fS) && a_pOldState && m_fR == m_fG && m_fG == m_fB)
		{
			float f[4];
			if (SUCCEEDED(a_pOldState->HLSAGet(f)))
			{
				m_fH = f[0];
				//m_fS = f[2];
			}
		}
		return a_pOldState == NULL ||
				m_fR != aPrevColor[0] || m_fG != aPrevColor[1] ||
				m_fB != aPrevColor[2] || m_fA != aPrevColor[3] ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return a_pRGBA ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CSharedStateColor::HLSASet(float const* a_pHLSA, ISharedStateColor* a_pOldState)
{
	try
	{
		float aPrevColor[4];
		if (a_pOldState)
			a_pOldState->HLSAGet(aPrevColor);
		m_fH = a_pHLSA[0];
		m_fL = a_pHLSA[1];
		m_fS = a_pHLSA[2];
		m_fA = a_pHLSA[3];
		HLS2RGB(m_fH, m_fL, m_fS, m_fR, m_fG, m_fB);
		return a_pOldState == NULL ||
				m_fH != aPrevColor[0] || m_fL != aPrevColor[1] ||
				m_fS != aPrevColor[2] || m_fA != aPrevColor[3] ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return a_pHLSA ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CSharedStateColor::RGBHLSAGet(float* a_pRGBHLSA)
{
	try
	{
		a_pRGBHLSA[0] = m_fR;
		a_pRGBHLSA[1] = m_fG;
		a_pRGBHLSA[2] = m_fB;
		a_pRGBHLSA[3] = m_fH;
		a_pRGBHLSA[4] = m_fL;
		a_pRGBHLSA[5] = m_fS;
		a_pRGBHLSA[6] = m_bAlphaEnabled ? m_fA : 1.0f;
		return S_OK;
	}
	catch (...)
	{
		return a_pRGBHLSA ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CSharedStateColor::RGBHLSASet(float const* a_pRGBHLSA, ISharedStateColor* a_pOldState)
{
	try
	{
		float aPrevColor[4];
		if (a_pOldState)
			a_pOldState->RGBAGet(aPrevColor);
		m_fR = a_pRGBHLSA[0];
		m_fG = a_pRGBHLSA[1];
		m_fB = a_pRGBHLSA[2];
		m_fH = a_pRGBHLSA[3];
		m_fL = a_pRGBHLSA[4];
		m_fS = a_pRGBHLSA[5];
		m_fA = a_pRGBHLSA[6];
		return a_pOldState == NULL ||
				m_fR != aPrevColor[0] || m_fG != aPrevColor[1] ||
				m_fB != aPrevColor[2] || m_fA != aPrevColor[3] ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return a_pRGBHLSA ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CSharedStateColor::AlphaEnabled()
{
	return m_bAlphaEnabled ? S_OK : S_FALSE;
}

STDMETHODIMP CSharedStateColor::AlphaSet(BYTE a_bEnable)
{
	m_bAlphaEnabled = a_bEnable;
	return S_OK;
}

