
#pragma once

#include <math.h>

struct TDrawCoord
{
	LONG nX, nY;
};

struct ZoomSingleItem
{
	ZoomSingleItem(size_t a_nSrc, size_t a_nDst) : m_nDst(a_nDst)
	{
		ATLASSERT(a_nSrc == 1);
		ATLASSERT(a_nDst > 0);
	}

	template<class TSrc, class TDst>
	inline void Execute(TSrc a_aSrc, TDst& a_aDst) const
	{
		TSrc::CachedItem pS(a_aSrc);
		++a_aSrc;
		for (size_t i = 0; i < m_nDst; ++i)
		{
			a_aDst.Blend(pS, 256, pS, 0);
			++a_aDst;
		}
	}

private:
	size_t m_nDst;
};

struct CopyWithoutZoom
{
	CopyWithoutZoom(size_t a_nSrc, size_t a_nDst) : m_nItems(a_nDst)
	{
		ATLASSERT(a_nSrc == a_nDst);
		ATLASSERT(a_nDst > 0);
	}

	template<class TSrc, class TDst>
	inline void Execute(TSrc a_aSrc, TDst& a_aDst) const
	{
		TSrc::CachedItem pS;
		for (size_t i = 0; i < m_nItems; ++i)
		{
			pS = a_aSrc;
			++a_aSrc;
			a_aDst.Blend(pS, 256, pS, 0);
			++a_aDst;

		}
	}

private:
	size_t m_nItems;
};

template<typename TCtrl>
struct LinearZoomOut
{
	LinearZoomOut(size_t a_nSrc, size_t a_nDst) : m_aCtrl(NULL), m_pRefCount(new size_t)
	{
		ATLASSERT(a_nSrc > a_nDst);
		ATLASSERT(a_nDst > 0);

		*m_pRefCount = 1;
		m_aCtrl = new TCtrl[a_nDst<<2];

		TCtrl* a_aCtrl = m_aCtrl;
		if (a_nDst == 0)
			return;
		size_t tPrevPos = 0;
		for (size_t i = 0; i < a_nDst; ++i)
		{
			double x1 = i*a_nSrc/double(a_nDst);
			double x2 = (i+1)*a_nSrc/double(a_nDst);
			if (x2 >= a_nSrc)
				x2 = a_nSrc-0.0001;
			size_t int_x1 = x1;
			size_t int_x2 = x2;
			if ((int_x2-tPrevPos) > 1)
			{
				a_aCtrl[0] = int_x2-tPrevPos-1; // count
				a_aCtrl[1] = (unsigned)((int_x1+1-x1)*256+0.5); // first w
				a_aCtrl[2] = (unsigned)((x2-int_x2)*256+0.5); // last w
				a_aCtrl[3] = a_aCtrl[1]+a_aCtrl[2]+256*a_aCtrl[0]; // total w
				a_aCtrl += 4;
				tPrevPos = int_x2;
			}
			else
			{
				*a_aCtrl = 0;
				++a_aCtrl;
				*a_aCtrl = (unsigned)((int_x2-x1)/(x2-x1)*256+0.5);
				++a_aCtrl;
				++tPrevPos;
			}
		}
		m_aCtrlEnd = a_aCtrl;
	}
	LinearZoomOut(LinearZoomOut const& a_rhs) : m_aCtrl(a_rhs.m_aCtrl), m_aCtrlEnd(a_rhs.m_aCtrlEnd), m_pRefCount(a_rhs.m_pRefCount)
	{
		++*m_pRefCount;
	}
	LinearZoomOut& operator=(LinearZoomOut const& a_rhs)
	{
		m_aCtrl = a_rhs.m_aCtrl;
		m_aCtrlEnd = a_rhs.m_aCtrlEnd;
		m_pRefCount = a_rhs.m_pRefCount;
		++*m_pRefCount;
		return *this;
	}
	~LinearZoomOut()
	{
		if (*m_pRefCount == 1)
		{
			delete[] m_aCtrl;
			delete m_pRefCount;
		}
		else
		{
			--*m_pRefCount;
		}
	}

	template<class TSrc, class TDst>
	inline void Execute(TSrc a_aSrc, TDst& a_aDst) const
	{
		TCtrl* aCtrl = m_aCtrl;
		TSrc::CachedItem pS1(a_aSrc);
		++a_aSrc;
		while (aCtrl != m_aCtrlEnd)
		{
			TSrc::CachedItem pS2;
			if (aCtrl[0] == 0)
			{
				pS2 = a_aSrc;
				++a_aSrc;
				a_aDst.Blend(pS1, aCtrl[1], pS2, 256-aCtrl[1]);
				++a_aDst;
				aCtrl += 2;
			}
			else
			{
				CAutoVectorPtr<TSrc::CachedItem> aFullyCovered(new TSrc::CachedItem[aCtrl[0]]);
				for (size_t i = 0; i < aCtrl[0]; ++i)
				{
					aFullyCovered[i] = a_aSrc;
					++a_aSrc;
				}
				pS2 = a_aSrc;
				++a_aSrc;
				a_aDst.Blend(pS1, aCtrl[1], pS2, aCtrl[2], aFullyCovered.m_p, aCtrl[0], aCtrl[3]);
				// TODO:
				aCtrl += 4; // count, first w, last w, total w
				++a_aDst;
			}

			if (aCtrl == m_aCtrlEnd)
				break;

			if (aCtrl[0] == 0)
			{
				pS1 = a_aSrc;
				++a_aSrc;
				a_aDst.Blend(pS2, aCtrl[1], pS1, 256-aCtrl[1]);
				++a_aDst;
				aCtrl += 2;
			}
			else
			{
				CAutoVectorPtr<TSrc::CachedItem> aFullyCovered(new TSrc::CachedItem[aCtrl[0]]);
				for (size_t i = 0; i < aCtrl[0]; ++i)
				{
					aFullyCovered[i] = a_aSrc;
					++a_aSrc;
				}
				pS1 = a_aSrc;
				++a_aSrc;
				a_aDst.Blend(pS2, aCtrl[1], pS1, aCtrl[2], aFullyCovered.m_p, aCtrl[0], aCtrl[3]);
				aCtrl += 4; // count, first w, last w, total w
				++a_aDst;
			}
		}
	}

private:
	TCtrl* m_aCtrl;
	TCtrl* m_aCtrlEnd;
	size_t* m_pRefCount;
};

template<typename TCtrl>
struct LinearZoomIn
{
	LinearZoomIn(size_t a_nSrc, size_t a_nDst) : m_aCtrl(NULL), m_pRefCount(new size_t)
	{
		ATLASSERT(a_nSrc < a_nDst);
		ATLASSERT(a_nSrc > 0);

		*m_pRefCount = 1;
		m_aCtrl = new TCtrl[a_nSrc+a_nDst+1];

		TCtrl* a_aCtrl = m_aCtrl;
		TCtrl* tPrevCount = a_aCtrl;
		*tPrevCount = 0;
		if (a_nDst == 0)
			return;
		++a_aCtrl;
		size_t tPrevPos = 0;
		for (size_t i = 0; i < a_nDst; ++i)
		{
			double x = (i+0.5)*a_nSrc/double(a_nDst)-0.5;
			if (x < 0.0)
				x = 0.0;
			else if (x >= (a_nSrc-1))
				x = a_nSrc-1.0001;
			size_t int_x = x;
			if (tPrevPos == int_x)
			{
				++*tPrevCount;
			}
			else
			{
				tPrevCount = a_aCtrl;
				*tPrevCount = 1;
				++a_aCtrl;
				tPrevPos = int_x;
			}
			*a_aCtrl = (unsigned)((x-tPrevPos)*256+0.5);
			++a_aCtrl;
		}
		*a_aCtrl = 0;
	}
	LinearZoomIn(LinearZoomIn const& a_rhs) : m_aCtrl(a_rhs.m_aCtrl), m_pRefCount(a_rhs.m_pRefCount)
	{
		++*m_pRefCount;
	}
	LinearZoomIn& operator=(LinearZoomIn const& a_rhs)
	{
		m_aCtrl = a_rhs.m_aCtrl;
		m_pRefCount = a_rhs.m_pRefCount;
		++*m_pRefCount;
		return *this;
	}
	~LinearZoomIn()
	{
		if (*m_pRefCount == 1)
		{
			delete[] m_aCtrl;
			delete m_pRefCount;
		}
		else
		{
			--*m_pRefCount;
		}
	}

	template<class TSrc, class TDst>
	inline void Execute(TSrc a_aSrc, TDst& a_aDst) const
	{
		TCtrl* a_aCtrl = m_aCtrl;
		TSrc::CachedItem pS1(a_aSrc);
		++a_aSrc;
		while (*a_aCtrl)
		{
			TCtrl const tCount1 = *a_aCtrl;
			++a_aCtrl;
			typename TSrc::CachedItem const pS2 = a_aSrc;
			++a_aSrc;
			for (int i = tCount1; i > 0; --i)
			{
				a_aDst.Blend(pS2, *a_aCtrl, pS1, 256-*a_aCtrl);
				++a_aDst;
				++a_aCtrl;
			}
			if (!*a_aCtrl)
				break;
			TCtrl const tCount2 = *a_aCtrl;
			++a_aCtrl;
			pS1 = a_aSrc;
			++a_aSrc;
			for (int i = tCount2; i > 0; --i)
			{
				a_aDst.Blend(pS1, *a_aCtrl, pS2, 256-*a_aCtrl);
				++a_aDst;
				++a_aCtrl;
			}
		}
	}

private:
	TCtrl* m_aCtrl;
	size_t* m_pRefCount;
};

template<typename TCtrl>
struct CubicZoomIn
{
	CubicZoomIn(size_t a_nSrc, size_t a_nDst) : m_aCtrl(NULL), m_pRefCount(new size_t)
	{
		ATLASSERT(a_nSrc < a_nDst);
		ATLASSERT(a_nSrc > 1);

		*m_pRefCount = 1;
		m_aCtrl = new TCtrl[a_nSrc+a_nDst*4+3];

		TCtrl* a_aCtrl = m_aCtrl;
		if (a_nSrc == 2)
		{
			*(a_aCtrl++) = 0; // last block marker
		}
		TCtrl* tPrevCount = a_aCtrl;
		*tPrevCount = 0;
		if (a_nDst == 0)
			return;
		++a_aCtrl;
		size_t tPrevPos = 0;
		for (size_t i = 0; i < a_nDst; ++i)
		{
			double xo = (i+0.5)*a_nSrc/double(a_nDst)-0.5;
			double x = (xo < 0.0) ? 0.0 : ((xo >= (a_nSrc-1)) ? a_nSrc-1.0001 : xo);
			size_t int_x = x;
			if (tPrevPos == int_x)
			{
				++*tPrevCount;
			}
			else
			{
				if (int_x == a_nSrc-2)
				{
					*(a_aCtrl++) = 0; // last block marker
				}
				tPrevCount = a_aCtrl;
				*tPrevCount = 1;
				++a_aCtrl;
				tPrevPos = int_x;
			}
			if (xo < 0.0)
			{
				a_aCtrl[0] = a_aCtrl[3] = 0;
				a_aCtrl[2] = (unsigned)(((xo+0.5)*0.333333333333333)*256+0.499);
				a_aCtrl[1] = 256-a_aCtrl[2];
			}
			else if (xo >= (a_nSrc-1))
			{
				a_aCtrl[0] = a_aCtrl[3] = 0;
				a_aCtrl[1] = (unsigned)(((a_nSrc-0.5-xo)*0.333333333333333)*256+0.499);
				a_aCtrl[2] = 256-a_aCtrl[1];
			}
			else
			{
				double const fr2 = x-tPrevPos;
				double const fr1 = 1.0f-fr2;
				double f1 = (fr1*fr1*fr1*0.166666666666666)*256;
				a_aCtrl[0] = (unsigned)(f1);
				double f2 = (fr2*fr2*(0.5*fr2-1)+0.666666666666666)*256;
				a_aCtrl[1] = (unsigned)(f2);
				double f3 = (fr1*fr1*(0.5*fr1-1)+0.666666666666666)*256;
				a_aCtrl[2] = (unsigned)(f3);
				double f4 = (fr2*fr2*fr2*0.166666666666666)*256;
				a_aCtrl[3] = (unsigned)(f4);
				while (a_aCtrl[0]+a_aCtrl[1]+a_aCtrl[2]+a_aCtrl[3] < 256)
				{
					if (f1-a_aCtrl[0] > f2-a_aCtrl[1])
					{
						if (f3-a_aCtrl[2] > f4-a_aCtrl[3])
						{
							if (f3-a_aCtrl[2] > f1-a_aCtrl[0])
								f3 = ++a_aCtrl[2];
							else
								f1 = ++a_aCtrl[0];
						}
						else
						{
							if (f4-a_aCtrl[3] > f1-a_aCtrl[0])
								f4 = ++a_aCtrl[3];
							else
								f1 = ++a_aCtrl[0];
						}
					}
					else
					{
						if (f3-a_aCtrl[2] > f4-a_aCtrl[3])
						{
							if (f3-a_aCtrl[2] > f2-a_aCtrl[1])
								f3 = ++a_aCtrl[2];
							else
								f2 = ++a_aCtrl[1];
						}
						else
						{
							if (f4-a_aCtrl[3] > f2-a_aCtrl[1])
								f4 = ++a_aCtrl[3];
							else
								f2 = ++a_aCtrl[1];
						}
					}
				}
			}
			ATLASSERT(unsigned(a_aCtrl[0]) <= 256);
			ATLASSERT(unsigned(a_aCtrl[1]) <= 256);
			ATLASSERT(unsigned(a_aCtrl[2]) <= 256);
			ATLASSERT(unsigned(a_aCtrl[3]) <= 256);
			a_aCtrl += 4;
		}
		a_aCtrl[0] = a_aCtrl[1] = 0;
	}
	CubicZoomIn(CubicZoomIn const& a_rhs) : m_aCtrl(a_rhs.m_aCtrl), m_pRefCount(a_rhs.m_pRefCount)
	{
		++*m_pRefCount;
	}
	CubicZoomIn& operator=(CubicZoomIn const& a_rhs)
	{
		m_aCtrl = a_rhs.m_aCtrl;
		m_pRefCount = a_rhs.m_pRefCount;
		++*m_pRefCount;
		return *this;
	}
	~CubicZoomIn()
	{
		if (*m_pRefCount == 1)
		{
			delete[] m_aCtrl;
			delete m_pRefCount;
		}
		else
		{
			--*m_pRefCount;
		}
	}

	template<class TSrc, class TDst>
	inline void Execute(TSrc a_aSrc, TDst& a_aDst) const
	{
		TCtrl* a_aCtrl = m_aCtrl;
		TSrc::CachedItem pS1(a_aSrc);
		TSrc::CachedItem pS2(pS1);
		++a_aSrc;
		TSrc::CachedItem pS3(a_aSrc);
		++a_aSrc;
		TSrc::CachedItem pS4;
		while (true)
		{
			if (!*a_aCtrl)
			{
				++a_aCtrl;
				if (!*a_aCtrl)
					break;
				pS4 = pS3;
			}
			else
			{
				pS4 = a_aSrc;
				++a_aSrc;
			}
			TCtrl const tCount1 = *a_aCtrl;
			++a_aCtrl;
			for (int i = tCount1; i > 0; --i)
			{
				a_aDst.Blend(pS1, a_aCtrl[0], pS2, a_aCtrl[1], pS3, a_aCtrl[2], pS4, a_aCtrl[3]);
				++a_aDst;
				a_aCtrl += 4;
			}

			if (!*a_aCtrl)
			{
				++a_aCtrl;
				if (!*a_aCtrl)
					break;
				pS1 = pS4;
			}
			else
			{
				pS1 = a_aSrc;
				++a_aSrc;
			}
			TCtrl const tCount2 = *a_aCtrl;
			++a_aCtrl;
			for (int i = tCount2; i > 0; --i)
			{
				a_aDst.Blend(pS2, a_aCtrl[0], pS3, a_aCtrl[1], pS4, a_aCtrl[2], pS1, a_aCtrl[3]);
				++a_aDst;
				a_aCtrl += 4;
			}

			if (!*a_aCtrl)
			{
				++a_aCtrl;
				if (!*a_aCtrl)
					break;
				pS2 = pS1;
			}
			else
			{
				pS2 = a_aSrc;
				++a_aSrc;
			}
			TCtrl const tCount3 = *a_aCtrl;
			++a_aCtrl;
			for (int i = tCount3; i > 0; --i)
			{
				a_aDst.Blend(pS3, a_aCtrl[0], pS4, a_aCtrl[1], pS1, a_aCtrl[2], pS2, a_aCtrl[3]);
				++a_aDst;
				a_aCtrl += 4;
			}

			if (!*a_aCtrl)
			{
				++a_aCtrl;
				if (!*a_aCtrl)
					break;
				pS3 = pS2;
			}
			else
			{
				pS3 = a_aSrc;
				++a_aSrc;
			}
			TCtrl const tCount4 = *a_aCtrl;
			++a_aCtrl;
			for (int i = tCount4; i > 0; --i)
			{
				a_aDst.Blend(pS4, a_aCtrl[0], pS1, a_aCtrl[1], pS2, a_aCtrl[2], pS3, a_aCtrl[3]);
				++a_aDst;
				a_aCtrl += 4;
			}
		}
	}

private:
	TCtrl* m_aCtrl;
	size_t* m_pRefCount;
};

struct CPremultiply
{
	CPremultiply(TRasterImagePixel const* a_p) : m_p(a_p) {}
	CPremultiply() {}
	void Init(TRasterImagePixel const* a_p) { m_p = a_p; }

	typedef unsigned TComponent;
	struct CachedItem
	{
		CachedItem() {}
		CachedItem(CPremultiply const a_t) :
			r(TComponent(a_t.m_p->bR)*a_t.m_p->bA),
			g(TComponent(a_t.m_p->bG)*a_t.m_p->bA),
			b(TComponent(a_t.m_p->bB)*a_t.m_p->bA),
			a(a_t.m_p->bA)
		{
		}
		void operator =(CPremultiply const a_t)
		{
			r = TComponent(a_t.m_p->bR)*a_t.m_p->bA;
			g = TComponent(a_t.m_p->bG)*a_t.m_p->bA;
			b = TComponent(a_t.m_p->bB)*a_t.m_p->bA;
			a = a_t.m_p->bA;
		}
		TComponent R() const { return r; }
		TComponent G() const { return g; }
		TComponent B() const { return b; }
		TComponent A() const { return a; }
		void R(TComponent a_r) { r = a_r; }
		void G(TComponent a_g) { g = a_g; }
		void B(TComponent a_b) { b = a_b; }
		void A(TComponent a_a) { a = a_a; }

	private:
		TComponent r, g, b, a;
	};

	void operator++()
	{
		++m_p;
	}
	void operator++(int)
	{
		++m_p;
	}

private:
	TRasterImagePixel const* m_p;
};

template<class TRow, typename TInternalZoom>
struct CPixelRow
{
	CPixelRow(TRow* a_pRows, TInternalZoom& a_tInternalZoom, size_t a_nRowLen) :
		m_pRows(a_pRows), m_tInternalZoom(a_tInternalZoom), m_nRowLen(a_nRowLen) {}

	typedef unsigned __int64 TComponent;
	struct CachedItem
	{
		typedef typename TRow::CachedItem item;
		typedef typename TRow::CachedItem* iterator;
		iterator begin() const { return m_p; }
		iterator end() const { return m_p+m_nRowLen; }

		CachedItem() : m_p(NULL), m_nRowLen(0) {}
		CachedItem(CPixelRow const& a_t) : m_p(NULL)
		{
			m_p = new TRow::CachedItem[a_t.m_nRowLen];
			m_nRowLen = a_t.m_nRowLen;
			operator=(a_t);
		}
		CachedItem(CachedItem const& a_rhs)
		{
			m_p = new TRow::CachedItem[a_rhs.m_nRowLen];
			m_nRowLen = a_rhs.m_nRowLen;
			operator=(a_rhs);
		}
		~CachedItem()
		{
			delete[] m_p;
		}
		void operator =(CPixelRow const& a_t)
		{
			if (m_p == NULL)
			{
				m_p = new TRow::CachedItem[a_t.m_nRowLen];
				m_nRowLen = a_t.m_nRowLen;
			}
			m_pDst = m_p;
			a_t.m_tInternalZoom.Execute(*a_t.m_pRows, *this);
		}
		void operator =(CachedItem const& a_t)
		{
			if (this == &a_t)
				return;
			if (m_p == NULL)
			{
				m_p = new TRow::CachedItem[a_t.m_nRowLen];
				m_nRowLen = a_t.m_nRowLen;
			}
			CopyMemory(m_p, a_t.m_p, sizeof(TRow::CachedItem)*m_nRowLen);
			m_pDst = m_p + (a_t.m_pDst-a_t.m_p);
		}
		template<typename TSrc, typename TWeight>
		inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw)
		{
			m_pDst->R(a_lhs.R()*a_lw+a_rhs.R()*a_rw);
			m_pDst->G(a_lhs.G()*a_lw+a_rhs.G()*a_rw);
			m_pDst->B(a_lhs.B()*a_lw+a_rhs.B()*a_rw);
			m_pDst->A(a_lhs.A()*a_lw+a_rhs.A()*a_rw);
		}
		template<typename TSrc, typename TWeight>
		inline void Blend(TSrc const& a_1, TWeight const a_w1, TSrc const& a_2, TWeight const a_w2, TSrc const& a_3, TWeight const a_w3, TSrc const& a_4, TWeight const a_w4)
		{
			m_pDst->R(a_1.R()*a_w1+a_2.R()*a_w2+a_3.R()*a_w3+a_4.R()*a_w4);
			m_pDst->G(a_1.G()*a_w1+a_2.G()*a_w2+a_3.G()*a_w3+a_4.G()*a_w4);
			m_pDst->B(a_1.B()*a_w1+a_2.B()*a_w2+a_3.B()*a_w3+a_4.B()*a_w4);
			m_pDst->A(a_1.A()*a_w1+a_2.A()*a_w2+a_3.A()*a_w3+a_4.A()*a_w4);
		}
		template<typename TSrc, typename TWeight>
		inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw, TSrc const* a_pFullyCovered, size_t a_nFullyCovered, TWeight a_tTotalWeight)
		{
			TComponent a = a_lhs.A()*a_lw + a_rhs.A()*a_rw;
			TComponent r = TComponent(a_lhs.R())*a_lw + TComponent(a_rhs.R())*a_rw;
			TComponent g = TComponent(a_lhs.G())*a_lw + TComponent(a_rhs.G())*a_rw;
			TComponent b = TComponent(a_lhs.B())*a_lw + TComponent(a_rhs.B())*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += TComponent(a_pFullyCovered->R())<<8;
				g += TComponent(a_pFullyCovered->G())<<8;
				b += TComponent(a_pFullyCovered->B())<<8;
				a += a_pFullyCovered->A()<<8;
			}
			if (a == 0)
			{
				m_pDst->R(0);
				m_pDst->G(0);
				m_pDst->B(0);
				m_pDst->A(0);
			}
			else
			{
				TWeight const tHalf = a_tTotalWeight>>1;
				m_pDst->R(((r<<8)+tHalf)/a_tTotalWeight);
				m_pDst->G(((g<<8)+tHalf)/a_tTotalWeight);
				m_pDst->B(((b<<8)+tHalf)/a_tTotalWeight);
				m_pDst->A(((a<<8)+tHalf)/a_tTotalWeight);
			}
		}
		void operator++()
		{
			++m_pDst;
		}
		void operator++(int)
		{
			++m_pDst;
		}

	private:
		typename TRow::CachedItem* m_p;
		typename TRow::CachedItem* m_pDst;
		size_t m_nRowLen;
	};


	void operator++()
	{
		++m_pRows;
	}
	void operator++(int)
	{
		++m_pRows;
	}

private:
	TRow* m_pRows;
	TInternalZoom m_tInternalZoom;
	size_t m_nRowLen;
};

template<typename TComponent, int a_tShift>
struct CDemultiplyingDst
{
	CDemultiplyingDst() {}
	CDemultiplyingDst(TRasterImagePixel* a_pDst) : m_pDst(a_pDst) {}
	void Init(TRasterImagePixel* a_pDst) { m_pDst = a_pDst; }

	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw)
	{
		TComponent a = a_lhs.A()*a_lw + a_rhs.A()*a_rw;
		TComponent tFinalA = a>>a_tShift;
		if (tFinalA == 0)
		{
			TRasterImagePixel static const t = {0, 0, 0, 0};
			*m_pDst = t;
		}
		else
		{
			TRasterImagePixel const t =
			{
				(a_lhs.B()*a_lw + a_rhs.B()*a_rw)/a,
				(a_lhs.G()*a_lw + a_rhs.G()*a_rw)/a,
				(a_lhs.R()*a_lw + a_rhs.R()*a_rw)/a,
				tFinalA
			};
			*m_pDst = t;
		}
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_1, TWeight const a_w1, TSrc const& a_2, TWeight const a_w2, TSrc const& a_3, TWeight const a_w3, TSrc const& a_4, TWeight const a_w4)
	{
		TComponent a = a_1.A()*a_w1+a_2.A()*a_w2+a_3.A()*a_w3+a_4.A()*a_w4;
		TComponent tFinalA = a>>a_tShift;
		if (tFinalA == 0)
		{
			TRasterImagePixel static const t = {0, 0, 0, 0};
			*m_pDst = t;
		}
		else
		{
			TRasterImagePixel const t =
			{
				(a_1.B()*a_w1+a_2.B()*a_w2+a_3.B()*a_w3+a_4.B()*a_w4)/a,
				(a_1.G()*a_w1+a_2.G()*a_w2+a_3.G()*a_w3+a_4.G()*a_w4)/a,
				(a_1.R()*a_w1+a_2.R()*a_w2+a_3.R()*a_w3+a_4.R()*a_w4)/a,
				tFinalA
			};
			*m_pDst = t;
		}
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw, TSrc const* a_pFullyCovered, size_t a_nFullyCovered, TWeight a_tTotalWeight)
	{
		if (a_tShift > 8)
		{
			TComponent r = (a_lhs.R()>>8)*a_lw + (a_rhs.R()>>8)*a_rw;
			TComponent g = (a_lhs.G()>>8)*a_lw + (a_rhs.G()>>8)*a_rw;
			TComponent b = (a_lhs.B()>>8)*a_lw + (a_rhs.B()>>8)*a_rw;
			TComponent a = (a_lhs.A()*a_lw + a_rhs.A()*a_rw)>>8;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R();
				g += a_pFullyCovered->G();
				b += a_pFullyCovered->B();
				a += a_pFullyCovered->A();
			}
			TComponent tFinalA = (a/a_tTotalWeight)>>(a_tShift-16);
			if (tFinalA == 0)
			{
				TRasterImagePixel static const t = {0, 0, 0, 0};
				*m_pDst = t;
			}
			else
			{
				TRasterImagePixel t =
				{
					b/a,
					g/a,
					r/a,
					tFinalA
				};
				*m_pDst = t;
			}
		}
		else
		{
			TComponent a = a_lhs.A()*a_lw + a_rhs.A()*a_rw;
			TComponent r = a_lhs.R()*a_lw + a_rhs.R()*a_rw;
			TComponent g = a_lhs.G()*a_lw + a_rhs.G()*a_rw;
			TComponent b = a_lhs.B()*a_lw + a_rhs.B()*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R()<<8;
				g += a_pFullyCovered->G()<<8;
				b += a_pFullyCovered->B()<<8;
				a += a_pFullyCovered->A()<<8;
			}
			TComponent tFinalA = (a/a_tTotalWeight)>>(a_tShift-8);
			if (tFinalA == 0)
			{
				TRasterImagePixel static const t = {0, 0, 0, 0};
				*m_pDst = t;
			}
			else
			{
				TRasterImagePixel t =
				{
					b/a,
					g/a,
					r/a,
					tFinalA
				};
				*m_pDst = t;
			}
		}
	}
	void operator ++()
	{
		++m_pDst;
	}
	void operator ++(int)
	{
		++m_pDst;
	}
private:
	TRasterImagePixel* m_pDst;
};

template<class TPixelDst>
struct CRowDst
{
	CRowDst(TPixelDst* a_pRow) : m_pRow(a_pRow)
	{
	}

	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw)
	{
		TSrc::iterator l_end = a_lhs.end();
		TSrc::iterator l = a_lhs.begin();
		TSrc::iterator r = a_rhs.begin();
		while (l != l_end)
		{
			m_pRow->Blend(*l, a_lw, *r, a_rw);
			++l;
			++r;
			++*m_pRow;
		}
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_1, TWeight const a_w1, TSrc const& a_2, TWeight const a_w2, TSrc const& a_3, TWeight const a_w3, TSrc const& a_4, TWeight const a_w4)
	{
		TSrc::iterator a1_end = a_1.end();
		TSrc::iterator a1 = a_1.begin();
		TSrc::iterator a2 = a_2.begin();
		TSrc::iterator a3 = a_3.begin();
		TSrc::iterator a4 = a_4.begin();
		while (a1 != a1_end)
		{
			m_pRow->Blend(*a1, a_w1, *a2, a_w2, *a3, a_w3, *a4, a_w4);
			++a1;
			++a2;
			++a3;
			++a4;
			++*m_pRow;
		}
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw, TSrc const* a_pFullyCovered, size_t a_nFullyCovered, TWeight a_tTotalWeight)
	{
		TSrc::iterator l_end = a_lhs.end();
		TSrc::iterator l = a_lhs.begin();
		TSrc::iterator r = a_rhs.begin();
		CAutoVectorPtr<TSrc::iterator> m(new TSrc::iterator[a_nFullyCovered]);
		for (size_t i = 0; i < a_nFullyCovered; ++i)
			m[i] = a_pFullyCovered[i].begin();
		CAutoVectorPtr<TSrc::item> aFC(new TSrc::item[a_nFullyCovered]);

		while (l != l_end)
		{
			for (size_t i = 0; i < a_nFullyCovered; ++i)
				aFC[i] = *(m[i]);
			m_pRow->Blend(*l, a_lw, *r, a_rw, aFC.m_p, a_nFullyCovered, a_tTotalWeight);
			++l;
			++r;
			for (size_t i = 0; i < a_nFullyCovered; ++i)
				++(m[i]);
			++*m_pRow;
		}
	}

	void operator ++()
	{
		++m_pRow;
	}
	void operator ++(int)
	{
		++m_pRow;
	}
private:
	TPixelDst* m_pRow;
};

template<typename TImagePixel, class TSource, template<typename,int> class TDestination>
class CResamplingT
{
	template<class TXZoom, class TYZoom, int t_nShift>
	void LinearZoomHelper() const
	{
		TXZoom aCtrlX(m_nSrcSizeX, m_nDstSizeX);
		TYZoom aCtrlY(m_nSrcSizeY, m_nDstSizeY);
		CAutoVectorPtr<TSource> aRowSrcs(new TSource[m_nSrcSizeY]);
		CAutoVectorPtr<TDestination<unsigned, t_nShift> > aRowDsts(new TDestination<unsigned, t_nShift>[m_nDstSizeY]);
		for (LONG y = 0; y < m_nDstSizeY; ++y)
		{
			aRowDsts[y].Init(reinterpret_cast<TImagePixel*>(reinterpret_cast<BYTE*>(m_pDst)+m_nDstStride*y));
		}
		for (LONG y = 0; y < m_nSrcSizeY; ++y)
		{
			aRowSrcs[y].Init(reinterpret_cast<TImagePixel const*>(reinterpret_cast<BYTE const*>(m_pSrc)+m_nSrcStride*y));
		}
		CPixelRow<TSource, TXZoom > const cRowSrc(aRowSrcs.m_p, aCtrlX, m_nDstSizeX);
		CRowDst<TDestination<unsigned, t_nShift> > cRowDst(aRowDsts.m_p);
		aCtrlY.Execute(cRowSrc, cRowDst);
	}

public:
	CResamplingT(LONG a_nDstSizeX, LONG a_nDstSizeY, LONG a_nSrcSizeX, LONG a_nSrcSizeY, TImagePixel* a_pDst, TImagePixel const* a_pSrc) :
		m_nDstSizeX(a_nDstSizeX), m_nDstSizeY(a_nDstSizeY), m_nDstStride(a_nDstSizeX*sizeof(TImagePixel)), m_nSrcSizeX(a_nSrcSizeX), m_nSrcSizeY(a_nSrcSizeY), m_nSrcStride(a_nSrcSizeX*sizeof(TImagePixel)), m_pDst(a_pDst), m_pSrc(a_pSrc)
	{
	}
	CResamplingT(LONG a_nDstSizeX, LONG a_nDstSizeY, LONG a_nSrcSizeX, LONG a_nSrcSizeY, TImagePixel* a_pDst, LONG a_nDstStride, TImagePixel const* a_pSrc, LONG a_nSrcStride) :
		m_nDstSizeX(a_nDstSizeX), m_nDstSizeY(a_nDstSizeY), m_nDstStride(a_nDstStride), m_nSrcSizeX(a_nSrcSizeX), m_nSrcSizeY(a_nSrcSizeY), m_nSrcStride(a_nSrcStride), m_pDst(a_pDst), m_pSrc(a_pSrc)
	{
	}

	void Nearest() const
	{
		TDrawCoord t;
		float fStepX = m_nSrcSizeX/static_cast<float>(m_nDstSizeX);
		float fStepY = m_nSrcSizeY/static_cast<float>(m_nDstSizeY);
		for (t.nY = 0; t.nY < m_nDstSizeY; t.nY++)
		{
			TImagePixel* pDstRow = reinterpret_cast<TImagePixel*>(reinterpret_cast<BYTE*>(m_pDst)+m_nDstStride*t.nY);
			TImagePixel const* pSrcRow = reinterpret_cast<TImagePixel const*>(reinterpret_cast<BYTE const*>(m_pSrc)+m_nSrcStride*static_cast<int>(t.nY*fStepY));
			for (t.nX = 0; t.nX < m_nDstSizeX; t.nX++)
			{
				pDstRow[t.nX] = pSrcRow[static_cast<int>(t.nX*fStepX)];
			}
		}
	}

	void Linear() const
	{
		if (m_nSrcSizeX == 1)
		{
			if (m_nSrcSizeY == 1)
			{
				Nearest();//LinearZoomHelper<TPixelMixer, ZoomSingleItem, ZoomSingleItem, 16>(a_pImage);
			}
			else
			{
				if (m_nDstSizeY == m_nSrcSizeY)
					Nearest();//LinearZoomHelper<TPixelMixer, ZoomSingleItem, CopyWithoutZoom, 16>(a_pImage);
				else if (m_nDstSizeY > m_nSrcSizeY)
					LinearZoomHelper<ZoomSingleItem, LinearZoomIn<unsigned>, 16>();
				else
					LinearZoomHelper<ZoomSingleItem, LinearZoomOut<unsigned>, 16>();
			}
		}
		else
		{
			if (m_nSrcSizeY == 1)
			{
				if (m_nDstSizeX == m_nSrcSizeX)
					Nearest();//LinearZoomHelper<TPixelMixer, CopyWithoutZoom, ZoomSingleItem, 16>(a_pImage);
				else if (m_nDstSizeX > m_nSrcSizeX)
					LinearZoomHelper<LinearZoomIn<unsigned>, ZoomSingleItem, 16>();
				else
					LinearZoomHelper<LinearZoomOut<unsigned>, ZoomSingleItem, 16>();
			}
			else
			{
				if (m_nDstSizeY == m_nSrcSizeY)
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						Nearest();//LinearZoomHelper<TPixelMixer, CopyWithoutZoom, CopyWithoutZoom, 16>(a_pImage);
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<LinearZoomIn<unsigned>, CopyWithoutZoom, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned>, CopyWithoutZoom, 16>();
				}
				else if (m_nDstSizeY > m_nSrcSizeY)
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						LinearZoomHelper<CopyWithoutZoom, LinearZoomIn<unsigned>, 16>();
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<LinearZoomIn<unsigned>, LinearZoomIn<unsigned>, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned>, LinearZoomIn<unsigned>, 16>();
				}
				else
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						LinearZoomHelper<CopyWithoutZoom, LinearZoomOut<unsigned>, 16>();
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<LinearZoomIn<unsigned>, LinearZoomOut<unsigned>, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned>, LinearZoomOut<unsigned>, 16>();
				}
			}
		}
	}

	void Cubic() const
	{
		if (m_nSrcSizeX == 1)
		{
			if (m_nSrcSizeY == 1)
			{
				Nearest();//LinearZoomHelper<TPixelMixer, ZoomSingleItem, ZoomSingleItem, 16>(a_pImage);
			}
			else
			{
				if (m_nDstSizeY == m_nSrcSizeY)
					Nearest();//LinearZoomHelper<TPixelMixer, ZoomSingleItem, CopyWithoutZoom, 16>(a_pImage);
				else if (m_nDstSizeY > m_nSrcSizeY)
					LinearZoomHelper<ZoomSingleItem, CubicZoomIn<unsigned>, 16>();
				else
					LinearZoomHelper<ZoomSingleItem, LinearZoomOut<unsigned>, 16>();
			}
		}
		else
		{
			if (m_nSrcSizeY == 1)
			{
				if (m_nDstSizeX == m_nSrcSizeX)
					Nearest();//LinearZoomHelper<TPixelMixer, CopyWithoutZoom, ZoomSingleItem, 16>(a_pImage);
				else if (m_nDstSizeX > m_nSrcSizeX)
					LinearZoomHelper<CubicZoomIn<unsigned>, ZoomSingleItem, 16>();
				else
					LinearZoomHelper<LinearZoomOut<unsigned>, ZoomSingleItem, 16>();
			}
			else
			{
				if (m_nDstSizeY == m_nSrcSizeY)
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						Nearest();//LinearZoomHelper<TPixelMixer, CopyWithoutZoom, CopyWithoutZoom, 16>(a_pImage);
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<CubicZoomIn<unsigned>, CopyWithoutZoom, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned>, CopyWithoutZoom, 16>();
				}
				else if (m_nDstSizeY > m_nSrcSizeY)
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						LinearZoomHelper<CopyWithoutZoom, CubicZoomIn<unsigned>, 16>();
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<CubicZoomIn<unsigned>, CubicZoomIn<unsigned>, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned>, CubicZoomIn<unsigned>, 16>();
				}
				else
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						LinearZoomHelper<CopyWithoutZoom, LinearZoomOut<unsigned>, 16>();
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<CubicZoomIn<unsigned>, LinearZoomOut<unsigned>, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned>, LinearZoomOut<unsigned>, 16>();
				}
			}
		}
	}

private:
	LONG const m_nDstSizeX;
	LONG const m_nDstSizeY;
	LONG const m_nDstStride;
	LONG const m_nSrcSizeX;
	LONG const m_nSrcSizeY;
	LONG const m_nSrcStride;
	TImagePixel* const m_pDst;
	TImagePixel const* const m_pSrc;
};

typedef CResamplingT<TRasterImagePixel, CPremultiply, CDemultiplyingDst> CResampling;

struct SResamplingRGB8 { BYTE r,g,b; };
struct SResamplingRGBX8 { BYTE r,g,b,x; };

template<typename TPixel>
struct CRGB8Src
{
	CRGB8Src(TPixel const* a_p) : m_p(a_p) {}
	CRGB8Src() {}
	void Init(TPixel const* a_p) { m_p = a_p; }

	typedef unsigned TComponent;
	struct CachedItem
	{
		CachedItem() {}
		CachedItem(CRGB8Src<TPixel> const a_t) :
			r(a_t.m_p->r),
			g(a_t.m_p->g),
			b(a_t.m_p->b)
		{
		}
		void operator =(CRGB8Src<TPixel> const a_t)
		{
			r = a_t.m_p->r;
			g = a_t.m_p->g;
			b = a_t.m_p->b;
		}
		TComponent R() const { return r; }
		TComponent G() const { return g; }
		TComponent B() const { return b; }
		TComponent A() const { return 255; }
		void R(TComponent a_r) { r = a_r; }
		void G(TComponent a_g) { g = a_g; }
		void B(TComponent a_b) { b = a_b; }
		void A(TComponent a_a) { }

	private:
		TComponent r, g, b;
	};

	void operator++()
	{
		++m_p;
	}
	void operator++(int)
	{
		++m_p;
	}

private:
	TPixel const* m_p;
};

template<typename TComponent, int a_tShift>
struct CRGB8Dst
{
	CRGB8Dst() {}
	CRGB8Dst(SResamplingRGB8* a_pDst) : m_pDst(a_pDst) {}
	void Init(SResamplingRGB8* a_pDst) { m_pDst = a_pDst; }

	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw)
	{
		m_pDst->r = (a_lhs.R()*a_lw + a_rhs.R()*a_rw)>>a_tShift;
		m_pDst->g = (a_lhs.G()*a_lw + a_rhs.G()*a_rw)>>a_tShift;
		m_pDst->b = (a_lhs.B()*a_lw + a_rhs.B()*a_rw)>>a_tShift;
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_1, TWeight const a_w1, TSrc const& a_2, TWeight const a_w2, TSrc const& a_3, TWeight const a_w3, TSrc const& a_4, TWeight const a_w4)
	{
		m_pDst->r = (a_1.R()*a_w1+a_2.R()*a_w2+a_3.R()*a_w3+a_4.R()*a_w4)>>a_tShift;
		m_pDst->g = (a_1.G()*a_w1+a_2.G()*a_w2+a_3.G()*a_w3+a_4.G()*a_w4)>>a_tShift;
		m_pDst->b = (a_1.B()*a_w1+a_2.B()*a_w2+a_3.B()*a_w3+a_4.B()*a_w4)>>a_tShift;
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw, TSrc const* a_pFullyCovered, size_t a_nFullyCovered, TWeight a_tTotalWeight)
	{
		if (a_tShift > 16)
		{
			TComponent r = (a_lhs.R()>>8)*a_lw + (a_rhs.R()>>8)*a_rw;
			TComponent g = (a_lhs.G()>>8)*a_lw + (a_rhs.G()>>8)*a_rw;
			TComponent b = (a_lhs.B()>>8)*a_lw + (a_rhs.B()>>8)*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R();
				g += a_pFullyCovered->G();
				b += a_pFullyCovered->B();
			}
			a_tTotalWeight <<= (a_tShift-16);
			m_pDst->r = r/a_tTotalWeight;
			m_pDst->g = g/a_tTotalWeight;
			m_pDst->b = b/a_tTotalWeight;
		}
		else
		{
			TComponent r = a_lhs.R()*a_lw + a_rhs.R()*a_rw;
			TComponent g = a_lhs.G()*a_lw + a_rhs.G()*a_rw;
			TComponent b = a_lhs.B()*a_lw + a_rhs.B()*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R()<<8;
				g += a_pFullyCovered->G()<<8;
				b += a_pFullyCovered->B()<<8;
			}
			a_tTotalWeight <<= (a_tShift-8);
			m_pDst->r = r/a_tTotalWeight;
			m_pDst->g = g/a_tTotalWeight;
			m_pDst->b = b/a_tTotalWeight;
		}
	}
	void operator ++()
	{
		++m_pDst;
	}
	void operator ++(int)
	{
		++m_pDst;
	}
private:
	SResamplingRGB8* m_pDst;
};

template<typename TComponent, int a_tShift>
struct CRGBX8Dst
{
	CRGBX8Dst() {}
	CRGBX8Dst(SResamplingRGBX8* a_pDst) : m_pDst(a_pDst) {}
	void Init(SResamplingRGBX8* a_pDst) { m_pDst = a_pDst; }

	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw)
	{
		m_pDst->r = (a_lhs.R()*a_lw + a_rhs.R()*a_rw)>>a_tShift;
		m_pDst->g = (a_lhs.G()*a_lw + a_rhs.G()*a_rw)>>a_tShift;
		m_pDst->b = (a_lhs.B()*a_lw + a_rhs.B()*a_rw)>>a_tShift;
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_1, TWeight const a_w1, TSrc const& a_2, TWeight const a_w2, TSrc const& a_3, TWeight const a_w3, TSrc const& a_4, TWeight const a_w4)
	{
		m_pDst->r = (a_1.R()*a_w1+a_2.R()*a_w2+a_3.R()*a_w3+a_4.R()*a_w4)>>a_tShift;
		m_pDst->g = (a_1.G()*a_w1+a_2.G()*a_w2+a_3.G()*a_w3+a_4.G()*a_w4)>>a_tShift;
		m_pDst->b = (a_1.B()*a_w1+a_2.B()*a_w2+a_3.B()*a_w3+a_4.B()*a_w4)>>a_tShift;
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw, TSrc const* a_pFullyCovered, size_t a_nFullyCovered, TWeight a_tTotalWeight)
	{
		if (a_tShift > 16)
		{
			TComponent r = (a_lhs.R()>>8)*a_lw + (a_rhs.R()>>8)*a_rw;
			TComponent g = (a_lhs.G()>>8)*a_lw + (a_rhs.G()>>8)*a_rw;
			TComponent b = (a_lhs.B()>>8)*a_lw + (a_rhs.B()>>8)*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R();
				g += a_pFullyCovered->G();
				b += a_pFullyCovered->B();
			}
			a_tTotalWeight <<= (a_tShift-16);
			m_pDst->r = r/a_tTotalWeight;
			m_pDst->g = g/a_tTotalWeight;
			m_pDst->b = b/a_tTotalWeight;
		}
		else
		{
			TComponent r = a_lhs.R()*a_lw + a_rhs.R()*a_rw;
			TComponent g = a_lhs.G()*a_lw + a_rhs.G()*a_rw;
			TComponent b = a_lhs.B()*a_lw + a_rhs.B()*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R()<<8;
				g += a_pFullyCovered->G()<<8;
				b += a_pFullyCovered->B()<<8;
			}
			a_tTotalWeight <<= (a_tShift-8);
			m_pDst->r = r/a_tTotalWeight;
			m_pDst->g = g/a_tTotalWeight;
			m_pDst->b = b/a_tTotalWeight;
		}
	}
	void operator ++()
	{
		++m_pDst;
	}
	void operator ++(int)
	{
		++m_pDst;
	}
private:
	SResamplingRGBX8* m_pDst;
};

typedef CResamplingT<SResamplingRGB8, CRGB8Src<SResamplingRGB8>, CRGB8Dst> CResamplingRGB8;
typedef CResamplingT<SResamplingRGBX8, CRGB8Src<SResamplingRGBX8>, CRGBX8Dst> CResamplingRGBX8;

struct CGammaPremultiply
{
	CGammaPremultiply(TRasterImagePixel const* a_p, WORD const* a_pGamma) : m_p(a_p), m_pGamma(a_pGamma) {}
	CGammaPremultiply() {}
	void Init(TRasterImagePixel const* a_p, WORD const* a_pGamma) { m_p = a_p; m_pGamma = a_pGamma; }

	typedef unsigned __int64 TComponent;
	struct CachedItem
	{
		CachedItem() {}
		CachedItem(CGammaPremultiply const a_t) :
			r(TComponent(a_t.m_pGamma[a_t.m_p->bR])*a_t.m_p->bA),
			g(TComponent(a_t.m_pGamma[a_t.m_p->bG])*a_t.m_p->bA),
			b(TComponent(a_t.m_pGamma[a_t.m_p->bB])*a_t.m_p->bA),
			a(a_t.m_p->bA)
		{
		}
		void operator =(CGammaPremultiply const a_t)
		{
			r = TComponent(a_t.m_pGamma[a_t.m_p->bR])*a_t.m_p->bA;
			g = TComponent(a_t.m_pGamma[a_t.m_p->bG])*a_t.m_p->bA;
			b = TComponent(a_t.m_pGamma[a_t.m_p->bB])*a_t.m_p->bA;
			a = a_t.m_p->bA;
		}
		TComponent R() const { return r; }
		TComponent G() const { return g; }
		TComponent B() const { return b; }
		TComponent A() const { return a; }
		void R(TComponent a_r) { r = a_r; }
		void G(TComponent a_g) { g = a_g; }
		void B(TComponent a_b) { b = a_b; }
		void A(TComponent a_a) { a = a_a; }

	private:
		TComponent r, g, b, a;
	};

	void operator++()
	{
		++m_p;
	}
	void operator++(int)
	{
		++m_p;
	}

private:
	TRasterImagePixel const* m_p;
	WORD const* m_pGamma;
};

template<typename TComponent, int a_tShift>
struct CGammaDemultiplyingDst
{
	CGammaDemultiplyingDst() {}
	CGammaDemultiplyingDst(TRasterImagePixel* a_pDst, BYTE const* a_pInvGamma) : m_pDst(a_pDst), m_pInvGamma(a_pInvGamma) {}
	void Init(TRasterImagePixel* a_pDst, BYTE const* a_pInvGamma) { m_pDst = a_pDst; m_pInvGamma = a_pInvGamma; }

	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw)
	{
		TComponent a = a_lhs.A()*a_lw + a_rhs.A()*a_rw;
		TComponent tFinalA = a>>a_tShift;
		if (tFinalA == 0)
		{
			TRasterImagePixel static const t = {0, 0, 0, 0};
			*m_pDst = t;
		}
		else
		{
			a >>= 8;
			TRasterImagePixel const t =
			{
				m_pInvGamma[((a_lhs.B()>>8)*a_lw + (a_rhs.B()>>8)*a_rw)/a],
				m_pInvGamma[((a_lhs.G()>>8)*a_lw + (a_rhs.G()>>8)*a_rw)/a],
				m_pInvGamma[((a_lhs.R()>>8)*a_lw + (a_rhs.R()>>8)*a_rw)/a],
				tFinalA
			};
			*m_pDst = t;
		}
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_1, TWeight const a_w1, TSrc const& a_2, TWeight const a_w2, TSrc const& a_3, TWeight const a_w3, TSrc const& a_4, TWeight const a_w4)
	{
		TComponent a = a_1.A()*a_w1+a_2.A()*a_w2+a_3.A()*a_w3+a_4.A()*a_w4;
		TComponent tFinalA = a>>a_tShift;
		if (tFinalA == 0)
		{
			TRasterImagePixel static const t = {0, 0, 0, 0};
			*m_pDst = t;
		}
		else
		{
			a >>= 8;
			TRasterImagePixel const t =
			{
				m_pInvGamma[((a_1.B()>>8)*a_w1+(a_2.B()>>8)*a_w2+(a_3.B()>>8)*a_w3+(a_4.B()>>8)*a_w4)/a],
				m_pInvGamma[((a_1.G()>>8)*a_w1+(a_2.G()>>8)*a_w2+(a_3.G()>>8)*a_w3+(a_4.G()>>8)*a_w4)/a],
				m_pInvGamma[((a_1.R()>>8)*a_w1+(a_2.R()>>8)*a_w2+(a_3.R()>>8)*a_w3+(a_4.R()>>8)*a_w4)/a],
				tFinalA
			};
			*m_pDst = t;
		}
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw, TSrc const* a_pFullyCovered, size_t a_nFullyCovered, TWeight a_tTotalWeight)
	{
		TComponent r = a_lhs.R()*a_lw + a_rhs.R()*a_rw;
		TComponent g = a_lhs.G()*a_lw + a_rhs.G()*a_rw;
		TComponent b = a_lhs.B()*a_lw + a_rhs.B()*a_rw;
		TComponent a = a_lhs.A()*a_lw + a_rhs.A()*a_rw;
		for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
		{
			r += a_pFullyCovered->R()<<8;
			g += a_pFullyCovered->G()<<8;
			b += a_pFullyCovered->B()<<8;
			a += a_pFullyCovered->A()<<8;
		}
		TComponent tFinalA = (a/a_tTotalWeight)>>(a_tShift-8);
		if (tFinalA == 0)
		{
			TRasterImagePixel static const t = {0, 0, 0, 0};
			*m_pDst = t;
		}
		else
		{
			TRasterImagePixel t =
			{
				m_pInvGamma[b/a],
				m_pInvGamma[g/a],
				m_pInvGamma[r/a],
				tFinalA
			};
			*m_pDst = t;
		}
	}
	void operator ++()
	{
		++m_pDst;
	}
	void operator ++(int)
	{
		++m_pDst;
	}
private:
	TRasterImagePixel* m_pDst;
	BYTE const* m_pInvGamma;
};

template<typename TImagePixel, class TSource, template<typename,int> class TDestination, int t_nGammaBits>
class CGammaResamplingT
{
	template<class TXZoom, class TYZoom, int t_nShift>
	void LinearZoomHelper() const
	{
		TXZoom aCtrlX(m_nSrcSizeX, m_nDstSizeX);
		TYZoom aCtrlY(m_nSrcSizeY, m_nDstSizeY);
		CAutoVectorPtr<TSource> aRowSrcs(new TSource[m_nSrcSizeY]);
		CAutoVectorPtr<TDestination<unsigned __int64, t_nShift> > aRowDsts(new TDestination<unsigned __int64, t_nShift>[m_nDstSizeY]);
		for (LONG y = 0; y < m_nDstSizeY; ++y)
		{
			aRowDsts[y].Init(reinterpret_cast<TImagePixel*>(reinterpret_cast<BYTE*>(m_pDst)+m_nDstStride*y), m_pInvGamma);
		}
		for (LONG y = 0; y < m_nSrcSizeY; ++y)
		{
			aRowSrcs[y].Init(reinterpret_cast<TImagePixel const*>(reinterpret_cast<BYTE const*>(m_pSrc)+m_nSrcStride*y), m_pGamma);
		}
		CPixelRow<TSource, TXZoom > const cRowSrc(aRowSrcs.m_p, aCtrlX, m_nDstSizeX);
		CRowDst<TDestination<unsigned __int64, t_nShift> > cRowDst(aRowDsts.m_p);
		aCtrlY.Execute(cRowSrc, cRowDst);
	}
	void InitGamma(float a_fGamma)
	{
		m_pInvGamma = new BYTE[(1<<t_nGammaBits)+16];
		m_pGamma = new WORD[256];
		float const fMul = (1<<t_nGammaBits)-1;
		for (ULONG i = 0; i < 256; ++i)
		{
			m_pGamma[i] = powf(i/255.0f, a_fGamma)*fMul+0.5f;
		}
		for (ULONG i = 0; i < (1<<t_nGammaBits); ++i)
		{
			m_pInvGamma[i] = powf(i/fMul, 1.0f/a_fGamma)*255.0f+0.5f;
		}
		// it can happen due to rounding errors
		for (ULONG i = 0; i < 16; ++i)
			m_pInvGamma[i+(1<<t_nGammaBits)] = 255;
	}

public:
	CGammaResamplingT(float a_fGamma, LONG a_nDstSizeX, LONG a_nDstSizeY, LONG a_nSrcSizeX, LONG a_nSrcSizeY, TImagePixel* a_pDst, TImagePixel const* a_pSrc) :
		m_nDstSizeX(a_nDstSizeX), m_nDstSizeY(a_nDstSizeY), m_nDstStride(a_nDstSizeX*sizeof(TImagePixel)), m_nSrcSizeX(a_nSrcSizeX), m_nSrcSizeY(a_nSrcSizeY), m_nSrcStride(a_nSrcSizeX*sizeof(TImagePixel)), m_pDst(a_pDst), m_pSrc(a_pSrc)
	{
		InitGamma(a_fGamma);
	}
	CGammaResamplingT(float a_fGamma, LONG a_nDstSizeX, LONG a_nDstSizeY, LONG a_nSrcSizeX, LONG a_nSrcSizeY, TImagePixel* a_pDst, LONG a_nDstStride, TImagePixel const* a_pSrc, LONG a_nSrcStride) :
		m_nDstSizeX(a_nDstSizeX), m_nDstSizeY(a_nDstSizeY), m_nDstStride(a_nDstStride), m_nSrcSizeX(a_nSrcSizeX), m_nSrcSizeY(a_nSrcSizeY), m_nSrcStride(a_nSrcStride), m_pDst(a_pDst), m_pSrc(a_pSrc)
	{
		InitGamma(a_fGamma);
	}
	~CGammaResamplingT()
	{
		delete[] m_pInvGamma;
		delete[] m_pGamma;
	}

	void Nearest() const
	{
		TDrawCoord t;
		float fStepX = m_nSrcSizeX/static_cast<float>(m_nDstSizeX);
		float fStepY = m_nSrcSizeY/static_cast<float>(m_nDstSizeY);
		for (t.nY = 0; t.nY < m_nDstSizeY; t.nY++)
		{
			TImagePixel* pDstRow = reinterpret_cast<TImagePixel*>(reinterpret_cast<BYTE*>(m_pDst)+m_nDstStride*t.nY);
			TImagePixel const* pSrcRow = reinterpret_cast<TImagePixel const*>(reinterpret_cast<BYTE const*>(m_pSrc)+m_nSrcStride*static_cast<int>(t.nY*fStepY));
			for (t.nX = 0; t.nX < m_nDstSizeX; t.nX++)
			{
				pDstRow[t.nX] = pSrcRow[static_cast<int>(t.nX*fStepX)];
			}
		}
	}

	void Linear() const
	{
		if (m_nSrcSizeX == 1)
		{
			if (m_nSrcSizeY == 1)
			{
				Nearest();//LinearZoomHelper<TPixelMixer, ZoomSingleItem, ZoomSingleItem, 16>(a_pImage);
			}
			else
			{
				if (m_nDstSizeY == m_nSrcSizeY)
					Nearest();//LinearZoomHelper<TPixelMixer, ZoomSingleItem, CopyWithoutZoom, 16>(a_pImage);
				else if (m_nDstSizeY > m_nSrcSizeY)
					LinearZoomHelper<ZoomSingleItem, LinearZoomIn<unsigned __int64>, 16>();
				else
					LinearZoomHelper<ZoomSingleItem, LinearZoomOut<unsigned __int64>, 16>();
			}
		}
		else
		{
			if (m_nSrcSizeY == 1)
			{
				if (m_nDstSizeX == m_nSrcSizeX)
					Nearest();//LinearZoomHelper<TPixelMixer, CopyWithoutZoom, ZoomSingleItem, 16>(a_pImage);
				else if (m_nDstSizeX > m_nSrcSizeX)
					LinearZoomHelper<LinearZoomIn<unsigned __int64>, ZoomSingleItem, 16>();
				else
					LinearZoomHelper<LinearZoomOut<unsigned __int64>, ZoomSingleItem, 16>();
			}
			else
			{
				if (m_nDstSizeY == m_nSrcSizeY)
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						Nearest();//LinearZoomHelper<TPixelMixer, CopyWithoutZoom, CopyWithoutZoom, 16>(a_pImage);
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<LinearZoomIn<unsigned __int64>, CopyWithoutZoom, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned __int64>, CopyWithoutZoom, 16>();
				}
				else if (m_nDstSizeY > m_nSrcSizeY)
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						LinearZoomHelper<CopyWithoutZoom, LinearZoomIn<unsigned __int64>, 16>();
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<LinearZoomIn<unsigned __int64>, LinearZoomIn<unsigned __int64>, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned __int64>, LinearZoomIn<unsigned __int64>, 16>();
				}
				else
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						LinearZoomHelper<CopyWithoutZoom, LinearZoomOut<unsigned __int64>, 16>();
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<LinearZoomIn<unsigned __int64>, LinearZoomOut<unsigned __int64>, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned __int64>, LinearZoomOut<unsigned __int64>, 16>();
				}
			}
		}
	}

	void Cubic() const
	{
		if (m_nSrcSizeX == 1)
		{
			if (m_nSrcSizeY == 1)
			{
				Nearest();//LinearZoomHelper<TPixelMixer, ZoomSingleItem, ZoomSingleItem, 16>(a_pImage);
			}
			else
			{
				if (m_nDstSizeY == m_nSrcSizeY)
					Nearest();//LinearZoomHelper<TPixelMixer, ZoomSingleItem, CopyWithoutZoom, 16>(a_pImage);
				else if (m_nDstSizeY > m_nSrcSizeY)
					LinearZoomHelper<ZoomSingleItem, CubicZoomIn<unsigned __int64>, 16>();
				else
					LinearZoomHelper<ZoomSingleItem, LinearZoomOut<unsigned __int64>, 16>();
			}
		}
		else
		{
			if (m_nSrcSizeY == 1)
			{
				if (m_nDstSizeX == m_nSrcSizeX)
					Nearest();//LinearZoomHelper<TPixelMixer, CopyWithoutZoom, ZoomSingleItem, 16>(a_pImage);
				else if (m_nDstSizeX > m_nSrcSizeX)
					LinearZoomHelper<CubicZoomIn<unsigned __int64>, ZoomSingleItem, 16>();
				else
					LinearZoomHelper<LinearZoomOut<unsigned __int64>, ZoomSingleItem, 16>();
			}
			else
			{
				if (m_nDstSizeY == m_nSrcSizeY)
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						Nearest();//LinearZoomHelper<TPixelMixer, CopyWithoutZoom, CopyWithoutZoom, 16>(a_pImage);
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<CubicZoomIn<unsigned __int64>, CopyWithoutZoom, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned __int64>, CopyWithoutZoom, 16>();
				}
				else if (m_nDstSizeY > m_nSrcSizeY)
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						LinearZoomHelper<CopyWithoutZoom, CubicZoomIn<unsigned __int64>, 16>();
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<CubicZoomIn<unsigned __int64>, CubicZoomIn<unsigned __int64>, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned __int64>, CubicZoomIn<unsigned __int64>, 16>();
				}
				else
				{
					if (m_nDstSizeX == m_nSrcSizeX)
						LinearZoomHelper<CopyWithoutZoom, LinearZoomOut<unsigned __int64>, 16>();
					else if (m_nDstSizeX > m_nSrcSizeX)
						LinearZoomHelper<CubicZoomIn<unsigned __int64>, LinearZoomOut<unsigned __int64>, 16>();
					else
						LinearZoomHelper<LinearZoomOut<unsigned __int64>, LinearZoomOut<unsigned __int64>, 16>();
				}
			}
		}
	}

private:
	LONG const m_nDstSizeX;
	LONG const m_nDstSizeY;
	LONG const m_nDstStride;
	LONG const m_nSrcSizeX;
	LONG const m_nSrcSizeY;
	LONG const m_nSrcStride;
	TImagePixel* const m_pDst;
	TImagePixel const* const m_pSrc;
	BYTE* m_pInvGamma;
	WORD* m_pGamma;
};

typedef CGammaResamplingT<TRasterImagePixel, CGammaPremultiply, CGammaDemultiplyingDst, 16> CGammaResampling;


template<typename TPixel>
struct CRGB8GammaSrc
{
	CRGB8GammaSrc(TPixel const* a_p, WORD const* a_pGamma) : m_p(a_p), m_pGamma(a_pGamma) {}
	CRGB8GammaSrc() {}
	void Init(TPixel const* a_p, WORD const* a_pGamma) { m_p = a_p; m_pGamma = a_pGamma; }

	typedef unsigned TComponent;
	struct CachedItem
	{
		CachedItem() {}
		CachedItem(CRGB8GammaSrc<TPixel> const a_t) :
			r(a_t.m_pGamma[a_t.m_p->r]),
			g(a_t.m_pGamma[a_t.m_p->g]),
			b(a_t.m_pGamma[a_t.m_p->b])
		{
		}
		void operator =(CRGB8GammaSrc<TPixel> const a_t)
		{
			r = a_t.m_pGamma[a_t.m_p->r];
			g = a_t.m_pGamma[a_t.m_p->g];
			b = a_t.m_pGamma[a_t.m_p->b];
		}
		TComponent R() const { return r; }
		TComponent G() const { return g; }
		TComponent B() const { return b; }
		TComponent A() const { return 255; }
		void R(TComponent a_r) { r = a_r; }
		void G(TComponent a_g) { g = a_g; }
		void B(TComponent a_b) { b = a_b; }
		void A(TComponent a_a) { }

	private:
		TComponent r, g, b;
	};

	void operator++()
	{
		++m_p;
	}
	void operator++(int)
	{
		++m_p;
	}

private:
	TPixel const* m_p;
	WORD const* m_pGamma;
};

template<typename TComponent, int a_tShift>
struct CRGB8GammaDst
{
	CRGB8GammaDst() {}
	CRGB8GammaDst(SResamplingRGB8* a_pDst, BYTE const* a_pInvGamma) : m_pDst(a_pDst), m_pInvGamma(a_pInvGamma) {}
	void Init(SResamplingRGB8* a_pDst, BYTE const* a_pInvGamma) { m_pDst = a_pDst; m_pInvGamma = a_pInvGamma; }

	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw)
	{
		m_pDst->r = m_pInvGamma[(a_lhs.R()*a_lw + a_rhs.R()*a_rw)>>a_tShift];
		m_pDst->g = m_pInvGamma[(a_lhs.G()*a_lw + a_rhs.G()*a_rw)>>a_tShift];
		m_pDst->b = m_pInvGamma[(a_lhs.B()*a_lw + a_rhs.B()*a_rw)>>a_tShift];
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_1, TWeight const a_w1, TSrc const& a_2, TWeight const a_w2, TSrc const& a_3, TWeight const a_w3, TSrc const& a_4, TWeight const a_w4)
	{
		m_pDst->r = m_pInvGamma[(a_1.R()*a_w1+a_2.R()*a_w2+a_3.R()*a_w3+a_4.R()*a_w4)>>a_tShift];
		m_pDst->g = m_pInvGamma[(a_1.G()*a_w1+a_2.G()*a_w2+a_3.G()*a_w3+a_4.G()*a_w4)>>a_tShift];
		m_pDst->b = m_pInvGamma[(a_1.B()*a_w1+a_2.B()*a_w2+a_3.B()*a_w3+a_4.B()*a_w4)>>a_tShift];
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw, TSrc const* a_pFullyCovered, size_t a_nFullyCovered, TWeight a_tTotalWeight)
	{
		if (a_tShift > 16)
		{
			TComponent r = (a_lhs.R()>>8)*a_lw + (a_rhs.R()>>8)*a_rw;
			TComponent g = (a_lhs.G()>>8)*a_lw + (a_rhs.G()>>8)*a_rw;
			TComponent b = (a_lhs.B()>>8)*a_lw + (a_rhs.B()>>8)*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R();
				g += a_pFullyCovered->G();
				b += a_pFullyCovered->B();
			}
			a_tTotalWeight <<= (a_tShift-16);
			m_pDst->r = m_pInvGamma[r/a_tTotalWeight];
			m_pDst->g = m_pInvGamma[g/a_tTotalWeight];
			m_pDst->b = m_pInvGamma[b/a_tTotalWeight];
		}
		else
		{
			TComponent r = a_lhs.R()*a_lw + a_rhs.R()*a_rw;
			TComponent g = a_lhs.G()*a_lw + a_rhs.G()*a_rw;
			TComponent b = a_lhs.B()*a_lw + a_rhs.B()*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R()<<8;
				g += a_pFullyCovered->G()<<8;
				b += a_pFullyCovered->B()<<8;
			}
			a_tTotalWeight <<= (a_tShift-8);
			m_pDst->r = m_pInvGamma[r/a_tTotalWeight];
			m_pDst->g = m_pInvGamma[g/a_tTotalWeight];
			m_pDst->b = m_pInvGamma[b/a_tTotalWeight];
		}
	}
	void operator ++()
	{
		++m_pDst;
	}
	void operator ++(int)
	{
		++m_pDst;
	}
private:
	SResamplingRGB8* m_pDst;
	BYTE const* m_pInvGamma;
};

template<typename TComponent, int a_tShift>
struct CRGBX8GammaDst
{
	CRGBX8GammaDst() {}
	CRGBX8GammaDst(SResamplingRGBX8* a_pDst, BYTE const* a_pInvGamma) : m_pDst(a_pDst), m_pInvGamma(a_pInvGamma) {}
	void Init(SResamplingRGBX8* a_pDst, BYTE const* a_pInvGamma) { m_pDst = a_pDst; m_pInvGamma = a_pInvGamma; }

	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw)
	{
		m_pDst->r = m_pInvGamma[(a_lhs.R()*a_lw + a_rhs.R()*a_rw)>>a_tShift];
		m_pDst->g = m_pInvGamma[(a_lhs.G()*a_lw + a_rhs.G()*a_rw)>>a_tShift];
		m_pDst->b = m_pInvGamma[(a_lhs.B()*a_lw + a_rhs.B()*a_rw)>>a_tShift];
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_1, TWeight const a_w1, TSrc const& a_2, TWeight const a_w2, TSrc const& a_3, TWeight const a_w3, TSrc const& a_4, TWeight const a_w4)
	{
		m_pDst->r = m_pInvGamma[(a_1.R()*a_w1+a_2.R()*a_w2+a_3.R()*a_w3+a_4.R()*a_w4)>>a_tShift];
		m_pDst->g = m_pInvGamma[(a_1.G()*a_w1+a_2.G()*a_w2+a_3.G()*a_w3+a_4.G()*a_w4)>>a_tShift];
		m_pDst->b = m_pInvGamma[(a_1.B()*a_w1+a_2.B()*a_w2+a_3.B()*a_w3+a_4.B()*a_w4)>>a_tShift];
	}
	template<typename TSrc, typename TWeight>
	inline void Blend(TSrc const& a_lhs, TWeight const a_lw, TSrc const& a_rhs, TWeight const a_rw, TSrc const* a_pFullyCovered, size_t a_nFullyCovered, TWeight a_tTotalWeight)
	{
		if (a_tShift > 16)
		{
			TComponent r = (a_lhs.R()>>8)*a_lw + (a_rhs.R()>>8)*a_rw;
			TComponent g = (a_lhs.G()>>8)*a_lw + (a_rhs.G()>>8)*a_rw;
			TComponent b = (a_lhs.B()>>8)*a_lw + (a_rhs.B()>>8)*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R();
				g += a_pFullyCovered->G();
				b += a_pFullyCovered->B();
			}
			a_tTotalWeight <<= (a_tShift-16);
			m_pDst->r = m_pInvGamma[r/a_tTotalWeight];
			m_pDst->g = m_pInvGamma[g/a_tTotalWeight];
			m_pDst->b = m_pInvGamma[b/a_tTotalWeight];
		}
		else
		{
			TComponent r = a_lhs.R()*a_lw + a_rhs.R()*a_rw;
			TComponent g = a_lhs.G()*a_lw + a_rhs.G()*a_rw;
			TComponent b = a_lhs.B()*a_lw + a_rhs.B()*a_rw;
			for (size_t i = 0; i < a_nFullyCovered; ++i, ++a_pFullyCovered)
			{
				r += a_pFullyCovered->R()<<8;
				g += a_pFullyCovered->G()<<8;
				b += a_pFullyCovered->B()<<8;
			}
			a_tTotalWeight <<= (a_tShift-8);
			m_pDst->r = m_pInvGamma[r/a_tTotalWeight];
			m_pDst->g = m_pInvGamma[g/a_tTotalWeight];
			m_pDst->b = m_pInvGamma[b/a_tTotalWeight];
		}
	}
	void operator ++()
	{
		++m_pDst;
	}
	void operator ++(int)
	{
		++m_pDst;
	}
private:
	SResamplingRGBX8* m_pDst;
	BYTE const* m_pInvGamma;
};

typedef CGammaResamplingT<SResamplingRGB8, CRGB8GammaSrc<SResamplingRGB8>, CRGB8GammaDst, 16> CGammaResamplingRGB8;
typedef CGammaResamplingT<SResamplingRGBX8, CRGB8GammaSrc<SResamplingRGBX8>, CRGBX8GammaDst, 16> CGammaResamplingRGBX8;
