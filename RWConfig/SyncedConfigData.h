
#pragma once

struct TScrollInfo
{
	TScrollInfo() : nRangeH(400), nRangeV(500), nPageH(400), nPageV(500), nActPosH(0), nActPosV(0) {};

	int nRangeH;
	int nRangeV;
	int nPageH;
	int nPageV;
	int nActPosH;
	int nActPosV;
};

struct TColumnInfo
{
	TColumnInfo() : nWidthName(150), nWidthValue(250) {}
	int nWidthName;
	int nWidthValue;
};

