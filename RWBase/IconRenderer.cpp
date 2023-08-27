
#include "stdafx.h"

#include <IconRenderer.h>
#include "../clipper/clipper.hpp"
#undef min
#undef max
#include "../clipper/clipper.cpp"

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_gamma_lut.h>
#include <agg_curves.h>
#include <agg_span_allocator.h>
#include <agg_scanline_u.h>
#include <agg_rasterizer_compound_aa.h>

#include <GammaCorrection.h>


class ATL_NO_VTABLE CIconRenderer : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CIconRenderer, &__uuidof(IconRenderer)>,
	public IIconRenderer
{
public:
	CIconRenderer()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CIconRenderer)

BEGIN_COM_MAP(CIconRenderer)
	COM_INTERFACE_ENTRY(IIconRenderer)
END_COM_MAP()


	template<typename TSource>
    HICON CreateIconInternal(ULONG a_nSizeX, ULONG a_nSizeY, TSource a_tSource)
	{
		DWORD nXOR = a_nSizeY*a_nSizeX<<2;
		DWORD nAND = a_nSizeY*((((a_nSizeX+7)>>3)+3)&0xfffffffc);
		CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
		ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);
		BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
		pHead->biSize = sizeof(BITMAPINFOHEADER);
		pHead->biWidth = a_nSizeX;
		pHead->biHeight = a_nSizeY<<1;
		pHead->biPlanes = 1;
		pHead->biBitCount = 32;
		pHead->biCompression = BI_RGB;
		pHead->biSizeImage = nXOR+nAND;
		pHead->biXPelsPerMeter = 0;
		pHead->biYPelsPerMeter = 0;
		DWORD *pXOR = reinterpret_cast<DWORD*>(pIconRes+sizeof BITMAPINFOHEADER);

		a_tSource(pXOR, a_nSizeX);

		BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(a_nSizeY*a_nSizeX));
		int nANDLine = ((((a_nSizeX+7)>>3)+3)&0xfffffffc);
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			for (ULONG x = 0; x < a_nSizeX; ++x)
			{
				if (0 == (0xff000000&*pXOR))
				{
					pAND[x>>3] |= 0x80 >> (x&7);
				}
				++pXOR;
			}
			pAND += nANDLine;
		}
		return CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, a_nSizeX, a_nSizeY, LR_DEFAULTCOLOR);
	}

	struct SCopyBuffer
	{
		SCopyBuffer(ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, DWORD const* a_pData) : m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY), m_nStride(a_nStride), m_pData(a_pData) {}
		void operator()(DWORD* pDst, ULONG nStride)
		{
			for (ULONG y = 0; y < m_nSizeY; ++y)
				CopyMemory(pDst+y*nStride, m_pData+y*m_nStride, m_nSizeX*sizeof*pDst);
		}

	private:
		ULONG const m_nSizeX;
		ULONG const m_nSizeY;
		ULONG const m_nStride;
		DWORD const* const m_pData;
	};

    template<class ColorT, class Order> struct blender_rgba_gamma
	{
		typedef ColorT color_type;
		typedef Order order_type;
		typedef typename color_type::value_type value_type;
		typedef typename color_type::calc_type calc_type;
		enum base_scale_e 
		{ 
			base_shift = color_type::base_shift,
			base_mask  = color_type::base_mask
		};
		blender_rgba_gamma()
		{

		}

		//--------------------------------------------------------------------
		static AGG_INLINE void blend_pix(value_type* p, 
										 unsigned cr, unsigned cg, unsigned cb,
										 unsigned alpha)
		{
			calc_type a = p[Order::A];
			if (a == 0 || alpha >= 255)
			{
				p[Order::R] = cr;
				p[Order::G] = cg;
				p[Order::B] = cb;
				p[Order::A] = alpha;
				return;
			}
			unsigned nNewA = alpha*255 + (255-alpha)*a;
			if (nNewA)
			{
				ULONG const bA1 = (255-alpha)*a;
				ULONG const bA2 = alpha*255;
				p[Order::B] = pGT->InvGamma((pGT->m_aGamma[p[Order::B]]*bA1 + pGT->m_aGamma[cb]*bA2)/nNewA);
				p[Order::G] = pGT->InvGamma((pGT->m_aGamma[p[Order::G]]*bA1 + pGT->m_aGamma[cg]*bA2)/nNewA);
				p[Order::R] = pGT->InvGamma((pGT->m_aGamma[p[Order::R]]*bA1 + pGT->m_aGamma[cr]*bA2)/nNewA);
			}
			else
			{
				p[Order::R] = p[Order::G] = p[Order::B] = 0;
			}
			p[Order::A] = nNewA/255;
		}
		static AGG_INLINE void blend_pix(value_type* p, 
										 unsigned cr, unsigned cg, unsigned cb,
										 unsigned alpha, 
										 unsigned cover)
		{
			blend_pix(p, cr, cg, cb, (alpha*(cover+1))>>base_shift);
		}

		static CGammaTables const* pGT; // TODO: well...
	};

	struct SSection
	{
		float from;
		float base;
		float scale;
	};
	struct CGridTransformer
	{
		ClipperLib::cInt operator()(float s) const
		{
			std::vector<SSection>::const_reverse_iterator i = sections.rbegin();
			while (s < i->from)
			{
				++i;
				if (i == sections.rend())
				{
					--i;
					return 0.5+(1024.0*(i->base+(s-i->from)*defScale));
				}
			}
			return 0.5+(1024.0*(i->base+(s-i->from)*i->scale));
		}
		void fromGrid(ULONG count, IRGridItem const* items, float offset, float scale)
		{
			std::map<int, std::pair<int, float> > ids;
			for (ULONG i = 1; i < count; ++i)
			{
				int id = items[i].flags&EGIFIdMask;
				if (id == EGIFNoId)
					continue;
				std::map<int, std::pair<int, float> >::iterator j = ids.find(id);
				if (j != ids.end())
				{
					++j->second.first;
					j->second.second += items[i].pos-items[i-1].pos;
				}
				else
				{
					std::pair<int, float>& t = ids[id];
					t.first = 1;
					t.second = items[i].pos-items[i-1].pos;
				}
			}
			for (std::map<int, std::pair<int, float> >::iterator i = ids.begin(); i != ids.end(); )
			{
				std::map<int, std::pair<int, float> >::iterator ii = i;
				++i;
				if (ii->second.first == 1)
					ids.erase(ii);
			}
			if (ids.empty())
			{
				for (ULONG i = 0; i < count; ++i)
				{
					float orig = items[i].pos*scale+offset;
					float adjusted = items[i].flags&EGIFMidPixel ? (((orig-0.5f) + 12582912.0f) - 12582912.0f)+0.5f : (orig + 12582912.0f) - 12582912.0f;
					SSection item;
					item.from = items[i].pos;
					item.base = adjusted;
					item.scale = scale;
					sections.push_back(item);
				}
			}
			else
			{
				std::vector<SSection> sec;
				float bestErr = -1.0f;
				std::map<int, float> guide;
				for (int c = 0; c < 1<<ids.size(); ++c)
				{
					int b = 0;
					for (std::map<int, std::pair<int, float> >::iterator i = ids.begin(); i != ids.end(); ++i, ++b)
						guide[i->first] = (c>>b)&1 ? ceilf(i->second.second/i->second.first*scale) : floorf(i->second.second/i->second.first*scale);

					std::vector<SSection> sec;
					float err = 0.0f;
					float prevPos = 0.0f;
					for (ULONG i = 0; i < count; ++i)
					{
						float orig = items[i].pos*scale+offset;
						float adjusted = items[i].flags&EGIFMidPixel ? (((orig-0.5f) + 12582912.0f) - 12582912.0f)+0.5f : (orig + 12582912.0f) - 12582912.0f;
						int id = items[i].flags&EGIFIdMask;
						if (id != EGIFNoId)
						{
							adjusted = prevPos + guide[id];
						}
						prevPos = adjusted;
						SSection item;
						item.from = items[i].pos;
						item.base = adjusted;
						item.scale = scale;
						sec.push_back(item);
						err += (adjusted-orig)*(adjusted-orig);
					}
					if (sections.empty() || bestErr > err)
					{
						std::swap(sec, sections);
						bestErr = err;
					}
				}
			}
			for (ULONG i = 1; i < count; ++i)
			{
				if (sections[i].base == sections[i-1].base)
					sections[i-1].scale = 0.0f;
				else
					sections[i-1].scale = (sections[i].base-sections[i-1].base)/(sections[i].from-sections[i-1].from);
			}
			defScale = scale;
		}
		void addSection(float from, float base, float scale)
		{
			SSection section;
			section.from = from;
			section.base = base;
			section.scale = scale;
			sections.push_back(section);
			defScale = scale;
		}

		std::vector<SSection> sections;
		float defScale;
	};
	typedef blender_rgba_gamma<agg::rgba8, agg::order_bgra> blender_bgra32_gamma; //----blender_bgra32
	typedef agg::pixfmt_alpha_blend_rgba<blender_bgra32_gamma, agg::rendering_buffer, agg::pixel32_type> pixfmt_bgra32_gamma; //----pixfmt_bgra32
	struct CRenderLayers
	{
		CRenderLayers(ULONG sizeX, ULONG sizeY, ULONG layerCount, IRLayer const* layers, IRTarget const* target = NULL) : sizeX(sizeX), sizeY(sizeY), layerCount(layerCount), layers(layers), target(target) {}
		void operator()(DWORD* pDst, ULONG nStride)
		{
			CComPtr<IGammaTableCache> pGT;
			RWCoCreateInstance(pGT, __uuidof(GammaTableCache));

			agg::rendering_buffer rbuf;
			rbuf.attach(reinterpret_cast<agg::int8u*>(pDst), sizeX, sizeY, -nStride*4); // Use negative stride in order to keep Y-axis consistent with WinGDI, i.e., going down.
			// Pixel format and basic primitives renderer
			pixfmt_bgra32_gamma pixf(rbuf);
			blender_bgra32_gamma::pGT = pGT->GetSRGBTable();
			agg::renderer_base<pixfmt_bgra32_gamma> renb(pixf);
			//renb.clear(agg::rgba8(0, 0, 0, 0)); // must be initialized by the caller
			// Scanline renderer for solid filling.
			agg::renderer_scanline_aa_solid<agg::renderer_base<pixfmt_bgra32_gamma> > ren(renb);
			// Rasterizer & scanline
			agg::rasterizer_scanline_aa<> ras;
			agg::scanline_p8 sl;

			IRTarget t;
			if (target) t = *target;
			IRLayer const* const le = layers+layerCount;
			for (IRLayer const* l = layers; l != le; ++l)
			{
				CGridTransformer transX;
				CGridTransformer transY;
				double scale;
				{
					double scaleX = sizeX*t.relativeSize/(l->canvas->x1-l->canvas->x0);
					double scaleY = sizeY*t.relativeSize/(l->canvas->y1-l->canvas->y0);
					scale = min(scaleX, scaleY);
					
					double offsetX = (t.alignX+1.0f)*0.5f*(sizeX-(l->canvas->x1-l->canvas->x0)*scale)-l->canvas->x0*scale;
					double offsetY = (t.alignY+1.0f)*0.5f*(sizeY-(l->canvas->y1-l->canvas->y0)*scale)-l->canvas->y0*scale;

					if (l->canvas->countX && l->canvas->itemsX)
						transX.fromGrid(l->canvas->countX, l->canvas->itemsX, offsetX, scale);
					else
						transX.addSection(0, offsetX, scale);
					if (l->canvas->countY && l->canvas->itemsY)
						transY.fromGrid(l->canvas->countY, l->canvas->itemsY, offsetY, scale);
					else
						transY.addSection(0, offsetY, scale);
				}

				ClipperLib::Paths paths;
				paths.reserve(l->polygonCount+l->pathCount);
				bool autoClose = l->material->type != EIRMStroke;
				IRPolygon const* const ple = l->polygons+l->polygonCount;
				for (IRPolygon const* pl = l->polygons; pl != ple; ++pl)
				{
					if (pl->count < ULONG(autoClose ? 3 : 2))
						continue;
					paths.resize(paths.size()+1);
					ClipperLib::Path& path = paths[paths.size()-1];
					IRPolyPoint const* const pe = pl->points+pl->count;
					for (IRPolyPoint const* p = pl->points; p != pe; ++p)
					{
						ClipperLib::IntPoint ipt(transX(p->x), transY(p->y));
						add_point(path, ipt);
					}
					if (autoClose && path.size() > 2 && *path.begin() == *path.rbegin())
						path.resize(path.size()-1);
				}
				IRPath const* const pae = l->paths+l->pathCount;
				for (IRPath const* pa = l->paths; pa != pae; ++pa)
				{
					if (pa->count < 2)
						continue;
					paths.resize(paths.size()+1);
					ClipperLib::Path& path = paths[paths.size()-1];
					IRPathPoint const* const pe = autoClose ? pa->points+pa->count : pa->points+pa->count-1;
					for (IRPathPoint const* p = pa->points; p != pe; ++p)
					{
						IRPathPoint const* pn = p+1 == pa->points+pa->count ? pa->points : p+1;

						if (p->tnx == 0 && p->tny == 0 && pn->tpx == 0 && pn->tpy == 0)
						{
							ClipperLib::IntPoint ipt1(transX(p->x), transY(p->y));
							add_point(path, ipt1);
							ClipperLib::IntPoint ipt2(transX(pn->x), transY(pn->y));
							add_point(path, ipt2);
						}
						else
						{
							agg::curve4 c(p->x, p->y, p->x+p->tnx, p->y+p->tny, pn->x+pn->tpx, pn->y+pn->tpy, pn->x, pn->y);
							double x = 0.0;
							double y = 0.0;
							while (!agg::is_stop(c.vertex(&x, &y)))
							{
								ClipperLib::IntPoint ipt(transX(x), transY(y));
								add_point(path, ipt);
							}
						}
					}
					if (autoClose && path.size() > 2 && *path.begin() == *path.rbegin())
						path.resize(path.size()-1);
				}

				if (l->material->type == EIRMFill)
				{
					IRFill const* fill = static_cast<IRFill const*>(l->material);

					agg::path_storage path;
					to_agg_path(paths, path);

					ren.color(agg::argb8_packed(fill->color));
					ras.add_path(path);
					ras.filling_rule(agg::fill_even_odd);
					agg::render_scanlines(ras, sl, ren);
				}
				else if (l->material->type == EIRMStroke)
				{
					IRStroke const* mat = static_cast<IRStroke const*>(l->material);

					agg::path_storage path;
					to_agg_stroke(paths, path);

					agg::conv_stroke<agg::path_storage> stroke(path);
					stroke.line_join(agg::miter_join);
					stroke.width(max(sizeX, sizeY)*mat->width);
					ren.color(agg::argb8_packed(mat->color));
					ras.add_path(stroke);
					agg::render_scanlines(ras, sl, ren);
				}
				else if (l->material->type == EIRMOutlinedFill)
				{
					IROutlinedFill const* mat = static_cast<IROutlinedFill const*>(l->material);
					IRFill const* fill = mat->inside->type == EIRMFill ? static_cast<IRFill const*>(mat->inside) : NULL;
					IRFill const* outline = mat->outline->type == EIRMFill ? static_cast<IRFill const*>(mat->outline) : NULL;

					ClipperLib::Paths outer;
					ClipperLib::SimplifyPolygons(paths, outer, ClipperLib::pftEvenOdd);

					ClipperLib::Paths inner;
					{
						ClipperLib::JoinType jt = ClipperLib::jtRound;//a_eJoinType == EOJTMiter ? ClipperLib::jtMiter : (a_eJoinType == EOJTBevel ? ClipperLib::jtSquare : ClipperLib::jtRound);
						ClipperLib::ClipperOffset cOff(4.0f);
						cOff.AddPaths(outer, jt, ClipperLib::etClosedPolygon);
						cOff.Execute(inner, -1024.0*(max(sizeX, sizeY)*mat->widthRelative+mat->widthAbsolute));
					}

					if (inner.empty())
					{
						agg::path_storage path;
						to_agg_path(outer, path);

						ren.color(agg::argb8_packed(outline->color));
						ras.add_path(path);
						ras.filling_rule(agg::fill_even_odd);
						agg::render_scanlines(ras, sl, ren);
					}
					else
					{
						agg::scanline_u8 sl;

						agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> rasc;
						rasc.styles(1, -1);
						agg::path_storage path;
						to_agg_path(outer, path);
						to_agg_path_rev(inner, path);
						rasc.add_path(path);
						//for (std::vector<std::vector<TPixelCoords> >::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
						//	rasc.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSource(*i));
						//for (std::vector<std::vector<TPixelCoords> >::iterator i = cInner.begin(); i != cInner.end(); ++i)
						//	rasc.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSourceHole(*i));
						rasc.styles(0, -1);
						path.remove_all();
						to_agg_path(inner, path);
						rasc.add_path(path);
						//for (std::vector<std::vector<TPixelCoords> >::iterator i = cInner.begin(); i != cInner.end(); ++i)
						//	rasc.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSource(*i));

						agg::span_allocator<agg::rgba8> alloc;
						compound_layered_add layerBlender;
						layerBlender.pGT = pGT->GetSRGBTable();
						style_handler_outlined_fill styleHandler;
						styleHandler.inside = fill;
						styleHandler.outline = outline;
						agg::render_scanlines_compound_layered(rasc, sl, renb/*_pre*/, alloc, styleHandler, layerBlender);
					}

					agg::path_storage outside;
					to_agg_path(paths, outside);

				}
				else if (l->material->type == EIRMFillWithInternalOutline)
				{
					IRFillWithInternalOutline const* mat = static_cast<IRFillWithInternalOutline const*>(l->material);

					ClipperLib::Paths outer;
					ClipperLib::SimplifyPolygons(paths, outer, ClipperLib::pftEvenOdd);

					ClipperLib::Paths inner;
					{
						ClipperLib::JoinType jt = ClipperLib::jtRound;//a_eJoinType == EOJTMiter ? ClipperLib::jtMiter : (a_eJoinType == EOJTBevel ? ClipperLib::jtSquare : ClipperLib::jtRound);
						ClipperLib::ClipperOffset cOff(4.0f);
						cOff.AddPaths(outer, jt, ClipperLib::etClosedPolygon);
						cOff.Execute(inner, -1024.0*mat->width*scale);
					}

					if (inner.empty())
					{
						agg::path_storage path;
						to_agg_path(outer, path);

						ren.color(agg::argb8_packed(mat->outline));
						ras.add_path(path);
						ras.filling_rule(agg::fill_even_odd);
						agg::render_scanlines(ras, sl, ren);
					}
					else
					{
						agg::scanline_u8 sl;

						agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> rasc;
						rasc.styles(1, -1);
						agg::path_storage path;
						to_agg_path(outer, path);
						to_agg_path_rev(inner, path);
						rasc.add_path(path);
						//for (std::vector<std::vector<TPixelCoords> >::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
						//	rasc.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSource(*i));
						//for (std::vector<std::vector<TPixelCoords> >::iterator i = cInner.begin(); i != cInner.end(); ++i)
						//	rasc.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSourceHole(*i));
						rasc.styles(0, -1);
						path.remove_all();
						to_agg_path(inner, path);
						rasc.add_path(path);
						//for (std::vector<std::vector<TPixelCoords> >::iterator i = cInner.begin(); i != cInner.end(); ++i)
						//	rasc.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSource(*i));

						agg::span_allocator<agg::rgba8> alloc;
						compound_layered_add layerBlender;
						layerBlender.pGT = pGT->GetSRGBTable();
						style_handler_fill_with_outline styleHandler;
						styleHandler.mat = mat;
						agg::render_scanlines_compound_layered(rasc, sl, renb/*_pre*/, alloc, styleHandler, layerBlender);
					}

					agg::path_storage outside;
					to_agg_path(paths, outside);

				}

				//if (p->outline.a != 0)
				//{
				//	agg::conv_stroke<agg::path_storage> stroke(path);
				//	stroke.line_join(agg::miter_join);
				//	stroke.width(1.0f);
				//	ren.color(p->outline);
				//	ras.add_path(stroke);
				//	agg::render_scanlines(ras, sl, ren);
				//}
			}

			//pixf.demultiply();

			//// gamma correction
			//agg::gamma_lut<agg::int8u, agg::int8u, 8, 8> gp(2.2);
			//pixf.apply_gamma_inv(gp);

		}

		struct compound_layered_add
		{
			void inline operator()(agg::rgba8* t, unsigned char* this_cover, agg::rgba8 const& c, unsigned cover) const
			{
				if (*this_cover == 0 || cover == 255)
				{
					*t = c;
					*this_cover = cover;
					return;
				}
				unsigned const total_cover = *this_cover + cover;
				ULONG const ac = ULONG(t->a)**this_cover;
				ULONG const cac = ULONG(c.a)*cover;
				t->a = (ac + cac + (total_cover>>1))/total_cover;
				ULONG const div = ac + cac;
				if (div)
				{
					if (pGT)
					{
						t->r = pGT->InvGamma((pGT->m_aGamma[t->r]*ac + pGT->m_aGamma[c.r]*cac)/div); // TODO: +(div>>1) ?
						t->g = pGT->InvGamma((pGT->m_aGamma[t->g]*ac + pGT->m_aGamma[c.g]*cac)/div);
						t->b = pGT->InvGamma((pGT->m_aGamma[t->b]*ac + pGT->m_aGamma[c.b]*cac)/div);
					}
					else
					{
						t->r = (t->r*ac + c.r*cac)/div; // TODO: +(div>>1) ?
						t->g = (t->g*ac + c.g*cac)/div;
						t->b = (t->b*ac + c.b*cac)/div;
					}
				}
				else
				{
					t->r = t->g = t->b = 0;
				}
				*this_cover = total_cover;
			}

			CGammaTables const* pGT;
		};
		struct style_handler_outlined_fill
		{
			bool is_solid(unsigned a_nLayer)
			{
				return true;
			}
			agg::rgba8 color(unsigned a_nLayer) const
			{
				if (a_nLayer)
				{
					return agg::argb8_packed(outline->color);
				}
				else
				{
					return agg::argb8_packed(inside->color);
				}
			}
			void generate_span(agg::rgba8* span, int x, int y, unsigned len, unsigned style)
			{
			}
			void generate(agg::rgba8* span, int x, int y, unsigned len)
			{
			}
			void prepare()
			{
			}

			IRFill const* inside;
			IRFill const* outline;
		};
		struct style_handler_fill_with_outline
		{
			bool is_solid(unsigned a_nLayer)
			{
				return true;
			}
			agg::rgba8 color(unsigned a_nLayer) const
			{
				if (a_nLayer)
				{
					return agg::argb8_packed(mat->outline);
				}
				else
				{
					return agg::argb8_packed(mat->inside);
				}
			}
			void generate_span(agg::rgba8* span, int x, int y, unsigned len, unsigned style)
			{
			}
			void generate(agg::rgba8* span, int x, int y, unsigned len)
			{
			}
			void prepare()
			{
			}

			IRFillWithInternalOutline const* mat;
		};

	private:
		static void add_point(ClipperLib::Path& path, ClipperLib::IntPoint point)
		{
			if (path.empty() || point != *path.rbegin())
				path.push_back(point);
		}
		static void to_agg_path(ClipperLib::Paths const& paths, agg::path_storage& path)
		{
			for (ClipperLib::Paths::const_iterator i0 = paths.begin(); i0 != paths.end(); ++i0)
			{
				if (i0->size() < 3)
					continue;
				ClipperLib::Path::const_iterator i1 = i0->begin();
				path.move_to(i1->X/1024.0, i1->Y/1024.0);
				++i1;
				for (ClipperLib::Path::const_iterator i1e = i0->end(); i1 != i1e; ++i1)
				{
					path.line_to(i1->X/1024.0, i1->Y/1024.0);
				}
				path.close_polygon();
			}
		}
		static void to_agg_path_rev(ClipperLib::Paths const& paths, agg::path_storage& path)
		{
			for (ClipperLib::Paths::const_iterator i0 = paths.begin(); i0 != paths.end(); ++i0)
			{
				if (i0->size() < 3)
					continue;
				ClipperLib::Path::const_reverse_iterator i1 = i0->rbegin();
				path.move_to(i1->X/1024.0, i1->Y/1024.0);
				++i1;
				for (ClipperLib::Path::const_reverse_iterator i1e = i0->rend(); i1 != i1e; ++i1)
				{
					path.line_to(i1->X/1024.0, i1->Y/1024.0);
				}
				path.close_polygon();
			}
		}
		static void to_agg_stroke(ClipperLib::Paths const& paths, agg::path_storage& path)
		{
			for (ClipperLib::Paths::const_iterator i0 = paths.begin(); i0 != paths.end(); ++i0)
			{
				if (i0->size() < 2)
					continue;
				ClipperLib::Path::const_iterator i1 = i0->begin();
				path.move_to(i1->X/1024.0, i1->Y/1024.0);
				++i1;
				for (ClipperLib::Path::const_iterator i1e = i0->end()-1; i1 != i1e; ++i1)
				{
					path.line_to(i1->X/1024.0, i1->Y/1024.0);
				}
				if (i1->X == i0->begin()->X && i1->Y == i0->begin()->Y)
				{
					path.close_polygon();
				}
				else
				{
					path.line_to(i1->X/1024.0, i1->Y/1024.0);
				}
			}
		}

	private:
		ULONG const sizeX;
		ULONG const sizeY;
		ULONG const layerCount;
		IRLayer const* const layers;
		IRTarget const* const target;
	};

	// IIconRenderer methods
public:
    virtual bool STDMETHODCALLTYPE RenderLayer(ULONG sizeX, ULONG sizeY, ULONG stride, DWORD* buffer, IRCanvas const* grid, ULONG polygonCount, ULONG pathCount, IRPolygon const* polygons, IRPath const* paths, IRMaterial const* material, IRTarget const* target)
	{
		IRLayer layer = {grid, polygonCount, pathCount, polygons, paths, material};
		return RenderLayers(sizeX, sizeY, stride, buffer, 1, &layer, target);
	}
    virtual bool STDMETHODCALLTYPE RenderLayers(ULONG sizeX, ULONG sizeY, ULONG stride, DWORD* buffer, ULONG layerCount, IRLayer const* layers, IRTarget const* target)
	{
		try
		{
			CRenderLayers renderer(sizeX, sizeY, layerCount, layers, target);
			renderer(buffer, stride);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size, DWORD const* buffer)
	{
		if (size > 256)
			return NULL;
		try
		{
			return CreateIconInternal(size, size, SCopyBuffer(size, size, size, buffer));
		}
		catch (...)
		{
			return NULL;
		}
	}
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG sizeX, ULONG sizeY, ULONG stride, DWORD const* buffer)
	{
		if (sizeX > 256 || sizeY > 256)
			return NULL;
		return CreateIconInternal(sizeX, sizeY, SCopyBuffer(sizeX, sizeY, stride, buffer));
	}
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size, ULONG layerCount, IRLayer const* layers)
	{
		if (size > 256)
			return NULL;
		try
		{
			return CreateIconInternal(size, size, CRenderLayers(size, size, layerCount, layers));
		}
		catch (...)
		{
			return NULL;
		}
	}
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size, IRCanvas const* grid, ULONG polygonCount, IRPolygon const* polygons, IRMaterial const* material)
	{
		IRLayer layer = {grid, polygonCount, 0, polygons, NULL, material};
		return CreateIcon(size, 1, &layer);
	}
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size, IRCanvas const* grid, ULONG pathCount, IRPath const* paths, IRMaterial const* material)
	{
		IRLayer layer = {grid, 0, pathCount, NULL, paths, material};
		return CreateIcon(size, 1, &layer);
	}
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size, IRCanvas const* grid, ULONG polygonCount, ULONG pathCount, IRPolygon const* polygons, IRPath const* paths, IRMaterial const* material)
	{
		IRLayer layer = {grid, polygonCount, pathCount, polygons, paths, material};
		return CreateIcon(size, 1, &layer);
	}
	virtual HCURSOR STDMETHODCALLTYPE CreateCursor(ULONG a_nSizeX, ULONG a_nSizeY, ULONG stride, DWORD const* buffer, ULONG hotSpotX, ULONG hotSpotY)
	{
		DWORD nXOR = a_nSizeY*a_nSizeX<<2;
		DWORD nAND = a_nSizeY*((((a_nSizeX+7)>>3)+3)&0xfffffffc);
		CAutoVectorPtr<BYTE> pIconRes(new BYTE[4+nXOR+nAND+sizeof BITMAPINFOHEADER]);
		ZeroMemory(pIconRes.m_p, 4+nXOR+nAND+sizeof BITMAPINFOHEADER);
		pIconRes[0] = static_cast<BYTE>(hotSpotX&0xff);
		pIconRes[1] = static_cast<BYTE>((hotSpotX>>8)&0xff);
		pIconRes[2] = static_cast<BYTE>(hotSpotY&0xff);
		pIconRes[3] = static_cast<BYTE>((hotSpotY>>8)&0xff);
		BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p+4);
		pHead->biSize = sizeof(BITMAPINFOHEADER);
		pHead->biWidth = a_nSizeX;
		pHead->biHeight = a_nSizeY<<1;
		pHead->biPlanes = 1;
		pHead->biBitCount = 32;
		pHead->biCompression = BI_RGB;
		pHead->biSizeImage = nXOR+nAND;
		pHead->biXPelsPerMeter = 0;
		pHead->biYPelsPerMeter = 0;
		DWORD *pXOR = reinterpret_cast<DWORD*>(pHead+1);

		SCopyBuffer tSource(a_nSizeX, a_nSizeY, stride, buffer);
		tSource(pXOR, stride);

		BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(a_nSizeY*a_nSizeX));
		int nANDLine = ((((a_nSizeX+7)>>3)+3)&0xfffffffc);
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			for (ULONG x = 0; x < a_nSizeX; ++x)
			{
				if (0 == (0xff000000&*pXOR))
				{
					pAND[x>>3] |= 0x80 >> (x&7);
				}
				++pXOR;
			}
			pAND += nANDLine;
		}
		return reinterpret_cast<HCURSOR>(CreateIconFromResourceEx(pIconRes.m_p, 4+nXOR+nAND+sizeof BITMAPINFOHEADER, FALSE, 0x00030000, a_nSizeX, a_nSizeY, LR_DEFAULTCOLOR));
	}

};

CGammaTables const* CIconRenderer::blender_bgra32_gamma::pGT = NULL; // TODO: well...

OBJECT_ENTRY_AUTO(__uuidof(IconRenderer), CIconRenderer)
