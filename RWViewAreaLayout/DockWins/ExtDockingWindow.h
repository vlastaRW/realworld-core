// Copyright (c) 2002
// Sergey Klimov (kidd@ukr.net)
// WTL Docking windows
//
// This code is provided "as is", with absolutely no warranty expressed
// or implied. Any use is at your own risk.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name is included. If
// the source code in  this file is used in any commercial application
// then a simple email would be nice.

#ifndef WTL_DW_EXTDOCKINGWINDOW_H_INCLUDED_
#define WTL_DW_EXTDOCKINGWINDOW_H_INCLUDED_

#include <DockingWindow.h>

namespace dockwins{

#ifdef DF_AUTO_HIDE_FEATURES
class CPinIcons
{
public:
	enum States
	{
		sUnPinned=0,
		sPinned=1
	};
	CPinIcons()
	{
		static BYTE pinnedIconData[]={
				0x28, 0000, 0000, 0000, 0x0b, 0000, 0000, 0000,
				0x16, 0000, 0000, 0000, 0x01, 0000, 0x04, 0000,
				0000, 0000, 0000, 0000, 0x84, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0x10, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0x80, 0000,
				0000, 0x80, 0000, 0000, 0000, 0x80, 0x80, 0000,
				0x80, 0000, 0000, 0000, 0x80, 0000, 0x80, 0000,
				0x80, 0x80, 0000, 0000, 0x80, 0x80, 0x80, 0000,
				0xc0, 0xc0, 0xc0, 0000, 0000, 0000, 0xff, 0000,
				0000, 0xff, 0000, 0000, 0000, 0xff, 0xff, 0000,
				0xff, 0000, 0000, 0000, 0xff, 0000, 0xff, 0000,
				0xff, 0xff, 0000, 0000, 0xff, 0xff, 0xff, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0x07, 0x70, 0000, 0000, 0000, 0000, 0000,
				0000, 0x77, 0x77, 0000, 0000, 0000, 0000, 0000,
				0000, 0x7f, 0x80, 0000, 0000, 0000, 0000, 0000,
				0000, 0xf0, 0x07, 0x77, 0000, 0000, 0000, 0000,
				0000, 0x0f, 0000, 0xf7, 0x70, 0000, 0000, 0000,
				0000, 0000, 0x0f, 0x0f, 0x70, 0000, 0000, 0000,
				0000, 0000, 0000, 0xf0, 0x70, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0x7f, 0xff, 0xff, 0xff, 0x81, 0xff, 0xff, 0xff,
				0x80, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff,
				0x80, 0x7f, 0xff, 0xff, 0x90, 0x3f, 0xff, 0xff,
				0xa4, 0x3f, 0xff, 0xff, 0xd2, 0x3f, 0xff, 0xff,
				0xe5, 0x3f, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff};
		static BYTE unpinnedIconData[]={
				0x28, 0000, 0000, 0000, 0x0b, 0000, 0000, 0000,
				0x16, 0000, 0000, 0000, 0x01, 0000, 0x04, 0000,
				0000, 0000, 0000, 0000, 0x84, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0x10, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0x80, 0000,
				0000, 0x80, 0000, 0000, 0000, 0x80, 0x80, 0000,
				0x80, 0000, 0000, 0000, 0x80, 0000, 0x80, 0000,
				0x80, 0x80, 0000, 0000, 0x80, 0x80, 0x80, 0000,
				0xc0, 0xc0, 0xc0, 0000, 0000, 0000, 0xff, 0000,
				0000, 0xff, 0000, 0000, 0000, 0xff, 0xff, 0000,
				0xff, 0000, 0000, 0000, 0xff, 0000, 0xff, 0000,
				0xff, 0xff, 0000, 0000, 0xff, 0xff, 0xff, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0000, 0x08, 0000, 0x08, 0000, 0000, 0000,
				0000, 0000, 0000, 0x08, 0000, 0000, 0000, 0000,
				0000, 0000, 0x0f, 0x0f, 0x8f, 0000, 0000, 0000,
				0000, 0000, 0000, 0xf0, 0000, 0000, 0000, 0000,
				0000, 0000, 0x0f, 0000, 0x0f, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
				0xff, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff,
				0xf3, 0x9f, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff,
				0xf4, 0x5f, 0xff, 0xff, 0x02, 0x1f, 0xff, 0xff,
				0xf5, 0xdf, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff,
				0xf3, 0x9f, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff};
		m_icons[0]=CreateIconFromResourceEx(unpinnedIconData, 0xec, TRUE, 0x00030000, 11, 11, LR_DEFAULTCOLOR);
		m_icons[1]=CreateIconFromResourceEx(pinnedIconData, 0xec, TRUE, 0x00030000, 11, 11, LR_DEFAULTCOLOR);
	}
	~CPinIcons()
	{
		DestroyIcon(m_icons[0]);
		DestroyIcon(m_icons[1]);
	}
	HICON GetIcon(States state) const
	{
		return m_icons[state];
	}
	int Width() const
	{
		return 11;
	}
	int Height() const
	{
		return 11;
	}
protected:
	HICON m_icons[2];
};
#endif

class COutlookLikeExCaption : public CCaptionBase
{
	typedef COutlookLikeExCaption	thisClass;
	typedef CCaptionBase			baseClass;
public:
	enum{fntSpace=10,cFrameSpace=2,btnSpace=1};
protected:
	typedef baseClass::CButton CButtonBase;
	struct CButton 
		: CButtonBase
	{
        virtual void CalculateRect(CRect& rc,bool bHorizontal=true)
        {
            CopyRect(rc);
    		DeflateRect(cFrameSpace+btnSpace,cFrameSpace+btnSpace);
			if(bHorizontal)
			{
				left=right-Height();
				rc.right=left;
			}
			else
			{
				bottom=top+Width();
				rc.top=bottom;
			}
        }
	};

	class CCloseButton 
		: public CButton
	{
	public:
        virtual void Draw(CDCHandle dc)
        {
            CPen pen;
#ifdef DF_FOCUS_FEATURES
			CCaptionFocus cf(dc);
			dc.FillRect(this,cf.GetCaptionBgBrush());
            pen.CreatePen(PS_SOLID, 0, cf.GetCaptionTextColor());
#else
			dc.FillRect(this,(HBRUSH)LongToPtr(COLOR_3DFACE + 1));
            pen.CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_BTNTEXT));
#endif

            HPEN hPenOld = dc.SelectPen(pen);
			const int sp=5;
            dc.MoveTo(left+sp, top+sp);
            dc.LineTo(right-sp, bottom-sp);
            dc.MoveTo(left + sp+1, top+sp);
            dc.LineTo(right - sp + 1, bottom - sp);

            dc.MoveTo(left+sp, bottom - sp-1);
            dc.LineTo(right-sp, top +sp -1 );
            dc.MoveTo(left + sp +1, bottom - sp -1);
            dc.LineTo(right - sp +1, top + sp -1);

            dc.SelectPen(hPenOld);
			CButton::Draw(dc);
		}
	};
#ifdef DF_AUTO_HIDE_FEATURES
	class CPinButton 
		: public CButton
	{
	public:
		typedef CPinIcons CIcons;
		CPinButton()
			:m_sicon(CIcons::sPinned)
		{
		}

		void State(CIcons::States state)
		{
			m_sicon=state;
		}

        virtual void Draw(CDCHandle dc)
        {
#ifdef DF_FOCUS_FEATURES
			CCaptionFocus cf(dc);
			dc.FillRect(this,cf.GetCaptionBgBrush());
#else
			dc.FillRect(this,(HBRUSH)LongToPtr(COLOR_3DFACE + 1));
#endif
			CPoint pt(left,top);
			CSize sz(Width(),Height());
			int dif = sz.cx-m_icons.Width();
			if(dif>0)
			{
				pt.x+=dif/2;
				sz.cx=m_icons.Width();
			}

			dif = sz.cy-m_icons.Height();
			if(dif>0)
			{
				pt.y+=dif/2;
				sz.cy=m_icons.Height();
			}
			dc.DrawIconEx(pt,m_icons.GetIcon(m_sicon),sz);
			CButton::Draw(dc);
		}
	protected:
		static CIcons	m_icons;
		CIcons::States	m_sicon;
	};
public:
	void SetPinButtonState(CPinButton::CIcons::States state)
	{
		m_btnPin.State(state);
	}
#endif
public:
	COutlookLikeExCaption():baseClass(0,false)
	{
		SetOrientation(!IsHorizontal());
	}

	void UpdateMetrics()
	{
		m_thickness=23;
		CDWSettings settings;
		HFONT hFont = IsHorizontal() ? settings.HSysFont() : settings.VSysFont();
		assert(hFont);
		HDC dc=::GetDC(NULL);
		if(dc!=NULL)
		{
			HFONT hOldFont = reinterpret_cast<HFONT>(::SelectObject(dc,hFont));
			if(hOldFont!=NULL)
			{
				TEXTMETRIC tm;
				if(::GetTextMetrics(dc, &tm))
					m_thickness=tm.tmHeight+fntSpace;
				::SelectObject(dc,hOldFont);
			}
			::ReleaseDC(NULL,dc);
		}
	}

    void SetOrientation(bool bHorizontal)
    {
		if(IsHorizontal()!=bHorizontal)
		{
			baseClass::SetOrientation(bHorizontal);
			UpdateMetrics();
		}
    }

	bool CalculateRect(CRect& rc,bool bTop)
	{
		bool bRes=baseClass::CalculateRect(rc,bTop);
		CRect rcSpace(*this);
		m_btnClose.CalculateRect(rcSpace,IsHorizontal());
#ifdef DF_AUTO_HIDE_FEATURES
		m_btnPin.CalculateRect(rcSpace,IsHorizontal());
#endif
		return bRes;
	}
	void Draw(HWND hWnd,CDC& dc)
	{
#ifdef DF_FOCUS_FEATURES
		CCaptionFocus cf(dc);
		dc.FillRect(this,cf.GetCaptionBgBrush());
#else
		dc.FillRect(this,(HBRUSH)LongToPtr(COLOR_3DFACE + 1));
#endif
		int len=GetWindowTextLength(hWnd)+1;
		TCHAR* sText=new TCHAR[len];
		if(GetWindowText(hWnd,sText,len)!=0)
		{
			HFONT hFont;
			CDWSettings settings;
			CRect rc(this);
			if(IsHorizontal())
			{
				rc.left+=fntSpace+cFrameSpace;
#ifdef DF_AUTO_HIDE_FEATURES
				rc.right=m_btnPin.left-cFrameSpace-btnSpace;
#else
				rc.right=m_btnClose.left-cFrameSpace-btnSpace;
#endif
				hFont = settings.HSysFont();
			}
			else
			{
				rc.bottom-=fntSpace-cFrameSpace;
#ifdef DF_AUTO_HIDE_FEATURES
				rc.top=m_btnPin.bottom+cFrameSpace+btnSpace;
#else
				rc.top=m_btnClose.bottom+cFrameSpace+btnSpace;
#endif
				 hFont = settings.VSysFont();
			}
#ifdef DF_FOCUS_FEATURES
			dc.SetTextColor(cf.GetCaptionTextColor());
#else
			dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
#endif
			dc.SetBkMode(TRANSPARENT);
			HFONT hFontOld = dc.SelectFont(hFont);
			if( (rc.left<rc.right) && (rc.top<rc.bottom))
				DrawEllipsisText(dc,sText,_tcslen(sText),&rc,IsHorizontal());
			dc.SelectFont(hFontOld);
		}
		m_btnClose.Draw(dc.m_hDC);
#ifdef DF_AUTO_HIDE_FEATURES
		m_btnPin.Draw(dc.m_hDC);
#endif
		dc.DrawEdge(this,EDGE_ETCHED,BF_RECT);
		delete [] sText;
	}

	LRESULT HitTest(const CPoint& pt) const
	{
		LRESULT lRes=HTNOWHERE;
		if(PtInRect(pt))
		{
			lRes=HTCAPTION;
			if(m_btnClose.PtInRect(pt))
				lRes=HTCLOSE;
#ifdef DF_AUTO_HIDE_FEATURES
			else
			{
				if(m_btnPin.PtInRect(pt))
					lRes=HTPIN;
			}
#endif
		}
		return lRes;
	}

	void HotTrack(HWND hWnd,unsigned int nHitTest)
	{
		m_btnClose.Hot(hWnd,nHitTest==HTCLOSE);
#ifdef DF_AUTO_HIDE_FEATURES
		m_btnPin.Hot(hWnd,nHitTest==HTPIN);
#endif
	}

	bool Action(HWND hWnd,const CPoint& /*pt*/,unsigned int nHitTest)
	{
		bool res=false;
		switch(nHitTest)
		{
			case HTCLOSE:
				m_btnClose.Press(hWnd,true);
				res=true;
				break;
#ifdef DF_AUTO_HIDE_FEATURES
			case HTPIN:
				m_btnPin.Press(hWnd,true);
				res=true;
				break;
#endif
		}
		return res;
	}
	void ActionDone(HWND hWnd,unsigned int nHitTest,bool /*ok*/)
	{
		switch(nHitTest)
		{
			case HTCLOSE:
				m_btnClose.Press(hWnd,false);
				break;
#ifdef DF_AUTO_HIDE_FEATURES
			case HTPIN:
				m_btnPin.Press(hWnd,false);
				break;
#endif
		}
	}
protected:
#ifdef DF_AUTO_HIDE_FEATURES
	CPinButton		m_btnPin;
#endif
	CCloseButton	m_btnClose;
};
struct COutlookLikeCaption :  COutlookLikeExCaption
{
    void SetOrientation(bool /*bHorizontal*/)
    {
		// horizontal only
	}
};

typedef CDockingWindowTraits<COutlookLikeCaption,
								WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW>
							COutlookLikeTitleDockingWindowTraits;

typedef CDockingWindowTraits<COutlookLikeExCaption,
								WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW>
							COutlookLikeExTitleDockingWindowTraits;

class CVC6LikeCaption : public CCaptionBase
{
	typedef CVC6LikeCaption thisClass;
	typedef CCaptionBase	baseClass;
public:
	enum{btnSpace=2,grThick=3};
protected:
	typedef baseClass::CButton CButtonBase;
	struct CButton : CButtonBase
	{
        virtual void CalculateRect(CRect& rc,bool bHorizontal)
        {
            CopyRect(rc);
    		DeflateRect(btnSpace,btnSpace);
			if(bHorizontal)
			{
				left=right-Height();
				rc.right=left+btnSpace;
			}
			else
			{
				bottom=top+Width();
				rc.top=bottom+btnSpace;
			}
        }
	};

	class CCloseButton: public CButton
	{
	public:
        virtual void Draw(CDCHandle dc)
        {

            CPen pen;
#ifdef DF_FOCUS_FEATURES
			CCaptionFocus cf(dc);
			dc.FillRect(this,cf.GetCaptionBgBrush());
            pen.CreatePen(PS_SOLID, 0, cf.GetCaptionTextColor());
#else
			dc.FillRect(this,(HBRUSH)LongToPtr(COLOR_3DFACE + 1));
            pen.CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_BTNTEXT));
#endif

            HPEN hPenOld = dc.SelectPen(pen);
			const int sp=3;
            dc.MoveTo(left+sp, top+sp);
            dc.LineTo(right-sp, bottom-sp);
            dc.MoveTo(left + sp+1, top+sp);
            dc.LineTo(right - sp + 1, bottom - sp);

            dc.MoveTo(left+sp, bottom - sp-1);
            dc.LineTo(right-sp, top +sp -1 );
            dc.MoveTo(left + sp +1, bottom - sp -1);
            dc.LineTo(right - sp +1, top + sp -1);


            dc.SelectPen(hPenOld);
			CButton::Draw(dc);
		}
	};
#ifdef DF_AUTO_HIDE_FEATURES
	class CPinButton : public CButton
	{
	public:
		typedef CPinIcons CIcons;
		CPinButton()
			:m_sicon(CIcons::sPinned)
		{
		}
		void State(CIcons::States state)
		{
			m_sicon=state;
		}
        virtual void Draw(CDCHandle dc)
        {
#ifdef DF_FOCUS_FEATURES
			CCaptionFocus cf(dc);
			dc.FillRect(this,cf.GetCaptionBgBrush());
#else
			dc.FillRect(this,(HBRUSH)LongToPtr(COLOR_3DFACE + 1));
#endif

			CPoint pt(left,top);
			CSize sz(Width(),Height());
			int dif = sz.cx-m_icons.Width();
			if(dif>0)
			{
				pt.x+=dif/2;
				sz.cx=m_icons.Width();
			}
			dif = sz.cy-m_icons.Height();
			if(dif>0)
			{
				pt.y+=dif/2;
				sz.cy=m_icons.Height();
			}
			dc.DrawIconEx(pt,m_icons.GetIcon(m_sicon),sz);
			CButton::Draw(dc);
		}
	protected:
		static CIcons	m_icons;
		CIcons::States	m_sicon;
	};
public:
	void SetPinButtonState(CPinButton::CIcons::States state)
	{
		m_btnPin.State(state);
	}
#endif
public:
	bool CalculateRect(CRect& rc,bool bTop)
	{
		bool bRes=baseClass::CalculateRect(rc,bTop);
		CRect rcSpace(*this);
		m_btnClose.CalculateRect(rcSpace,IsHorizontal());
#ifdef DF_AUTO_HIDE_FEATURES
		m_btnPin.CalculateRect(rcSpace,IsHorizontal());
#endif
		return bRes;
	}
	void Draw(HWND /*hWnd*/,CDC& dc)
	{
#ifdef DF_FOCUS_FEATURES
		CCaptionFocus cf(dc);
		dc.FillRect(this,cf.GetCaptionBgBrush());
#else
		dc.FillRect(this,(HBRUSH)LongToPtr(COLOR_3DFACE + 1));
#endif
		CRect rc;
		if(IsHorizontal())
		{
			rc.left=left+btnSpace;
#ifdef DF_AUTO_HIDE_FEATURES
			rc.right=m_btnPin.left-btnSpace;
#else
			rc.right=m_btnClose.left-btnSpace;
#endif
			if(rc.left<rc.right)
			{
#ifdef DF_AUTO_HIDE_FEATURES
				long offset=(m_btnPin.Height()-grThick*2/*+btnSpace*/)/2;
				rc.top=m_btnPin.top+offset;
#else
				long offset=(m_btnClose.Height()-grThick*2/*+btnSpace*/)/2;
				rc.top=m_btnClose.top+offset;
#endif
				rc.bottom=rc.top+grThick;
				dc.Draw3dRect(&rc, ::GetSysColor(COLOR_BTNHIGHLIGHT), ::GetSysColor(COLOR_BTNSHADOW));
				rc.top=rc.bottom/*+btnSpace*/;
				rc.bottom=rc.top+grThick;
				dc.Draw3dRect(&rc, ::GetSysColor(COLOR_BTNHIGHLIGHT), ::GetSysColor(COLOR_BTNSHADOW));
			}
		}
		else
		{
#ifdef DF_AUTO_HIDE_FEATURES
			rc.top=m_btnPin.bottom+btnSpace;
#else
			rc.top=m_btnClose.bottom+btnSpace;
#endif
			rc.bottom=bottom-btnSpace;
			if(rc.top<rc.bottom)
			{
#ifdef DF_AUTO_HIDE_FEATURES
				long offset=(m_btnPin.Width()-grThick*2/*+btnSpace*/)/2;
				rc.left=m_btnPin.left+offset;
#else
				long offset=(m_btnClose.Width()-grThick*2/*+btnSpace*/)/2;
				rc.left=m_btnClose.left+offset;
#endif
				rc.right=rc.left+grThick;
				dc.Draw3dRect(&rc, ::GetSysColor(COLOR_BTNHIGHLIGHT), ::GetSysColor(COLOR_BTNSHADOW));
				rc.left=rc.right/*+btnSpace*/;
				rc.right=rc.left+grThick;
				dc.Draw3dRect(&rc, ::GetSysColor(COLOR_BTNHIGHLIGHT), ::GetSysColor(COLOR_BTNSHADOW));
			}
		}

		m_btnClose.Draw(dc.m_hDC);
#ifdef DF_AUTO_HIDE_FEATURES
		m_btnPin.Draw(dc.m_hDC);
#endif
	}

	LRESULT HitTest(const CPoint& pt) const
	{
		LRESULT lRes=HTNOWHERE;
		if(PtInRect(pt))
		{
			lRes=HTCAPTION;
			if(m_btnClose.PtInRect(pt))
				lRes=HTCLOSE;
#ifdef DF_AUTO_HIDE_FEATURES
			else
			{
				if(m_btnPin.PtInRect(pt))
					lRes=HTPIN;
			}
#endif
		}
		return lRes;
	}

	void HotTrack(HWND hWnd,unsigned int nHitTest)
	{
		m_btnClose.Hot(hWnd,nHitTest==HTCLOSE);
#ifdef DF_AUTO_HIDE_FEATURES
		m_btnPin.Hot(hWnd,nHitTest==HTPIN);
#endif
	}

	bool Action(HWND hWnd,const CPoint& /*pt*/,unsigned int nHitTest)
	{
		bool res=false;
		switch(nHitTest)
		{
			case HTCLOSE:
				m_btnClose.Press(hWnd,true);
				res=true;
				break;
#ifdef DF_AUTO_HIDE_FEATURES
			case HTPIN:
				m_btnPin.Press(hWnd,true);
				res=true;
				break;
#endif
		}
		return res;
	}
	void ActionDone(HWND hWnd,unsigned int nHitTest,bool /*ok*/)
	{
		switch(nHitTest)
		{
			case HTCLOSE:
				m_btnClose.Press(hWnd,false);
				break;
#ifdef DF_AUTO_HIDE_FEATURES
			case HTPIN:
				m_btnPin.Press(hWnd,false);
				break;
#endif
		}
	}
protected:
#ifdef DF_AUTO_HIDE_FEATURES
	CPinButton		m_btnPin;
#endif
	CCloseButton	m_btnClose;
};


typedef CDockingWindowTraits<CVC6LikeCaption,
								WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW>
							 CVC6LikeTitleDockingWindowTraits;
}//namespace dockwins
#endif // WTL_DW_EXTDOCKINGWINDOW_H_INCLUDED_
