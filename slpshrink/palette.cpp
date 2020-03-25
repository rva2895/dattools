#include "stdafx.h"
#include "palette.h"

T_PALETTE::T_PALETTE(const char* s)
{
	int n;
	char dummy[100];
	FILE* f;
	fopen_s(&f, s, "rt");
	if (!f)
	{
		printf(" error: cannot load palette\n");
		exit(0);
		return;
	}
	fscanf_s(f, "%s", dummy, 100);
	if (strncmp(dummy, "JASC-PAL", 9))
	{
		printf(" error: invalid palette file\n");
		exit(0);
		return;
	}
	fscanf_s(f, "%s", dummy, 100);
	fscanf_s(f, "%d", &n);
	if (n != 256)
	{
		printf(" error: invalid palette file\n");
		exit(0);
		return;
	}
	
	LOGPALETTE* lp = (LOGPALETTE*) malloc(sizeof(PALETTEENTRY) * 256 + sizeof(LOGPALETTE));
	lp->palVersion = 0x300;
	lp->palNumEntries = 256;
	for (int i = 0; i < 256; i++)
	{
		int r, g, b;
		fscanf_s(f, "%d %d %d", &r, &g, &b);
		lp->palPalEntry[i].peRed = r;
		lp->palPalEntry[i].peGreen = g;
		lp->palPalEntry[i].peBlue = b;
		lp->palPalEntry[i].peFlags = 0;
		entries[i] = RGB(r, g, b);
	}
	pal = CreatePalette(lp);
}

int T_PALETTE::get_index(COLORREF c)
{
	int index;
	auto it = cache.find(c);
	if (it == cache.end())
	{
		index = GetNearestPaletteIndex(pal, c);
		if (cache.size() < 256)
			cache.insert(std::pair<COLORREF, int>(c, index));
	}
	else
		index = it->second;
	return index;
}

COLORREF T_PALETTE::get_entry(int index)
{
	return entries[index];
}
