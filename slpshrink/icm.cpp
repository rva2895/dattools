#include "stdafx.h"

#include "icm.h"
#include "palette.h"
#include <math.h>

using namespace cimg_library;

//ICM S and V coefficients for dark/bringht ICMs
const double ICM_S[9] = {
	0.80,
	0.85,
	0.90,
	0.95,
	1.00,
	0.98,
	0.96,
	0.94,
	0.92
};

const double ICM_V[9] = {
	0.60,
	0.70,
	0.80,
	0.90,
	1.00,
	1.05,
	1.10,
	1.15,
	1.20
};
/*const double ICM_S[9] = {
	0.60,
	0.70,
	0.80,
	0.90,
	1.00,
	0.96,
	0.92,
	0.88,
	0.84
};

const double ICM_V[9] = {
	0.40,
	0.55,
	0.70,
	0.85,
	1.00,
	1.10,
	1.20,
	1.30,
	1.40
};*/

#define CLR_DELTA 4

ICM::ICM(const char* palette)
{
	T_PALETTE pal(palette);
	img = new CImg<unsigned char>(32 * 32, 32 * 9, 1, 3);
	//make ICMs:
	rgb rgb;
	hsv hsv;
	int r, g, b;
	for (int j = 0; j < 32 * 9; j++)
		for (int k = 0; k < 32 * 32; k++)
		{
			r = (k % 32) * 8 + CLR_DELTA;
			g = (j % 32) * 8 + CLR_DELTA;
			b = (k / 32) * 8 + CLR_DELTA;
			rgb.r = (double)r / 255.0;
			rgb.g = (double)g / 255.0;
			rgb.b = (double)b / 255.0;
			hsv = rgb2hsv(rgb);
			hsv.s *= ICM_S[j / 32];
			hsv.v *= ICM_V[j / 32];
			if (hsv.v > 1.0)
				hsv.v = 1.0;
			rgb = hsv2rgb(hsv);
			unsigned char index = pal.get_index(RGB(rgb.r * 255, rgb.g * 255, rgb.b * 255));
			COLORREF c = pal.get_entry(index);
			(*img)(k, j, 0) = GetRValue(c);
			(*img)(k, j, 1) = GetGValue(c);
			(*img)(k, j, 2) = GetBValue(c);
			icms[j / 32][r >> 3][g >> 3][b >> 3] = index;
		}
}

ICM::~ICM()
{
	delete img;
}

void ICM::make_image(const char* filename)
{
	img->save(filename);
}

void ICM::make_icm(const char * filename)
{
	FILE* f;
	fopen_s(&f, filename, "wb");
	if (!f)
	{
		printf(" error: cannot write file\n");
		exit(0);
	}
	fwrite(icms, sizeof(icms), 1, f);
	fwrite(icms[4], sizeof(icms[0]), 1, f);
	fclose(f);
}
