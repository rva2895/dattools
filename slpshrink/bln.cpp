#include "stdafx.h"
#include "bln.h"

#include "palette.h"

BLN::BLN(const char* palette)
{
	entries = (unsigned char*)malloc(256 * 256 * 20);

	T_PALETTE pal(palette);

	for (int k = 0; k < 20; k++)
		for (int j = 0; j < 256; j++)
			for (int i = 0; i < 256; i++)
			{
				COLORREF cb = pal.get_entry(i);
				COLORREF cf = pal.get_entry(j);
				rgb rgb_b = { GetRValue(cb) / 255.0, GetGValue(cb) / 255.0, GetBValue(cb) / 255.0 };
				rgb rgb_f = { GetRValue(cf) / 255.0, GetGValue(cf) / 255.0, GetBValue(cf) / 255.0 };
				rgb result = mix(rgb_f, rgb_b, k / 20.0, 1.0);
				unsigned char index = pal.get_index(RGB(result.r * 255, result.g * 255, result.b * 255));
				entries[k * 256 * 256 + j * 256 + i] = index;
			}
}

void BLN::make_bln(const char* filename)
{
	FILE* f;
	fopen_s(&f, filename, "wb");
	if (!f)
	{
		printf(" error: cannot write file\n");
		exit(0);
	}
	float float_1 = 1.0f;
	fwrite(&float_1, sizeof(float_1), 1, f);
	fwrite(entries, 256 * 256 * 20, 1, f);
	fclose(f);
}

BLN::~BLN()
{
	free(entries);
}
