// MouseGesturesHelper.cpp : Implementation of CMouseGesturesHelper

#include "stdafx.h"
#include "MouseGesturesHelper.h"

#include <MultiLanguageString.h>


static const OLECHAR CFGID_2DEDIT_GESTUREPREFIX[] = L"Gesture";

struct SGestureSpecification
{
	OLECHAR szCode[10];
	wchar_t const* pszName;
	POINT aPoints[32]; // 0...256; -1 = invalid point
};

extern __declspec(selectany) SGestureSpecification const g_aGestureSpecifications[] =
{
	{ L"R", L"[0409]Right[0405]Vpravo", {{16, 128}, {240, 128}, {-1, -1}} },
	{ L"L", L"[0409]Left[0405]Vlevo", {{240, 128}, {16, 128}, {-1, -1}} },
	{ L"RL", L"[0409]Right-Left[0405]Vpravo-vlevo", {{16, 80}, {240, 80}, {240, 176}, {32, 176}, {-1, -1}} },
	{ L"LR", L"[0409]Left-Right[0405]Vlevo-vpravo", {{240, 80}, {16, 80}, {16, 176}, {224, 176}, {-1, -1}} },
	{ L"D", L"[0409]Down[0405]Dolů", {{128, 16}, {128, 240}, {-1, -1}} },
	{ L"U", L"[0409]Up[0405]Nahoru", {{128, 240}, {128, 16}, {-1, -1}} },
	{ L"DU", L"[0409]Down-Up[0405]Dolů-nahoru", {{80, 16}, {80, 240}, {176, 240}, {176, 32}, {-1, -1}} },
	{ L"UD", L"[0409]Up-Down[0405]Nahoru-dolů", {{80, 240}, {80, 16}, {176, 16}, {176, 224}, {-1, -1}} },
	{ L"DR", L"[0409]Down-Right[0405]Dolů-vpravo", {{16, 16}, {16, 240}, {240, 240}, {-1, -1}} },
	{ L"DL", L"[0409]Down-Left[0405]Dolů-vlevo", {{240, 16}, {240, 240}, {16, 240}, {-1, -1}} },
	{ L"UR", L"[0409]Up-Right[0405]Nahoru-vpravo", {{16, 240}, {16, 16}, {240, 16}, {-1, -1}} },
	{ L"UL", L"[0409]Up-Left[0405]Nahoru-vlevo", {{240, 240}, {240, 16}, {16, 16}, {-1, -1}} },
	{ L"AU", L"[0409]Arrow Up[0405]Šipka nahoru", {{16, 200}, {128, 56}, {240, 200}, {-1, -1}} },
	{ L"AD", L"[0409]Arrow Down[0405]Šipka dolů", {{16, 56}, {128, 200}, {240, 56}, {-1, -1}} },
	{ L"AL", L"[0409]Arrow Left[0405]Šipka vlevo", {{200, 16}, {56, 128}, {200, 240}, {-1, -1}} },
	{ L"AR", L"[0409]Arrow Right[0405]Šipka vpravo", {{56, 16}, {200, 128}, {56, 240}, {-1, -1}} },
	{ L"SQUARE", L"[0409]Rectangle[0405]Obdélník", {{16, 16}, {16, 240}, {240, 240}, {240, 16}, {96, 16}, {-1, -1}} },
	{ L"CIRCLE", L"[0409]Circle[0405]Kružnice", {
		{128, 16}, {157, 20}, {184, 31}, {207, 49}, {225, 72}, {236, 99},
		{240, 128}, {236, 157}, {225, 184}, {207, 207}, {184, 225}, {157, 236},
		{128, 240}, {99, 236}, {72, 225}, {49, 207}, {31, 184}, {20, 157},
		{16, 128}, {20, 99}, {31, 72}, {49, 49}, {-1, -1}} },
	{ L"HOURGLASS", L"[0409]Hourglass[0405]Přesýpací hodiny", {{48, 16}, {208, 16}, {48, 240}, {208, 240}, {88, 72}, {-1, -1}} },
	{ L"A", L"[0409]A[0405]A", {{56, 218}, {128, 16}, {208, 240}, {72, 128}, {240, 128}, {-1, -1}} },
	{ L"Z", L"[0409]Z[0405]Z", {{64, 16}, {208, 16}, {48, 240}, {208, 240}, {-1, -1}} },
	{ L"M", L"[0409]M[0405]M", {{16, 240}, {64, 16}, {128, 144}, {192, 16}, {240, 240}, {-1, -1}} },
	{ L"W", L"[0409]W[0405]W", {{16, 16}, {64, 240}, {128, 112}, {192, 240}, {240, 16}, {-1, -1}} },
	{ L"N", L"[0409]N[0405]N", {{48, 224}, {48, 16}, {208, 240}, {208, 16}, {-1, -1}} },
	{ L"IN", L"[0409]Mirrored N[0405]Zrcadlové N", {{48, 32}, {48, 240}, {208, 16}, {208, 240}, {-1, -1}} },
	{ L"CROSS", L"[0409]Cross[0405]Kříž", {{128, 240}, {128, 16}, {240, 128}, {16, 128}, {-1, -1}} },
};

// CMouseGesturesHelper

STDMETHODIMP CMouseGesturesHelper::InitConfig(IOperationManager* a_pOpMgr, IConfigWithDependencies* a_pMainCfg)
{
	try
	{
		// extra mouse gestures -> document operation
		OLECHAR szID[64];
		wcscpy(szID, CFGID_2DEDIT_GESTUREPREFIX);
		LPWSTR pszID = szID+wcslen(szID);

		CComPtr<ILocalizedString> pGestName;
		pGestName.Attach(new CMultiLanguageString(L"[0409]Mouse gesture[0405]Gesto myši"));
		CComPtr<ILocalizedString> pGestDesc;
		pGestDesc.Attach(new CMultiLanguageString(L"[0409]Operation to execute when a mouse gesture is completed. Read more about mouse gestures in application help.[0405]Operace, která bude spuštěna po dokončení gesta myší. Více o gestech naleznete v nápovědě k aplikaci."));

		for (SGestureSpecification const* p = g_aGestureSpecifications; p != g_aGestureSpecifications+itemsof(g_aGestureSpecifications); ++p)
		{
			wcscpy(pszID, p->szCode);
			a_pOpMgr->InsertIntoConfigAs(a_pOpMgr, a_pMainCfg, CComBSTR(szID), pGestName, pGestDesc, 0, NULL);
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "MouseGesturesDlg.h"

STDMETHODIMP CMouseGesturesHelper::Configure(RWHWND a_hParent, LCID a_tLCID, IConfig* a_pMainCfg)
{
	try
	{
		CComPtr<IConfig> pConfig;
		a_pMainCfg->DuplicateCreate(&pConfig);
		CMouseGesturesDlg cDlg(a_tLCID, pConfig);
		if (IDOK == cDlg.DoModal(a_hParent))
		{
			CopyConfigValues(a_pMainCfg, pConfig);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "GestureData.h"
#include <numeric>
#include <algorithm>

float sigmoid(float f)	{return float(1. / (1. + exp(-f)));}
typedef std::vector<POINT> CGesturePoints;

STDMETHODIMP CMouseGesturesHelper::Recognize(ULONG a_nPoints, POINT const* a_pPoints, IConfig* a_pMainCfg, TConfigValue* a_pOpID, IConfig** a_ppOpCfg)
{
	try
	{
		ConfigValueClear(*a_pOpID);
		*a_ppOpCfg = NULL;

		if (a_nPoints < 2)
			return E_RW_INVALIDPARAM;

		if (m_pNet == NULL) // not thread safe, unlikely to be a problem
		{
			m_pNet.Attach(new MLNet(3, NET_INPUT_SIZE, NET_INPUT_SIZE, NET_OUTPUT_SIZE));
			m_pNet->set_transfer_function(sigmoid);
			m_pNet->set_bias(0.5);
			m_pNet->set_minmax(0.0, 1.0);
			HRSRC hRsrc = FindResource(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDR_DEFAULTGESTURES), _T("GESTURES"));
			HGLOBAL hMem = LoadResource(_pModule->get_m_hInst(), hRsrc);
			MLNet::array_t::value_type* pSrc = reinterpret_cast<MLNet::array_t::value_type*>(LockResource(hMem));
			for (MLNet::array_t::iterator i = m_pNet->begin_weight(); i != m_pNet->end_weight(); ++i, ++pSrc)
				*i = *pSrc;
		}

		CGesturePoints c16Pts;
		c16Pts.assign(a_pPoints, a_pPoints+a_nPoints);

		if ((RANGE_SIZE+1) > a_nPoints)
		{
			while (c16Pts.size() < (RANGE_SIZE+1))
			{
				CGesturePoints::iterator p_max  = c16Pts.begin();
				//++p_max;

				CGesturePoints::iterator p		= p_max;
				CGesturePoints::iterator i		= p_max;
				++i;

				CGesturePoints::iterator last	= c16Pts.end();
				if (i != last)
				{
					//--last;

					double d_max = -1.0;

					for (; i != last; ++i)
					{
						double d = sqrt(pow((*p).x - (*i).x, 2.0) + pow((*p).y - (*i).y, 2.0));
						if (d > d_max)
						{
							d_max = d;
							p_max = p;
						}
						p = i;
					}
				}

				p = p_max;
				i = ++p_max;

				POINT pt = {((*p).x + (*i).x) / 2, ((*p).y + (*i).y) / 2};
				c16Pts.insert(i, pt);

			}
		}	
		else
		{
			// smooth path		
			// finds smallest interval and replaces two points on median point

			while (c16Pts.size() > (RANGE_SIZE+1))
			{
				double d;
				double d_min = 1e20;
				
				CGesturePoints::iterator p_min  = c16Pts.begin();
				++p_min;

				CGesturePoints::iterator p		= p_min;
				CGesturePoints::iterator i		= p_min;
				++i;

				CGesturePoints::iterator last	= c16Pts.end();
				--last;

				for (; i != last; ++i)
				{
					d = sqrt(pow((*p).x - (*i).x, 2.0) + pow((*p).y - (*i).y, 2.0));
					if (d < d_min)
					{
						d_min = d;
						p_min = p;
					}
					p = i;
				}

				p = p_min;
				i = ++p_min;

				POINT pt = {((*p).x + (*i).x) / 2, ((*p).y + (*i).y) / 2};
				*i = pt;				// changes coord of a base point
				c16Pts.erase(p);		// erases an odd point 
			}
		}		
		
		// computes angles, cosines and sines

		CGesturePoints::iterator i = c16Pts.begin();
		CGesturePoints::iterator p = i++;
		unsigned n = 0;

		float cosines[RANGE_SIZE];
		float sinuses[RANGE_SIZE];
		float angles[RANGE_SIZE];

		for (; i != c16Pts.end(); ++i, ++n)
		{
			POINT pt2 = (*i);
			POINT pt1 = (*p);

			pt2.x -= pt1.x;
			pt2.y -= pt1.y;

			if (pt2.x || pt2.y)
			{
				cosines[n] = pt2.y / sqrtf(pt2.x * pt2.x + pt2.y * pt2.y);
				sinuses[n] = sqrtf(1.0f - cosines[n] * cosines[n]);		
				if (pt2.x < 0) sinuses[n] = - sinuses[n];		
				angles[n] = acosf(cosines[n]) * 180.0f / 3.141528f;
				if (pt2.x < 0) angles[n] = 360.0f - angles[n];
			}
			else
			{
				cosines[n] = 1;
				sinuses[n] = 0;
				angles[n]  = 0;
			}
			
			p = i;

		}	

		MLNet::array_t v_in (NET_INPUT_SIZE);
		MLNet::array_t v_out(NET_OUTPUT_SIZE);

		std::copy(cosines, cosines+itemsof(cosines), v_in.begin());
		std::copy(sinuses, sinuses+itemsof(sinuses), v_in.begin() + RANGE_SIZE);

		m_pNet->propagate(v_in, v_out);

		// apply softmax to a net output vector and find winner
		double sum = std::accumulate(v_out.begin(), v_out.end(), 0.);

		struct winner
		{
			unsigned m_id;				// index of pattern in pattern_data array
			float    m_probability;
		};
		winner winners[3];

		for (unsigned n = 0; n < sizeof(winners)/sizeof(winners[0]); ++n)
		{
			MLNet::array_t::iterator i = std::max_element(v_out.begin(), v_out.end());
			winners[n].m_id = i - v_out.begin();
			winners[n].m_probability = (float)(double(*i) / sum);
			*i = 0;
		}	

		// verify winner 

		#define MIN_PROBABILITY 0.25
		#define MIN_DIFFERENCE	0.25

		if (winners[0].m_probability <= MIN_PROBABILITY || (winners[0].m_probability - winners[1].m_probability) <= MIN_DIFFERENCE)
			return E_RW_ITEMNOTFOUND;

		OLECHAR szID[64];
		wcscpy(szID, CFGID_2DEDIT_GESTUREPREFIX);
		wcscat(szID, g_aGestureSpecifications[winners[0].m_id].szCode);
		CComBSTR bstrID(szID);
		a_pMainCfg->ItemValueGet(bstrID, a_pOpID);
		a_pMainCfg->SubConfigGet(bstrID, a_ppOpCfg);

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

