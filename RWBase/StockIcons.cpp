
#include "stdafx.h"

#include <IconRenderer.h>
#include <math.h>
#include <GammaCorrection.h>

enum EColorScheme
{
	ECSDefault = 0,
	ECSLight,
	ECSDark,
	ECSCustom,
	ECSIvalid = 0xffffffff
};

static float const g_fOutlineWidthRelative = 1.0f/40.0f;
static float const g_fOutlineWidthAbsolute = 0.225f;
static float const g_fOutlineWidthLargeRelative = 1.0f/80.0f;
static float const g_fOutlineWidthLargeAbsolute = 0.4f;
static IRFill g_tContrast(0xff000000);
static IRStroke g_tOutlinePen(0xff000000, 0.05f);
static IRStroke g_tOutlinePen50(0xff000000, 0.03f);
static IRFill g_tBackgroundFill(0xffffffff);
static IROutlinedFill g_tBackground(&g_tBackgroundFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tButtonFill(0xffe0e0e0);
static IROutlinedFill g_tButton(&g_tButtonFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tInteriorFill(0xfff0f0f0);
static IROutlinedFill g_tInterior(&g_tInteriorFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IROutlinedFill g_tInteriorLarge(&g_tInteriorFill, &g_tContrast, g_fOutlineWidthLargeRelative, g_fOutlineWidthLargeAbsolute);
static IRFill g_tConfirmFill(0xff78f278);
static IROutlinedFill g_tConfirm(&g_tConfirmFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tCancelFill(0xfff69797);
static IROutlinedFill g_tCancel(&g_tCancelFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tHelpFill(0xff1157b8);
static IROutlinedFill g_tHelp(&g_tHelpFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tCreateFill(0xff6782eb);
static IROutlinedFill g_tCreate(&g_tCreateFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tManipulateFill(0xff6782eb);
static IROutlinedFill g_tManipulate(&g_tManipulateFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tDeleteFill(0xfff69797);
static IROutlinedFill g_tDelete(&g_tDeleteFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tBrightLightFill(0xffebd467);
static IROutlinedFill g_tBrightLight(&g_tBrightLightFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tScheme1Color1Fill(0xffed9191);
static IROutlinedFill g_tScheme1Color1(&g_tScheme1Color1Fill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tScheme1Color2Fill(0xffd39a56);
static IROutlinedFill g_tScheme1Color2(&g_tScheme1Color2Fill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tScheme1Color3Fill(0xffe8f07e);
static IROutlinedFill g_tScheme1Color3(&g_tScheme1Color3Fill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tScheme2Color1Fill(0xff6d89eb);
static IROutlinedFill g_tScheme2Color1(&g_tScheme2Color1Fill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tScheme2Color2Fill(0xfff4d2a5);
static IROutlinedFill g_tScheme2Color2(&g_tScheme2Color2Fill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tScheme2Color3Fill(0xff83ca83);
static IROutlinedFill g_tScheme2Color3(&g_tScheme2Color3Fill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tScheme3Color1Fill(0xffc895e3);
static IROutlinedFill g_tScheme3Color1(&g_tScheme3Color1Fill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tScheme3Color2Fill(0xff97dae5);
static IROutlinedFill g_tScheme3Color2(&g_tScheme3Color2Fill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tScheme3Color3Fill(0xffead09f);
static IROutlinedFill g_tScheme3Color3(&g_tScheme3Color3Fill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tFolderFill(0xfffbe99e);
static IROutlinedFill g_tFolder(&g_tFolderFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRFill g_tFloppyFill(0xff6782eb);//0xff0061ff);
static IROutlinedFill g_tFloppy(&g_tFloppyFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);

static IRGridItem const g_tGridWhole[] = {{EGIFInteger, 0.0f}, {EGIFInteger, 256.0f}};
static IRCanvas const g_tCanvasPlain = {0, 0, 256, 256, 0, 0, NULL, NULL};

static IRPolyPoint const g_tPointsPlus[] =
{
	{80, 0}, {176, 0}, {176, 80}, {256, 80}, {256, 176}, {176, 176}, {176, 256}, {80, 256}, {80, 176}, {0, 176}, {0, 80}, {80, 80}
};
static IRPolygon const g_tShapesPlus[] = {{itemsof(g_tPointsPlus), g_tPointsPlus}};
static IRGridItem const g_tGridPlus[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
static IRCanvas const g_tCanvasPlus = {0, 0, 256, 256, itemsof(g_tGridPlus), itemsof(g_tGridPlus), g_tGridPlus, g_tGridPlus};
static IRLayer const g_tLayerPlus = {&g_tCanvasPlus, itemsof(g_tShapesPlus), 0, g_tShapesPlus, NULL, &g_tManipulate};
static IRLayer const g_tLayerCreate = {&g_tCanvasPlus, itemsof(g_tShapesPlus), 0, g_tShapesPlus, NULL, &g_tCreate};

static IRPolyPoint const g_tPointsMinus[] =
{
	{256, 80}, {256, 176}, {0, 176}, {0, 80},
};
static IRPolygon const g_tShapesMinus[] = {{itemsof(g_tPointsMinus), g_tPointsMinus}};
static IRLayer const g_tLayerMinus = {&g_tCanvasPlus, itemsof(g_tShapesMinus), 0, g_tShapesMinus, NULL, &g_tManipulate};

static IRPolyPoint const g_tPointsCheck[] =
{
	{0, 128}, {68, 60}, {113, 105}, {188, 30}, {256, 98}, {113, 241},
};
static IRPolygon const g_tShapesCheck[] = {{itemsof(g_tPointsCheck), g_tPointsCheck}};
static IRLayer const g_tLayerCheck = {&g_tCanvasPlain, itemsof(g_tShapesCheck), 0, g_tShapesCheck, NULL, &g_tConfirm};

static IRPolyPoint const g_tPointsCross[] =
{
	{184.569, 3.54921}, {252.451, 71.4315}, {195.882, 128}, {252.451, 184.569}, {184.569, 252.451}, {128, 195.882}, {71.4315, 252.451}, {3.54921, 184.569}, {60.1178, 128}, {3.54921, 71.4315}, {71.4315, 3.54921}, {128, 60.1178},
};
static IRPolygon const g_tShapesCross[] = {{itemsof(g_tPointsCross), g_tPointsCross}};
static IRCanvas const g_tCanvasCross = {3.5f, 3.5f, 252.5f, 252.5f, 0, 0, NULL, NULL};
static IRLayer const g_tLayerCancel = {&g_tCanvasCross, itemsof(g_tShapesCross), 0, g_tShapesCross, NULL, &g_tCancel};
static IRLayer const g_tLayerDelete = {&g_tCanvasCross, itemsof(g_tShapesCross), 0, g_tShapesCross, NULL, &g_tDelete};

static IRPathPoint const g_tPointsCircle[] =
{
	{256, 128, 0, -70.6925, 0, 70.6925},
	{128, 0, -70.6925, 0, 70.6925, 0},
	{0, 128, 0, 70.6925, 0, -70.6925},
	{128, 256, 70.6925, 0, -70.6925, 0},
};
static IRPath const g_tShapesCircle[] = {{itemsof(g_tPointsCircle), g_tPointsCircle}};
static IRLayer const g_tLayerCircle = {&g_tCanvasPlain, 0, itemsof(g_tShapesCircle), NULL, g_tShapesCircle, &g_tHelp};

static IRPathPoint const g_tPointsQuestion1[] =
{
	{140, 166, 0, 0, -0.135417, -7.59375},
	{115, 166, -0.0625, -3.60417, 0, 0},
	{115, 159, 0, -8.11458, 0, 0.78125},
	{119, 139, 2.60417, -5.22917, -2.60417, 5.23958},
	{134, 122, 7.82292, -6.54167, -7.82292, 6.55208},
	{148, 109, 2.35417, -3.20833, -1.52083, 2.02083},
	{152, 98, 0, -5.29167, 0, 3.86458},
	{146, 85, -4.19792, -3.77083, 4.19792, 3.78125},
	{129, 79, -6.86458, 0, 7.11458, 0},
	{112, 84, -4.61458, 3.625, 4.61458, -3.61458},
	{102, 101, 0, 0, 1.72917, -7.42708},
	{77, 99, 0.71875, -11.4063, 0, 0},
	{92, 70, 9.22917, -7.95833, -9.23958, 7.96875},
	{128, 58, 15.8021, 0, -15.0104, 0},
	{166, 70, 9.33333, 8.22917, -9.33333, -8.21875},
	{180, 99, 0, 6.08333, 0, -10.9167},
	{175, 116, -3.4375, 5.4375, 3.4375, -5.42708},
	{153, 139, -5.83333, 4.84375, 11.2708, -9.36458},
	{142, 150, -1.41667, 2.94792, 1.40625, -2.9375},

};
static IRPolyPoint const g_tPointsQuestion2[] =
{
	{115, 203}, {115, 175}, {140, 175}, {140, 203},
};
static IRPath const g_tShapesQuestion1[] = {{itemsof(g_tPointsQuestion1), g_tPointsQuestion1}};
static IRPolygon const g_tShapesQuestion2[] = {{itemsof(g_tPointsQuestion2), g_tPointsQuestion2}};
static IRLayer const g_tLayerQuestion = {&g_tCanvasPlain, itemsof(g_tShapesQuestion2), itemsof(g_tShapesQuestion1), g_tShapesQuestion2, g_tShapesQuestion1, &g_tBackgroundFill};

static IRPolyPoint const g_tPointsDuplicate[] =
{
	{0, 80}, {0, 176}, {74, 176}, {150, 252}, {256, 252}, {256, 156}, {188, 156}, {160, 128}, {188, 100}, {256, 100}, {256, 4}, {150, 4}, {74, 80}
};
static IRPolygon const g_tShapesDuplicate[] = {{itemsof(g_tPointsDuplicate), g_tPointsDuplicate}};
static IRGridItem const g_tGridDuplicateX[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 256.0f}};
static IRGridItem const g_tGridDuplicateY[] = { {EGIFInteger, 4.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 100.0f}, {EGIFInteger, 156.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 252.0f}};
static IRCanvas const g_tCanvasDuplicate = {0, 0, 256, 256, itemsof(g_tGridDuplicateX), itemsof(g_tGridDuplicateY), g_tGridDuplicateX, g_tGridDuplicateY};
static IRLayer const g_tLayerDuplicate = {&g_tCanvasDuplicate, itemsof(g_tShapesDuplicate), 0, g_tShapesDuplicate, NULL, &g_tManipulate};

static IRGridItem const g_tGridDocumentY[] = {{EGIFInteger, 0.0f}, {EGIFMidPixel, 82.5f}, {EGIFInteger, 256.0f}};
static IRGridItem const g_tGridDocumentX[] = {{EGIFInteger, 0.0f}, {EGIFMidPixel, 82.5f}, {EGIFInteger, 200.0f}};
static IRCanvas const g_tCanvasDocument = {0, 0, 200, 256, itemsof(g_tGridDocumentX), itemsof(g_tGridDocumentY), g_tGridDocumentX, g_tGridDocumentY};

static IRPolyPoint const g_tPointsDocumentPaper[] =
{
	{76, 0}, {200, 0}, {200, 256}, {0, 256}, {0, 76},
};
static IRPolygon const g_tShapesDocumentPaper[] = {{itemsof(g_tPointsDocumentPaper), g_tPointsDocumentPaper}};
static IRLayer const g_tLayerDocumentPaper = {&g_tCanvasDocument, itemsof(g_tShapesDocumentPaper), 0, g_tShapesDocumentPaper, NULL, &g_tBackground};

static IRPolyPoint const g_tPointsDocumentCorner[] =
{
	{6.5, 82.5}, {82.5, 82.5}, {82.5, 6.5},
};
static IRPolygon const g_tShapesDocumentCorner[] = {{itemsof(g_tPointsDocumentCorner), g_tPointsDocumentCorner}};
static IRLayer const g_tLayerDocumentCorner = {&g_tCanvasDocument, itemsof(g_tShapesDocumentCorner), 0, g_tShapesDocumentCorner, NULL, &g_tOutlinePen50};

static IRPolyPoint const g_tPointsFolderBack[] =
{
	{0, 24}, {103, 24}, {143, 64}, {256, 64}, {256, 230}, {0, 230},
};
static IRPolygon const g_tShapesFolderBack[] = {{itemsof(g_tPointsFolderBack), g_tPointsFolderBack}};
static IRGridItem const g_tGridFolder[] = {{EGIFInteger, 24.0f}, {EGIFInteger, 64.0f}, {EGIFInteger, 104.0f}, {EGIFInteger, 230.0f}};
static IRCanvas const g_tCanvasFolder = {0, 0, 256, 256, itemsof(g_tGridWhole), itemsof(g_tGridFolder), g_tGridWhole, g_tGridFolder};
static IRLayer const g_tLayerFolderBack = {&g_tCanvasFolder, itemsof(g_tShapesFolderBack), 0, g_tShapesFolderBack, NULL, &g_tFolder};

static IRPolyPoint const g_tPointsFolderFront[] =
{
	{0, 104}, {130, 104}, {178, 64}, {256, 64}, {256, 230}, {0, 230},
};
static IRPolygon const g_tShapesFolderFront[] = {{itemsof(g_tPointsFolderFront), g_tPointsFolderFront}};
static IRLayer const g_tLayerFolderFront = {&g_tCanvasFolder, itemsof(g_tShapesFolderFront), 0, g_tShapesFolderFront, NULL, &g_tFolder};

static IRPathPoint const g_tPointsFolderClip[] =
{
	{52, 252, 0, 0, 0, 0},
	{52, 193, 0, -28.1665, 0, 0},
	{103, 142, 0, 0, -28.1665, 0},
	{153, 142, 28.1665, 0, 0, 0},
	{204, 193, 0, 0, 0, -28.1665},
	{204, 252, 0, 0, 0, 0},
	{153, 252, 0, 0, 0, 0},
	{153, 193, 0, 0, 0, 0},
	{103, 193, 0, 0, 0, 0},
	{103, 252, 0, 0, 0, 0},
};
static IRPath const g_tShapesFolderClip[] = {{itemsof(g_tPointsFolderClip), g_tPointsFolderClip}};
static IRGridItem const g_tGridFolderClipX[] = {{EGIFInteger, 52.0f}, {EGIFInteger, 103.0f}, {EGIFInteger, 153.0f}, {EGIFInteger, 204.0f}};
static IRGridItem const g_tGridFolderClipY[] = {{EGIFInteger, 142.0f}, {EGIFInteger, 193.0f}, {EGIFInteger, 252.0f}};
static IRCanvas const g_tCanvasFolderClip = {0, 0, 256, 256, itemsof(g_tGridFolderClipX), itemsof(g_tGridFolderClipY), g_tGridFolderClipX, g_tGridFolderClipY};
static IRFill const g_tFolderClipFill(0xffd5ebf1);
static IROutlinedFill g_tFolderClip(&g_tFolderClipFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRLayer const g_tLayerFolderClip = {&g_tCanvasFolderClip, 0, itemsof(g_tShapesFolderClip), NULL, g_tShapesFolderClip, &g_tFolderClip};


// floppy

static IRPolyPoint const g_tPointsFloppyOutline[] =
{
	{0, 0}, {0, 256}, {256, 256}, {256, 56}, {200, 0},
};
static IRPolygon const g_tShapesFloppyOutline[] = {{itemsof(g_tPointsFloppyOutline), g_tPointsFloppyOutline}};
static IRCanvas const g_tCanvasFloppyOutline = {0, 0, 256, 256, itemsof(g_tGridWhole), itemsof(g_tGridWhole), g_tGridWhole, g_tGridWhole};
static IRLayer const g_tLayerFloppyOutline = {&g_tCanvasFloppyOutline, itemsof(g_tShapesFloppyOutline), 0, g_tShapesFloppyOutline, NULL, &g_tFloppy};

static IRPolyPoint const g_tPointsFloppyTop[] =
{
	{72, 0}, {176, 0}, {176, 72}, {72, 72},
};
static IRPolygon const g_tShapesFloppyTop = {itemsof(g_tPointsFloppyTop), g_tPointsFloppyTop};
static IRGridItem const g_tGridFloppyTopX[] = {{EGIFInteger, 72.0f}, {EGIFInteger, 176.0f}};
static IRGridItem const g_tGridFloppyTopY[] = {{EGIFInteger, 0.0f}, {EGIFInteger, 72.0f}};
static IRCanvas const g_tCanvasFloppyTop = {0, 0, 256, 256, itemsof(g_tGridFloppyTopX), itemsof(g_tGridFloppyTopY), g_tGridFloppyTopX, g_tGridFloppyTopY};
static IRLayer const g_tLayerFloppyTop = {&g_tCanvasFloppyTop, 1, 0, &g_tShapesFloppyTop, NULL, &g_tBackground};

static IRPolyPoint const g_tPointsFloppyBot[] =
{
	{208, 128}, {208, 256}, {48, 256}, {48, 128},
};
static IRPolygon const g_tShapesFloppyBot = {itemsof(g_tPointsFloppyBot), g_tPointsFloppyBot};
static IRGridItem const g_tGridFloppyBotX[] = {{EGIFInteger, 48.0f}, {EGIFInteger, 208.0f}};
static IRGridItem const g_tGridFloppyBotY[] = {{EGIFInteger, 128.0f}, {EGIFInteger, 256.0f}};
static IRCanvas const g_tCanvasFloppyBot = {0, 0, 256, 256, itemsof(g_tGridFloppyBotX), itemsof(g_tGridFloppyBotY), g_tGridFloppyBotX, g_tGridFloppyBotY};
static IRLayer const g_tLayerFloppyBot = {&g_tCanvasFloppyBot, 1, 0, &g_tShapesFloppyBot, NULL, &g_tBackground};

static IRPolyPoint const g_tPointsFloppyStroke1[] = { {80.5, 175.5}, {176.5, 175.5} };
static IRPolyPoint const g_tPointsFloppyStroke2[] = { {80.5, 210.5}, {176.5, 210.5} };
static IRPolyPoint const g_tPointsFloppyStroke3[] = { {141.5, 21.5}, {141.5, 50.5} };
static IRPolygon const g_tShapesFloppyStrokes[] = {{itemsof(g_tPointsFloppyStroke1), g_tPointsFloppyStroke1}, {itemsof(g_tPointsFloppyStroke2), g_tPointsFloppyStroke2}, {itemsof(g_tPointsFloppyStroke3), g_tPointsFloppyStroke3}};
static IRGridItem const g_tGridFloppyStrokes[] = {{EGIFMidPixel, 175.5f}, {EGIFMidPixel, 210.5f}};
static IRCanvas const g_tCanvasFloppyStrokes = {0, 0, 256, 256, 0, itemsof(g_tGridFloppyStrokes), NULL, g_tGridFloppyStrokes};
static IRLayer const g_tLayerFloppyStrokes = {&g_tCanvasFloppyStrokes, itemsof(g_tShapesFloppyStrokes), 0, g_tShapesFloppyStrokes, NULL, &g_tOutlinePen50};


// magnifier

static IRPathPoint const g_tPointsMagnifierHandle[] =
{
	{184, 92, 0, 15.7594, 0, -50.8102},
	{173, 136, 0, 0, 6.98304, -12.9665},
	{251, 204, 5.71143, 5.71141, 0, 0},
	{251, 224, 0, 0, 5.71143, -5.71141},
	{224, 251, -5.71141, 5.71143, 0, 0},
	{204, 251, 0, 0, 5.71141, 5.71143},
	{136, 173, -12.9665, 6.98305, 0, 0},
	{92, 184, -50.8102, 0, 15.7594, 0},
	{0, 92, 0, -50.8102, 0, 50.8102},
	{92, 0, 50.8102, 0, -50.8102, 0},
};
	
static IRPathPoint const g_tPointsMagnifierHandleHole[] =
{
	{92, 32, 33.1371, 0, -33.1371, 0},
	{152, 92, 0, 33.1371, 0, -33.1371},
	{92, 152, -33.1371, 0, 33.1371, 0},
	{32, 92, 0, -33.1371, 0, 33.1371},
};

static IRPath const g_tShapesMagnifierHandle[] = { {itemsof(g_tPointsMagnifierHandle), g_tPointsMagnifierHandle}, {itemsof(g_tPointsMagnifierHandleHole), g_tPointsMagnifierHandleHole} };
static IRFill g_tMagnifierHandleFill(0xff303030);
static IROutlinedFill g_tMagnifierHandle(&g_tMagnifierHandleFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRLayer const g_tLayerMagnifierHandle = {&g_tCanvasPlain, 0, itemsof(g_tShapesMagnifierHandle), NULL, g_tShapesMagnifierHandle, &g_tMagnifierHandle};
static IRLayer const g_tLayerMagnifierHandleAlt = {&g_tCanvasPlain, 0, itemsof(g_tShapesMagnifierHandle), NULL, g_tShapesMagnifierHandle, &g_tManipulate};

static IRPathPoint const g_tPointsMagnifierGlass[] =
{
	{168, 92, 0, -41.9736, 0, 41.9736},
	{92, 16, -41.9736, 0, 41.9736, 0},
	{16, 92, 0, 41.9736, 0, -41.9736},
	{92, 168, 41.9736, 0, -41.9736, 0},
};
static IRPath const g_tShapesMagnifierGlass = {itemsof(g_tPointsMagnifierGlass), g_tPointsMagnifierGlass};
static IRFill g_tGlassFill(0x4fffffff);
static IRLayer const g_tLayerMagnifierGlass = {&g_tCanvasPlain, 0, 1, NULL, &g_tShapesMagnifierGlass, &g_tGlassFill};

static IRPathPoint const g_tPointsMagnifierHilight[] =
{
	{106, 42, -32.2602, 2.18306, -4.68001, -1.38319},
	{49, 104, 0, 8.06891, 0, -32.8138},
	{53, 126, -8.32019, -9.21885, -2.8044, -7.0705},
	{40, 92, 0, -28.7188, 0, 13.3961},
	{92, 40, 5.12841, 0, -28.7188, 0},
};
static IRPath const g_tShapesMagnifierHilight = {itemsof(g_tPointsMagnifierHilight), g_tPointsMagnifierHilight};
static IRLayer const g_tLayerMagnifierHilight = {&g_tCanvasPlain, 0, 1, NULL, &g_tShapesMagnifierHilight, &g_tGlassFill};


// wrench

static IRPathPoint const g_tPointsWrench[] =
{
	{26.388, 148.576, 23.9864, 23.9864, -23.9864, -23.9864},
	{115.484, 169.79, 0, 0, -30.1123, 10.1169},
	{195.245, 249.551, 7.49808, 7.49805, 0, 0},
	{222.398, 249.551, 0, 0, -7.49809, 7.49811},
	{249.551, 222.398, 7.49808, -7.49806, 0, 0},
	{249.551, 195.245, 0, 0, 7.49808, 7.49808},
	{169.79, 115.484, 10.1169, -30.1123, 0, 0},
	{148.576, 26.3884, -23.9864, -23.9864, 23.9864, 23.9864},
	{59.481, 5.17519, 0, 0, 30.1123, -10.1169},
	{101.847, 47.5413, 0, 0, 0, 0},
	{93.059, 93.059, 0, 0, 0, 0},
	{47.541, 101.847, 0, 0, 0, 0},
	{5.175, 59.481, -10.1169, 30.1123, 0, 0},
	// left handed
	//{229.612, 148.576, -23.9864, 23.9864, 23.9864, -23.9864},
	//{140.516, 169.79, 0, 0, 30.1123, 10.1169},
	//{60.7545, 249.551, -7.49808, 7.49805, 0, 0},
	//{33.6016, 249.551, 0, 0, 7.49808, 7.49811},
	//{6.4487, 222.398, -7.49808, -7.49806, 0, 0},
	//{6.4487, 195.245, 0, 0, -7.49808, 7.49808},
	//{86.2103, 115.484, -10.1169, -30.1123, 0, 0},
	//{107.424, 26.3884, 23.9864, -23.9864, -23.9864, 23.9864},
	//{196.519, 5.17519, 0, 0, -30.1123, -10.1169},
	//{154.153, 47.5413, 0, 0, 0, 0},
	//{162.941, 93.059, 0, 0, 0, 0},
	//{208.459, 101.847, 0, 0, 0, 0},
	//{250.825, 59.481, 10.1169, 30.1123, 0, 0},
};
static IRPath const g_tShapesWrench = {itemsof(g_tPointsWrench), g_tPointsWrench};
static IRLayer const g_tLayerWrench = {&g_tCanvasPlain, 0, 1, NULL, &g_tShapesWrench, &g_tManipulate};


// move up & down

static IRPathPoint const g_tPointsMoveUp[] =
{
	{44, 84, 0, 0, 0, 0},
	{128, 0, 0, 0, 0, 0},
	{212, 84, 0, 0, 0, 0},
	{158, 84, 10, 107, 0, 0},
	{45, 252, 68, -28, 91, 13},
	{98, 84, 0, 0, 2, 82},
};
static IRPathPoint const g_tPointsMoveDown[] =
{
	{44, 172, 0, 0, 0, 0},
	{128, 256, 0, 0, 0, 0},
	{212, 172, 0, 0, 0, 0},
	{158, 172, 10, -107, 0, 0},
	{45, 4, 68, 28, 91, -13},
	{98, 172, 0, 0, 2, -82},
};
static IRGridItem const g_tGridMoveUp = {EGIFInteger, 84.0f};
static IRGridItem const g_tGridMoveDown = {EGIFInteger, 172.0f};
static IRCanvas const g_tCanvasMoveUp = {44, 0, 212, 256, 0, 1, NULL, &g_tGridMoveUp};
static IRCanvas const g_tCanvasMoveDown = {44, 0, 212, 256, 0, 1, NULL, &g_tGridMoveDown};
static IRPath const g_tShapesMoveUp = {itemsof(g_tPointsMoveUp), g_tPointsMoveUp};
static IRPath const g_tShapesMoveDown = {itemsof(g_tPointsMoveDown), g_tPointsMoveDown};
static IRLayer const g_tLayerMoveUp = {&g_tCanvasMoveUp, 0, 1, NULL, &g_tShapesMoveUp, &g_tManipulate};
static IRLayer const g_tLayerMoveDown = {&g_tCanvasMoveDown, 0, 1, NULL, &g_tShapesMoveDown, &g_tManipulate};


// heart

static IRPathPoint const g_tPointsHeart[] =
{
	{128, 256, 31, -32, -31, -32},
	{256, 91, 0, -62, 0, 78},
	{128, 48, -50, -57, 50, -57},
	{0, 91, 0, 78, 0, -62},
};
static IRPath const g_tShapesHeart = {itemsof(g_tPointsHeart), g_tPointsHeart};
static IRFill g_tHeartFill(0xffe34444);//d52a2a);
static IROutlinedFill g_tHeart(&g_tHeartFill, &g_tContrast, g_fOutlineWidthRelative, g_fOutlineWidthAbsolute);
static IRLayer const g_tLayerHeart = {&g_tCanvasPlain, 0, 1, NULL, &g_tShapesHeart, &g_tHeart};

// directions

static IRPolyPoint const g_tPointsDirectionUp[] = { {0, 213}, {128, 37}, {256, 213} };
static IRPolygon const g_tShapesDirectionUp = {itemsof(g_tPointsDirectionUp), g_tPointsDirectionUp};
static IRGridItem const g_tGridDirectionUp = {EGIFInteger, 213.0f};
static IRCanvas const g_tCanvasDirectionUp = {0, 0, 256, 256, 0, 1, NULL, &g_tGridDirectionUp};
static IRLayer const g_tLayerDirectionUp = {&g_tCanvasDirectionUp, 1, 0, &g_tShapesDirectionUp, NULL, &g_tInterior};

static IRPolyPoint const g_tPointsDirectionLeft[] = { {213, 0}, {37, 128}, {213, 256} };
static IRPolygon const g_tShapesDirectionLeft = {itemsof(g_tPointsDirectionLeft), g_tPointsDirectionLeft};
static IRCanvas const g_tCanvasDirectionLeft = {0, 0, 256, 256, 1, 0, &g_tGridDirectionUp, NULL};
static IRLayer const g_tLayerDirectionLeft = {&g_tCanvasDirectionLeft, 1, 0, &g_tShapesDirectionLeft, NULL, &g_tInterior};

static IRPolyPoint const g_tPointsDirectionDown[] = { {256, 43}, {128, 219}, {0, 43} };
static IRPolygon const g_tShapesDirectionDown = {itemsof(g_tPointsDirectionDown), g_tPointsDirectionDown};
static IRGridItem const g_tGridDirectionDown = {EGIFInteger, 43.0f};
static IRCanvas const g_tCanvasDirectionDown = {0, 0, 256, 256, 0, 1, NULL, &g_tGridDirectionDown};
static IRLayer const g_tLayerDirectionDown = {&g_tCanvasDirectionDown, 1, 0, &g_tShapesDirectionDown, NULL, &g_tInterior};

static IRPolyPoint const g_tPointsDirectionRight[] = { {43, 256}, {219, 128}, {43, 0} };
static IRPolygon const g_tShapesDirectionRight = {itemsof(g_tPointsDirectionRight), g_tPointsDirectionRight};
static IRCanvas const g_tCanvasDirectionRight = {0, 0, 256, 256, 1, 0, &g_tGridDirectionDown, NULL};
static IRLayer const g_tLayerDirectionRight = {&g_tCanvasDirectionRight, 1, 0, &g_tShapesDirectionRight, NULL, &g_tInterior};

// structure

static IRPolyPoint const g_tPointsStructure1[] = { {34, 0}, {103, 0}, {138, 60}, {103, 120}, {34, 120}, {0, 60} };
static IRPolygon const g_tShapesStructure1 = {itemsof(g_tPointsStructure1), g_tPointsStructure1};
static IRGridItem const g_tGridStructure1[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 120.0f} };
static IRCanvas const g_tCanvasStructure1 = {0, 0, 256, 256, 0, itemsof(g_tGridStructure1), NULL, g_tGridStructure1};
static IRPolyPoint const g_tPointsStructure2[] = { {152, 68}, {221, 68}, {256, 128}, {221, 188}, {152, 188}, {118, 128} };
static IRPolygon const g_tShapesStructure2 = {itemsof(g_tPointsStructure2), g_tPointsStructure2};
static IRGridItem const g_tGridStructure2[] = { {EGIFInteger, 68.0f}, {EGIFInteger, 188.0f} };
static IRCanvas const g_tCanvasStructure2 = {0, 0, 256, 256, 0, itemsof(g_tGridStructure2), NULL, g_tGridStructure2};
static IRPolyPoint const g_tPointsStructure3[] = { {34, 136}, {103, 136}, {138, 196}, {103, 256}, {34, 256}, {0, 196} };
static IRPolygon const g_tShapesStructure3 = {itemsof(g_tPointsStructure3), g_tPointsStructure3};
static IRGridItem const g_tGridStructure3[] = { {EGIFInteger, 136.0f}, {EGIFInteger, 256.0f} };
static IRCanvas const g_tCanvasStructure3 = {0, 0, 256, 256, 0, itemsof(g_tGridStructure3), NULL, g_tGridStructure3};
static IRLayer const g_tLayerStructure[] =
{
	{&g_tCanvasStructure1, 1, 0, &g_tShapesStructure1, NULL, &g_tInterior},
	{&g_tCanvasStructure2, 1, 0, &g_tShapesStructure2, NULL, &g_tInterior},
	{&g_tCanvasStructure3, 1, 0, &g_tShapesStructure3, NULL, &g_tButton},
};

// picture

static IRPolyPoint const g_tPointsPictureFrame[] = { {0, 32}, {256, 32}, {256, 224}, {0, 224} };
static IRPolygon const g_tShapesPictureFrame = {itemsof(g_tPointsPictureFrame), g_tPointsPictureFrame};
static IRGridItem const g_tGridPictureFrameX[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 256.0f} };
static IRGridItem const g_tGridPictureFrameY[] = { {EGIFInteger, 32.0f}, {EGIFInteger, 224.0f} };
static IRCanvas const g_tCanvasPictureFrame = {0, 32, 256, 224, itemsof(g_tGridPictureFrameX), itemsof(g_tGridPictureFrameY), g_tGridPictureFrameX, g_tGridPictureFrameY};
static IRLayer const g_tLayerPictureFrame = {&g_tCanvasPictureFrame, 1, 0, &g_tShapesPictureFrame, NULL, &g_tInterior};
static IRLayer const g_tLayerPictureFrameLarge = {&g_tCanvasPictureFrame, 1, 0, &g_tShapesPictureFrame, NULL, &g_tInteriorLarge};

static IRPolyPoint const g_tPointsPictureSky[] = { {24, 56}, {232, 56}, {232, 160}, {24, 160} };
static IRPolygon const g_tShapesPictureSky = {itemsof(g_tPointsPictureSky), g_tPointsPictureSky};
static IRGridItem const g_tGridPictureX[] = { {EGIFInteger, 24.0f}, {EGIFInteger, 232.0f} };
static IRGridItem const g_tGridPictureY[] = { {EGIFInteger, 56.0f}, {EGIFInteger, 200.0f} };
static IRCanvas const g_tCanvasPicture = {4, 36, 252, 220, itemsof(g_tGridPictureX), itemsof(g_tGridPictureY), g_tGridPictureX, g_tGridPictureY};
static IRFill const g_tMatPictureSky(0xffffdc50);
static IRLayer const g_tLayerPictureSky = {&g_tCanvasPicture, 1, 0, &g_tShapesPictureSky, NULL, &g_tMatPictureSky};

static IRPathPoint const g_tPointsPictureSunLarge[] =
{
	{132, 120, 0, -24.3005, 0, 24.3005},
	{88, 76, -24.3005, 0, 24.3005, 0},
	{44, 120, 0, 24.3005, 0, -24.3005},
	{88, 164, 24.3005, 0, -24.3005, 0},
};
static IRPathPoint const g_tPointsPictureSunSmall[] =
{
	{115, 95, 0, -16.5685, 0, 16.5685},
	{85, 65, -16.5685, 0, 16.5685, 0},
	{55, 95, 0, 16.5685, 0, -16.5685},
	{85, 125, 16.5685, 0, -16.5685, 0},
};
static IRPath const g_tShapesPictureSunLarge = {itemsof(g_tPointsPictureSunLarge), g_tPointsPictureSunLarge};
static IRPath const g_tShapesPictureSunSmall = {itemsof(g_tPointsPictureSunSmall), g_tPointsPictureSunSmall};
static IRFill const g_tMatPictureSun(0xfffffab7);
static IRLayer const g_tLayerPictureSunLarge = {&g_tCanvasPicture, 0, 1, NULL, &g_tShapesPictureSunLarge, &g_tMatPictureSun};
static IRLayer const g_tLayerPictureSunSmall = {&g_tCanvasPicture, 0, 1, NULL, &g_tShapesPictureSunSmall, &g_tMatPictureSun};

static IRPathPoint const g_tPointsPictureDune1[] =
{
	{232, 134, -40, -25, 0, 0},
	{144, 125, -62.1146, 41.4098, 36, -24},
	{24, 135, 0, 0, 37, -18},
	{24, 189, 0, 0, 0, 0},
	{232, 189, 0, 0, 0, 0},
};
static IRPath const g_tShapesPictureDune1 = {itemsof(g_tPointsPictureDune1), g_tPointsPictureDune1};
static IRFill const g_tMatPictureDune1(0xffd47b1d);
static IRLayer const g_tLayerPictureDune1 = {&g_tCanvasPicture, 0, 1, NULL, &g_tShapesPictureDune1, &g_tMatPictureDune1};

static IRPathPoint const g_tPointsPictureDune2[] =
{
	{24, 157, 30, -16, 0, 0},
	{147, 164, 0, 0, -36, -21},
	{146, 186, 0, 0, 0, 0},
	{24, 186, 0, 0, 0, 0},
};
static IRPath const g_tShapesPictureDune2 = {itemsof(g_tPointsPictureDune2), g_tPointsPictureDune2};
static IRFill const g_tMatPictureDune2(0xfff0882f);
static IRLayer const g_tLayerPictureDune2 = {&g_tCanvasPicture, 0, 1, NULL, &g_tShapesPictureDune2, &g_tMatPictureDune2};

static IRPathPoint const g_tPointsPictureDune3[] =
{
	{24, 175, 71, 2, 0, 0},
	{232, 154, 0, 0, -84, -22},
	{232, 200, 0, 0, 0, 0},
	{24, 200, 0, 0, 0, 0},
};
static IRPath const g_tShapesPictureDune3 = {itemsof(g_tPointsPictureDune3), g_tPointsPictureDune3};
static IRFill const g_tMatPictureDune3(0xffefa940);
static IRLayer const g_tLayerPictureDune3 = {&g_tCanvasPicture, 0, 1, NULL, &g_tShapesPictureDune3, &g_tMatPictureDune3};

static IRPathPoint const g_tPointsPictureCactusShadow[] =
{
	{108, 192, 11.9531, 5.83984, -14.3437, -7.00781},
	{166, 183, 4.78123, 1.16798, 0, 18.6875},
	{215, 191, 19.125, -18.6875, -8.98473, 8.77922},
	{167, 172, -2.21422, 2.49641, 15.539, -17.5195},
	{154, 174, -43.0312, -12.8477, 10.2245, 3.05269},
};
static IRPath const g_tShapesPictureCactusShadow = {itemsof(g_tPointsPictureCactusShadow), g_tPointsPictureCactusShadow};
static IRFill const g_tMatPictureCactusShadow(0x80000000);
static IRLayer const g_tLayerPictureCactusShadow = {&g_tCanvasPicture, 0, 1, NULL, &g_tShapesPictureCactusShadow, &g_tMatPictureCactusShadow};

static IRPathPoint const g_tPointsPictureCactus1[] =
{
	{169, 178, 0, -5.83984, 0, 5.83986},
	{169, 146, -17.9296, -2.33594, 0, 9.34377},
	{138, 97, 0, -14.0156, 0, 36.2259},
	{156, 97, 0, 24.5274, 0, -11.6797},
	{170, 128, 0, -7.00783, -8.36717, 0},
	{169, 75, 0, -19.8555, 0, 10.5117},
	{194, 75, 0, 12.8477, 0, -19.8555},
	{194, 146, 10.7578, 0, 0, -7.00781},
	{209, 128, 0, -15.1836, 0, 10.5117},
	{227, 129, 0, 26.8633, 0, -15.1836},
	{194, 167, 0, 2.33595, 13.1484, -1.16797},
	{194, 179, 0, 3.50391, 0, -3.50391},
};
static IRPathPoint const g_tPointsPictureCactus2[] =
{
	{106, 186, -10.7578, -18.6875, 10.7578, 4.67188},
	{129, 153, 19.125, 0, -19.125, 0},
	{152, 186, -10.7578, 4.67188, 10.7578, -17.5195},
};
static IRPath const g_tShapesPictureCactus[] = { {itemsof(g_tPointsPictureCactus1), g_tPointsPictureCactus1}, {itemsof(g_tPointsPictureCactus2), g_tPointsPictureCactus2} };
static IRFill const g_tMatPictureCactus(0xff59a334);
static IRLayer const g_tLayerPictureCactus = {&g_tCanvasPicture, 0, itemsof(g_tShapesPictureCactus), NULL, g_tShapesPictureCactus, &g_tMatPictureCactus};

static IRPathPoint const g_tPointsPictureCactusHilight1[] =
{
	{200, 145, 6.43588, -2.42496, 5.14804, -0.238495},
	{209, 128, 0, -7.59181, 0, 8.41504},
	{218, 117, -2.98827, 4.96387, -4.48241, -0.291992},
	{211, 129, -1.19531, 15.1836, 0, -7.59181},
};
static IRPathPoint const g_tPointsPictureCactusHilight2[] =
{
	{173, 181, -2.04375, -0.658569, 0.215179, -23.997},
	{169, 178, 0, -5.83984, 0, 1.34273},
	{169, 146, 2.39063, 3.50391, 0, 9.34377},
};
static IRPathPoint const g_tPointsPictureCactusHilight3[] =
{
	{170, 128, 0, -7.00783, 1.19531, -1.16797},
	{169, 75, 0, -10.2347, 0, 10.5117},
	{182, 60, -5.96085, 4.36565, -6.46323, -0.316467},
	{173, 77, 0, 12.8477, 0, -9.62077},
	{173, 125, -1.19531, 1.16797, 0, -7.00783},
};
static IRPathPoint const g_tPointsPictureCactusHilight4[] =
{
	{161, 144, -7.17186, -1.16797, -5.97655, -3.50391},
	{138, 97, 0, -5.42542, 0, 36.2259},
	{144, 88, -2.39406, 5.82825, -3.29332, 0.931511},
	{142, 103, 0, 26.8633, 0, -7.15851},
};
static IRPathPoint const g_tPointsPictureCactusHilight5[] =
{
	{106, 186, -10.7578, -18.6875, 1.19531, 1.16797},
	{129, 153, -19.125, 5.83986, -19.125, 0},
	{111, 187, -2.39061, 0, -9.56247, -14.0156},
};
static IRPath const g_tShapesPictureCactusHilight[] =
{
	{itemsof(g_tPointsPictureCactusHilight1), g_tPointsPictureCactusHilight1},
	{itemsof(g_tPointsPictureCactusHilight2), g_tPointsPictureCactusHilight2},
	{itemsof(g_tPointsPictureCactusHilight3), g_tPointsPictureCactusHilight3},
	{itemsof(g_tPointsPictureCactusHilight4), g_tPointsPictureCactusHilight4},
	{itemsof(g_tPointsPictureCactusHilight5), g_tPointsPictureCactusHilight5},
};
static IRFill const g_tMatPictureCactusHilight(0xffc5d639);
static IRLayer const g_tLayerPictureCactusHilight = {&g_tCanvasPicture, 0, itemsof(g_tShapesPictureCactusHilight), NULL, g_tShapesPictureCactusHilight, &g_tMatPictureCactusHilight};


// properties

static IRPolyPoint const g_tPointsPropertiesFrame[] = { {256, 64}, {148, 64}, {108, 20}, {0, 20}, {0, 216}, {256, 216} };
static IRPolygon const g_tShapesPropertiesFrame = {itemsof(g_tPointsPropertiesFrame), g_tPointsPropertiesFrame};
static IRGridItem const g_tGridPropertiesX[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 256.0f} };
static IRGridItem const g_tGridPropertiesY[] = { {EGIFInteger, 20.0f}, {EGIFInteger, 64.0f}, {EGIFInteger, 216.0f} };
static IRCanvas const g_tCanvasProperties = {0, 0, 256, 256, itemsof(g_tGridPropertiesX), itemsof(g_tGridPropertiesY), g_tGridPropertiesX, g_tGridPropertiesY};
static IRLayer const g_tLayerPropertiesFrame = {&g_tCanvasProperties, 1, 0, &g_tShapesPropertiesFrame, NULL, &g_tInterior};

static IRPolyPoint const g_tPointsPropertiesPt1[] = { {27, 102}, {41, 88}, {55, 102}, {41, 116} };
static IRPolyPoint const g_tPointsPropertiesPt2[] = { {27, 140}, {41, 126}, {55, 140}, {41, 154} };
static IRPolyPoint const g_tPointsPropertiesPt3[] = { {27, 178}, {41, 164}, {55, 178}, {41, 192} };
static IRPolygon const g_tShapesPropertiesPts[] = { {itemsof(g_tPointsPropertiesPt1), g_tPointsPropertiesPt1}, {itemsof(g_tPointsPropertiesPt2), g_tPointsPropertiesPt2}, {itemsof(g_tPointsPropertiesPt3), g_tPointsPropertiesPt3} };
static IRPolyPoint const g_tPointsPropertiesLn1[] = { {68.5, 102.5}, {80.5, 94.5}, {90.5, 110.5}, {104.5, 109.5}, {118.5, 94.5}, {123.5, 109.5}, {138.5, 98.5}, {151.5, 109.5}, {159.5, 100.5} };
static IRPolyPoint const g_tPointsPropertiesLn2[] = { {65.5, 140.5}, {78.5, 145.5}, {87.5, 134.5}, {99.5, 143.5}, {112.5, 143.5}, {120.5, 131.5}, {129.5, 144.5}, {144.5, 143.5}, {155.5, 133.5}, {172.5, 133.5}, {177.5, 145.5}, {189.5, 140.5} };
static IRPolyPoint const g_tPointsPropertiesLn3[] = { {67.5, 178.5}, {73.5, 167.5}, {82.5, 184.5}, {95.5, 175.5}, {105.5, 182.5}, {114.5, 168.5}, {128.5, 178.5} };
static IRPolygon const g_tShapesPropertiesLns[] = { {itemsof(g_tPointsPropertiesLn1), g_tPointsPropertiesLn1}, {itemsof(g_tPointsPropertiesLn2), g_tPointsPropertiesLn2}, {itemsof(g_tPointsPropertiesLn3), g_tPointsPropertiesLn3} };
static IRLayer const g_tLayerPropertiesPts = {&g_tCanvasProperties, itemsof(g_tShapesPropertiesPts), 0, g_tShapesPropertiesPts, NULL, &g_tContrast};
static IRLayer const g_tLayerPropertiesLns = {&g_tCanvasProperties, itemsof(g_tShapesPropertiesLns), 0, g_tShapesPropertiesLns, NULL, &g_tOutlinePen};


// settings

static IRPathPoint const g_tPointsSettingsRail1[] =
{
	{22, 30, 0, 0, -12.1503, 0},
	{234, 30, 12.1503, 0, 0, 0},
	{256, 52, 0, 12.1503, 0, -12.1503},
	{234, 74, 0, 0, 12.1503, 0},
	{22, 74, -12.1503, 0, 0, 0},
	{0, 52, 0, -12.1503, 0, 12.1503},
	//{0, 44, 0, -8.83656, 0, 0},
	//{16, 28, 0, 0, -8.83656, 0},
	//{240, 28, 8.83656, 0, 0, 0},
	//{256, 44, 0, 0, 0, -8.83656},
	//{256, 60, 0, 8.83656, 0, 0},
	//{240, 76, 0, 0, 8.83656, 0},
	//{16, 76, -8.83656, 0, 0, 0},
	//{0, 60, 0, 0, 0, 8.83656},
};
static IRPathPoint const g_tPointsSettingsRail2[] =
{
	{22, 110, 0, 0, -12.1503, 0},
	{234, 110, 12.1503, 0, 0, 0},
	{256, 132, 0, 12.1503, 0, -12.1503},
	{234, 154, 0, 0, 12.1503, 0},
	{22, 154, -12.1503, 0, 0, 0},
	{0, 132, 0, -12.1503, 0, 12.1503},
	//{0, 124, 0, -8.83656, 0, 0},
	//{16, 108, 0, 0, -8.83656, 0},
	//{240, 108, 8.83656, 0, 0, 0},
	//{256, 124, 0, 0, 0, -8.83656},
	//{256, 140, 0, 8.83656, 0, 0},
	//{240, 156, 0, 0, 8.83656, 0},
	//{16, 156, -8.83656, 0, 0, 0},
	//{0, 140, 0, 0, 0, 8.83656},
};
static IRPathPoint const g_tPointsSettingsRail3[] =
{
	{22, 190, 0, 0, -12.1503, 0},
	{234, 190, 12.1503, 0, 0, 0},
	{256, 212, 0, 12.1503, 0, -12.1503},
	{234, 234, 0, 0, 12.1503, 0},
	{22, 234, -12.1503, 0, 0, 0},
	{0, 212, 0, -12.1503, 0, 12.1503},
	//{0, 204, 0, -8.83656, 0, 0},
	//{16, 188, 0, 0, -8.83656, 0},
	//{240, 188, 8.83656, 0, 0, 0},
	//{256, 204, 0, 0, 0, -8.83656},
	//{256, 220, 0, 8.83656, 0, 0},
	//{240, 236, 0, 0, 8.83656, 0},
	//{16, 236, -8.83656, 0, 0, 0},
	//{0, 220, 0, 0, 0, 8.83656},
};
static IRPath const g_tShapesSettingsRails[] = { {itemsof(g_tPointsSettingsRail1), g_tPointsSettingsRail1}, {itemsof(g_tPointsSettingsRail2), g_tPointsSettingsRail2}, {itemsof(g_tPointsSettingsRail3), g_tPointsSettingsRail3} };
static IRGridItem const g_tGridSettingsX[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 256.0f} };
static IRGridItem const g_tGridSettingsY[] = { {EGIFInteger, 30.0f}, {EGIFInteger|1, 74.0f}, {EGIFInteger|2, 110.0f}, {EGIFInteger|1, 154.0f}, {EGIFInteger|2, 190.0f}, {EGIFInteger|1, 234.0f} };
//static IRGridItem const g_tGridSettingsY[] = { {EGIFInteger, 28.0f}, {EGIFInteger, 76.0f}, {EGIFInteger, 108.0f}, {EGIFInteger, 156.0f}, {EGIFInteger, 188.0f}, {EGIFInteger, 236.0f} };
static IRCanvas const g_tCanvasSettings = {0, 0, 256, 256, itemsof(g_tGridSettingsX), itemsof(g_tGridSettingsY), g_tGridSettingsX, g_tGridSettingsY};
static IRLayer const g_tLayerSettingsRails = {&g_tCanvasSettings, 0, itemsof(g_tShapesSettingsRails), NULL, g_tShapesSettingsRails, &g_tBackground};

static IRPathPoint const g_tPointsSettingsKnob1[] =
{
	{243, 52, 0, -22.0914, 0, 22.0914},
	{203, 12, -22.0914, 0, 22.0914, 0},
	{163, 52, 0, 22.0914, 0, -22.0914},
	{203, 92, 22.0914, 0, -22.0914, 0},
};
static IRPathPoint const g_tPointsSettingsKnob2[] =
{
	{124, 132, 0, -22.0914, 0, 22.0914},
	{84, 92, -22.0914, 0, 22.0914, 0},
	{44, 132, 0, 22.0914, 0, -22.0914},
	{84, 172, 22.0914, 0, -22.0914, 0},
};
static IRPathPoint const g_tPointsSettingsKnob3[] =
{
	{198, 212, 0, -22.0914, 0, 22.0914},
	{158, 172, -22.0914, 0, 22.0914, 0},
	{118, 212, 0, 22.0914, 0, -22.0914},
	{158, 252, 22.0914, 0, -22.0914, 0},
};
static IRPath const g_tShapesSettingsKnobs[] = { {itemsof(g_tPointsSettingsKnob1), g_tPointsSettingsKnob1}, {itemsof(g_tPointsSettingsKnob2), g_tPointsSettingsKnob2}, {itemsof(g_tPointsSettingsKnob3), g_tPointsSettingsKnob3} };
static IRLayer const g_tLayerSettingsKnobs = {&g_tCanvasSettings, 0, itemsof(g_tShapesSettingsKnobs), NULL, g_tShapesSettingsKnobs, &g_tConfirm};



class ATL_NO_VTABLE CStockIcons : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStockIcons, &__uuidof(StockIcons)>,
	public IStockIcons
{
public:
	CStockIcons() : m_eWanted(ECSDefault), m_eLast(ECSIvalid)
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CStockIcons)

BEGIN_COM_MAP(CStockIcons)
	COM_INTERFACE_ENTRY(IStockIcons)
END_COM_MAP()


	// IStockIcons methods
public:
    virtual DWORD STDMETHODCALLTYPE GetSRGBColor(ULONG id)
	{
		ValidateColorScheme();

		switch (id&ESMMainCategoryMask)
		{
		case ESMContrast:	return g_tContrast.color;
		case ESMBackground:	return g_tBackgroundFill.color;
		case ESMAltBackground:	return g_tButtonFill.color;
		case ESMInterior:	return g_tInteriorFill.color;
		case ESMConfirm:	return g_tConfirmFill.color;
		case ESMCancel:		return g_tCancelFill.color;
		case ESMHelp:		return g_tHelpFill.color;
		case ESMCreate:		return g_tCreateFill.color;
		case ESMManipulate:	return g_tManipulateFill.color;
		case ESMDelete:		return g_tDeleteFill.color;
		case ESMColor1:
			switch (id&ESMSubCategoryMask)
			{
			case ESMScheme2:	return g_tScheme2Color1Fill.color;
			case ESMScheme3:	return g_tScheme3Color1Fill.color;
			default:			return g_tScheme1Color1Fill.color;
			}
		case ESMColor2:
			switch (id&ESMSubCategoryMask)
			{
			case ESMScheme2:	return g_tScheme2Color2Fill.color;
			case ESMScheme3:	return g_tScheme3Color2Fill.color;
			default:			return g_tScheme1Color2Fill.color;
			}
		case ESMColor3:
			switch (id&ESMSubCategoryMask)
			{
			case ESMScheme2:	return g_tScheme2Color3Fill.color;
			case ESMScheme3:	return g_tScheme3Color3Fill.color;
			default:			return g_tScheme1Color3Fill.color;
			}
		}

		return 0;
	}
    virtual IRMaterial const* STDMETHODCALLTYPE GetMaterial(ULONG id, bool forceSolid)
	{
		ValidateColorScheme();

		switch (id&ESMMainCategoryMask)
		{
		case ESMContrast:		return &g_tContrast;
		case ESMBackground:		return forceSolid ? static_cast<IRMaterial const*>(&g_tBackgroundFill) : &g_tBackground;
		case ESMAltBackground:	return forceSolid ? static_cast<IRMaterial const*>(&g_tButtonFill) : &g_tButton;
		case ESMInterior:		return forceSolid ? static_cast<IRMaterial const*>(&g_tInteriorFill) : &g_tInterior;
		case ESMOutline:		return &g_tOutlinePen;
		case ESMOutlineSoft:	return &g_tOutlinePen50;
		case ESMConfirm:		return forceSolid ? static_cast<IRMaterial const*>(&g_tConfirmFill) : &g_tConfirm;
		case ESMCancel:			return forceSolid ? static_cast<IRMaterial const*>(&g_tCancelFill) : &g_tCancel;
		case ESMHelp:			return forceSolid ? static_cast<IRMaterial const*>(&g_tHelpFill) : &g_tHelp;
		case ESMCreate:			return forceSolid ? static_cast<IRMaterial const*>(&g_tCreateFill) : &g_tCreate;
		case ESMManipulate:		return forceSolid ? static_cast<IRMaterial const*>(&g_tManipulateFill) : &g_tManipulate;
		case ESMDelete:			return forceSolid ? static_cast<IRMaterial const*>(&g_tDeleteFill) : &g_tDelete;
		case ESMBrightLight:	return forceSolid ? static_cast<IRMaterial const*>(&g_tBrightLightFill) : &g_tBrightLight;
		case ESMColor1:
			switch (id&ESMSubCategoryMask)
			{
			case ESMScheme2:	return forceSolid ? static_cast<IRMaterial const*>(&g_tScheme2Color1Fill) : &g_tScheme2Color1;
			case ESMScheme3:	return forceSolid ? static_cast<IRMaterial const*>(&g_tScheme3Color1Fill) : &g_tScheme3Color1;
			default:			return forceSolid ? static_cast<IRMaterial const*>(&g_tScheme1Color1Fill) : &g_tScheme1Color1;
			}
		case ESMColor2:
			switch (id&ESMSubCategoryMask)
			{
			case ESMScheme2:	return forceSolid ? static_cast<IRMaterial const*>(&g_tScheme2Color2Fill) : &g_tScheme2Color2;
			case ESMScheme3:	return forceSolid ? static_cast<IRMaterial const*>(&g_tScheme3Color2Fill) : &g_tScheme3Color2;
			default:			return forceSolid ? static_cast<IRMaterial const*>(&g_tScheme1Color2Fill) : &g_tScheme1Color2;
			}
		case ESMColor3:
			switch (id&ESMSubCategoryMask)
			{
			case ESMScheme2:	return forceSolid ? static_cast<IRMaterial const*>(&g_tScheme2Color3Fill) : &g_tScheme2Color3;
			case ESMScheme3:	return forceSolid ? static_cast<IRMaterial const*>(&g_tScheme3Color3Fill) : &g_tScheme3Color3;
			default:			return forceSolid ? static_cast<IRMaterial const*>(&g_tScheme1Color3Fill) : &g_tScheme1Color3;
			}
		}

		return NULL;
	}
    virtual bool STDMETHODCALLTYPE GetLayers(ULONG id, ILayerReceiver& receiver, IRTarget const* target)
	{
		ULONG style = id&ESIStyleMask;
		switch (id&ESISymbolMask)
		{
		case ESIPlus:
			return receiver(g_tLayerPlus, target);
		case ESIMinus:
			return receiver(g_tLayerMinus, target);
		case ESIConfirm:
			return receiver(g_tLayerCheck, target);
		case ESICancel:
			return receiver(g_tLayerCancel, target);
		case ESIHelp:
			return receiver(g_tLayerCircle, target) &&
				   receiver(g_tLayerQuestion, target);
		case ESICreate:
			return receiver(g_tLayerCreate, target);
		case ESIDuplicate:
			return receiver(g_tLayerDuplicate, target);
		case ESIDelete:
			return receiver(g_tLayerDelete, target);
		case ESIDocument:
			return receiver(g_tLayerDocumentPaper, target) &&
				   receiver(g_tLayerDocumentCorner, target);
		case ESIFolder:
			return receiver(g_tLayerFolderBack, target) &&
				   receiver(g_tLayerFolderFront, target) &&
				   (style == ESISimplified || receiver(g_tLayerFolderClip, target));
		case ESIFloppy:
			return receiver(g_tLayerFloppyOutline, target) &&
				   receiver(g_tLayerFloppyTop, target) &&
				   receiver(g_tLayerFloppyBot, target) &&
				   (style == ESISimplified || receiver(g_tLayerFloppyStrokes, target));
		case ESIMagnifier:
			return receiver(g_tLayerMagnifierGlass, target) &&
				   receiver((id&ESIFancy) ? g_tLayerMagnifierHandleAlt : g_tLayerMagnifierHandle, target) &&
				   receiver(g_tLayerMagnifierHilight, target);
		case ESIModify:
			return receiver(g_tLayerWrench, target);
		case ESIMoveUp:
			return receiver(g_tLayerMoveUp, target);
		case ESIMoveDown:
			return receiver(g_tLayerMoveDown, target);
		case ESIHeart:
			return receiver(g_tLayerHeart, target);
		case ESIDirectionUp:
			return receiver(g_tLayerDirectionUp, target);
		case ESIDirectionDown:
			return receiver(g_tLayerDirectionDown, target);
		case ESIDirectionLeft:
			return receiver(g_tLayerDirectionLeft, target);
		case ESIDirectionRight:
			return receiver(g_tLayerDirectionRight, target);
		case ESIStructure:
			return receiver(g_tLayerStructure[0], target) && receiver(g_tLayerStructure[1], target) && receiver(g_tLayerStructure[2], target);
		case ESIProperties:
			return receiver(g_tLayerPropertiesFrame, target) &&
				(style == ESISimplified || (receiver(g_tLayerPropertiesPts, target) && receiver(g_tLayerPropertiesLns, target)));
		case ESISettings:
			return receiver(g_tLayerSettingsRails, target) && receiver(g_tLayerSettingsKnobs, target);
		case ESIPicture:
			return (id&ESILarge ? receiver(g_tLayerPictureFrameLarge, target) : receiver(g_tLayerPictureFrame, target)) &&
					receiver(g_tLayerPictureSky, target) &&
					(id&ESIFancy ? receiver(g_tLayerPictureSunSmall, target) : receiver(g_tLayerPictureSunLarge, target)) &&
					receiver(g_tLayerPictureDune1, target) &&
					receiver(g_tLayerPictureDune2, target) &&
					receiver(g_tLayerPictureDune3, target) &&
					(0 == (id&ESIFancy) ||
						receiver(g_tLayerPictureCactusShadow, target) &&
						receiver(g_tLayerPictureCactus, target) &&
						receiver(g_tLayerPictureCactusHilight, target));
		}
		return false;
	}

private:
	struct SColors
	{
		DWORD outline;
		DWORD interior;
		DWORD confirm;
		DWORD cancel;
		DWORD modify;
		DWORD help;
		DWORD sch1clr1;
		DWORD sch1clr2;
		DWORD sch1clr3;
		DWORD sch2clr1;
		DWORD sch2clr2;
		DWORD sch2clr3;
		DWORD sch3clr1;
		DWORD sch3clr2;
		DWORD sch3clr3;
	};

	static DWORD BlendColors(DWORD clr1, DWORD clr2, float w1)
	{
		DWORD r = CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetRValue(clr1))*w1 + CGammaTables::FromSRGB(GetRValue(clr2))*(1.0f-w1));
		DWORD g = CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetGValue(clr1))*w1 + CGammaTables::FromSRGB(GetGValue(clr2))*(1.0f-w1));
		DWORD b = CGammaTables::ToSRGB(CGammaTables::FromSRGB(GetBValue(clr1))*w1 + CGammaTables::FromSRGB(GetBValue(clr2))*(1.0f-w1));
		return RGB(r, g, b);
	}

	void ValidateColorScheme()
	{
		if (m_eWanted != m_eLast)
		{
			g_tContrast.color =
			g_tOutlinePen.color = 0xff000000|GetSysColor(COLOR_WINDOWTEXT);
			g_tOutlinePen50.color = g_tContrast.color;
			g_tBackgroundFill.color = 0xff000000|GetSysColor(COLOR_WINDOW);
			g_tInteriorFill.color = 0xff000000|BlendColors(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_3DFACE), 0.5f);
			g_tConfirmFill.color = 0xff91ed91;//0xff96f196;//0xff78f278;
			g_tCancelFill.color = 0xfff69797;
			g_tHelpFill.color = 0xff1157b8;
			g_tCreateFill.color = 0xff8a9cf0;//0xff6782eb;
			g_tManipulateFill.color = 0xff8a9cf0;//0xff6782eb;
			g_tDeleteFill.color = 0xfff69797;
			g_tBrightLightFill.color = 0xffebd467;
			g_tScheme1Color1Fill.color = 0xfffaa5a5;//0xfffababa;//0xffed9191;
			g_tScheme1Color2Fill.color = 0xffb89e7a;//0xffa57f43;//0xffa88a62;//0xffd39a56;
			g_tScheme1Color3Fill.color = 0xffeff5a7;//0xffe8f07e;
			g_tScheme2Color1Fill.color = 0xff6d89eb;
			g_tScheme2Color2Fill.color = 0xfff4d2a5;
			g_tScheme2Color3Fill.color = 0xff83ca83;
			g_tScheme3Color1Fill.color = 0xffc895e3;
			g_tScheme3Color2Fill.color = 0xff97dae5;
			g_tScheme3Color3Fill.color = 0xffead09f;

			m_eLast = m_eWanted;
		}
	}

private:
	EColorScheme m_eWanted;
	EColorScheme m_eLast;
};

OBJECT_ENTRY_AUTO(__uuidof(StockIcons), CStockIcons)
