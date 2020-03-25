// slpshrink.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "slp.h"
#include "icm.h"
#include "palette.h"
#include "bln.h"

int main(int argc, char* argv[])
{
#ifdef SLPSHRINK
	SLP slp;
#ifdef _DEBUG
	slp.load("C:\\Users\\Vasya\\Documents\\Visual Studio 2015\\Projects\\slpshrink\\Release\\4790-1.slp");//4732
	//slp.load("C:\\Users\\Vasya\\Documents\\Visual Studio 2015\\Projects\\slpshrink\\Release\\7850.slp");//4700
	slp.optimize("test.slp", true);
#else
	if (argc == 2)
	{
		slp.load(argv[1]);
		long olds = slp.getSize();
		slp.optimize(argv[1], true);
		long news = slp.getSize();
		printf("%s reduced from %d to %d bytes (%.2f%% reduction)\n", argv[1], olds, news, (float)(olds - news) * 100 / olds);
	}
	else if (argc == 3)
	{
		slp.load(argv[1]);
		//long color;
		//sscanf_s(argv[2], "%d", &color);
		//long pixel = slp.count_pixels(color);
		//printf("%s: pixels total: %d, color %d: %d (%.2f%%)\n",
		//	argv[1], slp.get_pixel_count(), color, pixel, (float)pixel * 100 / slp.get_pixel_count());
		long pixel_count[256];
		memset(&pixel_count, 0, 256 * sizeof(long));
		long total = slp.count_pixels(pixel_count);
		if (total == 0)
			total = 1;
		printf("%s", argv[1]);
		for (int i = 0; i < 256; i++)
			printf(",%d", (long)((float)pixel_count[i] * 10000 / total));
		printf("\n");
	}
	else
		printf("Usage: slpshrink image.slp\n");
#endif
	//system("pause");
#endif
#ifdef TERRAINMAP
	SLP slp;
#ifdef _DEBUG
	slp.load_terrainmap("C:\\Users\\Vasya\\Documents\\SWGB\\Screenshots\\map\\0.png",
		"C:\\Users\\Vasya\\Documents\\Visual Studio 2015\\Projects\\slpshrink\\swgb_default.pal", 2, 2);
	slp.optimize("test.slp", false);
#else
	if ((argc != 4) && (argc != 6))
	{
		printf("terrainmap v1.0 - Create SWGB/AOC terrain SLP from terrain map\n\n"
			"    Usage: terrainmap <palette> <image> <slp> [<size SW-NE> <size NW-SE>]\n\n"
			"Image must be in BMP or PNG format and have the following dimensions:\n"
			"             48*width+1 x 24*height+1\n"
			"If map size is not specified, square map is assumed\n");
		return 0;
	}

	printf("Loading image...");
	int dx = -1; int dy = -1;
	if (argc == 6)
	{
		sscanf_s(argv[4], "%d", &dx);
		sscanf_s(argv[5], "%d", &dy);
		if ((dx < 0) || (dy < 0))
		{
			printf("Error: invalid arguments\n");
			return 0;
		}
	}
	if (slp.load_terrainmap(argv[2], argv[1], dx, dy))
	{
		printf(" OK\nWriting SLP...");
		slp.optimize(argv[3], false);
		printf(" OK\nOperation completed successfully\n");
	}
	else
		printf(" error: invalid map dimensions\n");
#endif
#endif
#ifdef COLORREPLACE
	SLP slp;
#ifdef _DEBUG
	slp.load("C:\\Users\\Vasya\\Documents\\SWGB\\Data\\graphics_x2\\4700.slp");
	unsigned char c1[1] = { 0 };
	unsigned char c2[1] = { 0 };
	slp.color_replace(c1, c2, 1);
	slp.optimize("test.slp", false);
#else
	if ((argc < 3) && (argc != 5))
	{
		printf("colorreplace v1.0 - Replace color in an SLP\n\n"
			"    Usage: colorreplace <slp> [-n] {color replace list}\n\n"
			"        -n: don't use fill command (useful for terrain SLPs only)\n\n"
			"        color replace list: palette indices to replace (0-255)\n"
			"Example:\n\n    colorreplace 1.slp 0-48 255-135\n"
			"will replace color 0 with color 48 and color 255 with color 135\n");
		return 0;
	}
	if (!slp.load(argv[1]))
	{
		printf("Error: cannot load SLP\n");
		return 0;
	}
	int start_arg = 2;
	bool fill_flag = true;
	if (!strcmp(argv[2], "-n"))
	{
		fill_flag = false;
		start_arg++;
	}
	unsigned char* c1 = (unsigned char*)malloc(argc - start_arg);
	unsigned char* c2 = (unsigned char*)malloc(argc - start_arg);
	int arg1 = -1; int arg2 = -1;
	for (int i = start_arg; i < argc; i++)
	{
		sscanf_s(argv[i], "%d-%d", &arg1, &arg2);
		if ((arg1 < 0) || (arg1 > 255) || (arg2 < 0) || (arg2 > 255))
		{
			printf("Error: invalid parameters\n");
			return 0;
		}
		c1[i - start_arg] = arg1;
		c2[i - start_arg] = arg2;
	}
	slp.color_replace(c1, c2, argc - start_arg);
	slp.optimize(argv[1], fill_flag);

	printf("Operation completed successfully\n");
#endif
#endif
#ifdef ICMTOOL
#ifdef _DEBUG
	//ICM icm("C:\\Users\\Vasya\\Documents\\Visual Studio 2015\\Projects\\slpshrink\\swgb_default.pal");
	//icm.make_image("icm_d.png");
	//icm.make_icm("VIEW_ICM.dat");
	/*T_PALETTE pal("C:\\Users\\Vasya\\Documents\\Visual Studio 2015\\Projects\\slpshrink\\swgb_default.pal");
	FILE* f;
	fopen_s(&f, "shadow.pal", "wt");
	if (!f)
	{
		printf(" error: cannot write file\n");
		exit(0);
	}
	fprintf_s(f, "JASC-PAL\n0100\n256\n");
	FILE* f1;
	fopen_s(&f1, "C:\\Games\\Star Wars Galactic Battlegrounds\\Data\\SHADOW.COL", "rt");
	int c;
	COLORREF col;
	for (int i = 0; i < 256; i++)
	{
		fscanf_s(f1, "%d", &c);
		col = pal.get_entry(c);
		fprintf_s(f, "%d %d %d\n", GetRValue(col), GetGValue(col), GetBValue(col));
	}
	fclose(f);
	fclose(f1);*/
	//T_PALETTE pal("C:\\Users\\Vasya\\Documents\\Visual Studio 2015\\Projects\\slpshrink\\swgb_default.pal");
	T_PALETTE pal("C:\\Users\\Vasya\\Downloads\\EF50500_b9.pal");
	FILE* f;
	fopen_s(&f, "shadow-new.pal", "wt");
	FILE* g;
	fopen_s(&g, "shadow.col", "wt");
	if (!f)
	{
		printf(" error: cannot write file\n");
		exit(0);
	}
	fprintf_s(f, "JASC-PAL\n0100\n256\n");
	int c;
	COLORREF col;
	for (int i = 0; i < 256; i++)
	{
		col = pal.get_entry(i);
		hsv hsv;
		rgb rgb;
		rgb.r = (double)GetRValue(col) / 255;
		rgb.g = (double)GetGValue(col) / 255;
		rgb.b = (double)GetBValue(col) / 255;
		hsv = rgb2hsv(rgb);
		hsv.s *= 0.70; //70
		hsv.v *= 0.50; //50
		rgb = hsv2rgb(hsv);
		col = RGB(rgb.r * 255, rgb.g * 255, rgb.b * 255);
		col = pal.get_entry(pal.get_index(col));
		fprintf_s(f, "%d %d %d\n", GetRValue(col), GetGValue(col), GetBValue(col));
		fprintf_s(g, "%d\n", pal.get_index(col));
	}
	fclose(f);
	fclose(g);
#else
	if (argc != 4)
	{
		printf("icmtool v1.0 - Create ICM from palette\n\n"
			"    Usage: icmtool <palette> <image> <dat>\n\n"
			"        palette: .pal file to create ICM from\n"
			"        image: .png image representation of the ICM to be created\n"
			"        dat: VIEW_ICM.DAT to generate\n");
		return 0;
	}
	ICM icm(argv[1]);
	icm.make_image(argv[2]);
	icm.make_icm(argv[3]);
	printf("Operation completed successfully\n");
#endif
#endif
#ifdef SHADOWCOL
	if (argc != 3)
	{
		printf("shadowcol v1.0 - Create SHADOW.COL from palette\n\n"
			"    Usage: shadowcol <palette> <col>\n\n"
			"        palette: .pal file to create SHADOW.COL from\n"
			"        col: SHADOW.COL to generate\n");
		return 0;
	}
	printf("Loading palette...");
	T_PALETTE pal(argv[1]);
	printf(" ok\n");
	printf("Writing SHADOW.COL...");
	FILE* g;
	fopen_s(&g, argv[2], "wt");
	if (!g)
	{
		printf(" error: cannot write file\n");
		exit(0);
	}
	COLORREF col;
	for (int i = 0; i < 256; i++)
	{
		col = pal.get_entry(i);
		hsv hsv;
		rgb rgb;
		rgb.r = (double)GetRValue(col) / 255;
		rgb.g = (double)GetGValue(col) / 255;
		rgb.b = (double)GetBValue(col) / 255;
		hsv = rgb2hsv(rgb);
		hsv.s *= 0.70; //70
		hsv.v *= 0.50; //50
		rgb = hsv2rgb(hsv);
		col = RGB(rgb.r * 255, rgb.g * 255, rgb.b * 255);
		col = pal.get_entry(pal.get_index(col));
		fprintf_s(g, "%d\n", pal.get_index(col));
	}
	fclose(g);
	printf(" ok\n");
#endif
#ifdef MAKEBLN
#ifndef _DEBUG
	if (argc != 3)
	{
		printf("makebln v1.0 - Create BLN from palette\n\n"
			"    Usage: makebln <palette> <bln>\n\n"
			"        palette: .pal file to create BLN from\n"
			"        bln: *.bln file to generate\n");
		return 0;
	}
	BLN bln(argv[1]);
	bln.make_bln(argv[2]);
#else
	BLN bln("C:\\Users\\Vasya\\Documents\\SWGB\\Campaign\\Media\\2backgrd1.pal");
	bln.make_bln("C:\\Users\\Vasya\\Documents\\SWGB\\Campaign\\Media\\2cam1.bln.raw");
#endif
#endif
#ifdef _DEBUG
	system("pause");
#endif
    return 0;
}

//4830 - check in game!
