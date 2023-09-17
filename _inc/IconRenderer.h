
#pragma once

struct IRPolyPoint
{
	float x;
	float y;
};

struct IRPathPoint
{
	float x;
	float y;
	float tnx;
	float tny;
	float tpx;
	float tpy;
};

enum EGridItemFlags
{
	EGIFNoId = 0,
	EGIFIdMask = 0xffff,
	EGIFInteger = 0,
	EGIFMidPixel = 0x10000,
	EGIFArbitrary = 0x30000,
};

struct IRGridItem
{
	ULONG flags;
	float pos;
};

struct IRCanvas
{
	float x0;
	float y0;
	float x1;
	float y1;
	ULONG countX;
	ULONG countY;
	IRGridItem const* itemsX;
	IRGridItem const* itemsY;
};

struct IRPolygon
{
	ULONG count;
	IRPolyPoint const* points;
};

struct IRPath
{
	ULONG count;
	IRPathPoint const* points;
};

enum EIRMaterial
{
	EIRMFill,
	EIRMStroke,
	EIRMFillWithInternalOutline,
	EIRMOutlinedFill,
};

struct IRMaterial
{
	EIRMaterial type;
};

struct IRFill : public IRMaterial
{
	IRFill(DWORD color) : color(color) { type = EIRMFill; }
	DWORD color;
};

struct IRFillWithInternalOutline : public IRMaterial
{
	IRFillWithInternalOutline(DWORD inside, DWORD outline, float width) : inside(inside), outline(outline), width(width) { type = EIRMFillWithInternalOutline; }
	DWORD inside;
	DWORD outline;
	float width;
};

struct IRStroke : public IRMaterial
{
	IRStroke(DWORD color, float width) : color(color), width(width) { type = EIRMStroke; }
	DWORD color;
	float width;
};

struct IROutlinedFill : public IRMaterial
{
	IROutlinedFill(IRMaterial const* inside, IRMaterial const* outline, float widthRelative = 1.0f/40.0f, float widthAbsolute = 0.225f) : inside(inside), outline(outline), widthRelative(widthRelative), widthAbsolute(widthAbsolute) { type = EIRMOutlinedFill; }
	IRMaterial const* inside;
	IRMaterial const* outline;
	float widthRelative;
	float widthAbsolute;
};

struct IRLayer
{
	IRCanvas const* canvas;
	ULONG polygonCount;
	ULONG pathCount;
	IRPolygon const* polygons;
	IRPath const* paths;
	IRMaterial const* material;
};

struct IRTarget
{
	IRTarget() : relativeSize(1.0f), alignX(0.0f), alignY(0.0f) {}
	IRTarget(float relativeSize, float alignX = 0.0f, float alignY = 0.0f) : relativeSize(relativeSize), alignX(alignX), alignY(alignY) {}
	operator IRTarget const*() { return this; }
	float relativeSize;
	float alignX;
	float alignY;
};

MIDL_INTERFACE("DE95D718-E706-4FB3-A037-99315D5C2279")
IIconRenderer : public IUnknown
{
public:
    virtual bool STDMETHODCALLTYPE RenderLayer(ULONG sizeX, ULONG sizeY, ULONG stride, DWORD* buffer,
											   IRCanvas const* grid,
											   ULONG polygonCount, ULONG pathCount,
											   IRPolygon const* polygons, IRPath const* paths,
											   IRMaterial const* material,
											   IRTarget const* target = NULL) = 0;
    virtual bool STDMETHODCALLTYPE RenderLayers(ULONG sizeX, ULONG sizeY, ULONG stride, DWORD* buffer, ULONG layerCount, IRLayer const* layers, IRTarget const* target = NULL) = 0;

	virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG sizeX, ULONG sizeY, ULONG stride, DWORD const* buffer) = 0;
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size, DWORD const* buffer) = 0;
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size, ULONG layerCount, IRLayer const* layers) = 0;
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size, IRCanvas const* grid, ULONG polygonCount, IRPolygon const* polygons, IRMaterial const* material) = 0;
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size, IRCanvas const* grid, ULONG pathCount, IRPath const* paths, IRMaterial const* material) = 0;
    virtual HICON STDMETHODCALLTYPE CreateIcon(ULONG size,
											   IRCanvas const* grid,
											   ULONG polygonCount, ULONG pathCount,
											   IRPolygon const* polygons, IRPath const* paths,
											   IRMaterial const* material) = 0;
	virtual HCURSOR STDMETHODCALLTYPE CreateCursor(ULONG sizeX, ULONG sizeY, ULONG stride, DWORD const* buffer, ULONG hotSpotX, ULONG hotSpotY) = 0;
};

class DECLSPEC_UUID("563B37B5-964B-470D-8233-28710C22CE63")
IconRenderer;

enum EStockMaterial
{
	ESMMainCategoryMask = 0xff,
	ESMContrast = 0,
	ESMBackground = 1,
	ESMAltBackground = 2,
	ESMInterior = 3,
	ESMOutline = 4,
	ESMOutlineSoft = 5,
	ESMConfirm = 10,
	ESMCancel = 11,
	ESMHelp = 12,
	ESMCreate = 13,
	ESMManipulate = 14,
	ESMDelete = 15,
	ESMBrightLight = 16,
	ESMColor1 = 20,
	ESMColor2 = 21,
	ESMColor3 = 22,

	ESMSubCategoryMask = 0xf00,
	ESMScheme1 = 0x000,
	ESMScheme2 = 0x100,
	ESMScheme3 = 0x200,

	ESMScheme1Color1 = ESMColor1|ESMScheme1,
	ESMScheme1Color2 = ESMColor2|ESMScheme1,
	ESMScheme1Color3 = ESMColor3|ESMScheme1,
	ESMScheme2Color1 = ESMColor1|ESMScheme2,
	ESMScheme2Color2 = ESMColor2|ESMScheme2,
	ESMScheme2Color3 = ESMColor3|ESMScheme2,
	ESMScheme3Color1 = ESMColor1|ESMScheme3,
	ESMScheme3Color2 = ESMColor2|ESMScheme3,
	ESMScheme3Color3 = ESMColor3|ESMScheme3,
};

enum EStockIcon
{
	ESISymbolMask = 0xffff,
	ESIPlus = 0,
	ESIMinus,
	ESIConfirm,
	ESICancel,
	ESIHelp,
	ESICreate,
	ESIDuplicate,
	ESIDelete,
	ESIDocument,
	ESIFolder,
	ESIFloppy,
	ESIMagnifier,
	ESIModify,
	ESIMoveUp,
	ESIMoveDown,
	ESIHeart,
	ESIDirectionUp,
	ESIDirectionDown,
	ESIDirectionLeft,
	ESIDirectionRight,
	ESIStructure,
	ESIProperties,
	ESISettings,
	ESIPicture,

	ESIStyleMask = 0xf0000,
	ESINormal = 0,
	ESISimplified = 0x10000,
	ESIFancy = 0x20000,
	ESIDesert = 0x100000,
	ESILarge = 0x1000000,

	ESIFolderSimple = ESIFolder|ESISimplified,
	ESIFloppySimple = ESIFloppy|ESISimplified,
};

MIDL_INTERFACE("C5E88153-0061-4C1A-BBB0-CB6D80868ACC")
IStockIcons : public IUnknown
{
public:
	struct ILayerReceiver
	{
		virtual bool operator()(IRLayer const& layer, IRTarget const* target = NULL) = 0;
	};

public:
    virtual IRMaterial const* STDMETHODCALLTYPE GetMaterial(ULONG matId, bool forceSolid = false) = 0;
    virtual bool STDMETHODCALLTYPE GetLayers(ULONG iconId, ILayerReceiver& receiver, IRTarget const* target = NULL) = 0;
    virtual DWORD STDMETHODCALLTYPE GetSRGBColor(ULONG matId) = 0;
};

class DECLSPEC_UUID("CF4761A0-5595-4E05-9A47-90A81235265B")
StockIcons;

template<typename TContainer>
struct CIconLayersToVector : public IStockIcons::ILayerReceiver
{
	CIconLayersToVector(TContainer& container) : container(container) {}
	bool operator()(IRLayer const& layer, IRTarget const* target = NULL)
	{
		container.push_back(&layer);
		return true;
	}

private:
	TContainer& container;
};

struct CIconRendererReceiver : public IStockIcons::ILayerReceiver
{
	CIconRendererReceiver(ULONG sizeX, ULONG sizeY) : sizeX(sizeX), sizeY(sizeY)
	{
		buffer.Allocate(sizeX*sizeY);
		clear();
		RWCoCreateInstance(pIR, __uuidof(IconRenderer));
	}
	CIconRendererReceiver(ULONG size) : sizeX(size), sizeY(size)
	{
		buffer.Allocate(sizeX*sizeY);
		clear();
		RWCoCreateInstance(pIR, __uuidof(IconRenderer));
	}
	bool operator()(IRLayer const& layer, IRTarget const* target = NULL)
	{
		return pIR->RenderLayer(sizeX, sizeY, sizeX, buffer, layer.canvas, layer.polygonCount, layer.pathCount, layer.polygons, layer.paths, layer.material, target);
	}
	bool operator()(IRCanvas const* canvas, ULONG polygonCount, IRPolygon const* polygons, IRMaterial const* material, IRTarget const* target = NULL)
	{
		return pIR->RenderLayer(sizeX, sizeY, sizeX, buffer, canvas, polygonCount, 0, polygons, NULL, material, target);
	}
	bool operator()(IRCanvas const* canvas, ULONG pathCount, IRPath const* paths, IRMaterial const* material, IRTarget const* target = NULL)
	{
		return pIR->RenderLayer(sizeX, sizeY, sizeX, buffer, canvas, 0, pathCount, NULL, paths, material, target);
	}
	bool operator()(IRCanvas const* canvas, ULONG pointCount, IRPolyPoint const* points, IRMaterial const* material, IRTarget const* target = NULL)
	{
		IRPolygon poly = {pointCount, points};
		return pIR->RenderLayer(sizeX, sizeY, sizeX, buffer, canvas, 1, 0, &poly, NULL, material, target);
	}
	bool operator()(IRCanvas const* canvas, ULONG pointCount, IRPathPoint const* points, IRMaterial const* material, IRTarget const* target = NULL)
	{
		IRPath path = {pointCount, points};
		return pIR->RenderLayer(sizeX, sizeY, sizeX, buffer, canvas, 0, 1, NULL, &path, material, target);
	}
	HICON get() const
	{
		return pIR->CreateIcon(sizeX, sizeY, sizeX, buffer);
	}
	HICON get(RECT padding) const
	{
		if (padding.left == 0 && padding.right == 0 && padding.top == 0 && padding.bottom == 0)
			return get();
		SIZE sz = {sizeX+padding.left+padding.right, sizeY+padding.top+padding.bottom};
		CAutoVectorPtr<DWORD> buffer2(new DWORD[sz.cx*sz.cy]);
		ZeroMemory(buffer2.m_p, sz.cx*sz.cy*sizeof*buffer2.m_p);
		for (ULONG y = 0; y < sizeY; ++y)
			CopyMemory(buffer2.m_p+sz.cx*(sz.cy-1-y-padding.top)+padding.left, pixelRow(y), sizeX*sizeof*buffer2.m_p);
		
		return pIR->CreateIcon(sz.cx, sz.cy, sz.cx, buffer2);
	}
	HCURSOR getCursor(ULONG hotSpotX, ULONG hotSpotY) const
	{
		return pIR->CreateCursor(sizeX, sizeY, sizeX, buffer, hotSpotX, hotSpotY);
	}
	DWORD* pixelRow(int row) const
	{
		return buffer.m_p+(sizeY-1-row)*sizeX;
	}
	void clear()
	{
		ZeroMemory(buffer.m_p, sizeX * sizeY * sizeof * buffer.m_p);
	}

private:
	ULONG sizeX;
	ULONG sizeY;
	CComPtr<IIconRenderer> pIR;
	CAutoVectorPtr<DWORD> buffer;
};

#ifdef AUTOCANVASSUPPORT

#include <set>

struct IRAutoCanvas : public IRCanvas
{
	operator IRCanvas const*() { return this; }
	IRAutoCanvas(ULONG n, IRPolygon const* pp)
	{
		x0 = y0 = 1e6f;
		x1 = y1 = -1e6f;
		countX = countY = 0;
		itemsX = itemsY = NULL;

		std::set<float> gridX;
		std::set<float> gridY;
		for (ULONG j = 0; j < n; ++j)
		{
			IRPolyPoint const* p = pp[j].points;
			for (ULONG i = 0; i < pp[j].count; ++i)
			{
				if (x0 > p[i].x) x0 = p[i].x;
				if (y0 > p[i].y) y0 = p[i].y;
				if (x1 < p[i].x) x1 = p[i].x;
				if (y1 < p[i].y) y1 = p[i].y;
				char j = i == 0 ? n - 1 : i - 1;
				if (p[i].x == p[j].x) gridX.insert(p[i].x);
				if (p[i].y == p[j].y) gridY.insert(p[i].y);
			}
		}
		if (gridX.size()+gridY.size() > 0)
		{
			mem.Allocate(gridX.size()+gridY.size());
			itemsX = mem;
			itemsY = itemsX+gridX.size();
		}
		for (std::set<float>::const_iterator i = gridX.begin(); i != gridX.end(); ++i)
		{
			const_cast<IRGridItem*>(itemsX)[countX].flags = 0;
			const_cast<IRGridItem*>(itemsX)[countX].pos = *i;
			++countX;
		}
		for (std::set<float>::const_iterator i = gridY.begin(); i != gridY.end(); ++i)
		{
			const_cast<IRGridItem*>(itemsY)[countY].flags = 0;
			const_cast<IRGridItem*>(itemsY)[countY].pos = *i;
			++countY;
		}
	}

private:
	CAutoVectorPtr<IRGridItem> mem;
};

inline HICON IconFromPolygon(IStockIcons* pSI, IIconRenderer* pIR, int a_nVertices, IRPolyPoint const* a_pVertices, int a_nSize, bool a_bAutoScale, RECT const* padding = nullptr)
{
	CIconRendererReceiver cRenderer(a_nSize);
	IRPolygon const poly{ a_nVertices, a_pVertices };
	IRAutoCanvas canvas{ 1, &poly };
	if (!a_bAutoScale)
	{
		canvas.x0 = canvas.y0 = 0;
		canvas.x1 = canvas.y1 = 1;
	}
	cRenderer(&canvas, a_nVertices, a_pVertices, pSI->GetMaterial(ESMInterior));
	return padding ? cRenderer.get(*padding) : cRenderer.get();
}

inline HICON IconFromPolygon(IStockIcons* pSI, IIconRenderer* pIR, int a_nPolygons, IRPolygon const* a_pPolygons, int a_nSize, bool a_bAutoScale, RECT const* padding = nullptr)
{
	CIconRendererReceiver cRenderer(a_nSize);
	IRAutoCanvas canvas(a_nPolygons, a_pPolygons);
	if (!a_bAutoScale)
	{
		canvas.x0 = canvas.y0 = 0;
		canvas.x1 = canvas.y1 = 1;
	}
	cRenderer(&canvas, a_nPolygons, a_pPolygons, pSI->GetMaterial(ESMInterior));
	return padding ? cRenderer.get(*padding) : cRenderer.get();
}

inline HICON IconFromPolygon(IStockIcons* pSI, IIconRenderer* pIR, int a_nPolygons, IRPolygon const* a_pPolygons, int a_nSize, float tl, float br, RECT const* padding = nullptr)
{
	CIconRendererReceiver cRenderer(a_nSize);
	IRAutoCanvas canvas(a_nPolygons, a_pPolygons);
	canvas.x0 = canvas.y0 = tl;
	canvas.x1 = canvas.y1 = br;
	cRenderer(&canvas, a_nPolygons, a_pPolygons, pSI->GetMaterial(ESMInterior));
	return padding ? cRenderer.get(*padding) : cRenderer.get();
}


#endif