// DesignerFrameStockIcons.cpp : Implementation of CDesignerFrameStockIcons

#include "stdafx.h"
#include "DesignerFrameStockIcons.h"
#include <IconRenderer.h>
#include <math.h>


// CDesignerFrameStockIcons

typedef HICON (*pfnGetIcon)(ULONG size);

HICON GetIconWrench(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIModify, cRenderer);
	return cRenderer.get();
}

HICON GetIconCreate(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESICreate, cRenderer, IRTarget(0.8f));
	return cRenderer.get();
}

HICON GetIconDuplicate(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIDuplicate, cRenderer, IRTarget(0.8f));
	return cRenderer.get();
}

HICON GetIconDelete(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIDelete, cRenderer, IRTarget(0.8f));
	return cRenderer.get();
}

HICON GetIconStructure(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIStructure, cRenderer);
	return cRenderer.get();
}

HICON GetIconMagnifier(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIMagnifier|ESIFancy, cRenderer);
	return cRenderer.get();
}

HICON GetIconSettings(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESISettings, cRenderer);
	return cRenderer.get();
}

HICON GetIconOperation(ULONG size, EStockMaterial material)//DWORD fill)
{
	// cogwheel
	static int const extra = 8;
	static float const r1 = 0.11f;
	static float const r2 = 0.37f;
	static float const r3 = 0.5f;
	IRPolyPoint aInner[extra*4];
	IRPolyPoint aOuter[10];
	IRPolyPoint* p = aInner;
	for (int i = 0; i < extra; ++i)
	{
		float const a1 = ((i+0.125f)/extra)*2.0f*3.14159265359f;
		float const a2 = ((i+0.325f)/extra)*2.0f*3.14159265359f;
		float const a3 = ((i+0.675f)/extra)*2.0f*3.14159265359f;
		float const a4 = ((i+0.875f)/extra)*2.0f*3.14159265359f;
		p->x = 0.5f + cosf(a1)*r2;
		p->y = 0.5f + sinf(a1)*r2;
		++p;
		p->x = 0.5f + cosf(a2)*r3;
		p->y = 0.5f + sinf(a2)*r3;
		++p;
		p->x = 0.5f + cosf(a3)*r3;
		p->y = 0.5f + sinf(a3)*r3;
		++p;
		p->x = 0.5f + cosf(a4)*r2;
		p->y = 0.5f + sinf(a4)*r2;
		++p;
	}
	p = aOuter;
	for (int i = 0; i < 10; ++i)
	{
		float const a1 = (i*0.2f+0.05f)*3.14159265359f;
		p->x = 0.5f + cosf(a1)*r1;
		p->y = 0.5f + sinf(a1)*r1;
		++p;
	}
	IRPolygon const aCogwheel[] = { {itemsof(aInner), aInner}, {itemsof(aOuter), aOuter} };
	//IRFill matCogwheelFill(fill);
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	//IROutlinedFill matCogwheel(&matCogwheelFill, pSI->GetMaterial(ESMContrast));
	IRCanvas canvas = {0, 0, 1, 1, 0, 0, NULL, NULL};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(aCogwheel), aCogwheel, pSI->GetMaterial(material));//&matCogwheel);
	return cRenderer.get();
}

HICON GetIconOperationBlue(ULONG size) { return GetIconOperation(size, ESMScheme2Color1/*0xff98bbe0*/); }
HICON GetIconOperationBeige(ULONG size) { return GetIconOperation(size, ESMScheme2Color2/*0xfff2d3a8*/); }
HICON GetIconOperationGreen(ULONG size) { return GetIconOperation(size, ESMScheme2Color3/*0xffb6d28d*/); }

HICON GetIconUser(ULONG size, EStockMaterial material)
{
	//DWORD fill = 0xfff2d3a8;

	// user
	static IRPolyPoint const neck[] =
	{
		{86, 185}, {170, 185}, {156, 120}, {100, 120},
	};
	static IRPathPoint const face[] =
	{
		{183, 84, 0, -37.5554, 0, 37.5554},
		{128, 16, -30.3757, 0, 30.3757, 0},
		{73, 84, 0, 37.5554, 0, -37.5554},
		{128, 152, 30.3757, 0, -30.3757, 0},
	};
	static IRPathPoint const hair[] =
	{
		{73, 92, -9, -13, 3, -31},
		{82, 20, 17.6701, -22.4893, -22, 28},
		{171, 19, 24, 24, -23.5478, -23.5478},
		{184, 92, -3, -9, 7, -16},
		{167, 53, -10, 9, 15, 12},
		{112, 32, -14, 3, 18, 24},
	};
	static IRPathPoint const tshirt[] =
	{
		{21, 179, 8, -15, -8, 15},
		{90, 160, 14, 14, -10, -10},
		{166, 160, 10, -10, -14, 14},
		{235, 179, 8, 15, -8, -15},
		{246, 233, 0, 0, 0, 0},
		{10, 233, 0, 0, 0, 0},
	};
	static IRFill const faceFillMat(0xffffcda1);
	static IRFill const hairFillMat(0xff985941);
	//IRFill const tshirtFillMat(fill);
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IROutlinedFill const faceMat(&faceFillMat, pSI->GetMaterial(ESMContrast));
	IROutlinedFill const hairMat(&hairFillMat, pSI->GetMaterial(ESMContrast));
	//IROutlinedFill const tshirtMat(&tshirtFillMat, pSI->GetMaterial(ESMContrast));
	CIconRendererReceiver cRenderer(size);

	IRGridItem gridY = {0, 233};
	IRCanvas canvas = {10, 2, 246, 233, 0, 1, NULL, &gridY};

	cRenderer(&canvas, itemsof(neck), neck, &faceMat);
	cRenderer(&canvas, itemsof(face), face, &faceMat);
	cRenderer(&canvas, itemsof(hair), hair, &hairMat);
	cRenderer(&canvas, itemsof(tshirt), tshirt, pSI->GetMaterial(material));//&tshirtMat);

	return cRenderer.get();
}

HICON GetIconUserBlue(ULONG size) { return GetIconUser(size, ESMScheme2Color1/*0xff98bbe0*/); }
HICON GetIconUserBeige(ULONG size) { return GetIconUser(size, ESMScheme2Color2/*0xfff2d3a8*/); }
HICON GetIconUserGreen(ULONG size) { return GetIconUser(size, ESMScheme2Color3/*0xffb6d28d*/); }

HICON GetIconLibrary(ULONG size)
{
	static IRPathPoint const cover1[] =
	{
		{94, 123, 0, 0, 0, 0},
		{202, 123, 16.5685, 0, 0, 0},
		{232, 153, 0, 7.53273, 0, -16.5685},
		{224.639, 172.686, 0, 0, 4.58511, -5.26848},
		{158.375, 238.5, 0, 0, 0, 0},
		{56, 195, 0, 0, 0, 0},
		{6, 195, 0, 0, 0, 0},
		{6, 187, 0, 0, 0, 0},
	};
	static IRPathPoint const pages1[] =
	{
		{6, 239, 12.1503, 0, 0, 0},
		{28, 217, 0, -12.1503, 0, 12.1503},
		{6, 195, 0, 0, 12.1503, 0},
		{6, 187, 0, 0, 0, 0},
		{136, 187, 17.6731, 0, 0, 0},
		{168, 217, 0, 16.5685, 0, -16.5685},
		{136, 247, 0, 0, 17.6731, 0},
		{6, 247, 0, 0, 0, 0},
	};
	static IRPathPoint const cover2[] =
	{
		{118, 62, 0, 0, 0, 0},
		{226, 62, 16.5685, 0, 0, 0},
		{256, 92, 0, 7.53273, 0, -16.5685},
		{248.639, 111.686, 0, 0, 4.58511, -5.26848},
		{182.375, 177.5, 0, 0, 0, 0},
		{80, 134, 0, 0, 0, 0},
		{30, 134, 0, 0, 0, 0},
		{30, 126, 0, 0, 0, 0},
	};
	static IRPathPoint const pages2[] =
	{
		{30, 178, 12.1503, 0, 0, 0},
		{52, 156, 0, -12.1503, 0, 12.1503},
		{30, 134, 0, 0, 12.1503, 0},
		{30, 126, 0, 0, 0, 0},
		{160, 126, 17.6731, 0, 0, 0},
		{192, 156, 0, 16.5685, 0, -16.5685},
		{160, 186, 0, 0, 17.6731, 0},
		{30, 186, 0, 0, 0, 0},
	};
	static IRPathPoint const cover3[] =
	{
		{88, 12, 0, 0, 0, 0},
		{196, 12, 16.5685, 0, 0, 0},
		{226, 42, 0, 7.53273, 0, -16.5685},
		{218.639, 61.6855, 0, 0, 4.58511, -5.26848},
		{152.375, 127.5, 0, 0, 0, 0},
		{50, 84, 0, 0, 0, 0},
		{0, 84, 0, 0, 0, 0},
		{0, 76, 0, 0, 0, 0},
	};
	static IRPathPoint const pages3[] =
	{
		{0, 128, 12.1503, 0, 0, 0},
		{22, 106, 0, -12.1503, 0, 12.1503},
		{0, 84, 0, 0, 12.1503, 0},
		{0, 76, 0, 0, 0, 0},
		{130, 76, 17.6731, 0, 0, 0},
		{162, 106, 0, 16.5685, 0, -16.5685},
		{130, 136, 0, 0, 17.6731, 0},
		{0, 136, 0, 0, 0, 0},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(cover1), cover1, pSI->GetMaterial(ESMManipulate));
	cRenderer(&canvas, itemsof(pages1), pages1, pSI->GetMaterial(ESMBackground));
	cRenderer(&canvas, itemsof(cover2), cover2, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvas, itemsof(pages2), pages2, pSI->GetMaterial(ESMBackground));
	cRenderer(&canvas, itemsof(cover3), cover3, pSI->GetMaterial(ESMManipulate));
	cRenderer(&canvas, itemsof(pages3), pages3, pSI->GetMaterial(ESMBackground));
	return cRenderer.get();
}

HICON GetIconMoveUp(ULONG size)
{
	static IRPolyPoint const shape[] =
	{
		{186, 112}, {154, 112}, {154, 216}, {102, 216}, {102, 112}, {70, 112}, {128, 40},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRGridItem gridX[] = { {0, 102}, {0, 154} };
	IRGridItem gridY[] = { {0, 112}, {0, 216} };
	IRCanvas canvas = {40, 40, 216, 216, itemsof(gridX), itemsof(gridY), gridX, gridY};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMManipulate), IRTarget(0.8f));
	return cRenderer.get();
}

HICON GetIconMoveDown(ULONG size)
{
	static IRPolyPoint const shape[] =
	{
		{154, 144}, {186, 144}, {128, 216}, {70, 144}, {102, 144}, {102, 40}, {154, 40},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRGridItem gridX[] = { {0, 102}, {0, 154} };
	IRGridItem gridY[] = { {0, 40}, {0, 144} };
	IRCanvas canvas = {40, 40, 216, 216, itemsof(gridX), itemsof(gridY), gridX, gridY};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMManipulate), IRTarget(0.8f));
	return cRenderer.get();
}

HICON GetIconMoveLeft(ULONG size)
{
	static IRPolyPoint const shape[] =
	{
		{112, 102}, {216, 102}, {216, 154}, {112, 154}, {112, 186}, {40, 128}, {112, 70},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRGridItem gridX[] = { {0, 112}, {0, 216} };
	IRGridItem gridY[] = { {0, 102}, {0, 154} };
	IRCanvas canvas = {40, 40, 216, 216, itemsof(gridX), itemsof(gridY), gridX, gridY};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMManipulate), IRTarget(0.8f));
	return cRenderer.get();
}

HICON GetIconMoveRight(ULONG size)
{
	static IRPolyPoint const shape[] =
	{
		{216, 128}, {144, 186}, {144, 154}, {40, 154}, {40, 102}, {144, 102}, {144, 70},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRGridItem gridX[] = { {0, 40}, {0, 144} };
	IRGridItem gridY[] = { {0, 102}, {0, 154} };
	IRCanvas canvas = {40, 40, 216, 216, itemsof(gridX), itemsof(gridY), gridX, gridY};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMManipulate), IRTarget(0.8f));
	return cRenderer.get();
}

HICON GetIconRotateLeft(ULONG size)
{
	static IRPathPoint const shape[] =
	{
		{42, 99, 0, 0, 0, 0},
		{98, 173, 0, 0, 0, 0},
		{106, 142, 26.9319, 6.91495, 0, 0},
		{151, 200, 0, 0, -0.2099, -26.5505},
		{203, 199, -0.392212, -49.5606, 0, 0},
		{119, 92, 0, 0, 50.2727, 12.9078},
		{127, 61, 0, 0, 0, 0},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRCanvas canvas = {40, 40, 216, 216, 0, 0, NULL, NULL};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMManipulate), IRTarget(0.8f));
	return cRenderer.get();
}

HICON GetIconRotateRight(ULONG size)
{
	static IRPathPoint const shape[] =
	{
		{214, 99, 0, 0, 0, 0},
		{158, 173, 0, 0, 0, 0},
		{150, 142, -26.9319, 6.91495, 0, 0},
		{105, 200, 0, 0, 0.2099, -26.5505},
		{53, 199, 0.392204, -49.5606, 0, 0},
		{137, 92, 0, 0, -50.2727, 12.9078},
		{129, 61, 0, 0, 0, 0},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRCanvas canvas = {40, 40, 216, 216, 0, 0, NULL, NULL};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMManipulate), IRTarget(0.8f));
	return cRenderer.get();
}

HICON GetIconPalette(ULONG size)
{
	static IRPathPoint const palette1[] =
	{
		{27, 226, -45, -69, 20.6501, 31.6635},
		{77, 19, 69, -35, -83.7558, 42.4848},
		{244, 70, 27.1771, 63.0508, -25, -58},
		{175, 204, -26.9957, -4.0292, 67, 10},
		{104, 235, -18.3537, 23.7518, 17, -22},
	};
	static IRPathPoint const palette2[] =
	{
		{83, 216, 8, -9, -7.06229, 7.94507},
		{114, 189, 14, -7, -10, 5},
		{94, 168, -14, 0, 22, 0},
		{60, 197, -2.50845, 20.0676, 2, -16},
	};
	static IRPath const palette[] = { {itemsof(palette1), palette1}, {itemsof(palette2), palette2} };
	static IRPathPoint const color1[] =
	{
		{62, 86, -16.0484, -4.3001, 16.0484, 4.3001},
		{23, 114, -5.32201, 19.8622, 5.32201, -19.8622},
		{43, 158, 16.0484, 4.3001, -16.0484, -4.3001},
		{82, 130, 5.32201, -19.8622, -5.32201, 19.8622},
	};
	static IRPathPoint const color2[] =
	{
		{138, 44, -7.0997, -15.2254, 7.0997, 15.2254},
		{92, 32, -18.476, 8.61549, 18.476, -8.61549},
		{71, 76, 7.0997, 15.2254, -7.0997, -15.2254},
		{117, 88, 18.476, -8.61549, -18.476, 8.61549},
	};
	static IRPathPoint const color3[] =
	{
		{212, 93, 10.5907, -12.6215, -10.5907, 12.6215},
		{203, 46, -15.7634, -13.2271, 15.7634, 13.2271},
		{155, 45, -10.5907, 12.6215, 10.5907, -12.6215},
		{164, 92, 15.7634, 13.2271, -15.7634, -13.2271},
	};
	static IRPathPoint const color4[] =
	{
		{183, 179, 14.8927, 6.94458, -14.8927, -6.94458},
		{225, 159, 8.61552, -18.476, -8.61552, 18.476},
		{214, 113, -14.8927, -6.94458, 14.8927, 6.94458},
		{172, 133, -8.61552, 18.476, 8.61552, -18.476},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};

	IRFill matFill1(0xffe88465);
	IRFill matFill2(0xfff2cf83);
	IRFill matFill3(0xff9ac769);
	IRFill matFill4(0xff62d7ba);
	IROutlinedFill mat1(&matFill1, pSI->GetMaterial(ESMContrast));
	IROutlinedFill mat2(&matFill2, pSI->GetMaterial(ESMContrast));
	IROutlinedFill mat3(&matFill3, pSI->GetMaterial(ESMContrast));
	IROutlinedFill mat4(&matFill4, pSI->GetMaterial(ESMContrast));
	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(palette), palette, pSI->GetMaterial(ESMScheme2Color2));
	cRenderer(&canvas, itemsof(color1), color1, &mat1);
	cRenderer(&canvas, itemsof(color2), color2, &mat2);
	cRenderer(&canvas, itemsof(color3), color3, &mat3);
	cRenderer(&canvas, itemsof(color4), color4, &mat4);
	return cRenderer.get();
}

HICON GetIconPaints(ULONG a_nSize)
{
	static IRPathPoint const cup[] =
	{
		{32, 64, 0, 64, 0, 0},
		{54, 232, 32, 32, -20, -28},
		{202, 232, 20, -28, -32, 32},
		{224, 64, 0, 0, 0, 64},
	};
	static IRPathPoint const inside[] =
	{
		{224, 64, 0, 10.765, 0, -35.3462},
		{212.924, 93.7754, 0, 0, 7.04282, -8.90307},
		{43.0762, 93.7754, -7.04282, -8.90307, 0, 0},
		{32, 64, 0, -35.3462, 0, 10.765},
		{128, 0, 53.0193, 0, -53.0193, 0},
	};
	static IRPathPoint const paint[] =
	{
		{212.924, 93.7754, -16.0819, 20.3296, -10.1484, -24.6449},
		{128, 128, -36.8718, 0, 36.8719, 0},
		{43.0762, 93.7754, 10.1484, -24.6449, 16.0819, 20.3296},
		{128, 51, 40.605, 0, -40.605, 0},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRCanvas canvas = {32, 0, 224, 256, 0, 0, NULL, NULL};

	IRFill matFillR(0xffb92929);
	IRFill matFillG(0xff00b000);
	IRFill matFillB(0xff7782ff);
	IROutlinedFill matR(&matFillR, pSI->GetMaterial(ESMContrast));
	IROutlinedFill matG(&matFillG, pSI->GetMaterial(ESMContrast));
	IROutlinedFill matB(&matFillB, pSI->GetMaterial(ESMContrast));
	CIconRendererReceiver cRenderer(a_nSize);
	IRTarget targetR(0.6f, -1, -1);
	IRTarget targetG(0.6f, -0.1f, 1);
	IRTarget targetB(0.6f, 1, -0.6f);
	cRenderer(&canvas, itemsof(cup), cup, pSI->GetMaterial(ESMScheme1Color2), targetB);
	cRenderer(&canvas, itemsof(inside), inside, pSI->GetMaterial(ESMInterior), targetB);
	cRenderer(&canvas, itemsof(paint), paint, &matB, targetB);
	cRenderer(&canvas, itemsof(cup), cup, pSI->GetMaterial(ESMScheme1Color2), targetR);
	cRenderer(&canvas, itemsof(inside), inside, pSI->GetMaterial(ESMInterior), targetR);
	cRenderer(&canvas, itemsof(paint), paint, &matR, targetR);
	cRenderer(&canvas, itemsof(cup), cup, pSI->GetMaterial(ESMScheme1Color2), targetG);
	cRenderer(&canvas, itemsof(inside), inside, pSI->GetMaterial(ESMInterior), targetG);
	cRenderer(&canvas, itemsof(paint), paint, &matG, targetG);
	return cRenderer.get();
}

HICON GetIconPencil(ULONG a_nSize)
{
	IRPolyPoint const eraser[] =
	{
		{256, 56}, {256, 72}, {226, 102}, {154, 30}, {184, 0}, {200, 0},
	};
	IRPolyPoint const holder[] =
	{
		{160, 24}, {232, 96}, {213, 115}, {141, 43},
	};
	IRPolyPoint const body[] =
	{
		{147, 37}, {219, 109}, {90, 238}, {18, 166},
	};
	IRPolyPoint const slope[] =
	{
		{18, 166}, {24, 160}, {96, 232}, {90, 238}, {45, 247}, {9, 211},
	};
	IRPolyPoint const tip[] =
	{
		{11, 201}, {55, 245}, {0, 256},
	};

	IRCanvas const canvas = { 0, 0, 256, 256, 0, 0, NULL, NULL };
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(eraser), eraser, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvas, itemsof(holder), holder, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(body), body, pSI->GetMaterial(ESMBrightLight));
	cRenderer(&canvas, itemsof(slope), slope, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvas, itemsof(tip), tip, pSI->GetMaterial(ESMContrast));
	return cRenderer.get();
}

HICON GetIconEye(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const eye[] =
	{
		{256, 128, -23.4869, 46.8361, -23.4869, -46.8361},
		{128, 207, -55.9636, 0, 55.9637, 0},
		{0, 128, 23.487, -46.8359, 23.487, 46.8359},
		{128, 49, 55.9637, 0, -55.9636, 0},
	};
	static IRPathPoint const iris[] =
	{
		{188, 128, 0, -33.1371, 0, 33.1371},
		{128, 68, -33.1371, 0, 33.1371, 0},
		{68, 128, 0, 33.1371, 0, -33.1371},
		{128, 188, 33.1371, 0, -33.1371, 0},
	};
	static IRPathPoint const pupil[] =
	{
		{153, 128, 0, -13.8071, 0, 13.8071},
		{128, 103, -13.8071, 0, 13.8071, 0},
		{103, 128, 0, 13.8071, 0, -13.8071},
		{128, 153, 13.8071, 0, -13.8071, 0},
	};
	static IRPathPoint const hilight[] =
	{
		{125, 111, 0, -6.62742, 0, 6.62742},
		{113, 99, -6.62742, 0, 6.62742, 0},
		{101, 111, 0, 6.62742, 0, -6.62742},
		{113, 123, 6.62742, 0, -6.62742, 0},
	};
	static IRPathPoint const brow[] =
	{
		{260, 67, -35.0171, -26.8701, -27.3217, -36.1493},
		{128, 24, -50.6595, 0, 50.6598, 0},
		{-4, 67, 27.3217, -36.1491, 35.017, -26.8698},
		{128, 7, 55.7081, 0, -55.7079, 0},
	};
	static IRCanvas canvas = {0, 24, 256, 208, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	IRFill hilightMat(0x7fffffff);
	cRenderer(&canvas, itemsof(eye), eye, pSI->GetMaterial(ESMBackground));
	cRenderer(&canvas, itemsof(iris), iris, pSI->GetMaterial(ESMManipulate));
	cRenderer(&canvas, itemsof(pupil), pupil, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(hilight), hilight, &hilightMat);
	cRenderer(&canvas, itemsof(brow), brow, pSI->GetMaterial(ESMContrast));
	return cRenderer.get();
}

HICON GetIconCamera(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const body[] =
	{
		{175, 54, 0, 0, -1.01576, -12.3196},
		{232, 54, 13.2548, 0, 0, 0},
		{256, 78, 0, 0, 0, -13.2548},
		{256, 190, 0, 13.2548, 0, 0},
		{232, 214, 0, 0, 13.2548, 0},
		{24, 214, -13.2548, 0, 0, 0},
		{0, 190, 0, 0, 0, 13.2548},
		{0, 78, 0, -13.2548, 0, 0},
		{24, 54, 0, 0, -13.2548, 0},
		{81, 54, 1.01576, -12.3196, 0, 0},
		{105, 32, 0, 0, -12.5817, 0},
		{151, 32, 12.5817, 0, 0, 0},
	};
	static IRPolyPoint const stripe[] =
	{
		{0, 86}, {256, 86}, {256, 182}, {0, 182},
	};
	static IRPathPoint const objective[] =
	{
		{192, 134, 0, -35.3462, 0, 35.3462},
		{128, 70, -35.3462, 0, 35.3462, 0},
		{64, 134, 0, 35.3462, 0, -35.3462},
		{128, 198, 35.3462, 0, -35.3462, 0},
	};
	static IRPathPoint const glass[] =
	{
		{172, 134, 0, -24.3005, 0, 24.3005},
		{128, 90, -24.3005, 0, 24.3005, 0},
		{84, 134, 0, 24.3005, 0, -24.3005},
		{128, 178, 24.3005, 0, -24.3005, 0},
	};
	static IRPathPoint const hilight[] =
	{
		{139, 106, -21.0115, 0.701173, -3.3148, -1.26159},
		{100, 145, -1.26159, -3.3148, 0.701173, -21.0115},
		{98, 134, 0, -16.5685, 0, 3.75759},
		{128, 104, 3.75759, 0, -16.5685, 0},
	};
	static IRGridItem const gridX[] = { {0, 0}, {0, 256} };
	static IRGridItem const gridY[] = { {0, 32}, {0, 54}, {0, 214} };
	static IRGridItem const gridYStripe[] = { {0, 86}, {0, 182} };
	static IRCanvas const canvas = {0, 32, 256, 224, itemsof(gridX), itemsof(gridY), gridX, gridY};
	static IRCanvas const canvasStripe = {0, 32, 256, 224, itemsof(gridX), itemsof(gridYStripe), gridX, gridYStripe};
	CIconRendererReceiver cRenderer(a_nSize);
	IRFill hilightMat(0x7fffffff);
	IRFill glassFillMat(0xffd5ebf1);
	IROutlinedFill glassMat(&glassFillMat, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(body), body, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvasStripe, itemsof(stripe), stripe, pSI->GetMaterial(ESMManipulate));
	cRenderer(&canvas, itemsof(objective), objective, pSI->GetMaterial(ESMManipulate));
	cRenderer(&canvas, itemsof(glass), glass, &glassMat);
	cRenderer(&canvas, itemsof(hilight), hilight, &hilightMat);
	return cRenderer.get();
}

HICON GetIconTarget(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const stand[] =
	{
		{122.941, 0.128551, 0, 0, -5.6935, 0.38449},
		{140.995, 0.0957799, 6.10795, -0.0355666, 0, 0},
		{155, 10, 0, 0, -2.23006, -6.12707},
		{236, 232, 2.83337, 7.78468, 0, 0},
		{227, 251, 0, 0, 7.78468, -2.83337},
		{218, 255, -7.78468, 2.83337, 0, 0},
		{199, 246, 0, 0, 2.83337, 7.78468},
		{133, 64, 0, 0, 0, 0},
		{66, 246, -2.83339, 7.78467, 0, 0},
		{47, 255, 0, 0, 7.78467, 2.83339},
		{38, 251, -7.78467, -2.83339, 0, 0},
		{29, 232, 0, 0, -2.83339, 7.78467},
		{110, 10, 2.09994, -5.76954, 0, 0},
	};
	static IRPathPoint const circle1[] =
	{
		{240, 132, 0, -61.8559, 0, 61.8559},
		{128, 20, -61.8559, 0, 61.8559, 0},
		{16, 132, 0, 61.8559, 0, -61.8559},
		{128, 244, 61.8559, 0, -61.8559, 0},
	};
	static IRPathPoint const circle2[] =
	{
		{214, 132, 0, -47.4965, 0, 47.4965},
		{128, 46, -47.4965, 0, 47.4965, 0},
		{42, 132, 0, 47.4965, 0, -47.4965},
		{128, 218, 47.4965, 0, -47.4965, 0},
	};
	static IRPathPoint const circle3[] =
	{
		{188, 132, 0, -33.1371, 0, 33.1371},
		{128, 72, -33.1371, 0, 33.1371, 0},
		{68, 132, 0, 33.1371, 0, -33.1371},
		{128, 192, 33.1371, 0, -33.1371, 0},
	};
	static IRPathPoint const circle4[] =
	{
		{162, 132, 0, -18.7777, 0, 18.7777},
		{128, 98, -18.7777, 0, 18.7777, 0},
		{94, 132, 0, 18.7777, 0, -18.7777},
		{128, 166, 18.7777, 0, -18.7777, 0},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	//IRFill redFillMat(0xffff0000);
	//IROutlinedFill redMat(&redFillMat, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(stand), stand, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(circle1), circle1, pSI->GetMaterial(ESMBackground));
	cRenderer(&canvas, itemsof(circle2), circle2, pSI->GetMaterial(ESMScheme1Color1));//&redMat);
	cRenderer(&canvas, itemsof(circle3), circle3, pSI->GetMaterial(ESMBackground));
	cRenderer(&canvas, itemsof(circle4), circle4, pSI->GetMaterial(ESMScheme1Color1));//&redMat);
	return cRenderer.get();
}

HICON GetIcon3DEditor(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPolyPoint const cube[] =
	{
		{239, 64}, {239, 192}, {128, 256}, {17, 192}, {17, 64}, {128, 0}
	};
	static IRPolyPoint const shadow11[] =
	{
		{21, 190}, {21, 67}, {74.5, 98}, {74.5, 159}, {128, 190}, {128, 251}
	};
	static IRPolyPoint const shadow12[] =
	{
		{128, 66}, {181, 98}, {181, 158}, {128, 128}
	};
	static IRPolyPoint const shadow21[] =
	{
		{182, 98}, {235, 67}, {235, 190}, {128, 251}, {128, 190}, {182, 159}
	};
	static IRPolyPoint const shadow22[] =
	{
		{75, 98}, {128, 67}, {128, 128}, {75, 159}
	};
	static IRPolyPoint const line1[] = { {74.5, 159.5}, {74.5, 97.5}, {23.5, 67.5} };
	static IRPolyPoint const line2[] = { {232.5, 68.5}, {181.5, 97.5}, {128, 66} };
	static IRPolyPoint const line3[] = { {128, 248}, {128, 190}, {181.5, 159} };
	static IRPolyPoint const line4[] = { {74.5, 97.5}, {128, 66}, {128, 128} };
	static IRPolyPoint const line5[] = { {128, 190}, {74.5, 159}, {128, 128} };
	static IRPolyPoint const line6[] = { {181.5, 97.5}, {181.5, 159}, {128, 128} };
	static IRGridItem const grid[] = { {0, 17}, {0, 239} };
	static IRCanvas canvas = {0, 0, 256, 256, itemsof(grid), 0, grid, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	IRFill shadow1Mat(0x3f000000);
	IRFill shadow2Mat(0x7f000000);
	cRenderer(&canvas, itemsof(cube), cube, pSI->GetMaterial(ESMBackground));
	cRenderer(&canvas, itemsof(shadow11), shadow11, &shadow1Mat);
	cRenderer(&canvas, itemsof(shadow12), shadow12, &shadow1Mat);
	cRenderer(&canvas, itemsof(shadow21), shadow21, &shadow2Mat);
	cRenderer(&canvas, itemsof(shadow22), shadow22, &shadow2Mat);
	cRenderer(&canvas, itemsof(line1), line1, pSI->GetMaterial(ESMOutline));
	cRenderer(&canvas, itemsof(line2), line2, pSI->GetMaterial(ESMOutline));
	cRenderer(&canvas, itemsof(line3), line3, pSI->GetMaterial(ESMOutline));
	cRenderer(&canvas, itemsof(line4), line4, pSI->GetMaterial(ESMOutline));
	cRenderer(&canvas, itemsof(line5), line5, pSI->GetMaterial(ESMOutline));
	cRenderer(&canvas, itemsof(line6), line6, pSI->GetMaterial(ESMOutline));
	return cRenderer.get();
}

static struct {GUID tID; pfnGetIcon pfn;} const g_aIDs[] =
{
	{{0x01511A3D, 0x2F03, 0x4fa7, {0xBD, 0xB0, 0x53, 0x56, 0x88, 0x82, 0xC9, 0x2D}}, GetIconRotateLeft},
	{{0x20337D09, 0x4D1A, 0x4621, {0x80, 0x55, 0x6B, 0xBD, 0xC2, 0x05, 0x05, 0x05}}, GetIconRotateRight},
	{{0x62D02763, 0xFA90, 0x45d1, {0x85, 0xA2, 0x83, 0x11, 0x71, 0x41, 0xFA, 0x5D}}, GetIconOperationBlue},
	{{0x91A7D922, 0x8B41, 0x4953, {0x9F, 0x80, 0x64, 0x26, 0xB6, 0x41, 0xAF, 0x47}}, GetIconOperationGreen},
	{{0xce3b7b2d, 0xe555, 0x4568, {0x97, 0x79, 0x70, 0xb0, 0x52, 0x88, 0x5e, 0xef}}, GetIcon3DEditor},
	//{{0x77afc4b8, 0x9e92, 0x415e, {0xa6, 0x34, 0xf6, 0xe6, 0x72, 0x45, 0x7c, 0x06}}, IDI_STOCK_ICONEDITOR, NULL},
	{{0x2735ad9f, 0x999e, 0x4580, {0xa8, 0xa8, 0x90, 0x5c, 0x6f, 0xa5, 0x75, 0x28}}, GetIconPencil},
	{{0xb5acb30b, 0x5b2c, 0x4209, {0x98, 0x89, 0x76, 0xc6, 0xb1, 0xe7, 0x25, 0xe3}}, GetIconCamera},
	{{0x0384177c, 0x7809, 0x4d11, {0xab, 0x47, 0xa5, 0x6f, 0x11, 0xcd, 0xf2, 0x23}}, GetIconMagnifier},
	{{0x753d6f44, 0xa1fa, 0x4fa0, {0x97, 0x40, 0x7f, 0xbb, 0x3c, 0x06, 0x6d, 0xdb}}, GetIconCreate},
	{{0xccc5c355, 0x5d9e, 0x466c, {0x82, 0x98, 0x55, 0x09, 0xdb, 0xcd, 0xd1, 0x2e}}, GetIconDuplicate},
	{{0xa96d38f3, 0x446b, 0x4fee, {0x8b, 0x15, 0x56, 0xdf, 0x11, 0x38, 0xb0, 0x38}}, GetIconDelete},
	{{0x2c3bedf2, 0xf973, 0x11da, {0xb9, 0x34, 0x00, 0x50, 0x56, 0xc0, 0x00, 0x08}}, GetIconMoveLeft},
	{{0x2c3bedf3, 0xf973, 0x11da, {0xb9, 0x34, 0x00, 0x50, 0x56, 0xc0, 0x00, 0x08}}, GetIconMoveRight},
	{{0x2c3bedf4, 0xf973, 0x11da, {0xb9, 0x34, 0x00, 0x50, 0x56, 0xc0, 0x00, 0x08}}, GetIconMoveUp},
	{{0x2c3bedf5, 0xf973, 0x11da, {0xb9, 0x34, 0x00, 0x50, 0x56, 0xc0, 0x00, 0x08}}, GetIconMoveDown},
	{{0xe684d999, 0xa06f, 0x419b, {0xb8, 0xe7, 0x5c, 0xb7, 0x7c, 0x23, 0x63, 0xd7}}, GetIconPaints},
	{{0x028860b0, 0x58c4, 0x4eac, {0xb7, 0x61, 0xe8, 0x8b, 0x2e, 0xde, 0xd3, 0xe1}}, GetIconStructure},
	{{0xebb8be48, 0x100e, 0x4d34, {0x98, 0x43, 0xaf, 0x4f, 0xd6, 0x72, 0xa7, 0xe0}}, GetIconWrench},
	{{0xc600a985, 0xe58c, 0x4909, {0xa8, 0xa0, 0xc5, 0xd6, 0x4c, 0x80, 0x6e, 0x61}}, GetIconEye},
	{{0x211dd21b, 0x57a2, 0x470d, {0x9b, 0x0b, 0x1b, 0x60, 0x80, 0x28, 0x82, 0xb9}}, GetIconSettings},
	{{0x96ea5ee7, 0xda78, 0x4efb, {0x8b, 0x12, 0x17, 0x6c, 0x7a, 0x70, 0xe5, 0x7e}}, GetIconOperationBeige},
	{{0xc8cee069, 0xbb50, 0x44c0, {0x9e, 0x6e, 0xa8, 0x5c, 0x28, 0x5c, 0x91, 0xea}}, GetIconPalette},
	{{0x106c5121, 0x0678, 0x4a70, {0xba, 0xc7, 0xcc, 0x63, 0xda, 0xd9, 0x0b, 0x31}}, GetIconUserBlue},
	{{0x106c5121, 0x0678, 0x4a70, {0xba, 0xc7, 0xcc, 0x63, 0xda, 0xd9, 0x0b, 0x32}}, GetIconUserBeige},
	{{0x106c5121, 0x0678, 0x4a70, {0xba, 0xc7, 0xcc, 0x63, 0xda, 0xd9, 0x0b, 0x33}}, GetIconUserGreen},
	{{0x31f2e7e4, 0x6b67, 0x4097, {0xa4, 0x00, 0x83, 0x8f, 0xcd, 0x88, 0xec, 0x6e}}, GetIconTarget},
	{{0x82060ec2, 0x74ac, 0x4bd7, {0xaf, 0x99, 0x6f, 0xd3, 0x7b, 0xdc, 0xcb, 0x9e}}, GetIconLibrary},
};

STDMETHODIMP CDesignerFrameStockIcons::TimeStamp(ULONG* a_pTimeStamp)
{
	try
	{
		*a_pTimeStamp = 0;
		return S_OK;
	}
	catch (...)
	{
		return a_pTimeStamp == NULL ? E_POINTER: E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameStockIcons::EnumIconIDs(IEnumGUIDs** a_ppIDs)
{
	try
	{
		*a_ppIDs = NULL;

		CComPtr<IEnumGUIDsInit> pIDs;
		RWCoCreateInstance(pIDs, __uuidof(EnumGUIDs));

		for (size_t i = 0; i < itemsof(g_aIDs); i++)
		{
			pIDs->Insert(g_aIDs[i].tID);
		}

		*a_ppIDs = pIDs.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppIDs == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameStockIcons::GetIcon(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;

		for (size_t i = 0; i < itemsof(g_aIDs); i++)
		{
			if (IsEqualGUID(a_tIconID, g_aIDs[i].tID))
			{
				*a_phIcon = g_aIDs[i].pfn(a_nSize);
				return (*a_phIcon) ? S_OK : E_FAIL;
			}
		}

		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

